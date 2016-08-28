// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blocksizecalculator.h"
#include "test/test_bitcoin.h"
#include "script/sign.h"
#include "keystore.h"
#include <boost/test/unit_test.hpp>

/** things to test
 * 1. max block size should never be less than 1MB
 * 2. the median block size should be calculated correctly for a few examples
 */
static int nOutputs = 10000;

struct TestChainForComputingMediansSetup : public TestChain100Setup {
    bool GenerateRandomTransaction(CTransaction&);
    void BuildBlocks();
};

bool TestChainForComputingMediansSetup::GenerateRandomTransaction(CTransaction& txNew)
{
    CAmount amountToSend = 5000;
    std::vector<CTransaction> res;

    CKey key;
    key.MakeNewKey(true);
    CScript scriptPubKey = CScript() << ToByteVector(key.GetPubKey())
            << OP_CHECKSIG;

    CBasicKeyStore keystore;
    keystore.AddKeyPubKey(coinbaseKey, coinbaseKey.GetPubKey());

    CTransaction utxo = coinbaseTxns[0];
    coinbaseTxns.erase(coinbaseTxns.begin());

    txNew.nLockTime = chainActive.Height();
    txNew.vin.clear();
    txNew.vout.clear();

    for (int j = 0; j < nOutputs; ++j) {
        CTxOut txout(amountToSend, scriptPubKey);
        txNew.vout.push_back(txout);
    }

    //vin
    CTxIn vin = CTxIn(utxo.GetHash(), 0, CScript(),
            std::numeric_limits<unsigned int>::max() - 1);
    txNew.vin.push_back(vin);

    //previous tx's script pub key that we need to sign
    CScript& scriptSigRes = txNew.vin[0].scriptSig;
    CTransaction txNewConst(txNew);
    ProduceSignature(TransactionSignatureCreator(&keystore, &txNewConst, 0),
            utxo.vout[0].scriptPubKey, scriptSigRes);
    res.push_back(txNew);

    return true;
}

void TestChainForComputingMediansSetup::BuildBlocks()
{
    CScript scriptPubKey = CScript() <<  ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;

    for (int i = 0; i < 10; ++i) { //it takes about a second to validate a large block such as this
        std::vector<CTransaction> txs;
        CTransaction tx;

        if(GenerateRandomTransaction(tx)) {
            txs.push_back(tx);
            CBlock b = CreateAndProcessBlock(txs, scriptPubKey);
            coinbaseTxns.push_back(b.vtx[0]);
            //this needs to be called because the code itself isn't calling it due to the fact that it is not super majority
            BlockSizeCalculator::ComputeBlockSize(chainActive.Tip(), 10);
        }
        nOutputs += 1000;
    }
}

BOOST_FIXTURE_TEST_SUITE(ComputeBlockSizeSmallBlockchain, TestChainForComputingMediansSetup)

BOOST_AUTO_TEST_CASE(ComputeBlockSizeForShortBlockchain)
{
    //1MB because we don't have enough blocks to compute the median
    BOOST_CHECK(BlockSizeCalculator::ComputeBlockSize(chainActive.Tip()) == 1E6);
}

BOOST_AUTO_TEST_CASE(ComputeBlockSizeWithSmallBlocks)
{
    //Testing that we can compute a median over 10 blocks
    unsigned int size = BlockSizeCalculator::ComputeBlockSize(chainActive.Tip(), 10);
    BOOST_CHECK(size == 1E6);
}

BOOST_AUTO_TEST_CASE(ComputeBlockSizeWithEverIncreasingBlockSizes)
{
    //Testing that we can compute a median over 10 blocks
    TestChainForComputingMediansSetup::BuildBlocks();
    unsigned int size = BlockSizeCalculator::ComputeBlockSize(chainActive.Tip(), 10);
    BOOST_CHECK(size == 1276610 || size == 1276612); //the signatures could yield different lengths on different runs
}

BOOST_AUTO_TEST_SUITE_END()
