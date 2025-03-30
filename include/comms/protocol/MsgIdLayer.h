//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::protocol::MsgIdLayer

#pragma once

#include "comms/frame/MsgIdLayer.h"
#include "comms/protocol/ProtocolLayerBase.h"

namespace comms
{

namespace protocol
{

/// @brief Alias to the @ref comms::frame::MsgIdLayer
/// @deprecated Use @ref comms::frame::MsgIdLayer class instead
template <typename TField, typename TMessage, typename TAllMessages, typename TNextLayer, typename... TOptions>
using MsgIdLayer = comms::frame::MsgIdLayer<TField, TMessage, TAllMessages, TNextLayer, TOptions...>;

}  // namespace protocol

}  // namespace comms
