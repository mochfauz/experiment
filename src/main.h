// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2017-2018 The Folm developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MAIN_H
#define BITCOIN_MAIN_H

#if defined(HAVE_CONFIG_H)
#include "config/folm-config.h"
#endif

#include "amount.h"
#include "chain.h"
#include "chainparams.h"
#include "coins.h"
#include "net.h"
#include "pow.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "script/script.h"
#include "script/sigcache.h"
#include "script/standard.h"
#include "sync.h"
#include "tinyformat.h"
#include "txmempool.h"
#include "uint256.h"
#include "undo.h"
#include "spentindex.h"

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include <boost/unordered_map.hpp>

class CBlockIndex;
class CBlockTreeDB;
class CBloomFilter;
class CInv;
class CScriptCheck;
class CValidationInterface;
class CValidationState;

struct CBlockTemplate;
struct CNodeStateStats;

static const bool DEFAULT_ADDRESSINDEX = false;
static const bool DEFAULT_TIMESTAMPINDEX = false;
static const bool DEFAULT_SPENTINDEX = false;


struct BlockHasher {
    size_t operator()(const uint256& hash) const { return hash.GetLow64(); }
};

extern CScript COINBASE_FLAGS;
extern CCriticalSection cs_main;
extern CTxMemPool mempool;
typedef boost::unordered_map<uint256, CBlockIndex*, BlockHasher> BlockMap;
extern BlockMap mapBlockIndex;
extern uint64_t nLastBlockTx;
extern uint64_t nLastBlockSize;
extern const std::string strMessageMagic;
extern int64_t nTimeBestReceived;
extern CWaitableCriticalSection csBestBlock;
extern CConditionVariable cvBlockChange;
extern bool fImporting;
extern bool fReindex;
extern int nScriptCheckThreads;
extern bool fTxIndex;
extern bool fIsBareMultisigStd;
extern bool fCheckBlockIndex;
extern unsigned int nCoinCacheSize;
extern CFeeRate minRelayTxFee;
extern bool fAlerts;

extern bool fLargeWorkForkFound;
extern bool fLargeWorkInvalidChainFound;

extern unsigned int nStakeMinAge;
extern int64_t nLastCoinStakeSearchInterval;
extern int64_t nLastCoinStakeSearchTime;
extern int64_t nReserveBalance;

extern std::map<uint256, int64_t> mapRejectedBlocks;
extern std::map<unsigned int, unsigned int> mapHashedBlocks;
extern std::set<std::pair<COutPoint, unsigned int> > setStakeSeen;

/** Best header we've seen so far (used for getheaders queries' starting points). */
extern CBlockIndex* pindexBestHeader;

/** Minimum disk space required - used in CheckDiskSpace() */
static const uint64_t nMinDiskSpace = 52428800;

/** Register a wallet to receive updates from core */
void RegisterValidationInterface(CValidationInterface* pwalletIn);
/** Unregister a wallet from core */
void UnregisterValidationInterface(CValidationInterface* pwalletIn);
/** Unregister all wallets from core */
void UnregisterAllValidationInterfaces();
/** Push an updated transaction to all registered wallets */
void SyncWithWallets(const CTransaction& tx, const CBlock* pblock = NULL);

/** Register with a network node to receive its signals */
void RegisterNodeSignals(CNodeSignals& nodeSignals);
/** Unregister a network node */
void UnregisterNodeSignals(CNodeSignals& nodeSignals);

/** 
 * Process an incoming block. This only returns after the best known valid
 * block is made active. Note that it does not, however, guarantee that the
 * specific block passed to it has been checked for validity!
 * 
 * @param[out]  state   This may be set to an Error state if any error occurred processing it, including during validation/connection/etc of otherwise unrelated blocks during reorganisation; or it may be set to an Invalid state if pblock is itself invalid (but this is not guaranteed even when the block is checked). If you want to *possibly* get feedback on whether pblock is valid, you must also install a CValidationInterface - this will have its BlockChecked method called whenever *any* block completes validation.
 * @param[in]   pfrom   The node which we are receiving the block from; it is added to mapBlockSource and may be penalised if the block is invalid.
 * @param[in]   pblock  The block we want to process.
 * @param[out]  dbp     If pblock is stored to disk (or already there), this will be set to its location.
 * @return True if state.IsValid()
 */
