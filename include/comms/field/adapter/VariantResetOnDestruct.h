//
// Copyright 2017 - 2023 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#pragma once

#include <type_traits>

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TBase>
class VariantResetOnDestruct : public TBase
{
    using BaseImpl = TBase;
    static_assert(std::is_same<typename BaseImpl::CommsTag, comms::field::tag::Variant>::value, "Applicable only to variant fields");
public:

    ~VariantResetOnDestruct()
    {
        BaseImpl::reset();
    }

    VariantResetOnDestruct() = default;
    VariantResetOnDestruct(const VariantResetOnDestruct&) = default;
    VariantResetOnDestruct(VariantResetOnDestruct&&) = default;
    VariantResetOnDestruct& operator=(const VariantResetOnDestruct&) = default;
    VariantResetOnDestruct& operator=(VariantResetOnDestruct&&) = default;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

