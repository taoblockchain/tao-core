// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Darkcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ALPHANODE_H
#define ALPHANODE_H

#include "uint256.h"
#include "sync.h"
#include "net.h"
#include "key.h"
#include "util.h"
#include "base58.h"
#include "main.h"
#include "timedata.h"
#include "script.h"
#include "alphanode.h"

class uint256;

#define ALPHANODE_NOT_PROCESSED               0 // initial state
#define ALPHANODE_IS_CAPABLE                  1
#define ALPHANODE_NOT_CAPABLE                 2
#define ALPHANODE_STOPPED                     3
#define ALPHANODE_INPUT_TOO_NEW               4
#define ALPHANODE_PORT_NOT_OPEN               6
#define ALPHANODE_PORT_OPEN                   7
#define ALPHANODE_SYNC_IN_PROCESS             8
#define ALPHANODE_REMOTELY_ENABLED            9

#define ALPHANODE_MIN_CONFIRMATIONS           7
#define ALPHANODE_MIN_DSEEP_SECONDS           (30*60)
#define ALPHANODE_MIN_DSEE_SECONDS            (5*60)
#define ALPHANODE_PING_SECONDS                (1*60)
#define ALPHANODE_EXPIRATION_SECONDS          (65*60)
#define ALPHANODE_REMOVAL_SECONDS             (70*60)

using namespace std;

class CAlphanode;

extern CCriticalSection cs_alphanodes;
extern map<int64_t, uint256> mapCacheBlockHashes;

bool GetBlockHash(uint256& hash, int nBlockHeight);

//
// The Alpha Node Class. For managing the darksend process. It contains the input of the 1000TX, signature to prove
// it's the one who own that ip address and code for calculating the payment election.
//
class CAlphanode
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

