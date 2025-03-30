//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::protocol::ProtocolLayerBase

#pragma once

#include "comms/frame/FrameLayerBase.h"

namespace comms
{

namespace protocol
{

/// @brief Alias to the @ref comms::frame::ProtocolLayerBase
/// @deprecated Use @ref comms::frame::ProtocolLayerBase class instead
template <typename TField, typename TNextLayer, typename TDerived, typename...  TOptions>
using ProtocolLayerBase = comms::frame::FrameLayerBase<TField, TNextLayer, TDerived, TOptions...>;

/// @brief Same as @ref comms::frame::toFrameLayerBase()
/// @deprecated Use @ref comms::frame::toFrameLayerBase() instead
template <typename TField,typename TNextLayer, typename TDerived, typename...  TOptions>
auto toProtocolLayerBase(ProtocolLayerBase<TField, TNextLayer, TDerived, TOptions...>& layer) -> decltype(comms::frame::toFrameLayerBase(layer))
{
    return comms::frame::toFrameLayerBase(layer);
}

/// @brief Same as @ref comms::frame::toFrameLayerBase()
/// @deprecated Use @ref comms::frame::toFrameLayerBase() instead
template <typename TField,typename TNextLayer, typename TDerived, typename...  TOptions>
constexpr auto toFrameLayerBase(const ProtocolLayerBase<TField, TNextLayer, TDerived, TOptions...>& layer) -> decltype(comms::frame::toFrameLayerBase(layer))
{
    return comms::frame::toFrameLayerBase(layer);
}

/// @brief Same as @ref comms::frame::missingSize()
/// @deprecated Use @ref comms::frame::missingSize() instead
auto missingSize(std::size_t& val) -> decltype(comms::frame::missingSize(val))
{
    return comms::frame::missingSize(val);
}

/// @brief Same as @ref comms::frame::msgId()
/// @deprecated Use @ref comms::frame::msgId() instead
template <typename TId>
auto msgId(TId& val) -> decltype(comms::frame::msgId(val))
{
    return comms::frame::msgId(val);
}

/// @brief Same as @ref comms::frame::msgIndex()
/// @deprecated Use @ref comms::frame::msgIndex() instead
auto msgIndex(std::size_t& val) -> decltype(comms::frame::msgIndex(val))
{
    return comms::frame::msgIndex(val);
}

/// @brief Same as @ref comms::frame::msgPayload()
/// @deprecated Use @ref comms::frame::msgPayload() instead
template <typename TIter>
auto msgPayload(TIter& iter, std::size_t& len) -> decltype(comms::frame::msgPayload(iter, len))
{
    return comms::frame::msgPayload(iter, len);
}

}  // namespace protocol

}  // namespace comms
