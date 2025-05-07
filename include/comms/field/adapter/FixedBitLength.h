//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/details/tag.h"
#include "comms/ErrorStatus.h"
#include "comms/util/SizeToType.h"
#include "comms/util/BitSizeToByteSize.h"
#include "comms/util/type_traits.h"

#include <limits>
#include <type_traits>

namespace comms
{

namespace field
{

namespace adapter
{

template <std::size_t TLen, bool TSignExtend, typename TBase>
class FixedBitLength : public TBase
{
    using BaseImpl = TBase;
    using BaseSerialisedType = typename BaseImpl::SerialisedType;
    using BaseUnsignedSerialisedType = typename std::make_unsigned<BaseSerialisedType>::type;

    static const std::size_t BitLength = TLen;
    static const std::size_t Length =
        comms::util::BitSizeToByteSize<BitLength>::Value;

    static_assert(0 < BitLength, "Bit length is expected to be greater than 0");
    static_assert(Length <= sizeof(BaseSerialisedType),
        "The provided length limit is too big");

public:

    using ValueType = typename BaseImpl::ValueType;

    using SerialisedType = 
        typename comms::util::Conditional<
            (Length < sizeof(BaseSerialisedType))
        >::template Type<
            typename comms::util::SizeToType<Length, std::is_signed<BaseSerialisedType>::value>::Type,
            BaseSerialisedType
        >;

    using Endian = typename BaseImpl::Endian;

    FixedBitLength() = default;

    explicit FixedBitLength(const ValueType& val)
      : BaseImpl(val)
    {
    }

    FixedBitLength(const FixedBitLength&) = default;
    FixedBitLength(FixedBitLength&&) = default;
    FixedBitLength& operator=(const FixedBitLength&) = default;
    FixedBitLength& operator=(FixedBitLength&&) = default;

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

    static constexpr SerialisedType toSerialised(ValueType val)
    {
        return adjustToSerialised(BaseImpl::toSerialised(val), HasSignTag<>());
    }

    static constexpr ValueType fromSerialised(SerialisedType val)
    {
        return BaseImpl::fromSerialised(adjustFromSerialised(val, HasSignTag<>()));
    }

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t size)
    {
        if (size < length()) {
            return comms::ErrorStatus::NotEnoughData;
        }

        readNoStatus(iter);
        return comms::ErrorStatus::Success;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter)
    {
        auto serialisedValue =
            comms::util::readData<SerialisedType, Length>(iter, Endian());
        BaseImpl::setValue(fromSerialised(serialisedValue));
    }

    template <typename TIter>
    comms::ErrorStatus write(TIter& iter, std::size_t size) const
    {
        if (size < length()) {
            return comms::ErrorStatus::BufferOverflow;
        }

        writeNoStatus(iter);
        return comms::ErrorStatus::Success;
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) const
    {
        BaseImpl::template writeData<Length>(toSerialised(BaseImpl::getValue()), iter);
    }

private:
    template <typename... TParams>
    using UnsignedTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using SignedTag = comms::details::tag::Tag2<>;    

    template <typename... TParams>
    using NoSignExtTag = comms::details::tag::Tag3<>;      

    template <typename... TParams>
    using MustSignExtTag = comms::details::tag::Tag4<>;         

    template <typename...>
    using HasSignTag = 
        typename comms::util::LazyShallowConditional<
            TSignExtend && std::is_signed<SerialisedType>::value
        >::template Type<
            SignedTag,
            UnsignedTag
        >;

    using UnsignedSerialisedType = typename std::make_unsigned<SerialisedType>::type;

    template <typename... TParams>
    static SerialisedType adjustToSerialised(BaseSerialisedType val, UnsignedTag<TParams...>)
    {
        return static_cast<SerialisedType>(static_cast<BaseUnsignedSerialisedType>(val) & UnsignedValueMask);
    }

    template <typename... TParams>
    static SerialisedType adjustToSerialised(BaseSerialisedType val, SignedTag<TParams...>)
    {
        auto valueTmp =
            static_cast<UnsignedSerialisedType>(
                static_cast<BaseUnsignedSerialisedType>(val) & UnsignedValueMask);

        return signExtUnsignedSerialised(valueTmp);
    }

    template <typename... TParams>
    static BaseSerialisedType adjustFromSerialised(SerialisedType val, UnsignedTag<TParams...>)
    {
        return static_cast<BaseSerialisedType>(static_cast<UnsignedSerialisedType>(val) & UnsignedValueMask);
    }

    template <typename... TParams>
    static BaseSerialisedType adjustFromSerialised(SerialisedType val, SignedTag<TParams...>)
    {
        auto valueTmp = 
            static_cast<UnsignedSerialisedType>(
                static_cast<UnsignedSerialisedType>(val) & UnsignedValueMask);
        return
            static_cast<BaseSerialisedType>(
                signExtUnsignedSerialised(valueTmp));
    }

    static SerialisedType signExtUnsignedSerialised(UnsignedSerialisedType val)
    {
        using SignExtTag = 
            typename comms::util::LazyShallowConditional<
                BitLength < static_cast<std::size_t>(std::numeric_limits<UnsignedSerialisedType>::digits)
            >::template Type<
                MustSignExtTag,
                NoSignExtTag
            >;
        return signExtUnsignedSerialisedInternal(val, SignExtTag());
    }

    template <typename... TParams>
    static SerialisedType signExtUnsignedSerialisedInternal(UnsignedSerialisedType val, MustSignExtTag<TParams...>)
    {
        static_assert(
            BitLength < std::numeric_limits<UnsignedSerialisedType>::digits,
            "BitLength is expected to be less than number of bits in the value type");

        static const auto SignExtMask =
            static_cast<UnsignedSerialisedType>(
                ~((static_cast<UnsignedSerialisedType>(1U) << BitLength) - 1));

        static const auto SignMask =
            static_cast<UnsignedSerialisedType>(
                static_cast<UnsignedSerialisedType>(1U) << (BitLength - 1));

        if ((val & SignMask) != 0) {
            val |= SignExtMask;
        }
        return static_cast<SerialisedType>(val);
    }

    template <typename... TParams>
    static SerialisedType signExtUnsignedSerialisedInternal(UnsignedSerialisedType val, NoSignExtTag<TParams...>)
    {
        return static_cast<SerialisedType>(val);
    }

private:
    static const UnsignedSerialisedType UnsignedValueMask =
        (static_cast<UnsignedSerialisedType>(1U) << BitLength) - 1;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms


