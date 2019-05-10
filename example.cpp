#include "src/MicroCore.h"
#include "src/UniversalIdentifier.hpp"

#include "version.h"

#include <iostream>
#include <memory>


using namespace std;
using namespace cryptonote;

using xmreg::operator<<;


/**
 * A helper functor to get transaction based
 * on its hash in string
 */
struct TxGetter
{
    xmreg::MicroCore const* mcore {nullptr};

    boost::optional<transaction>
    operator()(string tx_hash_str) const 
    {
        assert(mcore);
        
        crypto::hash tx_hash;

        if (!epee::string_tools::hex_to_pod(tx_hash_str, tx_hash))
        {
            cerr << "Cant decode tx hash: " << tx_hash_str << '\n';
            return {};
        }

        boost::optional<transaction> tx = transaction {};

        if (!mcore->get_tx(tx_hash, *tx))
        {
            cerr << "Cant get tx: " << tx_hash_str << '\n';
            return {};
        } 

        return tx;
    }

};

inline std::ostream&
operator<<(std::ostream& os, boost::optional<transaction> const& tx)
{
    if (tx)
    {
        os << get_transaction_hash(*tx); 
    }
    else
    {
        os << "N/A";
    }

    return os;
}

template <typename T>
inline std::ostream&
operator<<(std::ostream& os, boost::optional<T> const& pid)
{
    if (pid)
    {
        os << epee::string_tools::pod_to_hex<T>(*pid); 
    }
    else
    {
        os << "N/A";
    }

    return os;
}

