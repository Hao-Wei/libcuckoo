/* For each table to support, we define a wrapper class which holds the table
 * and implements all of the benchmarked operations. Below we list all the
 * methods each wrapper must implement.
 *
 * constructor(size_t n) // n is the initial capacity
 * template <typename K, typename V>
 * bool read(const K& k, V& v) const
 * template <typename K, typename V>
 * bool insert(const K& k, const V& v)
 * template <typename K>
 * bool erase(const K& k)
 * template <typename K, typename V>
 * bool update(const K& k, const V& v)
 * template <typename K, typename V>
 * void upsert(const K& k, Updater fn, const V& v)
 */

#ifndef _UNIVERSAL_TABLE_WRAPPER_HH
#define _UNIVERSAL_TABLE_WRAPPER_HH

#include <atomic>
#include <memory>
#include <utility>

#ifndef KEY
#error Must define KEY symbol as valid key type
#endif

#ifndef VALUE
#error Must define VALUE symbol as valid value type
#endif

#ifdef TRACKING_ALLOCATOR
std::atomic<size_t> universal_benchmark_current_bytes_allocated =
    ATOMIC_VAR_INIT(0);

template <template <typename> class WrappedAlloc, typename T> class Allocator {
private:
  using traits = std::allocator_traits<WrappedAlloc<T>>;

public:
  using value_type = typename traits::value_type;
  using pointer = typename traits::pointer;
  using const_pointer = typename traits::const_pointer;
  using reference = value_type &;
  using const_reference = const value_type &;
  using void_pointer = typename traits::void_pointer;
  using const_void_pointer = typename traits::const_void_pointer;
  using size_type = typename traits::size_type;
  using difference_type = typename traits::difference_type;

  template <typename U> struct rebind {
    using other = Allocator<WrappedAlloc, U>;
  };

  Allocator() {}

  template <typename U>
  Allocator(const Allocator<WrappedAlloc, U> &alloc)
      : allocator_(alloc.allocator_) {}

  pointer address(reference x) const { return std::addressof(x); }

  const_pointer const_address(const_reference x) const {
    return std::addressof(x);
  }

  pointer allocate(size_type n, const_void_pointer hint = nullptr) {
    universal_benchmark_current_bytes_allocated.fetch_add(
        n * sizeof(value_type), std::memory_order_acq_rel);
    return traits::allocate(allocator_, n, hint);
  }

  void deallocate(pointer p, size_type n) {
    universal_benchmark_current_bytes_allocated.fetch_sub(
        n * sizeof(value_type), std::memory_order_acq_rel);
    traits::deallocate(allocator_, p, n);
  }

  size_type max_size() const { return traits::max_size(allocator_); }

  template <typename... Args> void construct(pointer p, Args &&... args) {
    traits::construct(allocator_, p, std::forward<Args>(args)...);
  }

  void destroy(pointer p) { traits::destroy(allocator_, p); }

  bool operator==(const Allocator &) { return true; }

  bool operator!=(const Allocator &) { return false; }

  typename traits::allocator_type allocator_;
};

class Sampler {
public:
  Sampler(size_t num_ops)
      : total_samples(num_ops / sampling_period), current_iter(0),
        samples(total_samples) {}

  void iter() {
    if ((current_iter & sampling_period_mask) == 0) {
      samples[current_iter >> sampling_period_power] =
          universal_benchmark_current_bytes_allocated.load(
              std::memory_order_acquire);
    }
    ++current_iter;
  }

  void store(std::vector<size_t> &v) const { v = samples; }

private:
  // We sample every 2^17 iterations (which is around 100K). Keeping the
  // sampling period a power of 2 makes it easy to check whether we need to
  // add a sample.
  static constexpr size_t sampling_period_power = 17;
  static constexpr size_t sampling_period = 1UL << 17;
  static constexpr size_t sampling_period_mask = sampling_period - 1;
  const size_t total_samples;
  size_t current_iter;
  std::vector<size_t> samples;
};

#else
template <template <typename> class WrappedAlloc, typename T>
using Allocator = WrappedAlloc<T>;

