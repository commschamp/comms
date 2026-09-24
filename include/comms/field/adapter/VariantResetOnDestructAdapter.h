//
// Copyright 2017 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <type_traits>

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TBase>
class VariantResetOnDestructAdapter : public TBase
{
    using BaseImpl = TBase;
    static_assert(std::is_same<typename BaseImpl::CommsTag, comms::field::tag::Variant>::value, "Applicable only to variant fields");
public:

    ~VariantResetOnDestructAdapter()
    {
        BaseImpl::reset();
    }

    VariantResetOnDestructAdapter() = default;
    VariantResetOnDestructAdapter(const VariantResetOnDestructAdapter&) = default;
    VariantResetOnDestructAdapter(VariantResetOnDestructAdapter&&) = default;
    VariantResetOnDestructAdapter& operator=(const VariantResetOnDestructAdapter&) = default;
    VariantResetOnDestructAdapter& operator=(VariantResetOnDestructAdapter&&) = default;

    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t len)
    {
        BaseImpl::reset();
        return BaseImpl::read(iter, len);
    }

};

}  // namespace adapter

}  // namespace field

}  // namespace comms

