#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../src/Account.h"

#include "mocks.h"



namespace
{

using namespace xmreg;

TEST(ACCOUNT, DefaultConstruction)
{
    Account acc;

    EXPECT_EQ(acc.type(), Account::ADDRESS_TYPE::NONE);
    EXPECT_FALSE(acc);
}

TEST(ACCOUNT, FullConstruction)
{
    auto jtx = construct_jsontx("ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2");

    ASSERT_TRUE(jtx);

    Account acc {jtx->ntype,
                jtx->sender.address,
                jtx->sender.viewkey,
                jtx->sender.spendkey};

    auto const& sender = jtx->jtx["sender"];

    EXPECT_EQ(acc.nt(), jtx->ntype);
    EXPECT_EQ(acc.ai2str(), sender["address"]);
    EXPECT_EQ(acc.vk2str(), sender["viewkey"]);
    EXPECT_EQ(acc.sk2str(), sender["spendkey"]);

    EXPECT_FALSE(acc.ai().is_subaddress);

    EXPECT_TRUE(acc);
}

TEST(ACCOUNT, FullConstructionFromStrings)
{
    auto jtx = construct_jsontx("ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2");

    ASSERT_TRUE(jtx);

    auto const& sender = jtx->jtx["sender"];

    Account acc {jtx->ntype,
                sender["address"],
                sender["viewkey"],
                sender["spendkey"]};

    EXPECT_EQ(acc.nt(), jtx->ntype);
    EXPECT_EQ(acc.ai().address, jtx->sender.address.address);
    EXPECT_EQ(acc.vk(), jtx->sender.viewkey);
    EXPECT_EQ(acc.sk(), jtx->sender.spendkey);
    EXPECT_EQ(acc.type(), Account::ADDRESS_TYPE::PRIMARY);
}

TEST(ACCOUNT, NoSpendandViewKeiesConstruction)
{
    auto jtx = construct_jsontx("ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2");

    ASSERT_TRUE(jtx);

    auto const& sender = jtx->jtx["sender"];

    Account acc {jtx->ntype,
                sender["address"],
                sender["viewkey"]};

    EXPECT_EQ(acc.nt(), jtx->ntype);
    EXPECT_EQ(acc.ai().address, jtx->sender.address.address);
    EXPECT_EQ(acc.vk(), jtx->sender.viewkey);
    EXPECT_FALSE(acc.sk());


    Account acc2 {jtx->ntype,
                sender["address"]};

    EXPECT_EQ(acc2.nt(), jtx->ntype);
    EXPECT_EQ(acc2.ai().address, jtx->sender.address.address);
    EXPECT_FALSE(acc2.vk());
    EXPECT_FALSE(acc2.sk());
}

TEST(ACCOUNT, FullConstructionSubAddress)
{
    auto jtx = construct_jsontx("d7dcb2daa64b5718dad71778112d48ad62f4d5f54337037c420cb76efdd8a21c");

    ASSERT_TRUE(jtx);

    auto const& recipient1 = jtx->recipients[1];

    Account acc {jtx->ntype,
                recipient1.address,
                recipient1.viewkey,
                recipient1.spendkey};

    auto const& jrecipient = jtx->jtx["recipient"][1];

    EXPECT_EQ(acc.nt(), jtx->ntype);
    EXPECT_EQ(acc.ai2str(), jrecipient["address"]);
    EXPECT_EQ(acc.vk2str(), jrecipient["viewkey"]);
    EXPECT_EQ(acc.sk2str(), jrecipient["spendkey"]);
    EXPECT_EQ(acc.type(), Account::ADDRESS_TYPE::SUBADDRRES);

    EXPECT_TRUE(acc.ai().is_subaddress);
    EXPECT_TRUE(acc);
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