class Sampler {
public:
  Sampler(size_t) {}
  void iter() {}
  void store(std::vector<size_t> &v) const { v = samples; }

private:
  std::vector<size_t> samples;
};

#endif

#ifdef LIBCUCKOO
#define TABLE "LIBCUCKOO"
#define TABLE_TYPE "cuckoohash_map"
#include <hashtables/libcuckoo/cuckoohash_map.hh>

class Table {
public:
  Table(size_t n, size_t n_threads) : tbl(2.5*n) {}

  template <typename K, typename V> bool read(const K &k, V &v) const {
    return tbl.find(k, v);
  }

  template <typename K, typename V> bool insert(const K &k, const V &v) {
    return tbl.insert(k, v);
  }

  template <typename K> bool erase(const K &k) { return tbl.erase(k); }

  template <typename K, typename V> bool update(const K &k, const V &v) {
    return tbl.update(k, v);
  }

  template <typename K, typename Updater, typename V>
  void upsert(const K &k, Updater fn, const V &v) {
    tbl.upsert(k, fn, v);
  }

private:
  cuckoohash_map<KEY, VALUE, std::hash<KEY>, std::equal_to<KEY>,
                 Allocator<std::allocator, std::pair<const KEY, VALUE>>>
      tbl;
};

#else
//#error Must define LIBCUCKOO
#endif

#ifdef DHASH
#define TABLE "DHASH"
#define TABLE_TYPE "deterministic_map"
#define MCX16 1
#include <hashtables/deterministicHash/deterministicHash.h>


//K and V have to be ints for now
template <typename K, typename V>
struct hashCustomPair {
  typedef pair<K, V> eType;
  typedef K kType;
  eType empty() {return make_pair(-1, -1);}
  kType getKey(eType v) {return v.first;}
  unsigned int hash(kType v) {return utils::hash(v);}
  int cmp(kType v, kType b) {return (v > b) ? 1 : ((v == b) ? 0 : -1);}
  bool replaceQ(eType v, eType b) {return 0;}
};

class Table {
public:
  Table(size_t n, size_t n_threads) : 
  	hashStruct(hashCustomPair<KEY, VALUE>()), 
  	tbl(n, hashStruct), 
  	empty(hashStruct.empty()) {}

  template <typename K, typename V> bool read(const K &k, V &v) {
  	pair<KEY,VALUE> kv = tbl.find(k);
  	if(kv == empty) return 0;
  	v = kv.second; 
    return 1;
  }

  template <typename K, typename V> bool insert(const K &k, const V &v) {
  	pair<KEY,VALUE> kv = make_pair(k, v);
    return tbl.insert(kv);
  }

  template <typename K> bool erase(const K &k) { return 0; }

  template <typename K, typename V> bool update(const K &k, const V &v) {
    return 0;
  }

  template <typename K, typename Updater, typename V>
  void upsert(const K &k, Updater fn, const V &v) {
  	return;
  }

  d_map<hashCustomPair<KEY, VALUE>, int32_t > tbl;
  hashCustomPair<KEY, VALUE> hashStruct;
  pair<KEY, VALUE> empty;

};

#else
//#error Must define DHASH
#endif


#ifdef NDHASH
#define TABLE "NDHASH"
#define TABLE_TYPE "nondeterministic_map"
#define MCX16 1
#include <hashtables/ndHash/ndHash.h>


//K and V have to be ints for now
template <typename K, typename V>
struct hashCustomPair {
  typedef pair<K, V> eType;
  typedef K kType;
  eType empty() {return make_pair(-1, -1);}
  kType getKey(eType v) {return v.first;}
  unsigned int hash(kType v) {return utils::hash(v);}
  int cmp(kType v, kType b) {return (v > b) ? 1 : ((v == b) ? 0 : -1);}
  bool replaceQ(eType v, eType b) {return 0;}
};

class Table {
public:
  Table(size_t n, size_t n_threads) : 
  	hashStruct(hashCustomPair<KEY, VALUE>()), 
  	tbl(n, hashStruct), 
  	empty(hashStruct.empty()) {}

