//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::protocol::checksum::BasicXor

#pragma once

#include "comms/frame/checksum/BasicXor.h"

namespace comms
{

namespace protocol
{

namespace checksum
{

/// @brief Alias to the @ref comms::frame::checksum::BasicXor
/// @deprecated Use @ref comms::frame::checksum::BasicXor class instead
template <typename TResult = std::uint8_t, TResult TInitValue = 0>
using BasicXor = comms::frame::checksum::BasicXor<TResult, TInitValue>;

}  // namespace checksum

}  // namespace protocol

}  // namespace comms



