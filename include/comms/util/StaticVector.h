//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains comms::util::StaticVector class.

#pragma once

#include "comms/Assert.h"
#include "comms/CompileControl.h"
#include "comms/util/AlignedStorage.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <utility>

COMMS_GNU_WARNING_PUSH

#if COMMS_IS_GCC_12 && defined(NDEBUG)
// Release compilation with gcc-12
// assumes size / capacity of the StaticVectorBase is 0 and generates
// unjustified warnings.
COMMS_GNU_WARNING_DISABLE("-Warray-bounds")
#endif

COMMS_MSVC_WARNING_PUSH
COMMS_MSVC_WARNING_DISABLE(4324) // Disable warning about alignment padding

namespace comms
{

namespace util
{

namespace details
{

template <typename T>
class StaticVectorBase
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using CellType = comms::util::AlignedStorage<sizeof(T), std::alignment_of<T>::value>;

    static_assert(sizeof(CellType) == sizeof(T), "Type T must be padded");

    StaticVectorBase(CellType* dataPtr, std::size_t cap)
      : m_data(dataPtr),
        m_capacity(cap)
    {
    }

    ~StaticVectorBase() noexcept
    {
        clear();
    }

    StaticVectorBase(const StaticVectorBase&) = delete;
    StaticVectorBase& operator=(const StaticVectorBase&) = delete;


    std::size_t size() const
    {
        return m_size;
    }

    std::size_t capacity() const
    {
        return m_capacity;
    }

    bool empty() const
    {
        return (size() == 0);
    }

    void pop_back()
    {
        COMMS_MSVC_WARNING_SUPPRESS(4189) // MSVC claims lastElem not referenced
        auto& lastElem = back();
        lastElem.~T();
        --m_size;
    }

    T& back()
    {
        COMMS_ASSERT(!empty());
        return elem(size() - 1);
    }

    const T& back() const
    {
        COMMS_ASSERT(!empty());
        return elem(size() - 1);
    }

    T& front()
    {
        COMMS_ASSERT(!empty());
        return elem(0);
    }

    const T& front() const
    {
        COMMS_ASSERT(!empty());
        return elem(0);
    }

    template <typename TIter>
    void assign(TIter from, TIter to)
    {
        clear();
        for (auto iter = from; iter != to; ++iter) {
            if (capacity() <= size()) {
                static constexpr bool Not_all_elements_are_copied = false;
                static_cast<void>(Not_all_elements_are_copied);
                COMMS_ASSERT(Not_all_elements_are_copied);
                return;
            }

            new (cellPtr(size())) T(*(reinterpret_cast<const T*>(&*iter)));
            ++m_size;
        }
    }

    void fill(std::size_t count, const T& value)
    {
        clear();
        COMMS_ASSERT(count <= capacity());
        for (auto idx = 0U; idx < count; ++idx) {
            new (cellPtr(idx)) T(value);
        }
        m_size = count;
    }

    void clear() {
        for (auto idx = 0U; idx < size(); ++idx) {
            elem(idx).~T();
        }
        m_size = 0;
    }


    T* begin()
    {
        if (size() == 0U) {
            return nullptr;
        }
        
        return &(elem(0));
    }

    const T* begin() const
    {
        return cbegin();
    }

    const T* cbegin() const
    {
        if (size() == 0U) {
            return nullptr;
        }

        return &(elem(0));
    }

    T* end()
    {
        return begin() + size();
    }

    const T* end() const
    {
        return cend();
    }

    const T* cend() const
    {
        return cbegin() + size();
    }

    T& at(std::size_t pos)
    {
        COMMS_ASSERT(pos < size());
        return elem(pos);
    }

    const T& at(std::size_t pos) const
    {
        COMMS_ASSERT(pos < size());
        return elem(pos);
    }

    T& operator[](std::size_t pos)
    {
        return elem(pos);
    }

    const T& operator[](std::size_t pos) const
    {
        return elem(pos);
    }

    T* data()
    {
        if (size() == 0U) {
            return nullptr;
        }

        return &(elem(0));
    }

    const T* data() const
    {
        if (size() == 0U) {
            return nullptr;
        }
                
        return &(elem(0));
    }

    template <typename U>
    T* insert(const T* pos, U&& value)
    {
        COMMS_ASSERT(pos <= end());
        COMMS_ASSERT(size() < capacity());
        if (end() <= pos) {
            push_back(std::forward<U>(value));
            return &(back());
        }

        COMMS_ASSERT(!empty());
        push_back(std::move(back()));
        auto* insertIter = begin() + std::distance(cbegin(), pos);
        std::move_backward(insertIter, end() - 2, end() - 1);
        *insertIter = std::forward<U>(value);
        return insertIter;
    }

