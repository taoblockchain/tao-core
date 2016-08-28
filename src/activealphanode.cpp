// Copyright (c) 2009-2012 The Darkcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "protocol.h"
#include "activealphanode.h"
#include "alphanodeman.h"
#include <boost/lexical_cast.hpp>
#include "clientversion.h"

//
// Bootup the alphanode, look for a 500 TAO input and register on the network
//
void CActiveAlphanode::ManageStatus()
{
    std::string errorMessage;

    if (fDebug) LogPrintf("CActiveAlphanode::ManageStatus() - Begin\n");

    if(!fAlphaNode) return;

    //need correct adjusted time to send ping
    bool fIsInitialDownload = IsInitialBlockDownload();
    if(fIsInitialDownload) {
        status = ALPHANODE_SYNC_IN_PROCESS;
        LogPrintf("CActiveAlphanode::ManageStatus() - Sync in progress. Must wait until sync is complete to start alphanode.\n");
        return;
    }

    if(status == ALPHANODE_INPUT_TOO_NEW || status == ALPHANODE_NOT_CAPABLE || status == ALPHANODE_SYNC_IN_PROCESS){
        status = ALPHANODE_NOT_PROCESSED;
    }

    if(status == ALPHANODE_NOT_PROCESSED) {
        if(strAlphaNodeAddr.empty()) {
            if(!GetLocal(service)) {
                notCapableReason = "Can't detect external address. Please use the alphanodeaddr configuration option.";
                status = ALPHANODE_NOT_CAPABLE;
                LogPrintf("CActiveAlphanode::ManageStatus() - not capable: %s\n", notCapableReason.c_str());
                return;
            }
        } else {
        	service = CService(strAlphaNodeAddr, true);
        }

        LogPrintf("CActiveAlphanode::ManageStatus() - Checking inbound connection to '%s'\n", service.ToString().c_str());

                  
            if(!ConnectNode((CAddress)service, service.ToString().c_str())){
                notCapableReason = "Could not connect to " + service.ToString();
                status = ALPHANODE_NOT_CAPABLE;
                LogPrintf("CActiveAlphanode::ManageStatus() - not capable: %s\n", notCapableReason.c_str());
                return;
            }
        

        if(pwalletMain->IsLocked()){
            notCapableReason = "Wallet is locked.";
            status = ALPHANODE_NOT_CAPABLE;
            LogPrintf("CActiveAlphanode::ManageStatus() - not capable: %s\n", notCapableReason.c_str());
            return;
        }

        // Set defaults
        status = ALPHANODE_NOT_CAPABLE;
        notCapableReason = "Unknown. Check debug.log for more information.\n";

        // Choose coins to use
        CPubKey pubKeyCollateralAddress;
        CKey keyCollateralAddress;

        if(GetAlphaNodeVin(vin, pubKeyCollateralAddress, keyCollateralAddress)) {

            if(GetInputAge(vin) < ALPHANODE_MIN_CONFIRMATIONS){
                notCapableReason = "Input must have least " + boost::lexical_cast<string>(ALPHANODE_MIN_CONFIRMATIONS) +
                        " confirmations - " + boost::lexical_cast<string>(GetInputAge(vin)) + " confirmations";
                LogPrintf("CActiveAlphanode::ManageStatus() - %s\n", notCapableReason.c_str());
                status = ALPHANODE_INPUT_TOO_NEW;
                return;
            }

            LogPrintf("CActiveAlphanode::ManageStatus() - Is capable master node!\n");

            status = ALPHANODE_IS_CAPABLE;
            notCapableReason = "";

            pwalletMain->LockCoin(vin.prevout);

            // send to all nodes
            CPubKey pubKeyAlphanode;
            CKey keyAlphanode;

            if(!darkSendSigner.SetKey(strAlphaNodePrivKey, errorMessage, keyAlphanode, pubKeyAlphanode))
            {
            	LogPrintf("ActiveAlphanode::Dseep() - Error upon calling SetKey: %s\n", errorMessage.c_str());
            	return;
            }

            /* donations are not supported in tao.conf */
            CScript donationAddress = CScript();
            int donationPercentage = 0;

            if(!Register(vin, service, keyCollateralAddress, pubKeyCollateralAddress, keyAlphanode, pubKeyAlphanode, donationAddress, donationPercentage, errorMessage)) {
                LogPrintf("CActiveAlphanode::ManageStatus() - Error on Register: %s\n", errorMessage.c_str());
            }

            return;
        } else {
            notCapableReason = "Could not find suitable coins!";
        	LogPrintf("CActiveAlphanode::ManageStatus() - Could not find suitable coins!\n");
        }
    }

    //send to all peers
    if(!Dseep(errorMessage)) {
    	LogPrintf("CActiveAlphanode::ManageStatus() - Error on Ping: %s\n", errorMessage.c_str());
    }
}

