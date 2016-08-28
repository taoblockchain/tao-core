

// Copyright (c) 2014-2015 The Dash developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ALPHANODE_POS_H
#define ALPHANODE_POS_H

#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "key.h"
#include "core.h"
#include "util.h"
#include "script.h"
#include "base58.h"
#include "main.h"

using namespace std;
using namespace boost;

class CAlphanodeScanning;
class CAlphanodeScanningError;

extern map<uint256, CAlphanodeScanningError> mapAlphanodeScanningErrors;
extern CAlphanodeScanning mnscan;

static const int MIN_ALPHANODE_POS_PROTO_VERSION = 61402;

/*
	1% of the network is scanned every 2.5 minutes, making a full
	round of scanning take about 4.16 hours. We're targeting about 
	a day of proof-of-service errors for complete removal from the 
	alphanode system.
*/
static const int ALPHANODE_SCANNING_ERROR_THESHOLD = 6;

#define SCANNING_SUCCESS                       1
#define SCANNING_ERROR_NO_RESPONSE             2
#define SCANNING_ERROR_IX_NO_RESPONSE          3
#define SCANNING_ERROR_MAX                     3

void ProcessMessageAlphanodePOS(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);

class CAlphanodeScanning
{
public:
    void DoAlphanodePOSChecks();
    void CleanAlphanodeScanningErrors();
};

// Returns how many alphanodes are allowed to scan each block
int GetCountScanningPerBlock();

class CAlphanodeScanningError
{
public:
    CTxIn vinAlphanodeA;
    CTxIn vinAlphanodeB;
    int nErrorType;
    int nExpiration;
    int nBlockHeight;
    std::vector<unsigned char> vchAlphaNodeSignature;

    CAlphanodeScanningError ()
    {
        vinAlphanodeA = CTxIn();
        vinAlphanodeB = CTxIn();
        nErrorType = 0;
        nExpiration = GetTime()+(60*60);
        nBlockHeight = 0;
    }

    CAlphanodeScanningError (CTxIn& vinAlphanodeAIn, CTxIn& vinAlphanodeBIn, int nErrorTypeIn, int nBlockHeightIn)
    {
    	vinAlphanodeA = vinAlphanodeAIn;
    	vinAlphanodeB = vinAlphanodeBIn;
    	nErrorType = nErrorTypeIn;
    	nExpiration = GetTime()+(60*60);
    	nBlockHeight = nBlockHeightIn;
    }

    CAlphanodeScanningError (CTxIn& vinAlphanodeBIn, int nErrorTypeIn, int nBlockHeightIn)
    {
        //just used for IX, AlphanodeA is any client
        vinAlphanodeA = CTxIn();
        vinAlphanodeB = vinAlphanodeBIn;
        nErrorType = nErrorTypeIn;
        nExpiration = GetTime()+(60*60);
        nBlockHeight = nBlockHeightIn;
    }

    uint256 GetHash() const {return SerializeHash(*this);}

    bool SignatureValid();
    bool Sign();
    bool IsExpired() {return GetTime() > nExpiration;}
    void Relay();
    bool IsValid() {
    	return (nErrorType > 0 && nErrorType <= SCANNING_ERROR_MAX);
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(vinAlphanodeA);
        READWRITE(vinAlphanodeB);
        READWRITE(nErrorType);
        READWRITE(nExpiration);
        READWRITE(nBlockHeight);
        READWRITE(vchAlphaNodeSignature);
    )
};


#endif