bool ProcessNewBlock(CValidationState& state, CNode* pfrom, CBlock* pblock, CDiskBlockPos* dbp = NULL);
/** Check whether enough disk space is available for an incoming block */
bool CheckDiskSpace(uint64_t nAdditionalBytes = 0);
/** Open a block file (blk?????.dat) */
FILE* OpenBlockFile(const CDiskBlockPos& pos, bool fReadOnly = false);
/** Open an undo file (rev?????.dat) */
FILE* OpenUndoFile(const CDiskBlockPos& pos, bool fReadOnly = false);
/** Translation to a filesystem path */
boost::filesystem::path GetBlockPosFilename(const CDiskBlockPos& pos, const char* prefix);
/** Import blocks from an external file */
bool LoadExternalBlockFile(FILE* fileIn, CDiskBlockPos* dbp = NULL);
/** Initialize a new block tree database + block data on disk */
bool InitBlockIndex();
/** Load the block tree and coins database from disk */
bool LoadBlockIndex();
/** Unload database information */
void UnloadBlockIndex();
/** See whether the protocol update is enforced for connected nodes */
int ActiveProtocol();
/** Process protocol messages received from a given node */
bool ProcessMessages(CNode* pfrom);
/**
 * Send queued protocol messages to be sent to a give node.
 *
 * @param[in]   pto             The node which we are sending messages to.
 * @param[in]   fSendTrickle    When true send the trickled data, otherwise trickle the data until true.
 */
bool SendMessages(CNode* pto, bool fSendTrickle);
/** Run an instance of the script checking thread */
void ThreadScriptCheck();

// ***TODO*** probably not the right place for these 2
/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits */
bool CheckProofOfWork(uint256 hash, unsigned int nBits);

/** Check whether we are doing an initial block download (synchronizing from disk or network) */
bool IsInitialBlockDownload();
/** Format a string that describes several potential problems detected by the core */
std::string GetWarnings(const std::string& strFor);
/** Retrieve a transaction (from memory pool, or from disk, if possible) */
bool GetTransaction(const uint256& hash, CTransaction& tx, uint256& hashBlock, bool fAllowSlow = false);
/** Find the best known block, and make it the tip of the block chain */

bool DisconnectBlocksAndReprocess(int blocks);

// ***TODO***
double ConvertBitsToDouble(unsigned int nBits);
int64_t GetMasternodePayment(int nHeight, int64_t blockValue, int nMasternodeCount = 0);
unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader* pblock, const Consensus::Params& param);

bool ActivateBestChain(CValidationState& state, CBlock* pblock = NULL);
CAmount GetBlockValue(int nHeight);

/** Create a new block index entry for a given block hash */
CBlockIndex* InsertBlockIndex(uint256 hash);
/** Get statistics from node state */
bool GetNodeStateStats(NodeId nodeid, CNodeStateStats& stats);
/** Increase a node's misbehavior score. */
void Misbehaving(NodeId nodeid, int howmuch);
/** Flush all state, indexes and buffers to disk. */
void FlushStateToDisk();


/** (try to) add transaction to memory pool **/
bool AcceptToMemoryPool(CTxMemPool& pool, CValidationState& state, const CTransaction& tx, bool fLimitFree, bool* pfMissingInputs, bool fRejectInsaneFee = false, bool ignoreFees = false);

bool AcceptableInputs(CTxMemPool& pool, CValidationState& state, const CTransaction& tx, bool fLimitFree, bool* pfMissingInputs, bool fRejectInsaneFee = false, bool isDSTX = false);

