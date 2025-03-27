//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::protocol::FrameLayerBase

#pragma once

#include "comms/frame/FrameLayerBase.h"

namespace comms
{

namespace protocol
{

/// @brief Alias to the @ref comms::frame::FrameLayerBase
/// @deprecated Use @ref comms::frame::FrameLayerBase class instead
template <typename TField, typename TNextLayer, typename TDerived, typename...  TOptions>
using FrameLayerBase = comms::frame::FrameLayerBase<TField, TNextLayer, TDerived, TOptions...>;

}  // namespace protocol

}  // namespace comms