// Send stop dseep to network for remote alphanode
bool CActiveAlphanode::StopAlphaNode(std::string strService, std::string strKeyAlphanode, std::string& errorMessage) {
	CTxIn vin;
    CKey keyAlphanode;
    CPubKey pubKeyAlphanode;

    if(!darkSendSigner.SetKey(strKeyAlphanode, errorMessage, keyAlphanode, pubKeyAlphanode)) {
    	LogPrintf("CActiveAlphanode::StopAlphaNode() - Error: %s\n", errorMessage.c_str());
		return false;
	}

	return StopAlphaNode(vin, CService(strService, true), keyAlphanode, pubKeyAlphanode, errorMessage);
}

// Send stop dseep to network for main alphanode
bool CActiveAlphanode::StopAlphaNode(std::string& errorMessage) {
	if(status != ALPHANODE_IS_CAPABLE && status != ALPHANODE_REMOTELY_ENABLED) {
		errorMessage = "alphanode is not in a running status";
    	LogPrintf("CActiveAlphanode::StopAlphaNode() - Error: %s\n", errorMessage.c_str());
		return false;
	}

	status = ALPHANODE_STOPPED;

    CPubKey pubKeyAlphanode;
    CKey keyAlphanode;

    if(!darkSendSigner.SetKey(strAlphaNodePrivKey, errorMessage, keyAlphanode, pubKeyAlphanode))
    {
    	LogPrintf("Register::ManageStatus() - Error upon calling SetKey: %s\n", errorMessage.c_str());
    	return false;
    }

	return StopAlphaNode(vin, service, keyAlphanode, pubKeyAlphanode, errorMessage);
}

// Send stop dseep to network for any alphanode
bool CActiveAlphanode::StopAlphaNode(CTxIn vin, CService service, CKey keyAlphanode, CPubKey pubKeyAlphanode, std::string& errorMessage) {
   	pwalletMain->UnlockCoin(vin.prevout);
	return Dseep(vin, service, keyAlphanode, pubKeyAlphanode, errorMessage, true);
}

bool CActiveAlphanode::Dseep(std::string& errorMessage) {
	if(status != ALPHANODE_IS_CAPABLE && status != ALPHANODE_REMOTELY_ENABLED) {
		errorMessage = "alphanode is not in a running status";
    	LogPrintf("CActiveAlphanode::Dseep() - Error: %s\n", errorMessage.c_str());
		return false;
	}

    CPubKey pubKeyAlphanode;
    CKey keyAlphanode;

    if(!darkSendSigner.SetKey(strAlphaNodePrivKey, errorMessage, keyAlphanode, pubKeyAlphanode))
    {
    	LogPrintf("CActiveAlphanode::Dseep() - Error upon calling SetKey: %s\n", errorMessage.c_str());
    	return false;
    }

	return Dseep(vin, service, keyAlphanode, pubKeyAlphanode, errorMessage, false);
}

