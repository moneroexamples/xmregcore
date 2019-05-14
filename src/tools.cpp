//
// Created by marcin on 5/11/15.
//

#include "tools.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>


namespace xmreg
{


namespace bf = boost::filesystem;


string
get_default_lmdb_folder(network_type nettype)
{
    // default path to monero folder
    // on linux this is /home/<username>/.bitmonero
    string default_monero_dir = tools::get_default_data_dir();

    if (nettype == cryptonote::network_type::TESTNET)
        default_monero_dir += "/testnet";
    if (nettype == cryptonote::network_type::STAGENET)
        default_monero_dir += "/stagenet";


    // the default folder of the lmdb blockchain database
    // is therefore as follows
    return default_monero_dir + string("/lmdb");
}

/**
 * Check if a character is a path seprator
 */
inline bool
is_separator(char c)
{
    // default linux path separator
    const char separator = '/';

    return c == separator;
}


/**
 * Remove trailinig path separator.
 */
string
remove_trailing_path_separator(const string& in_path)
{
    string new_string = in_path;
    if (!new_string.empty() && is_separator(new_string[new_string.size() - 1]))
        new_string.erase(new_string.size() - 1);
    return new_string;
}

bool
get_blockchain_path(string& blockchain_path,
                    cryptonote::network_type nettype)
{
    // the default folder of the lmdb blockchain database
    string default_lmdb_dir   = xmreg::get_default_lmdb_folder(nettype);

    blockchain_path = !blockchain_path.empty()
                      ? blockchain_path
                      : default_lmdb_dir;

    if (!bf::is_directory(blockchain_path))
    {
        cerr << "Given path \"" << blockchain_path   << "\" "
             << "is not a folder or does not exist \n";

        return false;
    }

    blockchain_path = xmreg::remove_trailing_path_separator(blockchain_path);

    return true;
}


bool
parse_str_address(const string& address_str,
                  address_parse_info& address_info,
                  network_type net_type)
{

    if (!get_account_address_from_str(address_info, net_type, address_str))
    {
        //cerr << "Error getting address: " << address_str << '\n';
        return false;
    }

    return true;
}


/**
 * Parse key string, e.g., a viewkey in a string
 * into crypto::secret_key or crypto::public_key
 * depending on the template argument.
 */
template <typename T>
bool
parse_str_secret_key(const string& key_str, T& secret_key)
{

    // hash and keys have same structure, so to parse string of
    // a key, e.g., a view key, we can first parse it into the hash
    // object using parse_hash256 function, and then copy the reslting
    // hash data into secret key.
    crypto::hash hash_;

    if(!parse_hash256(key_str, hash_))
    {
        cerr << "Cant parse a key (e.g. viewkey): " << key_str << endl;
        return false;
    }

    // crypto::hash and crypto::secret_key have basicly same
    // structure. They both keep they key/hash as c-style char array
    // of fixed size. Thus we can just copy data from hash
    // to key
    copy(begin(hash_.data), end(hash_.data), secret_key.data);

    return true;
}

// explicit instantiations of get template function
template bool parse_str_secret_key<crypto::secret_key>(const string& key_str, crypto::secret_key& secret_key);
template bool parse_str_secret_key<crypto::public_key>(const string& key_str, crypto::public_key& secret_key);
template bool parse_str_secret_key<crypto::hash>(const string& key_str, crypto::hash& secret_key);
template bool parse_str_secret_key<crypto::hash8>(const string& key_str, crypto::hash8& secret_key);


bool
addr_and_viewkey_from_string(string const& addres_str,
                             string const& viewkey_str,
                             network_type net_type,
                             address_parse_info& address,
                             crypto::secret_key& viewkey)
{
    if (!parse_str_address(addres_str, address, net_type))
        return false;

    if (!parse_str_secret_key(viewkey_str, viewkey))
          return false;

    return true;
}

bool
hex_to_tx(string const& tx_hex,
          transaction& tx,
          crypto::hash& tx_hash,
          crypto::hash& tx_prefix_hash)
{
    std::string tx_blob;

    epee::string_tools::parse_hexstr_to_binbuff(tx_hex, tx_blob);

    return parse_and_validate_tx_from_blob(tx_blob, tx, tx_hash, tx_prefix_hash);
}


pair<network_type, address_type>
nettype_based_on_address(string const& address)
{

    network_type determined_network_type {network_type::UNDEFINED};
    address_type determined_address_type {address_type::UNDEFINED};

    for_each_network_type([&address,
                          &determined_network_type,
                          &determined_address_type]
                          (network_type nt)
    {

       uint64_t address_prefix = get_config(nt)
               .CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
       uint64_t integrated_address_prefix = get_config(nt)
               .CRYPTONOTE_PUBLIC_INTEGRATED_ADDRESS_BASE58_PREFIX;
       uint64_t subaddress_prefix = get_config(nt)
               .CRYPTONOTE_PUBLIC_SUBADDRESS_BASE58_PREFIX;

       blobdata data;
       uint64_t prefix;

       if (!tools::base58::decode_addr(address, prefix, data))
       {
         cerr << "Invalid address format\n";
         return;
       }

       if (address_prefix == prefix)
       {
           determined_address_type = address_type::REGULAR;
           determined_network_type = nt;
           return;

       }
       else if (integrated_address_prefix == prefix)
       {
           determined_address_type = address_type::INTEGRATED;
           determined_network_type = nt;
           return;
       }
       else if (subaddress_prefix == prefix)
       {
           determined_address_type = address_type::SUBADDRESS;
           determined_network_type = nt;
           return;
       }

    });

    return {determined_network_type, determined_address_type};
}


boost::optional<subaddress_index>
parse_subaddress_index(string idx_str)
{
    vector<string> split_index;

    boost::split(split_index, idx_str, 
                 [](char c){return c == ',' || c == '/';});
    
    if (split_index.empty() 
            || split_index.size() != 2)
    {
        cerr << "Incorrect subaddress index given: "
              << idx_str << '\n';
        return {};
    }

    try 
    {
        auto idx_major 
            = boost::lexical_cast<uint32_t>(split_index[0]);
        auto idx_minor 
            = boost::lexical_cast<uint32_t>(split_index[1]);

        return subaddress_index {idx_major, idx_minor};

    }
    catch (boost::bad_lexical_cast const& e)
    {
        cerr << e.what() << '\n';
    }
    
    return {};
}



}
