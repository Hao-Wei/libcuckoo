/* Benchmarks a mix of operations for a compile-time specified key-value pair */

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "universal_table_wrapper.hh"

/* Run-time parameters -- operation mix and table configuration */

// Number of threads to run with
// size_t g_threads = std::thread::hardware_concurrency();
size_t g_threads = 8;
#define K_LEN (17)  // MUST BE LESS THAN 32

#define XSTR(s) STR(s)
#define STR(s) #s

using std::cerr;
using std::cout;
using std::endl;

const char* description =
    "A benchmark for testing hash tables on sequence data. \nMap type "
    "is " TABLE_TYPE "<" XSTR(KEY) ", " XSTR(VALUE) ">.";

// Convert the character to a [0,4) integer
uint64_t charInd(char c) {
  switch (c) {
    case 'G':
      return 0;
      break;
    case 'A':
      return 1;
      break;
    case 'T':
      return 2;
      break;
    case 'C':
      return 3;
      break;
    default:
      assert("unexpected character in key string" && false);
  }

  return -1;
}

uint64_t buildKey(const char* keyStart) {
  uint64_t key = 0;
  // cout << "Generating key.." << endl;
  for (uint64_t i = 0; i < K_LEN; i++) {
    uint64_t charVal = charInd(keyStart[i]);

    // Pack bits to build key
    key = key << 2;
    key |= charVal;
  }

  return key;
}

// Doesn't make a perf difference
// uint64_t updateKey(uint64_t oldKey, char newChar) {
//   // Zero out the highest bit
//   oldKey &= ~(((uint64_t)1) << (2*(K_LEN)-1));
//   oldKey &= ~(((uint64_t)1) << (2*(K_LEN)-2));
//   // Shift
//   oldKey = oldKey << 2;
//   // Add new character
//   oldKey |= charInd(newChar);
//   return oldKey;
// }

// There are undoubtably better ways to manage memory here, but let's start with
// this.
void preloadFile(std::vector<char>*& entries, std::string filename) {
  cout << "Preloading file: " << filename << endl;

  std::ifstream inFile(filename);
  if (!inFile) {
    cout << "Could not load file." << endl;
    exit(-1);
  }

  entries = new std::vector<char>();
  for (std::string line; getline(inFile, line);) {
    // Skip comments and blank lines
    if (line.size() == 0 || line[0] == ';' || line[0] == '>') {
      continue;
    }

    // Copy all GATC entries
    for (char& c : line) {
      if (c == 'G' || c == 'A' || c == 'T' || c == 'C') {
        entries->push_back(c);
      }
    }
  }

  cout << "Loaded " << entries->size() << " entries." << endl;
  printf("  --> aka %.3f billion characters\n", (double)entries->size() / 1e9);
  // for(char& c : *entries) {
  //   cout << c << endl;
  // }
}

bool add1(size_t& x) {
  x++;
  return true;
}

void countSequences(Table& tbl, const char* sequence, size_t startInd,
                    size_t endInd) {
  // First index special to initialize key
  // uint64_t key = buildKey(sequence + startInd);
  // tbl.upsert(key, add1, 1);

  // Walk the sequence hashing each substring
  for (size_t i = startInd; i < endInd; i++) {
    // Build the key
    uint64_t key = buildKey(sequence + i);

    // Increment
    tbl.upsert(key, add1, 1);
  }
}

// Doesn't make a perf difference
// void countSequences(Table& tbl, const char* sequence, size_t startInd,
//                     size_t endInd) {

//   // First index special to initialize key
//   uint64_t key = buildKey(sequence + startInd);
//   tbl.upsert(key, add1, 1);

//   // Walk the sequence hashing each substring
//   for (size_t i = startInd+1; i < endInd; i++) {
//     // Build the key
//     // std::string key(sequence + i, sequence + i + K_LEN);
//     key = updateKey(key, sequence[i]);

//     // Increment
//     tbl.upsert(key, add1, 1);
//   }
// }