    T* insert(const T* pos, std::size_t count, const T& value)
    {
        COMMS_ASSERT(pos <= end());
        COMMS_ASSERT((size() + count) <= capacity());
        auto dist = std::distance(cbegin(), pos);
        COMMS_ASSERT((0 <= dist) && static_cast<std::size_t>(dist) < size());
        auto* posIter = begin() + dist;
        if (end() <= posIter) {
            while (0 < count) {
                push_back(value);
                --count;
            }
            return posIter;
        }

        COMMS_ASSERT(!empty());
        auto tailCount = static_cast<std::size_t>(std::distance(posIter, end()));
        if (count <= tailCount) {
            auto pushBegIter = end() - count;
            auto pushEndIter = end();
            for (auto iter = pushBegIter; iter != pushEndIter; ++iter) {
                push_back(std::move(*iter));
            }

            auto moveBegIter = posIter;
            auto moveEndIter = moveBegIter + (tailCount - count);
            COMMS_ASSERT(moveEndIter < pushEndIter);

            COMMS_GNU_WARNING_PUSH
#if COMMS_IS_GCC_12 && defined(NDEBUG)
            // Release compilation with gcc-12
            // gives a warning here, while any debug build works fine.
            COMMS_GNU_WARNING_DISABLE("-Wstringop-overflow")
#endif                  
            std::move_backward(moveBegIter, moveEndIter, pushEndIter);
            COMMS_GNU_WARNING_POP           

            auto* assignBegIter = posIter;
            auto* assignEndIter = assignBegIter + count;
            for (auto iter = assignBegIter; iter != assignEndIter; ++iter) {
                *iter = value;
            }
            return posIter;
        }

        auto pushValueCount = count - tailCount;
        for (auto idx = 0U; idx < pushValueCount; ++idx) {
            push_back(value);
        }

        auto* pushBegIter = posIter;
        auto* pushEndIter = pushBegIter + tailCount;
        for (auto iter = pushBegIter; iter != pushEndIter; ++iter) {
            push_back(std::move(*iter));
        }

        auto assignBegIter = posIter;
        auto assignEndIter = assignBegIter + tailCount;
        for (auto iter = assignBegIter; iter != assignEndIter; ++iter) {
            *iter = value;
        }
        return posIter;
    }

    template <typename TIter>
    T* insert(const T* pos, TIter from, TIter to)
    {
        using Tag = typename std::iterator_traits<TIter>::iterator_category;
        return insert_internal(pos, from, to, Tag());
    }

    template <typename... TArgs>
    T* emplace(const T* iter, TArgs&&... args)
    {
        auto* insertIter = begin() + std::distance(cbegin(), iter);
        if (iter == cend()) {
            emplace_back(std::forward<TArgs>(args)...);
            return insertIter;
        }

        COMMS_ASSERT(!empty());
        push_back(std::move(back()));
        std::move_backward(insertIter, end() - 2, end() - 1);
        insertIter->~T();
        new (insertIter) T(std::forward<TArgs>(args)...);
        return insertIter;
    }

    T* erase(const T* from, const T* to)
    {
        COMMS_ASSERT(from <= cend());
        COMMS_ASSERT(to <= cend());
        COMMS_ASSERT(from <= to);

        auto tailCount = static_cast<std::size_t>(std::distance(to, cend()));
        auto eraseCount = static_cast<std::size_t>(std::distance(from, to));

        auto* moveSrc = begin() + std::distance(cbegin(), to);
        auto* moveDest = begin() + std::distance(cbegin(), from);
        std::move(moveSrc, end(), moveDest);

        auto* eraseFrom = moveDest + tailCount;
        auto* eraseTo = end();
        COMMS_ASSERT(eraseFrom <= end());
        COMMS_ASSERT(eraseCount <= size());
        COMMS_ASSERT(static_cast<std::size_t>(std::distance(eraseFrom, eraseTo)) == eraseCount);
        for (auto iter = eraseFrom; iter != eraseTo; ++iter) {
            iter->~T();
        }
        m_size -= eraseCount;
        return moveDest;
    }

    template <typename U>
    void push_back(U&& value)
    {
        COMMS_ASSERT(size() < capacity());
        new (cellPtr(size())) T(std::forward<U>(value));
        ++m_size;
    }

    template <typename... TArgs>
    void emplace_back(TArgs&&... args)
    {
        COMMS_ASSERT(size() < capacity());
        new (cellPtr(size())) T(std::forward<TArgs>(args)...);
        ++m_size;
    }

    void resize(std::size_t count, const T& value)
    {
        if (count < size()) {
            erase(begin() + count, end());
            COMMS_ASSERT(count == size());
            return;
        }

        while (size() < count) {
            push_back(value);
        }
    }

    void swap(StaticVectorBase<T>& other)
    {
        auto swapSize = std::min(other.size(), size());
        for (auto idx = 0U; idx < swapSize; ++idx) {
            std::swap(this->operator[](idx), other[idx]);
        }

        auto otherSize = other.size();
        auto thisSize = size();

        if (otherSize == thisSize) {
            return;
        }

        if (otherSize < thisSize) {
            auto limit = std::min(thisSize, other.capacity());
            for (auto idx = swapSize; idx < limit; ++idx) {
                new (other.cellPtr(idx)) T(std::move(elem(idx)));
            }

            other.m_size = thisSize;
            erase(begin() + otherSize, end());
            return;
        }

        auto limit = std::min(otherSize, capacity());
        for (auto idx = swapSize; idx < limit; ++idx) {
            new (cellPtr(idx)) T(std::move(other.elem(idx)));
        }
        m_size = otherSize;
        other.erase(other.begin() + thisSize, other.end());
    }

private:
    CellType& cell(std::size_t idx)
    {
        COMMS_ASSERT(idx < capacity());
        return m_data[idx];
    }

    const CellType& cell(std::size_t idx) const
    {
        COMMS_ASSERT(idx < capacity());
        return m_data[idx];
    }

    CellType* cellPtr(std::size_t idx)
    {
        COMMS_ASSERT(idx < capacity());
        return &m_data[idx];
    }

    T& elem(std::size_t idx)
    {
        return reinterpret_cast<T&>(cell(idx));
    }

    const T& elem(std::size_t idx) const
    {
        return reinterpret_cast<const T&>(cell(idx));
    }

