//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Field.h"
#include "comms/field/ArrayList.h"
#include "comms/options.h"

namespace comms
{

namespace protocol
{

namespace details
{

class MsgDataLayerField : public
    comms::field::ArrayList<
        comms::Field<comms::option::def::BigEndian>,
        std::uint8_t,
        comms::option::HasName
    >
{
public:
    static const char* name()    
    {
        return "Data";
    }
};

template <typename... TOptions>
class MsgDataLayerOptionsParser;

template <>
class MsgDataLayerOptionsParser<>
{
public:
    static constexpr bool HasFieldType = false;
    using FieldType = MsgDataLayerField;
};

template <typename TField, typename... TOptions>
class MsgDataLayerOptionsParser<comms::option::def::FieldType<TField>, TOptions...> :
        public MsgDataLayerOptionsParser<TOptions...>
{
public:
    static constexpr bool HasFieldType = true;
    using FieldType = TField; 
};

template <typename... TOptions>
class MsgDataLayerOptionsParser<
    comms::option::app::EmptyOption,
    TOptions...> : public MsgDataLayerOptionsParser<TOptions...>
{
};

template <typename... TBundledOptions, typename... TOptions>
class MsgDataLayerOptionsParser<
    std::tuple<TBundledOptions...>,
    TOptions...> : public MsgDataLayerOptionsParser<TBundledOptions..., TOptions...>
{
};

} // namespace details

} // namespace protocol

} // namespace comms