  template <typename K, typename V> bool read(const K &k, V &v) {
  	pair<KEY,VALUE> kv = tbl.find(k);
  	if(kv == empty) return 0;
  	v = kv.second; 
    return 1;
  }

  template <typename K, typename V> bool insert(const K &k, const V &v) {
  	pair<KEY,VALUE> kv = make_pair(k, v);
    return tbl.insert(kv);
  }

  template <typename K> bool erase(const K &k) { return 0; }

  template <typename K, typename V> bool update(const K &k, const V &v) {
    return 0;
  }

  template <typename K, typename Updater, typename V>
  void upsert(const K &k, Updater fn, const V &v) {
  	pair<KEY,VALUE> kv = make_pair(k, v);
    tbl.upsert(kv);
  }

  nd_map<hashCustomPair<KEY, VALUE>, int32_t > tbl;
  hashCustomPair<KEY, VALUE> hashStruct;
  pair<KEY, VALUE> empty;

};

#else
//#error Must define NDHASH
#endif



#ifdef HOPSCOTCH_V2
#define TABLE "HOPSCOTCH_V2"
#define TABLE_TYPE "hopscotch_v2_map"
#define MCX16 1
#include <hashtables/hopscotch_v2/hopscotchHash.h>


//K and V have to be ints for now
template <typename K, typename V>
struct hashCustomPair {
  typedef pair<K, V> eType;
  typedef K kType;
  eType empty() {return make_pair(-1, -1);}
  kType getKey(eType v) {return v.first;}
  unsigned int hash(kType v) {return utils::hash(v);}
  int cmp(kType v, kType b) {return (v > b) ? 1 : ((v == b) ? 0 : -1);}
  bool replaceQ(eType v, eType b) {return 0;}
};

class Table {
public:
  Table(size_t n, size_t n_threads) : 
  	hashStruct(hashCustomPair<KEY, VALUE>()), 
  	tbl(n, hashStruct, 10000), 
  	empty(hashStruct.empty()) {}

  template <typename K, typename V> bool read(const K &k, V &v) {
  	pair<KEY,VALUE> kv = tbl.find(k);
  	if(kv == empty) return 0;
  	v = kv.second; 
    return 1;
  }

  template <typename K, typename V> bool insert(const K &k, const V &v) {
  	pair<KEY,VALUE> kv = make_pair(k, v);
    return tbl.insert(kv);
  }

  template <typename K> bool erase(const K &k) { return 0; }

  template <typename K, typename V> bool update(const K &k, const V &v) {
    return 0;
  }

  template <typename K, typename Updater, typename V>
  void upsert(const K &k, Updater fn, const V &v) {
  	pair<KEY,VALUE> kv = make_pair(k, v);
    tbl.upsert(kv);
  }

  hopscotch_map<hashCustomPair<KEY, VALUE>, int32_t > tbl;
  hashCustomPair<KEY, VALUE> hashStruct;
  pair<KEY, VALUE> empty;

};

#else
//#error Must define NDHASH
#endif


#ifdef HOPSCOTCH
#define TABLE "HOPSCOTCH"
#define TABLE_TYPE "hopscotch_map"
#define INTEL64 1
#include "hashtables/hopscotch/framework/cpp_framework.h"
#include "hashtables/hopscotch/test/Configuration.h"
#include "hashtables/hopscotch/data_structures/BitmapHopscotchHashMap.h"
#include "hashtables/hopscotch/data_structures/HopscotchHashMap.h"

class HASH_INT {
public:
	//you must define the following fields and properties
	static const unsigned int _EMPTY_HASH;
	static const unsigned int _BUSY_HASH;
	static const uint64_t _EMPTY_KEY;
	static const uint64_t _EMPTY_DATA;

  inline_ static unsigned int Calc(uint64_t key) {
  		unsigned int ret = std::hash<uint64_t>{}(key);
  		if(ret < 2) ret = 2;
    	return ret;
	}

	inline_ static bool IsEqual(uint64_t left_key, uint64_t right_key) {
		return left_key == right_key;
	}

