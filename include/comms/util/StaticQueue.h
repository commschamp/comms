//
// Copyright 2012 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file comms/util/StaticQueue.h
/// @brief This file contains the definition and implementation of the static queue,
///     which also can be used as circular buffer.

#pragma once

#include "comms/Assert.h"
#include "comms/CompileControl.h"
#include "comms/util/AlignedStorage.h"
#include "comms/util/SizeToType.h"
#include "comms/util/type_traits.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

COMMS_MSVC_WARNING_PUSH
COMMS_MSVC_WARNING_DISABLE(4324) // Disable warning about alignment padding

namespace comms
{

namespace util
{

namespace details
{

template <typename T>
class StaticQueueBase
{
public:
    template <typename TDerived, typename TQueueType>
    class IteratorBase;

    class ConstIterator;
    class Iterator;

protected:
    using ValueType = T;
    using StorageType = comms::util::AlignedStorage<sizeof(ValueType), std::alignment_of<ValueType>::value>;
    using StorageTypePtr = StorageType*;
    using ConstStorageTypePtr = const StorageType*;
    using SizeType = std::size_t;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;
    using Pointer = ValueType*;
    using ConstPointer = const ValueType*;
    using LinearisedIterator = Pointer;
    using ConstLinearisedIterator = ConstPointer;
    using ReverseLinearisedIterator = std::reverse_iterator<LinearisedIterator>;
    using ConstReverseLinearisedIterator = std::reverse_iterator<ConstLinearisedIterator>;
    using LinearisedIteratorRange = std::pair<LinearisedIterator, LinearisedIterator>;
    using ConstLinearisedIteratorRange = std::pair<ConstLinearisedIterator, ConstLinearisedIterator>;

    StaticQueueBase(StorageTypePtr data, std::size_t capacity)
        : m_data(data),
          m_capacity(capacity),
          m_startIdx(0),
          m_count(0)
    {
    }

    StaticQueueBase(const StaticQueueBase&) = delete;
    StaticQueueBase(StaticQueueBase&&) = delete;

    ~StaticQueueBase() noexcept
    {
        clear();
    }

    StaticQueueBase& operator=(const StaticQueueBase& other)
    {
        if (this == &other) {
            return *this;
        }

        clear();
        assignElements(other);
        return *this;
    }

    StaticQueueBase& operator=(StaticQueueBase&& other)
    {
        if (this == &other) {
            return *this;
        }

        clear();
        assignElements(std::move(other));
        return *this;
    }

    constexpr std::size_t capacity() const
    {
        return m_capacity;
    }

    std::size_t size() const
    {
        return m_count;
    }

    void clear()
    {
        while (!empty()) {
            popFront();
        }
    }

    bool empty() const
    {
        return (size() == 0);
    }

    bool full() const
    {
        return (size() == capacity());
    }

    Reference front()
    {
        COMMS_ASSERT(!empty());
        return (*this)[0];
    }

    ConstReference front() const
    {
        COMMS_ASSERT(!empty());
        return (*this)[0];
    }

    Reference back()
    {
        auto constThis = static_cast<const StaticQueueBase*>(this);
        return const_cast<Reference>(constThis->back());
    }

    ConstReference back() const
    {
        COMMS_ASSERT(!empty());
        if (empty()) {
            return (*this)[0]; // Back() on empty queue
        }

        return (*this)[m_count - 1];
    }


    void popFront()
    {
        COMMS_ASSERT(!empty()); // Queue musn't be empty
        if (empty())
        {
            // Do nothing
            return;
        }

        Reference element = front();
        element.~T();

        --m_count;
        ++m_startIdx;
        if ((capacity() <= m_startIdx) ||
            (empty())) {
            m_startIdx = 0;
        }
    }

    void popFront(std::size_t count)
    {
        COMMS_ASSERT(count <= size());
        while ((!empty()) && (count > 0)) {
            popFront();
            --count;
        }
    }

    void popBack()
    {
        COMMS_ASSERT(!empty());
        if (empty())
        {
            // Do nothing
            return;
        }

        Reference element = back();
        element.~T();

        --m_count;
    }

    void popBack(std::size_t count)
    {
        COMMS_ASSERT(count <= size());
        while ((!empty()) && (count > 0)) {
            popBack();
            --count;
        }
    }

    Reference operator[](std::size_t index)
    {
        auto constThis = static_cast<const StaticQueueBase*>(this);
        return const_cast<Reference>((*constThis)[index]);
    }

    ConstReference operator[](std::size_t index) const
    {
        COMMS_ASSERT(index < size());
        return elementAtIndex(index);
    }

    Reference at(std::size_t index)
    {
        auto constThis = static_cast<const StaticQueueBase*>(this);
        return const_cast<Reference>(constThis->at(index));
    }

    ConstReference at(std::size_t index) const
    {
        if (index >= size()) {
            throw std::out_of_range(std::string("Index is out of range"));
        }
        return (*this)[index];
    }

    int indexOf(ConstReference element) const
    {
        ConstPointer elementPtr = &element;
        auto cellPtr = reinterpret_cast<ConstStorageTypePtr>(elementPtr);
        if ((cellPtr < &m_data[0]) ||
            (&m_data[capacity()] <= cellPtr)) {
            // invalid, element is not within array boundaries
            return -1;
        }

        auto rawIdx = cellPtr - &m_data[0];
        std::size_t actualIdx = capacity(); // invalid value
        if (rawIdx < m_startIdx) {
            actualIdx = (capacity() - m_startIdx) + rawIdx;
        }
        else {
            actualIdx = rawIdx - m_startIdx;
        }

        if (size() <= actualIdx)
        {
            // Element is out of range
            return -1;
        }

        return actualIdx;
    }

    LinearisedIterator invalidIter()
    {
        return reinterpret_cast<LinearisedIterator>(&m_data[capacity()]);
    }

    ConstLinearisedIterator invalidIter() const
    {
        return reinterpret_cast<LinearisedIterator>(&m_data[capacity()]);
    }

    ReverseLinearisedIterator invalidReverseIter()
    {
        return ReverseLinearisedIterator(
            reinterpret_cast<LinearisedIterator>(&m_data[0]));
    }

    ConstReverseLinearisedIterator invalidReverseIter() const
    {
        return ReverseLinearisedIterator(
            reinterpret_cast<LinearisedIterator>(&m_data[0]));
    }

    LinearisedIterator lbegin()
    {
        if (!linearised()) {
            return invalidIter();
        }

        return reinterpret_cast<LinearisedIterator>(&m_data[0] + m_startIdx);
    }

    ConstLinearisedIterator lbegin() const
    {
        return clbegin();
    }

    ConstLinearisedIterator clbegin() const
    {
        if (!linearised()) {
            return invalidIter();
        }

        return reinterpret_cast<LinearisedIterator>(&m_data[0] + m_startIdx);
    }

    ReverseLinearisedIterator rlbegin()
    {
        if (!linearised()) {
            return invalidReverseIter();
        }

        return ReverseLinearisedIterator(lend());
    }

    ConstReverseLinearisedIterator rlbegin() const
    {
        return crlbegin();
    }

    ConstReverseLinearisedIterator crlbegin() const
    {
        if (!linearised()) {
            return invalidReverseIter();
        }
        return ConstReverseLinearisedIterator(lend());
    }

    LinearisedIterator lend()
    {
        if (!linearised()) {
            return invalidIter();
        }

        return lbegin() + size();
    }

    ConstLinearisedIterator lend() const
    {
        return clend();
    }

    ConstLinearisedIterator clend() const
    {
        if (!linearised()) {
            return invalidIter();
        }

        return clbegin() + size();
    }

    ReverseLinearisedIterator rlend()
    {
        if (!linearised()) {
            return invalidReverseIter();
        }

        return rlbegin() + size();
    }

    ConstReverseLinearisedIterator rlend() const
    {
        return crlend();
    }

    ConstReverseLinearisedIterator crlend() const
    {
        if (!linearised()) {
            return invalidReverseIter();
        }

        return rlbegin() + size();
    }

    void linearise()
    {
        if (linearised()) {
            // Nothing to do
            return;
        }

        auto rangeOne = arrayOne();
        auto rangeOneSize = std::distance(rangeOne.first, rangeOne.second);
        COMMS_ASSERT(0 < rangeOneSize);
        auto rangeTwo = arrayTwo();
        auto rangeTwoSize = std::distance(rangeTwo.first, rangeTwo.second);
        COMMS_ASSERT(0 < rangeTwoSize);
        COMMS_ASSERT((rangeOneSize + rangeTwoSize) == size());
        auto remSpaceSize = capacity() - size();

        if (rangeTwoSize <= remSpaceSize) {
            lineariseByMoveOneTwo(rangeOne, rangeTwo);
            return;
        }

        if (rangeOneSize <= remSpaceSize) {
            lineariseByMoveTwoOne(rangeOne, rangeTwo);
            return;
        }

        if (rangeOneSize < rangeTwoSize) {
            lineariseByPopOne();
        }

        lineariseByPopTwo();
    }

    bool linearised() const
    {
        return (empty() || ((m_startIdx + size()) <= capacity()));
    }

    LinearisedIteratorRange arrayOne()
    {
        auto constThis = static_cast<const StaticQueueBase*>(this);
        auto constRange = constThis->arrayOne();
        return
            LinearisedIteratorRange(
                const_cast<LinearisedIterator>(constRange.first),
                const_cast<LinearisedIterator>(constRange.second));
    }

    ConstLinearisedIteratorRange arrayOne() const
    {
        auto begCell = &m_data[m_startIdx];
        auto endCell = std::min(&m_data[m_startIdx + size()], &m_data[capacity()]);
        return
            ConstLinearisedIteratorRange(
                reinterpret_cast<ConstLinearisedIterator>(begCell),
                reinterpret_cast<ConstLinearisedIterator>(endCell));
    }

    LinearisedIteratorRange arrayTwo()
    {
        auto constThis = static_cast<const StaticQueueBase*>(this);
        auto constRange = constThis->arrayTwo();
        return
            LinearisedIteratorRange(
                const_cast<LinearisedIterator>(constRange.first),
                const_cast<LinearisedIterator>(constRange.second));
    }

    ConstLinearisedIteratorRange arrayTwo() const
    {
        if (linearised()) {
            auto iter = arrayOne().second;
            return ConstLinearisedIteratorRange(iter, iter);
        }

        auto begCell = &m_data[0];
        auto endCell = &m_data[(m_startIdx + size()) - capacity()];

        return
            ConstLinearisedIteratorRange(
                reinterpret_cast<ConstLinearisedIterator>(begCell),
                reinterpret_cast<ConstLinearisedIterator>(endCell));
    }

    void resize(std::size_t newSize)
    {
        COMMS_ASSERT(newSize <= capacity());
        if (capacity() < newSize) {
            return;
        }

        if (size() <= newSize) {
            while (size() < newSize) {
                pushBackNotFull(ValueType());
            }

            return;
        }

        // New requested size is less than existing now
        popBack(size() - newSize);
    }