int GetInputAge(CTxIn& vin);
int GetInputAgeIX(uint256 nTXHash, CTxIn& vin);
bool GetCoinAge(const CTransaction& tx, unsigned int nTxTime, uint64_t& nCoinAge);
int GetIXConfirmations(uint256 nTXHash);

struct CNodeStateStats {
    int nMisbehavior;
    int nSyncHeight;
    int nCommonHeight;
    std::vector<int> vHeightInFlight;
};

struct CTimestampIndexIteratorKey {
    unsigned int timestamp;

    size_t GetSerializeSize(int nType, int nVersion) const {
        return 4;
    }
    template<typename Stream>
    void Serialize(Stream& s, int nType, int nVersion) const {
        ser_writedata32be(s, timestamp);
    }
    template<typename Stream>
    void Unserialize(Stream& s, int nType, int nVersion) {
        timestamp = ser_readdata32be(s);
    }

    CTimestampIndexIteratorKey(unsigned int time) {
        timestamp = time;
    }

    CTimestampIndexIteratorKey() {
        SetNull();
    }

    void SetNull() {
        timestamp = 0;
    }
};

struct CTimestampIndexKey {
    unsigned int timestamp;
    uint256 blockHash;

    size_t GetSerializeSize(int nType, int nVersion) const {
        return 36;
    }
    template<typename Stream>
    void Serialize(Stream& s, int nType, int nVersion) const {
        ser_writedata32be(s, timestamp);
        blockHash.Serialize(s, nType, nVersion);
    }
    template<typename Stream>
    void Unserialize(Stream& s, int nType, int nVersion) {
        timestamp = ser_readdata32be(s);
        blockHash.Unserialize(s, nType, nVersion);
    }

    CTimestampIndexKey(unsigned int time, uint256 hash) {
        timestamp = time;
        blockHash = hash;
    }

    CTimestampIndexKey() {
        SetNull();
    }

    void SetNull() {
        timestamp = 0;
        blockHash.SetNull();
    }
};

struct CAddressUnspentKey {
    unsigned int type;
    uint160 hashBytes;
    uint256 txhash;
    size_t index;

    size_t GetSerializeSize(int nType, int nVersion) const {
        return 57;
    }
    template<typename Stream>
    void Serialize(Stream& s, int nType, int nVersion) const {
        ser_writedata8(s, type);
        hashBytes.Serialize(s, nType, nVersion);
        txhash.Serialize(s, nType, nVersion);
        ser_writedata32(s, index);
    }
    template<typename Stream>
    void Unserialize(Stream& s, int nType, int nVersion) {
        type = ser_readdata8(s);
        hashBytes.Unserialize(s, nType, nVersion);
        txhash.Unserialize(s, nType, nVersion);
        index = ser_readdata32(s);
    }

    CAddressUnspentKey(unsigned int addressType, uint160 addressHash, uint256 txid, size_t indexValue) {
        type = addressType;
        hashBytes = addressHash;
        txhash = txid;
        index = indexValue;
    }

    CAddressUnspentKey() {
        SetNull();
    }

    void SetNull() {
        type = 0;
        hashBytes.SetNull();
        txhash.SetNull();
        index = 0;
    }
};

struct CAddressUnspentValue {
    CAmount satoshis;
    CScript script;
    int blockHeight;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(satoshis);
        READWRITE(*(CScript*)(&script));
        READWRITE(blockHeight);
    }

    CAddressUnspentValue(CAmount sats, CScript scriptPubKey, int height) {
        satoshis = sats;
        script = scriptPubKey;
        blockHeight = height;
    }

    CAddressUnspentValue() {
        SetNull();
    }

    void SetNull() {
        satoshis = -1;
        script.clear();
        blockHeight = 0;
    }

    bool IsNull() const {
        return (satoshis == -1);
    }
};

