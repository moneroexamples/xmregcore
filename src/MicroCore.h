//
// Created by mwo on 5/11/15.
//

#ifndef XMREG01_MICROCORE_H
#define XMREG01_MICROCORE_H

#include "monero_headers.h"

namespace xmreg
{
using namespace cryptonote;
using namespace crypto;
using namespace std;


class AbstractCore 
{
    public:

    // the three methods below are used in input identification
    // in the UniveralIdentifier. Thus we are going to make
    // the identifier relay on the AbstractCore, instead of the
    // concrete implementation defined below.     
    
    virtual uint64_t
    get_num_outputs(uint64_t amount) const = 0;

    virtual void 
    get_output_key(uint64_t amount,
                   vector<uint64_t> const& absolute_offsets,
                   vector<cryptonote::output_data_t>& outputs) const = 0;

    
    virtual void
    get_output_tx_and_index(
            uint64_t amount,
            std::vector<uint64_t> const& offsets,
            std::vector<tx_out_index>& indices) const = 0;


    virtual bool
    get_tx(crypto::hash const& tx_hash, transaction& tx) const = 0;

    // below, with time we can other pure virtual methods 
    // to the AbstractCore, if needed. For now, the above three are 
    // essential

};

/**
 * Micro version of cryptonode::core class
 * Micro version of constructor,
 * init and destructor are implemted.
 *
 * Just enough to read the blockchain
 * database for use in the example.
 */
class MicroCore : public AbstractCore 
{

    string blockchain_path;

    tx_memory_pool m_mempool;
    Blockchain core_storage;

    hw::device* m_device;

    network_type nettype;

    bool initialization_succeded {false};

public:

    //   <amoumt,
    //    tuple<total_instances, unlocked_instances, recent_instances>
    using histogram_map = std::map<uint64_t,
                               std::tuple<uint64_t,  uint64_t, uint64_t>>;

    MicroCore();

    MicroCore(string _blockchain_path, network_type nettype);

    /**
     * Initialized the MicroCore object.
     *
     * Create BlockchainLMDB on the heap.
     * Open database files located in blockchain_path.
     * Initialize m_blockchain_storage with the BlockchainLMDB object.
     */    
    virtual bool
    init(const string& _blockchain_path, network_type nt);

    virtual Blockchain const&
    get_core() const;

    virtual tx_memory_pool const&
    get_mempool() const;

    virtual hw::device* const
    get_device() const;

    virtual network_type
    get_nettype() const;
    
    virtual uint64_t
    get_num_outputs(uint64_t amount) const override;

    virtual void
    get_output_key(uint64_t amount,
                   vector<uint64_t> const& absolute_offsets,
                   vector<cryptonote::output_data_t>& outputs) 
                    const override;

    virtual output_data_t
    get_output_key(uint64_t amount,
                   uint64_t global_amount_index) const; 

    virtual bool
    get_transactions(
            std::vector<crypto::hash> const& txs_ids,
            std::vector<transaction>& txs,
            std::vector<crypto::hash>& missed_txs) const;

    virtual std::vector<block>
    get_blocks_range(uint64_t h1, uint64_t h2) const;

    virtual uint64_t
    get_tx_unlock_time(crypto::hash const& tx_hash) const;

    virtual bool
    have_tx(crypto::hash const& tx_hash) const;

    virtual bool
    tx_exists(crypto::hash const& tx_hash, uint64_t& tx_id) const;

    virtual tx_out_index
    get_output_tx_and_index(uint64_t amount, uint64_t index) const;

    virtual uint64_t
    get_tx_block_height(crypto::hash const& tx_hash) const;

    virtual std::vector<uint64_t>
    get_tx_amount_output_indices(uint64_t tx_id) const;

    virtual bool
    get_mempool_txs(
            std::vector<tx_info>& tx_infos,
            std::vector<spent_key_image_info>& key_image_infos) const;

    virtual bool
    get_mempool_txs(std::vector<transaction>& txs) const;

    virtual uint64_t
    get_current_blockchain_height() const;

    virtual uint64_t
    get_hard_fork_version(uint64_t height) const;

    virtual void
    get_output_tx_and_index(
            uint64_t amount,
            std::vector<uint64_t> const& offsets,
            std::vector<tx_out_index>& indices) const override;

    virtual bool
    get_output_histogram(
            vector<uint64_t> const& amounts,
            uint64_t min_count,
            histogram_map& histogram,
            bool unlocked = true,
            uint64_t recent_cutoff = 0) const;


    // mimicks core_rpc_server::on_get_output_histogram(..)
    virtual bool
    get_output_histogram(
            COMMAND_RPC_GET_OUTPUT_HISTOGRAM::request const& req,
            COMMAND_RPC_GET_OUTPUT_HISTOGRAM::response& res) const;


    virtual bool
    get_outs(COMMAND_RPC_GET_OUTPUTS_BIN::request const& req,
             COMMAND_RPC_GET_OUTPUTS_BIN::response& res) const;

    virtual uint64_t
    get_dynamic_base_fee_estimate(uint64_t grace_blocks) const;

    bool
    get_block_complete_entry(block const& b, block_complete_entry& bce);

    virtual bool
    get_block_from_height(uint64_t height, block& blk) const;

    virtual bool
    get_tx(crypto::hash const& tx_hash, transaction& tx) const override;

    virtual bool
    decrypt_payment_id(crypto::hash8& payment_id,
                       public_key const& public_key,
                       secret_key const& secret_key) const;

    virtual bool
    init_success() const;    

    virtual ~MicroCore() = default;
};

}



#endif //XMREG01_MICROCORE_H
