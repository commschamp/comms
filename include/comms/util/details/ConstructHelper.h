//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/CompileControl.h"

#include <iterator>

#if COMMS_HAS_CPP20_SPAN
#include <span>
#endif // #if COMMS_HAS_CPP20_SPAN

namespace comms
{

namespace util
{

namespace details
{

template <typename T>
class ConstructHelper
{
public:
    using RetType = T;

    template <typename TIter>
    static RetType construct(TIter from, TIter to)
    {
        return RetType(from, to);
    }
};

#if COMMS_HAS_CPP20_SPAN
template <typename T, std::size_t TExtent>
class ConstructHelper<std::span<T, TExtent>>
{
public:
    using RetType = std::span<T, TExtent>;

    template <typename TIter>
    static RetType construct(TIter from, TIter to)
    {
        return RetType(&(*from), static_cast<typename RetType::size_type>(std::distance(from, to)));
    }
};
#endif // #if COMMS_HAS_CPP20_SPAN

} // namespace details

} // namespace util

} // namespace comms