struct CAddressIndexKey {
    unsigned int type;
    uint160 hashBytes;
    int blockHeight;
    unsigned int txindex;
    uint256 txhash;
    size_t index;
    bool spending;

    size_t GetSerializeSize(int nType, int nVersion) const {
        return 66;
    }
    template<typename Stream>
    void Serialize(Stream& s, int nType, int nVersion) const {
        ser_writedata8(s, type);
        hashBytes.Serialize(s, nType, nVersion);
        // Heights are stored big-endian for key sorting in LevelDB
        ser_writedata32be(s, blockHeight);
        ser_writedata32be(s, txindex);
        txhash.Serialize(s, nType, nVersion);
        ser_writedata32(s, index);
        char f = spending;
        ser_writedata8(s, f);
    }
    template<typename Stream>
    void Unserialize(Stream& s, int nType, int nVersion) {
        type = ser_readdata8(s);
        hashBytes.Unserialize(s, nType, nVersion);
        blockHeight = ser_readdata32be(s);
        txindex = ser_readdata32be(s);
        txhash.Unserialize(s, nType, nVersion);
        index = ser_readdata32(s);
        char f = ser_readdata8(s);
        spending = f;
    }

    CAddressIndexKey(unsigned int addressType, uint160 addressHash, int height, int blockindex,
                     uint256 txid, size_t indexValue, bool isSpending) {
        type = addressType;
        hashBytes = addressHash;
        blockHeight = height;
        txindex = blockindex;
        txhash = txid;
        index = indexValue;
        spending = isSpending;
    }

    CAddressIndexKey() {
        SetNull();
    }

    void SetNull() {
        type = 0;
        hashBytes.SetNull();
        blockHeight = 0;
        txindex = 0;
        txhash.SetNull();
        index = 0;
        spending = false;
    }

};

struct CAddressIndexIteratorKey {
    unsigned int type;
    uint160 hashBytes;

    size_t GetSerializeSize(int nType, int nVersion) const {
        return 21;
    }
    template<typename Stream>
    void Serialize(Stream& s, int nType, int nVersion) const {
        ser_writedata8(s, type);
        hashBytes.Serialize(s, nType, nVersion);
    }
    template<typename Stream>
    void Unserialize(Stream& s, int nType, int nVersion) {
        type = ser_readdata8(s);
        hashBytes.Unserialize(s, nType, nVersion);
    }

    CAddressIndexIteratorKey(unsigned int addressType, uint160 addressHash) {
        type = addressType;
        hashBytes = addressHash;
    }

    CAddressIndexIteratorKey() {
        SetNull();
    }

    void SetNull() {
        type = 0;
        hashBytes.SetNull();
    }
};

struct CAddressIndexIteratorHeightKey {
    unsigned int type;
    uint160 hashBytes;
    int blockHeight;

    size_t GetSerializeSize(int nType, int nVersion) const {
        return 25;
    }
    template<typename Stream>
    void Serialize(Stream& s, int nType, int nVersion) const {
        ser_writedata8(s, type);
        hashBytes.Serialize(s, nType, nVersion);
        ser_writedata32be(s, blockHeight);
    }
    template<typename Stream>
    void Unserialize(Stream& s, int nType, int nVersion) {
        type = ser_readdata8(s);
        hashBytes.Unserialize(s, nType, nVersion);
        blockHeight = ser_readdata32be(s);
    }

    CAddressIndexIteratorHeightKey(unsigned int addressType, uint160 addressHash, int height) {
        type = addressType;
        hashBytes = addressHash;
        blockHeight = height;
    }

    CAddressIndexIteratorHeightKey() {
        SetNull();
    }

    void SetNull() {
        type = 0;
        hashBytes.SetNull();
        blockHeight = 0;
    }
};

