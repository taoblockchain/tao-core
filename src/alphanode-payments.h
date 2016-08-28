

// Copyright (c) 2014-2015 The Dash developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ALPHANODE_PAYMENTS_H
#define ALPHANODE_PAYMENTS_H

#include "sync.h"
#include "net.h"
#include "key.h"
#include "util.h"
#include "base58.h"
#include "main.h"
#include "alphanode.h"
#include "timedata.h"

using namespace std;

class CAlphanodePayments;
class CAlphanodePaymentWinner;

extern CAlphanodePayments alphanodePayments;
extern map<uint256, CAlphanodePaymentWinner> mapSeenAlphanodeVotes;

void ProcessMessageAlphanodePayments(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);


// for storing the winning payments
class CAlphanodePaymentWinner
{
public:
    int nBlockHeight;
    CTxIn vin;
    CScript payee;
    std::vector<unsigned char> vchSig;
    uint64_t score;

    CAlphanodePaymentWinner() {
        nBlockHeight = 0;
        score = 0;
        vin = CTxIn();
        payee = CScript();
    }

    uint256 GetHash(){
        uint256 n2 = Hash(BEGIN(nBlockHeight), END(nBlockHeight));
        uint256 n3 = vin.prevout.hash > n2 ? (vin.prevout.hash - n2) : (n2 - vin.prevout.hash);

        return n3;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nBlockHeight);
        READWRITE(payee);
        READWRITE(vin);
        READWRITE(score);
        READWRITE(vchSig);
    )
};

//
// Alpha Node Payments Class
// Keeps track of who should get paid for which blocks
//

class CAlphanodePayments
{
private:
    std::vector<CAlphanodePaymentWinner> vWinning;
    int nSyncedFromPeer;
    std::string strMasterPrivKey;
    std::string strMainPubKey;
    bool enabled;
    int nLastBlockHeight;

public:

    CAlphanodePayments() {
        strMainPubKey = "";
        enabled = false;
    }

    bool SetPrivKey(std::string strPrivKey);
    bool CheckSignature(CAlphanodePaymentWinner& winner);
    bool Sign(CAlphanodePaymentWinner& winner);

    // Deterministically calculate a given "score" for a alphanode depending on how close it's hash is
    // to the blockHeight. The further away they are the better, the furthest will win the election
    // and get paid this block
    //

    uint64_t CalculateScore(uint256 blockHash, CTxIn& vin);
    bool GetWinningAlphanode(int nBlockHeight, CTxIn& vinOut);
    bool AddWinningAlphanode(CAlphanodePaymentWinner& winner);
    bool ProcessBlock(int nBlockHeight);
    void Relay(CAlphanodePaymentWinner& winner);
    void Sync(CNode* node);
    void CleanPaymentList();
    int LastPayment(CAlphanode& mn);
    int GetMinAlphanodePaymentsProto();

    bool GetBlockPayee(int nBlockHeight, CScript& payee, CTxIn& vin);
};


#endif