//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::protocol::MsgSizeLayer

#pragma once

#include "comms/frame/MsgSizeLayer.h"
#include "comms/protocol/ProtocolLayerBase.h"

namespace comms
{

namespace protocol
{

/// @brief Alias to the @ref comms::frame::MsgSizeLayer
/// @deprecated Use @ref comms::frame::MsgSizeLayer class instead
template <typename TField, typename TNextLayer, typename... TOptions>
using MsgSizeLayer = comms::frame::MsgSizeLayer<TField, TNextLayer, TOptions...>;

}  // namespace protocol

}  // namespace comms
