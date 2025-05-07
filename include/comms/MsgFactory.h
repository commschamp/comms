//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of comms::MsgFactory class.

#pragma once

#include "comms/Assert.h"
#include "comms/details/MsgFactoryBase.h"
#include "comms/details/MsgFactoryOptionsParser.h"
#include "comms/MsgFactoryCreateFailureReason.h"
#include "comms/util/Tuple.h"
#include "comms/util/alloc.h"

#include <algorithm>
#include <type_traits>

namespace comms
{

/// @brief Message factory class.
/// @details It is responsible to create message objects given the ID of the
///     message. This class @b DOESN'T use dynamic memory allocation to store its
///     internal data structures, hence can be used in any bare-metal and other
///     embedded environment.@n
///     The types of all messages provided in @b TAllMessages are analysed at
///     compile time and best "id to message object" mapping strategy is chosen,
///     unless the "dispatch" type forcing options are used (see description below).
/// @tparam TMsgBase Common base class for all the messages, smart pointer to
///     this type is returned when allocation of specify message is requested.
/// @tparam TAllMessages All custom message types, that this factory is capable
///     of creating, bundled in std::tuple<>. The message types must be sorted
///     based on their IDs. Different variants of the same message (reporting
///     same ID, but implemented as different classes) are also supported. However
///     they must follow one another in this std::tuple, i.e. be sorted.
/// @tparam TOptions Zero or more options. The supported options are:
///     @li @ref comms::option::app::InPlaceAllocation - Option to specify that custom
///         message objects are @b NOT allocated using dynamic memory, instead
///         an uninitialised area of memory in private members is used to contain
///         any type of custom message (provided with TAllMessages template parameter) and
///         placement "new" operator is used to initialise requested message in
///         this area.
///         The allocated message objects are returned from createMsg() function
///         wrapped in the smart pointer (variant of std::unique_ptr). If
///         @ref comms::option::app::InPlaceAllocation option is used, then the smart pointer
///         definition contains custom deleter, which will explicitly invoke
///         destructor of the message when the smart pointer is out of scope. It
///         means that it is @b NOT possible to create new message with this factory
///         if previously allocated one wasn't destructed yet.
///         If @ref comms::option::app::InPlaceAllocation option is NOT used, than the
///         requested message objects are allocated using dynamic memory and
///         returned wrapped in std::unique_ptr without custom deleter.
///     @li @ref comms::option::app::SupportGenericMessage - Option used to allow
///         allocation of @ref comms::GenericMessage. If such option is
///         provided, the createGenericMsg() member function will be able
///         to allocate @ref comms::GenericMessage object. @b NOTE, that
///         the base class of @ref comms::GenericMessage type (first template
///         parameter) must be equal to @b TMsgBase (first template parameter)
///         of @b this class.
///     @li @ref comms::option::app::ForceDispatchPolymorphic,
///         @ref comms::option::app::ForceDispatchStaticBinSearch, or
///         @ref comms::option::app::ForceDispatchLinearSwitch - Force a particular
///         dispatch way when creating message object given the numeric ID
///         (see @ref comms::MsgFactory::createMsg()). The dispatch methods
///         are properly described in @ref page_dispatch tutorial page.
///         If none of these options are provided, then the MsgFactory
///         used a default way, which is equivalent to calling 
///         @ref comms::dispatchMsgType() (see also @ref page_dispatch_message_type_default).
///         To inquire what actual dispatch type is used, please use one
///         of the following constexpr member functions: 
///         @ref comms::MsgFactory::isDispatchPolymorphic(),
///         @ref comms::MsgFactory::isDispatchStaticBinSearch(), and
///         @ref comms::MsgFactory::isDispatchLinearSwitch()
/// @pre TMsgBase is a base class for all the messages in TAllMessages.
/// @pre Message type is TAllMessages must be sorted based on their IDs.
/// @pre If @ref comms::option::app::InPlaceAllocation option is provided, only one custom
///     message can be allocated. The next one can be allocated only after previous
///     message has been destructed.
/// @headerfile comms/MsgFactory.h
template <typename TMsgBase, typename TAllMessages, typename... TOptions>
class MsgFactory : private details::MsgFactoryBase<TMsgBase, TAllMessages, TOptions...>
{
    using Base = details::MsgFactoryBase<TMsgBase, TAllMessages, TOptions...>;
    static_assert(TMsgBase::hasMsgIdType(),
        "Usage of MsgFactory requires Message interface to provide ID type. "
        "Use comms::option::def::MsgIdType option in message interface type definition.");

public:
    /// @brief Parsed options
    using ParsedOptions = typename Base::ParsedOptions;

    /// @brief Type of the common base class of all the messages.
    using Message = typename Base::Message;

    /// @brief Type of the message ID when passed as a parameter.
    using MsgIdParamType = typename Base::MsgIdParamType;

    /// @brief Type of the message ID.
    using MsgIdType = typename Base::MsgIdType;

    /// @brief Smart pointer to @ref Message which holds allocated message object.
    /// @details It is a variant of std::unique_ptr, based on whether
    ///     @ref comms::option::app::InPlaceAllocation option was used.
    using MsgPtr = typename Base::MsgPtr;

