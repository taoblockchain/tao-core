#include "alphanodeman.h"
#include "alphanode.h"
#include "activealphanode.h"
#include "darksend.h"
#include "core.h"
#include "util.h"
#include "addrman.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>


/** Alpha Node manager */
CAlphanodeMan mnodeman;
CCriticalSection cs_process_message;

struct CompareValueOnly
{
    bool operator()(const pair<int64_t, CTxIn>& t1,
                    const pair<int64_t, CTxIn>& t2) const
    {
        return t1.first < t2.first;
    }
};
struct CompareValueOnlyMN
{
    bool operator()(const pair<int64_t, CAlphanode>& t1,
                    const pair<int64_t, CAlphanode>& t2) const
    {
        return t1.first < t2.first;
    }
};


//
// CAlphanodeDB
//

CAlphanodeDB::CAlphanodeDB()
{
    pathMN = GetDataDir() / "alphacache.dat";
    strMagicMessage = "AlphanodeCache";
}

bool CAlphanodeDB::Write(const CAlphanodeMan& mnodemanToSave)
{
    int64_t nStart = GetTimeMillis();

    // serialize addresses, checksum data up to that point, then append csum
    CDataStream ssAlphanodes(SER_DISK, CLIENT_VERSION);
    ssAlphanodes << strMagicMessage; // alphanode cache file specific magic message
    ssAlphanodes << FLATDATA(Params().MessageStart()); // network specific magic number
    ssAlphanodes << mnodemanToSave;
    uint256 hash = Hash(ssAlphanodes.begin(), ssAlphanodes.end());
    ssAlphanodes << hash;

    // open output file, and associate with CAutoFile
    FILE *file = fopen(pathMN.string().c_str(), "wb");
    CAutoFile fileout = CAutoFile(file, SER_DISK, CLIENT_VERSION);
    if (!fileout)
        return error("%s : Failed to open file %s", __func__, pathMN.string());

    // Write and commit header, data
    try {
        fileout << ssAlphanodes;
    }
    catch (std::exception &e) {
        return error("%s : Serialize or I/O error - %s", __func__, e.what());
    }
    FileCommit(fileout);
    fileout.fclose();

    LogPrintf("Written info to alphacache.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrintf("  %s\n", mnodemanToSave.ToString());

    return true;
}

CAlphanodeDB::ReadResult CAlphanodeDB::Read(CAlphanodeMan& mnodemanToLoad)
{
    int64_t nStart = GetTimeMillis();
    // open input file, and associate with CAutoFile
    FILE *file = fopen(pathMN.string().c_str(), "rb");
    CAutoFile filein = CAutoFile(file, SER_DISK, CLIENT_VERSION);
    if (!filein)
    {
        error("%s : Failed to open file %s", __func__, pathMN.string());
        return FileError;
    }

    // use file size to size memory buffer
    int fileSize = boost::filesystem::file_size(pathMN);
    int dataSize = fileSize - sizeof(uint256);
    // Don't try to resize to a negative number if file is small
    if (dataSize < 0)
        dataSize = 0;
    vector<unsigned char> vchData;
    vchData.resize(dataSize);
    uint256 hashIn;

    // read data and checksum from file
    try {
        filein.read((char *)&vchData[0], dataSize);
        filein >> hashIn;
    }
    catch (std::exception &e) {
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return HashReadError;
    }
    filein.fclose();

    CDataStream ssAlphanodes(vchData, SER_DISK, CLIENT_VERSION);

    // verify stored checksum matches input data
    uint256 hashTmp = Hash(ssAlphanodes.begin(), ssAlphanodes.end());
    if (hashIn != hashTmp)
    {
        error("%s : Checksum mismatch, data corrupted", __func__);
        return IncorrectHash;
    }

    unsigned char pchMsgTmp[4];
    std::string strMagicMessageTmp;
    try {
        // de-serialize file header (alphanode cache file specific magic message) and ..

        ssAlphanodes >> strMagicMessageTmp;

        // ... verify the message matches predefined one
        if (strMagicMessage != strMagicMessageTmp)
        {
            error("%s : Invalid alphanode cache magic message", __func__);
            return IncorrectMagicMessage;
        }

        // de-serialize file header (network specific magic number) and ..
        ssAlphanodes >> FLATDATA(pchMsgTmp);

        // ... verify the network matches ours
        if (memcmp(pchMsgTmp, Params().MessageStart(), sizeof(pchMsgTmp)))
        {
            error("%s : Invalid network magic number", __func__);
            return IncorrectMagicNumber;
        }

        // de-serialize address data into one CMnList object
        ssAlphanodes >> mnodemanToLoad;
    }
    catch (std::exception &e) {
        mnodemanToLoad.Clear();
        error("%s : Deserialize or I/O error - %s", __func__, e.what());
        return IncorrectFormat;
    }

    mnodemanToLoad.CheckAndRemove(); // clean out expired
    LogPrintf("Loaded info from alphacache.dat  %dms\n", GetTimeMillis() - nStart);
    LogPrintf("  %s\n", mnodemanToLoad.ToString());

    return Ok;
}

void DumpAlphanodes()
{
    int64_t nStart = GetTimeMillis();

    CAlphanodeDB mndb;
    CAlphanodeMan tempMnodeman;

    LogPrintf("Verifying alphacache.dat format...\n");
    CAlphanodeDB::ReadResult readResult = mndb.Read(tempMnodeman);
    // there was an error and it was not an error on file openning => do not proceed
    if (readResult == CAlphanodeDB::FileError)
        LogPrintf("Missing alphanode list file - alphacache.dat, will try to recreate\n");
    else if (readResult != CAlphanodeDB::Ok)
    {
        LogPrintf("Error reading alphacache.dat: ");
        if(readResult == CAlphanodeDB::IncorrectFormat)
            LogPrintf("magic is ok but data has invalid format, will try to recreate\n");
        else
        {
            LogPrintf("file format is unknown or invalid, please fix it manually\n");
            return;
        }
    }
    LogPrintf("Writting info to alphacache.dat...\n");
    mndb.Write(mnodeman);

    LogPrintf("Alpha Node dump finished  %dms\n", GetTimeMillis() - nStart);
}

CAlphanodeMan::CAlphanodeMan() {
    nDsqCount = 0;
}

bool CAlphanodeMan::Add(CAlphanode &mn)
{
    LOCK(cs);

    if (!mn.IsEnabled())
        return false;

    CAlphanode *pmn = Find(mn.vin);

    if (pmn == NULL)
    {
        LogPrint("alphanode", "CAlphanodeMan: Adding new alphanode %s - %i now\n", mn.addr.ToString().c_str(), size() + 1);
        vAlphanodes.push_back(mn);
        return true;
    }

    return false;
}

void CAlphanodeMan::AskForMN(CNode* pnode, CTxIn &vin)
{
    std::map<COutPoint, int64_t>::iterator i = mWeAskedForAlphanodeListEntry.find(vin.prevout);
    if (i != mWeAskedForAlphanodeListEntry.end())
    {
        int64_t t = (*i).second;
        if (GetTime() < t) return; // we've asked recently
    }

    // ask for the mnb info once from the node that sent mnp

    LogPrintf("CAlphanodeMan::AskForMN - Asking node for missing entry, vin: %s\n", vin.ToString());
    pnode->PushMessage("dseg", vin);
    int64_t askAgain = GetTime() + ALPHANODE_MIN_DSEEP_SECONDS;
    mWeAskedForAlphanodeListEntry[vin.prevout] = askAgain;
}

void CAlphanodeMan::Check()
{
    LOCK(cs);

    BOOST_FOREACH(CAlphanode& mn, vAlphanodes)
        mn.Check();
}

void CAlphanodeMan::CheckAndRemove()
{
    LOCK(cs);

    Check();

    //remove inactive
    vector<CAlphanode>::iterator it = vAlphanodes.begin();
    while(it != vAlphanodes.end()){
        if((*it).activeState == CAlphanode::ALPHANODE_REMOVE || (*it).activeState == CAlphanode::ALPHANODE_VIN_SPENT || (*it).protocolVersion < nAlphanodeMinProtocol){
            LogPrint("alphanode", "CAlphanodeMan: Removing inactive alphanode %s - %i now\n", (*it).addr.ToString().c_str(), size() - 1);
            it = vAlphanodes.erase(it);
        } else {
            ++it;
        }
    }

    // check who's asked for the alphanode list
    map<CNetAddr, int64_t>::iterator it1 = mAskedUsForAlphanodeList.begin();
    while(it1 != mAskedUsForAlphanodeList.end()){
        if((*it1).second < GetTime()) {
            mAskedUsForAlphanodeList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check who we asked for the alphanode list
    it1 = mWeAskedForAlphanodeList.begin();
    while(it1 != mWeAskedForAlphanodeList.end()){
        if((*it1).second < GetTime()){
            mWeAskedForAlphanodeList.erase(it1++);
        } else {
            ++it1;
        }
    }

    // check which alphanodes we've asked for
    map<COutPoint, int64_t>::iterator it2 = mWeAskedForAlphanodeListEntry.begin();
    while(it2 != mWeAskedForAlphanodeListEntry.end()){
        if((*it2).second < GetTime()){
            mWeAskedForAlphanodeListEntry.erase(it2++);
        } else {
            ++it2;
        }
    }

}

void CAlphanodeMan::Clear()
{
    LOCK(cs);
    vAlphanodes.clear();
    mAskedUsForAlphanodeList.clear();
    mWeAskedForAlphanodeList.clear();
    mWeAskedForAlphanodeListEntry.clear();
    nDsqCount = 0;
}

int CAlphanodeMan::CountEnabled(int protocolVersion)
{
    int i = 0;
    protocolVersion = protocolVersion == -1 ? alphanodePayments.GetMinAlphanodePaymentsProto() : protocolVersion;

    BOOST_FOREACH(CAlphanode& mn, vAlphanodes) {
        mn.Check();
        if(mn.protocolVersion < protocolVersion || !mn.IsEnabled()) continue;
        i++;
    }

    return i;
}

int CAlphanodeMan::CountAlphanodesAboveProtocol(int protocolVersion)
{
    int i = 0;

    BOOST_FOREACH(CAlphanode& mn, vAlphanodes) {
        mn.Check();
        if(mn.protocolVersion < protocolVersion || !mn.IsEnabled()) continue;
        i++;
    }

    return i;
}

void CAlphanodeMan::DsegUpdate(CNode* pnode)
{
    LOCK(cs);

    std::map<CNetAddr, int64_t>::iterator it = mWeAskedForAlphanodeList.find(pnode->addr);
    if (it != mWeAskedForAlphanodeList.end())
    {
        if (GetTime() < (*it).second) {
            LogPrintf("dseg - we already asked %s for the list; skipping...\n", pnode->addr.ToString());
            return;
        }
    }
    pnode->PushMessage("dseg", CTxIn());
    int64_t askAgain = GetTime() + ALPHANODES_DSEG_SECONDS;
    mWeAskedForAlphanodeList[pnode->addr] = askAgain;
}

CAlphanode *CAlphanodeMan::Find(const CTxIn &vin)
{
    LOCK(cs);

    BOOST_FOREACH(CAlphanode& mn, vAlphanodes)
    {
        if(mn.vin.prevout == vin.prevout)
            return &mn;
    }
    return NULL;
}

CAlphanode* CAlphanodeMan::FindOldestNotInVec(const std::vector<CTxIn> &vVins, int nMinimumAge)
{
    LOCK(cs);

    CAlphanode *pOldestAlphanode = NULL;

    BOOST_FOREACH(CAlphanode &mn, vAlphanodes)
    {   
        mn.Check();
        if(!mn.IsEnabled()) continue;

        if(mn.GetAlphanodeInputAge() < nMinimumAge) continue;

        bool found = false;
        BOOST_FOREACH(const CTxIn& vin, vVins)
            if(mn.vin.prevout == vin.prevout)
            {   
                found = true;
                break;
            }
        if(found) continue;

        if(pOldestAlphanode == NULL || pOldestAlphanode->SecondsSincePayment() < mn.SecondsSincePayment())
        {
            pOldestAlphanode = &mn;
        }
    }

    return pOldestAlphanode;
}

CAlphanode *CAlphanodeMan::FindRandom()
{
    LOCK(cs);

    if(size() == 0) return NULL;

    return &vAlphanodes[GetRandInt(vAlphanodes.size())];
}

CAlphanode *CAlphanodeMan::Find(const CPubKey &pubKeyAlphanode)
{
    LOCK(cs);

    BOOST_FOREACH(CAlphanode& mn, vAlphanodes)
    {
        if(mn.pubkey2 == pubKeyAlphanode)
            return &mn;
    }
    return NULL;
}

CAlphanode *CAlphanodeMan::FindRandomNotInVec(std::vector<CTxIn> &vecToExclude, int protocolVersion)
{
    LOCK(cs);

    protocolVersion = protocolVersion == -1 ? alphanodePayments.GetMinAlphanodePaymentsProto() : protocolVersion;

    int nCountEnabled = CountEnabled(protocolVersion);
    LogPrintf("CAlphanodeMan::FindRandomNotInVec - nCountEnabled - vecToExclude.size() %d\n", nCountEnabled - vecToExclude.size());
    if(nCountEnabled - vecToExclude.size() < 1) return NULL;

    int rand = GetRandInt(nCountEnabled - vecToExclude.size());
    LogPrintf("CAlphanodeMan::FindRandomNotInVec - rand %d\n", rand);
    bool found;

    BOOST_FOREACH(CAlphanode &mn, vAlphanodes) {
        if(mn.protocolVersion < protocolVersion || !mn.IsEnabled()) continue;
        found = false;
        BOOST_FOREACH(CTxIn &usedVin, vecToExclude) {
            if(mn.vin.prevout == usedVin.prevout) {
                found = true;
                break;
            }
        }
        if(found) continue;
        if(--rand < 1) {
            return &mn;
        }
    }

    return NULL;
}

CAlphanode* CAlphanodeMan::GetCurrentAlphaNode(int mod, int64_t nBlockHeight, int minProtocol)
{
    unsigned int score = 0;
    CAlphanode* winner = NULL;

    // scan for winner
    BOOST_FOREACH(CAlphanode& mn, vAlphanodes) {
        mn.Check();
        if(mn.protocolVersion < minProtocol || !mn.IsEnabled()) continue;

        // calculate the score for each alphanode
        uint256 n = mn.CalculateScore(mod, nBlockHeight);
        unsigned int n2 = 0;
        memcpy(&n2, &n, sizeof(n2));

        // determine the winner
        if(n2 > score){
            score = n2;
            winner = &mn;
        }
    }

    return winner;
}

int CAlphanodeMan::GetAlphanodeRank(const CTxIn& vin, int64_t nBlockHeight, int minProtocol, bool fOnlyActive)
{
    std::vector<pair<unsigned int, CTxIn> > vecAlphanodeScores;

    //make sure we know about this block
    uint256 hash = 0;
    if(!GetBlockHash(hash, nBlockHeight)) return -1;

    // scan for winner
    BOOST_FOREACH(CAlphanode& mn, vAlphanodes) {

        if(mn.protocolVersion < minProtocol) continue;
        if(fOnlyActive) {
            mn.Check();
            if(!mn.IsEnabled()) continue;
        }

        uint256 n = mn.CalculateScore(1, nBlockHeight);
        unsigned int n2 = 0;
        memcpy(&n2, &n, sizeof(n2));

        vecAlphanodeScores.push_back(make_pair(n2, mn.vin));
    }

    sort(vecAlphanodeScores.rbegin(), vecAlphanodeScores.rend(), CompareValueOnly());

    int rank = 0;
    BOOST_FOREACH (PAIRTYPE(unsigned int, CTxIn)& s, vecAlphanodeScores){
        rank++;
        if(s.second == vin) {
            return rank;
        }
    }

    return -1;
}

std::vector<pair<int, CAlphanode> > CAlphanodeMan::GetAlphanodeRanks(int64_t nBlockHeight, int minProtocol)
{
    std::vector<pair<unsigned int, CAlphanode> > vecAlphanodeScores;
    std::vector<pair<int, CAlphanode> > vecAlphanodeRanks;

    //make sure we know about this block
    uint256 hash = 0;
    if(!GetBlockHash(hash, nBlockHeight)) return vecAlphanodeRanks;

    // scan for winner
    BOOST_FOREACH(CAlphanode& mn, vAlphanodes) {

        mn.Check();

        if(mn.protocolVersion < minProtocol) continue;
        if(!mn.IsEnabled()) {
            continue;
        }

        uint256 n = mn.CalculateScore(1, nBlockHeight);
        unsigned int n2 = 0;
        memcpy(&n2, &n, sizeof(n2));

        vecAlphanodeScores.push_back(make_pair(n2, mn));
    }

    sort(vecAlphanodeScores.rbegin(), vecAlphanodeScores.rend(), CompareValueOnlyMN());

    int rank = 0;
    BOOST_FOREACH (PAIRTYPE(unsigned int, CAlphanode)& s, vecAlphanodeScores){
        rank++;
        vecAlphanodeRanks.push_back(make_pair(rank, s.second));
    }

    return vecAlphanodeRanks;
}

CAlphanode* CAlphanodeMan::GetAlphanodeByRank(int nRank, int64_t nBlockHeight, int minProtocol, bool fOnlyActive)
{
    std::vector<pair<unsigned int, CTxIn> > vecAlphanodeScores;

    // scan for winner
    BOOST_FOREACH(CAlphanode& mn, vAlphanodes) {

        if(mn.protocolVersion < minProtocol) continue;
        if(fOnlyActive) {
            mn.Check();
            if(!mn.IsEnabled()) continue;
        }

        uint256 n = mn.CalculateScore(1, nBlockHeight);
        unsigned int n2 = 0;
        memcpy(&n2, &n, sizeof(n2));

        vecAlphanodeScores.push_back(make_pair(n2, mn.vin));
    }

    sort(vecAlphanodeScores.rbegin(), vecAlphanodeScores.rend(), CompareValueOnly());

    int rank = 0;
    BOOST_FOREACH (PAIRTYPE(unsigned int, CTxIn)& s, vecAlphanodeScores){
        rank++;
        if(rank == nRank) {
            return Find(s.second);
        }
    }

    return NULL;
}

void CAlphanodeMan::ProcessAlphanodeConnections()
{
    LOCK(cs_vNodes);

    if(!darkSendPool.pSubmittedToAlphanode) return;
    
    BOOST_FOREACH(CNode* pnode, vNodes)
    {
        if(darkSendPool.pSubmittedToAlphanode->addr == pnode->addr) continue;

        if(pnode->fDarkSendMaster){
            LogPrintf("Closing alphanode connection %s \n", pnode->addr.ToString().c_str());
            pnode->CloseSocketDisconnect();
        }
    }
}

void CAlphanodeMan::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{

    //Normally would disable functionality, NEED this enabled for staking.
    //if(fLiteMode) return;

    if(!darkSendPool.IsBlockchainSynced()) return;

    LOCK(cs_process_message);

    if (strCommand == "dsee") { //AlphaSend Election Entry

        CTxIn vin;
        CService addr;
        CPubKey pubkey;
        CPubKey pubkey2;
        vector<unsigned char> vchSig;
        int64_t sigTime;
        int count;
        int current;
        int64_t lastUpdated;
        int protocolVersion;
        CScript donationAddress;
        int donationPercentage;
        std::string strMessage;

        // 70047 and greater
        vRecv >> vin >> addr >> vchSig >> sigTime >> pubkey >> pubkey2 >> count >> current >> lastUpdated >> protocolVersion >> donationAddress >> donationPercentage;

        // make sure signature isn't in the future (past is OK)
        if (sigTime > GetAdjustedTime() + 60 * 60) {
            LogPrintf("dsee - Signature rejected, too far into the future %s\n", vin.ToString().c_str());
            return;
        }

        bool isLocal = addr.IsRFC1918() || addr.IsLocal();
        //if(RegTest()) isLocal = false;

        std::string vchPubKey(pubkey.begin(), pubkey.end());
        std::string vchPubKey2(pubkey2.begin(), pubkey2.end());

        strMessage = addr.ToString() + boost::lexical_cast<std::string>(sigTime) + vchPubKey + vchPubKey2 + boost::lexical_cast<std::string>(protocolVersion)  + donationAddress.ToString() + boost::lexical_cast<std::string>(donationPercentage);

        if(donationPercentage < 0 || donationPercentage > 100){
            LogPrintf("dsee - donation percentage out of range %d\n", donationPercentage);
            return;     
        }
        if(protocolVersion < MIN_POOL_PEER_PROTO_VERSION) {
            LogPrintf("dsee - ignoring outdated alphanode %s protocol version %d\n", vin.ToString().c_str(), protocolVersion);
            return;
        }

        CScript pubkeyScript;
        pubkeyScript.SetDestination(pubkey.GetID());

        if(pubkeyScript.size() != 25) {
            LogPrintf("dsee - pubkey the wrong size\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        CScript pubkeyScript2;
        pubkeyScript2.SetDestination(pubkey2.GetID());

        if(pubkeyScript2.size() != 25) {
            LogPrintf("dsee - pubkey2 the wrong size\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        if(!vin.scriptSig.empty()) {
            LogPrintf("dsee - Ignore Not Empty ScriptSig %s\n",vin.ToString().c_str());
            return;
        }

        std::string errorMessage = "";
        if(!darkSendSigner.VerifyMessage(pubkey, vchSig, strMessage, errorMessage)){
            LogPrintf("dsee - Got bad alphanode address signature\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        //search existing alphanode list, this is where we update existing alphanodes with new dsee broadcasts
        CAlphanode* pmn = this->Find(vin);
        // if we are a alphanode but with undefined vin and this dsee is ours (matches our Alpha Node privkey) then just skip this part
        if(pmn != NULL && !(fAlphaNode && activeAlphanode.vin == CTxIn() && pubkey2 == activeAlphanode.pubKeyAlphanode))
        {
            // count == -1 when it's a new entry
            //   e.g. We don't want the entry relayed/time updated when we're syncing the list
            // mn.pubkey = pubkey, IsVinAssociatedWithPubkey is validated once below,
            //   after that they just need to match
            if(count == -1 && pmn->pubkey == pubkey && !pmn->UpdatedWithin(ALPHANODE_MIN_DSEE_SECONDS)){
                pmn->UpdateLastSeen();

                if(pmn->sigTime < sigTime){ //take the newest entry
                    LogPrintf("dsee - Got updated entry for %s\n", addr.ToString().c_str());
                    pmn->pubkey2 = pubkey2;
                    pmn->sigTime = sigTime;
                    pmn->sig = vchSig;
                    pmn->protocolVersion = protocolVersion;
                    pmn->addr = addr;
                    pmn->donationAddress = donationAddress;
                    pmn->donationPercentage = donationPercentage;
                    pmn->Check();
                    if(pmn->IsEnabled())
                        mnodeman.RelayAlphanodeEntry(vin, addr, vchSig, sigTime, pubkey, pubkey2, count, current, lastUpdated, protocolVersion, donationAddress, donationPercentage);
                }
            }

            return;
        }

        // make sure the vout that was signed is related to the transaction that spawned the alphanode
        //  - this is expensive, so it's only done once per alphanode
        if(!darkSendSigner.IsVinAssociatedWithPubkey(vin, pubkey)) {
            LogPrintf("dsee - Got mismatched pubkey and vin\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        LogPrint("alphanode", "dsee - Got NEW alphanode entry %s\n", addr.ToString().c_str());

        // make sure it's still unspent
        //  - this is checked later by .check() in many places and by ThreadCheckDarkSendPool()

        CValidationState state;
        CTransaction tx = CTransaction();
        CTxOut vout = CTxOut(Params().DarkSendPoolMax(), darkSendPool.collateralPubKey);
        tx.vin.push_back(vin);
        tx.vout.push_back(vout);
        bool fAcceptable = false;
        {
            TRY_LOCK(cs_main, lockMain);
            if(!lockMain) return;
            fAcceptable = AcceptableInputs(mempool, tx, false, NULL);
        }
        if(fAcceptable){
            LogPrint("alphanode", "dsee - Accepted alphanode entry %i %i\n", count, current);

            if(GetInputAge(vin) < ALPHANODE_MIN_CONFIRMATIONS){
                LogPrintf("dsee - Input must have least %d confirmations\n", ALPHANODE_MIN_CONFIRMATIONS);
                Misbehaving(pfrom->GetId(), 20);
                return;
            }

            // verify that sig time is legit in past
            // should be at least not earlier than block when 10000 TansferCoin tx got ALPHANODE_MIN_CONFIRMATIONS
            uint256 hashBlock = 0;
            GetTransaction(vin.prevout.hash, tx, hashBlock);
            map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
           if (mi != mapBlockIndex.end() && (*mi).second)
            {
                CBlockIndex* pMNIndex = (*mi).second; // block for 10000 TansferCoin tx -> 1 confirmation
                CBlockIndex* pConfIndex = FindBlockByHeight((pMNIndex->nHeight + ALPHANODE_MIN_CONFIRMATIONS - 1)); // block where tx got ALPHANODE_MIN_CONFIRMATIONS
                if(pConfIndex->GetBlockTime() > sigTime)
                {
                    LogPrintf("dsee - Bad sigTime %d for alphanode %20s %105s (%i conf block is at %d)\n",
                              sigTime, addr.ToString(), vin.ToString(), ALPHANODE_MIN_CONFIRMATIONS, pConfIndex->GetBlockTime());
                    return;
                }
            }


            // use this as a peer
            addrman.Add(CAddress(addr), pfrom->addr, 2*60*60);

            //doesn't support multisig addresses
            if(donationAddress.IsPayToScriptHash()){
                donationAddress = CScript();
                donationPercentage = 0;
            }

            // add our alphanode
            CAlphanode mn(addr, vin, pubkey, vchSig, sigTime, pubkey2, protocolVersion, donationAddress, donationPercentage);
            mn.UpdateLastSeen(lastUpdated);
            this->Add(mn);

            // if it matches our alphanodeprivkey, then we've been remotely activated
            if(pubkey2 == activeAlphanode.pubKeyAlphanode && protocolVersion == PROTOCOL_VERSION){
                activeAlphanode.EnableHotColdAlphaNode(vin, addr);
            }

            if(count == -1 && !isLocal)
                mnodeman.RelayAlphanodeEntry(vin, addr, vchSig, sigTime, pubkey, pubkey2, count, current, lastUpdated, protocolVersion, donationAddress, donationPercentage);

        } else {
            LogPrintf("dsee - Rejected alphanode entry %s\n", addr.ToString().c_str());

            int nDoS = 0;
            if (state.IsInvalid(nDoS))
            {
                LogPrintf("dsee - %s from %s %s was not accepted into the memory pool\n", tx.GetHash().ToString().c_str(),
                    pfrom->addr.ToString().c_str(), pfrom->cleanSubVer.c_str());
                if (nDoS > 0)
                    Misbehaving(pfrom->GetId(), nDoS);
            }
        }
    }

    else if (strCommand == "dseep") { //AlphaSend Election Entry Ping

        CTxIn vin;
        vector<unsigned char> vchSig;
        int64_t sigTime;
        bool stop;
        vRecv >> vin >> vchSig >> sigTime >> stop;

        //LogPrintf("dseep - Received: vin: %s sigTime: %lld stop: %s\n", vin.ToString().c_str(), sigTime, stop ? "true" : "false");

        if (sigTime > GetAdjustedTime() + 60 * 60) {
            LogPrintf("dseep - Signature rejected, too far into the future %s\n", vin.ToString().c_str());
            return;
        }

        if (sigTime <= GetAdjustedTime() - 60 * 60) {
            LogPrintf("dseep - Signature rejected, too far into the past %s - %d %d \n", vin.ToString().c_str(), sigTime, GetAdjustedTime());
            return;
        }

        // see if we have this alphanode
        CAlphanode* pmn = this->Find(vin);
        if(pmn != NULL && pmn->protocolVersion >= MIN_POOL_PEER_PROTO_VERSION)
        {
            // LogPrintf("dseep - Found corresponding mn for vin: %s\n", vin.ToString().c_str());
            // take this only if it's newer
            if(pmn->lastDseep < sigTime)
            {
                std::string strMessage = pmn->addr.ToString() + boost::lexical_cast<std::string>(sigTime) + boost::lexical_cast<std::string>(stop);

                std::string errorMessage = "";
                if(!darkSendSigner.VerifyMessage(pmn->pubkey2, vchSig, strMessage, errorMessage))
                {
                    LogPrintf("dseep - Got bad alphanode address signature %s \n", vin.ToString().c_str());
                    //Misbehaving(pfrom->GetId(), 100);
                    return;
                }

                pmn->lastDseep = sigTime;

                if(!pmn->UpdatedWithin(ALPHANODE_MIN_DSEEP_SECONDS))
                {
                    if(stop) pmn->Disable();
                    else
                    {
                        pmn->UpdateLastSeen();
                        pmn->Check();
                        if(!pmn->IsEnabled()) return;
                    }
                    mnodeman.RelayAlphanodeEntryPing(vin, vchSig, sigTime, stop);
                }
            }
            return;
        }

        LogPrint("alphanode", "dseep - Couldn't find alphanode entry %s\n", vin.ToString().c_str());

        std::map<COutPoint, int64_t>::iterator i = mWeAskedForAlphanodeListEntry.find(vin.prevout);
        if (i != mWeAskedForAlphanodeListEntry.end())
        {
            int64_t t = (*i).second;
            if (GetTime() < t) return; // we've asked recently
        }

        // ask for the dsee info once from the node that sent dseep

        LogPrintf("dseep - Asking source node for missing entry %s\n", vin.ToString().c_str());
        pfrom->PushMessage("dseg", vin);
        int64_t askAgain = GetTime()+ ALPHANODE_MIN_DSEEP_SECONDS;
        mWeAskedForAlphanodeListEntry[vin.prevout] = askAgain;

    } else if (strCommand == "mvote") { //Alpha Node Vote

        CTxIn vin;
        vector<unsigned char> vchSig;
        int nVote;
        vRecv >> vin >> vchSig >> nVote;

        // see if we have this Alpha Node
        CAlphanode* pmn = this->Find(vin);
        if(pmn != NULL)
        {
            if((GetAdjustedTime() - pmn->lastVote) > (60*60))
            {
                std::string strMessage = vin.ToString() + boost::lexical_cast<std::string>(nVote);

                std::string errorMessage = "";
                if(!darkSendSigner.VerifyMessage(pmn->pubkey2, vchSig, strMessage, errorMessage))
                {
                    LogPrintf("mvote - Got bad Alpha Node address signature %s \n", vin.ToString().c_str());
                    return;
                }

                pmn->nVote = nVote;
                pmn->lastVote = GetAdjustedTime();

                //send to all peers
                LOCK(cs_vNodes);
                BOOST_FOREACH(CNode* pnode, vNodes)
                    pnode->PushMessage("mvote", vin, vchSig, nVote);
            }

            return;
        }

    } else if (strCommand == "dseg") { //Get alphanode list or specific entry

        CTxIn vin;
        vRecv >> vin;

        if(vin == CTxIn()) { //only should ask for this once
            //local network
            if(!pfrom->addr.IsRFC1918() && Params().NetworkID() == CChainParams::MAIN)
            {
                std::map<CNetAddr, int64_t>::iterator i = mAskedUsForAlphanodeList.find(pfrom->addr);
                if (i != mAskedUsForAlphanodeList.end())
                {
                    int64_t t = (*i).second;
                    if (GetTime() < t) {
                        Misbehaving(pfrom->GetId(), 34);
                        LogPrintf("dseg - peer already asked me for the list\n");
                        return;
                    }
                }

                int64_t askAgain = GetTime() + ALPHANODES_DSEG_SECONDS;
                mAskedUsForAlphanodeList[pfrom->addr] = askAgain;
            }
        } //else, asking for a specific node which is ok

        int count = this->size();
        int i = 0;

        BOOST_FOREACH(CAlphanode& mn, vAlphanodes) {

            if(mn.addr.IsRFC1918()) continue; //local network

            if(mn.IsEnabled())
            {
                LogPrint("alphanode", "dseg - Sending alphanode entry - %s \n", mn.addr.ToString().c_str());
                if(vin == CTxIn()){
                    pfrom->PushMessage("dsee", mn.vin, mn.addr, mn.sig, mn.sigTime, mn.pubkey, mn.pubkey2, count, i, mn.lastTimeSeen, mn.protocolVersion, mn.donationAddress, mn.donationPercentage);
                } else if (vin == mn.vin) {
                    pfrom->PushMessage("dsee", mn.vin, mn.addr, mn.sig, mn.sigTime, mn.pubkey, mn.pubkey2, count, i, mn.lastTimeSeen, mn.protocolVersion, mn.donationAddress, mn.donationPercentage);
                    LogPrintf("dseg - Sent 1 alphanode entries to %s\n", pfrom->addr.ToString().c_str());
                    return;
                }
                i++;
            }
        }

        LogPrintf("dseg - Sent %d alphanode entries to %s\n", i, pfrom->addr.ToString().c_str());
    }

}

void CAlphanodeMan::RelayAlphanodeEntry(const CTxIn vin, const CService addr, const std::vector<unsigned char> vchSig, const int64_t nNow, const CPubKey pubkey, const CPubKey pubkey2, const int count, const int current, const int64_t lastUpdated, const int protocolVersion, CScript donationAddress, int donationPercentage)
{
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
        pnode->PushMessage("dsee", vin, addr, vchSig, nNow, pubkey, pubkey2, count, current, lastUpdated, protocolVersion, donationAddress, donationPercentage);
}

void CAlphanodeMan::RelayAlphanodeEntryPing(const CTxIn vin, const std::vector<unsigned char> vchSig, const int64_t nNow, const bool stop)
{
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
        pnode->PushMessage("dseep", vin, vchSig, nNow, stop);
}

void CAlphanodeMan::Remove(CTxIn vin)
{
    LOCK(cs);

    vector<CAlphanode>::iterator it = vAlphanodes.begin();
    while(it != vAlphanodes.end()){
        if((*it).vin == vin){
            LogPrint("alphanode", "CAlphanodeMan: Removing Alpha Node %s - %i now\n", (*it).addr.ToString().c_str(), size() - 1);
            vAlphanodes.erase(it);
            break;
        }
    }
}

std::string CAlphanodeMan::ToString() const
{
    std::ostringstream info;

    info << "alphanodes: " << (int)vAlphanodes.size() <<
            ", peers who asked us for alphanode list: " << (int)mAskedUsForAlphanodeList.size() <<
            ", peers we asked for alphanode list: " << (int)mWeAskedForAlphanodeList.size() <<
            ", entries in Alpha Node list we asked for: " << (int)mWeAskedForAlphanodeListEntry.size() <<
            ", nDsqCount: " << (int)nDsqCount;

    return info.str();
}
