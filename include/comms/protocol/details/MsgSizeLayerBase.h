//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/protocol/details/MsgSizeLayerOptionsParser.h"
#include "comms/protocol/ProtocolLayerBase.h"
#include "comms/options.h"

#include <cstddef>

namespace comms
{

namespace protocol
{

// Forward declaration
template <typename TField, typename TNextLayer, typename... TOptions>
class MsgSizeLayer;

namespace details
{

template <typename TField, typename TNextLayer, typename... TOptions>
class MsgSizeLayerBaseHelper
{
    using ParsedOptionsInternal = comms::protocol::details::MsgSizeLayerOptionsParser<TOptions...>;
    using ActualLayer = typename ParsedOptionsInternal::template DefineExtendingClass<MsgSizeLayer<TField, TNextLayer, TOptions...>>;
    using TopBase = 
        comms::protocol::ProtocolLayerBase<
            TField,
            TNextLayer,
            ActualLayer,
            comms::option::ProtocolLayerDisallowReadUntilDataSplit
        >;

public:
    using Base = TopBase;
};

template <typename TField, typename TNextLayer, typename... TOptions>
using MsgSizeLayerBase = typename MsgSizeLayerBaseHelper<TField, TNextLayer, TOptions...>::Base;

} // namespace details

} // namespace protocol

} // namespace comms
