#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <boost/iterator/filter_iterator.hpp>

#include "../src/UniversalIdentifier.hpp"

#include "mocks.h"

#define ADD_MOCKS(mcore) \
    EXPECT_CALL(mcore, get_output_tx_and_index(_, _, _)) \
            .WillRepeatedly( \
                Invoke(&*jtx, &JsonTx::get_output_tx_and_index)); \
    \
    EXPECT_CALL(mcore, get_tx(_, _)) \
            .WillRepeatedly( \
                Invoke(&*jtx, &JsonTx::get_tx)); \
    \
    EXPECT_CALL(mcore, get_output_key(_, _, _)) \
            .WillRepeatedly( \
                Invoke(&*jtx, &JsonTx::get_output_key));\
    \
    EXPECT_CALL(mcore, get_num_outputs(_)) \
            .WillRepeatedly(Return(1e10));

namespace
{

using namespace xmreg;


// equality operators for outputs

inline bool
operator==(const Output::info& lhs, const JsonTx::output& rhs)
{
    return lhs.amount == rhs.amount
            && lhs.pub_key == rhs.pub_key
            && lhs.idx_in_tx == rhs.index;
}

inline bool
operator!=(const Output::info& lhs, const JsonTx::output& rhs)
{
    return !(lhs == rhs);
}

inline bool
operator==(const vector<Output::info>& lhs, const vector<JsonTx::output>& rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (size_t i = 0; i < lhs.size(); i++)
    {
        if (lhs[i] != rhs[i])
            return false;
    }

    return true;
}



// equality operators for inputs

inline bool
operator==(const Input::info& lhs, const JsonTx::input& rhs)
{
    return lhs.amount == rhs.amount
            && lhs.out_pub_key == rhs.out_pub_key
            && lhs.key_img == rhs.key_img;
}

inline bool
operator!=(const Input::info& lhs, const JsonTx::input& rhs)
{
    return !(lhs == rhs);
}

inline bool
operator==(const vector<Input::info>& lhs, const vector<JsonTx::input>& rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (size_t i = 0; i < lhs.size(); i++)
    {
        if (lhs[i] != rhs[i])
            return false;
    }

    return true;
}

class DifferentJsonTxs :
        public ::testing::TestWithParam<string>
{

};

class ModularIdentifierTest : public DifferentJsonTxs
{};

INSTANTIATE_TEST_CASE_P(
    DifferentJsonTxs, ModularIdentifierTest,
    ::testing::Values(
        "ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2"s,
        "f3c84fe925292ec5b4dc383d306d934214f4819611566051bca904d1cf4efceb"s,
        "d7dcb2daa64b5718dad71778112d48ad62f4d5f54337037c420cb76efdd8a21c"s,
        "61f756a299efd17442eed5437fa03cbda6b01f341907845f8880bf30319fa01c"s,
        "ae8f3ad29a40e02dff6a3267c769f08c0af3dc8858683c90ce3ef90212cb7e4b"s,
        "140807b970e52b7c633d7ca0ba5be603922aa7a2a1213bdd16d3c1a531402bf6"s,
        "a7a4e3bdb305b97c43034440b0bc5125c23b24d0730189261151c0aa3f2a05fc"s,
        "c06df274acc273fbce0666b2c8846ac6925a1931fb61e3020b7cc5410d4646b1"s,
        "d89f32f1434b6a668cbbc5c55cb1c0c64e41fccb89f6b1eef210fefdacbdd89f"s,
        "bd461b938c3c8c8e4d9909852221d5c37350ade05e99ef836d6ccb628f6a5a0e"s
        ));

TEST_P(ModularIdentifierTest, OutputsRingCT)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    auto identifier = make_identifier(jtx->tx,
          make_unique<Output>(&jtx->sender.address,
                              &jtx->sender.viewkey));

    identifier.identify();

    ASSERT_EQ(identifier.get<0>()->get().size(),
              jtx->sender.outputs.size());

    ASSERT_TRUE(identifier.get<0>()->get() == jtx->sender.outputs);

    ASSERT_EQ(identifier.get<0>()->get_total(),
              jtx->sender.change);
}



TEST_P(ModularIdentifierTest, OutputsRingCTCoinbaseTx)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    auto identifier = make_identifier(jtx->tx,
          make_unique<Output>(&jtx->recipients.at(0).address,
                              &jtx->recipients.at(0).viewkey));

    identifier.identify();

    ASSERT_TRUE(identifier.get<0>()->get()
                == jtx->recipients.at(0).outputs);

    ASSERT_EQ(identifier.get<0>()->get_total(),
              jtx->recipients.at(0).amount);
}

TEST_P(ModularIdentifierTest, MultiOutputsRingCT)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    for (auto const& jrecipient: jtx->recipients)
    {
        auto identifier = make_identifier(jtx->tx,
              make_unique<Output>(&jrecipient.address,
                                  &jrecipient.viewkey));

       identifier.identify();

       EXPECT_TRUE(identifier.get<0>()->get()
                    == jrecipient.outputs);
    }
}


