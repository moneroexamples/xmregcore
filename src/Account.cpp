#include "Account.h"

namespace xmreg
{

Account::Account(
        network_type _nettype,
        address_parse_info const& _addr_info,
        secret_key const& _viewkey,
        secret_key const& _spendkey)
    : nettype {_nettype},
      addr_info {_addr_info},
      viewkey {_viewkey},
      spendkey {_spendkey}
{}

Account::Account(
        network_type _nettype,
        address_parse_info const& _addr_info,
        secret_key const& _viewkey)
    : nettype {_nettype},
      addr_info {_addr_info},
      viewkey {_viewkey}
{}

Account::Account(
        network_type _nettype,
        address_parse_info const& _addr_info)
    : nettype {_nettype},
      addr_info {_addr_info}
{}

Account::Account(network_type _nettype,
                string const& _address,
                string const& _viewkey,
                string const& _spendkey)
    : nettype {_nettype}
{
    if (!get_account_address_from_str(addr_info, nettype, _address))
        throw std::runtime_error("Cant parse address: " + _address);

    if (!_viewkey.empty())
        viewkey = parse_secret_key(_viewkey);

    if (!_spendkey.empty())
        spendkey = parse_secret_key(_spendkey);
}


}
