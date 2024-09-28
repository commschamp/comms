//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file comms/util/AlignedStorage.h
/// @brief Replacement of std::aligned_storage due to deprecation since C++23.

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace comms
{

namespace util
{

#if COMMS_IS_CPP23

template <std::size_t TSize, std::size_t TAlign>
using AlignedStorage = std::uint8_t[TSize];

#else // #if COMMS_IS_CPP23

template <std::size_t TSize, std::size_t TAlign>
using AlignedStorage = typename std::aligned_storage<TSize, TAlign>::type;

#endif // #if COMMS_IS_CPP23

} // namespace util

} // namespace comms
