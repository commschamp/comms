//
// Copyright 2014 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Provides common base class for the custom messages with default implementation.

#pragma once

#include "comms/details/detect.h"
#include "comms/details/field_alias.h"
#include "comms/details/fields_access.h"
#include "comms/details/macro_common.h"
#include "comms/details/MessageImplBuilder.h"

namespace comms
{

/// @brief Base class for all the custom protocol messages.
/// @details The main purpose of this class is to provide default implementation
///     for some pure virtual functions defined in @ref Message class. Just
///     like with @ref Message class, the provided methods implementation
///     depends on the options passed as TOption template parameter.
/// @tparam TMessage The main interface class of the custom protocol messages.
///     It may be either @ref Message class itself or any other class that
///     extends @ref Message. The @ref MessageBase inherits from class provided
///     as TMessage template parameter. As the result the real inheritance
///     diagram will look like: comms::Message <-- TMessage <-- comms::MessageBase.
/// @tparam TOptions Variadic template parameter that can include zero or more
///     options that specify behaviour. The options may be comma separated as well as
///     bundled into std::tuple. Supported options are:
///     @li @ref comms::option::def::StaticNumIdImpl - In case message have numeric IDs
///         (comms::Message::MsgIdType is of integral or enum type), usage of
///         this option will cause this class to implement getIdImpl() virtual
///         function that returns provided numeric value.
///     @li @ref comms::option::def::NoIdImpl - Some message may not have valid IDs and
///         their getId() function is never going to be called. Usage of this
///         option will create dummy implementation of getIdImpl() virtual
///         function that contains always failing assertion. In DEBUG mode
///         compilation the application will crash while in release mode the
///         default constructed value of comms::Message::MsgIdType will be returned.
///     @li @ref comms::option::def::MsgType - Provide type of actual message that
///         inherits from this comms::MessageBase class.
///     @li @ref comms::option::def::FieldsImpl - Usually implementation of read, write,
///         validity check, and length calculation is pretty straight forward. For
///         example the message is considered valid if all the field values
///         are considered to be valid, or read operation is to perform read for
///         all the fields in the message. If the @ref comms::option::def::FieldsImpl
///         option with all the message field classes bundled into
///         the std::tuple is provided, then @ref MessageBase class can implement
///         readImpl(), writeImpl(), validImpl(), lengthImpl() virtual functions
///         declared as pure in comms::Message interface. The option also
///         provides an accessor functions to the all the field objects: fields().
///     @li @ref comms::option::def::ZeroFieldsImpl - This option is an alias to
///         @ref comms::option::def::FieldsImpl<std::tuple<> >, which provides implementation
///         readImpl(), writeImpl(), validImpl(), lengthImpl() virtual functions
///         when message contains no fields, i.e. readImpl() and writeImple() will
///         always report success doing nothing, validImpl() will always return
///         true, and lengthImpl() will always return 0.
///     @li @ref comms::option::def::HasCustomRefresh - Notify @ref comms::MessageBase that
///             there is custom doRefresh() member function in the message definition
///             class.
///     @li @ref comms::option::def::HasDoGetId - Enable implementation of getIdImpl() even if
///         @ref comms::option::def::StaticNumIdImpl option wasn't used. Must be paired with
///         @ref comms::option::def::MsgType.
///     @li @ref comms::option::def::HasName - Notify @ref comms::MessageBase that
///             there is custom doName() member function in the message definition class.
///     @li @ref comms::option::def::FailOnInvalid - Fail the read operation if the contents are invalid.
///     @li @ref comms::option::app::NoReadImpl - Inhibit the implementation of readImpl().
///     @li @ref comms::option::app::NoWriteImpl - Inhibit the implementation of writeImpl().
///     @li @ref comms::option::app::NoLengthImpl - Inhibit the implementation of lengthImpl().
///     @li @ref comms::option::app::NoValidImpl - Inhibit the implementation of validImpl().
///     @li @ref comms::option::app::NoDispatchImpl - Inhibit the implementation of dispatchImpl().
/// @extends Message
/// @headerfile comms/MessageBase.h
/// @see @ref toMessageBase()
/// @see #COMMS_MSG_FIELDS_NAMES()
template <typename TMessage, typename... TOptions>
class MessageBase : public details::MessageImplBuilderT<TMessage, TOptions...>
{
    using BaseImpl = details::MessageImplBuilderT<TMessage, TOptions...>;
public:
    /// @brief All the options provided to this class bundled into struct.
    /// @details For internal use only
    using ImplOptions = details::MessageImplOptionsParser<TOptions...>;

    /// @brief Type of the actual message provided via 
    ///     @ref comms::option::def::MsgType.
    /// @details If @ref comms::option::def::MsgType hasn't been used
    ///     equals to @b void.
    using MsgType = typename ImplOptions::MsgType;

    /// @brief Compile type inquiry whether static numeric id has been provided via
    ///     @ref comms::option::def::StaticNumIdImpl.
    static constexpr bool hasStaticMsgId()
    {
        return ImplOptions::HasStaticMsgId;
    }    

    /// @brief Compile time retrieval of the message id provided via 
    ///    @ref comms::option::def::StaticNumIdImpl
    /// @details If comms::option::def::StaticNumIdImpl hasn't been used,
    ///     @b std::numeric_limits<std::intmax_t>::max() is returned.
    static constexpr std::intmax_t staticMsgId()
    {
        return ImplOptions::MsgId;
    }

    /// @brief Compile type inquiry whether fields have been provided via
    ///     @ref comms::option::def::FieldsImpl.
    static constexpr bool hasFields()
    {
        return ImplOptions::HasFieldsImpl;
    }

    /// @brief Compile time inquiry of whether fail on invalid has been requested
    ///     @ref comms::option::def::FailOnInvalid option.
    static constexpr bool hasFailOnInvalid()
    {
        return ImplOptions::HasFailOnInvalid;
    }

    /// @brief Compile time inquiry of whether the actual message type has
    ///     been provided via @ref comms::option::def::MsgType.
    static constexpr bool hasMsgType()
    {
        return ImplOptions::HasMsgType;
    }

    /// @brief Compile time inquiry of whether polymoriphic read has been
    ///     requested via interface options and hasn't been inhibited by
    ///     the @ref comms::option::app::NoReadImpl.
    static constexpr bool hasPolymorphicRead()
    {
        return BaseImpl::hasRead() && (!ImplOptions::HasNoReadImpl);
    }

    /// @brief Compile time inquiry of whether polymoriphic write has been
    ///     requested via interface options and hasn't been inhibited by
    ///     the @ref comms::option::app::NoWriteImpl.
    static constexpr bool hasPolymorphicWrite()
    {
        return BaseImpl::hasWrite() && (!ImplOptions::HasNoWriteImpl);
    }    

