#pragma once

#include "monero_headers.h"
#include "tools.h"

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

    enum ADDRESS_TYPE {NONE, PRIMARY, SUBADDRESS};

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

    virtual ADDRESS_TYPE type() const = 0;

    inline bool is_subaddress() const
    {return type() == SUBADDRESS;}

    inline auto const& ai() const
    {return addr_info;}

    inline auto ai2str() const
    {return ai_to_str(addr_info, nettype);}

    inline auto const& vk() const
    {return viewkey;}
    
    inline auto vk2str() const
    {return viewkey ? pod_to_hex(*viewkey) : ""s;}

    inline auto const& pvk() const
    {return addr_info.address.m_view_public_key;}
    
    inline auto pvk2str() const
    {return pod_to_hex(pvk());};
    
	inline auto const& psk() const
    {return addr_info.address.m_spend_public_key;}
    
    inline auto psk2str() const
    {return pod_to_hex(psk());};

    inline auto const& sk() const
    {return spendkey;}

    inline auto index() const
    {return subaddr_idx;}

    inline auto const& keys()
    {
        if (!acc_keys)
        {
            assert(bool{viewkey});

            account_keys akeys;
            akeys.m_account_address = ai().address;
            akeys.m_view_secret_key = *viewkey;

            if (spendkey)
                akeys.m_spend_secret_key = *spendkey;

            acc_keys = std::move(akeys);
        }

        return acc_keys;
    }
    
    inline void set_index(subaddress_index idx) 
    {subaddr_idx = std::move(idx);}

    inline auto sk2str() const
    {return spendkey ? pod_to_hex(*spendkey) : ""s;}

    inline auto nt() const
    {return nettype;}

    explicit operator bool() const
    {return type() != NONE;}

    static inline string
    ai_to_str(address_parse_info const& addr_info,
              network_type net_type);

    static inline secret_key
    parse_secret_key(string const& sk);

    friend std::ostream&
    operator<<(std::ostream& os, Account const& _acc);

    virtual ~Account() = default;

protected:

    network_type nettype {network_type::STAGENET};
    address_parse_info addr_info {};
    boost::optional<secret_key> viewkey;
    boost::optional<secret_key> spendkey;
    boost::optional<subaddress_index> subaddr_idx;
    boost::optional<account_keys> acc_keys;
};


// for now subclassess of the Account don't do much.
// but with time it is expected to change and
// and new functionality dependent on whether
// we have primary or subaddress to be added
// to the respective subclasses.

class EmptyAccount : public Account
{
public:
    using Account::Account;

    virtual ADDRESS_TYPE type() const override
    {return NONE;}
};

class SubaddressAccount : public Account
{
public:

    using Account::Account;
    
    virtual inline ADDRESS_TYPE type() const override
    {return SUBADDRESS;}
};


class PrimaryAccount : public Account
{
public:

    static constexpr uint32_t SUBADDRESS_LOOKAHEAD_MAJOR {50};
    static constexpr uint32_t SUBADDRESS_LOOKAHEAD_MINOR {200};

    using subaddr_map_t =  std::unordered_map<
                    public_key,
                    subaddress_index>;

    template <typename... T>
    PrimaryAccount(T&&... args)
    : Account(std::forward<T>(args)...)
    {
        subaddr_idx = subaddress_index {0, 0};
		
		// register the PrimaryAccount into
		// subaddresses map as special case
		// for uniform handling of all addresses
		subaddresses[psk()] = *subaddr_idx;
    }

    virtual ADDRESS_TYPE type() const override
    {return PRIMARY;}
    
    std::unique_ptr<SubaddressAccount>
    gen_subaddress(subaddress_index idx);
    
    std::unique_ptr<SubaddressAccount>
    gen_subaddress(uint32_t acc_id, uint32_t addr_id)
    {
        return gen_subaddress({acc_id, addr_id});
    }

    unique_ptr<subaddress_index>
    has_subaddress(public_key const& pub_spend_key)
    {
        auto it = subaddresses.find(pub_spend_key);
        
        if (it == subaddresses.end())
            return nullptr;

        return make_unique<subaddress_index>(it->second);
    }

    auto get_next_subbaddress_acc_id() const 
    {
        return next_acc_id_to_populate;
    }

    auto const& get_subaddress_map() const
    {
        return subaddresses;
    }

    /** 
     * expand subaddress map by to new_acc_id
     */
    void
    expand_subaddresses(uint32_t new_acc_id);

    /**
     * Unlike above, it does not produce SubaddressAcount 
     * It just calcualtes public spend key for a subaddress
     * with given index and saves it in subaddresses map
     */
    subaddr_map_t::const_iterator 
    add_subaddress_index(uint32_t acc_id, uint32_t addr_id);