bool CActiveAlphanode::Dseep(CTxIn vin, CService service, CKey keyAlphanode, CPubKey pubKeyAlphanode, std::string &retErrorMessage, bool stop) {
    std::string errorMessage;
    std::vector<unsigned char> vchAlphaNodeSignature;
    std::string strAlphaNodeSignMessage;
    int64_t masterNodeSignatureTime = GetAdjustedTime();

    std::string strMessage = service.ToString() + boost::lexical_cast<std::string>(masterNodeSignatureTime) + boost::lexical_cast<std::string>(stop);

    if(!darkSendSigner.SignMessage(strMessage, errorMessage, vchAlphaNodeSignature, keyAlphanode)) {
    	retErrorMessage = "sign message failed: " + errorMessage;
    	LogPrintf("CActiveAlphanode::Dseep() - Error: %s\n", retErrorMessage.c_str());
        return false;
    }

    if(!darkSendSigner.VerifyMessage(pubKeyAlphanode, vchAlphaNodeSignature, strMessage, errorMessage)) {
    	retErrorMessage = "Verify message failed: " + errorMessage;
    	LogPrintf("CActiveAlphanode::Dseep() - Error: %s\n", retErrorMessage.c_str());
        return false;
    }

    // Update Last Seen timestamp in alphanode list
    CAlphanode* pmn = mnodeman.Find(vin);
    if(pmn != NULL)
    {
        if(stop)
            mnodeman.Remove(pmn->vin);
        else
            pmn->UpdateLastSeen();
    } else {
    	// Seems like we are trying to send a ping while the alphanode is not registered in the network
    	retErrorMessage = "AlphaSend Alpha Node List doesn't include our alphanode, Shutting down alphanode pinging service! " + vin.ToString();
    	LogPrintf("CActiveAlphanode::Dseep() - Error: %s\n", retErrorMessage.c_str());
        status = ALPHANODE_NOT_CAPABLE;
        notCapableReason = retErrorMessage;
        return false;
    }

    //send to all peers
    LogPrintf("CActiveAlphanode::Dseep() - RelayAlphanodeEntryPing vin = %s\n", vin.ToString().c_str());
    mnodeman.RelayAlphanodeEntryPing(vin, vchAlphaNodeSignature, masterNodeSignatureTime, stop);

    return true;
}

bool CActiveAlphanode::Register(std::string strService, std::string strKeyAlphanode, std::string txHash, std::string strOutputIndex, std::string strDonationAddress, std::string strDonationPercentage, std::string& errorMessage) {
    CTxIn vin;
    CPubKey pubKeyCollateralAddress;
    CKey keyCollateralAddress;
    CPubKey pubKeyAlphanode;
    CKey keyAlphanode;
    CScript donationAddress = CScript();
    int donationPercentage = 0;

    if(!darkSendSigner.SetKey(strKeyAlphanode, errorMessage, keyAlphanode, pubKeyAlphanode))
    {
        LogPrintf("CActiveAlphanode::Register() - Error upon calling SetKey: %s\n", errorMessage.c_str());
        return false;
    }

    if(!GetAlphaNodeVin(vin, pubKeyCollateralAddress, keyCollateralAddress, txHash, strOutputIndex)) {
        errorMessage = "could not allocate vin";
        LogPrintf("CActiveAlphanode::Register() - Error: %s\n", errorMessage.c_str());
        return false;
    }
    CTaoAddress address;
    if (strDonationAddress != "")
    {
        if(!address.SetString(strDonationAddress))
        {
            LogPrintf("ActiveAlphanode::Register - Invalid Donation Address\n");
            return false;
        }
        donationAddress.SetDestination(address.Get());

        try {
            donationPercentage = boost::lexical_cast<int>( strDonationPercentage );
        } catch( boost::bad_lexical_cast const& ) {
            LogPrintf("ActiveAlphanode::Register - Invalid Donation Percentage (Couldn't cast)\n");
            return false;
        }

        if(donationPercentage < 0 || donationPercentage > 100)
        {
            LogPrintf("ActiveAlphanode::Register - Donation Percentage Out Of Range\n");
            return false;
        }
    }

	return Register(vin, CService(strService, true), keyCollateralAddress, pubKeyCollateralAddress, keyAlphanode, pubKeyAlphanode, donationAddress, donationPercentage, errorMessage);
}