    /// @brief Compile time inquiry of whether polymoriphic validity check has been
    ///     requested via interface options and hasn't been inhibited by
    ///     the @ref comms::option::app::NoValidImpl.
    static constexpr bool hasPolymorphicValid()
    {
        return BaseImpl::hasValid() && (!ImplOptions::HasNoValidImpl);
    }

    /// @brief Compile time inquiry of whether polymoriphic length has been
    ///     requested via interface options and hasn't been inhibited by
    ///     the @ref comms::option::app::NoLengthImpl.
    static constexpr bool hasPolymorphicLength()
    {
        return BaseImpl::hasLength() && (!ImplOptions::HasNoLengthImpl);
    }

    /// @brief Compile time inquiry of whether polymoriphic dispatch has been
    ///     requested via interface options and hasn't been inhibited by
    ///     the @ref comms::option::app::NoDispatchImpl.
    static constexpr bool hasPolymorphicDispatch()
    {
        return BaseImpl::hasDispatch() && (!ImplOptions::HasNoDispatchImpl);
    }

    /// @brief Compile time inquiry of whether @ref comms::MessageBase has
    ///    notified about custom refresh functionality in the derived class
    ///    via @ref comms::option::def::HasCustomRefresh.
    static constexpr bool hasCustomRefresh()
    {
        return ImplOptions::HasCustomRefresh;
    }    

    /// @brief Compile time inquiry of whether @ref comms::MessageBase has
    ///    notified about custom name retrieval function in the derived class
    ///    via @ref comms::option::def::HasName.
    static constexpr bool hasCustomName()
    {
        return ImplOptions::HasName;
    }  
    
    /// @brief Compile type inquiry whether the class provides @ref doGetId() member function.
    static constexpr bool hasDoGetId()
    {
        return ImplOptions::HasStaticMsgId || ImplOptions::HasDoGetId;
    }        

#ifdef FOR_DOXYGEN_DOC_ONLY

    /// @brief All field classes provided with @ref comms::option::def::FieldsImpl option.
    /// @details The type is not defined if @ref comms::option::def::FieldsImpl option
    ///     wasn't provided to comms::MessageBase.
    using AllFields = FieldsProvidedWithOption;

    /// @brief Get an access to the fields of the message.
    /// @details The function doesn't exist if @ref comms::option::def::FieldsImpl option
    ///     wasn't provided to comms::MessageBase.
    /// @return Reference to the fields of the message.
    AllFields& fields();

    /// @brief Get an access to the fields of the message.
    /// @details The function doesn't exist if @ref comms::option::def::FieldsImpl option
    ///     wasn't provided to comms::MessageBase.
    /// @return Const reference to the fields of the message.
    const AllFields& fields() const;

    /// @brief Compile time check of whether the message fields are
    ///     version dependent.
    /// @details The function doesn't exist if @ref comms::option::def::FieldsImpl option
    ///     wasn't provided to comms::MessageBase.
    /// @return @b true if at least one of the fields is version dependent.
    static constexpr bool areFieldsVersionDependent();

    /// @brief Default implementation of ID retrieval functionality.
    /// @details This function exists only if @ref comms::option::def::StaticNumIdImpl option
    ///     was provided to comms::MessageBase. @n
    /// @return Numeric ID of the message.
    static constexpr MsgIdParamType doGetId();

    /// @brief Default implementation of read functionality.
    /// @details This function exists only if @ref comms::option::def::FieldsImpl option
    ///     was provided to comms::MessageBase. @n
    ///     To make this function works, every field class must provide "read"
    ///     function with following signature:
    ///     @code
    ///     template <typename TIter>
    ///     ErrorStatus read(TIter& iter, std::size_t size);
    ///     @endcode
    ///     This function will invoke such "read()" member function for every
    ///     field object listed with @ref comms::option::def::FieldsImpl option. If
    ///     any field doesn't report ErrorStatus::Success, then read operation
    ///     stops, i.e. the provided iterator is not advanced any more.
    /// @tparam TIter Type of the iterator used for reading.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @param[in] size Maximum number of bytes that can be read.
    /// @return Status of the operation.
    template <typename TIter>
    ErrorStatus doRead(TIter& iter, std::size_t size);

    /// @brief Default implementation of write functionality.
    /// @details This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided
    ///     to @ref comms::MessageBase. @n
    ///     To make this function works, every field class must provide "write"
    ///     function with following signature:
    ///     @code
    ///     template <typename TIter>
    ///     ErrorStatus write(TIter& iter, std::size_t size) const;
    ///     @endcode
    ///     This function will invoke such "write()" member function for every
    ///     field object listed with @ref comms::option::def::FieldsImpl option. If
    ///     any field doesn't report ErrorStatus::Success, then write operation
    ///     stops, i.e. the provided iterator is not advanced any more.
    /// @tparam TIter Type of the iterator used for writing.
    /// @param[in, out] iter Iterator used for writing the data.
    /// @param[in] size Maximum number of bytes that can be written.
    /// @return Status of the operation.
    template <typename TIter>
    ErrorStatus doWrite(TIter& iter, std::size_t size) const;

    /// @brief Default implementation of validity check functionality.
    /// @details This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     To make this function works, every field class must provide "valid()"
    ///     function with following signature:
    ///     @code
    ///     bool valid() const;
    ///     @endcode
    ///     This function will invoke such "valid()" member function for every
    ///     field object listed with @ref comms::option::def::FieldsImpl option.
    /// @return true when @b all fields are valid.
    bool doValid() const;

    /// @brief Default implementation of refreshing functionality.
    /// @details This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     To make this function works, every field class must provide "refresh()"
    ///     function with following signature:
    ///     @code
    ///     bool refresh() const;
    ///     @endcode
    ///     This function will invoke such "refresh()" member function for every
    ///     field object listed with @ref comms::option::def::FieldsImpl option and will
    ///     return @b true if <b>at least</b> one of the invoked functions returned
    ///     @b true.
    /// @return true when <b>at least</b> one of the fields has been updated.
    bool doRefresh() const;

    /// @brief Default implementation of length calculation functionality.
    /// @details This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     To make this function works, every field class must provide "length()"
    ///     function with following signature:
    ///     @code
    ///     std::size_t length() const;
    ///     @endcode
    ///     This function will invoke such "length()" member function for every
    ///     field object listed with @ref comms::option::def::FieldsImpl option. The
    ///     final result is a summary of the "length" values of all the
    ///     fields.
    /// @return Serialisation length of the message.
    std::size_t doLength() const;

