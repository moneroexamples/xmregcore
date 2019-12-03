//
// Created by mwo on 5/11/15.
//

#pragma once

#include "monero_headers.h"

#include <boost/optional.hpp>

#include <string>
#include <vector>

/**
 * Some helper functions that might or might not be useful in this project.
 * Names are rather self-explanatory, so I think
 * there is no reason for any detailed explanations here
 */
namespace xmreg
{

using namespace cryptonote;
using namespace crypto;
using namespace std;

using  epee::string_tools::pod_to_hex;
using  epee::string_tools::hex_to_pod;

constexpr network_type network_types[]
            = {network_type::MAINNET,
               network_type::TESTNET,
               network_type::STAGENET};

enum class address_type : uint8_t
{
    REGULAR = 0, INTEGRATED, SUBADDRESS, UNDEFINED = 255
};

string
get_default_lmdb_folder(network_type nettype = network_type::MAINNET);

bool
get_blockchain_path(string& blockchain_path,
                    network_type nettype = network_type::MAINNET);

template <typename T>
bool
parse_str_secret_key(const string& key_str, T& secret_key);

bool
parse_str_address(const string& address_str,
                  address_parse_info& address_info,
                  cryptonote::network_type nettype = cryptonote::network_type::MAINNET);


bool
addr_and_viewkey_from_string(string const& addres_str,
                             string const& viewkey_str,
                             network_type net_type,
                             address_parse_info& address_info,
                             crypto::secret_key& viewkey);

bool
hex_to_tx(string const& tx_hex, transaction& tx,
          crypto::hash& tx_hash,  crypto::hash& tx_prefix_hash);

template <typename F>
//requires F to be callable
void
for_each_network_type(F f)
{
    for (const auto& nt: network_types)
        f(nt);
}

pair<network_type, address_type>
nettype_based_on_address(string const& address);

boost::optional<subaddress_index>
parse_subaddress_index(string idx_str);


string
remove_trailing_path_separator(const string& in_path);

}
