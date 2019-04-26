
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../src/Account.h"

#include "mocks.h"


namespace
{

using namespace xmreg;

TEST(ACCOUNT, DefaultConstruction)
{
    auto acc = account_factory();

    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::NONE);
    EXPECT_FALSE(*acc);
}

TEST(ACCOUNT, FullConstruction)
{
    auto jtx = construct_jsontx("ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2");

    ASSERT_TRUE(jtx);

    auto acc = account_factory(jtx->ntype,
                               jtx->sender.address,
                               jtx->sender.viewkey,
                               jtx->sender.spendkey);

    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::PRIMARY);

    auto const& sender = jtx->jtx["sender"];

    EXPECT_EQ(acc->nt(), jtx->ntype);
    EXPECT_EQ(acc->ai2str(), sender["address"]);
    EXPECT_EQ(acc->vk2str(), sender["viewkey"]);
    EXPECT_EQ(acc->sk2str(), sender["spendkey"]);

    EXPECT_FALSE(acc->ai().is_subaddress);

    EXPECT_TRUE(acc);
}

TEST(ACCOUNT, FullConstructionFromStrings)
{
    auto jtx = construct_jsontx("ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2");

    ASSERT_TRUE(jtx);

    auto const& sender = jtx->jtx["sender"];

    auto acc = account_factory(sender["address"],
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

    auto acc = account_factory(sender["address"],
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

    auto acc = account_factory(sender["address"],
                               sender["viewkey"]);

    EXPECT_EQ(acc->nt(), jtx->ntype);
    EXPECT_EQ(acc->ai().address, jtx->sender.address.address);
    EXPECT_EQ(acc->vk(), jtx->sender.viewkey);
    EXPECT_FALSE(acc->sk());

    auto acc2 = account_factory(sender["address"]);

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

    auto acc = account_factory(jtx->ntype,
                               recipient1.address,
                               recipient1.viewkey,
                               recipient1.spendkey);

    auto const& jrecipient = jtx->jtx["recipient"][1];

    EXPECT_EQ(acc->nt(), jtx->ntype);
    EXPECT_EQ(acc->ai2str(), jrecipient["address"]);
    EXPECT_EQ(acc->vk2str(), jrecipient["viewkey"]);
    EXPECT_EQ(acc->sk2str(), jrecipient["spendkey"]);
    EXPECT_EQ(acc->type(), Account::ADDRESS_TYPE::SUBADDRESS);

    EXPECT_TRUE(acc->ai().is_subaddress);
    EXPECT_TRUE(acc);
}

TEST(ACCOUNT, FailedConstructionFromString1)
{
    string const wrong_address = "fgdgsfdfgs";
    string const wrong_viewkey = "fgdgsfdfgs";
    string const wrong_spendkey = "fgdgsfdfgs";

    auto acc = account_factory(wrong_address, wrong_viewkey, wrong_spendkey);

    EXPECT_EQ(acc, nullptr);
}

TEST(ACCOUNT, FailedConstructionFromString2)
{
    // last  letter missing "V"
    string const wrong_address = "56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbP";
    string const wrong_viewkey = "b45e6f38b2cd1c667459527decb438cdeadf9c64d93c8bccf40a9bf98943dc09";

    auto acc = account_factory(wrong_address, wrong_viewkey);

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
    
    auto acc = account_factory(network_type::STAGENET, wrong_address);

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

    //cout << i << ": " <<  subaddr_str << '\n';

    EXPECT_EQ(subaddr_str,
              "77JBM7fQNgNKyqHN8dc7DN1mJ4CQZyHg5fXFUstQcHCYEp3rUXVGd8U8ezAdNPDwW7AxejmjQLhz9HjtuW4BwvCdBAcGxH5"s);
}

}
