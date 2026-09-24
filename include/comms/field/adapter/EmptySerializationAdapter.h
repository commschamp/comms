//
// Copyright    2017 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"

#include <cstddef>
#include <utility>

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TBase>
class EmptySerializationAdapter : public TBase
{
    using BaseImpl = TBase;
public:

    using ValueType = typename BaseImpl::ValueType;

    EmptySerializationAdapter() = default;

    explicit EmptySerializationAdapter(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit EmptySerializationAdapter(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    EmptySerializationAdapter(const EmptySerializationAdapter&) = default;
    EmptySerializationAdapter(EmptySerializationAdapter&&) = default;
    EmptySerializationAdapter& operator=(const EmptySerializationAdapter&) = default;
    EmptySerializationAdapter& operator=(EmptySerializationAdapter&&) = default;

    static constexpr std::size_t length()
    {
        return 0U;
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
    static comms::ErrorStatus read(TIter&, std::size_t)
    {
        return comms::ErrorStatus::Success;
    }

    template <typename TIter>
    static void readNoStatus(TIter&)
    {
    }

    static constexpr bool canWrite()
    {
        return true;
    }

    template <typename TIter>
    static comms::ErrorStatus write(TIter&, std::size_t)
    {
        return comms::ErrorStatus::Success;
    }

    template <typename TIter>
    static void writeNoStatus(TIter&)
    {
    }
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