struct CDiskTxPos : public CDiskBlockPos {
    unsigned int nTxOffset; // after header

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(*(CDiskBlockPos*)this);
        READWRITE(VARINT(nTxOffset));
    }

    CDiskTxPos(const CDiskBlockPos& blockIn, unsigned int nTxOffsetIn) : CDiskBlockPos(blockIn.nFile, blockIn.nPos), nTxOffset(nTxOffsetIn)
    {
    }

    CDiskTxPos()
    {
        SetNull();
    }

    void SetNull()
    {
        CDiskBlockPos::SetNull();
        nTxOffset = 0;
    }
};


CAmount GetMinRelayFee(const CTransaction& tx, unsigned int nBytes, bool fAllowFree);
bool MoneyRange(CAmount nValueOut);

/**
 * Check transaction inputs, and make sure any
 * pay-to-script-hash transactions are evaluating IsStandard scripts
 * 
 * Why bother? To avoid denial-of-service attacks; an attacker
 * can submit a standard HASH... OP_EQUAL transaction,
 * which will get accepted into blocks. The redemption
 * script can be anything; an attacker could use a very
 * expensive-to-check-upon-redemption script like:
 *   DUP CHECKSIG DROP ... repeated 100 times... OP_1
 */

/** 
 * Check for standard transaction types
 * @param[in] mapInputs    Map of previous transactions that have outputs we're spending
 * @return True if all inputs (scriptSigs) use only standard transaction forms
 */
bool AreInputsStandard(const CTransaction& tx, const CCoinsViewCache& mapInputs);

/** 
 * Count ECDSA signature operations the old-fashioned (pre-0.6) way
 * @return number of sigops this transaction's outputs will produce when spent
 * @see CTransaction::FetchInputs
 */
unsigned int GetLegacySigOpCount(const CTransaction& tx);

/**
 * Count ECDSA signature operations in pay-to-script-hash inputs.
 * 
 * @param[in] mapInputs Map of previous transactions that have outputs we're spending
 * @return maximum number of sigops required to validate this transaction's inputs
 * @see CTransaction::FetchInputs
 */
unsigned int GetP2SHSigOpCount(const CTransaction& tx, const CCoinsViewCache& mapInputs);


/**
 * Check whether all inputs of this transaction are valid (no double spends, scripts & sigs, amounts)
 * This does not modify the UTXO set. If pvChecks is not NULL, script checks are pushed onto it
 * instead of being performed inline.
 */
bool CheckInputs(const CTransaction& tx, CValidationState& state, const CCoinsViewCache& view, bool fScriptChecks, unsigned int flags, bool cacheStore, std::vector<CScriptCheck>* pvChecks = NULL);

/** Apply the effects of this transaction on the UTXO set represented by view */
void UpdateCoins(const CTransaction& tx, CValidationState& state, CCoinsViewCache& inputs, CTxUndo& txundo, int nHeight);

/** Context-independent validity checks */
bool CheckTransaction(const CTransaction& tx, CValidationState& state);

/**
 * Check if transaction will be final in the next block to be created.
 *
 * Calls IsFinalTx() with current block height and appropriate block time.
 *
 * See consensus/consensus.h for flag definitions.
 */
bool CheckFinalTx(const CTransaction& tx, int flags = -1);

/** Check for standard transaction types
 * @return True if all outputs (scriptPubKeys) use only standard transaction forms
 */
bool IsStandardTx(const CTransaction& tx, std::string& reason);

bool IsFinalTx(const CTransaction& tx, int nBlockHeight = 0, int64_t nBlockTime = 0);

/** Undo information for a CBlock */
class CBlockUndo
{
public:
    std::vector<CTxUndo> vtxundo; // for all but the coinbase

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(vtxundo);
    }

    bool WriteToDisk(CDiskBlockPos& pos, const uint256& hashBlock);
    bool ReadFromDisk(const CDiskBlockPos& pos, const uint256& hashBlock);
};


/** 
 * Closure representing one script verification
 * Note that this stores references to the spending transaction 
 */
