//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/options.h"
#include "comms/frame/details/TransportValueLayerAllBases.h"

namespace comms
{

namespace frame
{

namespace details
{

template <typename... TOptions>
class TransportValueLayerOptionsParser;

template <>
class TransportValueLayerOptionsParser<>
{
public:
    static const bool HasPseudoValue = false;
    static constexpr bool HasExtendingClass = false;

    using ExtendingClass = void;

    template <typename TBase>
    using BuildPseudoBase = TBase;

    template <typename TLayer>
    using DefineExtendingClass = TLayer;

    template <typename TNextLayer>
    using ForceReadUntilDataSplitIfNeeded = 
        typename comms::util::Conditional<
            std::is_void<typename FrameLayerMsgPtr<TNextLayer>::Type>::value
        >::template Type <
            comms::option::app::EmptyOption,
            comms::option::def::FrameLayerForceReadUntilDataSplit
            // If the MsgPtr is not defined, then the MsgIdLayer is 
            // probably an outer layer, as the result the message 
            // object is properly allocated when transport value
            // read operation is reached.
        >;
};

template <typename... TOptions>
class TransportValueLayerOptionsParser<comms::option::def::PseudoValue, TOptions...> :
        public TransportValueLayerOptionsParser<TOptions...>
{
public:
    static const bool HasPseudoValue = true;

    template <typename TBase>
    using BuildPseudoBase = TransportValueLayerPseudoBase<TBase>;    
};

template <typename T, typename... TOptions>
class TransportValueLayerOptionsParser<comms::option::def::ExtendingClass<T>, TOptions...> :
        public TransportValueLayerOptionsParser<TOptions...>
{
public:
    static constexpr bool HasExtendingClass = true;
    using ExtendingClass = T;

    template <typename TLayer>
    using DefineExtendingClass = ExtendingClass;    
};

template <typename... TOptions>
class TransportValueLayerOptionsParser<
    comms::option::def::FrameLayerSuppressReadUntilDataSplitForcing, 
    TOptions...> :
        public TransportValueLayerOptionsParser<TOptions...>
{
public:
    template <typename TNextLayer>
    using ForceReadUntilDataSplitIfNeeded = comms::option::app::EmptyOption;
};

template <typename... TOptions>
class TransportValueLayerOptionsParser<
    comms::option::app::EmptyOption,
    TOptions...> : public TransportValueLayerOptionsParser<TOptions...>
{
};

template <typename... TBundledOptions, typename... TOptions>
class TransportValueLayerOptionsParser<
    std::tuple<TBundledOptions...>,
    TOptions...> : public TransportValueLayerOptionsParser<TBundledOptions..., TOptions...>
{
};

} // namespace details

} // namespace frame

} // namespace comms