    template <typename TIter>
    T* insert_random_access(const T* pos, TIter from, TIter to)
    {
        COMMS_ASSERT(pos <= end());
        auto* posIter = begin() + std::distance(cbegin(), pos);
        if (end() <= posIter) {
            for (; from != to; ++from) {
                push_back(*from);
            }

            return posIter;
        }

        auto count = static_cast<std::size_t>(std::distance(from, to));
        COMMS_ASSERT(!empty());
        auto tailCount = static_cast<std::size_t>(std::distance(posIter, end()));
        if (count <= tailCount) {
            auto pushBegIter = end() - count;
            auto pushEndIter = end();
            for (auto iter = pushBegIter; iter != pushEndIter; ++iter) {
                push_back(std::move(*iter));
            }

            auto moveBegIter = posIter;
            auto moveEndIter = moveBegIter + (tailCount - count);
            COMMS_ASSERT(moveEndIter < pushEndIter);
            std::move_backward(moveBegIter, moveEndIter, pushEndIter);

            auto* assignBegIter = posIter;
            auto* assignEndIter = assignBegIter + count;
            for (auto iter = assignBegIter; iter != assignEndIter; ++iter) {
                *iter = *from;
                ++from;
            }
            return posIter;
        }

        auto pushValueCount = count - tailCount;
        auto pushInsertedBegIter = to - pushValueCount;
        for (auto idx = 0U; idx < pushValueCount; ++idx) {
            push_back(*pushInsertedBegIter);
            ++pushInsertedBegIter;
        }

        auto* pushBegIter = posIter;
        auto* pushEndIter = pushBegIter + tailCount;
        for (auto iter = pushBegIter; iter != pushEndIter; ++iter) {
            push_back(std::move(*iter));
        }

        auto assignBegIter = posIter;
        auto assignEndIter = assignBegIter + tailCount;
        for (auto iter = assignBegIter; iter != assignEndIter; ++iter) {
            *iter = *from;
            ++from;
        }

        return posIter;
    }

    template <typename TIter>
    T* insert_input(const T* pos, TIter from, TIter to)
    {
        T* ret = nullptr;
        for (; from != to; ++from) {
            if (ret == nullptr) {
                ret = begin() + std::distance(cbegin(), pos);
            }
            insert(pos, *from);
            ++pos;
        }
        return ret;
    }

    template <typename TIter>
    T* insert_internal(const T* pos, TIter from, TIter to, std::random_access_iterator_tag)
    {
        return insert_random_access(pos, from, to);
    }

    template <typename TIter>
    T* insert_internal(const T* pos, TIter from, TIter to, std::input_iterator_tag)
    {
        return insert_input(pos, from, to);
    }


    CellType* m_data = nullptr;
    std::size_t m_capacity = 0;
    std::size_t m_size = 0;
};

template <typename T, std::size_t TSize>
struct StaticVectorStorageBase
{
    using ElementType = comms::util::AlignedStorage<sizeof(T), std::alignment_of<T>::value>; 
    using StorageType = std::array<ElementType, TSize>;
    alignas(alignof(T)) StorageType m_data;
};

template <typename T, std::size_t TSize>
class StaticVectorGeneric :
    public StaticVectorStorageBase<T, TSize>,
    public StaticVectorBase<T>
{
    using StorageBase = StaticVectorStorageBase<T, TSize>;
    using Base = StaticVectorBase<T>;

public:
    using value_type = typename Base::value_type;
    using size_type = typename Base::size_type;
    using difference_type = typename StorageBase::StorageType::difference_type;
    using reference = typename Base::reference;
    using const_reference = typename Base::const_reference;
    using pointer = typename Base::pointer;
    using const_pointer = typename Base::const_pointer;
    using iterator = typename Base::iterator;
    using const_iterator = typename Base::const_iterator;
    using reverse_iterator = typename Base::reverse_iterator;
    using const_reverse_iterator = typename Base::const_reverse_iterator;

    StaticVectorGeneric()
      : Base(StorageBase::m_data.data(), StorageBase::m_data.size())
    {
    }

    StaticVectorGeneric(size_type count, const T& value)
      : Base(StorageBase::m_data.data(), StorageBase::m_data.size())
    {
        assign(count, value);
    }

    explicit StaticVectorGeneric(size_type count)
      : Base(StorageBase::m_data.data(), StorageBase::m_data.size())
    {
        COMMS_ASSERT(count < Base::capacity());
        while (0 < count) {
            Base::emplace_back();
            --count;
        }
    }

    template <typename TIter>
    StaticVectorGeneric(TIter from, TIter to)
      : Base(StorageBase::m_data.data(), StorageBase::m_data.size())
    {
        assign(from, to);
    }

    template <std::size_t TOtherSize>
    StaticVectorGeneric(const StaticVectorGeneric<T, TOtherSize>& other)
      : Base(StorageBase::m_data.data(), StorageBase::m_data.size())
    {
        assign(other.begin(), other.end());
    }

    StaticVectorGeneric(const StaticVectorGeneric& other)
      : Base(StorageBase::m_data.data(), StorageBase::m_data.size())
    {
        assign(other.begin(), other.end());
    }

    StaticVectorGeneric(std::initializer_list<value_type> init)
      : Base(StorageBase::m_data.data(), StorageBase::m_data.size())
    {
        assign(init.begin(), init.end());
    }

    ~StaticVectorGeneric() noexcept = default;

    StaticVectorGeneric& operator=(const StaticVectorGeneric& other)
    {
        if (&other == this) {
            return *this;
        }

        assign(other.begin(), other.end());
        return *this;
    }

    template <std::size_t TOtherSize>
    StaticVectorGeneric& operator=(const StaticVectorGeneric<T, TOtherSize>& other)
    {
        assign(other.cbegin(), other.cend());
        return *this;
    }

    StaticVectorGeneric& operator=(std::initializer_list<value_type> init)
    {
        assign(init);
        return *this;
    }

    void assign(size_type count, const T& value)
    {
        COMMS_ASSERT(count <= TSize);
        Base::fill(count, value);
    }

    template <typename TIter>
    void assign(TIter from, TIter to)
    {
        Base::assign(from, to);
    }

    void assign(std::initializer_list<value_type> init)
    {
        assign(init.begin(), init.end());
    }

    void reserve(size_type new_cap)
    {
        static_cast<void>(new_cap);
        COMMS_ASSERT(new_cap <= Base::capacity());
    }
};

template <typename TOrig, typename TCast, std::size_t TSize>
class StaticVectorCasted : public StaticVectorGeneric<TCast, TSize>
{
    using Base = StaticVectorGeneric<TCast, TSize>;
    static_assert(sizeof(TOrig) == sizeof(TCast), "The sizes are not equal");

public:
    using value_type = TOrig;
    using size_type = typename Base::size_type;
    using difference_type = typename Base::difference_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;

