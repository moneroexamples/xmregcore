#pragma once

#include "MicroCore.h"
#include "Account.h"

#include <tuple>
#include <utility>

namespace xmreg
{

using  epee::string_tools::pod_to_hex;
using  epee::string_tools::hex_to_pod;

using namespace std;


public_key
get_tx_pub_key_from_received_outs(transaction const& tx);


class AbstractIdentifier
{
public:
    virtual void identify(transaction const& tx,
                          public_key const& tx_pub_key,
                          vector<public_key> const& additional_tx_pub_keys
                                = vector<public_key>{}) = 0;
};


class BaseIdentifier : public AbstractIdentifier
{
public:
    BaseIdentifier(
            address_parse_info const* _address,
            secret_key const* _viewkey)
        : address_info {_address}, viewkey {_viewkey},
          hwdev {hw::get_device("default")}
   {}

   BaseIdentifier(Account* _acc)
        : BaseIdentifier(&_acc->ai(), &*_acc->vk())
   {
       acc = _acc; 
   }

    virtual void identify(transaction const& tx,
                          public_key const& tx_pub_key,
                          vector<public_key> const& additional_tx_pub_keys
                                = vector<public_key>{}) = 0;

    inline auto get_address() const {return address_info;}
    inline auto get_viewkey() const {return viewkey;}
    inline auto get_total() const {return total_xmr;}

    virtual ~BaseIdentifier() = default;

protected:
    address_parse_info const* address_info {nullptr};
    secret_key const* viewkey {nullptr};
    uint64_t total_xmr {0};
    Account* acc {nullptr};
    hw::device& hwdev;
};

/**
 * @brief The Output class identifies our
 * outputs in a given tx
 */
class Output : public BaseIdentifier
{
public:

    using BaseIdentifier::BaseIdentifier;

    void identify(transaction const& tx,
                  public_key const& tx_pub_key,
                  vector<public_key> const& additional_tx_pub_keys
                        = vector<public_key>{}) override;

    inline auto get() const
    {
        return identified_outputs;
    }


    bool
    decode_ringct(rct::rctSig const& rv,
                  crypto::key_derivation const& derivation,
                  unsigned int i,
                  rct::key& mask,
                  uint64_t& amount) const;

    struct info
    {
        public_key pub_key;
        uint64_t   amount;
        uint64_t   idx_in_tx;
        key_derivation derivation;
        rct::key   rtc_outpk;
        rct::key   rtc_mask;
        rct::key   rtc_amount;

        public_key subaddress_spendkey;
        subaddress_index subaddr_idx {
            UINT32_MAX, UINT32_MAX};
            // the max value means not given
        
        bool has_subaddress_index() const
        {
            return subaddr_idx.major != UINT32_MAX
                && subaddr_idx.minor != UINT32_MAX;
        }
            
        friend std::ostream& operator<<(std::ostream& os,
                                        info const& _info);
    };

protected:

    uint64_t total_received {0};
    vector<info> identified_outputs;
};

/**
 * @brief The Input class identifies our possible
 * inputs (key images) in a given tx
 */
class Input : public BaseIdentifier
{
public:
                                         //output_pubk, amount
    using known_outputs_t = unordered_map<public_key, uint64_t>;

    Input(address_parse_info const* _a,
           secret_key const* _viewkey,
           known_outputs_t const* _known_outputs,
           AbstractCore const* _mcore)
        : BaseIdentifier(_a, _viewkey),          
          known_outputs {_known_outputs},
          mcore {_mcore}
    {}
    
    Input(Account* _acc,
            known_outputs_t const* _known_outputs,
           AbstractCore const* _mcore)
        : BaseIdentifier(_acc),          
          known_outputs {_known_outputs},
          mcore {_mcore}
    {}

    void identify(transaction const& tx,
                  public_key const& tx_pub_key,
                  vector<public_key> const& additional_tx_pub_keys
                        = vector<public_key>{}) override;

