//
// Created by mwo on 15/06/18.
//

#include "../src/MicroCore.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


namespace
{


using namespace std;
using namespace cryptonote;
using namespace epee::string_tools;
using namespace std::chrono_literals;

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Le;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::internal::FilePath;

//INSTANTIATE_TEST_CASE_P(
//        DifferentMoneroNetworks, BlockchainSetupTest,
//        ::testing::Values(
//                network_type::MAINNET,
//                network_type::TESTNET,
//                network_type::STAGENET));


//class MICROCORE_TEST : public ::testing::Test
//{
//public:

//    static void
//    SetUpTestCase()
//    {
//        string config_path {"../config/config.json"};
//        config_json = xmreg::BlockchainSetup::read_config(config_path);
//    }

//protected:


//    virtual void
//    SetUp()
//    {
//        bc_setup = xmreg::BlockchainSetup{net_type, do_not_relay, config_json};
//    }

//     network_type net_type {network_type::STAGENET};
//     bool do_not_relay {false};
//     xmreg::BlockchainSetup bc_setup;

//     static json config_json;
//};

//json MICROCORE_TEST::config_json;

//TEST_F(MICROCORE_TEST, DefaultConstruction)
//{
//    xmreg::MicroCore mcore;
//    EXPECT_TRUE(true);
//}

//TEST_F(MICROCORE_TEST, InitializationSuccess)
//{
//    xmreg::MicroCore mcore;

//    EXPECT_TRUE(mcore.init(bc_setup.blockchain_path, net_type));

//    EXPECT_TRUE(mcore.get_core().get_db().is_open());
//    EXPECT_TRUE(mcore.get_core().get_db().is_read_only());
//}




}