    StaticVectorCasted() = default;

    StaticVectorCasted(size_type count, const_reference& value)
      : Base(count, *(reinterpret_cast<typename Base::const_pointer>(&value)))
    {
    }

    explicit StaticVectorCasted(size_type count)
      : Base(count)
    {
    }

    template <typename TIter>
    StaticVectorCasted(TIter from, TIter to)
      : Base(from, to)
    {
    }

    template <std::size_t TOtherSize>
    StaticVectorCasted(const StaticVectorCasted<TOrig, TCast, TOtherSize>& other)
      : Base(other)
    {
    }

    StaticVectorCasted(const StaticVectorCasted& other)
      : Base(other)
    {
    }

    StaticVectorCasted(std::initializer_list<value_type> init)
      : Base(init.begin(), init.end())
    {
    }

    ~StaticVectorCasted() noexcept = default;

    StaticVectorCasted& operator=(const StaticVectorCasted&) = default;

    template <std::size_t TOtherSize>
    StaticVectorCasted& operator=(const StaticVectorCasted<TOrig, TCast, TOtherSize>& other)
    {
        Base::operator=(other);
        return *this;
    }

    StaticVectorCasted& operator=(std::initializer_list<value_type> init)
    {
        Base::operator=(init);
        return *this;
    }

    void assign(size_type count, const_reference& value)
    {
        Base::assign(count, value);
    }

    template <typename TIter>
    void assign(TIter from, TIter to)
    {
        Base::assign(from, to);
    }

    void assign(std::initializer_list<value_type> init)
    {
        assign(init.begin(), init.end());
    }

    reference at(size_type pos)
    {
        return *(reinterpret_cast<pointer>(&(Base::at(pos))));
    }

    const_reference at(size_type pos) const
    {
        return *(reinterpret_cast<const_pointer>(&(Base::at(pos))));
    }

    reference operator[](size_type pos)
    {
        return *(reinterpret_cast<pointer>(&(Base::operator[](pos))));
    }

    const_reference operator[](size_type pos) const
    {
        return *(reinterpret_cast<const_pointer>(&(Base::operator[](pos))));
    }

    reference front()
    {
        return *(reinterpret_cast<pointer>(&(Base::front())));
    }

    const_reference front() const
    {
        return *(reinterpret_cast<const_pointer>(&(Base::front())));
    }

    reference back()
    {
        return *(reinterpret_cast<pointer>(&(Base::back())));
    }

    const_reference back() const
    {
        return *(reinterpret_cast<const_pointer>(&(Base::back())));
    }

    pointer data()
    {
        return reinterpret_cast<pointer>(Base::data());
    }

    const_pointer data() const
    {
        return reinterpret_cast<const_pointer>(Base::data());
    }

    iterator begin()
    {
        return reinterpret_cast<iterator>(Base::begin());
    }

    const_iterator begin() const
    {
        return cbegin();
    }

    const_iterator cbegin() const
    {
        return reinterpret_cast<const_iterator>(Base::cbegin());
    }

    iterator end()
    {
        return reinterpret_cast<iterator>(Base::end());
    }

    const_iterator end() const
    {
        return cend();
    }

    const_iterator cend() const
    {
        return reinterpret_cast<const_iterator>(Base::cend());
    }

    iterator insert(const_iterator iter, const_reference value)
    {
        return
            reinterpret_cast<iterator>(
                Base::insert(
                    reinterpret_cast<typename Base::const_iterator>(iter),
                    *(reinterpret_cast<typename Base::const_pointer>(&value))));
    }

    iterator insert(const_iterator iter, TCast&& value)
    {
        return
            reinterpret_cast<iterator>(
                Base::insert(
                    reinterpret_cast<typename Base::const_iterator>(iter),
                    std::move(*(reinterpret_cast<typename Base::pointer>(&value)))));
    }

    iterator insert(const_iterator iter, size_type count, const_reference value)
    {
        return
            reinterpret_cast<iterator>(
                Base::insert(
                    reinterpret_cast<typename Base::const_iterator>(iter),
                    count,
                    *(reinterpret_cast<typename Base::const_pointer>(&value))));
    }

    template <typename TIter>
    iterator insert(const_iterator iter, TIter from, TIter to)
    {
        return
            reinterpret_cast<iterator>(
                Base::insert(
                    reinterpret_cast<typename Base::const_iterator>(iter),
                    from,
                    to));
    }

    iterator insert(const_iterator iter, std::initializer_list<value_type> init)
    {
        return
            reinterpret_cast<iterator>(
                Base::insert(
                    reinterpret_cast<typename Base::const_iterator>(iter),
                    init.begin(),
                    init.end()));
    }

    template <typename... TArgs>
    iterator emplace(const_iterator iter, TArgs&&... args)
    {
        return
            reinterpret_cast<iterator>(
                Base::emplace(
                    reinterpret_cast<typename Base::const_iterator>(iter),
                    std::forward<TArgs>(args)...));
    }

    iterator erase(const_iterator iter)
    {
        return erase(iter, iter + 1);
    }

    /// @brief Erases elements.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/erase">Reference</a>
    iterator erase(const_iterator from, const_iterator to)
    {
        return
            reinterpret_cast<iterator>(
                Base::erase(
                    reinterpret_cast<typename Base::const_iterator>(from),
                    reinterpret_cast<typename Base::const_iterator>(to)));
    }

    void push_back(const_reference value)
    {
        Base::push_back(*(reinterpret_cast<typename Base::const_pointer>(&value)));
    }

