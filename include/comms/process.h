//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Provides auxiliary functions for processing input and dispatching messages

#pragma once

#include "comms/details/ProcessHelper.h"

#include <cstddef>
#include <utility>

namespace  comms
{

/// @brief Process input until first message is recognized and its object is created
///     or missing data is reported.
/// @details Can be used to implement @ref page_use_prot_transport_read.
/// @param[in, out] bufIter Iterator to input buffer. Passed by reference and is updated
///     when buffer is iterated over. Number of consumed bytes cat be determined by
///     calculating the distance between originally passed value and the one after
///     function returns.
/// @param[in] len Number of remaining bytes in input buffer.
/// @param[in] frame Protocol frame (see @ref page_use_prot_transport) that
///     is used to process the raw input.
/// @param[in, out] msg Smart pointer (see @ref comms::frame::FrameLayerBase::MsgPtr "MsgPtr"
///     defintion of the @ref page_use_prot_transport) to message object to be allocated,
///     or reference to actual message object (extending @ref comms::Message) when
///     such is known.
/// @param[in, out] extraValues Extra values that are passed as variadic parameters to
///     @ref comms::frame::FrameLayerBase::read() "read()" member function
///     of the protocol frame.
/// @return ErrorStatus of the protocol frame @ref comms::frame::FrameLayerBase::read() "read()"
///     operation.
/// @note Defined in comms/process.h
/// @see @ref page_dispatch
template <typename TBufIter, typename TFrame, typename TMsg, typename... TExtraValues>
comms::ErrorStatus processSingle(
    TBufIter& bufIter,
    std::size_t len,
    TFrame&& frame,
    TMsg& msg,
    TExtraValues... extraValues)
{
    return details::ProcessHelper::processSingle(bufIter, len, std::forward<TFrame>(frame), msg, std::forward<TExtraValues>(extraValues)...);
}

/// @brief Process input until first message is recognized, its object is created
///     and dispatched to appropriate handling function, or missing data is reported.
/// @details Similar to @ref comms::processSingle(), but adds dispatch stage.
///     Can be used to implement @ref page_use_prot_transport_read.
/// @param[in, out] bufIter Iterator to input buffer. Passed by reference and is updated
///     when buffer is iterated over. Number of consumed bytes cat be determined by
///     calculating the distance between originally passed value and the one after
///     function returns.
/// @param[in] len Number of remaining bytes in input buffer.
/// @param[in] frame Protocol frame (see @ref page_use_prot_transport) that
///     is used to process the raw input.
/// @param[in, out] msg Smart pointer (see @ref comms::frame::FrameLayerBase::MsgPtr "MsgPtr"
///     defintion of the @ref page_use_prot_transport) to message object to be allocated,
///     or reference to actual message object (extending @ref comms::Message) when
///     such is known.
/// @param[in] handler Handler to handle message object when dispatched. The dispatch
///     is performed using @ref comms::dispatchMsg() function.
/// @param[in, out] extraValues Extra values that are passed as variadic parameters to
///     @ref comms::frame::FrameLayerBase::read() "read()" member function
///     of the protocol frame.
/// @return ErrorStatus of the protocol frame @ref comms::frame::FrameLayerBase::read() "read()"
///     operation.
/// @note Defined in comms/process.h
/// @note If default dispatch behaviour of the @ref comms::dispatchMsg()
///     function doesn't suit the application needs, consider using
///     @ref comms::processSingleWithDispatchViaDispatcher() instead.
/// @see @ref page_dispatch
template <typename TBufIter, typename TFrame, typename TMsg, typename THandler, typename... TExtraValues>
comms::ErrorStatus processSingleWithDispatch(
    TBufIter& bufIter,
    std::size_t len,
    TFrame&& frame,
    TMsg& msg,
    THandler& handler,
    TExtraValues... extraValues)
{
    return details::ProcessHelper::processSingleWithDispatch(bufIter, len, std::forward<TFrame>(frame), msg, handler, std::forward<TExtraValues>(extraValues)...);
}

/// @brief Process input until first message is recognized, its object is created
///     and dispatched to appropriate handling function, or missing data is reported.
/// @details Similar to @ref comms::processSingleWithDispatch(), but allows forcing
///     a particular dispatch policy.
/// @tparam TDispatcher A variant of @ref comms::MsgDispatcher class. It's going
///     to be used to dispatch message object into appropriate handling function
///     instead of using @ref comms::dispatchMsg() like @ref comms::processSingleWithDispatch()
///     does.
/// @param[in, out] bufIter Iterator to input buffer. Passed by reference and is updated
///     when buffer is iterated over. Number of consumed bytes cat be determined by
///     calculating the distance between originally passed value and the one after
///     function returns.
/// @param[in] len Number of remaining bytes in input buffer.
/// @param[in] frame Protocol frame (see @ref page_use_prot_transport) that
///     is used to process the raw input.
/// @param[in, out] msg Smart pointer (see @ref comms::frame::FrameLayerBase::MsgPtr "MsgPtr"
///     defintion of the @ref page_use_prot_transport) to message object to be allocated,
///     or reference to actual message object (extending @ref comms::Message) when
///     such is known.
/// @param[in] handler Handler to handle message object when dispatched. The dispatch
///     is performed via provded @b TDispatcher class (see @ref comms::MsgDispatcher).
/// @param[in, out] extraValues Extra values that are passed as variadic parameters to
///     @ref comms::frame::FrameLayerBase::read() "read()" member function
///     of the protocol frame.
/// @return ErrorStatus of the protocol frame @ref comms::frame::FrameLayerBase::read() "read()"
///     operation.
/// @note Defined in comms/process.h
/// @see @ref comms::processSingleWithDispatch().
/// @see @ref page_dispatch
/// @see @ref page_use_prot_transport_read
template <typename TDispatcher, typename TBufIter, typename TFrame, typename TMsg, typename THandler, typename... TExtraValues>
comms::ErrorStatus processSingleWithDispatchViaDispatcher(
    TBufIter& bufIter,
    std::size_t len,
    TFrame&& frame,
    TMsg& msg,
    THandler& handler,
    TExtraValues... extraValues)
{
    return details::ProcessHelper::processSingleWithDispatchViaDispatcher<TDispatcher>(bufIter, len, std::forward<TFrame>(frame), msg, handler, std::forward<TExtraValues>(extraValues)...);
}

/// @brief Process all available input and dispatch all created message objects
///     to appropriate handling function.
/// @details All the created message objects are immediatelly destructed after
///     dispatching.
/// @param[in, out] bufIter Iterator to input buffer. Passed by value and is @b NOT updated
///     when buffer is iterated over (unlike @ref comms::processSingle(),
///     @ref comms::processSingleWithDispatch(), @ref comms::processSingleWithDispatchViaDispatcher()).
/// @param[in] len Number of remaining bytes in input buffer.
/// @param[in] frame Protocol framek (see @ref page_use_prot_transport) that
///     is used to process the raw input.
/// @param[in] handler Handler to handle message object when dispatched. The dispatch
///     is performed using @ref comms::dispatchMsg() function.
/// @return Number of consumed bytes from the buffer. The caller is responsible to
///     remove them from the buffer.
/// @note Defined in comms/process.h
/// @note If default dispatch behaviour of the @ref comms::dispatchMsg()
///     function doesn't suit the application needs, consider using
///     @ref comms::processAllWithDispatchViaDispatcher() instead.
/// @see @ref page_dispatch
/// @see @ref comms::processAllWithDispatchViaDispatcher().
/// @see @ref page_dispatch
/// @see @ref page_use_prot_transport_read
template <typename TBufIter, typename TFrame, typename THandler, typename... TExtraValues>
std::size_t processAllWithDispatch(
    TBufIter bufIter,
    std::size_t len,
    TFrame&& frame,
    THandler& handler)
{
    return details::ProcessHelper::processAllWithDispatch(bufIter, len, std::forward<TFrame>(frame), handler);
}

/// @brief Process all available input and dispatch all created message objects
///     to appropriate handling function.
/// @details Similar to @ref comms::processAllWithDispatch(), but allows forcing
///     a particular dispatch policy. All the created message objects are
///     immediatelly destructed after dispatching.
/// @tparam TDispatcher A variant of @ref comms::MsgDispatcher class. It's going
///     to be used to dispatch message object into appropriate handling function
///     instead of using @ref comms::dispatchMsg() like @ref comms::processSingleWithDispatch()
///     does.
/// @param[in, out] bufIter Iterator to input buffer. Passed by value and is @b NOT updated
///     when buffer is iterated over (unlike @ref comms::processSingle(),
///     @ref comms::processSingleWithDispatch(), @ref comms::processSingleWithDispatchViaDispatcher()).
/// @param[in] len Number of remaining bytes in input buffer.
/// @param[in] frame Protocol frame (see @ref page_use_prot_transport) that
///     is used to process the raw input.
/// @param[in] handler Handler to handle message object when dispatched. The dispatch
///     is performed via provded @b TDispatcher class (see @ref comms::MsgDispatcher).
/// @return Number of consumed bytes from the buffer. The caller is responsible to
///     remove them from the buffer.
/// @note Defined in comms/process.h
/// @see @ref comms::processAllWithDispatch().
/// @see @ref page_dispatch
/// @see @ref page_use_prot_transport_read
template <typename TDispatcher, typename TBufIter, typename TFrame, typename THandler, typename... TExtraValues>
std::size_t processAllWithDispatchViaDispatcher(
    TBufIter bufIter,
    std::size_t len,
    TFrame&& frame,
    THandler& handler)
{
    return details::ProcessHelper::processAllWithDispatchViaDispatcher<TDispatcher>(bufIter, len, std::forward<TFrame>(frame), handler);
}

} // namespace  comms
