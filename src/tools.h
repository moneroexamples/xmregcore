//
// Created by mwo on 5/11/15.
//

#pragma once

#include "monero_headers.h"

#include "../ext/json.hpp"

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

}