TEST_P(ModularIdentifierTest, LegacyPaymentID)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    auto identifier = make_identifier(jtx->tx,
          make_unique<LegacyPaymentID>(nullptr, nullptr));

   identifier.identify();

   EXPECT_TRUE(identifier.get<0>()->get()
                == jtx->payment_id);

   EXPECT_TRUE(identifier.get<0>()->raw()
                == jtx->payment_id);
}


TEST_P(ModularIdentifierTest, IntegratedPaymentID)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    auto identifier = make_identifier(jtx->tx,
          make_unique<IntegratedPaymentID>(
                         &jtx->recipients[0].address,
                         &jtx->recipients[0].viewkey));

   identifier.identify();
   
   //cout << "decrypted: " << pod_to_hex(identifier.get<0>()->get()) 
   //    << ", " << pod_to_hex(jtx->payment_id8e)  << endl;


   EXPECT_TRUE(identifier.get<0>()->get()
                == jtx->payment_id8e);

   EXPECT_TRUE(identifier.get<0>()->raw()
                == jtx->payment_id8);
   
   //cout << "row: " << pod_to_hex(identifier.get<0>()->raw()) 
   //    << ", " << pod_to_hex(jtx->payment_id8) << endl;
}

TEST_P(ModularIdentifierTest, InputWithKnownOutputs)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    // make vector of known outputs
    
    Input::known_outputs_t known_outputs;
    uint64_t expected_total {};

    // first add real public keys
    for (auto&& input: jtx->sender.inputs)
    {
        known_outputs[input.out_pub_key] = input.amount;
        expected_total += input.amount;
    }


    // now add some random ones
    
    for (size_t i = 0; i < 20; ++i)
    {
         auto rand_pk = crypto::rand<public_key>();
         known_outputs[rand_pk] = 4353534534; // some amount
    }

    MockMicroCore mcore;

    ADD_MOCKS(mcore);
    
    auto identifier = make_identifier(jtx->tx,
          make_unique<Input>(
                         &jtx->sender.address,
                         &jtx->sender.viewkey,
                         &known_outputs,
                         &mcore));

    
    identifier.identify();

    
    ASSERT_TRUE(identifier.get<0>()->get()
                == jtx->sender.inputs);

    ASSERT_EQ(identifier.get<0>()->get_total(),
              expected_total);
}

TEST_P(ModularIdentifierTest, GuessInputRingCT)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    MockMicroCore mcore;

    ADD_MOCKS(mcore);

    auto identifier = make_identifier(jtx->tx,
          make_unique<GuessInput>(
                    &jtx->sender.address,
                    &jtx->sender.viewkey,
                    &mcore));

   identifier.identify();

//   for (auto const& input_info: identifier.get<0>()->get())
//       cout << input_info << endl;

   auto const& found_inputs = identifier.get<0>()->get();

   EXPECT_GE(found_inputs.size(),
             jtx->sender.inputs.size());

   if (!jtx->sender.inputs.empty())
   {
       // to check whether GuessIdentifier is correct
       // we check weath all outputs and key images are present
       // in its found_inputs
       //
           
       for (auto const& input: jtx->sender.inputs)
       {
       
           // make lambda that will check wether
           // current input's public key matches a
           // given one
           //
           // to do this, first we find all public
           // output keys in found_inputs vector
           // that GuessInput populated

           using input_elem_type
            = std::remove_reference_t<decltype(found_inputs)>::value_type;

           vector<input_elem_type const*>
               matched_public_keys;

           for (auto const& fin: found_inputs)
           {
               if (input.out_pub_key == fin.out_pub_key)
                   matched_public_keys.push_back(&fin);
           }

           if (matched_public_keys.empty())
                FAIL() << "matched_public_keys is empty";

           // second, if we found something, check
           // if matched_public_keys contains
           // key image from senders.input

           bool match_found {false};
           
           for (auto const matched_pk: matched_public_keys)
               if (input.key_img == matched_pk->key_img)
               {
                   match_found = true;
                   SUCCEED();
               }

           if (!match_found)
               FAIL() << "No maching key image found";

       } //for (auto const& input: jtx->sender.inputs)

   } // if (!jtx->sender.inputs.empty())

}

TEST_P(ModularIdentifierTest, RealInputRingCT)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    MockMicroCore mcore;

    ADD_MOCKS(mcore);

    auto identifier = make_identifier(jtx->tx,
          make_unique<RealInput>(
                    &jtx->sender.address,
                    &jtx->sender.viewkey,
                    &jtx->sender.spendkey,
                    &mcore));

   identifier.identify();

//   for (auto const& input_info: identifier.get<0>()->get())
//       cout << input_info << endl;

   EXPECT_TRUE(identifier.get<0>()->get()
                == jtx->sender.inputs);
}

}
