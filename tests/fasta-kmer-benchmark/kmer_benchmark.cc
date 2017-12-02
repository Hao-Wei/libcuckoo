/* Benchmarks a mix of operations for a compile-time specified key-value pair */

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <pcg/pcg_random.hpp>
#include <test_util.hh>

#include "universal_table_wrapper.hh"

/* Run-time parameters -- operation mix and table configuration */

// Number of threads to run with
// size_t g_threads = std::thread::hardware_concurrency();
size_t g_threads = 8;
#define K_LEN (20)
size_t initial_capacity= 100000000;


#define XSTR(s) STR(s)
#define STR(s) #s

using std::cout; using std::cerr; using std::endl;

const char *description =
    "A benchmark for testing hash tables on sequence data. \nMap type is " TABLE_TYPE "<" XSTR(KEY) ", " XSTR(VALUE)
    ">.";


// There are undoubtably better ways to manage memory here, but let's start with this.
void preloadFile(std::vector<char>* &entries, std::string filename) {

  cout << "Preloading file: " << filename << endl;

  std::ifstream inFile(filename);
  if(!inFile) {
    cout << "Could not load file." << endl;
    exit(-1);
  }

  entries = new std::vector<char>();
  for( std::string line; getline( inFile, line ); )
  {

    // Skip comments and blank lines
    if(line.size() == 0 || line[0] == ';' || line[0] == '>') {
      continue;
    }

    // Copy all GATC entries
    for(char& c : line) {
      if(c == 'G' || c == 'A' || c == 'T' || c == 'C') {
        entries->push_back(c);
      }
    }

  }

  cout << "Loaded " << entries->size() << " entries." << endl;
  printf("  --> aka %.3f billion characters\n", (double)entries->size()/1e9);
  // for(char& c : *entries) {
  //   cout << c << endl;
  // }

}

size_t add1(size_t x) {
  return x + 1;
}

void countSequences(Table& tbl, const char* sequence, size_t startInd, size_t endInd) {

  // Walk the sequence hashing each substring
  for(size_t i = startInd; i < endInd; i++) {

    // Build the key
    std::string key(sequence + startInd, sequence + startInd + K_LEN);

    // Increment
    tbl.upsert(key, add1, 1); 

  }

}

int main(int argc, char **argv) {
  try {

    // Parse parameters
    if(argc != 2) {
      cout << "Usage: ./kmer FASTA_FILE" << endl;
      exit(-1);
    }

    std::string filename(argv[1]);

    // Load the file
    std::vector<char>* seq;
    preloadFile(seq, filename);
    char* seqArr = reinterpret_cast<char*>(&(*seq)[0]);

    // Compute a very explicit distribution of work
    std::vector<size_t> startInds, endInds; // (start is inclusize, end is exclusive)
    size_t Nchar = seq->size();
    size_t nSubSeq = Nchar - K_LEN + 1;
    size_t nPer = nSubSeq / g_threads + 1;
    size_t currStart = 0;
    cout << "Thread work distribution for " << g_threads << " threads:" << endl;
    for(size_t iT = 0; iT < g_threads; iT++) {
      size_t nextStart = std::min(currStart + nPer, nSubSeq);
      startInds.push_back(currStart); 
      endInds.push_back(nextStart); 
      printf("  Thread %3zu: [%15zu,%15zu)\n", iT, currStart, nextStart);
      currStart = nextStart;
    }
    
    
    // Create and size the table
    Table tbl(initial_capacity);


    // Run the counting "algorithm"
    std::cout << "Running operations\n";
    std::vector<std::thread> worker_threads(g_threads);
    auto start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < g_threads; ++i) {
      worker_threads[i] = std::thread(countSequences, std::ref(tbl), seqArr, startInds[i], endInds[i]);
    }
    for (auto &t : worker_threads) {
      t.join();
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    double seconds_elapsed =
        std::chrono::duration_cast<std::chrono::duration<double>>(end_time -
                                                                  start_time)
            .count();

    printf("Hash time: %.3f seconds\n", seconds_elapsed);

  } catch (const std::exception &e) {
    std::cerr << e.what();
    std::exit(1);
  }
}
