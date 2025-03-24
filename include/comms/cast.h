//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of various casts

#pragma once

#include "comms/details/FieldCastHelper.h"
#include "comms/details/ValueAssignWrapper.h"

namespace comms
{


/// @brief Helper function to assign value with static_cast to
///     appropriate type.
/// @details Can be used to assign any value, including value of a field.
///     @code
///     auto someValue = ...;
///     comms::cast_assign(field.value()) = someValue; // Sets field's value with appropriate static_cast.
///     @endcode
/// @note Defined in "comms/cast.h" headerfile
template <typename T>
details::ValueAssignWrapper<T> cast_assign(T& value)
{
    return details::ValueAssignWrapper<T>(value);
}

/// @brief Cast between fields.
/// @details Sometimes the protocol may treat some specific field differently
///     based on indication bit in some other field. This function can be
///     used to cast one field to another type.
///     @code
///     SomeFieldType someField = comms::field_cast<SomeFieldType>(someOtherField);
///     @endcode
///     The function will perform some compile time analysis and will do a simple
///     static_cast between the contained values if they are convertible. Otherwise
///     the write + read operations will be performed, i.e. the source field will 
///     be written into a temporary buffer, and the target field will perform a 
///     read operation from that buffer.
/// @tparam TFieldTo Type to cast to.
/// @tparam TFieldFrom Type to cast from.
/// @param[in] field Original field
/// @return Field of the new type with internal value equivalent to one of the
///     original field.
/// @note Defined in "comms/cast.h" headerfile
template <typename TFieldTo, typename TFieldFrom>
TFieldTo field_cast(const TFieldFrom& field)
{
    return details::FieldCastHelper<>::template cast<TFieldTo, TFieldFrom>(field);
}


} // namespace comms
