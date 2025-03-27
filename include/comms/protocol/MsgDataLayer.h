//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::protocol::MsgDataLayer

#pragma once

#include "comms/frame/MsgDataLayer.h"

namespace comms
{

namespace protocol
{

/// @brief Alias to the @ref comms::frame::MsgDataLayer
/// @deprecated Use @ref comms::frame::MsgDataLayer class instead
template <typename... TOptions>
using MsgDataLayer = comms::frame::MsgDataLayer<TOptions...>;

}  // namespace protocol

}  // namespace comms
