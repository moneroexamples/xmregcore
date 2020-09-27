#include "UniversalIdentifier.hpp"

namespace xmreg
{

using  epee::string_tools::pod_to_hex;
using  epee::string_tools::hex_to_pod;

public_key
get_tx_pub_key_from_received_outs(transaction const& tx)
{
  std::vector<tx_extra_field> tx_extra_fields;

  if(!parse_tx_extra(tx.extra, tx_extra_fields))
  {
      // Extra may only be partially parsed, it's OK if tx_extra_fields contains public key
  }

  // Due to a previous bug, there might be more than one tx pubkey in extra, one being
  // the result of a previously discarded signature.
  // For speed, since scanning for outputs is a slow process, we check whether extra
  // contains more than one pubkey. If not, the first one is returned. If yes, they're
  // checked for whether they yield at least one output
  tx_extra_pub_key pub_key_field;

  if (!find_tx_extra_field_by_type(tx_extra_fields, pub_key_field, 0))
  {
      return null_pkey;
  }

  public_key tx_pub_key = pub_key_field.pub_key;

  bool two_found = find_tx_extra_field_by_type(tx_extra_fields, pub_key_field, 1);

  if (!two_found)
  {
      // easy case, just one found
      return tx_pub_key;
  }
  else
  {
      // just return second one if there are two.
      // this does not require private view key, as
      // its not needed for my use case.
      return pub_key_field.pub_key;
  }

  return null_pkey;
}


void
Output::identify(transaction const& tx,
                 public_key const& tx_pub_key,
                 vector<public_key> const& additional_tx_pub_keys)
{
    auto tx_is_coinbase = is_coinbase(tx);

    key_derivation derivation;

    if (!generate_key_derivation(tx_pub_key,
                                 *get_viewkey(), derivation))
    {
        static_assert(sizeof(derivation) == sizeof(rct::key),
                "Mismatched sizes of key_derivation and rct::key");

        // use identity derivation instead
        // solution based on that found in wallet2.cpp in monero
        // this will cause the tx output to be effectively skipped
        memcpy(&derivation, rct::identity().bytes, sizeof(derivation));
    }

    // since introduction of subaddresses, a tx can
    // have extra public keys, thus we need additional
    // derivations

    vector<key_derivation> additional_derivations;

    if (!additional_tx_pub_keys.empty())
    {
        additional_derivations.resize(additional_tx_pub_keys.size());

        for (size_t i = 0; i < additional_tx_pub_keys.size(); ++i)
        {
            if (!generate_key_derivation(additional_tx_pub_keys[i],
                                         *get_viewkey(),
                                         additional_derivations[i]))
            {
                static_assert(sizeof(derivation) == sizeof(rct::key),
                        "Mismatched sizes of key_derivation and rct::key");

                // use identity derivation instead
                // solution based on that found in wallet2.cpp in monero
                // this will cause the tx output to be effectively skipped
                memcpy(&additional_derivations[i],
                       rct::identity().bytes,
                       sizeof(additional_derivations[i]));
            }
        }
    }
		

	auto const& pub_spend_key 
		= get_address()->address.m_spend_public_key;

    // if we have PrimaryAccount we can check 
    // if a given output belongs to any of its
    // its subaddresses
    PrimaryAccount* pacc {nullptr};

    if (acc && !acc->is_subaddress())
    {
        // so we have primary address
        pacc = static_cast<PrimaryAccount*>(acc);
    }


    for (auto i = 0u; i < tx.vout.size(); ++i)
    {
        // i will act as output indxes in the tx

        if (tx.vout[i].target.type() != typeid(txout_to_key))
            continue;

        // get tx input key
        txout_to_key const& txout_key
                = boost::get<txout_to_key>(tx.vout[i].target);

        uint64_t amount = tx.vout[i].amount;

		// calculate public spendkey using derivation
		// and tx output key. If this is our output
		// the caulcualted public spendkey should match
 		// our actuall public spend key avaiable in our
	    // public monero address. Primary address is 
		// a special case of subaddress. 

        // we are always going to have the subaddress_spend
        // key if an output is ours
        crypto::public_key subaddress_spendkey;

        // however we might not have its index, in case
        // we are not using primary addresses directly
        // but instead use a subaddress for searching
        // outputs
        std::unique_ptr<subaddress_index> subaddr_idx;

        hwdev.derive_subaddress_public_key(
					txout_key.key, derivation, i, 
					subaddress_spendkey);

        // this derivation is going to be saved 
        // it can be one of addiitnal derivations
        // if we are dealing with multiouput tx
        // which cointains subaddress
        auto derivation_to_save = derivation;

	    bool mine_output {false};

        if (!pacc)
        {
            // if pacc is not given, we check generated 
            // subaddress_spendkey against the spendkey 
            // of the address for which the Output identifier
            // was instantiated
    	    mine_output = (pub_spend_key == subaddress_spendkey);
        }
        else
        {
            // if pacc is given, we are going to use its 
            // subaddress unordered map to check if generated
            // subaddress_spendkey is one of its keys. this is 
            // because the map can contain spendkeys of subaddreses
            // assiciated with primary address. primary address's
            // spendkey will be one of the keys as a special case
            
            subaddr_idx = pacc->has_subaddress(subaddress_spendkey); 

            mine_output = bool {subaddr_idx};
        }

        auto with_additional = false;

        if (!mine_output && !additional_tx_pub_keys.empty())
        {
            // check for output using additional tx public keys
	    	hwdev.derive_subaddress_public_key(
						txout_key.key, additional_derivations[i], 
						i, 
						subaddress_spendkey);
	    
            // do same comparison as above depending of the 
            // avaliabity of the PrimaryAddress Account 
            if (!pacc)
            {
                mine_output = (pub_spend_key == subaddress_spendkey);
            }
            else
            {
                subaddr_idx = pacc->has_subaddress(subaddress_spendkey); 
                mine_output = bool {subaddr_idx};
            }

            with_additional = true;
        }

        // placeholder variable for ringct outputs info
        // that we need to save in database

        rct::key rtc_outpk {0};
        rct::key rtc_mask {0};
        rct::key rtc_amount {0};

        // if mine output has RingCT, i.e., tx version is 2
        // need to decode its amount. otherwise its zero.
        if (mine_output && tx.version == 2)
        {
            // initialize with regular amount value
            // for ringct, except coinbase, it will be 0
            uint64_t rct_amount_val = amount;

            // cointbase txs have amounts in plain sight.
            // so use amount from ringct, only for non-coinbase txs
            if (!tx_is_coinbase)
            {
                // for ringct non-coinbase txs, these values are given
                // with txs.
                // coinbase ringctx dont have this information. we will provide
                // them only when needed, in get_unspent_outputs. So go there
                // to see how we deal with ringct coinbase txs when we spent
                // them
                // go to CurrentBlockchainStatus::construct_output_rct_field
                // to see how we deal with coinbase ringct that are used
                // as mixins

                rtc_outpk = tx.rct_signatures.outPk[i].mask;
                rtc_mask = tx.rct_signatures.ecdhInfo[i].mask;
                rtc_amount = tx.rct_signatures.ecdhInfo[i].amount;

                rct::key mask =  tx.rct_signatures.ecdhInfo[i].mask;
        
                derivation_to_save = !with_additional ? derivation
                                             : additional_derivations[i];

                auto r = decode_ringct(tx.rct_signatures,
                                       derivation_to_save,
                                       i,
                                       mask,
                                       rct_amount_val);

                (void) mask;

                if (!r)
                {
                    throw std::runtime_error(
                                "Cant decode ringCT!");
                }

                amount = rct_amount_val;

            } // if (!tx_is_coinbase)

        } // if (mine_output && tx.version == 2)

        if (mine_output)
        {
            total_received += amount;

            identified_outputs.emplace_back(
                    info{
                        txout_key.key, amount, i, 
                        derivation_to_save,
                        rtc_outpk, rtc_mask, rtc_amount,
                        subaddress_spendkey
                    });

            if (subaddr_idx)
            {
                auto& out = identified_outputs.back();
                out.subaddr_idx = *subaddr_idx;

                // now need to check if we need to expand
                // list of initial 10'000 of subaddresses.
                // we do this only if account id (subaddress_idx.major)
                // is greater than 0.
                
                auto next_subaddr_acc_id 
                    = pacc->get_next_subbaddress_acc_id();

                auto no_of_new_accounts = std::min<int>(
                                static_cast<int>(out.subaddr_idx.major
                                + PrimaryAccount::SUBADDRESS_LOOKAHEAD_MAJOR)
                                - next_subaddr_acc_id
                                , 50);

                if (no_of_new_accounts > 0)
                {
                    auto new_last_acc_id 
                        = pacc->get_next_subbaddress_acc_id() 
                                      + no_of_new_accounts;
                    pacc->expand_subaddresses(new_last_acc_id);
                }
            }

            total_xmr += amount;
        } //  if (mine_output)

    } // for (uint64_t i = 0; i < tx.vout.size(); ++i)
}



bool
Output::decode_ringct(rct::rctSig const& rv,
              crypto::key_derivation const& derivation,
              unsigned int i,
              rct::key& mask,
              uint64_t& amount) const
{
    try
    {
        crypto::secret_key scalar1;

        hwdev.derivation_to_scalar(derivation, i, scalar1);

        switch (rv.type)
        {
            case rct::RCTTypeSimple:
            case rct::RCTTypeBulletproof:
            case rct::RCTTypeBulletproof2:
            case rct::RCTTypeCLSAG:    
                amount = rct::decodeRctSimple(rv,
                                              rct::sk2rct(scalar1),
                                              i,
                                              mask,
                                              hwdev);
                break;
            case rct::RCTTypeFull:
                amount = rct::decodeRct(rv,
                                        rct::sk2rct(scalar1),
                                        i,
                                        mask,
                                        hwdev);
                break;
            default:
                cerr << "Unsupported rct type: " << rv.type << '\n';
                return false;
        }
    }
    catch (...)
    {
        cerr << "Failed to decode input " << i << '\n';
        return false;
    }

    return true;
}


void Input::identify(transaction const& tx,
                     public_key const& tx_pub_key,
                     vector<public_key> const& additional_tx_pub_keys)
{

    // if known_outputs is null do nothing
    if (!known_outputs)
        return;

     //auto search_misses {0};

     auto input_no = tx.vin.size();

     for (auto i = 0u; i < input_no; ++i)
     {
         if(tx.vin[i].type() != typeid(txin_to_key))
             continue;

         // get tx input key
         txin_to_key const& in_key
                 = boost::get<txin_to_key>(tx.vin[i]);

         // get absolute offsets of mixins
         vector<uint64_t> absolute_offsets
                 = relative_output_offsets_to_absolute(
                         in_key.key_offsets);

         // get public keys of outputs used in the mixins that
         // match to the offests
         vector<output_data_t> mixin_outputs;

         // before we procced to fetch the outputs from lmdb
         // check if we are not trying to get the outputs
         // with non-existing offsets

         auto num_outputs = mcore->get_num_outputs(in_key.amount);

         if (absolute_offsets.back() >= num_outputs)
         {
             //cerr << "skipping offset" << endl;
             // we try to get output with offset 
             // greater what is storred in lmdb
             continue;
         }

         // this can THROW if no outputs are found
         // but previous check should prevent this
         mcore->get_output_key(in_key.amount,
                               absolute_offsets,
                               mixin_outputs);

         // indicates whether we found any matching mixin in the current input
         auto found_a_match {false};

         // for each found output public key check if its ours or not
         for (auto count = 0u; count < absolute_offsets.size(); ++count)
         {
             // get basic information about mixn's output
             output_data_t const& output_data
                     = mixin_outputs.at(count);

             // before going to the mysql, check our known outputs cash
             // if the key exists. Its much faster than going to mysql
             // for this.
             
             auto it = known_outputs->find(output_data.pubkey);

             if (it != known_outputs->end())
             {
                 // this seems to be our mixin.
                 // save it into identified_inputs vector

                 identified_inputs.push_back(info {
                         in_key.k_image,
                         it->second, // amount
                         output_data.pubkey});

                 total_xmr += it->second;

                 found_a_match = true;
             }  

         } // for (const cryptonote::output_data_t& output_data: outputs)

//         if (found_a_match == false)
//         {
//             // if we didnt find any match, break of the look.
//             // there is no reason to check remaining key images
//             // as when we spent something, our outputs should be
//             // in all inputs in a given txs. Thus, if a single input
//             // is without our output, we can assume this tx does
//             // not contain any of our spendings.

//             // just to be sure before we break out of this loop,
//             // do it only after two misses

//            // if (++search_misses > 2)
//               //  break;
//         }

     } //  for (auto i = 0u; i < input_no; ++i)
}


/*
 * Generate key_image of foran ith output
 */
bool
Input::generate_key_image(const crypto::key_derivation& derivation,
                   const std::size_t i,
                   const crypto::secret_key& sec_key,
                   const crypto::public_key& pub_key,
                   crypto::key_image& key_img) const
{

    cryptonote::keypair in_ephemeral;

    if (!crypto::derive_public_key(derivation, i,
                                   pub_key,
                                   in_ephemeral.pub))
    {
        cerr << "Error generating publick key " << pub_key << endl;
        return false;
    }

    try
    {
        crypto::derive_secret_key(derivation, i,
                                  sec_key,
                                  in_ephemeral.sec);
    }
    catch(const std::exception& e)
    {
        cerr << "Error generate secret image: " << e.what() << endl;
        return false;
    }


    try
    {
        crypto::generate_key_image(in_ephemeral.pub,
                                   in_ephemeral.sec,
                                   key_img);
    }
    catch(const std::exception& e)
    {
        cerr << "Error generate key image: " << e.what() << endl;
        return false;
    }

    return true;
}

void
GuessInput::identify(transaction const& tx,
                     public_key const& tx_pub_key,
                     vector<public_key> const& additional_tx_pub_keys)
{
    // to implement this method, we are just going
    // to generate known_outputs_t = unordered_map<public_key, uint64_t>;
    // based on ring members in each key image, and then
    // we will call identify method of the Input base class.

    // this will store guessed inputs
    vector<info> local_identified_inputs;
        
    // will keep output public key and amount
    // of mixins in the given key image which
    // are ours.
    known_outputs_t known_outputs_map;

    auto input_no = tx.vin.size();
           
    for (auto i = 0u; i < input_no; ++i)
    {
        if(tx.vin[i].type() != typeid(txin_to_key))
            continue;

        // get tx input key
        txin_to_key const& in_key
                = boost::get<cryptonote::txin_to_key>(tx.vin[i]);

        // get absolute offsets of mixins
        auto absolute_offsets
                = relative_output_offsets_to_absolute(
                        in_key.key_offsets);

        //tx_out_index is pair::<transaction hash, output index>
        vector<tx_out_index> indices;

        // get tx hashes and indices in the txs for the
        // given outputs of mixins
        //  this cant THROW DB_EXCEPTION
        mcore->get_output_tx_and_index(
                    in_key.amount, absolute_offsets, indices);

        // for each found mixin tx, check if any key image
        // generated using our outputs in the mixin tx
        // matches the given key image in the current tx
        for (auto const& txi : indices)
        {
           auto const& mixin_tx_hash = txi.first;          

           transaction mixin_tx;

           if (!mcore->get_tx(mixin_tx_hash, mixin_tx))
           {
               throw std::runtime_error("Cant get tx: "
                                        + pod_to_hex(mixin_tx_hash));
           }

           // use Output universal identifier to identify our outputs
           // in a mixin tx

           std::unique_ptr<Output> output_identifier;

           if (acc)
           {
               output_identifier = make_unique<Output>(acc);
           }
           else
           {
               output_identifier = make_unique<Output>(
                       get_address(), get_viewkey());
           }

           auto identifier = make_identifier(
                       mixin_tx, std::move(output_identifier));

           identifier.identify();

           for (auto const& found_output: identifier.get<Output>()->get())
           {
               // add found output into the map of known ouputs
               known_outputs_map.insert(
                    {found_output.pub_key, found_output.amount});
           }

        } // for (auto const& txi : indices)

        // so hopefully we found some of the mixins that are
        // ours. So now, lets try use that information to
        // guess which of them was used in the current
        // key image.

    } // for (auto i = 0u; i < input_no; ++i)
        
    // to do this, set known_outputs to the known_outputs_map
    known_outputs = &known_outputs_map;

    // and now execute baseclasses (i.e. Input) identify
    // method. The method will use known_outputs as
    // its list of outputs
    Input::identify(tx, tx_pub_key, additional_tx_pub_keys);
    
    // copy what was identified using Input::identify
    // into local_identified_inputs
    local_identified_inputs.insert(local_identified_inputs.end(),
                                   identified_inputs.begin(),
                                   identified_inputs.end());


    // once all key images have been scanned,
    // copy local_identified_inputs into identified_inputs
    // so that we can return them to the end user
    identified_inputs = std::move(local_identified_inputs);
}


void RealInput::identify(transaction const& tx,
                         public_key const& tx_pub_key,
                         vector<public_key> const& additional_tx_pub_keys)
{
     auto input_no = tx.vin.size();

     for (auto i = 0u; i < input_no; ++i)
     {
         if(tx.vin[i].type() != typeid(txin_to_key))
             continue;

         // get tx input key
         txin_to_key const& in_key
                 = boost::get<cryptonote::txin_to_key>(tx.vin[i]);

         // get absolute offsets of mixins
         auto absolute_offsets
                 = relative_output_offsets_to_absolute(
                         in_key.key_offsets);


         //tx_out_index is pair::<transaction hash, output index>
         vector<tx_out_index> indices;

         // get tx hashes and indices in the txs for the
         // given outputs of mixins
         //  this cant THROW DB_EXCEPTION
         mcore->get_output_tx_and_index(
                     in_key.amount, absolute_offsets, indices);

         // placeholder for information about key image that
         // we will find as ours
         unique_ptr<info> key_image_info {nullptr};

         // for each found mixin tx, check if any key image
         // generated using our outputs in the mixin tx
         // matches the given key image in the current tx
         for (auto const& txi : indices)
         {
            auto const& mixin_tx_hash = txi.first;         

            transaction mixin_tx;

            if (!mcore->get_tx(mixin_tx_hash, mixin_tx))
            {
                throw std::runtime_error("Cant get tx: "
                                         + pod_to_hex(mixin_tx_hash));
            }

            // use Output universal identifier to identify our outputs
            // in a mixin tx
           
            std::unique_ptr<Output> output_identifier;

            if (acc)
            {
                output_identifier = make_unique<Output>(acc);
            }
            else
            {
                output_identifier = make_unique<Output>(
                       get_address(), get_viewkey());
            }

            auto identifier = make_identifier(
                       mixin_tx, std::move(output_identifier));

            identifier.identify();

            //cout << "mixin tx hash: " << get_transaction_hash(mixin_tx) << '\n';

            for (auto const& found_output: identifier.get<Output>()->get())
            {
                //cout << "found_output: " << found_output << endl;

                // generate key_image using this output
                // to check for sure if the given key image is ours
                // or not
                crypto::key_image key_img_generated;

                if (acc)
                {
                    // if we have primary account, as we should when
                    // we want to include
                    // for spendings from subaddresses, use the below procedure
                    // to calcualted key_img_generated
                    
                    cryptonote::keypair in_ephemeral;
                    
                    if (!generate_key_image_helper_precomp(*acc->keys(), 
                                                      found_output.pub_key,
                                                      found_output.derivation,
                                                      found_output.idx_in_tx,
                                                      found_output.subaddr_idx,
                                                      in_ephemeral,
                                                      key_img_generated,
                                                      hwdev))
                    {
                        throw std::runtime_error("Cant get key_img_generated");
                    }

                    (void) in_ephemeral;
                }
                else
                {
                    // if we don't have acc, i.e., dont have info about subaddresses
                    // then use the simpler way
                    // to calcualate key_img_generated
                    
                    if (!generate_key_image(found_output.derivation,
                                            found_output.idx_in_tx, /* position in the tx */
                                            *spendkey,
                                            get_address()->address.m_spend_public_key,
                                            key_img_generated))
                    {
                        throw std::runtime_error("Cant generate " 
                                "key image for output: "
                                + pod_to_hex(found_output.pub_key));
                    }
                }

                //cout << pod_to_hex(in_key.k_image) << " == " 
                     //<< pod_to_hex(key_img_generated) << '\n';

                // now check if current key image in the tx which we
                // analyze matches generated key image
                if (in_key.k_image == key_img_generated)
                {
                    // this is our key image if they are equal!
                    key_image_info.reset(new info {key_img_generated,
                                                   found_output.amount,
                                                   found_output.pub_key});

                    break;
                }

            } // auto const& found_output: identifier.get<

            // if key_image_info has been populated, there is no
            // reason to keep check remaning outputs in the mixin tx
            // instead add its info to identified_inputs and move on
            // to the next key image
            if (key_image_info)
            {
                identified_inputs.push_back(*key_image_info);
                total_xmr += key_image_info->amount;
                break;
            }

         } // for (auto const& txi : indices)

     } //  for (auto i = 0u; i < input_no; ++i)
}



// just a copy from bool
// device_default::encrypt_payment_id(crypto::hash8 
// &payment_id, const crypto::public_key &public_key, 
// const crypto::secret_key &secret_key)
template <typename HashT>
bool
PaymentID<HashT>::encrypt_payment_id(
        crypto::hash8& payment_id,
        crypto::public_key const& public_key,
        crypto::secret_key const& secret_key) const
{
    #define ENCRYPTED_PAYMENT_ID_TAIL 0x8d

    crypto::key_derivation derivation;
    crypto::hash hash;
    char data[33]; /* A hash, and an extra byte */

    if (!generate_key_derivation(public_key, secret_key, derivation))
        return false;

    memcpy(data, &derivation, 32);
    data[32] = ENCRYPTED_PAYMENT_ID_TAIL;
    cn_fast_hash(data, 33, hash);

    for (size_t b = 0; b < 8; ++b)
        payment_id.data[b] ^= hash.data[b];

    return true;
}

template bool
PaymentID<crypto::hash8>::encrypt_payment_id(
        crypto::hash8& payment_id,
        crypto::public_key const& public_key,
        crypto::secret_key const& secret_key) const;

template bool
PaymentID<crypto::hash>::encrypt_payment_id(
        crypto::hash8& payment_id,
        crypto::public_key const& public_key,
        crypto::secret_key const& secret_key) const;

template <typename HashT>
typename PaymentID<HashT>::payments_t
PaymentID<HashT>::get_payment_id(
        transaction const& tx) const
{
    crypto::hash payment_id {0};
    crypto::hash8 payment_id8 {0};

    std::vector<tx_extra_field> tx_extra_fields;

    if(!parse_tx_extra(tx.extra, tx_extra_fields))
    {
        return make_tuple(payment_id, payment_id8);
    }

    tx_extra_nonce extra_nonce;

    if (find_tx_extra_field_by_type(tx_extra_fields, extra_nonce))
    {
        // first check for encrypted id and then for normal one
        if(get_encrypted_payment_id_from_tx_extra_nonce(extra_nonce.nonce, 
                    payment_id8))
        {
            return make_tuple(payment_id, payment_id8);
        }
        else if (get_payment_id_from_tx_extra_nonce(extra_nonce.nonce, 
                    payment_id))
        {
            return make_tuple(payment_id, payment_id8);
        }
    }

    return make_tuple(payment_id, payment_id8);
}

template tuple<crypto::hash, crypto::hash8> 
PaymentID<crypto::hash8>::get_payment_id(transaction const& tx) const;
template tuple<crypto::hash, crypto::hash8> 
PaymentID<crypto::hash>::get_payment_id(transaction const& tx) const;


}
