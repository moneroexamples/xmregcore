#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../src/Account.h"

#include "mocks.h"


namespace
{

using namespace xmreg;

class NetTypeDetermination :
        public ::testing::TestWithParam<
            pair<string, pair<network_type, address_type>>
        >
{

};

TEST_P(NetTypeDetermination, nettype_based_on_address)
{
    auto const& a_pair = GetParam();

    pair<network_type, address_type> nt = nettype_based_on_address(a_pair.first);
    EXPECT_EQ(nt.first, a_pair.second.first);
    EXPECT_EQ(nt.second, a_pair.second.second);
}


INSTANTIATE_TEST_CASE_P(nettype_based_on_address, NetTypeDetermination, ::testing::Values(
  make_pair("44AFFq5kSiGBoZ4NMDwYtN18obc8AemS33DBLWs3H7otXft3XjrpDtQGv7SqSsaBYBb98uNbr2VBBEt7f2wfn3RVGQBEP3A",
            make_pair(network_type::MAINNET, address_type::REGULAR)),
  make_pair("4Li3eYCMpkTLR4AT5GFwNRW45NL4hssGvawBJwKNWRKdQyMH1w2QHYT24X9g6xwXYTEdpmcU2bp5c7d3e6A9hc2aCgYUBqGZWUAUC9TWVx",
            make_pair(network_type::MAINNET, address_type::INTEGRATED)),
  make_pair("8AjZVBsAkdufJCbuasBLjXCgorUwg5cEBRkqC9JYa9VvMjTKitYSCRFBrQTnL4SgmfJvHSn3Uo7f2JEBXGn2xh89VDGfCnJ",
            make_pair(network_type::MAINNET, address_type::SUBADDRESS)),

  make_pair("9wUf8UcPUtb2huK7RphBw5PFCyKosKxqtGxbcKBDnzTCPrdNfJjLjtuht87zhTgsffCB21qmjxjj18Pw7cBnRctcKHrUB7N",
            make_pair(network_type::TESTNET, address_type::REGULAR)),
  make_pair("A7BL9HRt6A72huK7RphBw5PFCyKosKxqtGxbcKBDnzTCPrdNfJjLjtuht87zhTgsffCB21qmjxjj18Pw7cBnRctcU7jB41HjTfi8iRUdf3",
            make_pair(network_type::TESTNET, address_type::INTEGRATED)),
  make_pair("Bg64aV8pKpVgu8q45poGvpYfmrRm1pq1FibQ9n2j7Nu8YaF1AR1Pxcy6uZqZhQeU8fci2LRgxA4yPbvdHbCibXwzTTFgjNy",
            make_pair(network_type::TESTNET, address_type::SUBADDRESS)),

  make_pair("56heRv2ANffW1Py2kBkJDy8xnWqZsSrgjLygwjua2xc8Wbksead1NK1ehaYpjQhymGK4S8NPL9eLuJ16CuEJDag8Hq3RbPV",
            make_pair(network_type::STAGENET, address_type::REGULAR)),
  make_pair("78LbLrVuGpjWXFfazxJhP9RkEaKFoUgMvRhuAoEeeWvti4rQUQvNLRLW9NQyZAQ9KW3AzZfxYsfojFVJQbE8G1Kh7RxRPLW",
            make_pair(network_type::STAGENET, address_type::SUBADDRESS)),
  make_pair("5FGGRSBHAYuFS8pmrhHN5jMpgJwnnTXpTDmmM5wkrBBx4xD6aEnpZq7dPkeDeWs67TV9HunDQtT3qF2UGYWzGGxq5Q8b9ruDV7NPNkCXrA",
            make_pair(network_type::STAGENET, address_type::INTEGRATED)),

  make_pair("wrongaddress", make_pair(network_type::UNDEFINED, address_type::UNDEFINED))
));

}
