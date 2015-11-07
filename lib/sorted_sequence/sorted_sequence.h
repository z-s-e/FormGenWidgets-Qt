/* Copyright 2014, 2015 Zeno Sebastian Endemann <zeno.endemann@googlemail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef SORTED_SEQUENCE_H
#define SORTED_SEQUENCE_H

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

#include <cassert>


#ifdef SORTEDSEQUENCE_ENABLE_QHASH_WRAPPER
// wrapper for QHash iterator
#include <QHash>

namespace sorted_sequence {
inline int key(const QHash<int, int>::iterator &it)
{
    return it.value();
}

inline int& value_ref(QHash<int, int>::iterator &it)
{
    return *it;
}
}
#endif


#ifdef SORTEDSEQUENCE_ENABLE_STD_HASH_WRAPPER
// wrapper for unordered_map iterator access
#include <unordered_map>

namespace sorted_sequence {
inline ptrdiff_t key(const std::unordered_map<std::ptrdiff_t, ptrdiff_t>::iterator &it)
{
    return it->first;
}

inline ptrdiff_t& value_ref(typename std::unordered_map<ptrdiff_t, ptrdiff_t>::iterator &it)
{
    return it->second;
}
}
#endif


namespace sorted_sequence {


enum InsertMode {
    InsertFirst, /// Insert at first possible position
    InsertLast   /// Insert at last possible position
};


struct default_sort_algorithm {
    template<class RAIterator, class Compare>
    static void sort(RAIterator begin, RAIterator end, Compare compare)
    {
        std::stable_sort(begin, end, compare);
    }
};


// default (std::less wrapper) + lambda wrapper Compare
// ----------------------------------------------------------------------------

template< typename T >
struct default_compare : public std::less<T> {};

template< typename T >
bool operator==(const default_compare<T>&, const default_compare<T>&)
{
    return true;
}

template< typename T >
bool operator!=(const default_compare<T>&, const default_compare<T>&)
{
    return false;
}


template< typename T >
class lambda_compare {
public:
    typedef std::function<bool(const T&, const T&)> Func;

    explicit lambda_compare(const Func& f)
        : m_f(std::make_shared<const Func>(f))
    {
        assert(*m_f);
    }

    explicit lambda_compare(Func&& f)
        : m_f(std::make_shared<const Func>(std::move(f)))
    {
        assert(*m_f);
    }

    // + default copy/move ctor/assignment

    bool operator()(const T& x, const T& y) const
    {
        return (*m_f)(x, y);
    }

    bool operator==(const lambda_compare<T>& other) const
    {
        return m_f == other.m_f;
    }

    bool operator!=(const lambda_compare<T>& other) const
    {
        return !operator==(other);
    }

private:
    std::shared_ptr<const Func> m_f;
};


// container adaptor class
// ----------------------------------------------------------------------------

template< class Container,
          class Compare = default_compare<typename Container::value_type>,
          class SortAlgorithm = default_sort_algorithm >
class adaptor {
public:
    typedef typename Container::value_type          value_type;
    typedef typename Container::size_type           size_type;
    typedef typename Container::const_iterator      const_iterator;
    typedef typename Container::difference_type     difference_type;
    typedef difference_type                         index;
    typedef const_iterator                          ConstIterator;
    typedef Container                               container_type;
    typedef Compare                                 compare_type;

    adaptor() = default;
    explicit adaptor(const Container& container, const Compare& compare = {});
    explicit adaptor(Container&& container, const Compare& compare = {});
    explicit adaptor(const Compare& compare);

    // + default copy/move ctor/assignment


    Compare compareOperator() const;
    /// Change the sorting criteria to comparison and resort the container accordingly.
    void setCompareOperator(const Compare& comparison);
    /**
     * Like setCompareOperator, but also report for each key in oldToNew its new position
     * after resorting (as the value), so when returning oldToNew[oldIndex] == newIndex.
     * 
     * Map is supposed to work with std::unordered_map<index, index> and the Qt container
     * QHash<index, index>. Sadly their iterator interfaces are different, so a small wrapper
     * is used. To enable the QHash wrapper define SORTEDSEQUENCE_ENABLE_QHASH_WRAPPER, for
     * unordered_map define SORTEDSEQUENCE_ENABLE_STD_HASH_WRAPPER.
     */
    template< class Map >
    void setCompareOperatorGetReorderMap(const Compare& comparison,
                                         Map* oldToNew);


    index insertPosition(const value_type& value, InsertMode mode = InsertLast,
                         index positionHint = -1) const;
    index insert(const value_type& value, InsertMode mode = InsertLast,
                 index positionHint = -1);
    index insert(value_type&& value, InsertMode mode = InsertLast,
                 index positionHint = -1);

    /**
     * @brief Change the value at index i to newValue.
     * @param i Index of the value to change
     * @param newValue The new value
     * @param newPositionHintBeforeRemove Conceptually the new value gets inserted and then
     *                                    the old value gets removed, so this hint should be
     *                                    equal to insertPosition(newValue, mode) to prevent
     *                                    one binary search operation.
     * @return The new position after changing the value
     */
    index change(index i, const value_type& newValue, InsertMode mode = InsertLast,
                 index newPositionHintBeforeRemove = -1);
    index change(index i, value_type&& newValue, InsertMode mode = InsertLast,
                 index newPositionHintBeforeRemove = -1);

    const Container& container() const { return d.c; }
    Container takeContainer();

    const value_type& at(index i) const { return d.c.at(i); }
    const value_type& front() const { return d.c.front(); }
    const value_type& first() const { return front(); }
    const value_type& back() const { return d.c.back(); }
    const value_type& last() const { return back(); }

    const_iterator begin() const { return d.c.cbegin(); }
    const_iterator end() const { return d.c.cend(); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }
    const_iterator constBegin() const { return begin(); }
    const_iterator constEnd() const { return end(); }

    size_type size() const { return d.c.size(); }
    size_type count() const { return size(); }
    size_type length() const { return size(); }
    bool empty() const { return d.c.empty(); }
    bool isEmpty() const { return empty(); }

    bool contains(const value_type& value) const;
    size_type count(const value_type& value) const;
    index indexOf(const value_type& value, index from = 0) const;
    index lastIndexOf(const value_type& value, index from = -1) const;
    index findFirst(const value_type& value) const;
    index findLast(const value_type& value) const;
    std::pair<index, index> range(const value_type& value) const;

    void clear() { d.c.clear(); }
    void removeAt(index i) { d.c.erase(d.c.begin() + i); }
    void removeFirst() { removeAt(0); }
    void removeLast() { removeAt(size() - 1); }
    void removeRange(index begin, index end);
    bool removeOne(const value_type& value);
    index removeAll(const value_type& value);

    const_iterator erase(const_iterator position);
    const_iterator erase(const_iterator first, const_iterator last);
    void pop_back() { d.c.pop_back(); }

    value_type takeAt(index i);
    value_type takeFirst() { return takeAt(0); }
    value_type takeLast() { return takeAt(size() - 1); }

    void reserve(size_type alloc) { d.c.reserve(alloc); }

    bool operator==(const adaptor<Container, Compare, SortAlgorithm>& other) const;
    bool operator!=(const adaptor<Container, Compare, SortAlgorithm>& other) const;
    const value_type& operator[](index i) const { return d.c[i]; }

    template< class OtherContainer, class OtherSortAlgorithm >
    adaptor<Container, Compare, SortAlgorithm>& operator<<(const adaptor<OtherContainer, Compare, OtherSortAlgorithm>& other);
    adaptor<Container, Compare, SortAlgorithm>& operator<<(const Container& container);
    adaptor<Container, Compare, SortAlgorithm>& operator<<(const value_type& value);

    index offset(const_iterator it) const;

    template< class OtherContainer1, class OtherSortAlgorithm1, class OtherContainer2, class OtherSortAlgorithm2 >
    static adaptor<Container, Compare, SortAlgorithm> merge(const adaptor<OtherContainer1, Compare, OtherSortAlgorithm1>& l1,
                                                            const adaptor<OtherContainer2, Compare, OtherSortAlgorithm2>& l2);