    void push_back(TCast&& value)
    {
        Base::push_back(std::move(*(reinterpret_cast<TCast*>(&value))));
    }
};

template <bool TSignedIntegral>
struct StaticVectorBaseSignedIntegral;

template <>
struct StaticVectorBaseSignedIntegral<true>
{
    template <typename T, std::size_t TSize>
    using Type = StaticVectorCasted<T, typename std::make_unsigned<T>::type, TSize>;
};

template <>
struct StaticVectorBaseSignedIntegral<false>
{
    template <typename T, std::size_t TSize>
    using Type = StaticVectorGeneric<T, TSize>;
};

template <typename T, std::size_t TSize>
using ChooseStaticVectorBase =
    typename StaticVectorBaseSignedIntegral<std::is_integral<T>::value && std::is_signed<T>::value>::template Type<T, TSize>;

}  // namespace details

/// @brief Replacement to <a href="http://en.cppreference.com/w/cpp/container/vector">std::vector</a>
///     when no dynamic memory allocation is allowed.
/// @details Uses <a href="http://en.cppreference.com/w/cpp/container/array">std::array</a>
///     in its private members to store the data. Provides
///     almost the same interface as
///     <a href="http://en.cppreference.com/w/cpp/container/vector">std::vector</a>.
/// @tparam T Type of the stored elements.
/// @tparam TSize Maximum number of elements that StaticVector can store.
/// @headerfile "comms/util/StaticVector.h"
template <typename T, std::size_t TSize>
class StaticVector : public details::ChooseStaticVectorBase<T, TSize>
{
    using Base = details::ChooseStaticVectorBase<T, TSize>;
    using ElementType = typename Base::ElementType;

    static_assert(sizeof(T) == sizeof(ElementType),
        "Sizes are not equal as expected.");

    template <typename U, std::size_t TOtherSize>
    friend class StaticVector;

public:
    /// @brief Type of single element.
    using value_type = typename Base::value_type;

    /// @brief Type used for size information
    using size_type = typename Base::size_type;

    /// @brief Type used in pointer arithmetics
    using difference_type = typename Base::StorageType::difference_type;

    /// @brief Reference to single element
    using reference = typename Base::reference;

    /// @brief Const reference to single element
    using const_reference = typename Base::const_reference;

    /// @brief Pointer to single element
    using pointer = typename Base::pointer;

    /// @brief Const pointer to single element
    using const_pointer = typename Base::const_pointer;

    /// @brief Type of the iterator.
    using iterator = typename Base::iterator;

    /// @brief Type of the const iterator
    using const_iterator = typename Base::const_iterator;

    /// @brief Type of the reverse iterator
    using reverse_iterator = typename Base::reverse_iterator;

    /// @brief Type of the const reverse iterator
    using const_reverse_iterator = typename Base::const_reverse_iterator;

    /// @brief Default constructor.
    StaticVector() = default;

    /// @brief Constructor
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/vector">Reference</a>
    StaticVector(size_type count, const T& value)
      : Base(count, value)
    {
    }

    /// @brief Constructor
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/vector">Reference</a>
    explicit StaticVector(size_type count)
      : Base(count)
    {
    }

    /// @brief Constructor
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/vector">Reference</a>
    template <typename TIter>
    StaticVector(TIter from, TIter to)
      : Base(from, to)
    {
    }

    /// @brief Copy constructor
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/vector">Reference</a>
    template <std::size_t TOtherSize>
    StaticVector(const StaticVector<T, TOtherSize>& other)
      : Base(other)
    {
    }

    /// @brief Copy constructor
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/vector">Reference</a>
    StaticVector(const StaticVector& other)
      : Base(other)
    {
    }

    /// @brief Constructor
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/vector">Reference</a>
    StaticVector(std::initializer_list<value_type> init)
      : Base(init)
    {
    }

    /// @brief Destructor
    ~StaticVector() noexcept = default;

    /// @brief Copy assignement
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator%3D">Reference</a>
    StaticVector& operator=(const StaticVector&) = default;

    /// @brief Copy assignement
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator%3D">Reference</a>
    template <std::size_t TOtherSize>
    StaticVector& operator=(const StaticVector<T, TOtherSize>& other)
    {
        Base::operator=(other);
        return *this;
    }

    /// @brief Copy assignement
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator%3D">Reference</a>
    StaticVector& operator=(std::initializer_list<value_type> init)
    {
        Base::operator=(init);
        return *this;
    }

    /// @brief Assigns values to the container.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/assign">Reference</a>
    void assign(size_type count, const T& value)
    {
        Base::assign(count, value);
    }

    /// @brief Assigns values to the container.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/assign">Reference</a>
    template <typename TIter>
    void assign(TIter from, TIter to)
    {
        Base::assign(from, to);
    }

    /// @brief Assigns values to the container.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/assign">Reference</a>
    void assign(std::initializer_list<value_type> init)
    {
        assign(init.begin(), init.end());
    }

    /// @brief Access specified element with bounds checking.
    /// @details The bounds check is performed with COMMS_ASSERT() macro, which means
    ///     it is performed only in DEBUG mode compilation. In case NDEBUG
    ///     symbol is defined (RELEASE mode compilation), this call is equivalent
    ///     to operator[]().
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/at">Reference</a>
    reference at(size_type pos)
    {
        return Base::at(pos);
    }
    
    /// @brief Access specified element with bounds checking.
    /// @details The bounds check is performed with COMMS_ASSERT() macro, which means
    ///     it is performed only in DEBUG mode compilation. In case NDEBUG
    ///     symbol is defined (RELEASE mode compilation), this call is equivalent
    ///     to operator[]().
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/at">Reference</a>
    const_reference at(size_type pos) const
    {
        return Base::at(pos);
    }

