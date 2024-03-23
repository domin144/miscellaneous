/*
 * SPDX-FileCopyrightText: 2016 Dominik Wójt <domin144@o2.pl>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cache.h"

#include <iostream>
#include <stdexcept>
#include <random>
#include <utility>
#include <chrono>

template<typename Iterator>
void randomize(
        Iterator begin,
        Iterator end,
        const int seed,
        const int range = 1000000)
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(0, range - 1);
    for(auto i = begin; i != end; ++i)
    {
        *i = dis(gen);
    }
}

template<typename TData>
class Primitive_cache
{
public:
    using Data = TData;
    using Key = int;
    using Time = int;

private:
    struct Slot
    {
        Key m_key = -1;
        Time m_age = -1;
        Data *m_data = nullptr;
    };

private:
    std::vector<Slot> m_slots;

public:
    Primitive_cache(const Index cache_size) :
        m_slots(cache_size)
    { }

    Index get_cache_size() const
    {
        return m_slots.size();
    }

    Data *get(const Key key)
    {
        return get_or_update(key, false);
    }

    void update_key(const Key key)
    {
        Data *data = get_or_update(key, true);
        my_assert(data, "trying to update absent key");
    }

    /* more efficient if done at once */
    Data *get_and_update(const Key key)
    {
        return get_or_update(key, true);
    }

    Data *pop()
    {
        auto iter =
                std::max_element(
                    m_slots.begin(),
                    m_slots.end(),
                    [](const Slot lhs, const Slot rhs)
                    {
                        return lhs.m_age < rhs.m_age;
                    });
        Slot &slot = *iter;

        my_assert(slot.m_data, "trying to pop empty cache");

        Data *const result = slot.m_data;
        slot = Slot();

        return result;
    }

    void push(const Key key, Data *const data)
    {
        auto iter =
                std::find_if(
                    m_slots.begin(),
                    m_slots.end(),
                    [key](const Slot slot)
                    {
                        return slot.m_key == key;
                    });
        if(iter != m_slots.end())
        {
            my_assert(false, "trying to push data already present in cache");
        }
        else
        {
            auto empty_iter =
                    std::find_if(
                        m_slots.begin(),
                        m_slots.end(),
                        [](const Slot slot)
                        {
                            return slot.m_data == nullptr;
                        });
            my_assert(
                        empty_iter != m_slots.end(),
                        "trying to push more data than cache can hold");

            Slot &slot = *empty_iter;
            slot.m_data = data;
            slot.m_key = key;
            update(slot);
        }
    }

    bool full() const
    {
        auto empty_iter =
                std::find_if(
                    m_slots.begin(),
                    m_slots.end(),
                    [](const Slot slot)
                    {
                        return slot.m_data == nullptr;
                    });
        return empty_iter == m_slots.end();
    }

private:
    Data *get_or_update(const Key key, bool do_update)
    {
        auto iter =
                std::find_if(
                    m_slots.begin(),
                    m_slots.end(),
                    [key](const Slot slot)
                    {
                        return slot.m_key == key;
                    });
        if(iter != m_slots.end())
        {
            Slot &slot = *iter;
            if(do_update)
                update(slot);
            return slot.m_data;
        }
        else
        {
            return nullptr;
        }
    }

    void update(Slot &slot)
    {
        increment_time();
        slot.m_age = 0;
    }

    void increment_time()
    {
        for(Slot &slot : m_slots)
            if(slot.m_data)
                slot.m_age++;
    }
};

