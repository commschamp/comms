//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::protocol::checksum::Crc

#pragma once

#include "comms/frame/checksum/Crc.h"

namespace comms
{

namespace protocol
{

namespace checksum
{

/// @brief Alias to the @ref comms::frame::checksum::Crc
/// @deprecated Use @ref comms::frame::checksum::Crc class instead
template <
    typename TResult,
    TResult TPoly,
    TResult TInit = 0,
    TResult TFin = 0,
    bool TReflect = false,
    bool TReflectRem = false
>
using Crc = comms::frame::checksum::Crc<TResult, TPoly, TInit, TFin, TReflect, TReflectRem>;

}  // namespace checksum

}  // namespace protocol

}  // namespace comms