    /// @brief All messages provided as template parameter to this class.
    using AllMessages = typename Base::AllMessages;

    /// @brief Reason for message creation failure
    using CreateFailureReason = MsgFactoryCreateFailureReason;

    /// @brief type of generic message.
    /// @details If supported a variant of @ref comms::GenericMessage, @b void otherwise
    using GenericMessage = typename ParsedOptions::GenericMessage;

    /// @brief Create message object given the ID of the message.
    /// @details The id to mapping is performed using the chosen (or default) 
    ///     @b dispatch policy described in the class options.
    /// @param id ID of the message.
    /// @param idx Relative index (or offset) of the message with the same ID. In case
    ///     protocol implementation contains multiple distinct message types
    ///     that report same ID value, it must be possible to choose the
    ///     relative index of such message from the first message type reporting
    ///     the same ID. This parameter provides such an ability. However,
    ///     most protocols will implement single message class for single ID.
    ///     For such implementations, use default value of this parameter.
    /// @param[out] reason Failure reason in case creation has failed. May be nullptr.
    /// @return Smart pointer (variant of std::unique_ptr) to @ref Message type,
    ///     which is a common base class of all the messages (provided as
    ///     first template parameter to this class). If @ref comms::option::app::InPlaceAllocation
    ///     option was used and previously allocated message wasn't de-allocated
    ///     yet, the empty (null) pointer will be returned.
    MsgPtr createMsg(MsgIdParamType id, unsigned idx = 0U, CreateFailureReason* reason = nullptr) const
    {
        return Base::createMsg(id, idx, reason);
    }

    /// @brief Allocate and initialise @ref comms::GenericMessage object.
    /// @details If @ref comms::option::app::SupportGenericMessage option hasn't been
    ///     provided, this function will return empty @b MsgPtr pointer. Otherwise
    ///     the relevant allocator will be used to allocate @ref comms::GenericMessage.
    /// @param[in] id ID of the message, will be passed as a parameter to the
    ///     constructor of the @ref comms::GenericMessage class
    /// @param idx Relative index (or offset) of the message with the same ID. In case
    ///     protocol implementation contains multiple distinct message types
    ///     that report same ID value, it must be possible to choose the
    ///     relative index of such message from the first message type reporting
    ///     the same ID. This parameter provides such an ability. However,
    ///     most protocols will implement single message class for single ID.
    MsgPtr createGenericMsg(MsgIdParamType id, unsigned idx = 0U) const
    {
        return Base::createGenericMsg(id, idx);
    }

    /// @brief Inquiry whether allocation is possible
    bool canAllocate() const
    {
        return Base::canAllocate();
    }

    /// @brief Get number of message types from @ref AllMessages, that have the specified ID.
    /// @param id ID of the message.
    /// @return Number of message classes that report same ID.
    std::size_t msgCount(MsgIdParamType id) const
    {
        return Base::msgCount(id);
    }

    /// @brief Compile time inquiry whether all the message classes in the
    ///     @b TAllMessages bundle have unique IDs.
    static constexpr bool hasUniqueIds()
    {
        return Base::hasUniqueIds();
    }

    /// @brief Compile time inquiry whether polymorphic dispatch tables are 
    ///     generated internally to map message ID to actual type.
    /// @see @ref page_dispatch
    /// @see @ref comms::MsgFactory::isDispatchStaticBinSearch()
    /// @see @ref comms::MsgFactory::isDispatchLinearSwitch()
    static constexpr bool isDispatchPolymorphic()
    {
        return Base::isDispatchPolymorphic();
    }

    /// @brief Compile time inquiry whether static binary search dispatch is 
    ///     generated internally to map message ID to actual type.
    /// @see @ref page_dispatch
    /// @see @ref comms::MsgFactory::isDispatchPolymorphic()
    /// @see @ref comms::MsgFactory::isDispatchLinearSwitch()
    static constexpr bool isDispatchStaticBinSearch()
    {
        return Base::isDispatchStaticBinSearch();
    }

    /// @brief Compile time inquiry whether linear switch dispatch is 
    ///     generated internally to map message ID to actual type.
    /// @see @ref page_dispatch
    /// @see @ref comms::MsgFactory::isDispatchStaticBinSearch()
    /// @see @ref comms::MsgFactory::isDispatchLinearSwitch()
    static constexpr bool isDispatchLinearSwitch()
    {
        return Base::isDispatchLinearSwitch();
    }

    /// @brief Compile time inquiry whether factory supports in-place allocation
    /// @return @b true in case of in-place allocation, @b false in case of dynamic memory use.
    static constexpr bool hasInPlaceAllocation()
    {
        return ParsedOptions::HasInPlaceAllocation;
    }

    /// @brief Compile time inquiry whether factory supports @ref comms::GenericMessage allocation
    static constexpr bool hasGenericMessageSupport()
    {
        return ParsedOptions::HasSupportGenericMessage;
    }

    /// @brief Compile time inquiry whether factory has forced dispatch method
    static constexpr bool hasForcedDispatch()
    {
        return ParsedOptions::HasForcedDispatch;
    }    
};


}  // namespace comms