    LinearisedIterator erase(LinearisedIterator pos)
    {
        COMMS_ASSERT(pos != invalidIter());
        COMMS_ASSERT(!empty());
        auto rangeOne = arrayOne();
        auto rangeTwo = arrayTwo();

        auto isInRangeFunc =
            [](LinearisedIterator pos, const LinearisedIteratorRange range) -> bool
            {
                return ((range.first <= pos) && (pos < range.second));
            };

        COMMS_ASSERT(isInRangeFunc(pos, rangeOne) ||
               isInRangeFunc(pos, rangeTwo));

        if (isInRangeFunc(pos, rangeOne)) {
            std::move_backward(rangeOne.first, pos, pos + 1);

            popFront();
            rangeOne = arrayOne();
            if (isInRangeFunc(pos, rangeOne)) {
                return pos + 1;
            }

            return rangeOne.first;
        }

        if (isInRangeFunc(pos, rangeTwo)) {
            std::move(pos + 1, rangeTwo.second, pos);
            popBack();
            if (!linearised()) {
                return pos;
            }
            return arrayOne().second;
        }

        static constexpr bool Invalid_iterator_is_used = false;
        static_cast<void>(Invalid_iterator_is_used);
        COMMS_ASSERT(Invalid_iterator_is_used);
        return invalidIter();
    }

    Iterator erase(Iterator pos)
    {
        COMMS_ASSERT(pos != end());
        COMMS_ASSERT(!empty());
        Pointer elem = &(*pos);
        auto rangeOne = arrayOne();
        auto rangeTwo = arrayTwo();

        auto isInRangeFunc =
            [](Pointer elemPtr, const LinearisedIteratorRange range) -> bool
            {
                return ((&(*range.first) <= elemPtr) && (elemPtr < &(*range.second)));
            };

        COMMS_ASSERT(isInRangeFunc(elem, rangeOne) ||
                isInRangeFunc(elem, rangeTwo));

        if (isInRangeFunc(elem, rangeOne)) {
            std::move_backward(rangeOne.first, elem, elem + 1);

            popFront();
            rangeOne = arrayOne();
            if (isInRangeFunc(elem, rangeOne)) {
                return pos + 1;
            }

            return begin();
        }

        if (isInRangeFunc(elem, rangeTwo)) {
            std::move(elem + 1, rangeTwo.second, elem);
            popBack();
            if (!linearised()) {
                return pos;
            }
            return end();
        }

        static constexpr bool Invalid_iterator_is_used = false;
        static_cast<void>(Invalid_iterator_is_used);
        COMMS_ASSERT(Invalid_iterator_is_used);
        return end();
    }

    Iterator begin()
    {
        return Iterator(*this, reinterpret_cast<LinearisedIterator>(&m_data[0]));
    }

    ConstIterator begin() const
    {
        return cbegin();
    }

    ConstIterator cbegin() const
    {
        return ConstIterator(*this, reinterpret_cast<ConstLinearisedIterator>(&m_data[0]));
    }

    Iterator end()
    {
        return Iterator(*this, reinterpret_cast<LinearisedIterator>(&m_data[size()]));
    }

    ConstIterator end() const
    {
        return cend();
    }

    ConstIterator cend() const
    {
        return ConstIterator(*this, reinterpret_cast<ConstLinearisedIterator>(&m_data[size()]));
    }

    template <typename TOther>
    void assignElements(TOther&& other)
    {
        static_assert(std::is_base_of<StaticQueueBase, typename std::decay<TOther>::type>::value,
            "Assignment works only on the same types");


        using DecayedQueueType = typename std::decay<decltype(other)>::type;
        using NonRefQueueType = typename std::remove_reference<decltype(other)>::type;

        using QueueValueType = 
            typename comms::util::Conditional<
                std::is_const<NonRefQueueType>::value
            >::template Type<
                const typename DecayedQueueType::ValueType,
                typename DecayedQueueType::ValueType
            >;

        using ElemRefType = 
            typename comms::util::Conditional<
                std::is_rvalue_reference<decltype(other)>::value
            >::template Type<
                typename std::add_rvalue_reference<QueueValueType>::type,
                typename std::add_lvalue_reference<QueueValueType>::type
            >;

        COMMS_ASSERT(other.size() <= capacity());
        COMMS_ASSERT(empty());

        auto rangeOne = other.arrayOne();
        for (auto iter = rangeOne.first; iter != rangeOne.second; ++iter) {
            pushBackNotFull(std::forward<ElemRefType>(*iter));
        }

        auto rangeTwo = other.arrayTwo();
        for (auto iter = rangeTwo.first; iter != rangeTwo.second; ++iter) {
            pushBackNotFull(std::forward<ElemRefType>(*iter));
        }
    }

    template <typename U>
    void pushBack(U&& value)
    {
        COMMS_ASSERT(!full());
        if (full()) {
            return;
        }
        pushBackNotFull(std::forward<U>(value));
    }

    template <typename... TArgs>
    void emplaceBack(TArgs&&... args)
    {
        COMMS_ASSERT(!full());
        if (full()) {
            return;
        }
        emplaceBackNotFull(std::forward<TArgs>(args)...);
    }

    template <typename U>
    void pushFront(U&& value)
    {
        COMMS_ASSERT(!full());
        if (full()) {
            return;
        }

        pushFrontNotFull(std::forward<U>(value));
    }

    template <typename U>
    LinearisedIterator insert(LinearisedIterator pos, U&& value)
    {
        COMMS_ASSERT(!full());
        if (full()) {
            return invalidIter();
        }

        return insertNotFull(pos, std::forward<U>(value));
    }

    bool operator==(const StaticQueueBase& other) const
    {
        if (size() != other.size()) {
            return false;
        }

        auto rangeOne = arrayOne();
        auto rangeOneSize = std::distance(rangeOne.first, rangeOne.second);
        auto rangeTwo = arrayTwo();
        auto otherRangeOne = other.arrayOne();
        auto otherRangeOneSize = std::distance(otherRangeOne.first, otherRangeOne.second);
        auto otherRangeTwo = other.arrayTwo();

        auto firstCompSize = std::min(rangeOneSize, otherRangeOneSize);
        auto firstCompEnd = rangeOne.first + firstCompSize;

        auto currIter = rangeOne.first;
        auto otherCurrIter = otherRangeOne.first;
        if (!std::equal(currIter, firstCompEnd, otherCurrIter)) {
            return false;
        }

        currIter = firstCompEnd;
        otherCurrIter += firstCompSize;

        if (currIter != rangeOne.first) {
            otherCurrIter = otherRangeTwo.first;
            if (!std::equal(currIter, rangeOne.second, otherCurrIter)) {
                return false;
            }
            otherCurrIter += rangeOne.second - currIter;
            currIter = rangeTwo.first;
        }
        else {
            currIter = rangeTwo.first;
            if (!std::equal(otherCurrIter, otherRangeOne.second, currIter)) {
                return false;
            }

            currIter += otherRangeOne.second - otherCurrIter;
            otherCurrIter = otherRangeOne.first;
        }

        COMMS_ASSERT(std::distance(currIter, rangeTwo.second) == std::distance(otherCurrIter, otherRangeTwo.second));
        return std::equal(currIter, rangeTwo.second, otherCurrIter);
    }

    bool operator!=(const StaticQueueBase& other) const
    {
        return !(*this == other);
    }

private:

    template <typename U>
    void createValueAtIndex(U&& value, std::size_t index)
    {
        COMMS_ASSERT(index < capacity());
        Reference elementRef = elementAtIndex(index);
        auto elementPtr = new(&elementRef) ValueType(std::forward<U>(value));
        static_cast<void>(elementPtr);
    }

    template <typename U>
    void pushBackNotFull(U&& value)
    {
        COMMS_ASSERT(!full());
        createValueAtIndex(std::forward<U>(value), size());
        ++m_count;
    }

    template <typename... TArgs>
    void emplaceBackNotFull(TArgs&&... args)
    {
        COMMS_ASSERT(!full());
        Reference elementRef = elementAtIndex(size());
        auto elementPtr = new(&elementRef) ValueType(std::forward<TArgs>(args)...);
        static_cast<void>(elementPtr);
        ++m_count;
    }

    template <typename U>
    void pushFrontNotFull(U&& value)
    {
        COMMS_ASSERT(!full());
        createValueAtIndex(std::forward<U>(value), capacity() - 1);
        if (m_startIdx == 0) {
            m_startIdx = capacity() - 1;
        }
        else {
            --m_startIdx;
        }

        ++m_count;
    }

    template <typename U>
    LinearisedIterator insertNotFull(LinearisedIterator pos, U&& value)
    {
        COMMS_ASSERT(!full());
        COMMS_ASSERT(pos != invalidIter());
        auto rangeOne = arrayOne();
        auto rangeTwo = arrayTwo();

        if (pos == rangeOne.first) {
            pushFrontNotFull(std::forward<U>(value));
            return arrayOne().first;
        }

        if (pos == rangeTwo.second) {
            pushBackNotFull(std::forward<U>(value));
            return arrayTwo().second - 1;
        }

        auto isInRangeFunc = [](LinearisedIterator pos, const LinearisedIteratorRange range) -> bool
            {
                return ((range.first <= pos) && (pos < range.second));
            };

        COMMS_ASSERT(isInRangeFunc(pos, rangeOne) ||
                isInRangeFunc(pos, rangeTwo));

        if (isInRangeFunc(pos, rangeOne)) {
            pushFrontNotFull(std::move(front())); // move first element

            std::move(rangeOne.first + 1, pos, rangeOne.first);
            *pos = std::forward<U>(value);
            return pos;
        }

        if (isInRangeFunc(pos, rangeTwo)) {
            pushBackNotFull(std::move(back())); // move last element
            std::move_backward(pos, rangeTwo.second - 1, rangeTwo.second);
            *pos = std::forward<U>(value);
            return pos;
        }

        return invalidIter();
    }

    Reference elementAtIndex(std::size_t index) {
        auto constThis = static_cast<const StaticQueueBase*>(this);
        return const_cast<Reference>(constThis->elementAtIndex(index));
    }

    ConstReference elementAtIndex(std::size_t index) const
    {
        std::size_t rawIdx = m_startIdx + index;
        while (capacity() <= rawIdx) {
            rawIdx = rawIdx - capacity();
        }

        auto cellAddr = &m_data[rawIdx];
        return *(reinterpret_cast<ConstPointer>(cellAddr));
    }

