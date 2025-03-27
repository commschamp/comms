//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::protocol::TransportValueLayer

#pragma once

#include "comms/frame/TransportValueLayer.h"

namespace comms
{

namespace protocol
{

/// @brief Alias to the @ref comms::frame::TransportValueLayer
/// @deprecated Use @ref comms::frame::TransportValueLayer class instead
template <typename TField, std::size_t TIdx, typename TNextLayer, typename... TOptions>
using TransportValueLayer = comms::frame::TransportValueLayer<TField, TIdx, TNextLayer, TOptions...>;

}  // namespace protocol

}  // namespace comms
