// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "assert.h"

#include "chainparams.h"
#include "main.h"
#include "util.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

//
// Main network
//

// Convert the pnSeeds array into usable address objects.
static void convertSeeds(std::vector<CAddress> &vSeedsOut, const unsigned int *data, unsigned int count, int port)
{
     // It'll only connect to one or two seed nodes because once it connects,
     // it'll get a pile of addresses with newer timestamps.
     // Seed nodes are given a random 'last seen time' of between one and two
     // weeks ago.
     const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int k = 0; k < count; ++k)
    {
        struct in_addr ip;
        unsigned int i = data[k], t;
        
        // -- convert to big endian
        t =   (i & 0x000000ff) << 24u
            | (i & 0x0000ff00) << 8u
            | (i & 0x00ff0000) >> 8u
            | (i & 0xff000000) >> 24u;
        
        memcpy(&ip, &t, sizeof(ip));
        
        CAddress addr(CService(ip, port));
        addr.nTime = GetTime()-GetRand(nOneWeek)-nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

class CMainParams : public CChainParams {
public:
    CMainParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0x1d;
        pchMessageStart[1] = 0xd1;
        pchMessageStart[2] = 0x1e;
        pchMessageStart[3] = 0xe1;
        vAlertPubKey = ParseHex("045f81956d5826bad7d30daed2b5c8c98e72046c1ec8323da336445476183fb7ca54ba511b8b782bc5085962412e8b9879496e3b60bebee7c36987d1d5848b9a50");
        nDefaultPort = 15150;
        nRPCPort = 15151;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 16);

        // Build the genesis block. Note that the output of the genesis coinbase cannot
        // be spent as it did not originally exist in the database.
        //
        const char* pszEpochStart = "August 24, 2016 - Bitcoin.com - Four Big Banks to Create a New Bitcoin Alternative";
        const int64_t nGenesisTime = 1472101200; // July 4, 2016 00:00:00GMT
        std::vector<CTxIn> vin;
        vin.resize(1);
        vin[0].scriptSig = CScript() << 0 << CBigNum(42) << vector<unsigned char>((const unsigned char*)pszEpochStart, (const unsigned char*)pszEpochStart + strlen(pszEpochStart));
        std::vector<CTxOut> vout;
        vout.resize(1);
        vout[0].SetEmpty();
        CTransaction txNew(1, nGenesisTime, vin, vout, 0);
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = nGenesisTime;
        genesis.nBits    = bnProofOfWorkLimit.GetCompact(); 
        genesis.nNonce   = 194328;

        hashGenesisBlock = genesis.GetHash();
        /*
        CBlock(hash=0000c1c4b036f822bd91dc2006b5575b9c3617903925b8e738803e094cd23f20, ver=1, hashPrevBlock=0000000000000000000000000000000000000000000000000000000000000000, hashMerkleRoot=a2355ab88f8cbe5a8ffc188e71289e73dca06e4de21e51e9a7f34ba258a8d6cf, nTime=1472101200, nBits=1f00ffff, nNonce=194328, vtx=1, vchBlockSig=)
          Coinbase(hash=a2355ab88f8cbe5a8ffc188e71289e73dca06e4de21e51e9a7f34ba258a8d6cf, nTime=1472101200, ver=1, vin.size=1, vout.size=1, nLockTime=0, data.size=0)
            CTxIn(COutPoint(0000000000, 4294967295), coinbase 00012a4c524175677573742032342c2032303136202d20426974636f696e2e636f6d202d20466f7572204269672042616e6b7320746f204372656174652061204e657720426974636f696e20416c7465726e6174697665)
            CTxOut(empty)

          vMerkleTree:  a2355ab88f8cbe5a8ffc188e71289e73dca06e4de21e51e9a7f34ba258a8d6cf
        */
        if (false) {
            uint256 hashTarget = CBigNum().SetCompact(genesis.nBits).getuint256();
            while (genesis.GetHash() > hashTarget)
               {
                   ++genesis.nNonce;
                   if (genesis.nNonce == 0)
                   {
                       printf("NONCE WRAPPED, incrementing time");
                       ++genesis.nTime;
                   }
               }
            printf("%s",genesis.ToString().c_str());
        }


        assert(hashGenesisBlock == uint256("0x0000c1c4b036f822bd91dc2006b5575b9c3617903925b8e738803e094cd23f20"));
        assert(genesis.hashMerkleRoot == uint256("0xa2355ab88f8cbe5a8ffc188e71289e73dca06e4de21e51e9a7f34ba258a8d6cf"));

        
        base58Prefixes[PUBKEY_ADDRESS]  =       std::vector<unsigned char>(1,66);   // 0x42
        base58Prefixes[SCRIPT_ADDRESS]  =       std::vector<unsigned char>(1,3);    // 0x03
        base58Prefixes[SECRET_KEY]      =       std::vector<unsigned char>(1,76);   // 0x4c
        base58Prefixes[ENCAPSULATED_ADDRESS] =  std::vector<unsigned char>(1,40);   // 0x28
        base58Prefixes[BPKA_ADDRESS]    =       std::vector<unsigned char>(1,127);  // 0x7f
        base58Prefixes[EXT_PUBLIC_KEY]  =       list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY]  =       list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        vSeeds.push_back(CDNSSeedData("Seed01",  "taoexplorer.com"));
        vSeeds.push_back(CDNSSeedData("Seed02",  "seed1.tao.network"));
        vSeeds.push_back(CDNSSeedData("Seed03",  "seed2.tao.network"));
        vSeeds.push_back(CDNSSeedData("Seed04",  "seed3.tao.network"));
        vSeeds.push_back(CDNSSeedData("Seed05",  "seed4.tao.network"));
        vSeeds.push_back(CDNSSeedData("Seed06",  "seed5.tao.network"));
        vSeeds.push_back(CDNSSeedData("Seed07",  "seed6.tao.network"));

        convertSeeds(vFixedSeeds, pnSeed, ARRAYLEN(pnSeed), nDefaultPort);
        nSubsidyHalvingInterval = 70080;
        nPoolMaxTransactions = 3;
        //strSporkKey = "045f81956d5826bad7d30daed2b5c8c98e72046c1ec8323da336445476183fb7ca54ba511b8b782bc5085962412e8b9879496e3b60bebee7c36987d1d5848b9a50";
        //strAlphanodePaymentsPubKey = "045f81956d5826bad7d30daed2b5c8c98e72046c1ec8323da336445476183fb7ca54ba511b8b782bc5085962412e8b9879496e3b60bebee7c36987d1d5848b9a50";
        strDarksendPoolDummyAddress = "TcYM6qFTC9i1CHb4GoHTQchF7Z2Qru73gv";
        nLastPOWBlock = 999999999;
        nPOSStartBlock = 0;
        nTargetBlockSpacing = 7.5 * 60;
        nBaseProofOfStakeReward = 4 * CENT;
        nDarkSendPoolMax = (9999.99*COIN);
        nDarkSendCollateral = (0.01*COIN);
        nStakeMinAge = 1 * 60;
        nCoinbaseMaturity = 80;
        nMinStakeConfirmations = 100;   
        nAlphaNodeCollateral = 10000 * COIN;
        nBasePOWReward = 1 * COIN;
        nPOWRewardBlock = 10000;
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::MAIN; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet
//