    template <typename TIter>
    void lineariseByMove(
        const std::pair<TIter, TIter> firstRange,
        const std::pair<TIter, TIter> secondRange)
    {
        auto movConstructFirstSize =
            std::min(
                std::size_t(std::distance(firstRange.first, firstRange.second)),
                capacity() - size());
        auto movConstructFirstEnd = firstRange.first + movConstructFirstSize;
        COMMS_ASSERT(movConstructFirstEnd <= firstRange.second);
        COMMS_ASSERT(movConstructFirstSize <= (firstRange.second - firstRange.first));

        auto newPlacePtr = secondRange.second;
        for (auto iter = firstRange.first; iter != movConstructFirstEnd; ++iter) {
            auto ptr = new (&(*newPlacePtr)) ValueType(std::move(*iter));
            static_cast<void>(ptr);
            ++newPlacePtr;
        }

        std::move(movConstructFirstEnd, firstRange.second, newPlacePtr);
        newPlacePtr += (firstRange.second - movConstructFirstEnd);

        auto movConstructTwoSize = 0;
        if (newPlacePtr < firstRange.first) {
            movConstructTwoSize =
                std::min(std::distance(newPlacePtr, firstRange.first),
                         std::distance(secondRange.first, secondRange.second));
        }

        auto movConstructTwoEnd = secondRange.first + movConstructTwoSize;
        for (auto iter = secondRange.first; iter != movConstructTwoEnd; ++iter) {
            auto ptr = new (&(*newPlacePtr)) ValueType(std::move(*iter));
            static_cast<void>(ptr);
            ++newPlacePtr;
        }

        std::move(movConstructTwoEnd, secondRange.second, newPlacePtr);
        newPlacePtr += (secondRange.second - movConstructTwoEnd);

        for (auto iter = std::max(newPlacePtr, firstRange.first); iter != firstRange.second; ++iter) {
            iter->~T();
        }

        for (auto iter = secondRange.first; iter != secondRange.second; ++iter) {
            iter->~T();
        }
    }

    void lineariseByMoveOneTwo(
        const LinearisedIteratorRange& rangeOne,
        const LinearisedIteratorRange& rangeTwo)
    {
        lineariseByMove(rangeOne, rangeTwo);
        m_startIdx = std::distance(rangeTwo.first, rangeTwo.second);
    }

    void lineariseByMoveTwoOne(
        const LinearisedIteratorRange& rangeOne,
        const LinearisedIteratorRange& rangeTwo)
    {
        using RevIter = std::reverse_iterator<Pointer>;
        lineariseByMove(
            std::make_pair(RevIter(rangeTwo.second), RevIter(rangeTwo.first)),
            std::make_pair(RevIter(rangeOne.second), RevIter(rangeOne.first)));

        m_startIdx = (capacity() - std::distance(rangeOne.first, rangeOne.second)) - size();
    }

    void lineariseByPopOne()
    {
        if (linearised()) {
            return;
        }

        ValueType tmp(std::move(front()));
        popFront();
        lineariseByPopOne();
        if (m_startIdx == 0) {
            using RevIter = std::reverse_iterator<LinearisedIterator>;
            auto target =
                RevIter(reinterpret_cast<LinearisedIterator>(&m_data[capacity()]));
            moveRange(rlbegin(), rlend(), target);
            m_startIdx = capacity() - size();
        }
        pushFront(std::move(tmp));
        COMMS_ASSERT(linearised());
    }

    void lineariseByPopTwo()
    {
        if (linearised()) {
            return;
        }

        ValueType tmp(std::move(back()));
        popBack();
        lineariseByPopTwo();
        if (m_startIdx != 0) {
            auto target = reinterpret_cast<LinearisedIterator>(&m_data[0]);
            moveRange(lbegin(), lend(), target);
            m_startIdx = 0;
        }
        pushBack(std::move(tmp));
        COMMS_ASSERT(linearised());
    }

    template <typename TIter>
    void moveRange(TIter rangeBeg, TIter rangeEnd, TIter target)
    {
        COMMS_ASSERT(target < rangeBeg);
        auto moveConstructSize =
            std::min(
                std::distance(rangeBeg, rangeEnd),
                std::distance(target, rangeBeg));

        TIter moveConstructEnd = rangeBeg + moveConstructSize;
        for (auto iter = rangeBeg; iter != moveConstructEnd; ++iter) {
            auto ptr = new (&(*target)) ValueType(std::move(*iter));
            static_cast<void>(ptr);
            ++target;
        }

        COMMS_ASSERT(target < moveConstructEnd);
        std::move(moveConstructEnd, rangeEnd, target);
        target += std::distance(moveConstructEnd, rangeEnd);

        for (auto iter = std::max(target, rangeBeg); iter != rangeEnd; ++iter) {
            iter->~T();
        }
    }

    StorageTypePtr const m_data;
    const std::size_t m_capacity;
    std::size_t m_startIdx;
    std::size_t m_count;
};

template <typename T>
template <typename TDerived, typename TQueueType>
class StaticQueueBase<T>::IteratorBase
{
    friend class StaticQueueBase<T>;
public:

    IteratorBase(const IteratorBase&) = default;

    ~IteratorBase() noexcept = default;

protected:
    using Derived = TDerived;
    using QueueType = TQueueType;
    using ArrayIterator = decltype(std::declval<QueueType>().lbegin());
    using IteratorCategory = typename std::iterator_traits<ArrayIterator>::iterator_category;
    using iterator_category = IteratorCategory;
    using ValueType = typename std::iterator_traits<ArrayIterator>::value_type;
    using value_type = ValueType;
    using DifferenceType = typename std::iterator_traits<ArrayIterator>::difference_type;
    using difference_type = DifferenceType;
    using Pointer = typename std::iterator_traits<ArrayIterator>::pointer;
    using pointer = Pointer;
    using ConstPointer =
        typename std::add_pointer<
            typename std::add_const<
                typename std::remove_pointer<Pointer>::type
            >::type
        >::type;
    using Reference = typename std::iterator_traits<ArrayIterator>::reference;
    using reference = Reference;
    using ConstReference = typename std::add_const<Reference>::type;

    IteratorBase(QueueType& queue, ArrayIterator iterator)
        : m_queue(queue),
          m_iterator(iterator)
    {
    }

    Derived& operator=(const IteratorBase& other)
    {
        COMMS_ASSERT(&m_queue == &other.m_queue);
        m_iterator = other.m_iterator; // No need to check for self assignment
        return static_cast<Derived&>(*this);
    }

    Derived& operator++()
    {
        ++m_iterator;
        return static_cast<Derived&>(*this);
    }

    Derived operator++(int)
    {
        IteratorBase copy(*this);
        ++m_iterator;
        return std::move(*(static_cast<Derived*>(&copy)));
    }

    Derived& operator--()
    {
        --m_iterator;
        return static_cast<Derived&>(*this);
    }

    Derived operator--(int)
    {
        IteratorBase copy(*this);
        --m_iterator;
        return std::move(*(static_cast<Derived*>(&copy)));
    }

    Derived& operator+=(DifferenceType value)
    {
        m_iterator += value;
        return static_cast<Derived&>(*this);
    }

    Derived& operator-=(DifferenceType value)
    {
        m_iterator -= value;
        return static_cast<Derived&>(*this);
    }

    Derived operator+(DifferenceType value) const
    {
        IteratorBase copy(*this);
        copy += value;
        return std::move(*(static_cast<Derived*>(&copy)));
    }

    Derived operator-(DifferenceType value) const
    {
        IteratorBase copy(*this);
        copy -= value;
        return std::move(*(static_cast<Derived*>(&copy)));
    }

    DifferenceType operator-(const IteratorBase& other) const
    {
        return m_iterator - other.m_iterator;
    }

    bool operator==(const IteratorBase& other) const
    {
        return (m_iterator == other.m_iterator);
    }

    bool operator!=(const IteratorBase& other) const
    {
        return (m_iterator != other.m_iterator);
    }

    bool operator<(const IteratorBase& other) const
    {
        return m_iterator < other.m_iterator;
    }

    bool operator<=(const IteratorBase& other) const
    {
        return m_iterator <= other.m_iterator;
    }

    bool operator>(const IteratorBase& other) const
    {
        return m_iterator > other.m_iterator;
    }

    bool operator>=(const IteratorBase& other) const
    {
        return m_iterator >= other.m_iterator;
    }

    Reference operator*()
    {
        auto& constThisRef = static_cast<const IteratorBase&>(*this);
        auto& constRef = *constThisRef;
        return const_cast<Reference>(constRef);
    }

    ConstReference operator*() const
    {
        auto begCell = reinterpret_cast<ArrayIterator>(&m_queue.m_data[0]);
        auto idx = m_iterator - begCell;
        COMMS_ASSERT(0 <= idx);
        return m_queue[static_cast<std::size_t>(idx)];
    }

    Pointer operator->()
    {
        return &(*(*this));
    }

    ConstPointer operator->() const
    {
        return &(*(*this));
    }

    QueueType& getQueue() {
        return m_queue;
    }

    typename std::add_const<QueueType&>::type getQueue() const
    {
        return m_queue;
    }

    ArrayIterator& getIterator() {
        return m_iterator;
    }

    typename std::add_const<ArrayIterator>::type& getIterator() const
    {
        return m_iterator;
    }

private:

    QueueType& m_queue; ///< Queue
    ArrayIterator m_iterator; ///< Low level array iterator
};

template <typename T>
class StaticQueueBase<T>::ConstIterator :
            public StaticQueueBase<T>::template
                IteratorBase<typename StaticQueueBase<T>::ConstIterator, const StaticQueueBase<T> >
{
    using Base = typename StaticQueueBase<T>::template
        IteratorBase<typename StaticQueueBase<T>::ConstIterator, const StaticQueueBase<T> >;

public:
    using QueueType = typename Base::QueueType;
    using ArrayIterator = typename Base::ArrayIterator;

    ConstIterator(const ConstIterator&) = default;
    ~ConstIterator() noexcept = default;

    ConstIterator(QueueType& queue, ArrayIterator iterator)
        : Base(queue, iterator)
    {
    }

};

template <typename T>
class StaticQueueBase<T>::Iterator :
            public StaticQueueBase<T>::template
                IteratorBase<typename StaticQueueBase<T>::Iterator, StaticQueueBase<T> >
{
    using Base = typename StaticQueueBase<T>::template
        IteratorBase<typename StaticQueueBase<T>::Iterator, StaticQueueBase<T> >;

public:
    using QueueType = typename Base::QueueType;
    using ArrayIterator = typename Base::ArrayIterator;

    Iterator(const Iterator&) = default;
    ~Iterator() noexcept = default;

    Iterator(QueueType& queue, ArrayIterator iterator)
        : Base(queue, iterator)
    {
    }

    operator ConstIterator() const
    {
        return ConstIterator(Base::getQueue(), Base::getIterator());
    }
};


template <typename TWrapperElemType, typename TQueueElemType>
class CastWrapperQueueBase : public StaticQueueBase<TQueueElemType>
{
    using Base = StaticQueueBase<TQueueElemType>;
    using WrapperElemType = TWrapperElemType;

    using BaseValueType = typename Base::ValueType;
    using BaseStorageTypePtr = typename Base::StorageTypePtr;
    using BaseReference = typename Base::Reference;
    using BaseConstReference = typename Base::ConstReference;
    using BasePointer = typename Base::Pointer;
    using BaseConstPointer = typename Base::ConstPointer;
    using BaseLinearisedIterator = typename Base::LinearisedIterator;
    using BaseConstLinearisedIterator = typename Base::ConstLinearisedIterator;
    using BaseReverseLinearisedIterator = typename Base::ReverseLinearisedIterator;
    using BaseConstReverseLinearisedIterator = typename Base::ConstReverseLinearisedIterator;
    using BaseLinearisedIteratorRange = typename Base::LinearisedIteratorRange;
    using BaseConstLinearisedIteratorRange = typename Base::ConstLinearisedIteratorRange;

public:

