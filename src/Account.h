#pragma once

#include "monero_headers.h"

#include <boost/optional.hpp>

namespace xmreg
{

using  epee::string_tools::pod_to_hex;
using  epee::string_tools::hex_to_pod;

using namespace cryptonote;
using namespace crypto;
using namespace std;

class Account
{
public:

    enum ADDRESS_TYPE {NONE, PRIMARY, SUBADDRRES};

    Account() = default;

    Account(network_type _nettype,
            address_parse_info const& _addr_info,
            secret_key const& _viewkey,
            secret_key const& _spendkey);

    Account(network_type _nettype,
            address_parse_info const& _addr_info,
            secret_key const& _viewkey);

    Account(network_type _nettype,
            address_parse_info const& _addr_info);

    Account(network_type _nettype,
            string const& _addr_info,
            string const& _viewkey,
            string const& _spendkey);

    Account(network_type _nettype,
            string const& _addr_info,
            string const& _viewkey)
        : Account(_nettype, _addr_info, _viewkey, ""s)
    {}

    Account(network_type _nettype,
            string const& _addr_info)
        : Account(_nettype, _addr_info, ""s, ""s)
    {}

    inline auto type() const
    {return address_type;}

    inline auto const& ai() const
    {return addr_info;}

    inline auto ai2str() const
    {return ai_to_str(addr_info, nettype);}

    inline auto const& vk() const
    {return viewkey;}

    inline auto vk2str() const
    {return viewkey ? pod_to_hex(*viewkey) : ""s;}

    inline auto const& sk() const
    {return spendkey;}

    inline auto sk2str() const
    {return spendkey ? pod_to_hex(*spendkey) : ""s;}

    inline auto nt() const
    {return nettype;}

    inline auto is_none() const
    {return address_type == NONE;}

    inline auto is_primary() const
    {return address_type == PRIMARY;}

    inline auto is_subaddress() const
    {return address_type == SUBADDRRES;}

    static inline string
    ai_to_str(address_parse_info const& addr_info,
              network_type net_type);

    static inline secret_key
    parse_secret_key(string const& sk);

    friend std::ostream&
    operator<<(std::ostream& os, Account const& _acc);

    virtual ~Account() = default;

protected:
    ADDRESS_TYPE address_type {NONE};
    network_type nettype {network_type::STAGENET};
    address_parse_info addr_info;
    boost::optional<secret_key> viewkey;
    boost::optional<secret_key> spendkey;

    inline void
    set_address_type()
    {
        address_type = addr_info.is_subaddress
                ? SUBADDRRES : PRIMARY;
    }

};


inline secret_key
Account::parse_secret_key(string const& sk)
{
    secret_key k;

    if (!hex_to_pod(sk, k))
        throw std::runtime_error("Cant parse secret key: " + sk);

    return k;
}


inline string
Account::ai_to_str(address_parse_info const& addr_info,
                     network_type net_type)
{
    return get_account_address_as_str(
                net_type,
                addr_info.is_subaddress,
                addr_info.address);
}


inline std::ostream&
operator<<(std::ostream& os, Account const& _acc)
{
    return os << "nt:" << static_cast<size_t>(_acc.nettype)
              << ",a:" << _acc.ai2str()
              << ",v:" << _acc.vk2str()
              << ",s:" << _acc.sk2str();
}

}