    inline auto get() const
    {
        return identified_inputs;
    }

    bool
    generate_key_image(const crypto::key_derivation& derivation,
                      const std::size_t output_index,
                      const crypto::secret_key& sec_key,
                      const crypto::public_key& pub_key,
                      crypto::key_image& key_img) const;

    struct info
    {
        key_image key_img;
        uint64_t amount;
        public_key out_pub_key;

        friend std::ostream& operator<<(std::ostream& os,
                                        info const& _info);
    };


protected:

    secret_key const* viewkey {nullptr};   
    known_outputs_t const* known_outputs {nullptr};
    AbstractCore const* mcore {nullptr};
    vector<info> identified_inputs;
};

/**
 * Uses only viewkey to "guess" possible
 * inputs. These are only guessess, because
 * without spendkey it is not possible to know
 * for sure which key images were generated from
 * which outputs.
 */
class GuessInput : public Input
{
public:
    GuessInput(address_parse_info const* _a,
               secret_key const* _viewkey,
               MicroCore* _mcore)
        : Input(_a, _viewkey, nullptr, _mcore)
    {}
    
    GuessInput(Account* _acc, MicroCore* _mcore)
        : Input(_acc, nullptr, _mcore)
    {}

    void identify(transaction const& tx,
                  public_key const& tx_pub_key,
                  vector<public_key> const& additional_tx_pub_keys
                        = vector<public_key>{}) override;

};

/**
 * Spendkey is optional. But if we have it,
 * we can for sure determine which key images
 * are ours or not. This is especially useful
 * in unit testing, since we can compare wether
 * guessed key images do contain all our key images
 */
class RealInput : public Input
{

public:

    RealInput(address_parse_info const* _a,
              secret_key const* _viewkey,
              secret_key const* _spendkey,
              MicroCore* _mcore)
        : Input(_a, _viewkey, nullptr, _mcore),
          spendkey {_spendkey}
    {}
    
    RealInput(Account* _acc, MicroCore* _mcore)
        : Input(_acc, nullptr, _mcore)
    {
        assert(_acc->sk());
        spendkey = &(*_acc->sk());
    }

    void identify(transaction const& tx,
                  public_key const& tx_pub_key,
                  vector<public_key> const& additional_tx_pub_keys
                        = vector<public_key>{}) override;


protected:
    secret_key const* spendkey {nullptr};
};


template <typename HashT>
class PaymentID : public BaseIdentifier
{

public:

    using payments_t = tuple<crypto::hash, crypto::hash8>;
    
    PaymentID()
        : BaseIdentifier(nullptr, nullptr)
    {}

    PaymentID(address_parse_info const* _address,
              secret_key const* _viewkey)
        : BaseIdentifier(_address, _viewkey)
    {}
    
    PaymentID(Account* _acc)
        : BaseIdentifier(_acc)
    {}

    void identify(transaction const& tx,
                  public_key const& tx_pub_key,
                  vector<public_key> const& additional_tx_pub_keys
                        = vector<public_key>{}) override
    {   
        // get payment id. by default we are intrested
        // in short ids from integrated addresses
        payment_id_tuple = get_payment_id(tx);

        payment_id = std::get<HashT>(payment_id_tuple);
        
        //cout << "payment_id_found: " 
             //<< pod_to_hex(*payment_id) << endl;

        // if no payment_id found, return
        if (*payment_id == null_hash)
        {
            payment_id = boost::none;
            return;
        }
        
        // if no viewkey and we have integrated payment id
        if (get_viewkey() == nullptr 
                && sizeof(*payment_id) == sizeof(crypto::hash8))
        {
            payment_id = boost::none;
            return;
        }

        // decrypt integrated payment id. if its legacy payment id
        // nothing will happen.
        if (!decrypt(*payment_id, tx_pub_key))
        {
            throw std::runtime_error("Cant decrypt pay_id: "
                                     + pod_to_hex(payment_id));
        }


    }