    class ConstIterator;
    class Iterator;

protected:
    using ValueType = WrapperElemType;
    using StorageType = comms::util::AlignedStorage<sizeof(ValueType), std::alignment_of<ValueType>::value>;
    using StorageTypePtr = StorageType*;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;
    using Pointer = ValueType*;
    using ConstPointer = const ValueType*;
    using LinearisedIterator = Pointer;
    using ConstLinearisedIterator = ConstPointer;
    using ReverseLinearisedIterator = std::reverse_iterator<LinearisedIterator>;
    using ConstReverseLinearisedIterator = std::reverse_iterator<ConstLinearisedIterator>;
    using LinearisedIteratorRange = std::pair<LinearisedIterator, LinearisedIterator>;
    using ConstLinearisedIteratorRange = std::pair<ConstLinearisedIterator, ConstLinearisedIterator>;

    CastWrapperQueueBase(StorageTypePtr data, std::size_t capacity)
        : Base(reinterpret_cast<BaseStorageTypePtr>(data), capacity)
    {
        static_assert(sizeof(ValueType) == sizeof(BaseValueType),
            "The times must have identical size.");
    }

    ~CastWrapperQueueBase() noexcept = default;

    CastWrapperQueueBase& operator=(const CastWrapperQueueBase& other) = default;
    CastWrapperQueueBase& operator=(CastWrapperQueueBase&& other) = default;

    Reference front()
    {
        return reinterpret_cast<Reference>(Base::front());
    }

    ConstReference front() const
    {
        return reinterpret_cast<ConstReference>(Base::front());
    }

    Reference back()
    {
        return reinterpret_cast<Reference>(Base::back());
    }

    ConstReference back() const
    {
        return reinterpret_cast<Reference>(Base::back());
    }

    Reference operator[](std::size_t index)
    {
        return reinterpret_cast<Reference>(Base::operator[](index));
    }

    ConstReference operator[](std::size_t index) const
    {
        return reinterpret_cast<ConstReference>(Base::operator[](index));
    }

    Reference at(std::size_t index)
    {
        return reinterpret_cast<Reference>(Base::at(index));
    }

    ConstReference at(std::size_t index) const
    {
        return reinterpret_cast<ConstReference>(Base::at(index));
    }

    int indexOf(ConstReference element) const
    {
        return Base::indexOf(reinterpret_cast<BaseConstReference>(element));
    }

    LinearisedIterator invalidIter()
    {
        return reinterpret_cast<LinearisedIterator>(Base::invalidIter());
    }

    ConstLinearisedIterator invalidIter() const
    {
        return reinterpret_cast<ConstLinearisedIterator>(Base::invalidIter());
    }

    ReverseLinearisedIterator invalidReverseIter()
    {
        return ReverseLinearisedIterator(
            reinterpret_cast<LinearisedIterator>(
                Base::invalidReverseIter().base()));
    }

    ConstReverseLinearisedIterator invalidReverseIter() const
    {
        return ConstReverseLinearisedIterator(
            reinterpret_cast<ConstLinearisedIterator>(
                Base::invalidReverseIter().base()));
    }

    LinearisedIterator lbegin()
    {
        return reinterpret_cast<LinearisedIterator>(Base::lbegin());
    }

    ConstLinearisedIterator lbegin() const
    {
        return reinterpret_cast<ConstLinearisedIterator>(Base::lbegin());
    }

    ConstLinearisedIterator clbegin() const
    {
        return reinterpret_cast<ConstLinearisedIterator>(Base::clbegin());
    }

    ReverseLinearisedIterator rlbegin()
    {
        return ReverseLinearisedIterator(
            reinterpret_cast<LinearisedIterator>(
                Base::rlbegin().base()));
    }

    ConstReverseLinearisedIterator rlbegin() const
    {
        return ConstReverseLinearisedIterator(
            reinterpret_cast<ConstLinearisedIterator>(
                Base::rlbegin().base()));
    }

    ConstReverseLinearisedIterator crlbegin() const
    {
        return ConstReverseLinearisedIterator(
            reinterpret_cast<ConstLinearisedIterator>(
                Base::crlbegin().base()));
    }

    LinearisedIterator lend()
    {
        return reinterpret_cast<LinearisedIterator>(Base::lend());
    }

    ConstLinearisedIterator lend() const
    {
        return reinterpret_cast<ConstLinearisedIterator>(Base::lend());
    }

    ConstLinearisedIterator clend() const
    {
        return reinterpret_cast<ConstLinearisedIterator>(Base::clend());
    }

    ReverseLinearisedIterator rlend()
    {
        return ReverseLinearisedIterator(
            reinterpret_cast<LinearisedIterator>(
                Base::rlend().base()));
    }

    ConstReverseLinearisedIterator rlend() const
    {
        return ConstReverseLinearisedIterator(
            reinterpret_cast<ConstLinearisedIterator>(
                Base::rlend().base()));
    }

    ConstReverseLinearisedIterator crlend() const
    {
        return ConstReverseLinearisedIterator(
            reinterpret_cast<ConstLinearisedIterator>(
                Base::crlend().base()));
    }

    LinearisedIteratorRange arrayOne()
    {
        auto range = Base::arrayOne();
        return LinearisedIteratorRange(
            reinterpret_cast<LinearisedIterator>(range.first),
            reinterpret_cast<LinearisedIterator>(range.second));
    }

    ConstLinearisedIteratorRange arrayOne() const
    {
        auto range = Base::arrayOne();
        return ConstLinearisedIteratorRange(
            reinterpret_cast<ConstLinearisedIterator>(range.first),
            reinterpret_cast<ConstLinearisedIterator>(range.second));

    }

    LinearisedIteratorRange arrayTwo()
    {
        auto range = Base::arrayTwo();
        return LinearisedIteratorRange(
            reinterpret_cast<LinearisedIterator>(range.first),
            reinterpret_cast<LinearisedIterator>(range.second));

    }

    ConstLinearisedIteratorRange arrayTwo() const
    {
        auto range = Base::arrayTwo();
        return ConstLinearisedIteratorRange(
            reinterpret_cast<ConstLinearisedIterator>(range.first),
            reinterpret_cast<ConstLinearisedIterator>(range.second));

    }

    LinearisedIterator erase(LinearisedIterator pos)
    {
        return reinterpret_cast<LinearisedIterator>(
            Base::erase(
                reinterpret_cast<BaseLinearisedIterator>(pos)));
    }

    Iterator erase(Iterator pos)
    {
        auto tmp = Base::erase(pos);
        return *(reinterpret_cast<Iterator*>(&tmp));
    }

    Iterator begin()
    {
        auto tmp = Base::begin();
        return *(reinterpret_cast<Iterator*>(&tmp));
    }

    ConstIterator begin() const
    {
        auto tmp = Base::begin();
        return *(reinterpret_cast<ConstIterator*>(&tmp));
    }

    ConstIterator cbegin() const
    {
        auto tmp = Base::cbegin();
        return *(reinterpret_cast<ConstIterator*>(&tmp));

    }

    Iterator end()
    {
        auto tmp = Base::end();
        return *(reinterpret_cast<Iterator*>(&tmp));
    }

    ConstIterator end() const
    {
        auto tmp = Base::end();
        return *(reinterpret_cast<ConstIterator*>(&tmp));
    }

    ConstIterator cend() const
    {
        auto tmp = Base::cend();
        return *(reinterpret_cast<ConstIterator*>(&tmp));
    }

    void pushBack(ConstReference value)
    {
        Base::pushBack(reinterpret_cast<BaseConstReference>(value));
    }

    void pushFront(ConstReference value)
    {
        Base::pushFront(reinterpret_cast<BaseConstReference>(value));
    }

    LinearisedIterator insert(LinearisedIterator pos, ConstReference value)
    {
        return reinterpret_cast<LinearisedIterator>(
            Base::insert(
                reinterpret_cast<BaseLinearisedIterator>(pos),
                reinterpret_cast<BaseConstReference>(value)));
    }

    void assignElements(const CastWrapperQueueBase& other)
    {
        Base::assignElements(static_cast<const Base&>(other));
    }

