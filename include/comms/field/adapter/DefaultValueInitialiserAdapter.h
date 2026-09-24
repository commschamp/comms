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

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TInitialiser, typename TBase>
class DefaultValueInitialiserAdapter : public TBase
{
    using BaseImpl = TBase;
    using Initialiser = TInitialiser;
public:
    using ValueType = typename BaseImpl::ValueType;

    DefaultValueInitialiserAdapter()
    {
        Initialiser()(*this);
    }

    explicit DefaultValueInitialiserAdapter(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit DefaultValueInitialiserAdapter(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    DefaultValueInitialiserAdapter(const DefaultValueInitialiserAdapter&) = default;
    DefaultValueInitialiserAdapter(DefaultValueInitialiserAdapter&&) = default;
    DefaultValueInitialiserAdapter& operator=(const DefaultValueInitialiserAdapter&) = default;
    DefaultValueInitialiserAdapter& operator=(DefaultValueInitialiserAdapter&&) = default;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