bool CActiveAlphanode::Register(CTxIn vin, CService service, CKey keyCollateralAddress, CPubKey pubKeyCollateralAddress, CKey keyAlphanode, CPubKey pubKeyAlphanode, CScript donationAddress, int donationPercentage, std::string &retErrorMessage) {
    std::string errorMessage;
    std::vector<unsigned char> vchAlphaNodeSignature;
    std::string strAlphaNodeSignMessage;
    int64_t masterNodeSignatureTime = GetAdjustedTime();

    std::string vchPubKey(pubKeyCollateralAddress.begin(), pubKeyCollateralAddress.end());
    std::string vchPubKey2(pubKeyAlphanode.begin(), pubKeyAlphanode.end());

    std::string strMessage = service.ToString() + boost::lexical_cast<std::string>(masterNodeSignatureTime) + vchPubKey + vchPubKey2 + boost::lexical_cast<std::string>(PROTOCOL_VERSION) + donationAddress.ToString() + boost::lexical_cast<std::string>(donationPercentage);

    if(!darkSendSigner.SignMessage(strMessage, errorMessage, vchAlphaNodeSignature, keyCollateralAddress)) {
		retErrorMessage = "sign message failed: " + errorMessage;
		LogPrintf("CActiveAlphanode::Register() - Error: %s\n", retErrorMessage.c_str());
		return false;
    }

    if(!darkSendSigner.VerifyMessage(pubKeyCollateralAddress, vchAlphaNodeSignature, strMessage, errorMessage)) {
		retErrorMessage = "Verify message failed: " + errorMessage;
		LogPrintf("CActiveAlphanode::Register() - Error: %s\n", retErrorMessage.c_str());
		return false;
	}

    CAlphanode* pmn = mnodeman.Find(vin);
    if(pmn == NULL)
    {
        LogPrintf("CActiveAlphanode::Register() - Adding to alphanode list service: %s - vin: %s\n", service.ToString().c_str(), vin.ToString().c_str());
        CAlphanode mn(service, vin, pubKeyCollateralAddress, vchAlphaNodeSignature, masterNodeSignatureTime, pubKeyAlphanode, PROTOCOL_VERSION, donationAddress, donationPercentage);
        mn.UpdateLastSeen(masterNodeSignatureTime);
        mnodeman.Add(mn);
    }

    //send to all peers
    LogPrintf("CActiveAlphanode::Register() - RelayElectionEntry vin = %s\n", vin.ToString().c_str());
    mnodeman.RelayAlphanodeEntry(vin, service, vchAlphaNodeSignature, masterNodeSignatureTime, pubKeyCollateralAddress, pubKeyAlphanode, -1, -1, masterNodeSignatureTime, PROTOCOL_VERSION, donationAddress, donationPercentage);

    return true;
}

bool CActiveAlphanode::GetAlphaNodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {
	return GetAlphaNodeVin(vin, pubkey, secretKey, "", "");
}

bool CActiveAlphanode::GetAlphaNodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex) {
    CScript pubScript;

    // Find possible candidates
    vector<COutput> possibleCoins = SelectCoinsAlphanode();
    COutput *selectedOutput;

    // Find the vin
	if(!strTxHash.empty()) {
		// Let's find it
		uint256 txHash(strTxHash);
        int outputIndex = boost::lexical_cast<int>(strOutputIndex);
		bool found = false;
		BOOST_FOREACH(COutput& out, possibleCoins) {
			if(out.tx->GetHash() == txHash && out.i == outputIndex)
			{
				selectedOutput = &out;
				found = true;
				break;
			}
		}
		if(!found) {
			LogPrintf("CActiveAlphanode::GetAlphaNodeVin - Could not locate valid vin\n");
			return false;
		}
	} else {
		// No output specified,  Select the first one
		if(possibleCoins.size() > 0) {
			selectedOutput = &possibleCoins[0];
		} else {
			LogPrintf("CActiveAlphanode::GetAlphaNodeVin - Could not locate specified vin from possible list\n");
			return false;
		}
    }

	// At this point we have a selected output, retrieve the associated info
	return GetVinFromOutput(*selectedOutput, vin, pubkey, secretKey);
}

bool CActiveAlphanode::GetAlphaNodeVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {
	return GetAlphaNodeVinForPubKey(collateralAddress, vin, pubkey, secretKey, "", "");
}

