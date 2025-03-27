//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/options.h"

namespace comms
{

namespace frame
{

namespace details
{


template <typename... TOptions>
class FrameLayerBaseOptionsParser;

template <>
class FrameLayerBaseOptionsParser<>
{
public:
    static constexpr bool HasForceReadUntilDataSplit = false;
    static constexpr bool HasDisallowReadUntilDataSplit = false;
};

template <typename... TOptions>
class FrameLayerBaseOptionsParser<
    comms::option::def::FrameLayerForceReadUntilDataSplit, TOptions...> :
        public FrameLayerBaseOptionsParser<TOptions...>
{
public:
    static constexpr bool HasForceReadUntilDataSplit = true;
};

template <typename... TOptions>
class FrameLayerBaseOptionsParser<
    comms::option::def::FrameLayerDisallowReadUntilDataSplit, TOptions...> :
        public FrameLayerBaseOptionsParser<TOptions...>
{
public:
    static constexpr bool HasDisallowReadUntilDataSplit = true;
};

template <typename... TOptions>
class FrameLayerBaseOptionsParser<
    comms::option::app::EmptyOption,
    TOptions...> : public FrameLayerBaseOptionsParser<TOptions...>
{
};

template <typename... TBundledOptions, typename... TOptions>
class FrameLayerBaseOptionsParser<
    std::tuple<TBundledOptions...>,
    TOptions...> : public FrameLayerBaseOptionsParser<TBundledOptions..., TOptions...>
{
};

} // namespace details

} // namespace frame

} // namespace comms
