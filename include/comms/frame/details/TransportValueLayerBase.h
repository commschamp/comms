//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/frame/details/TransportValueLayerOptionsParser.h"
#include "comms/frame/FrameLayerBase.h"

#include <cstddef>

namespace comms
{

namespace frame
{

// Forward declaration
template <typename TField, std::size_t TIdx, typename TNextLayer, typename... TOptions>
class TransportValueLayer;

namespace details
{

template <typename TField, std::size_t TIdx, typename TNextLayer, typename... TOptions>
class TransportValueLayerBaseHelper
{
    using ParsedOptionsInternal = comms::frame::details::TransportValueLayerOptionsParser<TOptions...>;
    using ActualLayer = typename ParsedOptionsInternal::template DefineExtendingClass<TransportValueLayer<TField, TIdx, TNextLayer, TOptions...>>;
    using TopBase = 
        comms::frame::FrameLayerBase<
            TField,
            TNextLayer,
            ActualLayer,
            typename ParsedOptionsInternal::template ForceReadUntilDataSplitIfNeeded<TNextLayer>
        >;

    using AdaptedPseudoBase = typename ParsedOptionsInternal::template BuildPseudoBase<TopBase>;

public:    
    using Base = AdaptedPseudoBase;
};

template <typename TField, std::size_t TIdx, typename TNextLayer, typename... TOptions>
using TransportValueLayerBase = typename TransportValueLayerBaseHelper<TField, TIdx, TNextLayer, TOptions...>::Base;

} // namespace details

} // namespace frame

} // namespace comms
