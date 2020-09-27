//
// Created by mwo on 5/11/15.
//

#include "MicroCore.h"


namespace xmreg
{

/**
 * The constructor is interesting, as
 * m_mempool and m_blockchain_storage depend
 * on each other.
 *
 * So basically m_mempool is initialized with
 * reference to Blockchain (i.e., Blockchain&)
 * and m_blockchain_storage is initialized with
 * reference to m_mempool (i.e., tx_memory_pool&)
 *
 * The same is done in cryptonode::core.
 */
MicroCore::MicroCore():      
        m_mempool(core_storage),
        core_storage(m_mempool),
        m_device {&hw::get_device("default")}
{

}

MicroCore::MicroCore(string _blockchain_path, network_type nettype)
    : MicroCore()
{
    init(_blockchain_path, nettype);
}

/**
 * Initialized the MicroCore object.
 *
 * Create BlockchainLMDB on the heap.
 * Open database files located in blockchain_path.
 * Initialize m_blockchain_storage with the BlockchainLMDB object.
 */
bool
MicroCore::init(const string& _blockchain_path, network_type nt)
{
    blockchain_path = _blockchain_path;

    nettype = nt;

    std::unique_ptr<BlockchainDB> db = std::make_unique<BlockchainLMDB>();

    try
    {
        // try opening lmdb database files
        db->open(blockchain_path,  DBF_RDONLY);
    }
    catch (const std::exception& e)
    {
        cerr << "Error opening database: " << e.what();
        return false;
    }

    // check if the blockchain database
    // is successful opened
    if(!db->is_open())
        return false;

    // initialize Blockchain object to manage
    // the database.
    if (!core_storage.init(db.release(), nettype))
    {
        cerr << "Error opening database: core_storage->init(db, nettype)\n" ;
        return false;
    }

    if (!m_mempool.init())
    {
        cerr << "Error opening database: m_mempool.init()\n" ;
        return false;
    }

    initialization_succeded = true;

    return true;
}

/**
* Get m_blockchain_storage.
* Initialize m_blockchain_storage with the BlockchainLMDB object.
*/
Blockchain const&
MicroCore::get_core() const
{
    return core_storage;
}

tx_memory_pool const&
MicroCore::get_mempool() const
{
    return m_mempool;
}

network_type
MicroCore::get_nettype() const
{
    return nettype;
}

void
MicroCore::get_output_key(uint64_t amount,
               vector<uint64_t> const& absolute_offsets,
               vector<cryptonote::output_data_t>& outputs) 
                const
{
    core_storage.get_db()
            .get_output_key(epee::span<const uint64_t>(&amount, 1),
                            absolute_offsets, outputs);
}

uint64_t
MicroCore::get_num_outputs(uint64_t amount) const
{
    return core_storage.get_db().
            get_num_outputs(amount);
}


output_data_t
MicroCore::get_output_key(uint64_t amount,
               uint64_t global_amount_index) const 
{
    return core_storage.get_db()
                .get_output_key(amount, global_amount_index);
}

bool
MicroCore::get_transactions(
        std::vector<crypto::hash> const& txs_ids,
        std::vector<transaction>& txs,
        std::vector<crypto::hash>& missed_txs) const
{
    return core_storage.get_transactions(txs_ids, txs, missed_txs);
}


std::vector<block>
MicroCore::get_blocks_range(uint64_t h1, uint64_t h2) const
{
    return core_storage.get_db().get_blocks_range(h1, h2);
}

uint64_t
MicroCore::get_tx_unlock_time(crypto::hash const& tx_hash) const
{
    return core_storage.get_db().get_tx_unlock_time(tx_hash);
}

bool
MicroCore::have_tx(crypto::hash const& tx_hash) const
{
    return core_storage.have_tx(tx_hash);
}

bool
MicroCore::tx_exists(crypto::hash const& tx_hash, uint64_t& tx_id) const
{
    return core_storage.get_db().tx_exists(tx_hash, tx_id);
}

tx_out_index
MicroCore::get_output_tx_and_index(uint64_t amount, uint64_t index) const
{
    return core_storage.get_db().get_output_tx_and_index(amount, index);
}

uint64_t
MicroCore::get_tx_block_height(crypto::hash const& tx_hash) const
{
    return core_storage.get_db().get_tx_block_height(tx_hash);
}

uint64_t
MicroCore::get_hard_fork_version(uint64_t height) const
{
    return core_storage.get_hard_fork_version(height);
}

std::vector<uint64_t>
MicroCore::get_tx_amount_output_indices(uint64_t tx_id) const
{
    return core_storage.get_db()
            .get_tx_amount_output_indices(tx_id).front();
}

bool
MicroCore::get_mempool_txs(
        std::vector<tx_info>& tx_infos,
        std::vector<spent_key_image_info>& key_image_infos) const
{

    return m_mempool.get_transactions_and_spent_keys_info(
                tx_infos, key_image_infos, true);
}

bool
MicroCore::get_mempool_txs(
        std::vector<transaction>& txs) const
{
  try
  {
      m_mempool.get_transactions(txs);
      return true;
  }
  catch (std::exception const& e)
  {
      std::cerr << e.what() << std::endl;
      return false;
  }
}

uint64_t
MicroCore::get_current_blockchain_height() const
{
    return core_storage.get_current_blockchain_height();
}

void
MicroCore::get_output_tx_and_index(
        uint64_t amount,
        std::vector<uint64_t> const& offsets,
        std::vector<tx_out_index>& indices) const 
{
    //                           tx_hash     , index in tx
    // tx_out_index is std::pair<crypto::hash, uint64_t>;

    core_storage.get_db().get_output_tx_and_index(
                amount, offsets, indices);
}

bool
MicroCore::get_block_from_height(uint64_t height, block& blk) const
{

    try
    {
        blk = core_storage.get_db().get_block_from_height(height);
    }
    catch (const exception& e)
    {
        cerr << e.what() << endl;
        return false;
    }

    return true;
}


bool
MicroCore::get_outs(COMMAND_RPC_GET_OUTPUTS_BIN::request const& req,
         COMMAND_RPC_GET_OUTPUTS_BIN::response& res) const
{
    return core_storage.get_outs(req, res);
}

uint64_t
MicroCore::get_dynamic_base_fee_estimate(uint64_t grace_blocks) const
{
    return core_storage.get_dynamic_base_fee_estimate(grace_blocks);
}

bool
MicroCore::get_block_complete_entry(block const& b, block_complete_entry& bce)
{
    bce.block = cryptonote::block_to_blob(b);

    for (const auto &tx_hash: b.tx_hashes)
    {
      transaction tx;

      if (!get_tx(tx_hash, tx))
        return false;

      cryptonote::blobdata txblob =  tx_to_blob(tx);

      bce.txs.push_back(txblob);
    }

    return true;
}

bool
MicroCore::get_tx(crypto::hash const& tx_hash, transaction& tx) const
{
    if (core_storage.have_tx(tx_hash))
    {
        // get transaction with given hash
        try
        {
            tx = core_storage.get_db().get_tx(tx_hash);
        }
        catch (TX_DNE const& e)
        {
            try 
            {
                // coinbase txs are not considered pruned
                tx = core_storage.get_db().get_pruned_tx(tx_hash);
                return true;
            }
            catch (TX_DNE const& e)
            {
                cerr << "MicroCore::get_tx: " << e.what() << endl;
            }

            return false;
        }
    }
    else
    {
        cerr << "MicroCore::get_tx tx does not exist in blockchain: " << tx_hash << endl;
        return false;
    }     

    return true;
}

bool
MicroCore::get_output_histogram(
        vector<uint64_t> const& amounts,
        uint64_t min_count,
        histogram_map& histogram,
        bool unlocked,
        uint64_t recent_cutoff) const
{
    try
    {
        histogram = core_storage.get_output_histogram(
                        amounts,
                        unlocked,
                        recent_cutoff,
                        min_count);
    }
    catch (std::exception const& e)
    {
        cerr << e.what() << endl;
        return false;
    }

    return true;
}

bool
MicroCore::get_output_histogram(
        COMMAND_RPC_GET_OUTPUT_HISTOGRAM::request const& req,
        COMMAND_RPC_GET_OUTPUT_HISTOGRAM::response& res) const
{
    // based on bool core_rpc_server::on_get_output_histogram(const ...        

    MicroCore::histogram_map histogram;

    if (!get_output_histogram(req.amounts,
                              req.min_count,
                              histogram,
                              req.unlocked,
                              req.recent_cutoff))
    {
        return false;
    }


    res.histogram.clear();
    res.histogram.reserve(histogram.size());

    for (auto const& i: histogram)
    {
      if (std::get<0>(i.second)
              >= req.min_count
              && (std::get<0>(i.second) <= req.max_count
                  || req.max_count == 0))
        res.histogram.push_back(
                    COMMAND_RPC_GET_OUTPUT_HISTOGRAM::entry(
                        i.first,
                        std::get<0>(i.second),
                        std::get<1>(i.second),
                        std::get<2>(i.second)));
    }

    res.status = CORE_RPC_STATUS_OK;

    return true;
}


hw::device* const
MicroCore::get_device() const
{
    return m_device;
}


bool
MicroCore::decrypt_payment_id(crypto::hash8& payment_id,
                   public_key const& public_key,
                   secret_key const& secret_key) const
{
    return m_device->decrypt_payment_id(payment_id,
                                        public_key,
                                        secret_key);
}


bool
MicroCore::init_success() const
{
    return initialization_succeded;
}


}