class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0x2f;
        pchMessageStart[1] = 0xca;
        pchMessageStart[2] = 0x4d;
        pchMessageStart[3] = 0x3e;
        vAlertPubKey = ParseHex("045f81956d5826bad7d30daed2b5c8c98e72046c1ec8323da336445476183fb7ca54ba511b8b782bc5085962412e8b9879496e3b60bebee7c36987d1d5848b9a50");
        nDefaultPort = 16160;
        nRPCPort = 16161;
        strDataDir = "testnet";

        // Modify the testnet genesis block so the timestamp is valid for a later start.
        vFixedSeeds.clear();
        vSeeds.push_back(CDNSSeedData("Seed01",  "testnet.tao.network"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,127);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,130);
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1,138);
        base58Prefixes[ENCAPSULATED_ADDRESS] = std::vector<unsigned char>(1,40);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
//45.76.250.142
        convertSeeds(vFixedSeeds, pnTestnetSeed, ARRAYLEN(pnTestnetSeed), nDefaultPort);

        nLastPOWBlock = 999999999;
        nPOSStartBlock = 5;
        nCoinbaseMaturity = 5;
        nStakeMinAge = 30; 


    }
    virtual Network NetworkID() const { return CChainParams::TESTNET; }
};
static CTestNetParams testNetParams;

//
// Cakenet
//

class CCakeNetParams : public CTestNetParams {
public:
    CCakeNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        nDefaultPort = 18180;
        nRPCPort = 13131;
        strDataDir = "cakenet";
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,96);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,195);
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1,238);
        base58Prefixes[ENCAPSULATED_ADDRESS] = std::vector<unsigned char>(1,39);
    }
    virtual Network NetworkID() const { return CChainParams::CAKENET; }
};
static CCakeNetParams cakeNetParams;

static CChainParams *pCurrentParams = &mainParams;

const CChainParams &Params() {
    return *pCurrentParams;
}

void SelectParams(CChainParams::Network network) {
    switch (network) {
        case CChainParams::MAIN:
            pCurrentParams = &mainParams;
            break;
        case CChainParams::TESTNET:
            pCurrentParams = &testNetParams;
            break;
        case CChainParams::CAKENET:
            pCurrentParams = &cakeNetParams;
            break;
        default:
            assert(false && "Unimplemented network");
            return;
    }
}

bool SelectParamsFromCommandLine() {
    
    bool fTestNet = GetBoolArg("-testnet", false);
    bool fCakeNet = GetBoolArg("-cakenet", false);
    
    if (fTestNet) {
        
        SelectParams(CChainParams::TESTNET);
    } else {
        if (fCakeNet) {
            SelectParams(CChainParams::CAKENET);
        } else {
            SelectParams(CChainParams::MAIN);
        }
    }
    return true;
}
