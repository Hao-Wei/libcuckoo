#!/bin/bash

cd builds

cd libcuckoo
./tests/universal-benchmark/universal_benchmark --reads 100 --prefill 75 --total-ops 500 --initial-capacity 23
cd ..
	
cd dhash
./tests/universal-benchmark/universal_benchmark --reads 100 --prefill 75 --total-ops 500 --initial-capacity 23
cd ..

cd ..
