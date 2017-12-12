#!/bin/bash

rm -R builds
mkdir builds
cd builds

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

mkdir libcuckoo
cd libcuckoo
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=LIBCUCKOO ../..
make all
cd ..
	
mkdir dhash
cd dhash
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=DHASH ../..
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