    payments_t
    get_payment_id(transaction const& tx) const;

    inline bool
    decrypt(crypto::hash& p_id,
            public_key const& tx_pub_key) const
    {
        // don't need to do anything for legacy payment ids
        return true;
    }

    inline bool
    decrypt(crypto::hash8& p_id,
            public_key const& tx_pub_key) const
    {
        // overload for short payment id,
        // the we are going to decrypt
        return encrypt_payment_id(
                      p_id, tx_pub_key, *get_viewkey());
    }

    bool
    encrypt_payment_id(crypto::hash8& payment_id,
                       crypto::public_key const& public_key,
                       crypto::secret_key const& secret_key) const;

    inline auto get() const {return payment_id;}

    // for legacy payment id this will be same as get()
    // for integrated id, this will be encrypted version of the id
    // while the get() will return decrypted short payment id
    inline HashT raw() const
    {return std::get<HashT>(payment_id_tuple);}

private:
    boost::optional<HashT> payment_id;
    HashT null_hash {};
    payments_t payment_id_tuple;
};

using LegacyPaymentID = PaymentID<crypto::hash>;
using IntegratedPaymentID = PaymentID<crypto::hash8>;


template<typename... T>
class ModularIdentifier
{
public:
    tuple<unique_ptr<T>...> identifiers;

    ModularIdentifier(transaction const& _tx,
                      unique_ptr<T>... args)
        : identifiers {move(args)...},
          tx {_tx}
    {
        // having tx public key is very common for all identifiers
        // so we can get it here, instead of just obtaining it
        // for each identifier seprately
        tx_pub_key = get_tx_pub_key_from_received_outs(tx);

        // multi-output txs can have some additional public keys
        // in the extra field. So we also get them, just in case
        additional_tx_pub_keys = get_additional_tx_pub_keys_from_extra(tx);
    }

    void identify()
    {
         auto b = {(std::get<unique_ptr<T>>(
                        identifiers)->identify(
                            tx, tx_pub_key, additional_tx_pub_keys),
                   true)...};
         (void) b;
    }

     // overload to get value from tuple by type
    template <typename U>
    auto* const get() const
    {
        return std::get<unique_ptr<U>>(identifiers).get();
    }


    // overload to get value from tuple by number
    template <size_t No>
    auto* const get() const
    {
        return std::get<No>(identifiers).get();
    }

    inline auto get_tx_pub_key() const {return tx_pub_key;}

private:
    transaction const& tx;
    public_key tx_pub_key;
    vector<public_key> additional_tx_pub_keys;
};

/**
 * A helper function to create ModularIdentifier object
 */
template<typename... T>
auto make_identifier(transaction const& tx, T&&... identifiers)
{
    return ModularIdentifier<typename T::element_type...>(
                tx, std::forward<T>(identifiers)...);
}

template <typename T>
auto
calc_total_xmr(T&& infos)
{
    uint64_t total_xmr {0};

    for (auto const& info: infos)
        total_xmr += info.amount;

    return total_xmr;
}


inline std::ostream&
operator<<(std::ostream& os, xmreg::Output::info const& _info)
{
    os << _info.idx_in_tx << ", "
       << pod_to_hex(_info.pub_key) << ", "
       << _info.amount;

    if (_info.has_subaddress_index())
        os << ", " << _info.subaddr_idx;

    return os;
}

inline std::ostream&
operator<<(std::ostream& os, xmreg::Input::info const& _info)
{
    return os << pod_to_hex(_info.key_img) << ", "
              << pod_to_hex(_info.out_pub_key) << ", "
              << _info.amount;
}

template <typename T>
inline std::ostream&
operator<<(std::ostream& os, std::vector<T> const& _infos)
{
    for (auto&& _info: _infos)
        os << _info << '\n';

    return os;
}


}

