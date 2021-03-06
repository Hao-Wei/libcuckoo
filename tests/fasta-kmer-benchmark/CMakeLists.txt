add_executable(kmer_benchmark
    kmer_benchmark.cc
)

target_link_libraries(kmer_benchmark
    PRIVATE test_util
    PRIVATE libcuckoo
    PRIVATE pcg
)

if(NOT DEFINED UNIVERSAL_TABLE)
    set(UNIVERSAL_TABLE LIBCUCKOO)
endif()

target_compile_options(kmer_benchmark
    PRIVATE -DKEY=uint64_t
    PRIVATE -DVALUE=uint64_t
    PRIVATE -D${UNIVERSAL_TABLE}
    PRIVATE -O3
    PRIVATE -march=native
    PRIVATE -std=c++11
    -mcx16
)

if(UNIVERSAL_TRACKING_ALLOCATOR)
    target_compile_options(kmer_benchmark PRIVATE -DTRACKING_ALLOCATOR)
endif()
