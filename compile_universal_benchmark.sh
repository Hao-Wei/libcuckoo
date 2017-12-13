#!/bin/bash

rm -R builds
mkdir builds
cd builds

mkdir hopscotch
cd hopscotch
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=HOPSCOTCH ../..
make all
cd ..

mkdir ndquad
cd ndquad
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=NDQUAD ../..
make all
cd ..

mkdir folklore
cd folklore
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=FOLKLORE ../..
make all
cd ..

mkdir random
cd random
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=RANDOM ../..
make all
cd ..

mkdir empty
cd empty
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=EMPTY ../..
make all
cd ..

mkdir random2
cd random2
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=RANDOM2 ../..
make all
cd ..

mkdir libcuckoo
cd libcuckoo
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=LIBCUCKOO ../..
make all
cd ..
	

mkdir ndhash
cd ndhash
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=NDHASH ../..
make all
cd ..

mkdir paralleldp
cd paralleldp
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=PARALLELDP ../..
make all
cd ..

cd ..