    /// @brief Access specified element without bounds checking.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator_at">Reference</a>
    reference operator[](size_type pos)
    {
        return Base::operator[](pos);
    }

    /// @brief Access specified element without bounds checking.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator_at">Reference</a>
    const_reference operator[](size_type pos) const
    {
        return Base::operator[](pos);
    }

    /// @brief Access the first element.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/front">Reference</a>
    /// @pre The vector is not empty.
    reference front()
    {
        return Base::front();
    }

    /// @brief Access the first element.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/front">Reference</a>
    /// @pre The vector is not empty.
    const_reference front() const
    {
        return Base::front();
    }

    /// @brief Access the last element.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/back">Reference</a>
    /// @pre The vector is not empty.
    reference back()
    {
        return Base::back();
    }

    /// @brief Access the last element.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/back">Reference</a>
    /// @pre The vector is not empty.
    const_reference back() const
    {
        return Base::back();
    }

    /// @brief Direct access to the underlying array.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/data">Reference</a>
    pointer data()
    {
        return Base::data();
    }

    /// @brief Direct access to the underlying array.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/data">Reference</a>
    const_pointer data() const
    {
        return Base::data();
    }

    /// @brief Returns an iterator to the beginning.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/begin">Reference</a>
    iterator begin()
    {
        return Base::begin();
    }

    /// @brief Returns an iterator to the beginning.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/begin">Reference</a>
    const_iterator begin() const
    {
        return cbegin();
    }

    /// @brief Returns an iterator to the beginning.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/begin">Reference</a>
    const_iterator cbegin() const
    {
        return Base::cbegin();
    }

    /// @brief Returns an iterator to the end.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/end">Reference</a>
    iterator end()
    {
        return Base::end();
    }

    /// @brief Returns an iterator to the end.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/end">Reference</a>
    const_iterator end() const
    {
        return cend();
    }

    /// @brief Returns an iterator to the end.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/end">Reference</a>
    const_iterator cend() const
    {
        return Base::cend();
    }

    /// @brief Returns a reverse iterator to the beginning.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/rbegin">Reference</a>
    reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    /// @brief Returns a reverse iterator to the beginning.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/rbegin">Reference</a>
    const_reverse_iterator rbegin() const
    {
        return crbegin();
    }

    /// @brief Returns a reverse iterator to the beginning.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/rbegin">Reference</a>
    const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(cend());
    }

    /// @brief Returns a reverse iterator to the end.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/rend">Reference</a>
    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    /// @brief Returns a reverse iterator to the end.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/rend">Reference</a>
    const_reverse_iterator rend() const
    {
        return crend();
    }

    /// @brief Returns a reverse iterator to the end.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/rend">Reference</a>
    const_reverse_iterator crend() const
    {
        return const_reverse_iterator(cbegin());
    }

    /// @brief Checks whether the container is empty.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/empty">Reference</a>
    bool empty() const
    {
        return Base::empty();
    }

    /// @brief Returns the number of elements.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/size">Reference</a>
    size_type size() const
    {
        return Base::size();
    }

    /// @brief Returns the maximum possible number of elements.
    /// @details Same as capacity().
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/max_size">Reference</a>
    /// @return TSize provided as template argument.
    size_type max_size() const
    {
        return capacity();
    }

    /// @brief Reserves storage.
    /// @details Does nothing.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/reserve">Reference</a>
    void reserve(size_type new_cap)
    {
        return Base::reserve(new_cap);
    }

    /// @brief Returns the number of elements that can be held in currently allocated storage.
    /// @details Same as max_size().
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/capacity">Reference</a>
    /// @return TSize provided as template argument.
    size_type capacity() const
    {
        return Base::capacity();
    }

    /// @brief Reduces memory usage by freeing unused memory.
    /// @details Does nothing.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/shrink_to_fit">Reference</a>
    void shrink_to_fit()
    {
    }

    /// @brief Clears the contents.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/clear">Reference</a>
    void clear()
    {
        Base::clear();
    }

    /// @brief Inserts elements.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/insert">Reference</a>
    iterator insert(const_iterator iter, const T& value)
    {
        return Base::insert(iter, value);
    }

    /// @brief Inserts elements.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/insert">Reference</a>
    iterator insert(const_iterator iter, T&& value)
    {
        return Base::insert(iter, std::move(value));
    }

    /// @brief Inserts elements.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/insert">Reference</a>
    iterator insert(const_iterator iter, size_type count, const T& value)
    {
        return Base::insert(iter, count, value);
    }

    /// @brief Inserts elements.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/insert">Reference</a>
    template <typename TIter>
    iterator insert(const_iterator iter, TIter from, TIter to)
    {
        return Base::insert(iter, from, to);
    }

    /// @brief Inserts elements.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/insert">Reference</a>
    iterator insert(const_iterator iter, std::initializer_list<value_type> init)
    {
        return Base::insert(iter, init.begin(), init.end());
    }

    /// @brief Constructs elements in place.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/emplace">Reference</a>
    template <typename... TArgs>
    iterator emplace(const_iterator iter, TArgs&&... args)
    {
        return Base::emplace(iter, std::forward<TArgs>(args)...);
    }

    /// @brief Erases elements.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/erase">Reference</a>
    iterator erase(const_iterator iter)
    {
        return erase(iter, iter + 1);
    }

    /// @brief Erases elements.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/erase">Reference</a>
    iterator erase(const_iterator from, const_iterator to)
    {
        return Base::erase(from, to);
    }

    /// @brief Adds an element to the end.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/push_back">Reference</a>
    /// @pre The vector mustn't be full.
    void push_back(const T& value)
    {
        Base::push_back(value);
    }

    /// @brief Adds an element to the end.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/push_back">Reference</a>
    /// @pre The vector mustn't be full.
    void push_back(T&& value)
    {
        Base::push_back(std::move(value));
    }

