//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::protocol::ChecksumPrefixLayer

#pragma once

#include "comms/frame/ChecksumPrefixLayer.h"
#include "comms/protocol/ProtocolLayerBase.h"

namespace comms
{

namespace protocol
{

/// @brief Alias to the @ref comms::frame::ChecksumPrefixLayer
/// @deprecated Use @ref comms::frame::ChecksumPrefixLayer class instead
template <typename TField, typename TCalc, typename TNextLayer, typename... TOptions>
using ChecksumPrefixLayer = comms::frame::ChecksumPrefixLayer<TField, TCalc, TNextLayer, TOptions...>;

}  // namespace protocol

}  // namespace comms
