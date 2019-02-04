#include "src/MicroCore.h"
#include "src/tools.h"

#include "version.h"

#include <iostream>
#include <memory>


using namespace std;
using namespace cryptonote;

int
main(int ac, const char* av[])
{
    mlog_configure(mlog_get_default_log_path(""), true);
    mlog_set_log("1");

    cout << "Program is starting\n";

    network_type nettype = network_type::STAGENET;

    string blockchain_path = xmreg::get_default_lmdb_folder(nettype);

    cout << "Blockchain path: " << blockchain_path << '\n';

    cout << "Monero Version: " << MONERO_VERSION_FULL << endl;

    cout << "Initializaing MicroCore\n";

    xmreg::MicroCore mcore {blockchain_path, nettype};


    return EXIT_SUCCESS;
}
