// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef TAO_CORE_H
#define TAO_CORE_H

#include "uint256.h"
#include "serialize.h"
#include "script.h"

#include <stdio.h>

static const int64_t BASE_UTXO_ID = 0x100001;

class CScript;
class CTransaction;

/** An outpoint - a combination of a transaction hash and an index n into its vout */
class COutPoint
{
public:
    uint256 hash;
    unsigned int n;

    COutPoint() { SetNull(); }
    COutPoint(uint256 hashIn, unsigned int nIn) { hash = hashIn; n = nIn; }
    IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
    void SetNull() { hash = 0; n = (unsigned int) -1; }
    bool IsNull() const { return (hash == 0 && n == (unsigned int) -1); }

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
    }

    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
    std::string ToStringShort() const;

};

/** An inpoint - a combination of a transaction and an index n into its vin */
class CInPoint
{
public:
    CTransaction* ptx;
    unsigned int n;

    CInPoint() { SetNull(); }
    CInPoint(CTransaction* ptxIn, unsigned int nIn) { ptx = ptxIn; n = nIn; }
    void SetNull() { ptx = NULL; n = (unsigned int) -1; }
    bool IsNull() const { return (ptx == NULL && n == (unsigned int) -1); }
};

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CTxIn
{
public:
    COutPoint prevout;
    CScript scriptSig;
    CScript prevPubKey;
    unsigned int nSequence;

    CTxIn()
    {
        nSequence = std::numeric_limits<unsigned int>::max();
    }

    explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max());

    explicit CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max());

    IMPLEMENT_SERIALIZE
    (
        READWRITE(prevout);
        READWRITE(scriptSig);
        READWRITE(nSequence);
    )

    bool IsFinal() const
    {
        return (nSequence == std::numeric_limits<unsigned int>::max());
    }

    friend bool operator==(const CTxIn& a, const CTxIn& b)
    {
        return (a.prevout   == b.prevout &&
                a.scriptSig == b.scriptSig &&
                a.nSequence == b.nSequence);
    }

    friend bool operator!=(const CTxIn& a, const CTxIn& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CTxOut
{
public:
    int64_t nValue;
    int64_t nUXTOId;
    int nRounds;
    CScript scriptPubKey;

    CTxOut()
    {
        SetNull();
    }

    CTxOut(int64_t nValueIn, CScript scriptPubKeyIn);

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nValue);
        READWRITE(nUXTOId);
        READWRITE(scriptPubKey);
    )

    void SetNull()
    {
        nValue = -1;
        nRounds = -10; // an initial value, should be no way to get this by calculations
        nUXTOId = BASE_UTXO_ID;
        scriptPubKey.clear();
    }

    bool IsNull() const
    {
        return IsEmpty();
    }

    void SetEmpty()
    {
        nValue = 0;
        nUXTOId = BASE_UTXO_ID;
        scriptPubKey.clear();
    }

    bool IsEmpty() const
    {
        return (nValue == 0 && scriptPubKey.empty());
    }

    uint256 GetHash() const;

    bool IsDust(int64_t MIN_RELAY_TX_FEE) const
    {
        /* Tao Notes
        This code is left here in order to be 
        instructive.

        The reason for the prevention of dust level transaction values is 
        to prevent "spam" transactions from filling blocks.  Obviously if
        vout's are obscured then it's impossible to discern what is or is 
        not a dust transaction.

        Dust is an issue for Bitcoin because it fills blocks too quickly. 
        Dust prevention is what prevents Bitcoin from being an on-chain
        micropayment channel.

        Remove the block size limits with adaptive block sizes as has
        been done in Tao and dust is no longer an issue. The network
        also becomes slightly more suited for a micropayment channel.  As such, 
        this function simply returns false.

        While the logic of not paying 0.00001 TAO transaction fee for a 
        0.00000500TAO transaction may not seem obvious, the Tao developers
        do not consider it their place to burden the network with such an
        arbitrary limitation as to prevent it. 
        */
        return false;
    }

    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.nRounds      == b.nRounds &&
                a.nUXTOId    ==  b.nUXTOId &&
                a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const CTxOut& a, const CTxOut& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

#endif