int main() try
{
    std::cout << "Hello!\n";

    {
        const Index n = 1000000;
        for(int seed = 15; seed < 20; seed++)
        {
            std::vector<int> test_std(n);
            randomize(test_std.begin(), test_std.end(), seed);
            std::vector<int> test_custom = test_std;

            {
                Timer std_timer("custom");
                my_make_heap(
                            test_custom.begin(),
                            test_custom.end(),
                            std::less<int>());
                my_sort_heap(
                            test_custom.begin(),
                            test_custom.end(),
                            std::less<int>());
            }

            {
                Timer std_timer("std");
                std::make_heap(
                            test_std.begin(),
                            test_std.end(),
                            std::less<int>());
                std::sort_heap(
                            test_std.begin(),
                            test_std.end(),
                            std::less<int>());
            }

            {
                Timer std_timer("custom");
                my_make_heap(
                            test_custom.begin(),
                            test_custom.end(),
                            std::less<int>());
                my_sort_heap(
                            test_custom.begin(),
                            test_custom.end(),
                            std::less<int>());
            }

            {
                Timer std_timer("std");
                std::make_heap(
                            test_std.begin(),
                            test_std.end(),
                            std::less<int>());
                std::sort_heap(
                            test_std.begin(),
                            test_std.end(),
                            std::less<int>());
            }

            if(test_std == test_custom)
                std::cerr << "test OK\n";
            else
                std::cerr << "test fail\n";
        }
    }


    {
        const int cache_size = 10000;
        const int key_range = 20000;

        int resources[cache_size] = {1000, 1001, 1002};

        std::vector<int> tests(5 * cache_size);
        randomize(tests.begin(), tests.end(), 15, key_range);

        std::vector<int *> advanced_results;
        std::vector<int *> primitive_results;
        advanced_results.reserve(5 * cache_size);
        primitive_results.reserve(5 * cache_size);

        {
            Timer timer("advanced");
            Cache<int, int> advanced_cache(cache_size);
            for(int test : tests)
            {
                int *sought_resource =
                        advanced_cache.get_and_update(test);
                advanced_results.push_back(sought_resource);

                if(sought_resource)
                {
                    ;
                }
                else
                {
                    if(advanced_cache.is_full())
                        sought_resource = advanced_cache.pop();
                    else
                        sought_resource = &resources[test % cache_size];

                    advanced_cache.push(test, sought_resource);
                }
            }
        }

        {
            Timer timer("primitive");
            Primitive_cache<int> primitive_cache(cache_size);
            for(int test : tests)
            {
                int *sought_resource =
                        primitive_cache.get_and_update(test);
                primitive_results.push_back(sought_resource);

                if(sought_resource)
                {
                    ;
                }
                else
                {
                    if(primitive_cache.full())
                        sought_resource = primitive_cache.pop();
                    else
                        sought_resource = &resources[test % cache_size];

                    primitive_cache.push(test, sought_resource);
                }
            }
        }

        if(advanced_results == primitive_results)
            std::cerr << "Cache test passed!\n";
        else
            std::cerr << "Cache test failed!\n";
    }

    {
        const int cache_size = 3;
        Cache<int, int> cache_test(cache_size);

        int resources[cache_size] = {1000, 1001, 1002};
        cache_test.push(3, &resources[0]);
        cache_test.push(7, &resources[1]);
        cache_test.push(5, &resources[2]);


        std::pair<int, int *> tests[] = {
            {3, &resources[0]},
            {4, nullptr},
            {7, nullptr},
            {4, &resources[1]}
        };
        for(auto test : tests)
        {
            int *sought_resource =
                    cache_test.get_and_update(test.first);
            my_assert(
                        sought_resource == test.second,
                        "unexpected resource returned in test");
            if(sought_resource)
            {
                ;/* resource found, just use it */
            }
            else
            {
                /* resource not found in the cache */
                /* need to produce the resource, fortunately we can reuse memory
                 * allocated for the resource which is now removed from the
                 * cache */
                sought_resource = cache_test.pop();
                cache_test.push(test.first, sought_resource);
            }
            /* use the resource */
        }

        bool test_passed = false;
        try
        {
            cache_test.push(2, &resources[0]);
        }
        catch(std::runtime_error &e)
        {
            test_passed = true;
        }
        my_assert(test_passed, "push over cache size succeeded!");
    }

    std::cout << "Bye!\n";
    return 0;
}
catch(std::runtime_error &e)
{
    std::cerr << "runtime error caught: " << e.what() << std::endl;
    return -1;
}
catch(...)
{
    std::cerr << "unknown exception caught: " << std::endl;
    return -1;
}
