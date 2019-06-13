#include "JsonTx.h"

namespace xmreg
{

JsonTx::JsonTx(json _jtx): jtx {std::move(_jtx)}
{

}

JsonTx::JsonTx(string _path): jpath {std::move(_path)}
{
    if (!read_config())
    {
        throw std::runtime_error("Cant read " + jpath);
    }

    init();
}


void
JsonTx::get_output_tx_and_index(
        uint64_t const& amount,
        vector<uint64_t> const& offsets,
        vector<tx_out_index>& indices) const
{
    for (auto const& jinput: jtx["inputs"])
    {
        if (jinput["amount"] != amount
                || jinput["absolute_offsets"] != offsets)
            continue;

        for (auto const& jring_member: jinput["ring_members"])
        {
            crypto::hash tx_hash;

            if (!hex_to_pod(jring_member["tx_hash"], tx_hash))
                throw std::runtime_error(
                        "hex_to_pod(jring_member[\"tx_hash\"], tx_hash)");

            indices.emplace_back(tx_hash, jring_member["output_index_in_tx"]);
        }
    }
}

bool
JsonTx::get_tx(crypto::hash const& tx_hash,
               transaction& tx) const
{
    for (auto const& jinput: jtx["inputs"])
    {
        for (auto const& jring_member: jinput["ring_members"])
        {
            if (jring_member["tx_hash"] != pod_to_hex(tx_hash))
                continue;

            crypto::hash tx_hash_tmp;
            crypto::hash tx_prefix_hash_tmp;

            if (!hex_to_tx(jring_member["tx_hex"],
                                   tx, tx_hash_tmp, tx_prefix_hash_tmp))
                throw std::runtime_error(
                        "xmreg::hex_to_tx(jring_member[\"tx_hash\"]");

            (void) tx_hash_tmp;
            (void) tx_prefix_hash_tmp;

            return true;
        }
    }

    return false;
}

void
JsonTx::get_output_key(
           uint64_t amount,
           vector<uint64_t> const& absolute_offsets,
           vector<output_data_t>& outputs)
{
    for (auto const& jinput: jtx["inputs"])
    {
        if (jinput["amount"] != amount
                || jinput["absolute_offsets"] != absolute_offsets)
            continue;

        for (auto const& jring_member: jinput["ring_members"])
        {

            crypto::public_key out_pk;

            if (!hex_to_pod(jring_member["ouput_pk"], out_pk))
                throw std::runtime_error(
                        "hex_to_pod(jring_member[\"ouput_pk\"], out_pk)");


            rct::key commitment;

            if (!hex_to_pod(jring_member["commitment"], commitment))
                throw std::runtime_error(
                        "hex_to_pod(jring_member[\"commitment\"], commitment)");


            outputs.push_back(output_data_t {
                                  out_pk,
                                  jring_member["unlock_time"],
                                  jring_member["height"],
                                  commitment});
        }
    }
}

void
JsonTx::init()
{
    ntype = cryptonote::network_type {jtx["nettype"]};
    fee = jtx["fee"];

    if (jtx.count("payment_id"))
    {
        hex_to_pod(jtx["payment_id"], payment_id);
    }

    if (jtx.count("payment_id8"))
    {
        hex_to_pod(jtx["payment_id8"], payment_id8);
        hex_to_pod(jtx["payment_id8e"], payment_id8e);
        //cout << "jtx[\"payment_id8e\"] "<< jtx["tx_hash"] 
             //<< ", " << jtx["payment_id8e"] << endl;
    }

    
    if (jtx.count("is_payment_id8_real"))
    {
        is_payment_id8_real = jtx["is_payment_id8_real"];
    }

    if (jtx.count("sender"))
    {
        addr_and_viewkey_from_string(
                 jtx["sender"]["address"], jtx["sender"]["viewkey"],          
                 ntype, sender.address, sender.viewkey);

        parse_str_secret_key(jtx["sender"]["spendkey"], sender.spendkey);

        // actuall amount spent accounting fee
        sender.amount = jtx["sender"]["total_spent"].get<uint64_t>()
                - jtx["sender"]["total_recieved"].get<uint64_t>()
                + jtx["fee"].get<uint64_t>();

        sender.change = jtx["sender"]["total_recieved"];
        sender.ntype = ntype;

        populate_outputs(jtx["sender"]["outputs"], sender.outputs);
        
        populate_inputs(jtx["sender"]["inputs"], sender.inputs);
    }

    if (jtx.count("recipient"))
    {
        for (auto const& jrecpient: jtx["recipient"])
        {
            recipients.push_back(account{});

            addr_and_viewkey_from_string(
                     jrecpient["address"], jrecpient["viewkey"],              
                     ntype, recipients.back().address,
                     recipients.back().viewkey);

            parse_str_secret_key(jrecpient["spendkey"],
                    recipients.back().spendkey);

            recipients.back().amount = jrecpient["total_recieved"];

            recipients.back().is_subaddress = jrecpient["is_subaddress"];
            recipients.back().ntype = ntype;

            populate_outputs(jrecpient["outputs"], recipients.back().outputs);

            // recipients dont have inputs so we do not populate
            // them here.
            if (jrecpient.count("subaddress_index"))
            {
                auto saddr_idx = parse_subaddress_index(
                        jrecpient["subaddress_index"]);

                if (saddr_idx)
                {
                    recipients.back().subaddr_idx = saddr_idx;
                }
            }
        }
    }

    if (!hex_to_tx(jtx["tx_hex"], tx, tx_hash, tx_prefix_hash))
    {
        throw std::runtime_error("hex_to_tx(jtx[\"hex\"], "
                                 "tx, tx_hash, tx_prefix_hash)");
    }

}


bool
JsonTx::read_config()
{
    if (!boost::filesystem::exists(jpath))
    {
        cerr << "Config file " << jpath << " does not exist\n";
        return false;
    }

    try
    {
        // try reading and parsing json config file provided
        std::ifstream i(jpath);
        i >> jtx;
    }
    catch (std::exception const& e)
    {
        cerr << "Cant parse json string as json: " 
            << e.what() << endl;
        return false;
    }

    return true;
}


bool
check_and_adjust_path(string& in_path)
{
    if (!boost::filesystem::exists(in_path))
        in_path = "./tests/" + in_path;

    return boost::filesystem::exists(in_path);
}

boost::optional<JsonTx>
construct_jsontx(string tx_hash, string in_dir)
{
    string tx_path  = in_dir + "./tx/tx_" + tx_hash + ".json";

    if (!check_and_adjust_path(tx_path))
    {
        cerr << tx_path << " does not exist!\n";
        return {};
    }

    return JsonTx {tx_path};
}

void
JsonTx::populate_outputs(json const& joutputs, vector<output>& outs)
{
    for (auto const& jout: joutputs)
    {
        public_key out_pk;

        if (!hex_to_pod(jout[1], out_pk))
        {
            throw std::runtime_error("hex_to_pod(jout[1], out_pk)");
        }

        output out {jout[0], out_pk, jout[2]};

        outs.push_back(out);
    }
}

void
JsonTx::populate_inputs(json const& jinputs, vector<input>& ins)
{
    for (auto const& jin: jinputs)
    {
        public_key out_pk;

        if (!hex_to_pod(jin[1], out_pk))
        {
            throw std::runtime_error("hex_to_pod(jout[1], out_pk)");
        }
        
        key_image key_img;

        if (!hex_to_pod(jin[0], key_img))
        {
            throw std::runtime_error("hex_to_pod(jout[1], out_pk)");
        }

        input in {key_img, jin[2], out_pk};

        ins.push_back(in);
    }
}

}
