#!/bin/bash

rm -R builds
mkdir builds
cd builds

mkdir hopscotch_bitmap
cd hopscotch_bitmap
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=HOPSCOTCH_BITMAP ../..
make all
cd ..

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