    void assignElements(CastWrapperQueueBase&& other)
    {
        Base::assignElements(static_cast<Base&&>(std::move(other)));
    }
};

template <typename TWrapperElemType, typename TQueueElemType>
class CastWrapperQueueBase<TWrapperElemType, TQueueElemType>::ConstIterator :
                            public StaticQueueBase<TQueueElemType>::ConstIterator
{
    using Base = typename StaticQueueBase<TQueueElemType>::ConstIterator;
public:
    ConstIterator(const ConstIterator&) = default;
    ConstIterator& operator=(const ConstIterator&) = default;
    ~ConstIterator() noexcept = default;

protected:
    using ExpectedQueueType = const StaticQueueBase<TWrapperElemType>;
    using ActualQueueType = const StaticQueueBase<TQueueElemType>;
    using ValueType = TWrapperElemType;
    using Reference = const ValueType&;
    using ConstReference = const ValueType&;
    using Pointer = const ValueType*;
    using ConstPointer = const ValueType*;
    using DifferenceType = typename Base::DifferenceType;

    ConstIterator(ExpectedQueueType& queue, Pointer iterator)
        : Base(reinterpret_cast<ActualQueueType&>(queue), iterator)
    {
    }

    ConstIterator& operator++()
    {
        Base::operator++();
        return *this;
    }

    ConstIterator operator++(int dummy)
    {
        auto tmp = Base::operator++(dummy);
        return *(static_cast<ConstIterator*>(&tmp));
    }

    ConstIterator& operator--()
    {
        Base::operator--();
        return *this;
    }

    ConstIterator operator--(int dummy)
    {
        auto tmp = Base::operator--(dummy);
        return *(static_cast<ConstIterator*>(&tmp));
    }

    ConstIterator& operator+=(DifferenceType value)
    {
        Base::operator+=(value);
        return *this;
    }

    ConstIterator& operator-=(DifferenceType value)
    {
        Base::operator-=(value);
        return *this;
    }

    ConstIterator operator+(DifferenceType value) const
    {
        auto tmp = Base::operator+(value);
        return *(static_cast<ConstIterator*>(&tmp));
    }

    ConstIterator operator-(DifferenceType value) const
    {
        auto tmp = Base::operator-(value);
        return *(static_cast<ConstIterator*>(&tmp));
    }

    DifferenceType operator-(const ConstIterator& other) const
    {
        return Base::operator-(other);
    }

    Reference operator*()
    {
        auto& ref = Base::operator*();
        return reinterpret_cast<Reference>(ref);
    }

    ConstReference operator*() const
    {
        auto& ref = Base::operator*();
        return reinterpret_cast<ConstReference>(ref);
    }

    Pointer operator->()
    {
        auto* ptr = Base::operator->();
        return reinterpret_cast<Pointer>(ptr);
    }

    ConstPointer operator->() const
    {
        auto* ptr = Base::operator->();
        return reinterpret_cast<ConstPointer>(ptr);
    }
};

template <typename TWrapperElemType, typename TQueueElemType>
class CastWrapperQueueBase<TWrapperElemType, TQueueElemType>::Iterator :
                            public StaticQueueBase<TQueueElemType>::Iterator
{
    using Base = typename StaticQueueBase<TQueueElemType>::Iterator;
public:
    Iterator(const Iterator&) = default;
    Iterator& operator=(const Iterator&) = default;
    ~Iterator() noexcept = default;

protected:
    using ExpectedQueueType = const StaticQueueBase<TWrapperElemType>;
    using ActualQueueType = const StaticQueueBase<TQueueElemType>;
    using ValueType = TWrapperElemType;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;
    using Pointer = ValueType*;
    using ConstPointer = const ValueType*;
    using DifferenceType = typename Base::DifferenceType;

    Iterator(ExpectedQueueType& queue, Pointer iterator)
        : Base(reinterpret_cast<ActualQueueType&>(queue), iterator)
    {
    }

    Iterator& operator++()
    {
        Base::operator++();
        return *this;
    }

    Iterator operator++(int dummy)
    {
        auto tmp = Base::operator++(dummy);
        return *(static_cast<Iterator*>(&tmp));
    }

    Iterator& operator--()
    {
        Base::operator--();
        return *this;
    }

    Iterator operator--(int dummy)
    {
        auto tmp = Base::operator--(dummy);
        return *(static_cast<Iterator*>(&tmp));
    }

    Iterator& operator+=(DifferenceType value)
    {
        Base::operator+=(value);
        return *this;
    }

    Iterator& operator-=(DifferenceType value)
    {
        Base::operator-=(value);
        return *this;
    }

    Iterator operator+(DifferenceType value) const
    {
        auto tmp = Base::operator+(value);
        return *(static_cast<Iterator*>(&tmp));
    }

    Iterator operator-(DifferenceType value) const
    {
        auto tmp = Base::operator-(value);
        return *(static_cast<Iterator*>(&tmp));
    }

    DifferenceType operator-(const Iterator& other) const
    {
        return Base::operator-(other);
    }

    Reference operator*()
    {
        auto& ref = Base::operator*();
        return reinterpret_cast<Reference>(ref);
    }

    ConstReference operator*() const
    {
        auto& ref = Base::operator*();
        return reinterpret_cast<ConstReference>(ref);
    }

    Pointer operator->()
    {
        auto* ptr = Base::operator->();
        return reinterpret_cast<Pointer>(ptr);
    }

    ConstPointer operator->() const
    {
        auto* ptr = Base::operator->();
        return reinterpret_cast<ConstPointer>(ptr);
    }
};

template <typename T>
class StaticQueueBaseOptimised : public StaticQueueBase<T>
{
    usign Base = StaticQueueBase<T>;
protected:

    using StorageTypePtr = typename Base::StorageTypePtr;

    StaticQueueBaseOptimised(StorageTypePtr data, std::size_t capacity)
        : Base(data, capacity)
    {
    }

    ~StaticQueueBaseOptimised() noexcept = default;

    StaticQueueBaseOptimised& operator=(const StaticQueueBaseOptimised& other) = default;
    StaticQueueBaseOptimised& operator=(StaticQueueBaseOptimised&& other) = default;
};

template <>
class StaticQueueBaseOptimised<std::int8_t> : public CastWrapperQueueBase<std::int8_t, std::uint8_t>
{
    using Base = CastWrapperQueueBase<std::int8_t, std::uint8_t>;
protected:

    using StorageTypePtr = typename Base::StorageTypePtr;

    StaticQueueBaseOptimised(StorageTypePtr data, std::size_t capacity)
        : Base(data, capacity)
    {
    }

    ~StaticQueueBaseOptimised() noexcept = default;
    StaticQueueBaseOptimised& operator=(const StaticQueueBaseOptimised& other) = default;
    StaticQueueBaseOptimised& operator=(StaticQueueBaseOptimised&& other) = default;
};

template <>
class StaticQueueBaseOptimised<std::int16_t> : public CastWrapperQueueBase<std::int16_t, std::uint16_t>
{
    using Base = CastWrapperQueueBase<std::int16_t, std::uint16_t>;
protected:

    using StorageTypePtr = typename Base::StorageTypePtr;

    StaticQueueBaseOptimised(StorageTypePtr data, std::size_t capacity)
        : Base(data, capacity)
    {
    }

    ~StaticQueueBaseOptimised() noexcept = default;
    StaticQueueBaseOptimised& operator=(const StaticQueueBaseOptimised& other) = default;
    StaticQueueBaseOptimised& operator=(StaticQueueBaseOptimised&& other) = default;
};

template <>
class StaticQueueBaseOptimised<std::int32_t> : public CastWrapperQueueBase<std::int32_t, std::uint32_t>
{
    using Base = CastWrapperQueueBase<std::int32_t, std::uint32_t>;
protected:

    using StorageTypePtr = typename Base::StorageTypePtr;

    StaticQueueBaseOptimised(StorageTypePtr data, std::size_t capacity)
        : Base(data, capacity)
    {
    }

    ~StaticQueueBaseOptimised() noexcept = default;
    StaticQueueBaseOptimised& operator=(const StaticQueueBaseOptimised& other) = default;
    StaticQueueBaseOptimised& operator=(StaticQueueBaseOptimised&& other) = default;
};

template <>
class StaticQueueBaseOptimised<std::int64_t> : public CastWrapperQueueBase<std::int64_t, std::uint64_t>
{
    using Base = CastWrapperQueueBase<std::int64_t, std::uint64_t>;
protected:

    using StorageTypePtr = typename Base::StorageTypePtr;

    StaticQueueBaseOptimised(StorageTypePtr data, std::size_t capacity)
        : Base(data, capacity)
    {
    }

    ~StaticQueueBaseOptimised() noexcept = default;
    StaticQueueBaseOptimised& operator=(const StaticQueueBaseOptimised& other) = default;
    StaticQueueBaseOptimised& operator=(StaticQueueBaseOptimised&& other) = default;
};

template <typename T>
class StaticQueueBaseOptimised<T*> : public CastWrapperQueueBase<T*, typename comms::util::SizeToType<sizeof(T*)>::Type>
{
    using Base = CastWrapperQueueBase<T*, typename comms::util::SizeToType<sizeof(T*)>::Type>;
protected:

    using Base = typename Base::StorageTypePtr;

    StaticQueueBaseOptimised(StorageTypePtr data, std::size_t capacity)
        : Base(data, capacity)
    {
    }

    ~StaticQueueBaseOptimised() noexcept = default;
    StaticQueueBaseOptimised& operator=(const StaticQueueBaseOptimised& other) = default;
    StaticQueueBaseOptimised& operator=(StaticQueueBaseOptimised&& other) = default;
};


}  // namespace details

/// @cond SKIP_DOC

/// @brief Template class for definition of the static queues or circular
///        buffers.
/// @details The main distinction of this class from the alternatives
///          such as std::vector, std::deque or boost::circular_buffer is
///          that this implementation doesn't use dynamic memory allocation
///          and exceptions (except in "at()" member function). Hence, it
///          is suitable for use in pure embedded environment.
/// @tparam T Type of the stored element.
/// @tparam TSize Size of the queue in number - maximum number of stored
///         elements.
template <typename T, std::size_t TSize>
class StaticQueue : public details::StaticQueueBaseOptimised<T>
{
    using Base = details::StaticQueueBaseOptimised<T>;

    using StorageType = typename Base::StorageType;
public:
    /// @brief Type of the stored elements.
    using ValueType = typename Base::ValueType;

    /// @brief Same as ValueType
    using value_type = ValueType;

    /// @brief Size type.
    using SizeType = typename Base::SizeType;

    /// @brief Same as SizeType
    using size_type = SizeType;

    /// @brief Reference type to the stored elements.
    using Reference = typename Base::Reference;

    /// @brief Same as Reference
    using reference = Reference;

    /// @brief Const reference type to the stored elements.
    using ConstReference = typename Base::ConstReference;

    /// @brief Same as ConstReference
    using const_reference = ConstReference;

    /// @brief Pointer type to the stored elements.
    using Pointer = typename Base::Pointer;

    /// @brief Same as Pointer
    using pointer = Pointer;

    /// @brief Const pointer type to the stored elements.
    using ConstPointer = typename Base::ConstPointer;

    /// @brief Same as Pointer
    using const_pointer = ConstPointer;

    /// @brief Linearised iterator type
    using LinearisedIterator = typename Base::LinearisedIterator;

    /// @brief Const linearised iterator type
    using ConstLinearisedIterator = typename Base::ConstLinearisedIterator;

    /// @brief Reverse linearised iterator type
    using ReverseLinearisedIterator = typename Base::ReverseLinearisedIterator;

    /// @brief Const reverse linearised iterator type
    using ConstReverseLinearisedIterator = typename Base::ConstReverseLinearisedIterator;

    /// @brief Linearised iterator range type - std::pair of (first, one-past-last) iterators.
    using LinearisedIteratorRange = typename Base::LinearisedIteratorRange;

    /// @brief Const version of IteratorRange
    using ConstLinearisedIteratorRange = typename Base::ConstLinearisedIteratorRange;

    /// @brief Const iterator class
    class ConstIterator;

    /// @brief Same as ConstIterator
    using const_iterator = ConstIterator;

    /// @brief Iterator class
    class Iterator;

    /// @brief Same as Iterator
    using iterator = Iterator;

    // Member functions
    /// @brief Default constructor.
    /// @details Creates empty queue.
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw
    StaticQueue()
        : Base(&m_array[0], TSize)
    {
    }

    /// @brief Copy constructor
    /// @details Copies all the elements from the provided queue using
    ///          copy constructor of the elements.
    /// @param[in] queue Other queue.
    /// @note Thread safety: Depends of thread safety of the copy constructor
    ///       of the copied elements.
    /// @note Exception guarantee: No throw in case copy constructor
    ///       of the internal elements do not throw, Basic otherwise.
    StaticQueue(const StaticQueue& queue)
        : Base(&m_array[0], TSize)
    {
        Base::assignElements(queue);
    }

    /// @brief Move constructor
    /// @details Copies all the elements from the provided queue using
    ///          move constructor of the elements.
    /// @param[in] queue Other queue.
    /// @note Thread safety: Depends of thread safety of the move constructor
    ///       of the copied elements.
    /// @note Exception guarantee: No throw in case move constructor
    ///       of the internal elements do not throw, Basic otherwise.
    StaticQueue(StaticQueue&& queue)
        : Base(&m_array[0], TSize)
    {
        Base::assignElements(std::move(queue));
    }


