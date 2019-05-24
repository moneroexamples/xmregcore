
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../src/Account.h"

#include "mocks.h"
#include "JsonTx.h"

namespace
{

using namespace xmreg;

TEST(ACCOUNT, DefaultConstruction)
{
    auto acc = make_account();

    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::NONE);
    EXPECT_FALSE(*acc);
}

TEST(ACCOUNT, FullConstruction)
{
    auto jtx = construct_jsontx("ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2");

    ASSERT_TRUE(jtx);

    auto acc = make_account(jtx->ntype,
                               jtx->sender.address,
                               jtx->sender.viewkey,
                               jtx->sender.spendkey);

    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::PRIMARY);

    auto const& sender = jtx->jtx["sender"];

    EXPECT_EQ(acc->nt(), jtx->ntype);
    EXPECT_EQ(acc->ai2str(), sender["address"]);
    EXPECT_EQ(acc->vk2str(), sender["viewkey"]);
    EXPECT_EQ(acc->sk2str(), sender["spendkey"]);

    subaddress_index idx {0, 0};

    EXPECT_EQ(acc->index(), idx);

    EXPECT_FALSE(acc->ai().is_subaddress);

    EXPECT_TRUE(acc);
}

TEST(ACCOUNT, FullConstructionFromStrings)
{
    auto jtx = construct_jsontx("ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2");

    ASSERT_TRUE(jtx);

    auto const& sender = jtx->jtx["sender"];

    auto acc = make_account(sender["address"],
                               sender["viewkey"],
                               sender["spendkey"]);

    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::PRIMARY);

    EXPECT_EQ(acc->nt(), jtx->ntype);
    EXPECT_EQ(acc->ai().address, jtx->sender.address.address);
    EXPECT_EQ(acc->vk(), jtx->sender.viewkey);
    EXPECT_EQ(acc->sk(), jtx->sender.spendkey);
}

TEST(ACCOUNT, OnlyAddressAndViewkeyFromStrings)
{
    auto jtx = construct_jsontx("ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2");

    ASSERT_TRUE(jtx);

    auto const& sender = jtx->jtx["sender"];

    auto acc = make_account(sender["address"],
                               sender["viewkey"]);

    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::PRIMARY);

    EXPECT_EQ(acc->nt(), jtx->ntype);
    EXPECT_EQ(acc->ai().address, jtx->sender.address.address);
    EXPECT_EQ(acc->vk(), jtx->sender.viewkey);
}

TEST(ACCOUNT, NoSpendandViewKeysConstruction)
{
    auto jtx = construct_jsontx("ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2");

    ASSERT_TRUE(jtx);

    auto const& sender = jtx->jtx["sender"];

    auto acc = make_account(sender["address"],
                               sender["viewkey"]);

    EXPECT_EQ(acc->nt(), jtx->ntype);
    EXPECT_EQ(acc->ai().address, jtx->sender.address.address);
    EXPECT_EQ(acc->vk(), jtx->sender.viewkey);
    EXPECT_FALSE(acc->sk());

    auto acc2 = make_account(sender["address"]);

    EXPECT_EQ(acc2->nt(), jtx->ntype);
    EXPECT_EQ(acc2->ai().address, jtx->sender.address.address);
    EXPECT_FALSE(acc2->vk());
    EXPECT_FALSE(acc2->sk());
}

TEST(ACCOUNT, FullConstructionSubAddress)
{
    auto jtx = construct_jsontx("d7dcb2daa64b5718dad71778112d48ad62f4d5f54337037c420cb76efdd8a21c");

    ASSERT_TRUE(jtx);

    auto const& recipient1 = jtx->recipients[1];

    auto acc = make_account(jtx->ntype,
                               recipient1.address,
                               recipient1.viewkey,
                               recipient1.spendkey);

    auto const& jrecipient = jtx->jtx["recipient"][1];

    EXPECT_EQ(acc->nt(), jtx->ntype);
    EXPECT_EQ(acc->ai2str(), jrecipient["address"]);
    EXPECT_EQ(acc->vk2str(), jrecipient["viewkey"]);
    EXPECT_EQ(acc->sk2str(), jrecipient["spendkey"]);
    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::SUBADDRESS);

    EXPECT_FALSE(acc->index());

    EXPECT_TRUE(acc->ai().is_subaddress);
    EXPECT_TRUE(acc);
}

TEST(ACCOUNT, FailedConstructionFromString1)
{
    string const wrong_address = "fgdgsfdfgs";
    string const wrong_viewkey = "fgdgsfdfgs";
    string const wrong_spendkey = "fgdgsfdfgs";

    auto acc = make_account(wrong_address, wrong_viewkey, wrong_spendkey);

    EXPECT_EQ(acc, nullptr);
}

TEST(ACCOUNT, FailedConstructionFromString2)
{
    // last  letter missing "V"
    string const wrong_address = "56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbP";
    string const wrong_viewkey = "b45e6f38b2cd1c667459527decb438cdeadf9c64d93c8bccf40a9bf98943dc09";

    auto acc = make_account(wrong_address, wrong_viewkey);

    EXPECT_EQ(acc, nullptr);
}