    /// @brief Constructs an element in place at the end.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/emplace_back">Reference</a>
    /// @pre The vector mustn't be full.
    template <typename... TArgs>
    void emplace_back(TArgs&&... args)
    {
        Base::emplace_back(std::forward<TArgs>(args)...);
    }

    /// @brief Removes the last element.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/pop_back">Reference</a>
    /// @pre The vector mustn't be empty.
    void pop_back()
    {
        Base::pop_back();
    }

    /// @brief Changes the number of elements stored.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/resize">Reference</a>
    /// @pre New size mustn't exceed max_size().
    void resize(size_type count)
    {
        resize(count, T());
    }

    /// @brief Changes the number of elements stored.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/resize">Reference</a>
    /// @pre New size mustn't exceed max_size().
    void resize(size_type count, const value_type& value)
    {
        Base::resize(count, value);
    }

    /// @brief Swaps the contents.
    /// @see <a href="http://en.cppreference.com/w/cpp/container/vector/resize">Reference</a>
    /// @pre New size mustn't exceed max_size().
    template <std::size_t TOtherSize>
    void swap(StaticVector<T, TOtherSize>& other)
    {
        Base::swap(other);
    }
};

// Template specialization for zero sized vectors
template <typename T>
class StaticVector<T, 0U>
{
    using StorageType = std::array<T, 0U>;

    template <typename U, std::size_t TOtherSize>
    friend class StaticVector;

public:
    using value_type = typename StorageType::value_type;
    using size_type = typename StorageType::size_type;
    using difference_type = typename StorageType::difference_type;
    using reference = typename StorageType::reference;
    using const_reference = typename StorageType::const_reference;
    using pointer = typename StorageType::pointer;
    using const_pointer = typename StorageType::const_pointer;
    using iterator = typename StorageType::iterator;
    using const_iterator = typename StorageType::const_iterator;
    using reverse_iterator = typename StorageType::reverse_iterator;
    using const_reverse_iterator = typename StorageType::const_reverse_iterator;

    StaticVector() = default;

    StaticVector(size_type count, const T& value)
    {
        static_cast<void>(value);
        if (count == 0U) {
            return;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);
    }

    explicit StaticVector(size_type count)
    {
        if (count == 0U) {
            return;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
    }

    template <typename TIter>
    StaticVector(TIter from, TIter to)
    {
        if (from == to) {
            return;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
    }

    template <std::size_t TOtherSize>
    StaticVector(const StaticVector<T, TOtherSize>& other)
    {
        static_cast<void>(other);
        if (TOtherSize == 0U) {
            return;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
    }

    StaticVector(const StaticVector& other)
    {
        static_cast<void>(other);
    }

    StaticVector(std::initializer_list<value_type> init)
    {
        if (std::begin(init) == std::end(init)) {
            return;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
    }

    ~StaticVector() noexcept = default;

    StaticVector& operator=(const StaticVector&) = default;

    template <std::size_t TOtherSize>
    StaticVector& operator=(const StaticVector<T, TOtherSize>& other)
    {
        static_cast<void>(other);
        if (TOtherSize == 0U) {
            return *this;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);
        return *this;
    }

    StaticVector& operator=(std::initializer_list<value_type> init)
    {
        if (std::begin(init) == std::end(init)) {
            return *this;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);
        return *this;
    }

    void assign(size_type count, const T& value)
    {
        static_cast<void>(value);
        if (count == 0U) {
            return;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);
    }

    template <typename TIter>
    void assign(TIter from, TIter to)
    {
        if (from == to) {
            return;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);
    }

    void assign(std::initializer_list<value_type> init)
    {
        if (std::begin(init) == std::end(init)) {
            return;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);
    }

    reference at(size_type pos)
    {
        static_cast<void>(pos);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
        return m_data[pos];
    }

    const_reference at(size_type pos) const
    {
        static_cast<void>(pos);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
        return m_data[pos];
    }

    reference operator[](size_type pos)
    {
        static_cast<void>(pos);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
        return m_data[pos];
    }

    const_reference operator[](size_type pos) const
    {
        static_cast<void>(pos);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
        return m_data[pos];
    }

    reference front()
    {
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
        return m_data.front();
    }

    const_reference front() const
    {
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
        return m_data.front();
    }

    reference back()
    {
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
        return m_data.back();
    }

    const_reference back() const
    {
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
        return m_data.back();
    }

    pointer data()
    {
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
        return m_data.data();
    }

    const_pointer data() const
    {
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);        
        return m_data.data();
    }

    iterator begin()
    {
        return m_data.begin();
    }

    const_iterator begin() const
    {
        return m_data.begin();
    }

    const_iterator cbegin() const
    {
        return m_data.cbegin();
    }

    iterator end()
    {
        return m_data.end();
    }

    const_iterator end() const
    {
        return m_data.end();
    }

    const_iterator cend() const
    {
        return m_data.cend();
    }

    reverse_iterator rbegin()
    {
        return m_data.rbegin();
    }

    const_reverse_iterator rbegin() const
    {
        return m_data.rbegin();
    }

    const_reverse_iterator crbegin() const
    {
        return m_data.crbegin();
    }

    reverse_iterator rend()
    {
        return m_data.rend();
    }

    const_reverse_iterator rend() const
    {
        return m_data.rend();
    }

    const_reverse_iterator crend() const
    {
        return m_data.crend();
    }

    bool empty() const
    {
        return m_data.empty();
    }

    size_type size() const
    {
        return m_data.size();
    }

    size_type max_size() const
    {
        return m_data.max_size();
    }

    void reserve(size_type new_cap)
    {
        static_cast<void>(new_cap);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);           
    }

    size_type capacity() const
    {
        return max_size();
    }

    void shrink_to_fit()
    {
    }

    void clear()
    {
    }

    iterator insert(const_iterator iter, const T& value)
    {
        static_cast<void>(iter);
        static_cast<void>(value);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);           
        return m_data.end();
    }

    iterator insert(const_iterator iter, T&& value)
    {
        static_cast<void>(iter);
        static_cast<void>(value);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);           
        return m_data.end();
    }