    /// @brief Pseudo copy constructor
    /// @details Copies all the elements from a queue of any static size using
    ///          copy constructor of the elements.
    /// @param[in] queue Other queue.
    /// @note Thread safety: Depends of thread safety of the copy constructor
    ///       of the copied elements.
    /// @note Exception guarantee: No throw in case copy constructor
    ///       of the internal elements do not throw, Basic otherwise.
    template <std::size_t TAnySize>
    StaticQueue(const StaticQueue<T, TAnySize>& queue)
        : Base(&m_array[0], TSize)
    {
        Base::assignElements(queue);
    }

    /// @brief Pseudo move constructor
    /// @details Copies all the elements from a queue of any static size using
    ///          move constructor of the elements.
    /// @param[in] queue Other queue.
    /// @note Thread safety: Depends of thread safety of the move constructor
    ///       of the copied elements.
    /// @note Exception guarantee: No throw in case move constructor
    ///       of the internal elements do not throw, Basic otherwise.
    template <std::size_t TAnySize>
    StaticQueue(StaticQueue<T, TAnySize>&& queue)
        : Base(&m_array[0], TSize)
    {
        Base::assignElements(std::move(queue));
    }

    /// @brief Destructor
    /// @details The queue is cleared - the destructors of all the elements
    ///          in the queue are called
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       stored elements doesn't throw. Basic guarantee otherwise.
    ~StaticQueue() noexcept
    {
        clear();
    }

    /// @brief Copy assignment operator.
    /// @details Copies all the elements from the provided queue. Before the
    ///          copy, all the existing elements are cleared, i.e. their
    ///          destructors are called. To use this function the type of
    ///          stored element must provide copy constructor.
    /// @param[in] queue Queue to copy elements from
    /// @return Reference to current queue.
    /// @note Thread safety: Unsafe.
    /// @note Exception guarantee: No throw in case the copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    StaticQueue& operator=(const StaticQueue& queue)
    {
        return static_cast<StaticQueue&>(Base::operator=(queue));
    }

    /// @brief Move assignment operator.
    /// @details Copies all the elements from the provided queue. Before the
    ///          copy, all the existing elements are cleared, i.e. their
    ///          destructors are called. To use this function the type of
    ///          stored element must provide move constructor.
    /// @param[in] queue Queue to copy elements from
    /// @return Reference to current queue.
    /// @note Thread safety: Unsafe.
    /// @note Exception guarantee: No throw in case the move constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    StaticQueue& operator=(StaticQueue&& queue)
    {
        return static_cast<StaticQueue&>(Base::operator=(std::move(queue)));
    }


    /// @brief Pseudo copy assignment operator.
    /// @details Copies all the elements from the provided queue of any static
    ///          size. Before the copy, all the existing elements are cleared,
    ///          i.e. their destructors are called. To use this function the
    ///          type of stored element must provide copy constructor.
    /// @param[in] queue Queue to copy elements from
    /// @return Reference to current queue.
    /// @pre @code capacity() <= other.size() @endcode
    /// @note Thread safety: Unsafe.
    /// @note Exception guarantee: No throw in case the copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    template <std::size_t TAnySize>
    StaticQueue& operator=(const StaticQueue<T, TAnySize>& queue)
    {
        return static_cast<StaticQueue&>(Base::operator=(queue));
    }

    /// @brief Pseudo move assignment operator.
    /// @details Moves all the elements from the provided queue of any static
    ///          size. Before the move, all the existing elements are cleared,
    ///          i.e. their destructors are called. To use this function the
    ///          type of stored element must provide move constructor.
    /// @param[in] queue Queue to copy elements from
    /// @pre @code capacity() <= other.size() @endcode
    /// @return Reference to current queue.
    /// @note Thread safety: Unsafe.
    /// @note Exception guarantee: No throw in case the move constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    template <std::size_t TAnySize>
    StaticQueue& operator=(StaticQueue<T, TAnySize>&& queue)
    {
        return static_cast<StaticQueue&>(Base::operator=(std::move(queue)));
    }

    /// @brief Returns capacity of the current queue
    /// @details Returns size of the queue provided in the class template
    ///          arguments
    /// @return Unsigned value, capacity of the queue.
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw.
    static constexpr std::size_t capacity()
    {
        return TSize;
    }

    /// @brief Returns current size of the queue.
    /// @details When queue is empty 0 will be returned.
    ///          When queue is full the returned size is equal to the value
    ///          returned by capacity() member function.
    /// @return Unsigned value, size of the current queue.
    /// @note Thread safety: Safe for multiple readers.
    /// @note Exception guarantee: No throw.
    std::size_t size() const
    {
        return Base::size();
    }

    /// @brief Returns whether the queue is empty.
    /// @return Returns true if and only if (size() == 0U) is true,
    ///         false otherwise.
    /// @note Thread safety: Safe for multiple readers.
    /// @note Exception guarantee: No throw.
    bool empty() const
    {
        return Base::empty();
    }

    /// @brief Same as empty()
    bool isEmpty() const
    {
        return empty();
    }

    /// @brief Returns whether the queue is full.
    /// @return Returns true if and only if (size() == capacity()) is true,
    ///         false otherwise.
    /// @note Thread safety: Safe for multiple readers.
    /// @note Exception guarantee: No throw.
    bool full() const
    {
        return Base::full();
    }

    /// @brief Same as full();
    bool isFull() const
    {
        return full();
    }

    /// @brief Clears the queue from all the existing elements.
    /// @details The destructors of the stored elements will be called.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       stored elements doesn't throw. Basic guarantee otherwise.
    void clear()
    {
        Base::clear();
    }

    /// @brief Pop the element from the back of the queue.
    /// @details The destructor of the popped element is called.
    /// @pre The queue is not empty.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       popped element doesn't throw. Basic guarantee otherwise.
    void popBack()
    {
        Base::popBack();
    }

    /// @brief Same as popBack()
    inline void pop_back()
    {
        popBack();
    }

    /// @brief Pop number of the elements from the back of the queue.
    /// @details The destructors of the popped elements are called. In case the
    ///          queue is or gets empty in the process, no exception is
    ///          thrown.
    /// @param[in] count number of elements to pop.
    /// @pre @code count <= size() @endcode.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       popped element doesn't throw. Basic guarantee otherwise.
    void popBack(std::size_t count)
    {
        Base::popBack(count);
    }

    /// @brief Same as popBack(std::size_t)
    void pop_back(std::size_t count)
    {
        popBack(count);
    }

    /// @brief Pop the element from the front of the queue.
    /// @details The destructor of the popped element is called.
    /// @pre The queue is not empty.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       popped element doesn't throw. Basic guarantee otherwise.
    void popFront()
    {
        Base::popFront();
    }

    /// @brief Same as popFront()
    inline void pop_front()
    {
        popFront();
    }

    /// @brief Pop number of the elements from the front of the queue.
    /// @details The destructors of the popped elements are called.
    /// @param[in] count number of elements to pop.
    /// @pre @code count <= size() @endcode
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the destructor of the
    ///       popped element doesn't throw. Basic guarantee otherwise.
    void popFront(std::size_t count)
    {
        Base::popFront(count);
    }

    /// @brief Same as popFront(std::size_t)
    inline void pop_front(std::size_t count)
    {
        popFront(count);
    }

    /// @brief Add new element to the end of the queue.
    /// @details Uses copy/move constructor to copy/move the provided element.
    /// @param[in] value Value to insert
    /// @pre The queue is not full
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    template <typename U>
    void pushBack(U&& value)
    {
        Base::pushBack(std::forward<U>(value));
    }

    /// @brief Construct new element at the end of the queue.
    /// @details Passes all the provided arguments to the constructor of the
    ///          element.
    /// @param[in] args Parameters to the constructor of the element
    /// @pre The queue is not full
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    template <typename... TArgs>
    void emplaceBack(TArgs&&... args)
    {
        Base::emplaceBack(std::forward<TArgs>(args)...);
    }

    /// @brief Same as pushBack(U&&);
    template <typename U>
    inline void push_back(U&& value)
    {
        pushBack(std::forward<U>(value));
    }

    /// @brief Add new element to the beginning of the queue.
    /// @details Uses copy/move constructor to copy/move the provided element.
    /// @param[in] value Value to insert
    /// @pre The queue is not full.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    template <typename U>
    void pushFront(U&& value)
    {
        Base::pushFront(std::forward<U>(value));
    }

    /// @brief Same as pushFront(U&&);
    template <typename U>
    inline void push_front(U&& value)
    {
        pushFront(std::forward<U>(value));
    }

    /// @brief Insert new element at specified position.
    /// @details The function uses linearised iterator to specify position.
    /// @param[in] pos Linearised iterator to the insert position.
    /// @param[in] value New value to insert.
    /// @return Linearised iterator to the newly inserted element.
    /// @pre (pos != invalidIter())
    /// @pre pos is either in iterator range returned by arrayOne() or
    ///      returned by arrayTwo() or equal to arrayTwo().second.
    /// @pre The queue is not full.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case the move/copy constructor
    ///       of the stored elements doesn't throw. Basic guarantee otherwise.
    /// @see invalidIter()
    /// @see arrayOne()
    /// @see arrayTwo()
    template <typename U>
    LinearisedIterator insert(LinearisedIterator pos, U&& value)
    {
        return Base::insert(pos, std::forward<U>(value));
    }


    /// @brief Provides reference to the front element.
    /// @return Reference to the front element.
    /// @pre The queue is not empty.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    Reference front()
    {
        return Base::front();
    }

    /// @brief Const version of front().
    ConstReference front() const
    {
        return Base::front();
    }

    /// @brief Provides reference to the last element.
    /// @return Reference to the last element.
    /// @pre the queue is not empty.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    Reference back()
    {
        return Base::back();
    }

    /// @brief Const version of back().
    ConstReference back() const
    {
        return Base::back();
    }

    /// @brief Provides reference to the specified element
    /// @details In case the index is out of range, the returned value will be
    ///          a reference to some cell of the internal array. There is no
    ///          guarantee that the referenced object will be a valid one.
    /// @param[in] index Index of the object from the front of the queue.
    /// @return Reference to the stored element.
    /// @pre The index must be within the closed-open range of [0 - size()).
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    Reference operator[](std::size_t index)
    {
        return Base::operator[](index);
    }

    /// @brief Const version of operator[]().
    ConstReference operator[](std::size_t index) const
    {
        return Base::operator[](index);
    }

    /// @brief Provides reference to the specified element
    /// @details In case the index is out of range, the std::out_of_range
    ///          exception is thrown. This function is not suitable for the
    ///          pure embedded environment with small memory footprint
    ///          where exceptions mustn't be used. In this case use regular
    ///          boundary check combined with operator[].
    /// @param[in] index Index of the object from the front of the queue.
    /// @return Reference to the stored element.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: Strong.
    Reference at(std::size_t index)
    {
        return Base::at(index);
    }

    /// @brief Const version of at().
    ConstReference at(std::size_t index) const
    {
        return Base::at(index);
    }

