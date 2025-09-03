//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/ErrorStatus.h"
#include "comms/field/tag.h"

#include <ratio>
#include <type_traits>
#include <utility>

namespace comms
{

namespace field
{

namespace basic
{

template <typename TFieldBase, typename T>
class IntValue : public TFieldBase
{
    static_assert(std::is_integral<T>::value, "T must be integral value");

    using BaseImpl = TFieldBase;
public:

    using ValueType = T;
    using SerialisedType = ValueType;
    using ScalingRatio = std::ratio<1, 1>;
    using CommsTag = comms::field::tag::Int;
    using DisplayOffsetType = typename std::make_signed<ValueType>::type;

    IntValue() = default;

    explicit IntValue(ValueType val)
      : m_value(val)
    {
    }

    IntValue(const IntValue&) = default;
    IntValue(IntValue&&) = default;
    ~IntValue() noexcept = default;

    IntValue& operator=(const IntValue&) = default;
    IntValue& operator=(IntValue&&) = default;

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

    template <typename U>
    void setValue(U&& val)
    {
        value() = static_cast<ValueType>(std::forward<U>(val));
    }    

    static constexpr std::size_t length()
    {
        return sizeof(SerialisedType);
    }

    static constexpr std::size_t minLength()
    {
        return length();
    }

    static constexpr std::size_t maxLength()
    {
        return length();
    }

    static constexpr SerialisedType toSerialised(ValueType val)
    {
        return static_cast<SerialisedType>(val);
    }

    static constexpr ValueType fromSerialised(SerialisedType val)
    {
        return static_cast<ValueType>(val);
    }

    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t size)
    {
        if (size < length()) {
            return ErrorStatus::NotEnoughData;
        }

        readNoStatus(iter);
        return ErrorStatus::Success;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter)
    {
        auto serialisedValue =
            BaseImpl::template readData<SerialisedType>(iter);
        m_value = fromSerialised(serialisedValue);
    }

    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t size) const
    {
        if (size < length()) {
            return ErrorStatus::BufferOverflow;
        }

        writeNoStatus(iter);
        return ErrorStatus::Success;
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) const
    {
        BaseImpl::writeData(toSerialised(m_value), iter);
    }

    static constexpr DisplayOffsetType displayOffset()
    {
        return static_cast<DisplayOffsetType>(0);
    }

private:
    ValueType m_value = static_cast<ValueType>(0);
};

}  // namespace basic

}  // namespace field

}  // namespace comms


