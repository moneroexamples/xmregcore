# Set of c++ code that is used commonly among moneroexamples repositories

This repository include code that is oftenly used among moneroexamples projects.
It includes:

 - classess for decoding outputs/inputs, payment ids
 - general utility tools (e.g., get default monero blockchain path)
 - more to come ...

# C++14

C++14 is required to run this code.


# Example compilation on Ubuntu 18.04

#### Monero download and compilation

Download and compile recent Monero into your home folder:

```bash
# first install monero dependecines
sudo apt update

sudo apt install git build-essential cmake libboost-all-dev miniupnpc libunbound-dev graphviz doxygen libunwind8-dev pkg-config libssl-dev libcurl4-openssl-dev libgtest-dev libreadline-dev libzmq3-dev libsodium-dev libhidapi-dev libhidapi-libusb0

# go to home folder
cd ~

git clone --recursive https://github.com/monero-project/monero

cd monero/

USE_SINGLE_BUILDDIR=1 make
```


#### Compilation of the xmregcore

```bash

# go to home folder if still in ~/monero
cd ~

git clone --recursive-submodule https://github.com/moneroexamples/xmregcore.git

cd xmregcore

mkdir build && cd build

cmake ..

# altearnatively can use cmake -DMONERO_DIR=/path/to/monero_folder ..
# if monero is not in ~/monero

make

# run tests
make test
```

# Other examples

Other examples can be found on  [github](https://github.com/moneroexamples?tab=repositories).
Please know that some of the examples/repositories are not
finished and may not work as intended.

# How can you help?

Constructive criticism, code and website edits are always good. They can be made through github.
