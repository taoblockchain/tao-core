


#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "key.h"
#include "util.h"
#include "script.h"
#include "base58.h"
#include "protocol.h"
#include "activealphanode.h"
#include "alphanodeman.h"
#include "spork.h"
#include <boost/lexical_cast.hpp>
#include "alphanodeman.h"

using namespace std;
using namespace boost;

std::map<uint256, CAlphanodeScanningError> mapAlphanodeScanningErrors;
CAlphanodeScanning mnscan;

/* 
    Alpha Node - Proof of Service 

    -- What it checks

    1.) Making sure Alpha Nodes have their ports open
    2.) Are responding to requests made by the network

    -- How it works

    When a block comes in, DoAlphanodePOS is executed if the client is a 
    alphanode. Using the deterministic ranking algorithm up to 1% of the alphanode 
    network is checked each block. 

    A port is opened from Alpha Node A to Alpha Node B, if successful then nothing happens. 
    If there is an error, a CAlphanodeScanningError object is propagated with an error code.
    Errors are applied to the Alpha Nodes and a score is incremented within the alphanode object,
    after a threshold is met, the alphanode goes into an error state. Each cycle the score is 
    decreased, so if the alphanode comes back online it will return to the list. 

    Alpha Nodes in a error state do not receive payment. 

    -- Future expansion

    We want to be able to prove the nodes have many qualities such as a specific CPU speed, bandwidth,
    and dedicated storage. E.g. We could require a full node be a computer running 2GHz with 10GB of space.

*/

void ProcessMessageAlphanodePOS(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if(IsInitialBlockDownload()) return;

    if (strCommand == "mnse") //Alpha Node Scanning Error
    {

        CDataStream vMsg(vRecv);
        CAlphanodeScanningError mnse;
        vRecv >> mnse;

        CInv inv(MSG_ALPHANODE_SCANNING_ERROR, mnse.GetHash());
        pfrom->AddInventoryKnown(inv);

        if(mapAlphanodeScanningErrors.count(mnse.GetHash())){
            return;
        }
        mapAlphanodeScanningErrors.insert(make_pair(mnse.GetHash(), mnse));

        if(!mnse.IsValid())
        {
            LogPrintf("AlphanodePOS::mnse - Invalid object\n");   
            return;
        }

        CAlphanode* pmnA = mnodeman.Find(mnse.vinAlphanodeA);
        if(pmnA == NULL) return;
        if(pmnA->protocolVersion < MIN_ALPHANODE_POS_PROTO_VERSION) return;

        int nBlockHeight = pindexBest->nHeight;
        if(nBlockHeight - mnse.nBlockHeight > 10){
            LogPrintf("AlphanodePOS::mnse - Too old\n");
            return;   
        }

        // Lowest alphanodes in rank check the highest each block
        int a = mnodeman.GetAlphanodeRank(mnse.vinAlphanodeA, mnse.nBlockHeight, MIN_ALPHANODE_POS_PROTO_VERSION);
        if(a == -1 || a > GetCountScanningPerBlock())
        {
            if(a != -1) LogPrintf("AlphanodePOS::mnse - AlphanodeA ranking is too high\n");
            return;
        }

        int b = mnodeman.GetAlphanodeRank(mnse.vinAlphanodeB, mnse.nBlockHeight, MIN_ALPHANODE_POS_PROTO_VERSION, false);
        if(b == -1 || b < mnodeman.CountAlphanodesAboveProtocol(MIN_ALPHANODE_POS_PROTO_VERSION)-GetCountScanningPerBlock())
        {
            if(b != -1) LogPrintf("AlphanodePOS::mnse - AlphanodeB ranking is too low\n");
            return;
        }

        if(!mnse.SignatureValid()){
            LogPrintf("AlphanodePOS::mnse - Bad alphanode message\n");
            return;
        }

        CAlphanode* pmnB = mnodeman.Find(mnse.vinAlphanodeB);
        if(pmnB == NULL) return;

        if(fDebug) LogPrintf("ProcessMessageAlphanodePOS::mnse - nHeight %d AlphanodeA %s AlphanodeB %s\n", mnse.nBlockHeight, pmnA->addr.ToString().c_str(), pmnB->addr.ToString().c_str());

        pmnB->ApplyScanningError(mnse);
        mnse.Relay();
    }
}

// Returns how many alphanodes are allowed to scan each block
int GetCountScanningPerBlock()
{
    return std::max(1, mnodeman.CountAlphanodesAboveProtocol(MIN_ALPHANODE_POS_PROTO_VERSION)/100);
}