private:
    void sort()
    {
        SortAlgorithm::template sort(d.c.begin(), d.c.end(), d);
    }

    template< class T >
    index changeFwdAssign(index i, T&& newValue, InsertMode mode,
                          index newPositionHintBeforeRemove)
    {
        assert(i >= 0 && i < index(size()));

        const index pos = insertPosition(newValue, mode, newPositionHintBeforeRemove);

        if( pos == i || pos == i + 1) {
            d.c[i] = std::forward<T>(newValue);
            return i;
        } else if( pos < i) {
            auto first = d.c.begin();
            std::move_backward(first + pos, first + i, first + i + 1);
            d.c[pos] = std::forward<T>(newValue);
            return pos;
        } else {
            auto first = d.c.begin();
            std::move(first + i + 1, first + pos, first + i);
            d.c[pos - 1] = std::forward<T>(newValue);
            return pos - 1;
        }
    }

    struct Data : public Compare {
        // use inheritance of Compare (instead of a data member) to allow for the
        // empty base class optimization
        Data(const Compare& comp = Compare(), const Container& cntr = Container())
            : Compare(comp), c(cntr)
        {}
        Data(const Compare& comp, Container&& cntr)
            : Compare(comp), c(std::move(cntr))
        {}

        Container c;
    };

    Data d;
};