bool CActiveAlphanode::GetAlphaNodeVinForPubKey(std::string collateralAddress, CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex) {
    CScript pubScript;

    // Find possible candidates
    vector<COutput> possibleCoins = SelectCoinsAlphanodeForPubKey(collateralAddress);
    COutput *selectedOutput;

    // Find the vin
	if(!strTxHash.empty()) {
		// Let's find it
		uint256 txHash(strTxHash);
        int outputIndex = boost::lexical_cast<int>(strOutputIndex);
		bool found = false;
		BOOST_FOREACH(COutput& out, possibleCoins) {
			if(out.tx->GetHash() == txHash && out.i == outputIndex)
			{
				selectedOutput = &out;
				found = true;
				break;
			}
		}
		if(!found) {
			LogPrintf("CActiveAlphanode::GetAlphaNodeVinForPubKey - Could not locate valid vin\n");
			return false;
		}
	} else {
		// No output specified,  Select the first one
		if(possibleCoins.size() > 0) {
			selectedOutput = &possibleCoins[0];
		} else {
			LogPrintf("CActiveAlphanode::GetAlphaNodeVinForPubKey - Could not locate specified vin from possible list\n");
			return false;
		}
    }

	// At this point we have a selected output, retrieve the associated info
	return GetVinFromOutput(*selectedOutput, vin, pubkey, secretKey);
}


// Extract alphanode vin information from output
bool CActiveAlphanode::GetVinFromOutput(COutput out, CTxIn& vin, CPubKey& pubkey, CKey& secretKey) {

    CScript pubScript;

	vin = CTxIn(out.tx->GetHash(),out.i);
    pubScript = out.tx->vout[out.i].scriptPubKey; // the inputs PubKey

	CTxDestination address1;
    ExtractDestination(pubScript, address1);
    CTaoAddress address2(address1);

    CKeyID keyID;
    if (!address2.GetKeyID(keyID)) {
        LogPrintf("CActiveAlphanode::GetAlphaNodeVin - Address does not refer to a key\n");
        return false;
    }

    if (!pwalletMain->GetKey(keyID, secretKey)) {
        LogPrintf ("CActiveAlphanode::GetAlphaNodeVin - Private key for address is not known\n");
        return false;
    }

    pubkey = secretKey.GetPubKey();
    return true;
}

// get all possible outputs for running alphanode
vector<COutput> CActiveAlphanode::SelectCoinsAlphanode()
{
    vector<COutput> vCoins;
    vector<COutput> filteredCoins;

    // Retrieve all possible outputs
    pwalletMain->AvailableCoinsMN(vCoins);

    // Filter
    BOOST_FOREACH(const COutput& out, vCoins)
    {
        if(out.tx->vout[out.i].nValue == Params().AlphaNodeCollateral()) { //exactly
        	filteredCoins.push_back(out);
        }
    }
    return filteredCoins;
}

// get all possible outputs for running alphanode for a specific pubkey
vector<COutput> CActiveAlphanode::SelectCoinsAlphanodeForPubKey(std::string collateralAddress)
{
    CTaoAddress address(collateralAddress);
    CScript scriptPubKey;
    scriptPubKey.SetDestination(address.Get());
    vector<COutput> vCoins;
    vector<COutput> filteredCoins;

    // Retrieve all possible outputs
    pwalletMain->AvailableCoins(vCoins);

    // Filter
    BOOST_FOREACH(const COutput& out, vCoins)
    {
        if(out.tx->vout[out.i].scriptPubKey == scriptPubKey && out.tx->vout[out.i].nValue == Params().AlphaNodeCollateral()*COIN) { //exactly
        	filteredCoins.push_back(out);
        }
    }
    return filteredCoins;
}

// when starting a alphanode, this can enable to run as a hot wallet with no funds
bool CActiveAlphanode::EnableHotColdAlphaNode(CTxIn& newVin, CService& newService)
{
    if(!fAlphaNode) return false;

    status = ALPHANODE_REMOTELY_ENABLED;

    //The values below are needed for signing dseep messages going forward
    this->vin = newVin;
    this->service = newService;

    LogPrintf("CActiveAlphanode::EnableHotColdAlphaNode() - Enabled! You may shut down the cold daemon.\n");

    return true;
}