    /// @brief Default implementation of partial length calculation functionality.
    /// @details Similar to @ref length() member function but starts the calculation
    ///     at the the field specified using @b TFromIdx template parameter.
    /// @tparam TFromIdx Index of the field, from which length calculation will start
    /// @return Calculated serialisation length
    /// @pre TFromIdx < std::tuple_size<AllFields>::value
    template <std::size_t TFromIdx>
    std::size_t doLengthFrom() const;

    /// @brief Default implementation of partial length calculation functionality.
    /// @details Similar to @ref length() member function but stops the calculation
    ///     at the the field specified using @b TUntilIdx template parameter.
    /// @tparam TUntilIdx Index of the field, at which the calculation will stop.
    ///     The length of the filed with index @b TUntilIdx will @b NOT be taken
    ///     into account.
    /// @return Calculated serialisation length
    /// @pre TUntilIdx <= std::tuple_size<AllFields>::value
    template <std::size_t TUntilIdx>
    std::size_t doLengthUntil() const;

    /// @brief Default implementation of partial length calculation functionality.
    /// @details Similar to @ref length() member function but starts the calculation
    ///     at the the field specified using @b TFromIdx template parameter, and
    ///     stops the calculation
    ///     at the the field specified using @b TUntilIdx template parameter.
    /// @tparam TFromIdx Index of the field, from which length calculation will start
    /// @tparam TUntilIdx Index of the field, at which the calculation will stop.
    ///     The length of the filed with index @b TUntilIdx will @b NOT be taken
    ///     into account.
    /// @return Calculated serialisation length
    /// @pre TFromIdx < std::tuple_size<AllFields>::value
    /// @pre TUntilIdx <= std::tuple_size<AllFields>::value
    /// @pre TFromIdx < TUntilIdx
    template <std::size_t TFromIdx, std::size_t TUntilIdx>
    std::size_t doLengthFromUntil() const;

    /// @brief Compile time constant of minimal serialisation length.
    /// @details This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     To make this function works, every field class must provide "minLength()"
    ///     function with following signature:
    ///     @code
    ///     static constexpr std::size_t minLength();
    ///     @endcode
    /// @return Minimal serialisation length of the message.
    static constexpr std::size_t doMinLength();

    /// @brief Compile time constant of minimal partial serialisation length.
    /// @details Similar to @ref doMinLength() member function but starts the calculation
    ///     at the the field specified using @b TFromIdx template parameter.
    /// @tparam TFromIdx Index of the field, from which length calculation will start
    /// @return Calculated minimal serialisation length
    /// @pre TFromIdx < std::tuple_size<AllFields>::value
    template <std::size_t TFromIdx>
    static constexpr std::size_t doMinLengthFrom();

    /// @brief Compile time constant of minimal partial serialisation length.
    /// @details Similar to @ref doMinLength() member function but stops the calculation
    ///     at the the field specified using @b TUntilIdx template parameter.
    /// @tparam TUntilIdx Index of the field, at which the calculation will stop.
    ///     The length of the filed with index @b TUntilIdx will @b NOT be taken
    ///     into account.
    /// @return Calculated minimal serialisation length
    /// @pre TUntilIdx <= std::tuple_size<AllFields>::value
    template <std::size_t TUntilIdx>
    static constexpr std::size_t doMinLengthUntil();

    /// @brief Compile time constant of minimal partial serialisation length.
    /// @details Similar to @ref doMinLength() member function but starts the calculation
    ///     at the the field specified using @b TFromIdx template parameter, and
    ///     stops the calculation
    ///     at the the field specified using @b TUntilIdx template parameter.
    /// @tparam TFromIdx Index of the field, from which length calculation will start
    /// @tparam TUntilIdx Index of the field, at which the calculation will stop.
    ///     The length of the filed with index @b TUntilIdx will @b NOT be taken
    ///     into account.
    /// @return Calculated minimal serialisation length
    /// @pre TFromIdx < std::tuple_size<AllFields>::value
    /// @pre TUntilIdx <= std::tuple_size<AllFields>::value
    /// @pre TFromIdx < TUntilIdx
    template <std::size_t TFromIdx, std::size_t TUntilIdx>
    std::size_t doMinLengthFromUntil() const;

    /// @brief Compile time constant of maximal serialisation length.
    /// @details This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     To make this function works, every field class must provide "maxLength()"
    ///     function with following signature:
    ///     @code
    ///     static constexpr std::size_t maxLength();
    ///     @endcode
    /// @return Minimal serialisation length of the message.
    static constexpr std::size_t doMaxLength();

    /// @brief Compile time constant of maximal partial serialisation length.
    /// @details Similar to @ref doMaxLength() member function but starts the calculation
    ///     at the the field specified using @b TFromIdx template parameter.
    /// @tparam TFromIdx Index of the field, from which length calculation will start
    /// @return Calculated minimal serialisation length
    /// @pre TFromIdx < std::tuple_size<AllFields>::value
    template <std::size_t TFromIdx>
    static constexpr std::size_t doMaxLengthFrom();

    /// @brief Compile time constant of maximal partial serialisation length.
    /// @details Similar to @ref doMaxLength() member function but stops the calculation
    ///     at the the field specified using @b TUntilIdx template parameter.
    /// @tparam TUntilIdx Index of the field, at which the calculation will stop.
    ///     The length of the filed with index @b TUntilIdx will @b NOT be taken
    ///     into account.
    /// @return Calculated minimal serialisation length
    /// @pre TUntilIdx <= std::tuple_size<AllFields>::value
    template <std::size_t TUntilIdx>
    static constexpr std::size_t doMaxLengthUntil();

    /// @brief Compile time constant of maximal partial serialisation length.
    /// @details Similar to @ref doMaxLength() member function but starts the calculation
    ///     at the the field specified using @b TFromIdx template parameter, and
    ///     stops the calculation
    ///     at the the field specified using @b TUntilIdx template parameter.
    /// @tparam TFromIdx Index of the field, from which length calculation will start
    /// @tparam TUntilIdx Index of the field, at which the calculation will stop.
    ///     The length of the filed with index @b TUntilIdx will @b NOT be taken
    ///     into account.
    /// @return Calculated minimal serialisation length
    /// @pre TFromIdx < std::tuple_size<AllFields>::value
    /// @pre TUntilIdx <= std::tuple_size<AllFields>::value
    /// @pre TFromIdx < TUntilIdx
    template <std::size_t TFromIdx, std::size_t TUntilIdx>
    std::size_t doMaxLengthFromUntil() const;

