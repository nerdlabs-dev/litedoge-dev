// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <kernel/chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/amount.h>
#include <consensus/merkle.h>
#include <consensus/params.h>
#include <hash.h>
#include <chainparamsbase.h>
#include <logging.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTimeTx, uint32_t nTimeBlock, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.nTime = nTimeTx;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 0 << CScriptNum(42) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].SetEmpty();

    CBlock genesis;
    genesis.nTime    = nTimeBlock;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTimeTx, uint32_t nTimeBlock, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "plz time stamp. stahp";
    const CScript genesisOutputScript = CScript();
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTimeTx, nTimeBlock, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network on which people trade goods and services.
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = CBaseChainParams::MAIN;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();

        // LiteDoge has required the block height at the start of the coinbase
        // since block 1, and has never had BIP65/BIP66/CSV, segwit or taproot.
        // (The old 3.x wallets would reject blocks or transactions using them.)
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x8340e43b90828268419b18cfd40bbb1082c48d314ccd6f980eb5649b9503a5cc");
        consensus.BIP65Height = std::numeric_limits<int>::max();
        consensus.BIP66Height = std::numeric_limits<int>::max();
        consensus.CSVHeight = std::numeric_limits<int>::max();
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.bnInitialHashTarget = uint256S("0000000000ffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~arith_uint256(0) >> 40;

        consensus.nTargetTimespan = 16 * 60;
        consensus.nStakeTargetSpacing = 64; // LDOGE: 64 second blocks
        consensus.nTargetSpacingWorkMax = 12 * consensus.nStakeTargetSpacing; // 2-hour
        consensus.nPowTargetSpacing = consensus.nStakeTargetSpacing;
        consensus.nStakeMinAge =  8 * 60 * 60;
        consensus.nStakeMaxAge = 60 * 60 * 24 * 90;
        consensus.nModifierInterval = 10 * 60;
        consensus.nCoinbaseMaturity = 500;

        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1815; // 90% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing

        consensus.SegwitHeight = std::numeric_limits<int>::max(); // never

        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        // Signatures in blocks up to this one (height 3,420,000, a checkpoint of
        // the 3.6 wallets) aren't re-checked during the initial sync, as in
        // Bitcoin Core. Stake kernels and everything else still are.
        consensus.defaultAssumeValid = uint256S("0x5b575f84afb8a4c5bc4c49aea5481de045fd2d8c53aad277c9db025b3c61c3c3");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x65;
        pchMessageStart[1] = 0x44;
        pchMessageStart[2] = 0x15;
        pchMessageStart[3] = 0x06;
        nDefaultPort = 17014;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 8;
        m_assumed_chain_state_size = 1;

        genesis = CreateGenesisBlock(1426450258, 1426450258, 925125, 0x1e0fffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0000032101032f27e7cdddb1196353f7fc9e1b6294717432135add95534f67c6"));
        assert(genesis.hashMerkleRoot == uint256S("0x6e8089863e3811437cca6029c8eb113e0ddec4cb553e7cfdf8944c964cf86832"));

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as an addrfetch if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,90);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,8);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,171);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        // human readable prefix to bench32 address
        // LiteDoge has no segwit, so bech32 addresses are never used
        bech32_hrp = "ldoge";

        vSeeds.emplace_back("cloud.litedoge.info");
        vSeeds.emplace_back("seed01.litedogeofficial.org");
        vSeeds.emplace_back("seed02.litedogeofficial.org");
        vSeeds.emplace_back("seed03.litedogeofficial.org");
        vSeeds.emplace_back("seed04.litedogeofficial.org");
        vSeeds.emplace_back("seed05.litedogeofficial.org");
        vSeeds.emplace_back("seed06.litedogeofficial.org");
        vSeeds.emplace_back("seed07.litedogeofficial.org");
        vSeeds.emplace_back("seed08.litedogeofficial.org");
        vSeeds.emplace_back("ldoge.nerdlabs001.com");

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_main), std::end(chainparams_seed_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = false;
        m_is_mockable_chain = false;

        // The hardened checkpoints of the LiteDoge 3.6 wallets
        checkpointData = {
            {
                {        0, uint256S("0x0000032101032f27e7cdddb1196353f7fc9e1b6294717432135add95534f67c6")},
                {        1, uint256S("0x8340e43b90828268419b18cfd40bbb1082c48d314ccd6f980eb5649b9503a5cc")},
                {       10, uint256S("0x51e921bd45e607c9afc4f393aa9376e91dcc4fe410998dc3220b3d9dcc986031")},
                {      100, uint256S("0xaa7114900059c905d6ac4c17c6fb4422d9ff16e241ab14104529b63ac41db8e3")},
                {      500, uint256S("0xb586093f6d74c62e5b31e0cee334aec84cad3c53f69969271c52fb6dd3915911")},
                {      750, uint256S("0x0bca3e379edbabf60dfe7a7c7d7c647cb32e9465f400d71eacbdc24464d56881")},
                {      775, uint256S("0x9e9915c5523bccf938a3fa2790e450d405bcec4f3792a61777762a6040868981")},
                {      800, uint256S("0xffbec1ff3aef421a717a06b3d245e1047fe560ebf41451d2b92d89bde688becb")},
                {      850, uint256S("0xa5f1b9a29c6ea116487b72dc3c14c2c72fc96a9e8d04c5584ec7fd0bb485a923")},
                {      900, uint256S("0x20e37e4f119497735ad20120bc49f1798c8d5bed445b802c36fff6ca31bb2a6e")},
                {      950, uint256S("0xb9bfa30fb8654da75f7d38954419b88d22abf70c665ec2ebe4b0bde93370287e")},
                {      975, uint256S("0x3088fc3b53ba01cfb0c1a22e02260dd2d65205290de49e5e0bac7c369abe58f5")},
                {     1000, uint256S("0x0b8f35d5f8b2193c7d3387ec1eebfcc85542135fda57a9b55fc1689c98be486a")},
                {     1250, uint256S("0x0b8c690d34ed51c8091c42e8457c74ec31b588e4166f4176e7dd156146b2b769")},
                {     2000, uint256S("0x44e0027a7354e256a2ec259591d13a9588d0410c423ae6fc2b067bfc9671408b")},
                {     2500, uint256S("0x03ffd3394a80fa450e11135e2bf4d8faec423efc75b8493a572291b862747e59")},
                {     3000, uint256S("0x5723e6320ff6a874799d2f9bd755dda03c76dc7a1973c3de950c4a801b9a0380")},
                {     3750, uint256S("0xac36f8412d8bdb6f31ed7f25b6814b3d76302b1b3ca9bc9080713e581db9762b")},
                {     4000, uint256S("0x91263d39203077149075e6fb4de83275050fdeeea435144804f44c02c7f9a172")},
                {     5000, uint256S("0x4080a0f099f47402e76d626077b80c0fd87140482d3441e84128122751089b95")},
                {     5500, uint256S("0x6e112af3ddfcf2f5b95c99b422008f7526e047ffd357ff640f3028c77c6ee9b3")},
                {     5750, uint256S("0x240f45d9f06744beba51b1387dbde604be32cbe4cfffb8a386d8e944dcb4895a")},
                {     6000, uint256S("0xe4be6b2121828adc3ba61691d6d27f81fada1f6ce4bab3cb9b485d717bb6a977")},
                {     6025, uint256S("0x331cbe0dba242c203b64080a1cadf6befccdf055613986821c3c1e061f8fa22e")},
                {     6100, uint256S("0x201945534fb013d7ec38da646332ff94acaaffd416be5463c9df35ee80263b6d")},
                {     6250, uint256S("0xb68a442c28a9721725e7c163c8745ffeec9067fcc49381fd28c55c551a37f806")},
                {     6300, uint256S("0xb490a980c52a236cb0e104baa4b275bbb6c3a164b32d385307d36bd9e41ff414")},
                {     6500, uint256S("0xddea84f1319c53a592fc005b7cbb703b6797f60d1817f4e0c4b11af1eeb11e95")},
                {     6700, uint256S("0x3001a024d925fc3803f3b5a52c1e3c59c1bd5cd4aa04e120748b70667d8c89c0")},
                {     6900, uint256S("0x1c8c2edccc1505e979122777befcd0df8c2cbe20ef71ee1e50b30342a93b67fa")},
                {     7000, uint256S("0xcf3813a337cbd0f4f5a83c08c649095a814832418322cc195966012cabe415ff")},
                {     7500, uint256S("0xc6c1a6f6f3e9737b2d850b3ee20b70054b48ec09a03ce3deb427c50ec15a8e80")},
                {     7750, uint256S("0x32a60afd5c6ada2999945ce0ed356e56a6508833de9cbb378a2f7bde6904198c")},
                {     7950, uint256S("0x69d2e1aba30658eb06336c564f53cd9542d04e1be86edee6b7b9f6ff23b8eea3")},
                {     8000, uint256S("0xb3d0df43c60ce93853cbae43061c9c1bea89eddbf4b621ed1880c85fc065051c")},
                {     9000, uint256S("0x3e64239e6fdeb3fbdcc204a10124fbe90c017298491194c45f28f35ed04ab24e")},
                {    10000, uint256S("0x684e3f696c52c1098678af79daeabd1f719f8bbf7450b694ca18d30a3403b5b7")},
                {    20000, uint256S("0xf931e068bbeb87d6d41d9c04b2c2eb4b32413bcbdf4353dff42bb1d12564929e")},
                {    30000, uint256S("0x223a52e9335eb6cbc39157e55721b3f17b50bf3b0b63b01940ed56bcf462a967")},
                {    40000, uint256S("0xd7397dfafe4187dec15d410bf9fd04a97102ebe453f26f36f4a18995617c80d0")},
                {    40500, uint256S("0x673451c7f1500ea26eb73937207d8944105d7083d10babce9456e2db606449fe")},
                {    41000, uint256S("0xd20f7edb22dbb767d24e7c5b11a3eafac0a31b2e76f9e74935139491ef50a532")},
                {    41500, uint256S("0x6398c76f2a53e2c06bd4256efe8ffbede6a80ebe079bf1a72e9f70bc1b15384e")},
                {    42000, uint256S("0x351201b850ba062c7a1f2640817afee21e51f3eed1671edc84ee0c15a8bdef47")},
                {    42250, uint256S("0x4924aca05def717e3261bbb51a14d5f293a966117fbb52e4b39dad7ccefbfe60")},
                {    42500, uint256S("0x8960c690f875495557361810b179440f0bd7bf19212c4eaf5060bceafe1fb8db")},
                {    42700, uint256S("0x3522c34b2e5b91abb7bb683885dd336b9b0cd8571a092845a58025a2f941a7a3")},
                {    42800, uint256S("0x8f1572051e200416e063143a2df1dbd954cdd8c07e3c18a52975ee983e967d14")},
                {    42900, uint256S("0x916ef8572a77de294ca84581df1ac925be04b0f63129af71b79330266919d785")},
                {    42950, uint256S("0x870218ddb84c0d4b58abe6fe3756dcb9086084cfb12cd452ae09c62744a04e64")},
                {    43000, uint256S("0xde47c9a45480f99e8f5542f487873fd8fd8091eafa74deb7faa8a5705b70cd17")},
                {    45000, uint256S("0x776c8dedce6ff8f9c2d13f8c94cf8015a2b85ca16ed60c9880a6f21e067d0e2a")},
                {    46000, uint256S("0xd0cfa488b2cd227aa539905d01f2e065155f10bf9bd498f6f6378f0c5099e8ae")},
                {    47000, uint256S("0xa963e6a461d0c7171c1e92883cdcef44b95f5611a29dd94d9cc403b3ce20685d")},
                {    48000, uint256S("0x370cd2e8549d3d663ed3632464422a8c9549ca37c39c209d725a7f69796f7272")},
                {    49000, uint256S("0x40aebc3ccfa296109e6a8037ca729888003b32bfd3850a1cfe3fd1f5f3b938eb")},
                {    49100, uint256S("0xa07ecbdb8b6fa00a54cbfd77b76e47cc08c4688e15f512b255c22f0ad3bf79b8")},
                {    49200, uint256S("0x89d9baced62e5cf233f9a1a6db45c31cf23f627bac16a893c75a4b832f30569b")},
                {    49500, uint256S("0xb8f44eacf603f3ed488b519177841e5fddd5cc25d2b9dbff4b9874599914987c")},
                {    49750, uint256S("0x09deae291124815ac6dbeb4ca5769c668ee6cd7cc39b53bb332371d395a53e5e")},
                {    50000, uint256S("0xe2518a90420c32e88445952b694821b7eeafde7dd8f5162d8483fc66d3b7983f")},
                {    50500, uint256S("0x3cf556bea282b78ef76bd4fd1a855a6e865de1a02d519c330b4e14bcc3e47254")},
                {    51000, uint256S("0x46cff8875c28e4e55235d510498c068779d1865a6d533406664604b235a8ade3")},
                {    51001, uint256S("0xd68a3a22b66476f1deaceadf93afba76ad1783dc68fd97312cb9791a7ccd7781")},
                {    51100, uint256S("0xc5bf8e58b7e6e7db37b6dccc1f412e5582bf6eb4533e5fc51230a1f4be3efb5a")},
                {    52000, uint256S("0xb3fbca721a9e57095341b1d6d54381e24f88cd56993badde3539889cd1940b20")},
                {    52500, uint256S("0x7f8702c2e9e26569b645fa10b2d7925a580ee39576f6451d0b7228e7aa6ee114")},
                {    55000, uint256S("0xdb196a2fc3cfead392608e9f133dca6217173fa1c1fabcf1b20f413d60bb930c")},
                {    60000, uint256S("0x71d24e1a35618d912ae014913f7e8f0eb0cb9b4f9dd279452c1e569f48386fec")},
                {    62500, uint256S("0x96e8cab3a6702fcb5803fb58605ae988063fc11410cd6fc4a9216dac137ca22c")},
                {    70000, uint256S("0xe14279d5fe5429fd1304c323119eece4278151449afd2e7fa2621de63964e2ab")},
                {    90000, uint256S("0x97c1e8226b7ee719c54ea702453f24cbb9b39e88a5e599941d77acc836784cdc")},
                {   100000, uint256S("0x66c3dc5287b869c16509347d5962b3adfc9e52823a0a965e8076fc261ebfd9ef")},
                {   110000, uint256S("0xef1fa40fbd008a0045fdf09a0c4afef2d2730cffa56c4f40cd16cb426d3f58bf")},
                {   120000, uint256S("0x8739e3c8e29a939b03d051f7f3a3f9f6cb4254967efb5f6d77ab15a387e3a831")},
                {   130000, uint256S("0x270640ca3fab9af25014892772cc6f471680aba26c440de540d2b9a2b5ac09ed")},
                {   200000, uint256S("0xe517d2b6f40d92f9450afecb435ebb70edd4f8a188ecc0117b08602d9320bd75")},
                {   300000, uint256S("0x1d589c00e8bea984edd81d1c985acad28af6f513f71c4303d8fedecb9cc9f965")},
                {   400000, uint256S("0x1557b8443a9de9ef1cbb914f3e7fb3891b2160d3ea18dbd1f46d1756548570b5")},
                {   500000, uint256S("0x5b5f21b3ff0d909bc130a2389955975cc6e58bc676ade61673434fe8d930f3c8")},
                {   600000, uint256S("0x3bdf1f7d71cb95cebb66dda6bc860fe4de9580a0683adb3f84b560b139054197")},
                {   700000, uint256S("0x16c6932214d30322de1539f419996576b1c6d1986585d247080787f165a5bf35")},
                {   800000, uint256S("0xe9a3372cd2b6a5c1fc74818775b8a364de3c8b20db810dd238eac0ab85c1c1d3")},
                {   900000, uint256S("0xc059a2de4bb0136f2c0af90c89f2645a70abca796b718452b92ee97a9e04a36b")},
                {  1000000, uint256S("0x24377a41647b0918f5c44578679c12ae8842dcbc497fd5413da5a898705a7aa3")},
                {  1100000, uint256S("0xd96b9b58d11c9c846564ee115d3e36f91aac0d663d751288c398fd349fe077c9")},
                {  1200000, uint256S("0xd5d0dbec072e43d794bc57e2d88fa003d9916978da4108435f43098a5cd78c50")},
                {  1300000, uint256S("0x5f818bdb6704f5065ffb0d38d5473b09eab5696fd012ef0b290b27ddad75fe09")},
                {  1400000, uint256S("0x3d88b60e2f6bf7d7e78c752d797811d4c1efd7a4211c1457585ec375552371fd")},
                {  1500000, uint256S("0x97385486ca2652ceff14685ad2dc269e8ff5668f826615761e174bd926eb0633")},
                {  1600000, uint256S("0xe70b72565caa8431c4e2e2269b8a3331f707d2eb75169bdaeaf1eb39736b70d2")},
                {  1700000, uint256S("0xcd2441a689f3b6a369dcfd9324f223c4b1eb005d4be7123f986adbc52a8d4796")},
                {  1800000, uint256S("0x02b6d682836b05f53fc85a0eca90d845131ce8985798a74b923d3de4d679bf4c")},
                {  1900000, uint256S("0xc834124386d368c280f81f4f48677affc1cdef64ee3380d85b04b5c1f00131d7")},
                {  2000000, uint256S("0x5fc1a697c6b49a1918322501d9d218c2e68170a517541b2bafdd381bdaa6a7d1")},
                {  2100000, uint256S("0x9d04bfff84c1366483f2577c86b677b3368c3fb11955ec00d8a7cba0c9418fe7")},
                {  2200000, uint256S("0x47b2bf72fb7b8eafa201721fe047bac2b0faabc5aa739df8fb652a94c31d62fe")},
                {  2300000, uint256S("0x0fb119a44047163f2af374398ecd378aed7e40464c92cdb417883bd9d1596b06")},
                {  2400000, uint256S("0xfbe34eb653d4aa77b946b2914ad89d9073d82139fe89056f44ce5ea833c47874")},
                {  2500000, uint256S("0x1aa02240052528ea82fa83055524ea6c2c6b4acf6c24ff219ff80c085afb6a50")},
                {  2600000, uint256S("0x5758da40c83d4417a7b83666ce9b0e0de3aa59172b6bd31b022f63e5f6144a0e")},
                {  2700000, uint256S("0x863a6148b7de6add3f6f28129d83b754ff9e677c6ff1c7c53722553e65b03d81")},
                {  2800000, uint256S("0x63fb321514c94ef2f27e8dc491f59d24f7dfa5a7f4c11685d7cba03116deea82")},
                {  2900000, uint256S("0xedde2f9ef2042c0f2e1d1e41a5d0e808f6a1b00105cbc5391394256b62d6b347")},
                {  3000000, uint256S("0x37dda6d876516d5e10e90051bec6af6d119ea4b5ed15341ceda5da632d7d03db")},
                {  3100000, uint256S("0x9cb444db23b150a8960e40a236bfa84d7ef4baff0ae1e8f325090ec90056ba10")},
                {  3110000, uint256S("0xefa52a06ef737a99d12cc1a56ada8a76755753c4191a130824c79d58a17e6771")},
                {  3120000, uint256S("0xc83a18455827010dd774d0ca7b5d0ad2055a8734658dff2269fea87e5f8feaf7")},
                {  3130000, uint256S("0x1ffa5f4cb56456474ed56c50dbb6633b63ce6e934f27265ff939e72e13d2d1ee")},
                {  3140000, uint256S("0x3e167f869bdebd897f952080ef6e974b7c748710994ec709023c4b5e81206e4d")},
                {  3150000, uint256S("0xa7d7c35d2a2f539d5bdaf2b10b5120e39735cd81cb01bc7a8ba15fab8d0265f2")},
                {  3160000, uint256S("0x1b45c9164bc69a1ad00e13e85cb11311e77fda12e4e5e19ce720e9bc78f2a6f7")},
                {  3170000, uint256S("0x5d4c124e1d79f47e5d73bdf3706384025a49338b503a37b154e9f11be9ec825d")},
                {  3180000, uint256S("0x79add22825f0ea5f2d2a106f0a600b69edc28a6eaed90d4cfde51dfd71052697")},
                {  3190000, uint256S("0x702357d4732d0ca9f82b5a3663086dbe579fb409a1c912667fb5a9164e22ab64")},
                {  3200000, uint256S("0x224dae21a581d34452f78cb5ee1f1fd42ad34a314a907bcf64e2bccaae0b5371")},
                {  3300000, uint256S("0x38dea4213b70234b8b48c2bb7cd9d0fc50905607abce528a71a4fcaa60ad61d4")},
                {  3400000, uint256S("0xb378df6ff0049b9cd1d3170e26dfbb7d2d022d8e60e163aad167b23550fc6f59")},
                {  3410000, uint256S("0x4079f0ddc3403af52f4343d63d0569142f8d2de167ed286d882952527cd3fd51")},
                {  3415000, uint256S("0x0235a965317d8af19834acdb57ddc340591508c68872409cacf19b26e7e317e5")},
                {  3420000, uint256S("0x5b575f84afb8a4c5bc4c49aea5481de045fd2d8c53aad277c9db025b3c61c3c3")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
         // TODO to be specified in a future patch.
        };

        // Rough figures for the sync progress estimate: about two transactions
        // (coinbase + coinstake) per 64 second block, as of about block 3,420,000
        chainTxData = ChainTxData{
            1645000000, // * UNIX timestamp (approximate, Feb 2022)
            7000000,    // * transactions up to then (approximate)
            0.032,      // * transactions per second after that
        };
    }
};

/**
 * Testnet (v3): public test network which is reset from time to time.
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = CBaseChainParams::TESTNET;
        consensus.signet_blocks = false;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = std::numeric_limits<int>::max();
        consensus.BIP66Height = std::numeric_limits<int>::max();
        consensus.CSVHeight = std::numeric_limits<int>::max();
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.bnInitialHashTarget = uint256S("0000000000ffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~arith_uint256(0) >> 40;

        consensus.nTargetTimespan = 7 * 24 * 60 * 60;  // one week
        consensus.nStakeTargetSpacing = 10 * 60;  // 10-minute block spacing
        consensus.nTargetSpacingWorkMax = 12 * consensus.nStakeTargetSpacing; // 2-hour
        consensus.nPowTargetSpacing = consensus.nStakeTargetSpacing;
        consensus.nStakeMinAge = 60 * 60 * 24; // test net min age is 1 day
        consensus.nStakeMaxAge = 60 * 60 * 24 * 90;
        consensus.nModifierInterval = 60 * 20; // Modifier interval: time to elapse before new modifier is computed
        consensus.nCoinbaseMaturity = 60;

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing

        consensus.SegwitHeight = std::numeric_limits<int>::max(); // never

        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        pchMessageStart[0] = 0xbd;
        pchMessageStart[1] = 0xa5;
        pchMessageStart[2] = 0xd3;
        pchMessageStart[3] = 0xa7;
        nDefaultPort = 27099;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 1;

        genesis = CreateGenesisBlock(1426450258, 1426450258, 224610, 0x1f00ffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0000373f620a91549f65fe8c9938910e60a486d72b72376b8fad424c1762cce4"));
        assert(genesis.hashMerkleRoot == uint256S("0x6e8089863e3811437cca6029c8eb113e0ddec4cb553e7cfdf8944c964cf86832"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        // human readable prefix to bench32 address
        bech32_hrp = "tldoge";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        m_is_test_chain = true;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                {     0, genesis.GetHash()                                                         },
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
            // TODO to be specified in a future patch.
        };

        chainTxData = ChainTxData{
            // Data as of block a40f64181ee4a3bedda2eae0107d9da0e049fe285b6e6e2a7f1f11697f22c7ed (height 500000)
            1664282408, // * UNIX timestamp of last known number of transactions
            980877,     // * total number of transactions between genesis and that timestamp

                        //   (the tx=... number in the SetBestChain debug.log lines)
            0.003082068 // * estimated number of transactions per second after that timestamp
                        //   980877/(1664282408-1346029522) = 0.003082068

        };
       consensus.signet_challenge.clear();
    }
};

/**
 * Signet: test network with an additional consensus parameter (see BIP325).
 */
class SigNetParams : public CChainParams {
public:
    explicit SigNetParams(const SigNetOptions& options)
    {
        std::vector<uint8_t> bin;
        vSeeds.clear();

        if (!options.challenge) {
            bin = ParseHex("512103ad5e0edad18cb1f0fc0d28a3d4f1f3e445640337489abb10404f2d1e086be430210359ef5021964fe22d6f8e05b2463c9540ce96883fe3b278760f048f5189f2e6c452ae");
            vSeeds.emplace_back("seed.signet.bitcoin.sprovoost.nl.");

            // Hardcoded nodes can be removed once there are more DNS seeds
            vSeeds.emplace_back("178.128.221.177");
            vSeeds.emplace_back("v7ajjeirttkbnt32wpy3c6w3emwnfr3fkla7hpxcfokr3ysd3kqtzmqd.onion:38333");

            consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000000001899d8142b0");
            consensus.defaultAssumeValid = uint256S("0x0000004429ef154f7e00b4f6b46bfbe2d2678ecd351d95bbfca437ab9a5b84ec"); // 138000
            m_assumed_blockchain_size = 1;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{
                // Data from RPC: getchaintxstats 4096 0000004429ef154f7e00b4f6b46bfbe2d2678ecd351d95bbfca437ab9a5b84ec
                .nTime    = 1681127428,
                .nTxCount = 2226359,
                .dTxRate  = 0.006424463050600656,
            };
        } else {
            bin = *options.challenge;
            consensus.nMinimumChainWork = uint256{};
            consensus.defaultAssumeValid = uint256{};
            m_assumed_blockchain_size = 0;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{
                0,
                0,
                0,
            };
            LogPrintf("Signet with challenge %s\n", HexStr(bin));
        }

        if (options.seeds) {
            vSeeds = *options.seeds;
        }

        strNetworkID = CBaseChainParams::SIGNET;
        consensus.signet_blocks = true;
        consensus.signet_challenge.assign(bin.begin(), bin.end());
        //consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 1;
        consensus.nTargetTimespan = 7 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1815; // 90% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("00000377ae000000000000000000000000000000000000000000000000000000");
/*
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        // Activation of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay
*/
        // message start is defined as the first 4 bytes of the sha256d of the block script
        HashWriter h{};
        h << consensus.signet_challenge;
        uint256 hash = h.GetHash();
        memcpy(pchMessageStart, hash.begin(), 4);

        nDefaultPort = 38333;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1345083810, 1345090000, 122894938, 0x1d0fffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x00000001f757bb737f6596503e17cd17b0658ce630cc727c0cca81aec47c9f06"));
        //assert(genesis.hashMerkleRoot == uint256S("0x3c2d8f85fab4d17aac558cc648a1a58acff0de6deb890c29985690052c5993c2"));

        vFixedSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tb";

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = true;
        m_is_mockable_chain = false;
    }
};

/**
 * Regression test: intended for private networks only. Has minimal difficulty to ensure that
 * blocks can be found instantly.
 */
class CRegTestParams : public CChainParams
{
public:
    explicit CRegTestParams(const RegTestOptions& opts)
    {
        strNetworkID =  CBaseChainParams::REGTEST;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        //consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP34Height = 1; // Always active unless overridden
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1;  // Always active unless overridden
        consensus.BIP66Height = 1;  // Always active unless overridden
        consensus.CSVHeight = 1;    // Always active unless overridden
        consensus.SegwitHeight = 0; // Always active unless overridden
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nTargetTimespan = 7 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{};

        pchMessageStart[0] = 0xf2;
        pchMessageStart[1] = 0xa9;
        pchMessageStart[2] = 0xb7;
        pchMessageStart[3] = 0xca;
        nDefaultPort = 18445;
        nPruneAfterHeight = opts.fastprune ? 100 : 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        for (const auto& [dep, height] : opts.activation_heights) {
            switch (dep) {
            case Consensus::BuriedDeployment::DEPLOYMENT_SEGWIT:
                consensus.SegwitHeight = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_HEIGHTINCB:
                consensus.BIP34Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_DERSIG:
                consensus.BIP66Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_CLTV:
                consensus.BIP65Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_CSV:
                consensus.CSVHeight = int{height};
                break;
            }
        }

        for (const auto& [deployment_pos, version_bits_params] : opts.version_bits_parameters) {
            consensus.vDeployments[deployment_pos].nStartTime = version_bits_params.start_time;
            consensus.vDeployments[deployment_pos].nTimeout = version_bits_params.timeout;
            consensus.vDeployments[deployment_pos].min_activation_height = version_bits_params.min_activation_height;
        }

        genesis = CreateGenesisBlock(1426450258, 1426450258, 1, 0x207fffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x57c1442bbbb865ce8d3ac300e77cc8d584e8267e4670204235f07fdcb9554531"));
        assert(genesis.hashMerkleRoot == uint256S("0x6e8089863e3811437cca6029c8eb113e0ddec4cb553e7cfdf8944c964cf86832"));


        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();
        vSeeds.emplace_back("dummySeed.invalid.");

        fDefaultConsistencyChecks = true;
        fRequireStandard = true;
        m_is_test_chain = true;
        m_is_mockable_chain = true;

        checkpointData = {
            {
                {0, uint256S("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
            {
                110,
                {AssumeutxoHash{uint256S("0x1ebbf5850204c0bdb15bf030f47c7fe91d45c44c712697e4509ba67adb01c618")}, 110},
            },
            {
                200,
                {AssumeutxoHash{uint256S("0x51c8d11d8b5c1de51543c579736e786aa2736206d1e11e627568029ce092cf62")}, 200},
            },
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "bcrt";
    }
};

std::unique_ptr<const CChainParams> CChainParams::SigNet(const SigNetOptions& options)
{
    return std::make_unique<const SigNetParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::RegTest(const RegTestOptions& options)
{
    return std::make_unique<const CRegTestParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::Main()
{
    return std::make_unique<const CMainParams>();
}

std::unique_ptr<const CChainParams> CChainParams::TestNet()
{
    return std::make_unique<const CTestNetParams>();
}
