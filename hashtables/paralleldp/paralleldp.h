#ifndef A_HASH_INCLUDED
#define A_HASH_INCLUDED

#include <iostream>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

using namespace std;


#define USE_GOOD_HASH
#define ALLOW_UPDATE

/* linear probing step size */
#define OAHTTSLF_PROBE_STEP 1

/* marks unused slot (a key cannot have this value) */
#define OAHTTSLF_EMPTY_KEY 0

/* marks unset value (a value cannot have this value) */
#define OAHTTSLF_EMPTY_VALUE 0


/*****************************************************************************
 *
 * types
 *
 *****************************************************************************/

template <class KeyClass, class ValueClass>
class parallel_map {
 private:
  typedef struct oahttslf_entry_s
  {
      KeyClass key;   /* TODO need to be able to have a SET (128 bit) key */
      ValueClass value;
  } oahttslf_entry_t;


  size_t table_size;
  oahttslf_entry_t *hashtable;

  template<typename K>
  unsigned int hash_function(K key) {
    unsigned int i;

    key = (~key) + (key << 18); /* key = (key << 18) - key - 1; */
    key = key ^ (key >> 31);
    key = key * 21; /* key = (key + (key << 2)) + (key << 4); */
    key = key ^ (key >> 11);
    key = key + (key << 6);
    key = key ^ (key >> 22);

    i = key & (table_size - 1); /* depends on table_size being 2^n */

    return i;
  }


  /*
   * oahttslf_getent()
   *
   * Get the entry for a key from the hashtable
   *
   * Parameters:
   *     key -  key to look up
   *     vacant - (OUT) TRUE if reutrn pointer to entry for key
   *                    is not currently occupied by key
   *  Return value:
   *     pointer to entry with key, or for key (but currently empty) in hashtable
   *     or NULL if hashtable is full
   */
  template<typename K>
  volatile oahttslf_entry_t *oahttslf_getent(K key, bool *vacant)
  {
    unsigned int h;
    volatile oahttslf_entry_t *ent;
    int probes = 0;
    K entkey;

    h = hash_function(key);
    ent = &hashtable[h];
    entkey = ent->key;
    while (probes < table_size - 1 && entkey != key && entkey != OAHTTSLF_EMPTY_KEY)
    {
      ++probes;
      h = (h + OAHTTSLF_PROBE_STEP) & (table_size - 1); /*SIZE must be 2^n*/
      ent = &hashtable[h];
      entkey = ent->key;
    }
    if (probes >= table_size - 1)
      return NULL;
    else if (entkey == OAHTTSLF_EMPTY_KEY)
      *vacant = true;
    else
      *vacant = false;
    return ent;
  }


  template<typename K, typename V>
  bool internal_insert(K key, V value, bool should_update)
  {
    volatile oahttslf_entry_t *ent;
    K entkey;
    V oldvalue;
    bool vacant;

    assert(key != OAHTTSLF_EMPTY_KEY);

    ent = oahttslf_getent(key, &vacant);

    assert(ent);

    oldvalue = ent->value;
    entkey = ent->key;
    if (vacant)
    {
      if( !__sync_bool_compare_and_swap_8(&ent->key, entkey, key)) {
        // Somebody won the race to use this entry. So we try again
        return internal_insert(key, value, should_update);
      }
      if(! __sync_bool_compare_and_swap_8(&ent->value, oldvalue, value)) {
        return false; // Somebody won the race to insert the value
      }
    }
    else 
    {
      if(should_update) {
        /*if (oldvalue == value)  // shortcut to avoid expense of CAS instruction
          return true;
        while((! __sync_bool_compare_and_swap_8(&ent->value, oldvalue, value))) {
          oldvalue = ent->value;
        }*/
        __sync_fetch_and_add(&ent->value, 1);
      } else {
        return false;
      }
    }
    return true;
  }

 public:

  /**
   * @brief      Constructor for the Hashmap. Initializes a table of size size
   *             and initializes all the keys to OAHTTSLF_EMPTY_KEY and
   *             all the values to OAHTTSLF_EMPT_VALUE. Requires the size to
   *             be a power of 2.
   *
   * @param[in]  size   The size of the Hash table
   */
  parallel_map(size_t size) {
    size_t i;
    table_size = size;

    hashtable = (oahttslf_entry_t*) aligned_alloc(sizeof(uint64_t),
                               sizeof(oahttslf_entry_t) * table_size);

    // Initialize all the elements in the hash table
    for(i = 0; i < table_size; i++) {
      hashtable[i].key = OAHTTSLF_EMPTY_KEY;
      hashtable[i].value = OAHTTSLF_EMPTY_VALUE;
    }
  }


  /*
   * find()
   *
   * Get the value for a key from the hashtable
   *
   * Parameters:
   *     key -  key to look up
   *     value - (output) value for key,ony set if TRUE returned.
   * * Return value: *      TRUE if key found, FALSE otherwise.
   */
  template<typename K, typename V>
  bool find(const K key,V &value)
  {
    volatile oahttslf_entry_t *ent;
    bool vacant;

    ent = oahttslf_getent(key, &vacant);
    if (ent && !vacant)
    {
      value = ent->value;
      return true;
    }
    return false;
  }

  template<typename K, typename V>
  bool update(const K key,const V value) {
    volatile oahttslf_entry_t *ent;
    K entkey;
    V oldvalue;
    bool vacant;

    assert(key != OAHTTSLF_EMPTY_KEY);

    ent = oahttslf_getent(key, &vacant);
    if (!ent)
    {
      // Attempting to insert beyond hash table size
      return OAHTTSLF_EMPTY_VALUE;
    }

    assert(vacant == false);
    
    /*(while(! __sync_bool_compare_and_swap_8(&ent->value, oldvalue, value)) {
      oldvalue = ent->value;
    }*/
    __sync_fetch_and_add(&ent->value, 1);

    return true;
  }


  template<typename K, typename V>
  bool insert(K key, V value) {
    return internal_insert(key, value, false);
  }


  template<typename K, typename V>
  bool upsert(K key, V value) {
    return internal_insert(key, value, true);
  }
};
#endif // _A_HASH_INCLUDED
