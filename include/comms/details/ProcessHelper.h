//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Provides auxiliary functions for processing input and dispatching messages

#pragma once

#include "comms/Assert.h"
#include "comms/details/detect.h"
#include "comms/details/DispatchMsgPolymorphicHelper.h"
#include "comms/details/process.h"
#include "comms/details/tag.h"
#include "comms/dispatch.h"
#include "comms/ErrorStatus.h"
#include "comms/frame/FrameLayerBase.h"
#include "comms/iterator.h"
#include "comms/Message.h"
#include "comms/MsgDispatcher.h"
#include "comms/util/ScopeGuard.h"
#include "comms/util/type_traits.h"

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace  comms
{

namespace details
{
   
struct ProcessHelper
{

    template <typename TBufIter, typename TFrame, typename TMsg, typename... TExtraValues>
    static comms::ErrorStatus processSingle(
        TBufIter& bufIter,
        std::size_t len,
        TFrame&& frame,
        TMsg& msg,
        TExtraValues... extraValues)
    {
        std::size_t consumed = 0U;
        auto onExit =
            comms::util::makeScopeGuard(
                [&bufIter, &consumed]()
                {
                    std::advance(bufIter, consumed);
                });
        static_cast<void>(onExit);

        while (consumed < len) {
            auto begIter = comms::readIteratorFor(msg, bufIter + consumed);
            auto iter = begIter;

            // Do the read
            auto es = frame.read(msg, iter, len - consumed, extraValues...);
            if (es == comms::ErrorStatus::NotEnoughData) {
                return es;
            }

            if (es == comms::ErrorStatus::ProtocolError) {
                // Something is not right with the data, remove one character and try again
            ++consumed;
                continue;
            }

            consumed += static_cast<decltype(consumed)>(std::distance(begIter, iter));
            return es;
        }

        return comms::ErrorStatus::NotEnoughData;
    }

    template <typename TBufIter, typename TFrame, typename TMsg, typename THandler, typename... TExtraValues>
    static comms::ErrorStatus processSingleWithDispatch(
        TBufIter& bufIter,
        std::size_t len,
        TFrame&& frame,
        TMsg& msg,
        THandler& handler,
        TExtraValues... extraValues)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;
        using HandlerType = typename std::decay<decltype(handler)>::type;
        using Tag = DispatchTag<MsgType, HandlerType>;
        return processSingleWithDispatchInternal(bufIter, len, frame, msg, handler, extraValues..., Tag());
    }

    template <typename TDispatcher, typename TBufIter, typename TFrame, typename TMsg, typename THandler, typename... TExtraValues>
    static comms::ErrorStatus processSingleWithDispatchViaDispatcher(
        TBufIter& bufIter,
        std::size_t len,
        TFrame&& frame,
        TMsg& msg,
        THandler& handler,
        TExtraValues... extraValues)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;
        using FrameType = typename std::decay<decltype(frame)>::type;
        static_assert((!comms::isMessage<MsgType>()) || (details::allMessagesAreStrongSorted<typename FrameType::AllMessages>()),
            "Cannot dispatch with Dispatcher when using pre-allocated message, use regular polymorphic dispatch with "
            "comms::option::def::Handler option");

        using LocalMsgIdType = details::ProcessMsgIdType<MsgType>;
        static_assert(!std::is_void<LocalMsgIdType>(), "Invalid type of msg param");

        LocalMsgIdType id = LocalMsgIdType();
        std::size_t idx = 0U;

        auto es =
            processSingle(
                bufIter,
                len,
                std::forward<TFrame>(frame),
                msg,
                comms::frame::msgId(id),
                comms::frame::msgIndex(idx),
                extraValues...);

        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        using FrameType = typename std::decay<decltype(frame)>::type;
        using AllMessagesType = typename FrameType::AllMessages;
        static_assert(
            comms::isMsgDispatcher<TDispatcher>(),
            "TDispatcher is expected to be a variant of comms::MsgDispatcher");

        auto& msgObj = details::processMsgCastToMsgObj(msg);
        TDispatcher::template dispatch<AllMessagesType>(id, idx, msgObj, handler);
        return es;
    }

    template <typename TBufIter, typename TFrame, typename THandler, typename... TExtraValues>
    static std::size_t processAllWithDispatch(
        TBufIter bufIter,
        std::size_t len,
        TFrame&& frame,
        THandler& handler)
    {
        std::size_t consumed = 0U;
        using FrameType = typename std::decay<decltype(frame)>::type;
        using MsgPtr = typename FrameType::MsgPtr;
        while (consumed < len) {
            auto begIter = bufIter + consumed;
            auto iter = begIter;

            MsgPtr msg;
            auto es = processSingleWithDispatch(iter, len - consumed, std::forward<TFrame>(frame), msg, handler);
            consumed += static_cast<decltype(consumed)>(std::distance(begIter, iter));
            if (es == comms::ErrorStatus::NotEnoughData) {
                break;
            }
            COMMS_ASSERT(consumed <= len);
        }

        return consumed;
    }

    template <typename TDispatcher, typename TBufIter, typename TFrame, typename THandler, typename... TExtraValues>
    static std::size_t processAllWithDispatchViaDispatcher(
        TBufIter bufIter,
        std::size_t len,
        TFrame&& frame,
        THandler& handler)
    {
        std::size_t consumed = 0U;
        using FrameType = typename std::decay<decltype(frame)>::type;
        using MsgPtr = typename FrameType::MsgPtr;
        while (consumed < len) {
            auto begIter = bufIter + consumed;
            auto iter = begIter;

            MsgPtr msg;
            auto es = processSingleWithDispatchViaDispatcher<TDispatcher>(iter, len - consumed, std::forward<TFrame>(frame), msg, handler);
            consumed += static_cast<decltype(consumed)>(std::distance(begIter, iter));
            if (es == comms::ErrorStatus::NotEnoughData) {
                break;
            }
            COMMS_ASSERT(consumed <= len);
        }

        return consumed;
    }
private:
    template <typename... TParams>
    using PolymorphicDispatchTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using RegularDispatchTag = comms::details::tag::Tag2<>;

    template <typename TMsg, typename THandler, typename...>
    using DispatchTagTmp = 
        typename comms::util::LazyShallowConditional<
            details::dispatchMsgPolymorphicIsCompatibleHandler<TMsg, THandler>()
        >::template Type<
            PolymorphicDispatchTag,
            RegularDispatchTag
        >;     

    template <typename TMsg, typename THandler, typename...>
    using DispatchTag = 
        typename comms::util::LazyShallowConditional<
            comms::isMessage<TMsg>()
        >::template Type<
            DispatchTagTmp,
            RegularDispatchTag,
            TMsg, THandler
        >;    

        template <typename TBufIter, typename TFrame, typename TMsg, typename THandler, typename... TExtraValues>
        static comms::ErrorStatus processSingleWithDispatchInternal(
            TBufIter& bufIter,
            std::size_t len,
            TFrame&& frame,
            TMsg& msg,
            THandler& handler,
            TExtraValues... extraValues,
            RegularDispatchTag<>)
        {
            using MsgType = typename std::decay<decltype(msg)>::type;
            using FrameType = typename std::decay<decltype(frame)>::type;
            static_assert((!comms::isMessage<MsgType>()) || (details::allMessagesAreStrongSorted<typename FrameType::AllMessages>()),
                "Cannot process pre-allocated message, impossible to retrieve message index for proper dispatch, "
                "use comms::option::def::Handler option to support polymorphic dispatch");

            using LocalMsgIdType = details::ProcessMsgIdType<MsgType>;
            LocalMsgIdType id = LocalMsgIdType();
            std::size_t idx = 0U;
    
            auto es =
                processSingle(
                    bufIter,
                    len,
                    std::forward<TFrame>(frame),
                    msg,
                    comms::frame::msgId(id),
                    comms::frame::msgIndex(idx),
                    extraValues...);
    
            if (es != comms::ErrorStatus::Success) {
                return es;
            }
    
            using FrameType = typename std::decay<decltype(frame)>::type;
            using AllMessagesType = typename FrameType::AllMessages;
            auto& msgObj = details::processMsgCastToMsgObj(msg);
            comms::dispatchMsg<AllMessagesType>(id, idx, msgObj, handler);
            return es;
        }

        template <typename TBufIter, typename TFrame, typename TMsg, typename THandler, typename... TExtraValues>
        static comms::ErrorStatus processSingleWithDispatchInternal(
            TBufIter& bufIter,
            std::size_t len,
            TFrame&& frame,
            TMsg& msg,
            THandler& handler,
            TExtraValues... extraValues,
            PolymorphicDispatchTag<>)
        {
            auto es =
                processSingle(
                    bufIter,
                    len,
                    std::forward<TFrame>(frame),
                    msg,
                    extraValues...);
    
            if (es != comms::ErrorStatus::Success) {
                return es;
            }

            auto& msgObj = details::processMsgCastToMsgObj(msg);
            msgObj.dispatch(handler);
            return es;
        }        
    
};

} // namespace details

} // namespace  comms
