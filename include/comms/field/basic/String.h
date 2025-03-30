//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/CompileControl.h"
#include "comms/details/tag.h"
#include "comms/ErrorStatus.h"
#include "comms/field/basic/CommonFuncs.h"
#include "comms/field/tag.h"
#include "comms/util/access.h"
#include "comms/util/assign.h"
#include "comms/util/detect.h"
#include "comms/util/MaxSizeOf.h"
#include "comms/util/StaticString.h"
#include "comms/util/StaticVector.h"
#include "comms/util/type_traits.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <type_traits>

COMMS_MSVC_WARNING_PUSH
COMMS_MSVC_WARNING_DISABLE(4100)

namespace comms
{

namespace field
{

namespace basic
{

namespace details
{

template <typename TStorage>
struct StringMaxLengthRetrieveHelper
{
    static const std::size_t Value = CommonFuncs::maxSupportedLength();
};

template <std::size_t TSize>
struct StringMaxLengthRetrieveHelper<comms::util::StaticString<TSize> >
{
    static const std::size_t Value = TSize - 1;
};

}  // namespace details

template <typename TFieldBase, typename TStorage>
class String : public TFieldBase
{
    using BaseImpl = TFieldBase;
public:
    using Endian = typename BaseImpl::Endian;

    using ValueType = TStorage;
    using ElementType = typename TStorage::value_type;
    using CommsTag = comms::field::tag::String;

    static_assert(std::is_integral<ElementType>::value, "String of characters only supported");
    static_assert(sizeof(ElementType) == sizeof(char), "Single byte charactes only supported");

    String() = default;

    explicit String(const ValueType& val)
      : m_value(val)
    {
    }

    explicit String(ValueType&& val)
      : m_value(std::move(val))
    {
    }

    String(const String&) = default;
    String(String&&) = default;
    String& operator=(const String&) = default;
    String& operator=(String&&) = default;
    ~String() noexcept = default;

    const ValueType& value() const
    {
        return m_value;
    }

    ValueType& value()
    {
        return m_value;
    }

    const ValueType& getValue() const
    {
        return value();
    }

    template <typename T>
    void setValue(T&& val)
    {
        value() = std::forward<T>(val);
    }    

    ValueType& createBack()
    {
        m_value.push_back(ValueType());
        return m_value.back();
    }

    void clear()
    {
        static_assert(comms::util::detect::hasClearFunc<ValueType>(),
                "The string type must have clear() member function");
        m_value.clear();
    }

    constexpr std::size_t length() const
    {
        return m_value.size() * sizeof(ElementType);
    }

    static constexpr std::size_t minLength()
    {
        return 0U;
    }

    static constexpr std::size_t maxLength()
    {
        return
            details::StringMaxLengthRetrieveHelper<TStorage>::Value *
            sizeof(ElementType);
    }

    static constexpr bool valid()
    {
        return true;
    }

    static constexpr std::size_t minElementLength()
    {
        return sizeof(ElementType);
    }

    static constexpr std::size_t maxElementLength()
    {
        return minElementLength();
    }

    static constexpr std::size_t elementLength(const ElementType& elem)
    {
        return sizeof(typename std::decay<decltype(elem)>::type);
    }

    template <typename TIter>
    static ErrorStatus readElement(ElementType& elem, TIter& iter, std::size_t& len)
    {
        if (len < sizeof(ElementType)) {
            return ErrorStatus::NotEnoughData;
        }

        elem = comms::util::readData<ElementType>(iter,  Endian());
        len -= sizeof(ElementType);
        return ErrorStatus::Success;
    }

    template <typename TIter>
    static void readElementNoStatus(ElementType& elem, TIter& iter)
    {
        elem = comms::util::readData<ElementType>(iter,  Endian());
    }

    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t len)
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using IterCategory =
            typename std::iterator_traits<IterType>::iterator_category;
        static_assert(std::is_base_of<std::random_access_iterator_tag, IterCategory>::value,
            "Iterator for reading is expected to be random access one");

        using ConstPointer = typename ValueType::const_pointer;
        auto* str = reinterpret_cast<ConstPointer>(&(*iter));
        auto endStr = str;
        std::advance(endStr, std::min(len, comms::util::maxSizeOf(m_value)));
        comms::util::assign(m_value, str, endStr);
        std::advance(iter, len);
        return ErrorStatus::Success;
    }

    static constexpr bool hasReadNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;

    template <typename TIter>
    ErrorStatus readN(std::size_t count, TIter& iter, std::size_t& len)
    {
        if (len < count) {
            return comms::ErrorStatus::NotEnoughData;
        }

        return read(iter, count);
    }

    template <typename TIter>
    void readNoStatusN(std::size_t count, TIter& iter)
    {
        read(iter, count);
    }

    template <typename TIter>
    static ErrorStatus writeElement(const ElementType& elem, TIter& iter, std::size_t& len)
    {
        if (len < sizeof(ElementType)) {
            return ErrorStatus::BufferOverflow;
        }

        comms::util::writeData(elem, iter, Endian());
        len -= sizeof(ElementType);
        return ErrorStatus::Success;
    }

    template <typename TIter>
    static void writeElementNoStatus(const ElementType& elem, TIter& iter)
    {
        comms::util::writeData(elem, iter, Endian());
    }

    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t len) const
    {
        if (len < length()) {
            return comms::ErrorStatus::BufferOverflow;
        }

        writeNoStatus(iter);
        return comms::ErrorStatus::Success;
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) const
    {
        std::copy_n(m_value.begin(), m_value.size(), iter);
        doAdvance(iter, m_value.size());
    }

    template <typename TIter>
    ErrorStatus writeN(std::size_t count, TIter& iter, std::size_t& len) const
    {
        count = std::min(count, m_value.size());

        if (len < count) {
            return comms::ErrorStatus::BufferOverflow;
        }

        writeNoStatusN(count, iter);
        return comms::ErrorStatus::Success;
    }

    template <typename TIter>
    void writeNoStatusN(std::size_t count, TIter& iter) const
    {
        count = std::min(count, m_value.size());
        std::copy_n(m_value.begin(), count, iter);
        doAdvance(iter, count);
    }

private:
    template<typename... TParams>
    using AdvancableTag = comms::details::tag::Tag1<>;

    template<typename... TParams>
    using NotAdvancableTag = comms::details::tag::Tag2<>;

    template <typename TIter>
    static void doAdvance(TIter& iter, std::size_t len)
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using IterCategory = typename std::iterator_traits<IterType>::iterator_category;
        static const bool InputIter =
                std::is_base_of<std::input_iterator_tag, IterCategory>::value;
        using Tag =
            typename comms::util::LazyShallowConditional<
                InputIter
            >::template Type<
                AdvancableTag,
                NotAdvancableTag
            >;
        doAdvance(iter, len, Tag());
    }

    template <typename TIter, typename... TParams>
    static void doAdvance(TIter& iter, std::size_t len, AdvancableTag<TParams...>)
    {
        std::advance(iter, len);
    }

    template <typename TIter, typename... TParams>
    static void doAdvance(TIter&, std::size_t, NotAdvancableTag<TParams...>)
    {
    }

    ValueType m_value;
};

}  // namespace basic

}  // namespace field

}  // namespace comms

COMMS_MSVC_WARNING_POP
