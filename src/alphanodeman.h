
// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ALPHANODEMAN_H
#define ALPHANODEMAN_H

#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "key.h"
#include "core.h"
#include "util.h"
#include "script.h"
#include "base58.h"
#include "main.h"
#include "alphanode.h"

#define ALPHANODES_DUMP_SECONDS               (15*60)
#define ALPHANODES_DSEG_SECONDS               (3*60*60)

using namespace std;

class CAlphanodeMan;

extern CAlphanodeMan mnodeman;

void DumpAlphanodes();

/** Access to the MN database (alphacache.dat) */
class CAlphanodeDB
{
private:
    boost::filesystem::path pathMN;
    std::string strMagicMessage;
public:
    enum ReadResult {
        Ok,
       FileError,
        HashReadError,
        IncorrectHash,
        IncorrectMagicMessage,
        IncorrectMagicNumber,
        IncorrectFormat
    };

    CAlphanodeDB();
    bool Write(const CAlphanodeMan &mnodemanToSave);
    ReadResult Read(CAlphanodeMan& mnodemanToLoad);
};

class CAlphanodeMan
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

    // map to hold all MNs
    std::vector<CAlphanode> vAlphanodes;
    // who's asked for the alphanode list and the last time
    std::map<CNetAddr, int64_t> mAskedUsForAlphanodeList;
    // who we asked for the alphanode list and the last time
    std::map<CNetAddr, int64_t> mWeAskedForAlphanodeList;
    // which alphanodes we've asked for
    std::map<COutPoint, int64_t> mWeAskedForAlphanodeListEntry;

public:
    // keep track of dsq count to prevent alphanodes from gaming darksend queue
    int64_t nDsqCount;

    IMPLEMENT_SERIALIZE
    (
        // serialized format:
        // * version byte (currently 0)
        // * alphanodes vector
        {
                LOCK(cs);
                unsigned char nVersion = 0;
                READWRITE(nVersion);
                READWRITE(vAlphanodes);
                READWRITE(mAskedUsForAlphanodeList);
                READWRITE(mWeAskedForAlphanodeList);
                READWRITE(mWeAskedForAlphanodeListEntry);
                READWRITE(nDsqCount);
        }
    )

    CAlphanodeMan();
    CAlphanodeMan(CAlphanodeMan& other);

    // Add an entry
    bool Add(CAlphanode &mn);

    // Check all alphanodes
    void Check();

    /// Ask (source) node for mnb
    void AskForMN(CNode *pnode, CTxIn &vin);

    // Check all alphanodes and remove inactive
    void CheckAndRemove();

    // Clear alphanode vector
    void Clear();

    int CountEnabled(int protocolVersion = -1);

    int CountAlphanodesAboveProtocol(int protocolVersion);

    void DsegUpdate(CNode* pnode);

    // Find an entry
    CAlphanode* Find(const CTxIn& vin);
    CAlphanode* Find(const CPubKey& pubKeyAlphanode);

    //Find an entry thta do not match every entry provided vector
    CAlphanode* FindOldestNotInVec(const std::vector<CTxIn> &vVins, int nMinimumAge);

    // Find a random entry
    CAlphanode* FindRandom();

    /// Find a random entry
    CAlphanode* FindRandomNotInVec(std::vector<CTxIn> &vecToExclude, int protocolVersion = -1);

    // Get the current winner for this block
    CAlphanode* GetCurrentAlphaNode(int mod=1, int64_t nBlockHeight=0, int minProtocol=0);

    std::vector<CAlphanode> GetFullAlphanodeVector() { Check(); return vAlphanodes; }

    std::vector<pair<int, CAlphanode> > GetAlphanodeRanks(int64_t nBlockHeight, int minProtocol=0);
    int GetAlphanodeRank(const CTxIn &vin, int64_t nBlockHeight, int minProtocol=0, bool fOnlyActive=true);
    CAlphanode* GetAlphanodeByRank(int nRank, int64_t nBlockHeight, int minProtocol=0, bool fOnlyActive=true);

    void ProcessAlphanodeConnections();

    void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);

    // Return the number of (unique) alphanodes
    int size() { return vAlphanodes.size(); }

    std::string ToString() const;

    //
    // Relay Alpha Node Messages
    //

    void RelayAlphanodeEntry(const CTxIn vin, const CService addr, const std::vector<unsigned char> vchSig, const int64_t nNow, const CPubKey pubkey, const CPubKey pubkey2, const int count, const int current, const int64_t lastUpdated, const int protocolVersion, CScript donationAddress, int donationPercentage);
    void RelayAlphanodeEntryPing(const CTxIn vin, const std::vector<unsigned char> vchSig, const int64_t nNow, const bool stop);

    void Remove(CTxIn vin);

};

#endif