void serialResultCheck(Table& tbl, const char* sequence, size_t endInd) {
  // Good old std::unordered_map for safety check
  std::unordered_map<std::string, size_t> evilOldSchoolTable;
  for (size_t i = 0; i < endInd; i++) {
    std::string key(sequence + i, sequence + i + K_LEN);

    if (evilOldSchoolTable.find(key) == evilOldSchoolTable.end()) {
      evilOldSchoolTable[key] = 0;
    }
    evilOldSchoolTable[key]++;
  }

  // Statistics while we're at it
  size_t smallCountLim = 20;
  std::vector<size_t> smallCount(smallCountLim, 0);
  std::string maxStr;
  size_t maxStrCount = 0;
  size_t errCount = 0;

  // Walk the map
  for (const auto& pair : evilOldSchoolTable) {
    // Key at this index
    uint64_t key = buildKey(pair.first.c_str());

    // Read the value in the table
    uint64_t val;
    bool found = tbl.read(key, val);

    // If this key is not in the table, something is is very wrong
    // (tables seem to return 0 on miss)
    if (!found) {
      cerr << "ERROR: table is missing key for " << pair.first << " ["
           << std::bitset<2 * K_LEN>(key) << "]" << endl;
      errCount++;
    } else if (val != pair.second) {
      cerr << "ERROR: table has wrong value for " << pair.first << " ["
           << std::bitset<2 * K_LEN>(key) << "]. Is " << val << ", should be "
           << pair.second << "." << endl;
      errCount++;
    }

    // Update statistics
    if (val < smallCountLim) {
      smallCount[val]++;
    }
    if (val > maxStrCount) {
      maxStrCount = val;
      maxStr = pair.first;
    }

    if (errCount > 1000) {
      cout << "Your hash table is very wrong, I'm done with this." << endl;
      return;
    }
  }

  // Print statistics
  cout << "=== Statistics:" << endl;
  cout << "  Low-frequencies kmers:" << endl;
  for (int c = 0; c < smallCountLim; c++) {
    printf("    There were %15zu kmers which occured %d times\n", smallCount[c],
           c);
  }
  cout << "  Most common kmer was " << maxStr << ", with " << maxStrCount
       << " occurences." << endl;
}

int main(int argc, char** argv) {
  try {
    // Parse parameters
    if (argc != 2) {
      cout << "Usage: ./kmer FASTA_FILE" << endl;
      exit(-1);
    }

    std::string filename(argv[1]);

    // Load the file
    std::vector<char>* seq;
    preloadFile(seq, filename);
    char* seqArr = reinterpret_cast<char*>(&(*seq)[0]);

    // Compute a very explicit distribution of work
    std::vector<size_t> startInds,
        endInds;  // (start is inclusize, end is exclusive)
    size_t Nchar = seq->size();
    size_t nSubSeq = Nchar - K_LEN + 1;
    size_t nPer = nSubSeq / g_threads + 1;
    size_t currStart = 0;
    cout << "Thread work distribution for " << g_threads << " threads:" << endl;
    for (size_t iT = 0; iT < g_threads; iT++) {
      size_t nextStart = std::min(currStart + nPer, nSubSeq);
      startInds.push_back(currStart);
      endInds.push_back(nextStart);
      printf("  Thread %3zu: [%15zu,%15zu)\n", iT, currStart, nextStart);
      currStart = nextStart;
    }

    // Create and size the table
    Table tbl(Nchar, g_threads);

    // Run the counting "algorithm"
    std::cout << "Running operations\n";
    std::vector<std::thread> worker_threads(g_threads);
    auto start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < g_threads; ++i) {
      worker_threads[i] = std::thread(countSequences, std::ref(tbl), seqArr,
                                      startInds[i], endInds[i]);
    }
    for (auto& t : worker_threads) {
      t.join();
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    double seconds_elapsed =
        std::chrono::duration_cast<std::chrono::duration<double>>(end_time -
                                                                  start_time)
            .count();

    printf("Hash time: %.3f seconds\n", seconds_elapsed);

    // === Optional safety and sanity checks
    serialResultCheck(tbl, seqArr, nSubSeq);

  } catch (const std::exception& e) {
    std::cerr << e.what();
    std::exit(1);
  }
}
