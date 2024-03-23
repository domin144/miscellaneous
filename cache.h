/*
 * SPDX-FileCopyrightText: 2016 Dominik Wójt <domin144@o2.pl>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CACHE_H
#define CACHE_H

#include "trees_and_heaps.h"

#include <map>
#include <limits>

/*----------------------------------------------------------------------------*/
/* Cache class.
 *
 * Manages set of pointers to TData. Cache holds 0 to cache_size pointers at
 * any given time.
 *
 * Pointer not referenced for the most iterations is removed first. Pointers are
 * referenced by push, get_and_update and update_key. Pointer can be retrieved
 * without registering a reference using get function.
 *
 * Pointer push, pop, reference and get have logarithmic complexity in
 * the number of pointers being held. */
/*----------------------------------------------------------------------------*/
template<typename TKey, typename TData>
class Cache
{
public:
    using Key = TKey;
    using Data = TData;
    using Time = int;

private:
    struct Slot
    {
        Index m_index_in_heap = -1;
        Time m_last_reference = 0;
        Data *m_data = nullptr;
        Key m_key;
    };

    struct Heap_element
    {
        Slot *m_slot;

        /* comparison by time of last reference */
        bool operator<(const Heap_element& rhs) const
        {
            return m_slot->m_last_reference < rhs.m_slot->m_last_reference;
        }

        bool operator>(const Heap_element& rhs) const
        {
            return rhs < *this;
        }

        friend void swap(Heap_element &lhs, Heap_element &rhs)
        {
            std::swap(lhs.m_slot, rhs.m_slot);
            std::swap(lhs.m_slot->m_index_in_heap, rhs.m_slot->m_index_in_heap);
        }
    };

private:
    std::vector<Slot> m_slots;
    std::map<Key, Slot *> m_map;
    Heap<Heap_element, std::greater<Heap_element> > m_heap;
    std::vector<Slot *> m_free_slots;
    Time m_time;

public:
    Cache(const Index cache_size) :
        m_slots(cache_size),
        m_time(0)
    {
        m_heap.reserve(cache_size);

        m_free_slots.reserve(cache_size);
        for(Slot &slot : m_slots)
            m_free_slots.push_back(&slot);
    }

    Index get_cache_size() const
    {
        return m_slots.size();
    }

    Index get_used_slots_count() const
    {
        return m_heap.get_size();
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
        my_assert(m_map.size() != 0, "trying to pop empty cache");
        const Heap_element heap_element = m_heap.pop();
        Slot *const slot = heap_element.m_slot;
        my_assert(
                    slot->m_index_in_heap == m_heap.get_size(),
                    "invalid index in heap");

        /* remove key from the map */
        const Key key = slot->m_key;
        m_map.erase(key);

        Data *const result = slot->m_data;

        /* clear the values in slot */
        *slot = Slot();
        m_free_slots.push_back(slot);

        const Index map_size = m_map.size();
        my_assert(map_size == m_heap.get_size(), "heap - map size mismatch");

        return result;
    }

    void push(const TKey key, Data *const data)
    {
        if(m_map.find(key) != m_map.end())
        {
            my_assert(false, "trying to push data already present in cache");
        }
        else
        {
            const Index current_size = m_map.size();
            my_assert(
                        current_size < get_cache_size(),
                        "trying to push more data than cache can hold");

            Slot *const slot = m_free_slots.back();
            m_free_slots.pop_back();

            m_map[key] = slot;

            slot->m_data = data;
            slot->m_key = key;

            slot->m_index_in_heap = m_heap.get_size();
            Heap_element heap_element;
            heap_element.m_slot = slot;
            m_heap.push(heap_element);

            update(slot);
        }
    }

    bool is_full() const
    {
        return m_free_slots.empty();
    }

    bool is_empty() const
    {
        return m_heap.is_empty();
    }

private:
    Data *get_or_update(const Key key, bool do_update)
    {
        auto iter = m_map.find(key);
        if(iter != m_map.end())
        {
            Slot *slot = iter->second;
            if(do_update)
                update(slot);
            return slot->m_data;
        }
        else
        {
            return nullptr;
        }
    }

    void update(Slot *slot)
    {
        slot->m_last_reference = m_time;
        m_heap.update(slot->m_index_in_heap);
        increment_time();
    }

    void increment_time()
    {
        if(m_time == std::numeric_limits<Time>::max())
        {
            Time span = m_map.size();
            Time oldest_reference = m_time - span + 1;
            for(Slot &slot : m_slots)
                if(slot.m_data)
                    slot.m_last_reference -= oldest_reference;
            m_time -= oldest_reference;
        }
        m_time++;
    }
};

#endif /* CACHE_H */