    /// @brief Update version information of all the fields.
    /// @details This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to @ref comms::MessageBase and
    ///     @ref comms::option::def::VersionInExtraTransportFields was provided to the
    ///     message interface class (@ref comms::Message). @n
    ///     This function will invoke such @b setVersion() member function for every
    ///     field object listed with @ref comms::option::def::FieldsImpl option and will
    ///     return @b true if <b>at least</b> one of the invoked functions returned
    ///     @b true (similar to @ref doRefresh()).
    /// @return true when <b>at least</b> one of the fields has been updated.
    bool doFieldsVersionUpdate();

#endif // #ifdef FOR_DOXYGEN_DOC_ONLY

protected:
    ~MessageBase() noexcept = default;

#ifdef FOR_DOXYGEN_DOC_ONLY
    /// @brief Implementation of ID retrieval functionality.
    /// @details This function may exist only if ID retrieval is possible, i.e.
    ///     the ID type has been privded to comms::Message using
    ///     @ref comms::option::def::MsgIdType option and the polymorphic ID retrieval
    ///     functionality was requested (using @ref comms::option::app::IdInfoInterface).
    ///     In addition to the conditions listed earlier this function is
    ///     provided if local doGetId() function was generated. If not,
    ///     it may still be provided if
    ///     the derived class is known (@ref comms::option::def::MsgType option
    ///     was used) and the @ref comms::option::def::HasDoGetId option is used
    ///     to declare the derived type having doGetId() member function
    ///     defined.
    /// @return ID value passed as template parameter to @ref comms::option::def::StaticNumIdImpl
    ///     option.
    virtual MsgIdParamType getIdImpl() const override;

    /// @brief Implementation of dispatch functionality.
    /// @details This function exists only if the following conditions are @b true:
    ///     @li @ref comms::option::app::Handler option
    ///     option was provided to comms::Message.
    ///     @li @ref comms::option::def::MsgType option was used to specify actual type
    ///     of the inheriting message class.
    ///     @li @ref comms::option::app::NoDispatchImpl option was @b NOT used.
    ///
    ///     In order to properly implement the dispatch functionality
    ///     this class imposes several requirements. First of all, the custom
    ///     message class must provide its own type as an argument to
    ///     @ref comms::option::def::MsgType option:
    ///     @code
    ///     class MyMessageBase :  public comms::Message<...> { ...};
    ///
    ///     class Message1 :
    ///         public comms::MessageBase<
    ///             MyMessageBase,
    ///             ...
    ///             comms::option::def::MsgType<Message1>
    ///             ...
    ///         >
    ///     {
    ///         ...
    ///     };
    ///     @endcode
    ///     Second, The @ref Handler type (inherited from comms::Message) must
    ///     implement "handle()" member function for every message type (specified as
    ///     Message1, Message2, ...) it is supposed to handle:
    ///     @code
    ///     class MyHandler {
    ///     public:
    ///         DispatchRetType handle(Message1& msg);
    ///         DispatchRetType handle(Message2& msg);
    ///         ...
    ///     }
    ///     @endcode
    ///     The "handle()" functions may be virtual. If the handler is capable
    ///     of handling only limited number of messages, there is
    ///     a need to provide additional "handle()" member function to implement
    ///     default handling functionality (usually ignore the message by doing
    ///     nothing) for all other messages that weren't handled explicitly.
    ///     @code
    ///     class MyHandler {
    ///     public:
    ///         ...
    ///         DispatchRetType handle(MessageBase& msg);
    ///     }
    ///     @endcode
    ///     Where "MessageBase" is a common base class for all the possible
    ///     messages.
    ///
    ///     Once the requirements above are properly implemented, the implementation
    ///     of this message is very simple:
    ///     @code
    ///     DispatchRetType dispatchImpl(Handler& handler)
    ///     {
    ///         typedef <actual-message-type-provided-with-option> Actual;
    ///         return handler.handle(static_cast<Actual&>(*this));
    ///     }
    ///     @endcode
    ///     The code above forces a compiler to choose appropriate @b handle()
    ///     function in the Handler class, based on the actual type of the message.
    ///     If such function is not found, the compiler will choose to call
    ///     the one that covers all possible messages @b "void handle(MessageBase& msg)".
    /// @param handler Reference to handler object.
    virtual DispatchRetType dispatchImpl(Handler& handler) override;

    /// @brief Implementation of polymorphic read functionality.
    /// @details This function exists if @ref comms::option::app::ReadIterator option
    ///         was provided to comms::Message class when specifying interface, and
    ///         @ref comms::option::app::NoReadImpl option was @b NOT used to inhibit
    ///         the implementation. @n
    ///         If @ref comms::option::def::MsgType option was used to specify the actual
    ///         type of the message, and if it contains custom doRead()
    ///         function, it will be invoked. Otherwise, the invocation of
    ///         comms::MessageBase::doRead() will be chosen in case fields were
    ///         specified using @ref comms::option::def::FieldsImpl option.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @param[in] size Maximum number of bytes that can be read.
    /// @return Status of the operation.
    virtual ErrorStatus readImpl(ReadIterator& iter, std::size_t size) override;

    /// @brief Helper function that allows to read only limited number of fields.
    /// @details Sometimes the default implementation of doRead() is incorrect.
    ///     For example, some bit in specific field specifies whether other field
    ///     exists or must be skipped. In this case the derived class must
    ///     implement different read functionality. To help in such task this
    ///     function provides an ability to read all the fields up to (not including) requested
    ///     field. The overriding doRead() function in the custom message
    ///     definition class may use this function for such task.
    ///     This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     The requirements from field classes is the same as explained in
    ///     doRead() documentation.
    /// @tparam TIdx Zero based index of the field to read until. The function
    ///     returns when field with index "TIdx - 1" (if such exists) has been
    ///     read, while field with index "TIdx" still hasn't.
    /// @tparam TIter Type of the iterator used for reading.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @param[in] len Maximum number of bytes that can be read.
    /// @return Status of the operation.
    /// @pre TIdx <= std::tuple_size<AllFields>::value
    template <std::size_t TIdx, typename TIter>
    ErrorStatus doReadUntil(TIter& iter, std::size_t& len);

    /// @brief Same as @ref doReadUntil(), but updating length parameter.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @param[in, out] len Maximum number of bytes that can be read.    
    template <std::size_t TIdx, typename TIter>
    ErrorStatus doReadUntilAndUpdateLen(TIter& iter, std::size_t& len);

    /// @brief Helper function that allows to read only limited number of fields.
    /// @details Similar to @ref doReadUntil(), but doesn't check for errors
    ///     and doesn't report status. This function can be used instead of
    ///     @ref doReadUntil() when correction of the read operation was
    ///     ensured by other means prior to its invocation.
    /// @tparam TIdx Zero based index of the field to read until. The function
    ///     returns when field with index "TIdx - 1" (if such exists) has been
    ///     read, while field with index "TIdx" still hasn't.
    /// @tparam TIter Type of the iterator used for reading.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @pre TIdx <= std::tuple_size<AllFields>::value
    template <std::size_t TIdx, typename TIter>
    void doReadNoStatusUntil(TIter& iter);

