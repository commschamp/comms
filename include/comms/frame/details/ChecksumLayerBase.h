//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/frame/details/ChecksumLayerOptionsParser.h"
#include "comms/frame/FrameLayerBase.h"

#include <cstddef>

namespace comms
{

namespace frame
{

// Forward declaration
template <typename TField, typename TCalc, typename TNextLayer, typename... TOptions>
class ChecksumLayer;

namespace details
{

template <typename TField, typename TCalc, typename TNextLayer, typename... TOptions>
class ChecksumLayerBaseHelper
{
    using ParsedOptionsInternal = comms::frame::details::ChecksumLayerOptionsParser<TOptions...>;
    using ActualLayer = 
        typename ParsedOptionsInternal::template DefineExtendingClass<
            ChecksumLayer<TField, TCalc, TNextLayer, TOptions...>
        >;
        
    using TopBase = 
        comms::frame::FrameLayerBase<
            TField,
            TNextLayer,
            ActualLayer,
            comms::option::def::FrameLayerDisallowReadUntilDataSplit
        >;

public:    
    using Base = TopBase;
};

template <typename TField, typename TCalc, typename TNextLayer, typename... TOptions>
using ChecksumLayerBase = typename ChecksumLayerBaseHelper<TField, TCalc, TNextLayer, TOptions...>::Base;

} // namespace details

} // namespace frame

} // namespace comms