    iterator insert(const_iterator iter, size_type count, const T& value)
    {
        static_cast<void>(iter);
        static_cast<void>(count);
        static_cast<void>(value);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);           
        return m_data.end();
    }

    template <typename TIter>
    iterator insert(const_iterator iter, TIter from, TIter to)
    {
        static_cast<void>(iter);
        static_cast<void>(from);
        static_cast<void>(to);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);           
        return m_data.end();
    }

    iterator insert(const_iterator iter, std::initializer_list<value_type> init)
    {
        static_cast<void>(iter);
        static_cast<void>(init);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);           
        return m_data.end();
    }

    template <typename... TArgs>
    iterator emplace(const_iterator iter, TArgs&&...)
    {
        static_cast<void>(iter);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);           
        return m_data.end();
    }

    iterator erase(const_iterator iter)
    {
        static_cast<void>(iter);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);           
        return m_data.end();
    }

    iterator erase(const_iterator from, const_iterator to)
    {
        if (from != to) {
            static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
            static_cast<void>(Must_not_be_called_for_zero_sized_vector);
            COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);           
        }
        return m_data.end();        
    }

    void push_back(const T& value)
    {
        static_cast<void>(value);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);          
    }

    void push_back(T&& value)
    {
        static_cast<void>(value);
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);  
    }

    template <typename... TArgs>
    void emplace_back(TArgs&&...)
    {
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector); 
    }

    void pop_back()
    {
        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector); 
    }

    void resize(size_type count)
    {
        if (count == 0U) {
            return;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);         
    }

    void resize(size_type count, const value_type& value)
    {
        static_cast<void>(value);
        if (count == 0U) {
            return;
        }

        static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
        static_cast<void>(Must_not_be_called_for_zero_sized_vector);
        COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector); 
    }

    template <std::size_t TOtherSize>
    void swap(StaticVector<T, TOtherSize>& other)
    {
        static_cast<void>(other);
        if (TOtherSize != 0U) {
            static constexpr bool Must_not_be_called_for_zero_sized_vector = false;
            static_cast<void>(Must_not_be_called_for_zero_sized_vector);
            COMMS_ASSERT(Must_not_be_called_for_zero_sized_vector);             
        }
    }

private:
    StorageType m_data;    
};

/// @brief Lexicographically compares the values in the vector.
/// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator_cmp">Reference</a>
/// @related StaticVector
template <typename T, std::size_t TSize1, std::size_t TSize2>
bool operator<(const StaticVector<T, TSize1>& v1, const StaticVector<T, TSize2>& v2)
{
    return std::lexicographical_compare(v1.begin(), v1.end(), v2.begin(), v2.end());
}

/// @brief Lexicographically compares the values in the vector.
/// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator_cmp">Reference</a>
/// @related StaticVector
template <typename T, std::size_t TSize1, std::size_t TSize2>
bool operator<=(const StaticVector<T, TSize1>& v1, const StaticVector<T, TSize2>& v2)
{
    return !(v2 < v1);
}

/// @brief Lexicographically compares the values in the vector.
/// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator_cmp">Reference</a>
/// @related StaticVector
template <typename T, std::size_t TSize1, std::size_t TSize2>
bool operator>(const StaticVector<T, TSize1>& v1, const StaticVector<T, TSize2>& v2)
{
    return v2 < v1;
}

/// @brief Lexicographically compares the values in the vector.
/// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator_cmp">Reference</a>
/// @related StaticVector
template <typename T, std::size_t TSize1, std::size_t TSize2>
bool operator>=(const StaticVector<T, TSize1>& v1, const StaticVector<T, TSize2>& v2)
{
    return !(v1 < v2);
}

/// @brief Lexicographically compares the values in the vector.
/// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator_cmp">Reference</a>
/// @related StaticVector
template <typename T, std::size_t TSize1, std::size_t TSize2>
bool operator==(const StaticVector<T, TSize1>& v1, const StaticVector<T, TSize2>& v2)
{
    return (v1.size() == v2.size()) &&
           (!(v1 < v2)) &&
           (!(v2 < v1));
}

/// @brief Lexicographically compares the values in the vector.
/// @see <a href="http://en.cppreference.com/w/cpp/container/vector/operator_cmp">Reference</a>
/// @related StaticVector
template <typename T, std::size_t TSize1, std::size_t TSize2>
bool operator!=(const StaticVector<T, TSize1>& v1, const StaticVector<T, TSize2>& v2)
{
    return !(v1 == v2);
}

namespace details
{


template <typename T>
struct IsStaticVector
{
    static const bool Value = false;
};

template <typename T, std::size_t TSize>
struct IsStaticVector<comms::util::StaticVector<T, TSize> >
{
    static const bool Value = true;
};

} // namespace details

/// @brief Compile time check whether the provided type is a variant of
///     @ref comms::util::StaticVector
/// @related comms::util::StaticVector
template <typename T>
static constexpr bool isStaticVector()
{
    return details::IsStaticVector<T>::Value;
}

}  // namespace util

}  // namespace comms

namespace std
{

/// @brief Specializes the std::swap algorithm.
/// @see <a href="http://en.cppreference.com/w/cpp/container/vector/swap2">Reference</a>
/// @related comms::util::StaticVector
template <typename T, std::size_t TSize1, std::size_t TSize2>
void swap(comms::util::StaticVector<T, TSize1>& v1, comms::util::StaticVector<T, TSize2>& v2)
{
    v1.swap(v2);
}

}

COMMS_MSVC_WARNING_POP
COMMS_GNU_WARNING_POP