    /// @brief Helper function that allows to read only limited number of fields.
    /// @details Sometimes the default implementation of doRead() is incorrect.
    ///     For example, some bit in specific field specifies whether other field
    ///     exists or must be skipped. In this case the derived class must
    ///     implement different read functionality. To help in such task
    ///     @ref doReadUntil() function allows to read fields up to a specified one,
    ///     while this function provides an ability to resume reading from some
    ///     other field in the middle. The overriding doRead() function in the
    ///     custom message definition class may use this function for such task.
    ///     This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     The requirements from field classes is the same as explained in
    ///     doRead() documentation.
    /// @tparam TIdx Zero based index of the field to read from. The function
    ///     reads all the fields between the one indexed TIdx (included) and
    ///     the last one (also included).
    /// @tparam TIter Type of the iterator used for reading.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @param[in] len Maximum number of bytes that can be read.
    /// @return Status of the operation.
    /// @pre TIdx < std::tuple_size<AllFields>::value
    template <std::size_t TIdx, typename TIter>
    ErrorStatus doReadFrom(TIter& iter, std::size_t len);

    /// @brief Same as @ref doReadFrom(), but modifies length parameter.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @param[in, out] len Maximum number of bytes that can be read.    
    template <std::size_t TIdx, typename TIter>
    ErrorStatus doReadFromAndUpdateLen(TIter& iter, std::size_t& len);

    /// @brief Helper function that allows to read only limited number of fields.
    /// @details Similar to @ref doReadFrom(), but doesn't check for errors
    ///     and doesn't report status. This function can be used instead of
    ///     @ref doReadFrom() when correction of the read operation was
    ///     ensured by other means prior to its invocation.
    /// @tparam TIdx Zero based index of the field to read from. The function
    ///     reads all the fields between the one indexed TIdx (included) and
    ///     the last one (also included).
    /// @tparam TIter Type of the iterator used for reading.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @pre TIdx < std::tuple_size<AllFields>::value
    template <std::size_t TIdx, typename TIter>
    void doReadNoStatusFrom(TIter& iter);

    /// @brief Helper function that allows to read only limited number of fields.
    /// @details Sometimes the default implementation of doRead() is incorrect.
    ///     For example, some bit in specific field specifies whether other fields
    ///     exist or must be skipped. In this case the derived class must
    ///     implement different read functionality. In similar way to
    ///     doReadFrom() and doReadUntil() this function provides an
    ///     ability to read any number of fields.
    ///     This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     The requirements from field classes is the same as explained in
    ///     doRead() documentation.
    /// @tparam TFromIdx Zero based index of the field to read from.
    /// @tparam TUntilIdx Zero based index of the field to read until (not included).
    /// @tparam TIter Type of the iterator used for reading.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @param[in] len Maximum number of bytes that can be read.
    /// @return Status of the operation.
    /// @pre TFromIdx < std::tuple_size<AllFields>::value
    /// @pre TUntilIdx <= std::tuple_size<AllFields>::value
    /// @pre TFromIdx < TUntilIdx
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus doReadFromUntil(TIter& iter, std::size_t len);

    /// @brief Same as @ref doReadFromUntil(), but modifies length parameter.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @param[in, out] len Maximum number of bytes that can be read.    
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus doReadFromUntilAndUpdateLen(TIter& iter, std::size_t& len);

    /// @brief Helper function that allows to read only limited number of fields.
    /// @details Similar to @ref doReadFromUntil(), but doesn't check for errors
    ///     and doesn't report status. This function can be used instead of
    ///     @ref doReadFromUntil() when correction of the read operation was
    ///     ensured by other means prior to its invocation.
    /// @tparam TFromIdx Zero based index of the field to read from.
    /// @tparam TUntilIdx Zero based index of the field to read until (not included).
    /// @tparam TIter Type of the iterator used for reading.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @pre TFromIdx < std::tuple_size<AllFields>::value
    /// @pre TUntilIdx <= std::tuple_size<AllFields>::value
    /// @pre TFromIdx < TUntilIdx
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    void doReadNoStatusFromUntil(TIter& iter);

    /// @brief Implementation of polymorphic write functionality.
    /// @details This function exists if @ref comms::option::app::WriteIterator option
    ///         was provided to comms::Message class when specifying interface, and
    ///         @ref comms::option::app::NoWriteImpl option was @b NOT used to inhibit
    ///         the implementation. @n
    ///         If @ref comms::option::def::MsgType option was used to specify the actual
    ///         type of the message, and if it contains custom doWrite()
    ///         function, it will be invoked. Otherwise, the invocation of
    ///         comms::MessageBase::doWrite() will be chosen in case fields were
    ///         specified using @ref comms::option::def::FieldsImpl option.
    /// @param[in, out] iter Iterator used for writing the data.
    /// @param[in] size Maximum number of bytes that can be written.
    /// @return Status of the operation.
    virtual ErrorStatus writeImpl(WriteIterator& iter, std::size_t size) const override;

    /// @brief Helper function that allows to write only limited number of fields.
    /// @details In a similar way to doReadUntil(), this function allows
    ///     writing limited number of fields starting from the first one.
    ///     This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     The requirements from field classes is the same as explained in
    ///     doWrite() documentation.
    /// @tparam TIdx Zero based index of the field to write until. The function
    ///     returns when field with index "TIdx - 1" (if such exists) has been
    ///     written, while field with index "TIdx" still hasn't.
    /// @tparam TIter Type of iterator used for writing.
    /// @param[in, out] iter Iterator used for writing the data.
    /// @param[in] len Maximum number of bytes that can be written.
    /// @return Status of the operation.
    /// @pre TIdx <= std::tuple_size<AllFields>::value
    template <std::size_t TIdx, typename TIter>
    ErrorStatus doWriteUntil(TIter& iter, std::size_t len) const;

    /// @brief Same as @ref doWriteUntil(), but modifies length parameter.
    template <std::size_t TIdx, typename TIter>
    ErrorStatus doWriteUntilAndUpdateLen(TIter& iter, std::size_t& len) const;

    /// @brief Helper function that allows to write only limited number of fields.
    /// @details Similar to @ref doWriteUntil(), but doesn't check for errors
    ///     and doesn't report status. This function can be used instead of
    ///     @ref doWriteUntil() when correction of the write operation was
    ///     ensured by other means prior to its invocation.
    /// @tparam TIdx Zero based index of the field to write until. The function
    ///     returns when field with index "TIdx - 1" (if such exists) has been
    ///     written, while field with index "TIdx" still hasn't.
    /// @tparam TIter Type of the iterator used for writing.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @pre TIdx <= std::tuple_size<AllFields>::value
    template <std::size_t TIdx, typename TIter>
    void doWriteNoStatusUntil(TIter& iter) const;

