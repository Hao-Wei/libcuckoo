#include <atomic>
#include <thread>
#include <random>

#define MURMUR2
#include "utils/hashfct.h"

#include "utils/alignedallocator.h"

//////////////////////////////////////////////////////////////
// USING definitions.h (possibly slower compilation)
#include "data-structures/definitions.h"
using Table_t = growt::uaGrow<murmur2_hasher, growt::AlignedAllocator<> >;

//////////////////////////////////////////////////////////////
// EQUAL RESULT without definitions.h (possibly faster compilation)
//
// #include "data-structures/markableelement.h"
// #include "data-structures/circular.h"
// #include "data-structures/strategy/wstrat_user.h"
// #include "data-structures/strategy/estrat_async.h"
// #include "data-structures/growtable.h"
// using Table_t = growt::GrowTable<growt::Circular<growt::MarkableElement,
//                                                  murmur2_hasher,
//                                                  growt::AlignedAllocator<> >,
//                                  growt::WStratUser,
//                                  growt::EStratAsync>
//////////////////////////////////////////////////////////////


// insert all keys between 1 and n into table with <key=i, data=i>
// print message if insert is not successful
void insertions(Table_t& table, size_t n)
{
    // obtain a handle
    auto handle = table.getHandle();

    for (size_t i = 1; i <= n; ++i)
    {
        if (!successful(handle.insert(i,i)))
        {
            std::cout << "unsuccessful insert on key " << i << std::endl;
        }
    }
}

// repeatedly checks "table", if n was inserted (as a key) prints n and stops
void wait_for_k(Table_t& table, size_t k)
{
    // obtain a handle (alternative)
    Table_t::Handle handle{table};

    while (! handle.find(k)) { ; }

    std::cout << "found key " << k << std::endl;
}

// randomly looks for keys between 1 and n
// after n tries it prints the success rate and the average data height
void search_n_and_mean(Table_t& table, size_t n)
{
    auto handle = table.getHandle();

    size_t count = 0;
    size_t sum   = 0;
    murmur2_hasher randomizer{};

    for (size_t i = 0; i < n; ++i)
    {
        size_t key = (randomizer(i) % n) + 1;
        auto temp = handle.find(key);

        if (temp)
        {
            ++count;
            sum += temp.second;
        }
    }

    std::cout << "found " << count << " elements "
              << "with a mean of " << (double(sum)/double(count)) << std::endl;
}

// increases every second inserted number by 42
// this keeps repeating updates until one was successful (waits till key was inserted)
void update_every_scnd(Table_t& table, size_t n)
{
    auto handle = table.getHandle();

    size_t unsuccessful_updates = 0;
    for (int i = 1; i <= int(n); i+=2)
    {
        auto ret = handle.update(i, 42,
                                [](uint64_t& lhs, uint64_t /*key*/, uint64_t rhs)
                                { lhs = lhs + rhs; });
        if (!successful(ret))
        {
            ++unsuccessful_updates;
            i-=2;
        }
    }
}

// check update
void check_update(Table_t& table, size_t n)
{
    auto handle = table.getHandle();

    for (size_t i = 1; i < n; ++i)
    {
        auto temp = handle.find(i);
        if (temp)
        {
            if (i & 1) // was updated!
            {
                if (temp.second != i+42)
                {
                    std::cout << "Unexpected data at key "
                              << i << " found " << temp.second << std::endl;
                }
            }
            else // not updated!
            {
                if (temp.second != i)
                {
                    std::cout << "Unexpected data at key "
                              << i << " found " << temp.second << std::endl;
                }
            }
        }
        else
        {
            std::cout << "Key " << i << " not found!" << std::endl;
        }
    }
}

int main (int, char**)
{

    // set the capacity such that the table has to grow by a factor of 10
    size_t n   = 1000000;
    size_t cap =  100000;
    Table_t hashTable(cap);

    std::thread in_thread  (insertions       , std::ref(hashTable), n);
    std::thread wait_thread(wait_for_k       , std::ref(hashTable), n);
    std::thread up_thread  (update_every_scnd, std::ref(hashTable), n);
    std::thread find_thread(search_n_and_mean, std::ref(hashTable), n);
    in_thread.join();
    wait_thread.join();
    up_thread.join();
    find_thread.join();
    std::cout << "First Round Done!" << std::endl;


    std::thread rfind_thread(search_n_and_mean, std::ref(hashTable), n);
    std::thread check_thread(check_update     , std::ref(hashTable), n);
    rfind_thread.join();
    check_thread.join();
    std::cout << "Second Round Done|" << std::endl;

    return 0;
}
