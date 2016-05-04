/* 
 * Copyright 2016 Dominik Wójt
 * 
 * This file is part of YUVtool.
 * 
 * YUVtool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * YUVtool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with YUVtool.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef TREES_AND_HEAPS_H
#define TREES_AND_HEAPS_H

#include "utils.h"

#include <algorithm>
#include <vector>

/*----------------------------------------------------------------------------*/
template<Index n, typename Iterator>
Iterator get_child_in_n_tree(Iterator begin, Iterator parent, Index child_index)
{
    return begin + 1 + (parent - begin) * n + child_index;
}
/*----------------------------------------------------------------------------*/
template<Index n, typename Iterator>
Iterator get_parent_in_n_tree(Iterator begin, Iterator child)
{
    return begin + (child - begin - 1) / n;
}
/*----------------------------------------------------------------------------*/
template<Index n>
Index get_children_count_in_n_tree_at_level(const Index level)
{
    return cached_power<n>(level);
}
/*----------------------------------------------------------------------------*/
template<Index n, typename Iterator>
Index get_first_child_in_n_tree_at_level(Iterator begin, const Index level)
{
    return begin + (cached_power<n>(level) - 1) / (n - 1);
}
/*----------------------------------------------------------------------------*/
template<typename Iterator>
Iterator get_child_in_heap(Iterator begin, Iterator parent, Index child_index)
{
    return get_child_in_n_tree<2>(begin, parent, child_index);
}
/*----------------------------------------------------------------------------*/
template<typename Iterator>
Iterator get_parent_in_heap(Iterator begin, Iterator child)
{
    return get_parent_in_n_tree<2>(begin, child);
}
/*----------------------------------------------------------------------------*/
template<typename Iterator, typename Compare>
void fix_heap_down(
        Iterator begin,
        Iterator end,
        Iterator fixed,
        Compare compare)
{
    while(true)
    {
        const Iterator child_0 = get_child_in_heap(begin, fixed, 0);
        const Iterator child_1 = child_0 + 1;
        const Iterator big_child =
                child_1 < end && compare(*child_0, *child_1)
                ? child_1
                : (
                      child_0 < end
                      ? child_0
                      : fixed);

        if(compare(*fixed, *big_child))
        {
            std::iter_swap(fixed, big_child);
            fixed = big_child;
            continue;
        }
        else
        {
            break;
        }
    }
}
/*----------------------------------------------------------------------------*/
template<typename Iterator, typename Compare>
void fix_heap_up(
        Iterator begin,
        Iterator end,
        Iterator fixed,
        Compare compare)
{
    while(true)
    {
        const Iterator parent =
                fixed != begin
                ? get_parent_in_heap(begin, fixed)
                : fixed;

        if(compare(*parent, *fixed))
        {
            std::iter_swap(fixed, parent);
            fixed = parent;
            continue;
        }
        else
        {
            break;
        }
    }
}
/*----------------------------------------------------------------------------*/
template<typename Iterator, typename Compare>
void fix_heap(Iterator begin, Iterator end, Iterator fixed, Compare compare)
{
    fix_heap_down(begin, end, fixed, compare);
    fix_heap_up(begin, end, fixed, compare);
}
/*----------------------------------------------------------------------------*/
template<typename Iterator, typename Compare>
void my_pop_heap(Iterator begin, Iterator end, Compare compare)
{
    std::iter_swap(begin, end - 1);
    fix_heap_down(begin, end - 1, begin, compare);
}
/*----------------------------------------------------------------------------*/
template<typename Iterator, typename Compare>
void my_push_heap(Iterator begin, Iterator end, Compare compare)
{
    fix_heap_up(begin, end, end - 1, compare);
}
/*----------------------------------------------------------------------------*/
template<typename Iterator, typename Compare>
void my_make_heap(Iterator begin, Iterator end, Compare compare)
{
    if(end - begin < 2)
        return;
    for(Iterator i = get_parent_in_heap(begin, end - 1); i >= begin; --i)
    {
        fix_heap_down(begin, end, i, compare);
    }
}
/*----------------------------------------------------------------------------*/
template<typename Iterator, typename Compare>
void my_sort_heap(Iterator begin, Iterator end, Compare compare)
{
    while(begin != end)
    {
        my_pop_heap(begin, end, compare);
        --end;
    }
}
/*----------------------------------------------------------------------------*/
template<typename TValue, typename TCompare = std::less<TValue> >
class Heap
{
public:
    using Compare = TCompare;
    using Value = TValue;

private:
    Compare m_compare;
    std::vector<Value> m_array;

public:
    Heap(const TCompare &compare = TCompare()) :
        m_compare(compare)
    { }

    Value pop()
    {
        my_assert(get_size() > 0, "poping empty heap");
        my_pop_heap(m_array.begin(), m_array.end(), m_compare);
        Value result = m_array.back();
        m_array.pop_back();
        return result;
    }

    void push(const Value &value)
    {
        m_array.push_back(value);
        my_push_heap(m_array.begin(), m_array.end(), m_compare);
    }

    void update(const Index index)
    {
        fix_heap(
                    m_array.begin(),
                    m_array.end(),
                    m_array.begin() + index,
                    m_compare);
    }

    void reset()
    {
        m_array.clear();
    }

    Index get_size() const
    {
        return m_array.size();
    }

    void reserve(const Index size)
    {
        m_array.reserve(size);
    }

    bool is_empty() const
    {
        return m_array.empty();
    }
};
/*----------------------------------------------------------------------------*/

#endif /* TREES_AND_HEAPS_H */
