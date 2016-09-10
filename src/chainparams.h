// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TAO_CHAIN_PARAMS_H
#define TAO_CHAIN_PARAMS_H

#include "bignum.h"
#include "uint256.h"
#include "util.h"

#include <vector>

using namespace std;

#define MESSAGE_START_SIZE 4
typedef unsigned char MessageStartChars[MESSAGE_START_SIZE];

class CAddress;
class CBlock;

struct CDNSSeedData {
    string name, host;
    CDNSSeedData(const string &strName, const string &strHost) : name(strName), host(strHost) {}
};

/**
 * CChainParams defines various tweakable parameters of a given instance of the
 * Bitcoin system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class CChainParams
{
public:
    enum Network {
        MAIN,
        TESTNET,
        REGTEST,
        CAKENET,
        
        MAX_NETWORK_TYPES
    };

    enum Base58Type {
        PUBKEY_ADDRESS,
        SCRIPT_ADDRESS,
        SECRET_KEY,
        ENCAPSULATED_ADDRESS,
        EXT_PUBLIC_KEY,
        EXT_SECRET_KEY,
        BPKA_ADDRESS,
 
        MAX_BASE58_TYPES
    };

    const uint256& HashGenesisBlock() const { return hashGenesisBlock; }
    const MessageStartChars& MessageStart() const { return pchMessageStart; }
    const vector<unsigned char>& AlertKey() const { return vAlertPubKey; }
    int GetDefaultPort() const { return nDefaultPort; }
    const CBigNum& ProofOfWorkLimit() const { return bnProofOfWorkLimit; }
    int SubsidyHalvingInterval() const { return nSubsidyHalvingInterval; }
    virtual const CBlock& GenesisBlock() const = 0;
    virtual bool RequireRPCPassword() const { return true; }
    const string& DataDir() const { return strDataDir; }
    virtual Network NetworkID() const = 0;
    const vector<CDNSSeedData>& DNSSeeds() const { return vSeeds; }
    const std::vector<unsigned char> &Base58Prefix(Base58Type type) const { return base58Prefixes[type]; }
    virtual const vector<CAddress>& FixedSeeds() const = 0;
    int RPCPort() const { return nRPCPort; }
    int LastPOWBlock() const { return nLastPOWBlock; }
    int POSStartBlock() const { return nPOSStartBlock; }
    int PoolMaxTransactions() const { return nPoolMaxTransactions; }
    int TargetBlockSpacing() const { return nTargetBlockSpacing; }
    int64_t DarkSendCollateral() const { return nDarkSendCollateral; }
    int64_t DarkSendPoolMax() const { return nDarkSendPoolMax; }
    int BaseProofOfStakeReward(int nHeight = -1) const {
        if (nHeight >= 0) 
            if (nHeight >= POWRewardBlock()/2){
                return nBaseProofOfStakeReward + (0.5209 * CENT);
            } else {
                return nBaseProofOfStakeReward;
            }
        else
                return nBaseProofOfStakeReward;
        return nBaseProofOfStakeReward; 
    }
    std::string DarksendPoolDummyAddress() const { return strDarksendPoolDummyAddress; }
    int CoinbaseMaturity() const { return nCoinbaseMaturity; }
    int MinStakeConfirmations() const { return nMinStakeConfirmations; }
    int64_t AlphaNodeCollateral() const { return nAlphaNodeCollateral; }
    int BasePOWReward() const { return nBasePOWReward; }
    int POWRewardBlock() const { return nPOWRewardBlock; }
    unsigned int StakeMinAge(int nHeight = -1) const { 
        if (nHeight >= 0) 
            if (nHeight >= POWRewardBlock()/2){
                return nStakeMinAge * 84;
            } else {
                return nStakeMinAge;
            }
        else
                return nStakeMinAge;
    }

    //std::string SporkKey() const { return strSporkKey; }
    //std::string AlphanodePaymentPubKey() const { return strAlphanodePaymentsPubKey; }
protected:
    CChainParams() {};

    uint256 hashGenesisBlock;
    MessageStartChars pchMessageStart;
    // Raw pub key bytes for the broadcast alert signing key.
    vector<unsigned char> vAlertPubKey;
    int nDefaultPort;
    int nRPCPort;
    CBigNum bnProofOfWorkLimit;
    int nSubsidyHalvingInterval;
    string strDataDir;
    vector<CDNSSeedData> vSeeds;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
    int nLastPOWBlock;
    int nPOSStartBlock;
    int64_t nPoolMaxTransactions;
    int64_t nTargetBlockSpacing;
    std::string strDarksendPoolDummyAddress;
    int64_t nDarkSendCollateral;
    int64_t nDarkSendPoolMax;
    int64_t nBaseProofOfStakeReward;
    unsigned int nStakeMinAge;
    int nCoinbaseMaturity;
    int64_t nAlphaNodeCollateral;
    int nBasePOWReward;
    int nMinStakeConfirmations;
    int nPOWRewardBlock;

    //std::string strSporkKey;
    //std::string strAlphanodePaymentsPubKey;
};

/**
 * Return the currently selected parameters. This won't change after app startup
 * outside of the unit tests.
 */
const CChainParams &Params();

/** Sets the params returned by Params() to those for the given network. */
void SelectParams(CChainParams::Network network);

/**
 * Looks for -regtest or -testnet and then calls SelectParams as appropriate.
 * Returns false if an invalid combination is given.
 */
bool SelectParamsFromCommandLine();

inline bool TestNet() {
    // Note: it's deliberate that this returns "false" for regression test mode.
    return Params().NetworkID() == CChainParams::TESTNET;
}

inline bool CakeNet() {
    // Note: it's deliberate that this returns "false" for regression test mode.
    return Params().NetworkID() == CChainParams::CAKENET;
}

#endif