    /// @brief Helper function that allows to write only limited number of fields.
    /// @details In a similar way to doReadFrom(), this function allows
    ///     writing limited number of fields starting from the requested one until
    ///     the end.
    ///     This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     The requirements from field classes is the same as explained in
    ///     doWrite() documentation.
    /// @tparam TIdx Zero based index of the field to write from.
    /// @tparam TIter Type of iterator used for writing.
    /// @param[in, out] iter Iterator used for writing the data.
    /// @param[in] len Maximum number of bytes that can be written.
    /// @return Status of the operation.
    /// @pre TIdx < std::tuple_size<AllFields>::value
    template <std::size_t TIdx, typename TIter>
    ErrorStatus doWriteFrom(TIter& iter, std::size_t len) const;

    /// @brief Same as @ref doWriteFrom(), but modifies length parameter.
    template <std::size_t TIdx, typename TIter>
    ErrorStatus doWriteFromAndUpdateLen(TIter& iter, std::size_t& len) const;

    /// @brief Helper function that allows to write only limited number of fields.
    /// @details Similar to @ref doWriteFrom(), but doesn't check for errors
    ///     and doesn't report status. This function can be used instead of
    ///     @ref doWriteFrom() when correction of the write operation was
    ///     ensured by other means prior to its invocation.
    /// @tparam TIdx Zero based index of the field to write from.
    /// @tparam TIter Type of the iterator used for writing.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @pre TIdx < std::tuple_size<AllFields>::value
    template <std::size_t TIdx, typename TIter>
    void doWriteNoStatusFrom(TIter& iter) const;

    /// @brief Helper function that allows to write only limited number of fields.
    /// @details In a similar way to doReadFromUntil(), this function allows
    ///     writing limited number of fields between the requested indices.
    ///     This function exists only if @ref comms::option::def::FieldsImpl or
    ///     @ref comms::option::def::ZeroFieldsImpl option was provided to comms::MessageBase.
    ///     The requirements from field classes is the same as explained in
    ///     doWrite() documentation.
    /// @tparam TFromIdx Zero based index of the field to write from.
    /// @tparam TUntilIdx Zero based index of the field to write until (not including).
    /// @tparam TIter Type of iterator used for writing.
    /// @param[in, out] iter Iterator used for writing the data.
    /// @param[in] len Maximum number of bytes that can be written.
    /// @return Status of the operation.
    /// @pre TFromIdx < std::tuple_size<AllFields>::value
    /// @pre TUntilIdx <= std::tuple_size<AllFields>::value
    /// @pre TFromIdx < TUntilIdx
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus doWriteFromUntil(TIter& iter, std::size_t len) const;

    /// @brief Same as @ref doWriteNoStatusFrom(), but updates length information
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus doWriteFromUntilAndUpdateLen(TIter& iter, std::size_t& len) const;

    /// @brief Helper function that allows to write only limited number of fields.
    /// @details Similar to @ref doWriteFromUntil(), but doesn't check for errors
    ///     and doesn't report status. This function can be used instead of
    ///     @ref doWriteFromUntil() when correction of the write operation was
    ///     ensured by other means prior to its invocation.
    /// @tparam TFromIdx Zero based index of the field to write from.
    /// @tparam TUntilIdx Zero based index of the field to write until (not including).
    /// @tparam TIter Type of iterator used for writing.
    /// @param[in, out] iter Iterator used for reading the data.
    /// @pre TFromIdx < std::tuple_size<AllFields>::value
    /// @pre TUntilIdx <= std::tuple_size<AllFields>::value
    /// @pre TFromIdx < TUntilIdx
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    void doWriteNoStatusFromUntil(TIter& iter) const;

    /// @brief Implementation of polymorphic validity check functionality.
    /// @details This function exists if @ref comms::option::app::ValidCheckInterface option
    ///         was provided to comms::Message class when specifying interface, and
    ///         @ref comms::option::app::NoValidImpl option was @b NOT used to inhibit
    ///         the implementation. @n
    ///         If @ref comms::option::def::MsgType option was used to specify the actual
    ///         type of the message, and if it contains custom doValid()
    ///         function, it will be invoked. Otherwise, the invocation of
    ///         comms::MessageBase::doValid() will be chosen in case fields were
    ///         specified using @ref comms::option::def::FieldsImpl option.
    virtual bool validImpl() const override;

    /// @brief Implementation of polymorphic length calculation functionality.
    /// @details This function exists if @ref comms::option::app::LengthInfoInterface option
    ///         was provided to comms::Message class when specifying interface, and
    ///         @ref comms::option::app::NoLengthImpl option was @b NOT used to inhibit
    ///         the implementation. @n
    ///         If @ref comms::option::def::MsgType option was used to specify the actual
    ///         type of the message, and if it contains custom doLength()
    ///         function, it will be invoked. Otherwise, the invocation of
    ///         comms::MessageBase::doLength() will be chosen in case fields were
    ///         specified using @ref comms::option::def::FieldsImpl option.
    /// @return Serialisation length of the message.
    virtual std::size_t lengthImpl() const override;

    /// @brief Implementation of polymorphic refresh functionality.
    /// @details This function exists if @ref comms::option::app::RefreshInterface option
    ///         was provided to comms::Message class when specifying interface,
    ///         and @ref comms::option::def::HasCustomRefresh option was used (either on
    ///         on of the fields or when defining a message class) to
    ///         to notify about existence of custom refresh functionality.
    ///         If @ref comms::option::def::MsgType option was used to specify the actual
    ///         message class, the @b this pointer will be downcasted to it to
    ///         invoke doRefresh() member function defined there. If such
    ///         is not defined the default doRefresh() member function from
    ///         this class will be used.
    /// @return @b true in case fields were updated, @b false if nothing has changed.
    virtual bool refreshImpl() override;

