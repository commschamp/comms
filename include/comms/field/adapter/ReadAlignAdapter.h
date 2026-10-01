//
// Copyright 2015 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/ErrorStatus.h"

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

template <std::size_t TAlign, typename TBase>
class ReadAlignAdapter : public TBase
{
    using BaseImpl = TBase;
    static const auto Align = TAlign;
    static_assert((Align & (Align - 1U)) == 0, "Alignment parameter is expected to be power of 2");
    static const auto AlignMask = Align - 1U;
public:

    using ValueType = typename BaseImpl::ValueType;
    using SerialisedType = typename BaseImpl::SerialisedType;

    ReadAlignAdapter() = default;

    explicit ReadAlignAdapter(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit ReadAlignAdapter(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    ReadAlignAdapter(const ReadAlignAdapter&) = default;
    ReadAlignAdapter(ReadAlignAdapter&&) = default;
    ReadAlignAdapter& operator=(const ReadAlignAdapter&) = default;
    ReadAlignAdapter& operator=(ReadAlignAdapter&&) = default;

    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t size)
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using IterCategory = typename std::iterator_traits<IterType>::iterator_category;
        static_assert(std::is_base_of<std::random_access_iterator_tag, IterCategory>::value,
            "Use random access iterators when using comms::option::def::ReadAlign option");

        while (0U < size) {
            if ((reinterpret_cast<std::uintptr_t>(&(*iter)) & AlignMask) == 0) {
                return BaseImpl::read(iter, size);
            }

            std::advance(iter, 1);
            size -= 1U;
        }

        return ErrorStatus::NotEnoughData;
    }

    static constexpr bool hasReadNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