    /// @brief Return index of the element in the current queue.
    /// @param[in] element Reference to the element.
    /// @return Non-negative value in case the element is stored in the
    ///         current queue, -1 otherwise.
    /// @post if returned index is not -1, than the operator[](<returned index>)
    ///       returns reference to the same parameter element.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    int indexOf(ConstReference element) const
    {
        return Base::indexOf(element);
    }

    /// @brief Invalid iterator
    /// @details Returns value of invalid iterator, always equal to the end
    ///          of underlying array.
    /// @return Value of invalid iterator
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw
    LinearisedIterator invalidIter()
    {
        return Base::invalidIter();
    }

    /// @brief Const version of invalidIter()
    ConstLinearisedIterator invalidIter() const
    {
        return Base::invalidIter();
    }

    /// @brief Invalid reverse iterator
    /// @details Returns value of invalid reverse iterator.
    /// @return Value of invalid reverse iterator
    /// @note Thread safety: Safe
    /// @note Exception guarantee: No throw
    ReverseLinearisedIterator invalidReverseIter()
    {
        return Base::invalidReverseIter();
    }

    /// @brief Const version of invalidReverseIter()
    ConstReverseLinearisedIterator invalidReverseIter() const
    {
        return Base::invalidReverseIter();
    }

    /// @brief Returns iterator to the linearised beginning.
    /// @details In case the queue is not linearised
    ///          the returned iterator will be the same as one returned
    ///          by the invalidIter() member function. Like in most standard
    ///          sequential containers the iterator may get invalid in
    ///          case the queue is updated in the middle of the iteration.
    /// @return Linearised iterator pointing to the first element in the queue.
    /// @pre The queue is linearised.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see isLinearised()
    /// @see linearise()
    /// @see invalidIter()
    LinearisedIterator lbegin()
    {
        return Base::lbegin();
    }

    /// @brief Same as clbegin().
    ConstLinearisedIterator lbegin() const
    {
        return Base::lbegin();
    }

    /// @brief Const version of lbegin().
    ConstLinearisedIterator clbegin() const
    {
        return Base::clbegin();
    }

    /// @brief Returns reverse iterator to the reverse linearised beginning.
    /// @details In case the queue is not linearised
    ///          the returned iterator will be the same as one returned
    ///          by the invalidReverseIter() member function. Like in most
    ///          standard sequential containers the iterator may get invalid
    ///          in case the queue is updated in the middle of the iteration.
    /// @return Reverse iterator pointing to the last element in the queue.
    /// @pre The queue is linearised.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see isLinearised()
    /// @see linearise()
    /// @see invalidReverseIter()
    ReverseLinearisedIterator rlbegin()
    {
        return Base::rlbegin();
    }

    /// @brief Same as crlbegin().
    ConstReverseLinearisedIterator rlbegin() const
    {
        return Base::rlbegin();
    }

    /// @brief Const version of rlbegin().
    ConstReverseLinearisedIterator crlbegin() const
    {
        return Base::crlbegin();
    }

    /// @brief Returns iterator to the linearised end.
    /// @details In case the queue is not linearised the returned iterator
    ///          will the same as returned by invalidIter().
    /// @return Linearised iterator referring to the past-the-end element in
    ///         the queue.
    /// @pre The queue is linearised.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see isLinearised()
    /// @see linearise()
    /// @see invalidIter()
    LinearisedIterator lend()
    {
        return Base::lend();
    }

    /// @brief Same as clend().
    ConstLinearisedIterator lend() const
    {
        return Base::lend();
    }

    /// @brief Const version of lend().
    ConstLinearisedIterator clend() const
    {
        return Base::clend();
    }

    /// @brief Returns reverse iterator to the reverse linearised end.
    /// @details In case the queue is not linearised the returned iterator
    ///          will be the same as returned by invalidReverseIter().
    /// @return Reverse iterator pointing to the element right before the
    ///         first element in the queue.
    /// @pre The queue is linearised.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see isLinearised()
    /// @see linearise()
    /// @see invalidReverseIter()
    ReverseLinearisedIterator rlend()
    {
        return Base::rlend();
    }

    /// @brief Same as crlend().
    ConstReverseLinearisedIterator rlend() const
    {
        return Base::rlend();
    }

    /// @brief Const version of rlend();
    ConstReverseLinearisedIterator crlend() const
    {
        return Base::crlend();
    }

    /// @brief Linearise internal elements.
    /// @details The elements of the queue are considered to be linearised
    ///          when the the pointer to every subsequent element will be
    ///          greater than the pointer to its predecessor. This queue is
    ///          implemented as a circular buffer over std::array.
    ///          After several push/pop operations there could
    ///          be a case when the pointer to the first element will be
    ///          greater than the one to the last element. In this situation
    ///          it is not possible to safely iterate over the elements
    ///          without introducing a complexity of checking for this
    ///          particular case when the iterator is incremented or
    ///          decremented. The design decision was to create two types of
    ///          iterators: LinearisedIterator and Iterator. LinearisedIterator
    ///          may be safely used for iteration only when the queue is
    ///          linearised.
    ///          When linearising elements of the queue, the move constructor
    ///          of the type T may be called several times when moving elements
    ///          around.
    /// @post The queue elements are linearised.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case move constructor/desctructor
    ///       of the internal elements do not throw, Basic otherwise.
    /// @see isLinearised()
    /// @see arrayOne()
    /// @see arrayTwo()
    void linearise()
    {
        Base::linearise();
    }

    /// @brief Returns whether the internal elements are linearised.
    /// @details The elements of the queue are considered to be linearised
    ///          when the the pointer to every subsequent element will be
    ///          greater than the pointer to its predecessor. In this case
    ///          it is possible to iterate over all the elements in a single
    ///          loop.
    /// @return true in case the element are linearised, false otherwise.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    bool linearised() const
    {
        return Base::linearised();
    }

    /// @brief Same as linearised();
    bool isLinearised() const
    {
        return linearised();
    }

    /// @brief Get the first continuous array of the internal buffer.
    /// @details This queue is implemented as a circular buffer over std::array.
    ///          After several push/pop operations there could
    ///          be a case when the pointer to the first element will be
    ///          greater than the one to the last element. As a result
    ///          it is not possible to properly iterate over all the elements
    ///          in a single loop. This function returns range of the linearised
    ///          iterators for the first continuous array of the internal buffer.
    /// @return Closed-open range (std::pair) of the iterators. Where the first
    ///         iterator refers to the front element of the queue while the
    ///         second iterator refers to either one-past-last or
    ///         one-past-some-middle element. In case the queue is empty,
    ///         both iterators are equal.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see linearise()
    /// @see isLinearised()
    /// @see arrayTwo()
    LinearisedIteratorRange arrayOne()
    {
        return Base::arrayOne();
    }

    /// @brief Const version of former arrayOne().
    ConstLinearisedIteratorRange arrayOne() const
    {
        return Base::arrayOne();
    }

    /// @brief Get the second continuous array of the internal buffer.
    /// @details This queue is implemented as a circular buffer over std::array.
    ///          After several push/pop operations there could
    ///          be a case when the pointer to the first element will be
    ///          greater than the one to the last element. As a result
    ///          it is not possible to properly iterate over all the elements
    ///          in a single loop. This function returns range of the iterators
    ///          for the second continuous array of the internal buffer.
    /// @return Closed-open range (std::pair) of the iterators. Where the first
    ///         iterator refers to the one of the middle elements of the queue
    ///         while the second iterator refers to either one-past-last
    ///         element. In case the queue is empty or linearised,
    ///         both iterators are equal to one returned by end() member
    ///         function.
    /// @post In case the queue is empty or linearised the first
    ///       iterator in the returned pair is equal to the second iterator
    ///       returned by the arrayOne() function.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    /// @see linearise()
    /// @see isLinearised()
    /// @see arrayOne()
    LinearisedIteratorRange arrayTwo()
    {
        return Base::arrayTwo();
    }

    /// @brief Const version of former arrayTwo().
    ConstLinearisedIteratorRange arrayTwo() const
    {
        return Base::arrayTwo();
    }

    /// @brief Resize the queue.
    /// @details In case the new size is greater than the existing one,
    ///          new elements are added to the back of the queue (default
    ///          constructor is required). In case the new size is less
    ///          than existing one, existing elements are popped from the
    ///          back of the queue.
    /// @param[in] newSize New size of the queue.
    /// @pre New size can not be greater than the capacity of the queue.
    /// @post The queue is resized to new size or unchanged in case
    ///       precondition wasn't satisfied.
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case constructor/desctructor
    ///       of the internal elements do not throw, Basic otherwise.
    void resize(std::size_t newSize)
    {
        Base::resize(newSize);
    }

    /// @brief Erase element.
    /// @details Erases element from specified position
    /// @param[in] pos Linearised iterator to the element to be erased
    /// @return Linearised iterator pointing to new location of
    ///         the next element after the erased one.
    /// @pre (pos != invalidIter())
    /// @pre pos is either in iterator range returned by arrayOne() or
    ///      returned by arrayTwo()
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case copy assignment operator
    ///       of the internal elements do not throw, Basic otherwise.
    LinearisedIterator erase(LinearisedIterator pos)
    {
        return Base::erase(pos);
    }

    /// @brief Erase element.
    /// @details Erases element from specified position
    /// @param[in] pos Iterator to the element to be erased
    /// @return Iterator pointing to new location of
    ///         the next element after the erased one.
    /// @pre (pos != end())
    /// @pre pos is in range [begin(), end())
    /// @note Thread safety: Unsafe
    /// @note Exception guarantee: No throw in case copy assignment operator
    ///       of the internal elements do not throw, Basic otherwise.
    Iterator erase(Iterator pos)
    {
        auto iter = Base::erase(pos);
        return *(static_cast<Iterator*>(&iter));
    }


    /// @brief Returns iterator to the beginning.
    /// @details This iterator works on the non-linearised queue. It has extra
    ///          overhead to check for wrap arounds.
    /// @return Iterator pointing to the first element in the queue.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    Iterator begin()
    {
        auto iter = Base::begin();
        return *(static_cast<Iterator*>(&iter));
    }

    /// @brief Same as cbegin();
    ConstIterator begin() const
    {
        auto iter = Base::begin();
        return *(static_cast<ConstIterator*>(&iter));
    }

    /// @brief Const version of begin();
    ConstIterator cbegin() const
    {
        auto iter = Base::cbegin();
        return *(static_cast<ConstIterator*>(&iter));
    }

    /// @brief Returns iterator to the end.
    /// @details This iterator works on the non-linearised queue. It has extra
    ///          overhead to check for wrap arounds.
    /// @return Iterator referring to the past-the-end element in
    ///         the queue.
    /// @note Thread safety: Safe for multiple readers, unsafe if there is
    ///       a writer.
    /// @note Exception guarantee: No throw.
    Iterator end()
    {
        auto iter = Base::end();
        return *(static_cast<Iterator*>(&iter));
    }

    /// @brief Same as cend();
    ConstIterator end() const
    {
        auto iter = Base::end();
        return *(static_cast<ConstIterator*>(&iter));
    }

