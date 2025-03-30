//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"
#include "comms/field/basic/CommonFuncs.h"
#include "comms/field/details/FieldOpHelpers.h"
#include "comms/field/details/MembersVersionDependency.h"
#include "comms/field/IntValue.h"
#include "comms/field/tag.h"
#include "comms/util/access.h"
#include "comms/util/SizeToType.h"
#include "comms/util/Tuple.h"
#include "comms/util/type_traits.h"

#include <limits>
#include <type_traits>


namespace comms
{

namespace field
{

namespace basic
{

namespace details
{

template <typename TSerializedType>
class BitfieldReadHelper
{
public:
    BitfieldReadHelper(TSerializedType val, ErrorStatus& es)
      : m_value(val),
        m_es(es) 
    {
    }

    template <std::size_t TIdx, typename TFieldParam>
    void operator()(TFieldParam&& field)
    {
        if (m_es != comms::ErrorStatus::Success) {
            return;
        }

        using FieldType = typename std::decay<decltype(field)>::type;
        static const auto FieldBitLength = 
            comms::util::FieldBitLengthIntType<>::template Type<FieldType>::value;
        static const auto Mask =
            (static_cast<TSerializedType>(1) << FieldBitLength) - 1;

        auto fieldSerValue =
            static_cast<TSerializedType>((m_value >> m_pos) & Mask);

        m_pos += FieldBitLength;

        static_assert(FieldType::minLength() == FieldType::maxLength(),
            "Bitfield doesn't support members with variable length");

        static const std::size_t MaxLength = FieldType::maxLength();
        std::uint8_t buf[MaxLength];
        auto* writeIter = &buf[0];
        using FieldEndian = typename FieldType::Endian;
        comms::util::writeData<MaxLength>(fieldSerValue, writeIter, FieldEndian());

        const auto* readIter = &buf[0];
        m_es = field.read(readIter, MaxLength);
    }

private:
    TSerializedType m_value = TSerializedType();
    ErrorStatus& m_es;
    std::size_t m_pos = 0U;
};

template <typename TSerializedType>
class BitfieldReadNoStatusHelper
{
public:
    BitfieldReadNoStatusHelper(TSerializedType val)
        : m_value(val) {}

    template <std::size_t TIdx, typename TFieldParam>
    void operator()(TFieldParam&& field)
    {
        using FieldType = typename std::decay<decltype(field)>::type;
        static const auto FieldBitLength = 
            comms::util::FieldBitLengthIntType<>::template Type<FieldType>::value;
        static const auto Mask =
            (static_cast<TSerializedType>(1) << FieldBitLength) - 1;

        auto fieldSerValue =
            static_cast<TSerializedType>((m_value >> m_pos) & Mask);
        m_pos += FieldBitLength;

        static_assert(FieldType::minLength() == FieldType::maxLength(),
            "Bitfield doesn't support members with variable length");

        static const std::size_t MaxLength = FieldType::maxLength();
        std::uint8_t buf[MaxLength];
        auto* writeIter = &buf[0];
        using FieldEndian = typename FieldType::Endian;
        comms::util::writeData<MaxLength>(fieldSerValue, writeIter, FieldEndian());

        const auto* readIter = &buf[0];
        field.readNoStatus(readIter);
    }

private:
    TSerializedType m_value = TSerializedType();
    std::size_t m_pos = 0U;
};

template <typename TSerializedType>
class BitfieldWriteHelper
{
public:
    BitfieldWriteHelper(TSerializedType& val, ErrorStatus& es)
      : m_value(val),
        m_es(es) {}

