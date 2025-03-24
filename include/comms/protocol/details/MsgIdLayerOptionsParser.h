//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/MsgFactory.h"
#include "comms/options.h"
#include "comms/util/type_traits.h"

#include <tuple>

namespace comms
{

namespace protocol
{

namespace details
{


template <typename... TOptions>
class MsgIdLayerOptionsParser;

template <>
class MsgIdLayerOptionsParser<>
{
public:
    static const bool HasExtendingClass = false;
    static const bool HasMsgFactory = false;

    using ExtendingClass = void;
    using FactoryOptions = std::tuple<>;

    template <typename TInterface, typename TAllMessages, typename... TExtraOptions>
    using MsgFactory = comms::MsgFactory<TInterface, TAllMessages, TExtraOptions...>;

    template <typename TLayer>
    using DefineExtendingClass = TLayer;    
};

template <typename T, typename... TOptions>
class MsgIdLayerOptionsParser<comms::option::def::ExtendingClass<T>, TOptions...> :
        public MsgIdLayerOptionsParser<TOptions...>
{
public:
    static const bool HasExtendingClass = true;
    using ExtendingClass = T;

    template <typename TLayer>
    using DefineExtendingClass = ExtendingClass;       
};

template <typename TFactory, typename... TOptions>
class MsgIdLayerOptionsParser<comms::option::app::MsgFactory<TFactory>, TOptions...> :
        public MsgIdLayerOptionsParser<TOptions...>
{
public:
    static const bool HasMsgFactory = true;

    template <typename TInterface, typename TAllMessages, typename... TExtraOptions>
    using MsgFactory = TFactory;
};

template <template<typename, typename, typename...> class TFactory, typename... TOptions>
class MsgIdLayerOptionsParser<comms::option::app::MsgFactoryTempl<TFactory>, TOptions...> :
        public MsgIdLayerOptionsParser<TOptions...>
{
    using BaseImpl = MsgIdLayerOptionsParser<TOptions...>;
public:

    static const bool HasMsgFactory = true;

    template <typename TInterface, typename TAllMessages, typename... TExtraOptions>
    using MsgFactory = TFactory<TInterface, TAllMessages, typename BaseImpl::FactoryOptions, TExtraOptions...>;
};


template <typename... TOptions>
class MsgIdLayerOptionsParser<
    comms::option::app::EmptyOption,
    TOptions...> : public MsgIdLayerOptionsParser<TOptions...>
{
};

template <typename... TBundledOptions, typename... TOptions>
class MsgIdLayerOptionsParser<
    std::tuple<TBundledOptions...>,
    TOptions...> : public MsgIdLayerOptionsParser<TBundledOptions..., TOptions...>
{
};

template <typename T, typename... TOptions>
class MsgIdLayerOptionsParser<T, TOptions...> : public MsgIdLayerOptionsParser<TOptions...>
{
    using BaseImpl = MsgIdLayerOptionsParser<TOptions...>;
public:    
    using FactoryOptions = 
        typename std::decay<
            decltype(
                std::tuple_cat(
                    std::declval<std::tuple<T> >(),
                    std::declval<typename BaseImpl::FactoryOptions>()
                )
            )
        >::type;

    // Ignoring all the options if MsgFactory is overriden
    template <typename TInterface, typename TAllMessages, typename... TExtraOptions>
    using MsgFactory = 
        typename comms::util::Conditional<
            BaseImpl::HasMsgFactory
        >::template Type<
            typename BaseImpl::template MsgFactory<TInterface, TAllMessages, FactoryOptions, TExtraOptions...>,
            comms::MsgFactory<TInterface, TAllMessages, FactoryOptions, TExtraOptions...>  
        >;
};

} // namespace details

} // namespace protocol

} // namespace comms