    /// @brief Const version of end();
    ConstIterator cend() const
    {
        auto iter = Base::end();
        return *(static_cast<ConstIterator*>(&iter));
    }

    /// @brief Equality comparison operator
    template <std::size_t TAnySize>
    bool operator==(const StaticQueue<T, TAnySize>& other) const
    {
        return Base::operator==(other);
    }

    /// @brief Non-equality comparison operator
    template <std::size_t TAnySize>
    bool operator!=(const StaticQueue<T, TAnySize>& other) const
    {
        return Base::operator!=(other);
    }


private:
    using ArrayType = std::array<StorageType, TSize>;
    alignas(alignof(T)) ArrayType m_array;
};

/// @brief Const iterator for the elements of StaticQueue.
/// @details May be used to iterate over all the elements without worrying
///          about linearisation of the queue.
template <typename T, std::size_t TSize>
class StaticQueue<T, TSize>::ConstIterator : public StaticQueue<T, TSize>::Base::ConstIterator
{
    using Base = typename StaticQueue<T, TSize>::Base::ConstIterator;
public:

    /// @brief Type of iterator category
    using IteratorCategory = typename Base::IteratorCategory;

    /// @brief Same as IteratorCategory
    using iterator_category = IteratorCategory;

    /// @brief Type of the value referenced by the iterator
    using ValueType = typename Base::ValueType;

    /// @brief Same as ValueType
    using value_type = ValueType;

    /// @brief Type of the difference between two iterators
    using DifferenceType = typename Base::DifferenceType;

    /// @brief Same as DifferenceType
    using difference_type = DifferenceType;

    /// @brief Type of the pointer to the value referenced by the iterator
    using Pointer = typename Base::Pointer;

    /// @brief Same as Pointer
    using pointer = Pointer;

    /// @brief Const pointer type
    using ConstPointer = typename Base::ConstPointer;

    /// @brief Type of the reference to the value referenced by the iterator
    using Reference = typename Base::Reference;

    /// @brief Same as Reference
    using reference = Reference;

    /// @brief Const reference type
    using ConstReference = typename Base::ConstReference;

    /// @brief Queue type
    using QueueType = StaticQueue<T, TSize>;

    /// @brief Const linearised iterator
    using ConstLinearisedIterator = typename QueueType::ConstLinearisedIterator;

    /// @brief Constructor
    /// @param queue Reference to queue
    /// @param iterator Low level array iterator
    ConstIterator(const QueueType& queue, ConstLinearisedIterator iterator)
        : Base(queue, iterator)
    {
    }

    /// @brief Copy constructor is default.
    ConstIterator(const ConstIterator&) = default;

    /// @brief Copy assignment operator.
    /// @param other Other iterator
    /// @pre Other iterator must be iterator of the same queue.
    ConstIterator& operator=(const ConstIterator& other)
    {
        return static_cast<ConstIterator&>(Base::operator=(other));
    }

    /// @brief Pre increment operator.
    ConstIterator& operator++()
    {
        return static_cast<ConstIterator&>(Base::operator++());
    }

    /// @brief Post increment operator.
    ConstIterator operator++(int dummyParam)
    {
        auto tmp = Base::operator++(dummyParam);
        return *(static_cast<ConstIterator*>(&tmp));
    }

    /// @brief Pre decrement operator.
    ConstIterator& operator--()
    {
        return static_cast<ConstIterator&>(Base::operator--());
    }

    /// @brief Post-decrement operator.
    ConstIterator operator--(int dummyParam)
    {
        auto tmp = Base::operator--(dummyParam);
        return *(static_cast<ConstIterator*>(&tmp));
    }

    /// @brief Operator of adding constant value to the iterator
    /// @param value Value to be added
    ConstIterator& operator+=(DifferenceType value)
    {
        return static_cast<ConstIterator&>(Base::operator+=(value));
    }

    /// @brief Operator of substructing constant value from the iterator
    /// @param value Value to be substructed
    ConstIterator& operator-=(DifferenceType value)
    {
        return static_cast<ConstIterator&>(Base::operator-=(value));
    }

    /// @brief Operator +
    /// @details Creates new iterator object by adding constant to
    ///          current operator without changing it.
    /// @param value Value to be added
    ConstIterator operator+(DifferenceType value) const
    {
        auto tmp = Base::operator+(value);
        return *(static_cast<ConstIterator*>(&tmp));
    }

    /// @brief Operator -
    /// @details Creates new iterator object by substructing constant from
    ///          current operator without changing it.
    /// @param value Value to be substructed
    ConstIterator operator-(DifferenceType value) const
    {
        auto tmp = Base::operator-(value);
        return *(static_cast<ConstIterator*>(&tmp));
    }

    /// @brief Computes the distance between two iterators
    /// @param other Other iterator
    /// @pre other iterator must be of the same queue.
    DifferenceType operator-(const ConstIterator& other) const
    {
        return Base::operator-(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator==(const ConstIterator& other) const
    {
        return Base::operator==(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator!=(const ConstIterator& other) const
    {
        return Base::operator!=(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator<(const ConstIterator& other) const
    {
        return Base::operator<(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator<=(const ConstIterator& other) const
    {
        return Base::operator<=(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator>(const ConstIterator& other) const
    {
        return Base::operator>(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator>=(const ConstIterator& other) const
    {
        return Base::operator>=(other);
    }

    /// @brief Iterator dereference operator
    Reference operator*()
    {
        return Base::operator*();
    }

    /// @brief Const version of operator*
    ConstReference operator*() const
    {
        return Base::operator*();
    }

    /// @brief Iterator dereference operator
    Pointer operator->()
    {
        return Base::operator->();
    }

    /// @brief Const version of operator->
    ConstPointer operator->() const
    {
        return Base::operator->();
    }
};

/// @brief Iterator for the elements of StaticQueue.
/// @details May be used to iterate over all the elements without worrying
///          about linearisation of the queue.
template <typename T, std::size_t TSize>
class StaticQueue<T, TSize>::Iterator : public StaticQueue<T, TSize>::Base::Iterator
{
    using Base = typename StaticQueue<T, TSize>::Base::Iterator;
public:

    /// @brief Type of iterator category
    using IteratorCategory = typename Base::IteratorCategory;

    /// @brief Same as IteratorCategory
    using iterator_category = IteratorCategory;

    /// @brief Type of the value referenced by the iterator
    using ValueType = typename Base::ValueType;

    /// @brief Same as ValueType
    using value_type = ValueType;

    /// @brief Type of the difference between two iterators
    using DifferenceType = typename Base::DifferenceType;

    /// @brief Same as DifferenceType
    using difference_type = DifferenceType;

    /// @brief Type of the pointer to the value referenced by the iterator
    using Pointer = typename Base::Pointer;

    /// @brief Same as Pointer
    using pointer = Pointer;

    /// @brief Const pointer type
    using ConstPointer = typename Base::ConstPointer;

    /// @brief Type of the reference to the value referenced by the iterator
    using Reference = typename Base::Reference;

    /// @brief Same as Reference
    using reference = Reference;

    /// @brief Const reference type
    using ConstReference = typename Base::ConstReference;

    /// @brief Queue type
    using QueueType = StaticQueue<T, TSize>;

    /// @brief Linearised iterator
    using LinearisedIterator = typename QueueType::LinearisedIterator;

    /// @brief Const linearised iterator
    using ConstLinearisedIterator = typename QueueType::ConstLinearisedIterator;

    /// @brief Constructor
    /// @param queue Reference to queue
    /// @param iterator Low level array iterator
    Iterator(QueueType& queue, LinearisedIterator iterator)
        : Base(queue, iterator)
    {
    }

    /// @brief Copy constructor is default.
    Iterator(const Iterator&) = default;

    /// @brief Copy assignment operator.
    /// @param other Other iterator
    /// @pre Other iterator must be iterator of the same queue.
    Iterator& operator=(const Iterator& other)
    {
        return static_cast<Iterator&>(Base::operator=(other));
    }

    /// @brief Pre increment operator.
    Iterator& operator++()
    {
        return static_cast<Iterator&>(Base::operator++());
    }

    /// @brief Post increment operator.
    Iterator operator++(int dummyParam)
    {
        auto tmp = Base::operator++(dummyParam);
        return *(static_cast<Iterator*>(&tmp));
    }

    /// @brief Pre decrement operator.
    Iterator& operator--()
    {
        return static_cast<Iterator&>(Base::operator--());
    }

    /// @brief Post-decrement operator.
    Iterator operator--(int dummyParam)
    {
        auto tmp = Base::operator--(dummyParam);
        return *(static_cast<Iterator*>(&tmp));
    }

    /// @brief Operator of adding constant value to the iterator
    /// @param value Value to be added
    Iterator& operator+=(DifferenceType value)
    {
        return static_cast<Iterator&>(Base::operator+=(value));
    }

    /// @brief Operator of substructing constant value from the iterator
    /// @param value Value to be substructed
    Iterator& operator-=(DifferenceType value)
    {
        return static_cast<Iterator&>(Base::operator-=(value));
    }

    /// @brief Operator +
    /// @details Creates new iterator object by adding constant to
    ///          current operator without changing it.
    /// @param value Value to be added
    Iterator operator+(DifferenceType value) const
    {
        auto tmp = Base::operator+(value);
        return *(static_cast<Iterator*>(&tmp));
    }

    /// @brief Operator -
    /// @details Creates new iterator object by substructing constant from
    ///          current operator without changing it.
    /// @param value Value to be substructed
    Iterator operator-(DifferenceType value) const
    {
        auto tmp = Base::operator-(value);
        return *(static_cast<Iterator*>(&tmp));
    }

    /// @brief Computes the distance between two iterators
    /// @param other Other iterator
    /// @pre other iterator must be of the same queue.
    DifferenceType operator-(const Iterator& other) const
    {
        return Base::operator-(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator==(const Iterator& other) const
    {
        return Base::operator==(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator!=(const Iterator& other) const
    {
        return Base::operator!=(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator<(const Iterator& other) const
    {
        return Base::operator<(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator<=(const Iterator& other) const
    {
        return Base::operator<=(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator>(const Iterator& other) const
    {
        return Base::operator>(other);
    }

    /// @brief Iterator comparison operator
    /// @param other Other iterator
    bool operator>=(const Iterator& other) const
    {
        return Base::operator>=(other);
    }

    /// @brief Iterator dereference operator
    Reference operator*()
    {
        return Base::operator*();
    }

    /// @brief Const version of operator*
    ConstReference operator*() const
    {
        return Base::operator*();
    }

    /// @brief Iterator dereference operator
    Pointer operator->()
    {
        return Base::operator->();
    }

    /// @brief Const version of operator->
    ConstPointer operator->() const
    {
        return Base::operator->();
    }

    operator ConstIterator() const
    {
        auto iter = static_cast<ConstLinearisedIterator>(Base::getIterator());
        const auto& queue = static_cast<QueueType&>(Base::getQueue());
        return ConstIterator(queue, iter);
    }
};

/// @endcond

}  // namespace util

}  // namespace comms

COMMS_MSVC_WARNING_POP