void CAlphanodeScanning::CleanAlphanodeScanningErrors()
{
    if(pindexBest == NULL) return;

    std::map<uint256, CAlphanodeScanningError>::iterator it = mapAlphanodeScanningErrors.begin();

    while(it != mapAlphanodeScanningErrors.end()) {
        if(GetTime() > it->second.nExpiration){ //keep them for an hour
            LogPrintf("Removing old alphanode scanning error %s\n", it->second.GetHash().ToString().c_str());

            mapAlphanodeScanningErrors.erase(it++);
        } else {
            it++;
        }
    }

}

// Check other alphanodes to make sure they're running correctly
void CAlphanodeScanning::DoAlphanodePOSChecks()
{
    if(!fAlphaNode) return;
    if(IsInitialBlockDownload()) return;

    int nBlockHeight = pindexBest->nHeight-5;

    int a = mnodeman.GetAlphanodeRank(activeAlphanode.vin, nBlockHeight, MIN_ALPHANODE_POS_PROTO_VERSION);
    if(a == -1 || a > GetCountScanningPerBlock()){
        // we don't need to do anything this block
        return;
    }

    // The lowest ranking nodes (Alpha Node A) check the highest ranking nodes (Alpha Node B)
    CAlphanode* pmn = mnodeman.GetAlphanodeByRank(mnodeman.CountAlphanodesAboveProtocol(MIN_ALPHANODE_POS_PROTO_VERSION)-a, nBlockHeight, MIN_ALPHANODE_POS_PROTO_VERSION, false);
    if(pmn == NULL) return;

    // -- first check : Port is open

    if(!ConnectNode((CAddress)pmn->addr, NULL, true)){
        // we couldn't connect to the node, let's send a scanning error
        CAlphanodeScanningError mnse(activeAlphanode.vin, pmn->vin, SCANNING_ERROR_NO_RESPONSE, nBlockHeight);
        mnse.Sign();
        mapAlphanodeScanningErrors.insert(make_pair(mnse.GetHash(), mnse));
        mnse.Relay();
    }

    // success
    CAlphanodeScanningError mnse(activeAlphanode.vin, pmn->vin, SCANNING_SUCCESS, nBlockHeight);
    mnse.Sign();
    mapAlphanodeScanningErrors.insert(make_pair(mnse.GetHash(), mnse));
    mnse.Relay();
}

bool CAlphanodeScanningError::SignatureValid()
{
    std::string errorMessage;
    std::string strMessage = vinAlphanodeA.ToString() + vinAlphanodeB.ToString() + 
        boost::lexical_cast<std::string>(nBlockHeight) + boost::lexical_cast<std::string>(nErrorType);

    CAlphanode* pmn = mnodeman.Find(vinAlphanodeA);

    if(pmn == NULL)
    {
        LogPrintf("CAlphanodeScanningError::SignatureValid() - Unknown Alpha Node\n");
        return false;
    }

    CScript pubkey;
    pubkey.SetDestination(pmn->pubkey2.GetID());
    CTxDestination address1;
    ExtractDestination(pubkey, address1);
    CTaoAddress address2(address1);

    if(!darkSendSigner.VerifyMessage(pmn->pubkey2, vchAlphaNodeSignature, strMessage, errorMessage)) {
        LogPrintf("CAlphanodeScanningError::SignatureValid() - Verify message failed\n");
        return false;
    }

    return true;
}

bool CAlphanodeScanningError::Sign()
{
    std::string errorMessage;

    CKey key2;
    CPubKey pubkey2;
    std::string strMessage = vinAlphanodeA.ToString() + vinAlphanodeB.ToString() + 
        boost::lexical_cast<std::string>(nBlockHeight) + boost::lexical_cast<std::string>(nErrorType);

    if(!darkSendSigner.SetKey(strAlphaNodePrivKey, errorMessage, key2, pubkey2))
    {
        LogPrintf("CAlphanodeScanningError::Sign() - ERROR: Invalid alphanodeprivkey: '%s'\n", errorMessage.c_str());
        return false;
    }

    CScript pubkey;
    pubkey.SetDestination(pubkey2.GetID());
    CTxDestination address1;
    ExtractDestination(pubkey, address1);
    CTaoAddress address2(address1);
    //LogPrintf("signing pubkey2 %s \n", address2.ToString().c_str());

    if(!darkSendSigner.SignMessage(strMessage, errorMessage, vchAlphaNodeSignature, key2)) {
        LogPrintf("CAlphanodeScanningError::Sign() - Sign message failed");
        return false;
    }

    if(!darkSendSigner.VerifyMessage(pubkey2, vchAlphaNodeSignature, strMessage, errorMessage)) {
        LogPrintf("CAlphanodeScanningError::Sign() - Verify message failed");
        return false;
    }

    return true;
}

void CAlphanodeScanningError::Relay()
{
    CInv inv(MSG_ALPHANODE_SCANNING_ERROR, GetHash());

    vector<CInv> vInv;
    vInv.push_back(inv);
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes){
        pnode->PushMessage("inv", vInv);
    }
}
