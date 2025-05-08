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

/// @brief Alias to the @ref comms::frame::FrameLayerBase
/// @deprecated Use @ref comms::frame::FrameLayerBase class instead
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
inline auto missingSize(std::size_t& val) -> decltype(comms::frame::missingSize(val))
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
inline auto msgIndex(std::size_t& val) -> decltype(comms::frame::msgIndex(val))
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

/// @brief Same as #COMMS_FRAME_LAYERS_ACCESS()
/// @deprecated Use #COMMS_FRAME_LAYERS_ACCESS() instead
#define COMMS_PROTOCOL_LAYERS_ACCESS(...) COMMS_FRAME_LAYERS_ACCESS(__VA_ARGS__)

/// @brief Same as #COMMS_FRAME_LAYERS_NAMES()
/// @deprecated Use #COMMS_FRAME_LAYERS_NAMES() instead
#define COMMS_PROTOCOL_LAYERS_NAMES(...) COMMS_FRAME_LAYERS_NAMES(__VA_ARGS__)

/// @brief Same as #COMMS_FRAME_LAYERS_ACCESS_INNER()
/// @deprecated Use #COMMS_FRAME_LAYERS_ACCESS_INNER() instead
#define COMMS_PROTOCOL_LAYERS_ACCESS_INNER(...) COMMS_FRAME_LAYERS_ACCESS_INNER(__VA_ARGS__)

/// @brief Same as #COMMS_FRAME_LAYERS_NAMES_INNER()
/// @deprecated Use #COMMS_FRAME_LAYERS_NAMES_INNER() instead
#define COMMS_PROTOCOL_LAYERS_NAMES_INNER(...) COMMS_FRAME_LAYERS_NAMES_INNER(__VA_ARGS__)    

/// @brief Same as #COMMS_FRAME_LAYERS_ACCESS_OUTER()
/// @deprecated Use #COMMS_FRAME_LAYERS_ACCESS_OUTER() instead
#define COMMS_PROTOCOL_LAYERS_ACCESS_OUTER(...) COMMS_FRAME_LAYERS_ACCESS_OUTER(__VA_ARGS__)

/// @brief Same as #COMMS_FRAME_LAYERS_NAMES_OUTER()
/// @deprecated Use #COMMS_FRAME_LAYERS_NAMES_OUTER() instead
#define COMMS_PROTOCOL_LAYERS_NAMES_OUTER(...) COMMS_FRAME_LAYERS_NAMES_OUTER(__VA_ARGS__)
