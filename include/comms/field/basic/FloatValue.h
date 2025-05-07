//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/ErrorStatus.h"
#include "comms/field/tag.h"
#include "comms/util/SizeToType.h"

#include <ratio>
#include <type_traits>

namespace comms
{

namespace field
{

namespace basic
{

template <typename TFieldBase, typename T>
class FloatValue : public TFieldBase
{
    static_assert(std::is_floating_point<T>::value, "T must be floating point value");

    using BaseImpl = TFieldBase;
public:

    using ValueType = T;
    using SerialisedType = typename comms::util::SizeToType<sizeof(ValueType), false>::Type;
    using ScalingRatio = std::ratio<1, 1>;
    using CommsTag = comms::field::tag::Float;

    FloatValue() = default;

    explicit FloatValue(ValueType val)
      : m_value(val)
    {
    }

    FloatValue(const FloatValue&) = default;
    FloatValue(FloatValue&&) = default;
    ~FloatValue() noexcept = default;

    FloatValue& operator=(const FloatValue&) = default;
    FloatValue& operator=(FloatValue&&) = default;

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

    static SerialisedType toSerialised(ValueType val)
    {
        CastUnion<> castUnion;
        castUnion.m_value = val;
        return castUnion.m_serValue;
    }

    static ValueType fromSerialised(SerialisedType val)
    {
        CastUnion<> castUnion;
        castUnion.m_serValue = val;
        return castUnion.m_value;
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

private:
    template<typename...>
    union CastUnion
    {
        ValueType m_value;
        SerialisedType m_serValue;
    };

    ValueType m_value = static_cast<ValueType>(0.0);
};

}  // namespace basic

}  // namespace field

}  // namespace comms