    /// @brief Implementation of polymorphic name retrieval functionality.
    /// @details This function exists if @ref comms::option::app::NameInterface option
    ///         was provided to @ref comms::Message class when specifying interface,
    ///         and @ref comms::option::def::HasName as well as @ref comms::option::def::MsgType
    ///         options ware used for this class.
    ///         This function downcasts @b this pointer to actual message type and
    ///         invokes @b doName() member function.
    /// @return @b true in case fields were updated, @b false if nothing has changed.
    virtual const char* nameImpl() const override;

#endif // #ifdef FOR_DOXYGEN_DOC_ONLY
};

/// @brief Message object equality comparison operator
/// @details Messages are considered equal if all their fields are considered equal
/// @related MessageBase
template <typename TMessage1, typename TMessage2, typename... TOptions>
bool operator==(const MessageBase<TMessage1, TOptions...>& msg1, const MessageBase<TMessage2, TOptions...>& msg2) noexcept
{
    return msg1.fields() == msg2.fields();
}

/// @brief Message object inequality comparison operator
/// @details Messages are considered not equal if any their fields are considered inequal.
/// @related MessageBase
template <typename TMessage1, typename TMessage2, typename... TOptions>
bool operator!=(const MessageBase<TMessage1, TOptions...>& msg1, const MessageBase<TMessage2, TOptions...>& msg2) noexcept
{
    return !(msg1 == msg2);
}

/// @brief Upcast type of the message object to comms::MessageBase in order to have
///     access to its internal types.
template <typename TMessage, typename... TOptions>
inline
MessageBase<TMessage, TOptions...>& toMessageBase(MessageBase<TMessage, TOptions...>& msg)
{
    return msg;
}

/// @brief Upcast type of the message object to comms::MessageBase in order to have
///     access to its internal types.
template <typename TMessage, typename... TOptions>
inline
const MessageBase<TMessage, TOptions...>& toMessageBase(
    const MessageBase<TMessage, TOptions...>& msg)
{
    return msg;
}

/// @brief Compile time check of of whether the type
///     is a message extending @ref comms::MessageBase.
/// @details Checks existence of @b ImplOptions inner
///     type.
template <typename T>
constexpr bool isMessageBase()
{
    return details::hasImplOptions<T>();
}

}  // namespace comms

/// @brief Add convenience access enum and functions to message fields.
/// @details Very similar to #COMMS_MSG_FIELDS_NAMES(), but does @b NOT
///     require definition of @b Base inner member type (for some compilers) and does @b NOT
///     define inner @b Field_* types for used fields.
/// @param[in] ... List of fields' names.
/// @related comms::MessageBase
/// @see #COMMS_MSG_FIELDS_NAMES()
/// @note Defined in "comms/MessageBase.h"
#define COMMS_MSG_FIELDS_ACCESS(...) \
    COMMS_EXPAND(COMMS_DEFINE_FIELD_ENUM(__VA_ARGS__)) \
    COMMS_MSG_FIELDS_ACCESS_FUNC { \
        auto& val = comms::toMessageBase(*this).fields(); \
        using AllFieldsTuple = typename std::decay<decltype(val)>::type; \
        static_assert(std::tuple_size<AllFieldsTuple>::value == FieldIdx_numOfValues, \
            "Invalid number of names for fields tuple"); \
        return val; \
    } \
    COMMS_MSG_FIELDS_ACCESS_CONST_FUNC { \
        auto& val = comms::toMessageBase(*this).fields(); \
        using AllFieldsTuple =  typename std::decay<decltype(val)>::type; \
        static_assert(std::tuple_size<AllFieldsTuple>::value == FieldIdx_numOfValues, \
            "Invalid number of names for fields tuple"); \
        return val; \
    } \
    COMMS_EXPAND(COMMS_DO_FIELD_ACC_FUNC(AllFields, fields(), __VA_ARGS__))

/// @brief Provide names for message fields.
/// @details The @ref comms::MessageBase class provides access to its fields via
///     @ref comms::MessageBase::fields() member function(s). The fields are bundled
///     into <a href="http://en.cppreference.com/w/cpp/utility/tuple">std::tuple</a>
///     and can be accessed using indices with
///     <a href="http://en.cppreference.com/w/cpp/utility/tuple/get">std::get</a>.
///     For convenience, the fields should be named. The COMMS_MSG_FIELDS_NAMES()
///     macro does exactly that. @n
///     As an example, let's assume that custom message uses 3 fields of any
///     types:
///     @code
///     using Field1 = ... /* some field definition */;
///     using Field2 = ... /* some field definition */;
///     using Field3 = ... /* some field definition */;
///
///     using Message1Fields = std::tuple<Field1, Field2, Field3>;
///
///     class Message1 : public
///         comms::MessageBase<
///             MyInterface,
///             comms::option::def::FieldsImpl<Message1Fields>,
///             ... /* some other options */>
///     {
///         // Base class (re) definition required by COMMS_MSG_FIELDS_NAMES()
///         using Base =
///             comms::MessageBase<
///                 MyInterface,
///                 comms::option::def::FieldsImpl<Message1Fields>,
///                 ... /* some other options */>
///     public:
///         // Provide names for message fields
///         COMMS_MSG_FIELDS_NAMES(name1, name2, name3);
///     };
///     @endcode
///     @b NOTE that there is a required to have @b Base member type that
///     specifies base class used. It is needed to be able to
///     access @ref comms::MessageBase::AllFields type definition.
///
///     The usage of the COMMS_MSG_FIELDS_NAMES() macro with the list of the field's names
///     is equivalent to having the following definitions inside the message class
///     @code
///     class Message1 : public comms::MessageBase<...>
///     {
///         using Base = comms::MessageBase<...>;
///     public:
///         // Indices of the fields
///         enum FieldIdx {
///             FieldIdx_name1,
///             FieldIdx_name2,
///             FieldIdx_name3,
///             FieldIdx_nameOfValues
///         };
///
///         static_assert(std::tuple_size<Base::AllFields>::value == FieldIdx_nameOfValues,
///             "Number of expected fields is incorrect");
///
///         // Accessor to "name1" field.
///         auto field_name1() -> decltype(std::get<FieldIdx_name1>(Base::fields()))
///         {
///             return std::get<FieldIdx_name1>(Base::fields());
///         }
///
///         // Accessor to "name1" field.
///         auto field_name1() const -> decltype(std::get<FieldIdx_name1>(Base::fields()))
///         {
///             return std::get<FieldIdx_name1>(Base::fields());
///         }
///
///         // Accessor to "name2" field.
///         auto field_name2() -> decltype(std::get<FieldIdx_name2>(Base::fields()))
///         {
///             return std::get<FieldIdx_name2>(Base::fields());
///         }
///
///         // Accessor to "name2" field.
///         auto field_name2() const -> decltype(std::get<FieldIdx_name2>(Base::fields()))
///         {
///             return std::get<FieldIdx_name2>(Base::fields());
///         }
///
///         // Accessor to "name3" field.
///         auto field_name3() -> decltype(std::get<FieldIdx_name3>(Base::fields()))
///         {
///             return std::get<FieldIdx_name3>(Base::fields());
///         }
///
///         // Accessor to "name3" field.
///         auto field_name3() const -> decltype(std::get<FieldIdx_name3>(Base::fields()))
///         {
///             return std::get<FieldIdx_name3>(Base::fields());
///         }
///
///         // Redefinition of the field types:
///         using Field_name1 = Field1;
///         using Field_name2 = Field2;
///         using Field_name3 = Field3;
///     };
///     @endcode
///     @b NOTE, that provided names @b name1, @b name2, and @b name3 have
///     found their way to the following definitions:
///     @li @b FieldIdx enum. The names are prefixed with @b FieldIdx_. The
///         @b FieldIdx_nameOfValues value is automatically added at the end.
///     @li Accessor functions prefixed with @b field_*
///     @li Types of fields prefixed with @b Field_*
///
///     As the result, the fields can be accessed using @b FieldIdx enum
///     @code
///     void handle(Message1& msg)
///     {
///         auto& allFields = msg.fields();
///         auto& field1 = std::get<Message1::FieldIdx_name1>(allFields);
///         auto& field2 = std::get<Message1::FieldIdx_name2>(allFields);
///         auto& field3 = std::get<Message1::FieldIdx_name3>(allFields);
///
///         auto value1 = field1.value();
///         auto value2 = field2.value();
///         auto value3 = field3.value();
///     }
///     @endcode
///     or using accessor functions:
///     @code
///     void handle(Message1& msg)
///     {
///         auto value1 = field_name1().value();
///         auto value2 = field_name2().value();
///         auto value3 = field_name3().value();
///     }
///     @endcode
/// @param[in] ... List of fields' names.
/// @related comms::MessageBase
/// @pre Requires (re)definition of the message base class as
///     inner @b Base member type.
/// @see COMMS_MSG_FIELDS_ACCESS()
/// @note Defined in "comms/MessageBase.h"
#define COMMS_MSG_FIELDS_NAMES(...) \
    COMMS_EXPAND(COMMS_MSG_FIELDS_ACCESS(__VA_ARGS__)) \
    COMMS_EXPAND(COMMS_DO_FIELD_TYPEDEF(typename Base::AllFields, Field_, FieldIdx_, __VA_ARGS__))