    template <std::size_t TIdx, typename TFieldParam>
    void operator()(TFieldParam&& field)
    {
        if (m_es != comms::ErrorStatus::Success) {
            return;
        }

        using FieldType = typename std::decay<decltype(field)>::type;

        static_assert(FieldType::minLength() == FieldType::maxLength(),
            "Bitfield supports fixed length members only.");

        static const std::size_t MaxLength = FieldType::maxLength();
        std::uint8_t buf[MaxLength];
        auto* writeIter = &buf[0];
        m_es = field.write(writeIter, MaxLength);
        if (m_es != comms::ErrorStatus::Success) {
            return;
        }

        using FieldEndian = typename FieldType::Endian;
        const auto* readIter = &buf[0];
        auto fieldSerValue = comms::util::readData<TSerializedType, MaxLength>(readIter, FieldEndian());

        static const auto FieldBitLength = 
            comms::util::FieldBitLengthIntType<>::template Type<FieldType>::value;
        static const auto Mask =
            (static_cast<TSerializedType>(1) << FieldBitLength) - 1;

        static const auto ClearMask = static_cast<TSerializedType>(~(Mask << m_pos));

        auto valueMask =
            static_cast<TSerializedType>(
                (static_cast<TSerializedType>(fieldSerValue) & Mask) << m_pos);

        m_value &= ClearMask;
        m_value |= valueMask;
        m_pos += FieldBitLength;
    }


private:
    TSerializedType& m_value;
    ErrorStatus& m_es;
    std::size_t m_pos = 0U;
};

template <typename TSerializedType>
class BitfieldWriteNoStatusHelper
{
public:
    BitfieldWriteNoStatusHelper(TSerializedType& val)
        : m_value(val) {}

    template <std::size_t TIdx, typename TFieldParam>
    void operator()(TFieldParam&& field)
    {

        using FieldType = typename std::decay<decltype(field)>::type;

        static_assert(FieldType::minLength() == FieldType::maxLength(),
            "Bitfield supports fixed length members only.");

        static const std::size_t MaxLength = FieldType::maxLength();
        std::uint8_t buf[MaxLength];
        auto* writeIter = &buf[0];
        field.writeNoStatus(writeIter);

        using FieldEndian = typename FieldType::Endian;
        const auto* readIter = &buf[0];
        auto fieldSerValue = comms::util::readData<TSerializedType, MaxLength>(readIter, FieldEndian());

        static const auto FieldBitLength = 
            comms::util::FieldBitLengthIntType<>::template Type<FieldType>::value;

        static const auto Mask =
            (static_cast<TSerializedType>(1) << FieldBitLength) - 1;

        static const auto ClearMask = static_cast<TSerializedType>(~(Mask << m_pos));

        auto valueMask =
            static_cast<TSerializedType>(
                (static_cast<TSerializedType>(fieldSerValue) & Mask) << m_pos);

        m_value &= ClearMask;
        m_value |= valueMask;
        m_pos += FieldBitLength;
    }

private:
    TSerializedType& m_value;
    std::size_t m_pos = 0U;
};

template <comms::field::details::MembersVersionDependency TVersionDependency, typename... TMembers>
struct BitfieldVersionDependencyDetectHelper;

template <typename... TMembers>
struct BitfieldVersionDependencyDetectHelper<comms::field::details::MembersVersionDependency_NotSpecified, TMembers...>
{
    static constexpr bool Value = CommonFuncs::IsAnyFieldVersionDependentBoolType<TMembers...>::value;
};

template <typename... TMembers>
struct BitfieldVersionDependencyDetectHelper<comms::field::details::MembersVersionDependency_Independent, TMembers...>
{
    static constexpr bool Value = false;
};

template <typename... TMembers>
struct BitfieldVersionDependencyDetectHelper<comms::field::details::MembersVersionDependency_Dependent, TMembers...>
{
    static constexpr bool Value = true;
};

}  // namespace details

template <typename TFieldBase, comms::field::details::MembersVersionDependency TVersionDependency, typename TMembers>
class Bitfield;

template <typename TFieldBase, comms::field::details::MembersVersionDependency TVersionDependency, typename... TMembers>
class Bitfield<TFieldBase, TVersionDependency, std::tuple<TMembers...> > : public TFieldBase
{
    using BaseImpl = TFieldBase;
    using Members = std::tuple<TMembers...>;

    static_assert(
        1U < std::tuple_size<Members>::value,
        "Number of members is expected to be at least 2.");


    static const std::size_t TotalBits = CommonFuncs::FieldSumTotalBitLengthIntType<TMembers...>::value;
    static_assert(
        (TotalBits % std::numeric_limits<std::uint8_t>::digits) == 0,
        "Wrong number of total bits");

    static const std::size_t Length = TotalBits / std::numeric_limits<std::uint8_t>::digits;
    static_assert(0U < Length, "Serialised length is expected to be greater than 0");
    using SerializedType = typename comms::util::SizeToType<Length, false>::Type;

    using FixedIntValueField =
        comms::field::IntValue<
            TFieldBase,
            SerializedType,
            comms::option::def::FixedLength<Length>
        >;


