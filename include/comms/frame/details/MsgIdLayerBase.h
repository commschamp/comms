//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/frame/details/MsgIdLayerOptionsParser.h"
#include "comms/frame/FrameLayerBase.h"

#include <cstddef>

namespace comms
{

namespace frame
{

// Forward declaration
template <typename TField, typename TMessage, typename TAllMessages,typename TNextLayer, typename... TOptions>
class MsgIdLayer;

namespace details
{

template <typename TField, typename TMessage, typename TAllMessages,typename TNextLayer, typename... TOptions>
class MsgIdLayerBaseHelper
{
    using ParsedOptionsInternal = comms::frame::details::MsgIdLayerOptionsParser<TOptions...>;
    using ActualLayer = typename ParsedOptionsInternal::template DefineExtendingClass<MsgIdLayer<TField, TMessage, TAllMessages, TNextLayer, TOptions...>>;
    using TopBase = 
        comms::frame::FrameLayerBase<
            TField,
            TNextLayer,
            ActualLayer
        >;

public:
    using Base = TopBase;
};

template <typename TField, typename TMessage, typename TAllMessages,typename TNextLayer, typename... TOptions>
using MsgIdLayerBase = typename MsgIdLayerBaseHelper<TField, TMessage, TAllMessages, TNextLayer, TOptions...>::Base;

} // namespace details

} // namespace frame

} // namespace comms
