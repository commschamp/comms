//
// Copyright 2026 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains detection of the system endian

#pragma once

#include "comms/traits.h"

namespace comms
{

namespace util
{

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

using SysEndian = comms::traits::endian::Big;

#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

using SysEndian = comms::traits::endian::Little;

#elif defined(_MSC_VER)

// Microsoft is always little endian
using SysEndian = comms::traits::endian::Little;

#else

using SysEndian = void;

#endif

}  // namespace util

}  // namespace comms

