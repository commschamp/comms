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
class SyncSuffixLayerOptionsParser;

template <>
class SyncSuffixLayerOptionsParser<>
{
public:
    static constexpr bool HasVerifyBeforeRead = false;
    static constexpr bool HasExtendingClass = false;
    static constexpr bool HasSeekField = false;

    using ExtendingClass = void;

    using EscField = void;

    template <typename TLayer>
    using DefineExtendingClass = TLayer;
};

template <typename... TOptions>
class SyncSuffixLayerOptionsParser<comms::option::def::FrameLayerVerifyBeforeRead, TOptions...> :
        public SyncSuffixLayerOptionsParser<TOptions...>
{
public:
    static constexpr bool HasVerifyBeforeRead = true;
};

template <typename T, typename... TOptions>
class SyncSuffixLayerOptionsParser<comms::option::def::ExtendingClass<T>, TOptions...> :
        public SyncSuffixLayerOptionsParser<TOptions...>
{
public:
    static constexpr bool HasExtendingClass = true;
    using ExtendingClass = T;

    template <typename TLayer>
    using DefineExtendingClass = ExtendingClass;
};

template <typename TEscField, typename... TOptions>
class SyncSuffixLayerOptionsParser<comms::option::def::FrameLayerSeekField<TEscField>, TOptions...> :
        public SyncSuffixLayerOptionsParser<TOptions...>
{
public:
    using EscField = TEscField;
    static constexpr bool HasSeekField = true;
};

template <typename... TOptions>
class SyncSuffixLayerOptionsParser<
    comms::option::app::EmptyOption,
    TOptions...> : public SyncSuffixLayerOptionsParser<TOptions...>
{
};

template <typename... TBundledOptions, typename... TOptions>
class SyncSuffixLayerOptionsParser<
    std::tuple<TBundledOptions...>,
    TOptions...> : public SyncSuffixLayerOptionsParser<TBundledOptions..., TOptions...>
{
};

} // namespace details

} // namespace frame

} // namespace comms
