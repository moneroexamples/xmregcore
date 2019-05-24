#pragma once


#include "../src/MicroCore.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace
{

//using json = nlohmann::json;
using namespace std;
using namespace cryptonote;
using namespace epee::string_tools;
using namespace std::chrono_literals;

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Le;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::Return;
using ::testing::Throw;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::SetArgPointee;
using ::testing::_;
using ::testing::internal::FilePath;
using ::testing::Invoke;




class MockMicroCore : public xmreg::MicroCore
{
public:
    MOCK_METHOD2(init, bool(const string& _blockchain_path,
                            network_type nt));

    MOCK_CONST_METHOD0(get_current_blockchain_height, uint64_t());

    MOCK_CONST_METHOD2(get_block_from_height,
                       bool(uint64_t height, block& blk));

    MOCK_CONST_METHOD2(get_blocks_range,
                       std::vector<block>(uint64_t h1,
                                          uint64_t h2));

    MOCK_CONST_METHOD3(get_transactions,
                       bool(const std::vector<crypto::hash>& txs_ids,
                            std::vector<transaction>& txs,
                            std::vector<crypto::hash>& missed_txs));

    MOCK_CONST_METHOD1(have_tx, bool(crypto::hash const& tx_hash));

    MOCK_CONST_METHOD1(get_tx_block_height,
                       uint64_t(crypto::hash const& tx_hash));

    MOCK_CONST_METHOD2(tx_exists,
                       bool(crypto::hash const& tx_hash,
                            uint64_t& tx_id));

    MOCK_CONST_METHOD2(get_output_tx_and_index,
                       tx_out_index(uint64_t amount,
                                    uint64_t index));

    MOCK_CONST_METHOD3(get_output_tx_and_index,
                       void(uint64_t amount,
                            const std::vector<uint64_t> &offsets,
                            std::vector<tx_out_index> &indices));

    MOCK_CONST_METHOD2(get_tx,
                       bool(crypto::hash const& tx_hash,
                            transaction& tx));

    MOCK_CONST_METHOD1(get_num_outputs, uint64_t(uint64_t));

    MOCK_CONST_METHOD3(get_output_key,
                    void(uint64_t amount,
                         vector<uint64_t> const& absolute_offsets,
                         vector<output_data_t>& outputs));

    MOCK_CONST_METHOD2(get_output_key,
                    output_data_t(uint64_t amount,
                                  uint64_t global_amount_index));

    MOCK_CONST_METHOD1(get_tx_amount_output_indices,
                    std::vector<uint64_t>(uint64_t tx_id));

    MOCK_CONST_METHOD2(get_random_outs_for_amounts,
                    bool(COMMAND_RPC_GET_OUTPUT_HISTOGRAM::request const& req,
                         COMMAND_RPC_GET_OUTPUT_HISTOGRAM::response& res));

    MOCK_CONST_METHOD2(get_outs,
                        bool(const COMMAND_RPC_GET_OUTPUTS_BIN::request& req,
                             COMMAND_RPC_GET_OUTPUTS_BIN::response& res));

    MOCK_CONST_METHOD1(get_dynamic_base_fee_estimate,
                       uint64_t(uint64_t grace_blocks));

    MOCK_CONST_METHOD2(get_mempool_txs,
                       bool(vector<tx_info>& tx_infos,
                            vector<spent_key_image_info>& key_image_infos));

};


}