TEST(ACCOUNT, FailedConstructionFromNonString)
{
    auto jtx = construct_jsontx("d7dcb2daa64b5718dad71778112d48ad62f4d5f54337037c420cb76efdd8a21c");

    ASSERT_TRUE(jtx);

    auto const& sender = jtx->jtx["sender"];

    address_parse_info wrong_address {};

    // just a key for wich crypto::check_key fails.
    string wrong_key (64, 'z');

    wrong_address.address.m_view_public_key 
        = *reinterpret_cast<public_key const*>(wrong_key.data()); 
    
    //cout << '\n' << pod_to_hex(wrong_address.address.m_view_public_key) 
    //     << '\n';

    wrong_address.address.m_spend_public_key 
        = wrong_address.address.m_view_public_key; 
    
    auto acc = make_account(network_type::STAGENET, wrong_address);

    EXPECT_EQ(acc, nullptr);
}

TEST(SUBADDRESS, BasicGenerationTest)
{
    auto jtx = construct_jsontx("d7dcb2daa64b5718dad71778112d48ad62f4d5f54337037c420cb76efdd8a21c");

    ASSERT_TRUE(jtx);

    MicroCore mcore;

    auto* const hw = mcore.get_device();

    // generate first subaddress
    uint32_t i {1};

    subaddress_index index {0, i};

    account_public_address subaddres
            = hw->get_subaddress(jtx->sender.get_account_keys(),
                                 index);

    string subaddr_str = get_account_address_as_str(jtx->ntype,
                                      !index.is_zero(), subaddres);


    auto acc = make_account(index, subaddr_str);

    //cout << i << ": " <<  subaddr_str << '\n';

    EXPECT_EQ(subaddr_str,
              "77JBM7fQNgNKyqHN8dc7DN1mJ4CQZyHg5fXFUstQcHCYEp3rUXVGd8U8ezAdNPDwW7AxejmjQLhz9HjtuW4BwvCdBAcGxH5"s);

    EXPECT_TRUE(acc->is_subaddress());
    EXPECT_EQ(acc->ai2str(), subaddr_str);
    EXPECT_EQ(acc->index(), index);
}

TEST(SUBADDRESS, UsingGenSubAddress)
{
    auto jtx = construct_jsontx("d7dcb2daa64b5718dad71778112d48ad62f4d5f54337037c420cb76efdd8a21c");

    ASSERT_TRUE(jtx);
    
    auto const& sender = jtx->jtx["sender"];

    auto acc = make_account(sender["address"],
                               sender["viewkey"]);

    ASSERT_FALSE(acc->is_subaddress());

    auto pacc = static_cast<PrimaryAccount*>(acc.get());

    auto sacc = pacc->gen_subaddress({0, 1}); 
    
    EXPECT_EQ(sacc->ai2str(),
              "77JBM7fQNgNKyqHN8dc7DN1mJ4CQZyHg5fXFUstQcHCYEp3rUXVGd8U8ezAdNPDwW7AxejmjQLhz9HjtuW4BwvCdBAcGxH5"s);
}

TEST(SUBADDRESS, UsingGenSubAddress1)
{
    // monerowalletstagenet3
    string address = "56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbPV";
    string spendkey= "df0f5720ae0b69454ca7db35db677272c7c19513cd0dc4147b0e00792a10f406";
    string viewkey = "b45e6f38b2cd1c667459527decb438cdeadf9c64d93c8bccf40a9bf98943dc09";

    auto acc = make_account(address, viewkey, spendkey);

    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::PRIMARY);
    
    auto pacc = static_cast<PrimaryAccount*>(acc.get());

    string expected_subaddress_0_1 = "77JBM7fQNgNKyqHN8dc7DN1mJ4CQZyHg5fXFUstQcHCYEp3rUXVGd8U8ezAdNPDwW7AxejmjQLhz9HjtuW4BwvCdBAcGxH5";
    auto sacc1 = pacc->gen_subaddress(0, 1);
    EXPECT_EQ(sacc1->ai2str(), expected_subaddress_0_1);
    EXPECT_EQ(sacc1->vk2str(), viewkey);
    EXPECT_EQ(sacc1->sk2str(), spendkey);

    string expected_subaddress_0_10 = "7BTd51FGijwGquK2aPpiTPNvdKrf2CQpkCy2Z8EWFgSK1bapBkjvw2pLenzdLWeP91C3o5SHVuhRggQNHi3jP8jERBiZkBS";
    auto sacc2 = pacc->gen_subaddress(0, 10);
    EXPECT_EQ(sacc2->ai2str(), expected_subaddress_0_10);
    EXPECT_EQ(sacc2->vk2str(), viewkey);
    EXPECT_EQ(sacc2->sk2str(), spendkey);
    EXPECT_TRUE(sacc2->is_subaddress());

    string expected_subaddress_2_0 = "7A3KWyKJVWbc6ZKnygwoCSVsuSVHEvR3zGm4ak8cLdN3CDjDiKfdcQDfCTojpQX35PZtxqGJohm3aGxdvw7TMTLGBVkWZ3t";
    auto sacc3 = pacc->gen_subaddress(2, 0);
    EXPECT_EQ(sacc3->ai2str(), expected_subaddress_2_0);
    EXPECT_EQ(sacc3->vk2str(), viewkey);
    EXPECT_EQ(sacc3->sk2str(), spendkey);
    EXPECT_TRUE(sacc3->is_subaddress());

    string expected_subaddress_3_5 = "73TUymFmFiqejXhrr38VAP15CrcLF7efYNi7DzC4yKvZVM6a5PhXp3v76uVagnZFSTTJrtGSXqnYEYX8JUqnFBtbUG3QaTi";
    auto sacc4 = pacc->gen_subaddress(3, 5);
    EXPECT_EQ(sacc4->ai2str(), expected_subaddress_3_5);
    EXPECT_EQ(sacc4->vk2str(), viewkey);
    EXPECT_EQ(sacc4->sk2str(), spendkey);
    EXPECT_TRUE(sacc4->is_subaddress());
    subaddress_index idx4 {3, 5};
    EXPECT_EQ(sacc4->index().value(), idx4);
}