    /**
     * Generates all set public spend keys for 
     * 50 accounts x 200 subaddresess into 
     * subaddresses map
     */
    void 
    populate_subaddress_indices(
            uint32_t start_acc_id = 0,
            uint32_t last_acc_id = SUBADDRESS_LOOKAHEAD_MAJOR);

	auto begin() { return subaddresses.begin(); }
    auto begin() const { return subaddresses.cbegin(); }
	
	auto end() { return subaddresses.end(); }
    auto end() const { return subaddresses.cend(); }

private:
    subaddr_map_t subaddresses; 
    uint32_t next_acc_id_to_populate {0};
};

// account_factory functions are helper functions
// to easly create Account objects through uniqute_ptr

static unique_ptr<Account>
make_account()
{
    return make_unique<EmptyAccount>();
}

template <typename... T>
static unique_ptr<Account>
make_account(string const& addr_str,
                T&&... args)
{
    auto&& net_and_addr_type 
        = nettype_based_on_address(addr_str);

    if (net_and_addr_type.first == network_type::UNDEFINED)
    {
        return nullptr;
    }

    if (net_and_addr_type.second == address_type::SUBADDRESS)
        return make_unique<SubaddressAccount>(
                net_and_addr_type.first,
                addr_str,
                std::forward<T>(args)...);
    else if (net_and_addr_type.second == address_type::REGULAR
              || net_and_addr_type.second == address_type::INTEGRATED)
        return make_unique<PrimaryAccount>(
                net_and_addr_type.first,
                addr_str,
                std::forward<T>(args)...);

    return nullptr;
}

template <typename... T>
static unique_ptr<Account>
make_account(network_type net_type,
                address_parse_info const& addr_info,
                T&&... args)
{
    if (!crypto::check_key(addr_info.address.m_view_public_key)
            || !crypto::check_key(addr_info.address.m_spend_public_key))
    return nullptr;

    if (addr_info.is_subaddress)
        return make_unique<SubaddressAccount>(
                net_type, addr_info,
                std::forward<T>(args)...);
    else
        return make_unique<PrimaryAccount>(
                net_type, addr_info,
                std::forward<T>(args)...);
}

template <typename... T>
static unique_ptr<Account>
make_account(subaddress_index idx, T&&... args)
{
    auto acc = make_account(std::forward<T>(args)...); 

    if (!acc)
        return nullptr;

    if (acc->is_subaddress())
        acc->set_index(std::move(idx));

    return acc;
}

template <typename... T>
static unique_ptr<PrimaryAccount>
make_primaryaccount(string const& addr_str,
                    T&&... args)
{
    auto&& net_and_addr_type 
        = nettype_based_on_address(addr_str);

    if (net_and_addr_type.first == network_type::UNDEFINED)
    {
        return nullptr;
    }

    if (net_and_addr_type.second == address_type::REGULAR
              || net_and_addr_type.second == address_type::INTEGRATED)
    {
        auto pacc = make_unique<PrimaryAccount>(
                    net_and_addr_type.first,
                    addr_str,
                    std::forward<T>(args)...);

        pacc->populate_subaddress_indices();

        return pacc;
    }

    return nullptr;
}

template <typename... T>
static unique_ptr<Account>
make_primaryaccount(network_type net_type,
                    address_parse_info const& addr_info,
                    T&&... args)
{
    if (!crypto::check_key(addr_info.address.m_view_public_key)
            || !crypto::check_key(addr_info.address.m_spend_public_key))
    return nullptr;

    return make_unique<PrimaryAccount>(
                net_type, addr_info,
                std::forward<T>(args)...);
}

static unique_ptr<PrimaryAccount>
make_primaryaccount(std::unique_ptr<Account>&& acc)
{
    if (acc->type() != Account::PRIMARY)
    {
        return nullptr;
    }

    PrimaryAccount* p = static_cast<PrimaryAccount*>(
                         acc.release());

    unique_ptr<PrimaryAccount> pacc(p);

    pacc->populate_subaddress_indices();

    return pacc;
}


 unique_ptr<SubaddressAccount> 
 create(PrimaryAccount const& acc, subaddress_index idx);

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

    string subaddr_str {"n/a"};

    if (_acc.subaddr_idx)
    {
        stringstream ss;

        ss << *_acc.subaddr_idx;

        subaddr_str = ss.str(); 
    }

    string spendkey_str {"N/A"};

    if (_acc.sk())
    {
        spendkey_str = _acc.sk2str();
    }

    return os << "nt:" << static_cast<size_t>(_acc.nettype)
              << ","   << subaddr_str   
              << ",a:" << _acc.ai2str()
              << ",v:" << _acc.vk2str()
              << ",s:" << spendkey_str;
}

}