int
main(int ac, const char* av[])
{
    // setup monero logger for minimum output
    mlog_configure(mlog_get_default_log_path(""), true);
    mlog_set_log("1");

    cout << "Program is starting\n";

    network_type nettype = network_type::MAINNET;

    string blockchain_path = xmreg::get_default_lmdb_folder(nettype);

    cout << "Mainnet blockchain path: " << blockchain_path << '\n'
         << "Monero Version: " << MONERO_VERSION_FULL << '\n';

    cout << "Initializaing MicroCore\n\n";
    xmreg::MicroCore mcore {blockchain_path, nettype};

    // transaction getter helper
    TxGetter get_tx {&mcore};

    cout << "\n***Identify outputs in a tx based on address and viewkey (no subaddreses)***\n\n";

    {
        // use Monero donation address and viewkwey
        // will search of output in a give tx addressed 
        // to the primary address only. 
        auto account = xmreg::make_account(
                "44AFFq5kSiGBoZ4NMDwYtN18obc8AemS33DBLWs3H7otXft3XjrpDtQGv7SqSsaBYBb98uNbr2VBBEt7f2wfn3RVGQBEP3A",
                "f359631075708155cc3d92a32b75a7d02a5dcf27756707b47a2b31b21c389501");

        cout << "Monero donation account: " << *account << '\n';

        auto tx = get_tx("e8ceef12683b3180d83dd1c24f8f871d52d206b80d8a6db6c5504eb0596b0312");

        if (!tx)
            return EXIT_FAILURE;

        auto identifier = make_identifier(*tx,
              make_unique<xmreg::Output>(account.get()));

        identifier.identify();

        auto outputs_found = identifier.get<xmreg::Output>()->get();

        if (!outputs_found.empty())
        {
            cout << "Found following outputs in tx " << tx << ":\n"
                 << outputs_found << '\n';
        }
    }
    
    cout << "\n***Identify outputs in a tx based on address and viewkey (with subaddresses)***\n\n";

    {
        // use Monero forum donation address and viewkwey
        // will search of inputs in a give tx addressed 
        // to the primary address and its subaddress. 
        auto primary_account = xmreg::make_primaryaccount(
                "45ttEikQEZWN1m7VxaVN9rjQkpSdmpGZ82GwUps66neQ1PqbQMno4wMY8F5jiDt2GoHzCtMwa7PDPJUJYb1GYrMP4CwAwNp",
                "c9347bc1e101eab46d3a6532c5b6066e925f499b47d285d5720e6a6f4cc4350c");

        cout << "Monero forum donation account: " << *primary_account << '\n';

        auto tx = get_tx("54cef43983ca9eeded46c6cc1091bf6f689d91faf78e306a5ac6169e981105d8");

        if (!tx)
            return EXIT_FAILURE;

        auto identifier = make_identifier(*tx,
              make_unique<xmreg::Output>(primary_account.get()));

        identifier.identify();

        auto outputs_found = identifier.get<xmreg::Output>()->get();

        if (!outputs_found.empty())
        {
            cout << "Found following outputs in tx " << tx << ":\n"
                 << outputs_found << '\n';

            // identified output is for subaddress of index 0/10 which 
            // in this case is for the "xiphon part time coding (3 months)"
            // proposal https://ccs.getmonero.org/proposals/xiphon-part-time.html
        }
    }

    cout << "\n***Possible spending based on address and viewkey (no subaddress)***\n\n";

    {
        // use Monero donation address and viewkwey
        // will search of inputs in a give tx addressed 
        // to the primary address only. 
        auto account = xmreg::make_account(
                "44AFFq5kSiGBoZ4NMDwYtN18obc8AemS33DBLWs3H7otXft3XjrpDtQGv7SqSsaBYBb98uNbr2VBBEt7f2wfn3RVGQBEP3A",
                "f359631075708155cc3d92a32b75a7d02a5dcf27756707b47a2b31b21c389501");

        // not using subaddresess here as people donate directly to the primary 
        // address

        auto tx = get_tx("aa739a3ce8d3171a422ed3a3f5016384cdb17a5d3eb5905021f1103574d47eaf");

        if (!tx)
            return EXIT_FAILURE;

        // we can join individual identifiers as below, sice to estimate
        // spendings we need to identify possible inputs with their values,
        // as well as outputs corresponding to the change returned to Monero
        // donation address
        auto identifier = make_identifier(*tx,
              make_unique<xmreg::Output>(account.get()),
              make_unique<xmreg::GuessInput>(account.get(), &mcore));

        identifier.identify();

        auto outputs_found = identifier.get<xmreg::Output>()->get();
        auto inputs_found = identifier.get<xmreg::GuessInput>()->get();
        
        // basic sanity check. If the spending was correctly guesseid,
        // at least the number of identify inputs should match the 
        // number of inputs in the tx
        if (tx->vin.size() == inputs_found.size())
        {
            // possible spending is just basic math
            auto possible_total_spent = xmreg::calc_total_xmr(inputs_found)
                                        - xmreg::calc_total_xmr(outputs_found)
                                        - get_tx_fee(*tx);

            cout << "Possible spending from Monero project donation is: " 
                 << print_money(possible_total_spent) << " xmr\n";
        }
    }

    cout << "\n***Possible spending based on address and viewkey (with subaddress)***\n\n";

    {
        // use Monero forum donation address and viewkwey
        // will search of inputs in a give tx addressed 
        // to the primary address only. 
        auto account = xmreg::make_account(
                "45ttEikQEZWN1m7VxaVN9rjQkpSdmpGZ82GwUps66neQ1PqbQMno4wMY8F5jiDt2GoHzCtMwa7PDPJUJYb1GYrMP4CwAwNp",
                "c9347bc1e101eab46d3a6532c5b6066e925f499b47d285d5720e6a6f4cc4350c");

        // to work with subaddresses we need PrimaryAccount. We can
        // create it using xmreg::make_primaryaccount instead of 
        // xmreg::make_account. But in case we dont know ahead of time
        // what account we have (we can have subbaddress account) we
        // can manualy cast Account into PrimaryAccount

        auto primary_account = xmreg::make_primaryaccount(
                std::move(account));

        if (!primary_account)
        {
            cerr << "Cant convert Account into PrimaryAccount\n";
            return EXIT_FAILURE; 
        }

        cout << "Monero formum donation account: " << *primary_account << '\n';

        auto tx = get_tx("401bf77c9a49dd28df5f9fb15846f9de05fce5f0e11da16d980c4c9ac9156354");

        if (!tx)
            return EXIT_FAILURE;

        // we can join individual identifiers as below, sice to estimate
        // spendings we need to identify possible inputs with their values,
        // as well as outputs corresponding to the change returned to Monero
        // donation address
        auto identifier = make_identifier(*tx,
              make_unique<xmreg::Output>(primary_account.get()),
              make_unique<xmreg::GuessInput>(primary_account.get(), &mcore));

        identifier.identify();

        auto outputs_found = identifier.get<xmreg::Output>()->get();
        auto inputs_found = identifier.get<xmreg::GuessInput>()->get();
        
        // basic sanity check. If the spending was correctly guesses
        // at least the number of identify inputs should match the 
        // number of inputs in the tx
        if (tx->vin.size() == inputs_found.size())
        {
            // possible spending is just basic math
            auto possible_total_spent = xmreg::calc_total_xmr(inputs_found)
                                        - xmreg::calc_total_xmr(outputs_found)
                                        - get_tx_fee(*tx);

            cout << "Possible spending from Monero fourm donation is: " 
                 << print_money(possible_total_spent) << " xmr\n";
        }
    }

    cout << "\n***Identify legacy payment id***\n\n";

    {
        auto tx = get_tx("ce0d32093ca9cc5b7bcae3f4d3508c04846e8bceecc0819fd2c3191b64caad05");

        if (!tx)
            return EXIT_FAILURE;

        auto identifier = make_identifier(*tx, 
                                make_unique<xmreg::LegacyPaymentID>());

        identifier.identify();

        auto payment_id = identifier.get<xmreg::LegacyPaymentID>()->get();

        if (payment_id)
        {
            cout << "Found following legacy payment id in tx " << payment_id << '\n';
        }
    }
    
    cout << "\n***Identify and decrypt short payment id***\n\n";

    {
        // use Monero forum donation address and viewkwey
        auto account = xmreg::make_account(
                "45ttEikQEZWN1m7VxaVN9rjQkpSdmpGZ82GwUps66neQ1PqbQMno4wMY8F5jiDt2GoHzCtMwa7PDPJUJYb1GYrMP4CwAwNp",
                "c9347bc1e101eab46d3a6532c5b6066e925f499b47d285d5720e6a6f4cc4350c");

        auto tx = get_tx("401bf77c9a49dd28df5f9fb15846f9de05fce5f0e11da16d980c4c9ac9156354");

        if (!tx)
            return EXIT_FAILURE;

        auto identifier = make_identifier(*tx,
              make_unique<xmreg::IntegratedPaymentID>(account.get()));

        identifier.identify();
        
        auto payment_id = identifier.get<xmreg::IntegratedPaymentID>()->get();


        if (payment_id)
        {
            cout << "Found following itegrated payment id in tx " << payment_id << '\n';
        }
    }

    return EXIT_SUCCESS;
}
