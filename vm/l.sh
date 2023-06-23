#l/bin/bash

make clean
make 
cd build
make tests/vm/lazy-anon.result
cd ..