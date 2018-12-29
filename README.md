# Moneroexamples core repositories

This repository includes code that is oftenly used among moneroexamples projects.
It includes:

 - classess for decoding outputs/inputs, payment ids
 - general utility tools (e.g., get default monero blockchain path)
 - more to come ...

# C++14

C++14 is required to run this code.

#### Monero download and compilation

Follow instructions in the following link:

https://github.com/moneroexamples/monero-compilation/blob/master/README.md


#### Compilation of the xmregcore

```bash

# go to home folder if still in ~/monero
cd ~

git clone --recurse-submodules  https://github.com/moneroexamples/xmregcore.git

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