    using SimpleIntValueField =
        comms::field::IntValue<
            TFieldBase,
            SerializedType
        >;

    using IntValueField = 
        typename comms::util::Conditional<
            ((Length & (Length - 1)) == 0)
        >::template Type<
            SimpleIntValueField,
            FixedIntValueField
        >;

public:
    using Endian = typename BaseImpl::Endian;
    using VersionType = typename BaseImpl::VersionType;
    using ValueType = Members;
    using CommsTag = comms::field::tag::Bitfield;

    Bitfield() = default;
    explicit Bitfield(const ValueType& val)
      : m_members(val)
    {
    }

    explicit Bitfield(ValueType&& val)
      : m_members(std::move(val))
    {
    }

    const ValueType& value() const
    {
        return m_members;
    }

    ValueType& value()
    {
        return m_members;
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

    static constexpr std::size_t length()
    {
        return Length;
    }

    static constexpr std::size_t minLength()
    {
        return length();
    }

    static constexpr std::size_t maxLength()
    {
        return length();
    }

    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t size)
    {
        if (size < length()) {
            return ErrorStatus::NotEnoughData;
        }

        auto serValue = BaseImpl::template readData<SerializedType, Length>(iter);
        ErrorStatus es = ErrorStatus::Success;
        comms::util::tupleForEachWithTemplateParamIdx(
            m_members, details::BitfieldReadHelper<SerializedType>(serValue, es));
        return es;
    }

    static constexpr bool hasReadNoStatus()
    {
        return CommonFuncs::AllFieldsHaveReadNoStatusBoolType<TMembers...>::value;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter)
    {
        auto serValue = BaseImpl::template readData<SerializedType, Length>(iter);
        comms::util::tupleForEachWithTemplateParamIdx(
            m_members, details::BitfieldReadNoStatusHelper<SerializedType>(serValue));
    }

    bool canWrite() const
    {
        return 
            comms::util::tupleAccumulate(
                value(), true, comms::field::details::FieldCanWriteCheckHelper<>());        
    }

    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t size) const
    {
        if (size < length()) {
            return ErrorStatus::BufferOverflow;
        }

        SerializedType serValue = 0;
        ErrorStatus es = ErrorStatus::Success;
        comms::util::tupleForEachWithTemplateParamIdx(
            m_members, details::BitfieldWriteHelper<SerializedType>(serValue, es));
        if (es == ErrorStatus::Success) {
            comms::util::writeData<Length>(serValue, iter, Endian());
        }
        return es;
    }

    static constexpr bool hasWriteNoStatus()
    {
        return CommonFuncs::AllFieldsHaveWriteNoStatusBoolType<TMembers...>::value;
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) const
    {
        SerializedType serValue = 0;
        comms::util::tupleForEachWithTemplateParamIdx(
            m_members, details::BitfieldWriteNoStatusHelper<SerializedType>(serValue));
        comms::util::writeData<Length>(serValue, iter, Endian());
    }

    constexpr bool valid() const
    {
        return comms::util::tupleAccumulate(value(), true, comms::field::details::FieldValidCheckHelper<>());
    }

    bool refresh()
    {
        return comms::util::tupleAccumulate(
            m_members, false, comms::field::details::FieldRefreshHelper<>());
    }

    template <std::size_t TIdx>
    static constexpr std::size_t memberBitLength()
    {
        static_assert(
            TIdx < std::tuple_size<ValueType>::value,
            "Index exceeds number of fields");

        using FieldType = typename std::tuple_element<TIdx, ValueType>::type;
        return comms::util::FieldBitLengthIntType<>::template Type<FieldType>::value;
    }

    static constexpr bool isVersionDependent()
    {
        return details::BitfieldVersionDependencyDetectHelper<TVersionDependency, TMembers...>::Value;
    }

    static constexpr bool hasNonDefaultRefresh()
    {
        return CommonFuncs::AnyFieldHasNonDefaultRefreshBoolType<TMembers...>::value;
    }

    bool setVersion(VersionType version)
    {
        return CommonFuncs::setVersionForMembers(value(), version);
    }

protected:
    using BaseImpl::readData;
    using BaseImpl::writeData;

private:
    ValueType m_members;
};

}  // namespace basic

}  // namespace field

}  // namespace comms