// implementation
// ----------------------------------------------------------------------------

template<class Container, class Compare, class SortAlgorithm>
adaptor<Container, Compare, SortAlgorithm>::adaptor(const Container& container,
                                                    const Compare& compare)
    : d(compare, container)
{
    sort();
}

template<class Container, class Compare, class SortAlgorithm>
adaptor<Container, Compare, SortAlgorithm>::adaptor(Container&& container,
                                                    const Compare& compare)
    : d(compare, std::move(container))
{
    sort();
}

template<class Container, class Compare, class SortAlgorithm>
adaptor<Container, Compare, SortAlgorithm>::adaptor(const Compare& compare)
    : d(compare)
{
}

template<class Container, class Compare, class SortAlgorithm>
Compare adaptor<Container, Compare, SortAlgorithm>::compareOperator() const
{
    return d;
}

template<class Container, class Compare, class SortAlgorithm>
void adaptor<Container, Compare, SortAlgorithm>::setCompareOperator(const Compare& comparison)
{
    if( d == comparison )
        return;

    static_cast<Compare&>(d) = comparison;
    sort();
}

template<class Container, class Compare, class SortAlgorithm>
template< class Map >
void adaptor<Container, Compare, SortAlgorithm>::setCompareOperatorGetReorderMap(const Compare& comparison,
                                                                                 Map *oldToNew)
{
    assert(oldToNew);

    if( d == comparison ) {
        for( auto it = oldToNew->begin(), end = oldToNew->end(); it != end; ++it )
            value_ref(it) = key(it);
        return;
    }

    static_cast<Compare&>(d) = comparison;

    const index n = index(size());

    std::vector<index> reorderList(n);
    std::iota(reorderList.begin(), reorderList.end(), 0);

    { // sort reorderList as if it were the actual container
        const auto f = [this] (index lhs, index rhs) { return d(at(lhs), at(rhs)); };
        SortAlgorithm::template sort(reorderList.begin(), reorderList.end(), f);
    }

    { // fill out oldToNew with reorderList
        const auto mapEnd = oldToNew->end();
        index mapLeft = oldToNew->size();
        for( index i = 0; i < n && mapLeft > 0; ++i ) {
            auto it = oldToNew->find(reorderList.at(i));
            if( it != mapEnd ) {
                value_ref(it) = i;
                --mapLeft;
            }
        }
    }

    // sort actual container with reorderList (= permutation)
    for( index i = 0; i < n; ++i ) {
        index j = reorderList.at(i);

        if( i == j || j < 0 ) // trivial permutation cycle or already processed
            continue;

        // shift all values according to the permutation cycle containing position i

        value_type tmp = std::move(d.c[i]);

        index k = i;
        do {
            d.c[k] = std::move(d.c[j]);
            k = j;
            j = reorderList.at(j);
            reorderList[k] = -1; // mark as processed
        } while( j != i );

        d.c[k] = std::move(tmp);
    }
}


