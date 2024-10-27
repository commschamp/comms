//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <type_traits>

namespace comms
{

namespace field
{

namespace adapter
{


template <std::intmax_t TOffset, typename TBase>
class DisplayOffset : public TBase
{
    using BaseImpl = TBase;
public:
    using DisplayOffsetType = typename TBase::DisplayOffsetType;

    static constexpr DisplayOffsetType displayOffset()
    {
        return static_cast<DisplayOffsetType>(TOffset);
    }    
};

}  // namespace adapter

}  // namespace field

}  // namespace comms




