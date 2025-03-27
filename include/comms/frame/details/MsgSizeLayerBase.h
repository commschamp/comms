//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/frame/details/MsgSizeLayerOptionsParser.h"
#include "comms/frame/FrameLayerBase.h"
#include "comms/options.h"

#include <cstddef>

namespace comms
{

namespace frame
{

// Forward declaration
template <typename TField, typename TNextLayer, typename... TOptions>
class MsgSizeLayer;

namespace details
{

template <typename TField, typename TNextLayer, typename... TOptions>
class MsgSizeLayerBaseHelper
{
    using ParsedOptionsInternal = comms::frame::details::MsgSizeLayerOptionsParser<TOptions...>;
    using ActualLayer = typename ParsedOptionsInternal::template DefineExtendingClass<MsgSizeLayer<TField, TNextLayer, TOptions...>>;
    using TopBase = 
        comms::frame::FrameLayerBase<
            TField,
            TNextLayer,
            ActualLayer,
            comms::option::FrameLayerDisallowReadUntilDataSplit
        >;

public:
    using Base = TopBase;
};

template <typename TField, typename TNextLayer, typename... TOptions>
using MsgSizeLayerBase = typename MsgSizeLayerBaseHelper<TField, TNextLayer, TOptions...>::Base;

} // namespace details

} // namespace frame

} // namespace comms