template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::insertPosition(const value_type& value,
                                                           InsertMode mode,
                                                           index positionHint) const
{
    const index n = index(size());
    if( positionHint >= 0 && positionHint <= n ) {
        if( empty() )
            return 0;

        if( positionHint == 0 ) {
            if( mode == InsertFirst && ! d(first(), value) )
                return positionHint;
            else if( mode == InsertLast && d(value, first()) )
                return positionHint;
        }  else if( positionHint == n ) {
            if( mode == InsertFirst && d(last(), value) )
                return positionHint;
            else if( mode == InsertLast && ! d(value, last()))
                return positionHint;
        } else {
            if( mode == InsertFirst && d(at(positionHint - 1), value) ) {
                if( ! d(at(positionHint), value) )
                    return positionHint;
                else
                    return offset(std::lower_bound(begin() + positionHint, end(), value, d));
            } else if( mode == InsertLast && d(value, at(positionHint))) {
                if( ! d(value, at(positionHint - 1)) )
                    return positionHint;
                else
                    return offset(std::upper_bound(begin(), begin() + positionHint, value, d));
            }
        }
    }

    if( mode == InsertFirst )
        return offset(std::lower_bound(begin(), end(), value, d));
    else
        return offset(std::upper_bound(begin(), end(), value, d));
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::insert(const value_type& value,
                                                   InsertMode mode,
                                                   index positionHint)
{
    const index pos = insertPosition(value, mode, positionHint);
    d.c.insert(d.c.begin() + pos, value);
    return pos;
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::insert(value_type&& value,
                                                   InsertMode mode,
                                                   index positionHint)
{
    const index pos = insertPosition(value, mode, positionHint);
    d.c.insert(d.c.begin() + pos, std::move(value));
    return pos;
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::change(index i,
                                                   const value_type& newValue,
                                                   InsertMode mode,
                                                   index newPositionHintBeforeRemove)
{
    return changeFwdAssign(i, newValue, mode, newPositionHintBeforeRemove);
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::change(index i,
                                                   value_type&& newValue,
                                                   InsertMode mode,
                                                   index newPositionHintBeforeRemove)
{
    return changeFwdAssign(i, std::move(newValue), mode, newPositionHintBeforeRemove);
}

template<class Container, class Compare, class SortAlgorithm>
Container adaptor<Container, Compare, SortAlgorithm>::takeContainer()
{
    Container tmp(std::move(d.c));
    d.c = Container();
    return tmp;
}

template<class Container, class Compare, class SortAlgorithm>
bool adaptor<Container, Compare, SortAlgorithm>::contains(const value_type& value) const
{
    auto range = std::equal_range(begin(), end(), value, d);
    return std::find(range.first, range.second, value) != range.second;
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::size_type
adaptor<Container, Compare, SortAlgorithm>::count(const value_type& value) const
{
    auto range = std::equal_range(begin(), end(), value, d);
    return std::count(range.first, range.second, value);
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::indexOf(const value_type& value,
                                                    index from) const
{
    auto range = std::equal_range(begin() + std::max(index(0), std::min(from, index(size()))),
                                  end(), value, d);
    auto it = std::find(range.first, range.second, value);
    return it == range.second ? -1 : offset(range.first);
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::lastIndexOf(const value_type& value,
                                                        index from) const
{
    auto range = std::equal_range(begin() + std::max(index(0), std::min(from, index(size()))),
                                  end(), value, d);

    while( range.second != range.first ) {
        --range.second;
        if( *(range.second) == value )
            return offset(range.second);
    }

    return -1;
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::findFirst(const value_type& value) const
{
    return indexOf(value);
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::findLast(const value_type& value) const
{
    return lastIndexOf(value);
}

template<class Container, class Compare, class SortAlgorithm>
std::pair<typename adaptor<Container, Compare, SortAlgorithm>::index, typename adaptor<Container, Compare, SortAlgorithm>::index>
adaptor<Container, Compare, SortAlgorithm>::range(const value_type& value) const
{
    auto range = std::equal_range(begin(), end(), value, d);
    return std::pair<index, index>(offset(range.first), offset(range.second));
}

template<class Container, class Compare, class SortAlgorithm>
void adaptor<Container, Compare, SortAlgorithm>::removeRange(index begin,
                                                             index end)
{
    d.c.erase( d.c.begin() + begin, d.c.begin() + end );
}

template<class Container, class Compare, class SortAlgorithm>
bool adaptor<Container, Compare, SortAlgorithm>::removeOne(const value_type& value)
{
    index i = findLast(value);
    if( i < 0 )
        return false;
    removeAt(i);
    return true;
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::removeAll(const value_type& value)
{
    auto r = std::equal_range(d.c.begin(), d.c.end(), value, d);
    auto it = std::remove(r.first, r.second, value);
    auto newEnd = std::move(r.second, d.c.end(), it);
    const index count = r.second - it;
    erase(newEnd, d.c.end());
    return count;
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::const_iterator adaptor<Container, Compare, SortAlgorithm>::erase(const_iterator position)
{
    return d.c.erase(d.c.begin() + offset(position));
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::const_iterator adaptor<Container, Compare, SortAlgorithm>::erase(const_iterator first,
                                                                                                                      const_iterator last)
{
    return d.c.erase(d.c.begin() + offset(first), d.c.begin() + offset(last));
}

template<class Container, class Compare, class SortAlgorithm>
typename adaptor<Container, Compare, SortAlgorithm>::value_type adaptor<Container, Compare, SortAlgorithm>::takeAt(index i)
{
    value_type tmp = std::move(d.c[i]);
    removeAt(i);
    return tmp;
}

template<class Container, class Compare, class SortAlgorithm>
bool adaptor<Container, Compare, SortAlgorithm>::operator==(const adaptor<Container, Compare, SortAlgorithm>& other) const
{
    return (d == other.d && d.c == other.d.c);
}

template<class Container, class Compare, class SortAlgorithm>
bool adaptor<Container, Compare, SortAlgorithm>::operator!=(const adaptor<Container, Compare, SortAlgorithm>& other) const
{
    return ! operator==(other);
}

template< class Container, class Compare, class SortAlgorithm >
template< class OtherContainer, class OtherSortAlgorithm >
adaptor<Container, Compare, SortAlgorithm>& adaptor<Container, Compare, SortAlgorithm>::operator<<(const adaptor<OtherContainer, Compare, OtherSortAlgorithm>& other)
{
    if( other.empty() )
        return *this;

    if( other.d == d ) {
        const index i = index(size());
        d.c.insert(d.c.end(), other.d.c.begin(), other.d.c.end());
        std::inplace_merge(d.c.begin(), d.c.begin() + i, d.c.end(), d);
    } else {
        *this << other.d.c;
    }
    return *this;
}

template< class Container, class Compare, class SortAlgorithm >
adaptor<Container, Compare, SortAlgorithm>& adaptor<Container, Compare, SortAlgorithm>::operator<<(const Container& container)
{
    const index i = index(size());
    d.c.reserve(i + container.size());
    for( auto it = container.begin(), end = container.end(); it != end; ++it )
        d.c.push_back(*it); // Qt container do not support range insert

    SortAlgorithm::template sort(d.c.begin() + i, d.c.end(), d);

    std::inplace_merge(d.c.begin(), d.c.begin() + i, d.c.end(), d);

    return *this;
}

template< class Container, class Compare, class SortAlgorithm >
adaptor<Container, Compare, SortAlgorithm>& adaptor<Container, Compare, SortAlgorithm>::operator<<(const value_type& value)
{
    insert(value);
    return *this;
}

template< class Container, class Compare, class SortAlgorithm >
typename adaptor<Container, Compare, SortAlgorithm>::index
adaptor<Container, Compare, SortAlgorithm>::offset(const_iterator it) const
{
    return std::distance(begin(), it);
}

template< class Container, class Compare, class SortAlgorithm >
template< class OtherContainer1, class OtherSortAlgorithm1, class OtherContainer2, class OtherSortAlgorithm2 >
adaptor<Container, Compare, SortAlgorithm> adaptor<Container, Compare, SortAlgorithm>::merge(const adaptor<OtherContainer1, Compare, OtherSortAlgorithm1>& l1,
                                                                                             const adaptor<OtherContainer2, Compare, OtherSortAlgorithm2>& l2)
{
    if( l1.d != l2.d )
        return {};

    adaptor<Container, Compare, SortAlgorithm> result(l1.d);

    result.reserve(l1.size() + l2.size());
    std::merge(l1.cbegin(), l1.cend(), l2.cbegin(), l2.cend(),
               std::back_insert_iterator<Container>(result.d.c), result.d);

    return result;
}


} // namespace sorted_sequence

#endif // SORTED_SEQUENCE_H

