#!/bin/bash

cd builds

cd libcuckoo
./tests/universal-benchmark/universal_benchmark --reads 100 --prefill 75 --total-ops 500 --initial-capacity 23
./tests/universal-benchmark/universal_benchmark --upserts 100 --prefill 25 --total-ops 200 --initial-capacity 23
cd ..

cd dhash
./tests/universal-benchmark/universal_benchmark --reads 100 --prefill 75 --total-ops 500 --initial-capacity 23
cd ..
	
cd ndhash
./tests/universal-benchmark/universal_benchmark --reads 100 --prefill 75 --total-ops 500 --initial-capacity 23
./tests/universal-benchmark/universal_benchmark --upserts 100 --prefill 25 --total-ops 200 --initial-capacity 23
cd ..

cd folklore
./tests/universal-benchmark/universal_benchmark --reads 100 --prefill 75 --total-ops 500 --initial-capacity 23
./tests/universal-benchmark/universal_benchmark --upserts 100 --prefill 25 --total-ops 200 --initial-capacity 23
cd ..

cd ..