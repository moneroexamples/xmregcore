#ifndef JSONTX_H
#define JSONTX_H

#include "../src/tools.h"
#include "../ext/json.hpp"


namespace xmreg
{

using json = nlohmann::json;


/**
 * @brief reprsents txs found in tests/res/tx folder
 *
 * Usef for unit testing and mocking tx data, which
 * otherwise would need to be fetched from blockchain
 *
*/
class JsonTx
{
public:

    struct output
    {
        uint64_t index {0};
        public_key pub_key;
        uint64_t amount {0};

        bool 
        operator==(output const& other) const
        {
            return index == other.index 
                && pub_key == other.pub_key
                && amount == other.amount;
        }
    };
    
    struct input 
    {
        key_image key_img;
        uint64_t amount;
        public_key out_pub_key;
        
        bool 
        operator==(input const& other) const
        {
            return key_img == other.key_img
                && amount == other.amount
                && out_pub_key == other.out_pub_key;
        }
    };

    struct account
    {
        address_parse_info address {};
        secret_key viewkey {};
        secret_key spendkey {};
        bool is_subaddress {false};
        network_type ntype;
        uint64_t amount {0};
        uint64_t change {0};
        vector<output> outputs;
        vector<input> inputs;
        boost::optional<subaddress_index> subaddr_idx; 

        inline string
        address_str() const
        {
            return get_account_address_as_str(ntype,
                                              is_subaddress,
                                              address.address);
        }

        inline account_keys
        get_account_keys() const
        {
            return {address.address, spendkey, viewkey};
        }

        inline json
        get_addr_viewkey_as_json() const 
        {
            return {{"address", address_str()}, 
                    {"viewkey", pod_to_hex(viewkey)}};
        }

        friend std::ostream&
        operator<<(std::ostream& os, account const& _account);
    };

    json jtx;

    transaction tx;
    crypto::hash tx_hash {0};                                                 
    crypto::hash tx_prefix_hash {0};
    crypto::public_key tx_pub_key;
    crypto::hash payment_id {0};
    crypto::hash8 payment_id8 {0};
    crypto::hash8 payment_id8e {0};
    
    // starting from monero v0.14.0.0,
    // txs can have dummy encrypted payments id
    // you know if they are dummy, if after decryption
    // they are null
    bool is_payment_id8_real {false};

    uint64_t fee {0};

    string jpath;
    network_type ntype;

    account sender;
    vector<account> recipients;

    explicit JsonTx(json _jtx);
    explicit JsonTx(string _path);

    // generate output which normaly is produced by
    // monero's get_output_tx_and_index and blockchain,
    // but here we use data from the tx json data file
    // need this for mocking blockchain calls in unit tests
    void
    get_output_tx_and_index(
            uint64_t const& amount,
            vector<uint64_t> const& offsets,
            vector<tx_out_index>& indices) const;


    // will use data from the json file
    // to set tx. Used for mocking this operation
    // that is normaly done using blockchain
    bool
    get_tx(crypto::hash const& tx_hash,
           transaction& tx) const;

    // used for mocking get_output_key
    // that normally will fetch data from
    // the blockchain
    void
    get_output_key(uint64_t amount,
                   vector<uint64_t> const& absolute_offsets,
                   vector<cryptonote::output_data_t>& outputs);

private:
    void init();
    bool read_config();
    void populate_outputs(json const& joutputs, vector<output>& outs);
    void populate_inputs(json const& jinputs, vector<input>& ins);

};

inline std::ostream&
operator<<(std::ostream& os, JsonTx::account const& _account)
{
    return os << _account.address_str();
}

inline std::ostream&
operator<<(std::ostream& os, JsonTx::output const& _output)
{
    return os << _output.index << ',' 
              << pod_to_hex(_output.pub_key) << ','
              << _output.amount;
}

inline std::ostream&
operator<<(std::ostream& os, vector<JsonTx::output> const& _outputs)
{
    for (auto& output: _outputs)
    {
       os << output << '\n';
    }

    return os;
}

bool
check_and_adjust_path(string& in_path);


boost::optional<JsonTx>
construct_jsontx(string tx_hash, string in_dir = {});

}


#endif // JSONTX_H
