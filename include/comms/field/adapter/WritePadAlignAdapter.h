//
// Copyright 2015 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/details/tag.h"
#include "comms/ErrorStatus.h"
#include "comms/util/access.h"
#include "comms/util/type_traits.h"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>

namespace comms
{

namespace field
{

namespace adapter
{

template <std::size_t TAlign, bool TWritePadInLength, typename TBase>
class WritePadAlignAdapter : public TBase
{
    using BaseImpl = TBase;
    static const std::size_t Align = TAlign;
    static_assert((Align & (Align - 1U)) == 0, "Alignment parameter is expected to be power of 2");
    static const std::size_t AlignMask = Align - 1U;
    static const bool WritePadInLength = TWritePadInLength;

public:
    using ValueType = typename BaseImpl::ValueType;

    WritePadAlignAdapter() = default;

    explicit WritePadAlignAdapter(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit WritePadAlignAdapter(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    WritePadAlignAdapter(const WritePadAlignAdapter&) = default;
    WritePadAlignAdapter(WritePadAlignAdapter&&) = default;
    WritePadAlignAdapter& operator=(const WritePadAlignAdapter&) = default;
    WritePadAlignAdapter& operator=(WritePadAlignAdapter&&) = default;

    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t size) const
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using IterCategory = typename std::iterator_traits<IterType>::iterator_category;
        static_assert(std::is_base_of<std::random_access_iterator_tag, IterCategory>::value,
            "Use random access iterators when using comms::option::def::WritePadAlign option");

        auto fromIter = iter;
        COMMS_ASSERT((!WritePadInLength) || ((reinterpret_cast<std::uintptr_t>(&(*iter)) & AlignMask) == 0));
        auto es = BaseImpl::write(iter, size);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        auto consumed = static_cast<std::size_t>(std::distance(fromIter, iter));
        COMMS_ASSERT(consumed <= size);
        while (consumed < size) {
            if ((reinterpret_cast<std::uintptr_t>(&(*iter)) & AlignMask) == 0) {
                return es;
            }

            comms::util::writeBig(std::uint8_t(0), iter);
            ++consumed;
        }

        return ErrorStatus::BufferOverflow;
    }

    static constexpr bool hasWriteNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) = delete;

    std::size_t length() const
    {
        using Tag =
            typename comms::util::LazyShallowConditional<
                WritePadInLength
            >::template Type<
                PadInLengthTag,
                InheritedLengthTag
            >;

        return lengthInternal(Tag());
    }

private:
    template <typename... TParams>
    using PadInLengthTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using InheritedLengthTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    std::size_t lengthInternal(PadInLengthTag<TParams...>) const
    {
        return static_cast<std::size_t>((BaseImpl::length() + AlignMask) & static_cast<std::size_t>(~AlignMask));
    }

    template <typename... TParams>
    std::size_t lengthInternal(InheritedLengthTag<TParams...>) const
    {
        return BaseImpl::length();
    }
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

