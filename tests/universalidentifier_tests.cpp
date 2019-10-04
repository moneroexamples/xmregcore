#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <boost/iterator/filter_iterator.hpp>

#include "../src/UniversalIdentifier.hpp"

#include "mocks.h"
#include "JsonTx.h"

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
operator==(const vector<Input::info>& lhs, 
           const vector<JsonTx::input>& rhs)
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


inline bool
operator==(const Input::info& lhs, const Input::info& rhs)
{
    return lhs.amount == rhs.amount
            && lhs.out_pub_key == rhs.out_pub_key
            && lhs.key_img == rhs.key_img;
}

inline bool
operator!=(const Input::info& lhs, const Input::info& rhs)
{
    return !(lhs == rhs);
}

inline bool
operator==(const vector<Input::info>& lhs, 
           const vector<Input::info>& rhs)
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
        "bd461b938c3c8c8e4d9909852221d5c37350ade05e99ef836d6ccb628f6a5a0e"s,
        "f81ecd0381c0b89f23cffe86a799e924af7b5843c663e8c07db98a14e913585e"s,
        "386ac4fbf7d3d2ab6fd4f2d9c2e97d00527ca2867e33cd7aedb1fd05a4b791ec"s,
        "e658966b256ca30c85848751ff986e3ba7c7cfdadeb46ee1a845a042b3da90db"s
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
          make_unique<LegacyPaymentID>());

   identifier.identify();

   auto pid = identifier.get<0>()->get();

   //cout << pod_to_hex(jtx->payment_id) << '\n';

   if (jtx->payment_id == crypto::null_hash)
   {
       EXPECT_FALSE(pid);
   }
   else
   {
       EXPECT_TRUE(pid);
       EXPECT_TRUE(*pid == jtx->payment_id);
   }
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
   
   auto pid = identifier.get<0>()->get();

   if (jtx->payment_id8 == crypto::null_hash8)
   {
       EXPECT_FALSE(pid);
   }
   else
   {
       EXPECT_TRUE(pid);
       EXPECT_TRUE(*pid == jtx->payment_id8e);
   }
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
        known_outputs.insert({input.out_pub_key, input.amount});
        expected_total += input.amount;
    }


    // now add some random ones
    
    for (size_t i = 0; i < 20; ++i)
    {
         auto rand_pk = crypto::rand<public_key>();
         known_outputs.insert({rand_pk, 4353534534}); // some amount
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

// in some rare cases, different txs have same output
// public keys, but different amounts. so known_output map
// is of type of unordered_mulitimap to allow for such a 
// situation
//TEST_P(ModularIdentifierTest, InputWithKnownMultipleOutputs)
//{
    //string tx_hash_str = GetParam();

    //auto jtx = construct_jsontx(tx_hash_str);

    //ASSERT_TRUE(jtx);

    //// make vector of known outputs
    
    //Input::known_outputs_t known_outputs;
    //uint64_t expected_total {};

    //// first add real public keys
    //for (auto&& input: jtx->sender.inputs)
    //{
        //known_outputs.insert({input.out_pub_key, input.amount});

        //// insert duplicate output with different amount
        //known_outputs.insert({input.out_pub_key, input.amount*2});

        //expected_total += input.amount;
    //}

    //// now add some random ones
    
    //for (size_t i = 0; i < 20; ++i)
    //{
         //auto rand_pk = crypto::rand<public_key>();
         //known_outputs.insert({rand_pk, 4353534534}); // some amount
    //}

    //MockMicroCore mcore;

    //ADD_MOCKS(mcore);
    
    //auto identifier = make_identifier(jtx->tx,
          //make_unique<Input>(
                         //&jtx->sender.address,
                         //&jtx->sender.viewkey,
                         //&known_outputs,
                         //&mcore));

    
    //identifier.identify();

    //// we just expect to get two return vector of inputs
    //// twice the size of the actual inputs due to there being
    //// double outputs
    
    //ASSERT_EQ(identifier.get<0>()->get().size(), 
            //jtx->sender.inputs.size()*2);
    
    //// since second dupicate of each outputs has double
    //// amount, the expected total is 3 times greater
    //// than real amount
    //ASSERT_EQ(identifier.get<0>()->get_total(),
              //expected_total*3);
//}

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


TEST(Subaddresses, RegularTwoOutputTxToSubaddress)
{
    // this tx has funds for one subaddress. so we try to identify the outputs
    // and the subaddress using primary address of the recipient
    auto jtx = construct_jsontx("024dc13cb11d411682f04d41b52931849527d530e4cb198a63526c13da31a413");

    ASSERT_TRUE(jtx);

    // recipeint primary address and viewkey
    string const raddress {"56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbPV"};
    string const rviewkey {"b45e6f38b2cd1c667459527decb438cdeadf9c64d93c8bccf40a9bf98943dc09"};
    
    auto racc = make_primaryaccount(raddress, rviewkey);

    // make sure we have primary address
    ASSERT_FALSE(racc->is_subaddress());
    
    racc->populate_subaddress_indices();

    auto identifier = make_identifier(jtx->tx,
          make_unique<Output>(racc.get()));

    identifier.identify();

    EXPECT_EQ(identifier.get<0>()->get().size(),
              jtx->recipients.at(0).outputs.size());

    EXPECT_TRUE(identifier.get<0>()->get() 
            == jtx->recipients.at(0).outputs);

    auto const& output_info 
        = identifier.get<0>()->get().at(0);

    EXPECT_TRUE(output_info.has_subaddress_index());
    
    subaddress_index expected_idx {0, 6};

    EXPECT_EQ(output_info.subaddr_idx, expected_idx);
}

TEST(Subaddresses, MultiOutputTxToSubaddress)
{
    // this tx has funds for few subaddress of the same primary account. 
    // so we try to identify the outputs
    // and the subaddresses indices using primary address of the recipient
    auto jtx = construct_jsontx("f81ecd0381c0b89f23cffe86a799e924af7b5843c663e8c07db98a14e913585e");

    ASSERT_TRUE(jtx);

    // recipeint primary address and viewkey
    string const raddress {"56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbPV"};
    string const rviewkey {"b45e6f38b2cd1c667459527decb438cdeadf9c64d93c8bccf40a9bf98943dc09"};
    
    auto racc = make_primaryaccount(raddress, rviewkey);

    // make sure we have primary address
    ASSERT_FALSE(racc->is_subaddress());
    
    auto identifier = make_identifier(jtx->tx,
          make_unique<Output>(racc.get()));
    
    identifier.identify();
    
    auto const& outputs_found 
        = identifier.get<Output>()->get();

    auto total_outputs = calc_total_xmr(outputs_found);

    uint64_t expected_total {0};

    set<pair<string, string>> output_indices;

    for (auto&& out: outputs_found)
    {
        stringstream ss;

        ss << out.subaddr_idx;

        output_indices.insert({pod_to_hex(out.pub_key), 
                               ss.str()});

    }
    
    set<pair<string, string>> expected_indices;

    for (auto const& jrecipient: jtx->recipients)
    {
        auto identifier_rec = make_identifier(jtx->tx,
              make_unique<Output>(&jrecipient.address,
                                  &jrecipient.viewkey));

       identifier_rec.identify();

       auto const& output_rec
           = identifier_rec.get<Output>()->get();
    
       expected_total += calc_total_xmr(output_rec);


        for (auto&& out: output_rec)
        {
             stringstream ss;

             ss << *jrecipient.subaddr_idx;

             expected_indices.insert({pod_to_hex(out.pub_key), 
                                      ss.str()});
        }
    }

    EXPECT_EQ(total_outputs, expected_total);

    vector<decltype(output_indices)::value_type> result;

    std::set_symmetric_difference(output_indices.begin(),
                                  output_indices.end(),
                                  expected_indices.begin(),
                                  expected_indices.end(),
                                  std::back_inserter(result));

    EXPECT_TRUE(result.empty());

    // check if expansion of subbaddress list worked
    EXPECT_EQ(racc->get_next_subbaddress_acc_id(),
              PrimaryAccount::SUBADDRESS_LOOKAHEAD_MAJOR + 49);

    EXPECT_EQ(racc->get_subaddress_map().size(), 10'000 + 49*200);
}


TEST(Subaddresses, GuessInputFromSubaddress)
{
    auto jtx = construct_jsontx("386ac4fbf7d3d2ab6fd4f2d9c2e97d00527ca2867e33cd7aedb1fd05a4b791ec");

    ASSERT_TRUE(jtx);
    
    MockMicroCore mcore;
    ADD_MOCKS(mcore);


    // sender primary address and viewkey
    string const sender_addr = jtx->sender.address_str();
    string const sender_viewkey = pod_to_hex(jtx->sender.viewkey);
    string const sender_spendkey = pod_to_hex(jtx->sender.spendkey);

    auto sender = make_primaryaccount(sender_addr, 
                                      sender_viewkey,
                                      sender_spendkey);
    
    auto identifier = make_identifier(jtx->tx,
          make_unique<GuessInput>(sender.get(), &mcore),
          make_unique<RealInput>(sender.get(), &mcore));

   identifier.identify();
   
   auto const& found_inputs = identifier.get<GuessInput>()->get();
   auto const& expected_inputs = identifier.get<RealInput>()->get();

   EXPECT_EQ(found_inputs.size(), expected_inputs.size());
   
   EXPECT_TRUE(found_inputs == expected_inputs);


   // now lets check recipient outputs

   // recipeint primary address and viewkey
   string const raddress {"55ZbQdMnZHPFS8pmrhHN5jMpgJwnnTXpTDmmM5wkrBBx4xD6aEnpZq7dPkeDeWs67TV9HunDQtT3qF2UGYWzGGxq3zYWCBE"};
   string const rviewkey {"c8a4d62e3c86de907bd84463f194505ab07fc231b3da753342d93fccb5d39203"};


   auto acc = make_account(raddress, rviewkey);

   ASSERT_TRUE(acc->type() == Account::PRIMARY);

   auto primary_account = dynamic_cast<PrimaryAccount*>(acc.get());
    
   ASSERT_TRUE(primary_account);

   primary_account->populate_subaddress_indices();

   auto ridentifier = make_identifier(jtx->tx,
         make_unique<Output>(primary_account));
    
   ridentifier.identify();
    
   auto const& outputs_found 
       = ridentifier.get<Output>()->get();

   cout << outputs_found << endl;

   EXPECT_EQ(outputs_found.size(), 3);

   auto total_outputs = calc_total_xmr(outputs_found);
   
   EXPECT_EQ(total_outputs, 8000000000000);

   vector<subaddress_index> expected_indices {{0, 1}, {0, 2}, {0, 3}}; 

   for (size_t i {0}; i < outputs_found.size(); ++i)
   {
        EXPECT_EQ(outputs_found[i].subaddr_idx,
                  expected_indices[i]);
   }

   EXPECT_EQ(primary_account->get_next_subbaddress_acc_id(),
             PrimaryAccount::SUBADDRESS_LOOKAHEAD_MAJOR);
}

TEST(Subaddresses, RealInputsToSubaddress)
{
    auto jtx = construct_jsontx("e658966b256ca30c85848751ff986e3ba7c7cfdadeb46ee1a845a042b3da90db");

    ASSERT_TRUE(jtx);
    
    MockMicroCore mcore;
    ADD_MOCKS(mcore);

    // sender primary address and viewkey
    string const sender_addr = jtx->sender.address_str();
    string const sender_viewkey = pod_to_hex(jtx->sender.viewkey);
    string const sender_spendkey = pod_to_hex(jtx->sender.spendkey);

    auto sender = make_primaryaccount(sender_addr, 
                                      sender_viewkey,
                                      sender_spendkey);
    
    auto identifier = make_identifier(jtx->tx,
          make_unique<GuessInput>(sender.get(), &mcore),
          make_unique<RealInput>(sender.get(), &mcore));

   identifier.identify();
   
   auto const& found_inputs = identifier.get<GuessInput>()->get();
   auto const& expected_inputs = identifier.get<RealInput>()->get();

   EXPECT_EQ(found_inputs.size(), expected_inputs.size());
   
   EXPECT_TRUE(found_inputs == expected_inputs);
}

}
