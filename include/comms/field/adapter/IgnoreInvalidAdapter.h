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
class IgnoreInvalidAdapter : public TBase
{
    using BaseImpl = TBase;
public:

    using ValueType = typename BaseImpl::ValueType;

    IgnoreInvalidAdapter() = default;

    explicit IgnoreInvalidAdapter(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit IgnoreInvalidAdapter(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    IgnoreInvalidAdapter(const IgnoreInvalidAdapter&) = default;
    IgnoreInvalidAdapter(IgnoreInvalidAdapter&&) = default;
    IgnoreInvalidAdapter& operator=(const IgnoreInvalidAdapter&) = default;
    IgnoreInvalidAdapter& operator=(IgnoreInvalidAdapter&&) = default;

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        BaseImpl tmp;
        auto es = tmp.read(iter, len);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        if (tmp.valid()) {
            static_cast<BaseImpl&>(*this) = std::move(tmp);
        }

        return comms::ErrorStatus::Success;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter)
    {
        BaseImpl tmp;
        tmp.readNoStatus(iter);

        if (tmp.valid()) {
            static_cast<BaseImpl&>(*this) = std::move(tmp);
        }
    }
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

