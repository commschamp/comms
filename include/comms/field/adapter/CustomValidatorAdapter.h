//
// Copyright 2015 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TValidator, typename TBase>
class CustomValidatorAdapter : public TBase
{
    using BaseImpl = TBase;
    using Validator = TValidator;

public:

    using ValueType = typename BaseImpl::ValueType;

    CustomValidatorAdapter() = default;

    explicit CustomValidatorAdapter(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit CustomValidatorAdapter(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    CustomValidatorAdapter(const CustomValidatorAdapter&) = default;
    CustomValidatorAdapter(CustomValidatorAdapter&&) = default;
    CustomValidatorAdapter& operator=(const CustomValidatorAdapter&) = default;
    CustomValidatorAdapter& operator=(CustomValidatorAdapter&&) = default;

    bool valid() const
    {
        return BaseImpl::valid() && (Validator()(*this));
    }
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

