add_executable(kmer_benchmark
    kmer_benchmark.cc
    universal_table_wrapper.hh
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
    PRIVATE -DKEY=std::string
    PRIVATE -DVALUE=size_t
    PRIVATE -D${UNIVERSAL_TABLE}
)

if(UNIVERSAL_TRACKING_ALLOCATOR)
    target_compile_options(kmer_benchmark PRIVATE -DTRACKING_ALLOCATOR)
endif()