class CScriptCheck
{
private:
    CScript scriptPubKey;
    const CTransaction* ptxTo;
    unsigned int nIn;
    unsigned int nFlags;
    bool cacheStore;
    ScriptError error;

public:
    CScriptCheck() : ptxTo(0), nIn(0), nFlags(0), cacheStore(false), error(SCRIPT_ERR_UNKNOWN_ERROR) {}
    CScriptCheck(const CCoins& txFromIn, const CTransaction& txToIn, unsigned int nInIn, unsigned int nFlagsIn, bool cacheIn) : scriptPubKey(txFromIn.vout[txToIn.vin[nInIn].prevout.n].scriptPubKey),
                                                                                                                                ptxTo(&txToIn), nIn(nInIn), nFlags(nFlagsIn), cacheStore(cacheIn), error(SCRIPT_ERR_UNKNOWN_ERROR) {}

    bool operator()();

    void swap(CScriptCheck& check)
    {
        scriptPubKey.swap(check.scriptPubKey);
        std::swap(ptxTo, check.ptxTo);
        std::swap(nIn, check.nIn);
        std::swap(nFlags, check.nFlags);
        std::swap(cacheStore, check.cacheStore);
        std::swap(error, check.error);
    }

    ScriptError GetScriptError() const { return error; }
};

bool GetTimestampIndex(const unsigned int &high, const unsigned int &low, std::vector<uint256> &hashes);
bool GetSpentIndex(CSpentIndexKey &key, CSpentIndexValue &value);
bool GetAddressIndex(uint160 addressHash, int type,
                     std::vector<std::pair<CAddressIndexKey, CAmount> > &addressIndex,
                     int start = 0, int end = 0);
bool GetAddressUnspent(uint160 addressHash, int type,
                       std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> > &unspentOutputs);

/** Functions for disk access for blocks */
bool WriteBlockToDisk(CBlock& block, CDiskBlockPos& pos);
bool ReadBlockFromDisk(CBlock& block, const CDiskBlockPos& pos);
bool ReadBlockFromDisk(CBlock& block, const CBlockIndex* pindex);


/** Functions for validating blocks and updating the block tree */

/** Undo the effects of this block (with given index) on the UTXO set represented by coins.
 *  In case pfClean is provided, operation will try to be tolerant about errors, and *pfClean
 *  will be true if no problems were found. Otherwise, the return value will be false in case
 *  of problems. Note that in any case, coins may be modified. */
bool DisconnectBlock(CBlock& block, CValidationState& state, CBlockIndex* pindex, CCoinsViewCache& coins, bool* pfClean = NULL);

/** Reprocess a number of blocks to try and get on the correct chain again **/
bool DisconnectBlocksAndReprocess(int blocks);

/** Apply the effects of this block (with given index) on the UTXO set represented by coins */
bool ConnectBlock(const CBlock& block, CValidationState& state, CBlockIndex* pindex, CCoinsViewCache& coins, bool fJustCheck = false);

/** Context-independent validity checks */
bool CheckBlockHeader(const CBlockHeader& block, CValidationState& state, bool fCheckPOW = true);
bool CheckBlock(const CBlock& block, CValidationState& state, bool fCheckPOW = true, bool fCheckMerkleRoot = true, bool fCheckSig = true);
bool CheckWork(const CBlock block, CBlockIndex* const pindexPrev);

/** Context-dependent validity checks */
bool ContextualCheckBlockHeader(const CBlockHeader& block, CValidationState& state, CBlockIndex* pindexPrev);
bool ContextualCheckBlock(const CBlock& block, CValidationState& state, CBlockIndex* pindexPrev);

/** Check a block is completely valid from start to finish (only works on top of our current best block, with cs_main held) */
bool TestBlockValidity(CValidationState& state, const CBlock& block, CBlockIndex* pindexPrev, bool fCheckPOW = true, bool fCheckMerkleRoot = true);