TEST(SUBADDRESS, UsingGenSubAddressNoSpendkey)
{
    // monerowalletstagenet3
    string address = "56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbPV";
    string viewkey = "b45e6f38b2cd1c667459527decb438cdeadf9c64d93c8bccf40a9bf98943dc09";

    auto acc = make_account(address, viewkey);

    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::PRIMARY);
    
    auto pacc = static_cast<PrimaryAccount*>(acc.get());

    string expected_subaddress_0_1 = "77JBM7fQNgNKyqHN8dc7DN1mJ4CQZyHg5fXFUstQcHCYEp3rUXVGd8U8ezAdNPDwW7AxejmjQLhz9HjtuW4BwvCdBAcGxH5";
    
    auto sacc1 = pacc->gen_subaddress(0, 1);

    EXPECT_EQ(sacc1->ai2str(), expected_subaddress_0_1);
    EXPECT_EQ(sacc1->vk2str(), viewkey);
    EXPECT_EQ(sacc1->sk2str(), "");
}


TEST(SUBADDRESS, UsingGenSubAddressNoViewkey)
{
    // monerowalletstagenet3
    string address = "56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbPV";

    auto acc = make_account(address);

    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::PRIMARY);
    
    auto pacc = static_cast<PrimaryAccount*>(acc.get());

    auto sacc1 = pacc->gen_subaddress(0, 1);

    EXPECT_EQ(sacc1, nullptr);
}

TEST(SUBADDRESS, AddSubAddressNoSpendkey)
{
    // monerowalletstagenet3
    string address = "56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbPV";
    string viewkey = "b45e6f38b2cd1c667459527decb438cdeadf9c64d93c8bccf40a9bf98943dc09";

    auto acc = make_account(address, viewkey);

    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::PRIMARY);
    
    auto pacc = static_cast<PrimaryAccount*>(acc.get());

    subaddress_index idx1 {2, 4};

    auto sacc_it = pacc->add_subaddress_index(2, 4);

    auto sacc = pacc->gen_subaddress(sacc_it->second);

    EXPECT_EQ(*sacc->index(), idx1);
}


TEST(SUBADDRESS, PopulateSubaddresses)
{
	// monerowalletstagenet3
	string address = "56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbPV";
	string viewkey = "b45e6f38b2cd1c667459527decb438cdeadf9c64d93c8bccf40a9bf98943dc09";

	auto pacc = make_primaryaccount(address, viewkey);

	EXPECT_EQ(pacc->type(), Account::ADDRESS_TYPE::PRIMARY);

    EXPECT_EQ(pacc->get_subaddress_map().size(), 10'000);

    EXPECT_EQ(pacc->get_next_subbaddress_acc_id(), 
              PrimaryAccount::SUBADDRESS_LOOKAHEAD_MAJOR);
	
	for (auto const& kv: *pacc)
	{
		auto sacc = pacc->gen_subaddress(kv.second);
		if (!sacc) continue;
        //cout << *sacc << endl;
		EXPECT_EQ(kv.first, sacc->psk());
	}
}


TEST(SUBADDRESS, ExpandSubaddresses)
{
	// monerowalletstagenet3
	string address = "56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbPV";
	string viewkey = "b45e6f38b2cd1c667459527decb438cdeadf9c64d93c8bccf40a9bf98943dc09";

	auto pacc = make_primaryaccount(address, viewkey);

    EXPECT_EQ(pacc->get_subaddress_map().size(), 10'000);

    EXPECT_EQ(pacc->get_next_subbaddress_acc_id(), 
              PrimaryAccount::SUBADDRESS_LOOKAHEAD_MAJOR);


    auto new_last_acc_id = pacc->get_next_subbaddress_acc_id() 
                           + 10;

    pacc->expand_subaddresses(new_last_acc_id);

    EXPECT_EQ(pacc->get_subaddress_map().size(), 12'000);

    EXPECT_EQ(pacc->get_next_subbaddress_acc_id(), 
              PrimaryAccount::SUBADDRESS_LOOKAHEAD_MAJOR + 10);

}


}
