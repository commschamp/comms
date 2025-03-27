//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::protocol::ChecksumLayer

#pragma once

#include "comms/frame/ChecksumLayer.h"

namespace comms
{

namespace protocol
{

/// @brief Alias to the @ref comms::frame::ChecksumLayer
/// @deprecated Use @ref comms::frame::ChecksumLayer class instead
template <typename TField, typename TCalc, typename TNextLayer, typename... TOptions>
using ChecksumLayer = comms::frame::ChecksumLayer<TField, TCalc, TNextLayer, TOptions...>;

} // namespace protocol

} // namespace comms

