#!/bin/bash

rm -R fasta_builds
mkdir fasta_builds
cd fasta_builds

mkdir folklore
cd folklore
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_KMER=1 -DUNIVERSAL_TABLE=FOLKLORE ../..
make all
cd ..

mkdir hopscotch
cd hopscotch
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_KMER=1 -DUNIVERSAL_TABLE=HOPSCOTCH ../..
make all
cd ..

mkdir libcuckoo
cd libcuckoo
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_KMER=1 -DUNIVERSAL_TABLE=LIBCUCKOO ../..
make all
cd ..
	
mkdir dhash
cd dhash
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_KMER=1 -DUNIVERSAL_TABLE=DHASH ../..
make all
cd ..

mkdir ndhash
cd ndhash
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_KMER=1 -DUNIVERSAL_TABLE=NDHASH ../..
make all
cd ..

mkdir paralleldp
cd paralleldp
cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_KMER=1 -DUNIVERSAL_TABLE=PARALLELDP ../..
make all
cd ..

cd ..