	inline_ static void relocate_key_reference(uint64_t volatile& left, const uint64_t volatile& right) {
		left = right;
	}

	inline_ static void relocate_data_reference(uint64_t volatile& left, const uint64_t volatile& right) {
		left = right;
	}
};

const unsigned int HASH_INT::_EMPTY_HASH = 0;
const unsigned int HASH_INT::_BUSY_HASH  = 1;
const uint64_t HASH_INT::_EMPTY_KEY  = ((uint64_t)1) << 60;
const uint64_t HASH_INT::_EMPTY_DATA = ((uint64_t)1) << 60;

class Table {
public:
  Table(size_t n, size_t n_threads) :
  	tbl(2*n, 10000*n_threads) {}

  template <typename K, typename V> bool read(const K &k, V &v) {
  	if(tbl.containsKey(k))
  	{
  		v = tbl.putIfAbsent(k, 0);
  		return 1;
  	}
  	else
    	return 0;
    
  }

  template <typename K, typename V> bool insert(const K &k, const V &v) {
  	V tv = tbl.putIfAbsent(k, v);
    return tv == HASH_INT::_EMPTY_DATA;
  }

  template <typename K> bool erase(const K &k) { return 0; }

  template <typename K, typename V> bool update(const K &k, const V &v) {
    return 0;
  }

  template <typename K, typename Updater, typename V>
  void upsert(const K &k, Updater fn, const V &v) {
  	tbl.upsert(k, v);
  }

  HopscotchHashMap<uint64_t, uint64_t, HASH_INT, CMDR::TTASLock, CMDR::Memory> tbl;


};

#else
//#error Must define NDHASH
#endif


#ifdef FOLKLORE
#define TABLE "FOLKLORE"
#define TABLE_TYPE "folklore_map"
#include <hashtables/growt/data-structures/growtable.h>
#include <hashtables/growt/data-structures/returnelement.h>
#define MURMUR2
#include <hashtables/growt/utils/hashfct.h>
#include <hashtables/growt/utils/alignedallocator.h>
#include <hashtables/growt/data-structures/definitions.h>
#include <hashtables/growt/example/update_fcts.h>

class Table {
public:
  Table(size_t n, size_t n_threads) : 
  	tbl(n) {}

  template <typename K, typename V> bool read(const K &k, V &v) {
  	auto kv = tbl.find(k);
  	if(!kv) return 0;
  	v = kv.second; 
    return 1;
  }

  template <typename K, typename V> bool insert(const K &k, const V &v) {
    return successful(tbl.insert(k, v));
  }

  template <typename K> bool erase(const K &k) { return 0; }

  template <typename K, typename V> bool update(const K &k, const V &v) {
    return 0;
  }

  template <typename K, typename Updater, typename V>
  void upsert(const K &k, Updater fn, const V &v) {
    tbl.insertOrUpdate(k, v, growt::example::Increment());
  }

  growt::folklore<murmur2_hasher, growt::AlignedAllocator<> > tbl;


};

#else
//#error Must define NDHASH
#endif


#ifdef PARALLELDP
#define TABLE "PARALLELDP"
#define TABLE_TYPE "paralleldp_map"
#include <hashtables/paralleldp/paralleldp.h>

class Table {
public:
  Table(size_t n, size_t n_threads) : 
  	tbl(n) {}

  template <typename K, typename V> bool read(const K &k, V &v) {
    tbl.find(k, v);
  }

  template <typename K, typename V> bool insert(const K &k, const V &v) {
    return tbl.insert(k, v);
  }

  template <typename K> bool erase(const K &k) { return 0; }

  template <typename K, typename V> bool update(const K &k, const V &v) {
    return tbl.update(k, v);
  }

  template <typename K, typename Updater, typename V>
  void upsert(const K &k, Updater fn, const V &v) {
    tbl.upsert(k, v);
  }

  parallel_map<KEY,VALUE> tbl;


};

#else
//#error Must define PARALLELDP
#endif


#endif // _UNIVERSAL_TABLE_WRAPPER_HH