/** Store block on disk. If dbp is provided, the file is known to already reside on disk */
bool AcceptBlock(CBlock& block, CValidationState& state, CBlockIndex** pindex, CDiskBlockPos* dbp = NULL);
bool AcceptBlockHeader(const CBlockHeader& block, CValidationState& state, CBlockIndex** ppindex = NULL);


class CBlockFileInfo
{
public:
    unsigned int nBlocks;      //! number of blocks stored in file
    unsigned int nSize;        //! number of used bytes of block file
    unsigned int nUndoSize;    //! number of used bytes in the undo file
    unsigned int nHeightFirst; //! lowest height of block in file
    unsigned int nHeightLast;  //! highest height of block in file
    uint64_t nTimeFirst;       //! earliest time of block in file
    uint64_t nTimeLast;        //! latest time of block in file

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(VARINT(nBlocks));
        READWRITE(VARINT(nSize));
        READWRITE(VARINT(nUndoSize));
        READWRITE(VARINT(nHeightFirst));
        READWRITE(VARINT(nHeightLast));
        READWRITE(VARINT(nTimeFirst));
        READWRITE(VARINT(nTimeLast));
    }

    void SetNull()
    {
        nBlocks = 0;
        nSize = 0;
        nUndoSize = 0;
        nHeightFirst = 0;
        nHeightLast = 0;
        nTimeFirst = 0;
        nTimeLast = 0;
    }

    CBlockFileInfo()
    {
        SetNull();
    }

    std::string ToString() const;

    /** update statistics (does not update nSize) */
    void AddBlock(unsigned int nHeightIn, uint64_t nTimeIn)
    {
        if (nBlocks == 0 || nHeightFirst > nHeightIn)
            nHeightFirst = nHeightIn;
        if (nBlocks == 0 || nTimeFirst > nTimeIn)
            nTimeFirst = nTimeIn;
        nBlocks++;
        if (nHeightIn > nHeightLast)
            nHeightLast = nHeightIn;
        if (nTimeIn > nTimeLast)
            nTimeLast = nTimeIn;
    }
};


/** RAII wrapper for VerifyDB: Verify consistency of the block and coin databases */
class CVerifyDB
{
public:
    CVerifyDB();
    ~CVerifyDB();
    bool VerifyDB(CCoinsView* coinsview, int nCheckLevel, int nCheckDepth);
};

/** Find the last common block between the parameter chain and a locator. */
CBlockIndex* FindForkInGlobalIndex(const CChain& chain, const CBlockLocator& locator);

/** Mark a block as invalid. */
bool InvalidateBlock(CValidationState& state, CBlockIndex* pindex);

/** Remove invalidity status from a block and its descendants. */
bool ReconsiderBlock(CValidationState& state, CBlockIndex* pindex);

/** The currently-connected chain of blocks. */
extern CChain chainActive;

/** Global variable that points to the active CCoinsView (protected by cs_main) */
extern CCoinsViewCache* pcoinsTip;

/** Global variable that points to the active block tree (protected by cs_main) */
extern CBlockTreeDB* pblocktree;

struct CBlockTemplate {
    CBlock block;
    std::vector<CAmount> vTxFees;
    std::vector<int64_t> vTxSigOps;
};

/*
class CValidationInterface
{
protected:
    virtual void SyncTransaction(const CTransaction& tx, const CBlock* pblock){};
    virtual void EraseFromWallet(const uint256& hash){};
    virtual void SetBestChain(const CBlockLocator& locator){};
    virtual bool UpdatedTransaction(const uint256& hash) { return false; };
    virtual void Inventory(const uint256& hash){};
    virtual void ResendWalletTransactions(){};
    virtual void BlockChecked(const CBlock&, const CValidationState&){};
    friend void ::RegisterValidationInterface(CValidationInterface*);
    friend void ::UnregisterValidationInterface(CValidationInterface*);
    friend void ::UnregisterAllValidationInterfaces();
};
*/
#endif // BITCOIN_MAIN_H
