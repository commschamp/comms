//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Provides helper construct() function to allow easy construction of various collection types.

#pragma once

#include "comms/util/details/ConstructHelper.h"

namespace comms
{

namespace util
{

/// @brief Construct collection objects given two range iterators
/// @details The function selects proper constructor of the selected type
/// @param[in] from Iterator to the first element of the range
/// @param[in] to Iterator to one behind the last element of the range.
template <typename T, typename TIter>
T construct(TIter from, TIter to)
{
    return details::ConstructHelper<T>::construct(from, to);
}

} // namespace util

} // namespace comms

