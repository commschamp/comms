//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/protocol/ProtocolLayerBase.h"
#include "comms/protocol/details/ChecksumLayerOptionsParser.h"

#include <cstddef>

namespace comms
{

namespace protocol
{

// Forward declaration
template <typename TField, typename TCalc, typename TNextLayer, typename... TOptions>
class ChecksumPrefixLayer;

namespace details
{

template <typename TField, typename TCalc, typename TNextLayer, typename... TOptions>
class ChecksumPrefixLayerBaseHelper
{
    using ParsedOptionsInternal = comms::protocol::details::ChecksumLayerOptionsParser<TOptions...>;
    using ActualLayer = 
        typename ParsedOptionsInternal::template DefineExtendingClass<
            ChecksumPrefixLayer<TField, TCalc, TNextLayer, TOptions...>
        >;
        
    using TopBase = 
        comms::protocol::ProtocolLayerBase<
            TField,
            TNextLayer,
            ActualLayer,
            typename ParsedOptionsInternal::template SuppressForVerifyBeforeRead<comms::option::def::ProtocolLayerDisallowReadUntilDataSplit>
        >;

public:    
    using Base = TopBase;
};

template <typename TField, typename TCalc, typename TNextLayer, typename... TOptions>
using ChecksumPrefixLayerBase = typename ChecksumPrefixLayerBaseHelper<TField, TCalc, TNextLayer, TOptions...>::Base;

} // namespace details

} // namespace protocol

} // namespace comms