/// @brief Generate convinience alias access member functions for other
///     member fields.
/// @details Similar to @ref COMMS_MSG_FIELD_ALIAS() but requires usage
///     of #COMMS_MSG_FIELDS_ACCESS() instead of #COMMS_MSG_FIELDS_NAMES()
///     and does NOT create alias to the field type, only access functions.
/// @param[in] f_ Alias field name.
/// @param[in] ... List of fields' names.
/// @pre The macro #COMMS_MSG_FIELDS_ACCESS() needs to be used before
///     @ref COMMS_MSG_FIELD_ALIAS_ACCESS() to define convenience access functions.
/// @related comms::MessageBase
/// @note Defined in "comms/MessageBase.h"
/// @see @ref COMMS_MSG_FIELD_ALIAS()
#define COMMS_MSG_FIELD_ALIAS_ACCESS(f_, ...) COMMS_DO_ALIAS(field_, f_, __VA_ARGS__)

/// @brief Generate convinience alias types and access member functions for other
///     member fields.
/// @details The #COMMS_MSG_FIELDS_NAMES() macro generates inner types
///     and convenience access member functions for member fields. Sometimes the fields
///     may get renamed or moved to be a member of other fields, like
///     @ref comms::field::Bundle or @ref comms::field::Bitfield. In such
///     case the compilation of the existing client code (that already
///     uses published protocol definition) may fail. To avoid such scenarios
///     and make the transition to newer versions of the protocol easier,
///     the @ref COMMS_MSG_FIELD_ALIAS() macro can be used to create alias
///     to other fields. For example, let's assume that some message class was defined:
///     like this.
///     @code
///     class Message1 : public comms::MessageBase<...>
///     {
///         using Base = comms::MessageBase<...>;
///     public:
///         COMMS_MSG_FIELDS_NAMES(name1, name2, name3);
///     };
///     @endcode
///     In the future versions of the protocol "name3" was renamed to
///     "newName3". To keep the existing code (that uses "name3" name)
///     compiling it is possible to create an alias access function(s)
///     with:
///     @code
///     class Message1 : public comms::MessageBase<...>
///     {
///         using Base = comms::MessageBase<...>;
///     public:
///         COMMS_MSG_FIELDS_NAMES(name1, name2, newName3);
///         COMMS_MSG_FIELD_ALIAS(name3, newName3);
///     };
///     @endcode
///     The usage of @ref COMMS_MSG_FIELD_ALIAS() in the code above is
///     equivalent to having the following functions as well as type defined:
///     @code
///     class Message1 : public comms::MessageBase<...>
///     {
///     public:
///         ...
///
///         using Field_name3 = Field_newName3;
///
///         auto field_name3() -> decltype(field_newName3())
///         {
///             return field_newName3();
///         }
///
///         auto field_name3() const -> decltype(field_newName3())
///         {
///             return field_newName3();
///         }
///     };
///     @endcode
///     Another example would be a replacing a @ref comms::field::IntValue
///     with @ref comms::field::Bitfield in the future version of the
///     protocol. It can happen when the developer decides to split
///     the used storage into multiple values (because the range
///     of the used/valid values allows so). In order to keep the old client
///     code compiling, the access to the replaced field needs to be
///     an alias to the first member of the @ref comms::field::Bitfield.
///     In this case the usage of @ref COMMS_MSG_FIELD_ALIAS() will
///     look like this:
///     @code
///     class Message1 : public comms::MessageBase<...>
///     {
///         using Base = comms::MessageBase<...>;
///     public:
///         COMMS_MSG_FIELDS_NAMES(name1, name2, newName3);
///         COMMS_MSG_FIELD_ALIAS(name3, newName3, member1);
///     };
///     @endcode
///     The usage of @ref COMMS_MSG_FIELD_ALIAS() in the code above is
///     equivalent to having the following functions defined:
///     @code
///     class Message1 : public comms::MessageBase<...>
///     {
///     public:
///         ...
///
///         using Field_name3 = typename Field_newName3::Field_member1;
///
///         auto field_name3() -> decltype(field_newName3().field_member1())
///         {
///             return field_newName3().field_member1();
///         }
///
///         auto field_name3() const -> decltype(field_newName3().field_member1())
///         {
///             return field_newName3().field_member1();
///         }
///     };
///     @endcode
/// @param[in] f_ Alias field name.
/// @param[in] ... List of fields' names.
/// @pre The macro #COMMS_MSG_FIELDS_NAMES() needs to be used before
///     @ref COMMS_MSG_FIELD_ALIAS() to define convenience access functions.
/// @related comms::MessageBase
/// @see COMMS_MSG_FIELD_ALIAS_ACCESS()
/// @note Defined in "comms/MessageBase.h"
#define COMMS_MSG_FIELD_ALIAS(f_, ...) \
    COMMS_EXPAND(COMMS_MSG_FIELD_ALIAS_ACCESS(f_, __VA_ARGS__)) \
    COMMS_EXPAND(COMMS_DO_ALIAS_TYPEDEF(Field_, f_, __VA_ARGS__))
