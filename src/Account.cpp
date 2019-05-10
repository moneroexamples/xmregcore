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
{
}

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

constexpr uint32_t PrimaryAccount::SUBADDRESS_LOOKAHEAD_MAJOR;
constexpr uint32_t PrimaryAccount::SUBADDRESS_LOOKAHEAD_MINOR;

unique_ptr<SubaddressAccount> 
PrimaryAccount::gen_subaddress(subaddress_index idx) 
{
    if (!this->vk() || idx.is_zero())
        return nullptr;

    auto& device = hw::get_device("default");

    address_parse_info subaddr_info {
        device.get_subaddress(
                *(this->keys()), idx),
                 true /*is_subaddress*/,
                 false /*has_payment_id*/};

    std::unique_ptr<SubaddressAccount> sacc;

    if (this->sk())
    {
        sacc = std::make_unique<SubaddressAccount>(
                this->nt(), 
                std::move(subaddr_info), 
                *(this->vk()), *(this->sk()));
    }
    else
    {
        sacc = std::make_unique<SubaddressAccount>(
                this->nt(), 
                std::move(subaddr_info), 
                *(this->vk()));
    }

    sacc->set_index(std::move(idx));

    return sacc;
}

    
PrimaryAccount::subaddr_map_t::const_iterator 
PrimaryAccount::add_subaddress_index(uint32_t acc_id, uint32_t addr_id)
{
    auto& device = hw::get_device("default");

    subaddress_index idx {acc_id, addr_id};

    auto pub_spendkey = device.get_subaddress_spend_public_key(
            *(this->keys()), idx);

    auto it = subaddresses.insert(
                  std::make_pair(std::move(pub_spendkey), 
                      idx));
    return it.first;
}

void 
PrimaryAccount::populate_subaddress_indices(
        uint32_t start_acc_id,
        uint32_t last_acc_id)
{
    auto& device = hw::get_device("default");

    auto const& account_keys = *(this->keys());

    if (start_acc_id == 0)
    {
        // first we populate for account of 0 as we 
        // skip subaddr of 0.
        auto public_keys = device.get_subaddress_spend_public_keys(
               account_keys, 0, 1, SUBADDRESS_LOOKAHEAD_MINOR); 

        for (uint32_t addr_id {1}; 
                addr_id < SUBADDRESS_LOOKAHEAD_MINOR; 
                ++addr_id)
        {
            subaddresses.insert({public_keys[addr_id-1], 
                                {0, addr_id}});
        }
        ++start_acc_id;
    }

    // for all remaning accounts, we generated subaddresses
    // from 0
    for (uint32_t acc_id {start_acc_id}; 
            acc_id < last_acc_id; ++acc_id)
    {
       auto public_keys = device.get_subaddress_spend_public_keys(
               account_keys, acc_id, 0, 
               SUBADDRESS_LOOKAHEAD_MINOR); 

        for (uint32_t addr_id {0}; 
                addr_id < SUBADDRESS_LOOKAHEAD_MINOR; 
                ++addr_id)
        {
            subaddresses.insert({public_keys[addr_id], 
                                {acc_id, addr_id}});
        }
    }

    next_acc_id_to_populate = last_acc_id;
}

void
PrimaryAccount::expand_subaddresses(uint32_t new_acc_id)
{    
    if (new_acc_id < next_acc_id_to_populate)
        return;

    auto start_acc_id = next_acc_id_to_populate;

    populate_subaddress_indices(start_acc_id, new_acc_id);
}
}
