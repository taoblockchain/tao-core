// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The DarkCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ACTIVEALPHANODE_H
#define ACTIVEALPHANODE_H

#include "uint256.h"
#include "sync.h"
#include "net.h"
#include "key.h"
#include "alphanode.h"
#include "main.h"
#include "init.h"
#include "wallet.h"
#include "darksend.h"

// Responsible for activating the alphanode and pinging the network
class CActiveAlphanode
{
public:
	// Initialized by init.cpp
	// Keys for the main alphanode
	CPubKey pubKeyAlphanode;

	// Initialized while registering alphanode
	CTxIn vin;
    CService service;

    int status;
    std::string notCapableReason;

    CActiveAlphanode()
    {        
        status = ALPHANODE_NOT_PROCESSED;
    }

    void ManageStatus(); // manage status of main alphanode

    bool Dseep(std::string& errorMessage); // ping for main alphanode
    bool Dseep(CTxIn vin, CService service, CKey key, CPubKey pubKey, std::string &retErrorMessage, bool stop); // ping for any alphanode

    bool StopAlphaNode(std::string& errorMessage); // stop main alphanode
    bool StopAlphaNode(std::string strService, std::string strKeyAlphanode, std::string& errorMessage); // stop remote alphanode
    bool StopAlphaNode(CTxIn vin, CService service, CKey key, CPubKey pubKey, std::string& errorMessage); // stop any alphanode

    /// Register remote Alpha Node
    bool Register(std::string strService, std::string strKey, std::string txHash, std::string strOutputIndex, std::string strDonationAddress, std::string strDonationPercentage, std::string& errorMessage); 
    /// Register any Alpha Node
    bool Register(CTxIn vin, CService service, CKey key, CPubKey pubKey, CKey keyAlphanode, CPubKey pubKeyAlphanode, CScript donationAddress, int donationPercentage, std::string &retErrorMessage);  

    // get 10000TX input that can be used for the alphanode
    bool GetAlphaNodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey);
    bool GetAlphaNodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex);
    bool GetAlphaNodeVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey);
    bool GetAlphaNodeVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex);
    vector<COutput> SelectCoinsAlphanode();
    vector<COutput> SelectCoinsAlphanodeForPubKey(std::string collateralAddress);
    bool GetVinFromOutput(COutput out, CTxIn& vin, CPubKey& pubkey, CKey& secretKey);

    // enable hot wallet mode (run a alphanode with no funds)
    bool EnableHotColdAlphaNode(CTxIn& vin, CService& addr);
};

#endif