public:
    enum state {
        ALPHANODE_ENABLED = 1,
        ALPHANODE_EXPIRED = 2,
        ALPHANODE_VIN_SPENT = 3,
        ALPHANODE_REMOVE = 4,
        ALPHANODE_POS_ERROR = 5
    };

    CTxIn vin;  
    CService addr;
    CPubKey pubkey;
    CPubKey pubkey2;
    std::vector<unsigned char> sig;
    int activeState;
    int64_t sigTime; //dsee message times
    int64_t lastDseep;
    int64_t lastTimeSeen;
    int cacheInputAge;
    int cacheInputAgeBlock;
    bool unitTest;
    bool allowFreeTx;
    int protocolVersion;
    int64_t nLastDsq; //the dsq count from the last dsq broadcast of this node
    CScript donationAddress;
    int donationPercentage;
    int nVote;
    int64_t lastVote;
    int nScanningErrorCount;
    int nLastScanningErrorBlockHeight;
    int64_t nLastPaid;


    CAlphanode();
    CAlphanode(const CAlphanode& other);
    CAlphanode(CService newAddr, CTxIn newVin, CPubKey newPubkey, std::vector<unsigned char> newSig, int64_t newSigTime, CPubKey newPubkey2, int protocolVersionIn, CScript donationAddress, int donationPercentage);


    void swap(CAlphanode& first, CAlphanode& second) // nothrow
    {
        // enable ADL (not necessary in our case, but good practice)
        using std::swap;

        // by swapping the members of two classes,
        // the two classes are effectively swapped
        swap(first.vin, second.vin);
        swap(first.addr, second.addr);
        swap(first.pubkey, second.pubkey);
        swap(first.pubkey2, second.pubkey2);
        swap(first.sig, second.sig);
        swap(first.activeState, second.activeState);
        swap(first.sigTime, second.sigTime);
        swap(first.lastDseep, second.lastDseep);
        swap(first.lastTimeSeen, second.lastTimeSeen);
        swap(first.cacheInputAge, second.cacheInputAge);
        swap(first.cacheInputAgeBlock, second.cacheInputAgeBlock);
        swap(first.unitTest, second.unitTest);
        swap(first.allowFreeTx, second.allowFreeTx);
        swap(first.protocolVersion, second.protocolVersion);
        swap(first.nLastDsq, second.nLastDsq);
        swap(first.donationAddress, second.donationAddress);
        swap(first.donationPercentage, second.donationPercentage);
        swap(first.nVote, second.nVote);
        swap(first.lastVote, second.lastVote);
        swap(first.nScanningErrorCount, second.nScanningErrorCount);
        swap(first.nLastScanningErrorBlockHeight, second.nLastScanningErrorBlockHeight);
        swap(first.nLastPaid, second.nLastPaid);
    }

    CAlphanode& operator=(CAlphanode from)
    {
        swap(*this, from);
        return *this;
    }
    friend bool operator==(const CAlphanode& a, const CAlphanode& b)
    {
        return a.vin == b.vin;
    }
    friend bool operator!=(const CAlphanode& a, const CAlphanode& b)
    {
        return !(a.vin == b.vin);
    }

    uint256 CalculateScore(int mod=1, int64_t nBlockHeight=0);

    IMPLEMENT_SERIALIZE
    (
        // serialized format:
        // * version byte (currently 0)
        // * all fields (?)
        {
                LOCK(cs);
                unsigned char nVersion = 0;
                READWRITE(nVersion);
                READWRITE(vin);
                READWRITE(addr);
                READWRITE(pubkey);
                READWRITE(pubkey2);
                READWRITE(sig);
                READWRITE(activeState);
                READWRITE(sigTime);
                READWRITE(lastDseep);
                READWRITE(lastTimeSeen);
                READWRITE(cacheInputAge);
                READWRITE(cacheInputAgeBlock);
                READWRITE(unitTest);
                READWRITE(allowFreeTx);
                READWRITE(protocolVersion);
                READWRITE(nLastDsq);
                READWRITE(donationAddress);
                READWRITE(donationPercentage);
                READWRITE(nVote);
                READWRITE(lastVote);
                READWRITE(nScanningErrorCount);
                READWRITE(nLastScanningErrorBlockHeight);
                READWRITE(nLastPaid);
        }
    )

    int64_t SecondsSincePayment()
    {
        return (GetAdjustedTime() - nLastPaid);
    }

    void UpdateLastSeen(int64_t override=0)
    {
        if(override == 0){
            lastTimeSeen = GetAdjustedTime();
        } else {
            lastTimeSeen = override;
        }
    }

    inline uint64_t SliceHash(uint256& hash, int slice)
    {
        uint64_t n = 0;
        memcpy(&n, &hash+slice*64, 64);
        return n;
    }

    void Check();

    bool UpdatedWithin(int seconds)
    {
        // LogPrintf("UpdatedWithin %d, %d --  %d \n", GetAdjustedTime() , lastTimeSeen, (GetAdjustedTime() - lastTimeSeen) < seconds);

        return (GetAdjustedTime() - lastTimeSeen) < seconds;
    }

    void Disable()
    {
        lastTimeSeen = 0;
    }

    bool IsEnabled()
    {
        return activeState == ALPHANODE_ENABLED;
    }

    int GetAlphanodeInputAge()
    {
        if(pindexBest == NULL) return 0;

        if(cacheInputAge == 0){
            cacheInputAge = GetInputAge(vin);
            cacheInputAgeBlock = pindexBest->nHeight;
        }

        return cacheInputAge+(pindexBest->nHeight-cacheInputAgeBlock);
    }

    std::string Status() {
        std::string strStatus = "ACTIVE";

        if(activeState == CAlphanode::ALPHANODE_ENABLED) strStatus   = "ENABLED";
        if(activeState == CAlphanode::ALPHANODE_EXPIRED) strStatus   = "EXPIRED";
        if(activeState == CAlphanode::ALPHANODE_VIN_SPENT) strStatus = "VIN_SPENT";
        if(activeState == CAlphanode::ALPHANODE_REMOVE) strStatus    = "REMOVE";
        if(activeState == CAlphanode::ALPHANODE_POS_ERROR) strStatus = "POS_ERROR";

        return strStatus;
    }
};

#endif