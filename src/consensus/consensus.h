// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_CONSENSUS_H
#define BITCOIN_CONSENSUS_CONSENSUS_H

#include <cstdlib>
#include <stdint.h>

/** The maximum allowed size for a serialized block, in bytes (only for buffer size limits) */
static const unsigned int MAX_BLOCK_SERIALIZED_SIZE = 4000000;
/** The maximum allowed weight for a block, see BIP 141 (network rule) */
static const unsigned int MAX_BLOCK_WEIGHT = 4000000;
/** The maximum allowed number of signature check operations in a block (network rule) */
static const int64_t MAX_BLOCK_SIGOPS_COST = 80000;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;

static const int WITNESS_SCALE_FACTOR = 4;

/** LiteDoge: proof-of-work ended at this height, every later block is proof-of-stake */
static const int LAST_POW_BLOCK = 8000;
/** LiteDoge: proof-of-stake block (and coinstake) timestamps are multiples of 16 seconds */
static const unsigned int STAKE_TIMESTAMP_MASK = 15;
/** LiteDoge: a block may be at most this many seconds ahead of the network time,
 *  and of its coinbase's timestamp */
static const int64_t LDOGE_FUTURE_DRIFT = 15;
/** LiteDoge: the largest valid serialized block or transaction, in bytes */
static const unsigned int LDOGE_MAX_BLOCK_SIZE = 1000000;

/** LiteDoge: whether a block is proof-of-stake, judged from its header alone.
 *  Every block after LAST_POW_BLOCK is proof-of-stake; before that, staked
 *  blocks have a zero nonce. (Old peers send no PoS flag with headers.) The
 *  full block is checked against this when it arrives. */
inline bool IsProofOfStakeHeader(uint32_t nNonce, int nHeight)
{
    return nHeight > LAST_POW_BLOCK || nNonce == 0;
}

static const size_t MIN_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 60; // 60 is the lower bound for the size of a valid serialized CTransaction
static const size_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 10; // 10 is the lower bound for the size of a serialized CTransaction

/** Flags for nSequence and nLockTime locks */
/** Interpret sequence numbers as relative lock-time constraints. */
static constexpr unsigned int LOCKTIME_VERIFY_SEQUENCE = (1 << 0);

#endif // BITCOIN_CONSENSUS_CONSENSUS_H
