#!/bin/bash

rm -R builds
mkdir builds
cd builds

#mkdir hopscotch_v2
#cd hopscotch_v2
#cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=HOPSCOTCH_V2 ../..
#make all
#cd ..

mkdir hopscotch
cd hopscotch
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=HOPSCOTCH ../..
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

cd ..
