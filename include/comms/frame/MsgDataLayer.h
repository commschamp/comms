//
// Copyright 2014 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::frame::MsgDataLayer

#pragma once

#include "comms/Assert.h"
#include "comms/Field.h"
#include "comms/details/detect.h"
#include "comms/details/tag.h"
#include "comms/frame/details/MsgDataLayerOptionsParser.h"
#include "comms/frame/FrameLayerBase.h"
#include "comms/Message.h"
#include "comms/MessageBase.h"
#include "comms/util/Tuple.h"
#include "comms/util/type_traits.h"

#include <iterator>
#include <tuple>
#include <type_traits>

namespace comms
{

namespace frame
{

/// @brief Message data layer.
/// @details Must always be the last layer in protocol stack.
/// @tparam TOptions Default functionality extension options. Supported options are:
///     @li @ref comms::option::def::FieldType - Use this option to override default field type.
/// @headerfile comms/frame/MsgDataLayer.h
template <typename... TOptions>
class MsgDataLayer
{
    using ParsedOptionsInternal =  details::MsgDataLayerOptionsParser<TOptions...>;

public:
    /// @brief Type of this layer
    using ThisLayer = MsgDataLayer<TOptions...>;

    /// @brief Get access to this layer object.
    ThisLayer& thisLayer()
    {
        return *this;
    }

    /// @brief Get "const" access to this layer object.
    const ThisLayer& thisLayer() const
    {
        return *this;
    }

    /// @brief Raw data field type.
    /// @details This field is used only in @ref AllFields field and @ref
    ///     readFieldsCached() member function.
    ///     If not specified using @ref comms::option::def::FieldType option then
    ///     a variant of @b comms::field::ArrayList for @b std::uint8_t is used.
    using Field = typename ParsedOptionsInternal::FieldType;

    /// @brief All fields of the remaining transport layers, contains only @ref Field.
    using AllFields = std::tuple<Field>;

    /// @brief Default value of AllMessages type.
    /// @details Defined as @b void.
    using AllMessages = void;

    /// @brief Default value of MsgFactory type
    /// @details Defined as @b void.
    using MsgFactory = void;

    /// @brief Static constant indicating amount of transport layers used.
    static const std::size_t NumOfLayers = 1;

    /// @brief Default constructor
    MsgDataLayer() = default;

    /// @brief Copy constructor
    MsgDataLayer(const MsgDataLayer&) = default;

    /// @brief Move constructor
    MsgDataLayer(MsgDataLayer&&) = default;

    /// @brief Destructor
    ~MsgDataLayer() noexcept = default;

    /// @brief Copy assignment operator
    MsgDataLayer& operator=(const MsgDataLayer&) = default;

    /// @brief Move assignment operator
    MsgDataLayer& operator=(MsgDataLayer&&) = default;

    /// @brief Compile time check whether split read "until" and "from" data
    ///     layer is allowed.
    /// @return Always @b true.
    static constexpr bool canSplitRead()
    {
        return true;
    }

    /// @brief Read the message contents.
    /// @details Calls the read() member function of the message object.
    /// @tparam TMsg Type of the @b msg parameter.
    /// @tparam TIter Type of iterator used for reading.
    /// @param[in] msg Reference to the smart pointer holding message object or
    ///     to the message object itself.
    /// @param[in, out] iter Iterator used for reading.
    /// @param[in] size Number of bytes available for reading.
    /// @param[out] extraValues Extra output parameters provided using one
    ///     of the following functions
    ///     @li @ref comms::frame::missingSize()
    ///     @li @ref comms::frame::msgId()
    ///     @li @ref comms::frame::msgIndex()
    ///     @li @ref comms::frame::msgPayload()
    /// @return Status of the read operation.
    /// @pre If @b msg is a smart pointer to message object, it must point to
    ///     a real object.
    /// @post missingSize output value is updated if and only if function
    ///       returns comms::ErrorStatus::NotEnoughData.
    template <typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus read(
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TExtraValues... extraValues)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;
        using Tag =
            typename comms::util::Conditional<
                comms::isMessageBase<MsgType>()
            >::template Type<
                DirectOpTag<>,
                typename comms::util::Conditional<
                    comms::isMessage<MsgType>()
                >::template Type<
                    InterfaceOpTag<>,
                    typename comms::util::LazyShallowConditional<
                        std::is_pointer<MsgType>::value
                    >::template Type<
                        PointerOpTag,
                        OtherOpTag
                    >
                >
            >;

        if (setPayloadRequiredInternal(extraValues...)) {
            auto fromIter = iter;
            auto es = readInternal(msg, iter, size, Tag(), extraValues...);
            setMsgPayloadInternal(fromIter, static_cast<std::size_t>(std::distance(fromIter, iter)), extraValues...);
            return es;
        }

        return readInternal(msg, iter, size, Tag(), extraValues...);
    }

    /// @brief Read transport fields until data layer.
    /// @details Does nothing because it is data layer.
    /// @return @ref comms::ErrorStatus::Success;
    template <typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readUntilData(
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TExtraValues...)
    {
        static_cast<void>(msg);
        static_cast<void>(iter);
        static_cast<void>(size);
        return comms::ErrorStatus::Success;
    }

    /// @brief Same as @ref read().
    /// @details Expected to be called by the privous layers to properly
    ///     finalise read operation after the call to @ref readUntilData();
    /// @return @ref comms::ErrorStatus::Success;
    template <typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readFromData(
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TExtraValues... extraValues)
    {
        return read(msg, iter, size, extraValues...);
    }

    /// @brief Read the message contents while caching the read transport
    ///     information fields.
    /// @details Very similar to read() member function, but adds "allFields"
    ///     parameter to store raw data of the message.
    /// @tparam TIdx Index of the data field in TAllFields, expected to be last
    ///     element in the tuple.
    /// @tparam TAllFields std::tuple of all the transport fields, must be
    ///     @ref AllFields type defined in the last layer class that defines
    ///     protocol stack.
    /// @tparam TMsg Type of the @b msg parameter
    /// @tparam TIter Type of iterator used for reading.
    /// @param[out] allFields Reference to the std::tuple object that wraps all
    ///     transport fields (@ref AllFields type of the last protocol layer class).
    /// @param[in] msg Reference to the smart pointer holding message object or
    ///     to the message object itself.
    /// @param[in, out] iter Iterator used for reading.
    /// @param[in] size Number of bytes available for reading.
    /// @param[out] extraValues Extra output parameters provided using one
    ///     of the following functions
    ///     @li @ref comms::frame::missingSize()
    ///     @li @ref comms::frame::msgId()
    ///     @li @ref comms::frame::msgIndex()
    ///     @li @ref comms::frame::msgPayload()
    /// @return Status of the read operation.
    template <typename TAllFields, typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readFieldsCached(
        TAllFields& allFields,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TExtraValues... extraValues)
    {

        static_assert(comms::util::IsTuple<TAllFields>::Value,
                                        "Expected TAllFields to be tuple.");

        using AllFieldsDecayed = typename std::decay<TAllFields>::type;
        static_assert(util::tupleIsTailOf<AllFields, AllFieldsDecayed>(), "Passed tuple is wrong.");
        static const std::size_t Idx =
            std::tuple_size<AllFieldsDecayed>::value -
                                std::tuple_size<AllFields>::value;

        static_assert((Idx + 1) == std::tuple_size<TAllFields>::value,
                    "All fields must be read when MsgDataLayer is reached");

        using IterType = typename std::decay<decltype(iter)>::type;
        using IterTag = typename std::iterator_traits<IterType>::iterator_category;
        static_assert(std::is_base_of<std::random_access_iterator_tag, IterTag>::value,
                "Caching read from non random access iterators are not supported at this moment.");

        auto& dataField = std::get<Idx>(allFields);

        using FieldType = typename std::decay<decltype(dataField)>::type;
        static_assert(
            std::is_same<Field, FieldType>::value,
            "Field has wrong type");

        auto dataIter = iter;
        auto es = read(msg, iter, size, extraValues...);
        if (es != ErrorStatus::Success) {
            return es;
        }

        auto dataSize = static_cast<std::size_t>(std::distance(dataIter, iter));
        setMsgPayloadInternal(dataIter, dataSize, extraValues...);

        auto dataEs = dataField.read(dataIter, dataSize);
        static_cast<void>(dataEs);
        COMMS_ASSERT(dataEs == comms::ErrorStatus::Success);
        return es;
    }

    /// @brief Read transport fields with caching until data layer.
    /// @details Does nothing because it is data layer.
    /// @return @ref comms::ErrorStatus::Success;
    template <typename TAllFields, typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readUntilDataFieldsCached(
        TAllFields& allFields,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TExtraValues...)
    {
        static_cast<void>(allFields);
        static_cast<void>(msg);
        static_cast<void>(iter);
        static_cast<void>(size);
        return comms::ErrorStatus::Success;
    }

    /// @brief Same as @ref readFieldsCached().
    /// @details Expected to be called by the privous layers to properly
    ///     finalise read operation after the call to @ref readUntilDataFieldsCached();
    /// @return @ref comms::ErrorStatus::Success;
    template <typename TAllFields, typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readFromDataFeildsCached(
        TAllFields& allFields,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TExtraValues... extraValues)
    {
        return readFieldsCached(allFields, msg, iter, size, extraValues...);
    }

    /// @brief Write the message contents.
    /// @details The way the message contents are written is determined by the
    ///     type of the message. If TMsg type is recognised to be actual message
    ///     inheriting from comms::MessageBase with its fields provided using
    ///     @ref comms::option::def::FieldsImpl option, the function calls @b doWrite
    ///     non-virtual function defined by comms::MessageBase
    ///     (see comms::MessageBase::doWrite) or redefined by the actual
    ///     message itself. Otherwise, TMsg type is expected to be the used
    ///     interface which allows polymorphic write functionality.
    ///     It must define @b write() member function which will be
    ///     called.
    /// @tparam TMsg Type of the message.
    /// @tparam TIter Type of the iterator used for writing.
    /// @param[in] msg Reference to the message object,
    /// @param[in, out] iter Iterator used for writing.
    /// @param[in] size Max number of bytes that can be written.
    /// @return Status of the write operation.
    template <typename TMsg, typename TIter>
    static ErrorStatus write(
        const TMsg& msg,
        TIter& iter,
        std::size_t size)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;

        static_assert(
            comms::isMessage<MsgType>(),
            "The provided message object must inherit from comms::Message");

        using Tag =
            typename comms::util::Conditional<
                comms::isMessageBase<MsgType>()
            >::template Type<
                DirectOpTag<>,
                typename comms::util::Conditional<
                    comms::isMessage<MsgType>()
                >::template Type<
                    InterfaceOpTag<>,
                    typename comms::util::LazyShallowConditional<
                        std::is_pointer<MsgType>::value
                    >::template Type<
                        PointerOpTag,
                        OtherOpTag
                    >
                >
            >;

        return writeInternal(msg, iter, size, Tag());
    }

    /// @brief Write the message contents while caching the written transport
    ///     information fields.
    /// @details Very similar to write() member function, but adds "allFields"
    ///     parameter to store raw data of the message.
    /// @tparam TAllFields std::tuple of all the transport fields, must be
    ///     @ref AllFields type defined in the last layer class that defines
    ///     protocol stack.
    /// @tparam TMsg Type of the message.
    /// @tparam TIter Type of the iterator used for writing.
    /// @param[out] allFields Reference to the std::tuple object that wraps all
    ///     transport fields (@ref AllFields type of the last protocol layer class).
    /// @param[in] msg Reference to the message object that is being written,
    /// @param[in, out] iter Iterator used for writing.
    /// @param[in] size Max number of bytes that can be written.
    /// @return Status of the write operation.
    template <typename TAllFields, typename TMsg, typename TIter>
    static ErrorStatus writeFieldsCached(
        TAllFields& allFields,
        const TMsg& msg,
        TIter& iter,
        std::size_t size)
    {
        static_assert(comms::util::IsTuple<TAllFields>::Value,
                                        "Expected TAllFields to be tuple.");

        using AllFieldsDecayed = typename std::decay<TAllFields>::type;
        static_assert(util::tupleIsTailOf<AllFields, AllFieldsDecayed>(), "Passed tuple is wrong.");
        static const std::size_t Idx =
            std::tuple_size<AllFieldsDecayed>::value -
                                std::tuple_size<AllFields>::value;

        static_assert((Idx + 1) == std::tuple_size<TAllFields>::value,
                    "All fields must be written when MsgDataLayer is reached");

        auto& dataField = std::get<Idx>(allFields);
        using FieldType = typename std::decay<decltype(dataField)>::type;
        static_assert(
            std::is_same<Field, FieldType>::value,
            "Field has wrong type");

        using IterType = typename std::decay<decltype(iter)>::type;
        using IterTag = typename std::iterator_traits<IterType>::iterator_category;
        return writeWithFieldCachedInternal(dataField, msg, iter, size, IterTag());
    }

    /// @brief Update recently written (using write()) message contents data.
    /// @details Sometimes, when NON random access iterator is used for writing
    ///     (for example std::back_insert_iterator), some transport data cannot
    ///     be properly written. In this case, write() function will return
    ///     comms::ErrorStatus::UpdateRequired. When such status is returned
    ///     it is necessary to call update() with random access iterator on
    ///     the written buffer to update written dummy information with
    ///     proper values.
    ///     This function in this layer does nothing, just advances the iterator
    ///     by "size".
    /// @param[in, out] iter Any random access iterator.
    /// @param[in] size Number of bytes that have been written using write().
    /// @return Status of the update operation.
    template <typename TIter>
    static comms::ErrorStatus update(TIter& iter, std::size_t size)
    {
        std::advance(iter, size);
        return comms::ErrorStatus::Success;
    }

    /// @brief Update recently written (using write()) message contents data.
    /// @details Same as other @ref update(), but with extra message parameter/
    /// @param[in] msg Reference to recently writtem message object.
    /// @param[in, out] iter Any random access iterator.
    /// @param[in] size Number of bytes that have been written using write().
    /// @return Status of the update operation.
    template <typename TMsg, typename TIter>
    static comms::ErrorStatus update(const TMsg& msg, TIter& iter, std::size_t size)
    {
        static_cast<void>(msg);
        return update(iter, size);
    }

    /// @brief Update recently written (using writeFieldsCached()) message data as
    ///     well as cached transport information fields.
    /// @details Very similar to update() member function, but adds "allFields"
    ///     parameter to store raw data of the message.
    /// @tparam TAllFields std::tuple of all the transport fields, must be
    ///     @ref AllFields type defined in the last layer class that defines
    ///     protocol stack.
    /// @tparam TIter Type of the random access iterator.
    /// @param[out] allFields Reference to the std::tuple object that wraps all
    ///     transport fields (@ref AllFields type of the last protocol layer class).
    /// @param[in, out] iter Random access iterator to the written data.
    /// @param[in] size Number of bytes that have been written using writeFieldsCached().
    /// @return Status of the update operation.
    template <typename TAllFields, typename TIter>
    static ErrorStatus updateFieldsCached(
        TAllFields& allFields,
        TIter& iter,
        std::size_t size)
    {
        static_cast<void>(allFields);
        std::advance(iter, size);
        return comms::ErrorStatus::Success;
    }

    /// @brief Update recently written (using writeFieldsCached()) message data as
    ///     well as cached transport information fields.
    /// @details Very similar to other @ref updateFieldsCached() member function, 
    ///     but adds "msg" parameter to reference message object if needed.
    /// @tparam TAllFields std::tuple of all the transport fields, must be
    ///     @ref AllFields type defined in the last layer class that defines
    ///     protocol stack.
    /// @tparam TMsg Type of @b msg parameter.
    /// @tparam TIter Type of the random access iterator.
    /// @param[out] allFields Reference to the std::tuple object that wraps all
    ///     transport fields (@ref AllFields type of the last protocol layer class).
    /// @param[in] msg Reference to recently written message object.
    /// @param[in, out] iter Random access iterator to the written data.
    /// @param[in] size Number of bytes that have been written using writeFieldsCached().
    /// @return Status of the update operation.
    template <typename TAllFields, typename TMsg, typename TIter>
    static ErrorStatus updateFieldsCached(
        TAllFields& allFields,
        const TMsg& msg,
        TIter& iter,
        std::size_t size)
    {
        static_cast<void>(allFields);
        static_cast<void>(msg);
        std::advance(iter, size);
        return comms::ErrorStatus::Success;
    }

    /// @brief Get remaining length of wrapping transport information.
    /// @details The message data always get wrapped with transport information
    ///     to be successfully delivered to and unpacked on the other side.
    ///     This function return remaining length of the transport information.
    /// @return 0.
    static constexpr std::size_t length()
    {
        return 0U;
    }

    /// @brief Get remaining length of wrapping transport information + length
    ///     of the provided message.
    /// @details This function usually gets called when there is a need to
    ///     identify the size of the buffer required to write provided message
    ///     wrapped in the transport information. This function is very similar
    ///     to length(), but adds also length of the message.
    /// @param[in] msg Message
    /// @return Length of the message.
    template <typename TMsg>
    static constexpr std::size_t length(const TMsg& msg)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;

        static_assert(
            comms::isMessage<MsgType>(),
            "The provided message object must inherit from comms::Message");

        using Tag = 
            typename comms::util::LazyShallowConditional<
                details::FrameLayerHasFieldsImpl<MsgType>::Value
            >::template Type<
                MsgDirectLengthTag,
                MsgHasLengthTag
            >;
        return getMsgLength(msg, Tag());
    }

    /// @brief Access appropriate field from "cached" bundle of all the
    ///     protocol stack fields.
    /// @param allFields All fields of the protocol stack
    /// @return Reference to requested field.
    template <typename TAllFields>
    static auto accessCachedField(TAllFields& allFields) ->
        decltype(std::get<std::tuple_size<typename std::decay<TAllFields>::type>::value - std::tuple_size<AllFields>::value>(allFields))
    {
        using AllFieldsDecayed = typename std::decay<TAllFields>::type;
        static_assert(util::tupleIsTailOf<AllFields, AllFieldsDecayed>(), "Passed tuple is wrong.");
        static const std::size_t Idx =
            std::tuple_size<AllFieldsDecayed>::value -
                                std::tuple_size<AllFields>::value;

        return std::get<Idx>(allFields);
    }

private:
    template <typename... TParams>
    using MsgHasLengthTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using MsgNoLengthTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    using MsgDirectLengthTag = comms::details::tag::Tag3<>;    

    template <typename... TParams>
    using DirectOpTag = comms::details::tag::Tag4<>;     

    template <typename... TParams>
    using InterfaceOpTag = comms::details::tag::Tag5<>;    

    template <typename... TParams>
    using PointerOpTag = comms::details::tag::Tag6<>; 

    template <typename... TParams>
    using OtherOpTag = comms::details::tag::Tag7<>;              

    template <typename TMsg, typename TIter>
    static ErrorStatus writeWithFieldCachedInternal(
        Field& field,
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        std::random_access_iterator_tag)
    {
        return writeWithFieldCachedRandomAccess(field, msg, iter, size);
    }

    template <typename TMsg, typename TIter>
    static ErrorStatus writeWithFieldCachedInternal(
        Field& field,
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        std::output_iterator_tag)
    {
        return writeWithFieldCachedOutput(field, msg, iter, size);
    }

    template <typename TMsg, typename TIter>
    static ErrorStatus writeWithFieldCachedRandomAccess(
        Field& field,
        const TMsg& msg,
        TIter& iter,
        std::size_t size)
    {
        auto dataReadIter = iter;
        auto es = write(msg, iter, size);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        auto writtenCount = static_cast<std::size_t>(std::distance(dataReadIter, iter));
        auto dataEs = field.read(dataReadIter, writtenCount);
        COMMS_ASSERT(dataEs == comms::ErrorStatus::Success);
        static_cast<void>(dataEs);
        return comms::ErrorStatus::Success;
    }

    template <typename TMsg, typename TCollection>
    static ErrorStatus writeWithFieldCachedOutput(
        Field& field,
        const TMsg& msg,
        std::back_insert_iterator<TCollection>& iter,
        std::size_t size)
    {
        auto es = write(msg, iter, size);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        TCollection col;
        auto dataWriteIter = std::back_inserter(col);
        auto dataWriteEs = write(msg, dataWriteIter, size);
        COMMS_ASSERT(dataWriteEs == comms::ErrorStatus::Success);
        static_cast<void>(dataWriteEs);

        auto dataReadIter = col.cbegin();
        auto dataReadEs = field.read(dataReadIter, col.size());
        COMMS_ASSERT(dataReadEs == comms::ErrorStatus::Success);
        static_cast<void>(dataReadEs);

        return comms::ErrorStatus::Success;
    }

    template <typename TMsg, typename... TParams>
    static std::size_t getMsgLength(const TMsg& msg, MsgHasLengthTag<TParams...>)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;
        static_assert(MsgType::hasLength(), "Message interface must define length()");
        return msg.length();
    }

    template <typename TMsg, typename... TParams>
    static constexpr std::size_t getMsgLength(const TMsg& msg, MsgDirectLengthTag<TParams...>)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;
        static_assert(MsgType::hasFields(), "FieldsImpl option hasn't been used");
        return msg.doLength();
    }

    template <typename TMsg, typename... TParams>
    static constexpr std::size_t getMsgLength(const TMsg&, MsgNoLengthTag<TParams...>)
    {
        return 0U;
    }

    template <typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readInternalPolymorphic(
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TExtraValues... extraValues)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;

        static_assert(
            comms::isMessage<MsgType>(),
            "The provided message object must inherit from comms::Message");

        static_assert(MsgType::hasRead(),
            "Message interface must support polymorphic read operation");

        using IterType = typename std::decay<decltype(iter)>::type;

        static_assert(std::is_convertible<IterType, typename MsgType::ReadIterator>::value,
            "The provided iterator is not convertible to ReadIterator defined by Message class");

        using ReadIter = typename std::add_lvalue_reference<typename MsgType::ReadIterator>::type;

        auto result = msg.read(static_cast<ReadIter>(iter), size);
        if ((result == ErrorStatus::NotEnoughData) &&
            missingSizeRequiredInternal(extraValues...)) {
            using Tag = 
                typename comms::util::LazyShallowConditional<
                    MsgType::hasLength()
                >::template Type<
                    MsgHasLengthTag,
                    MsgNoLengthTag
                >;

            std::size_t missingSize = 1U;
            auto msgLen = getMsgLength(msg, Tag());
            if (size < msgLen) {
                missingSize = msgLen - size;
            }
            updateMissingSizeInternal(missingSize, extraValues...);
        }
        return result;
    }

    template <typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readInternalDirect(
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TExtraValues... extraValues)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;

        static_assert(
            comms::isMessageBase<MsgType>(),
            "The provided message object must inherit from comms::MessageBase");

        static_assert(details::frameLayerHasFieldsImpl<MsgType>(),
            "Message class must use FieldsImpl option");

        auto result = msg.doRead(iter, size);
        if ((result == ErrorStatus::NotEnoughData) &&
            (missingSizeRequiredInternal(extraValues...))) {
            std::size_t missingSize = 1U;
            auto msgLen = getMsgLength(msg, MsgDirectLengthTag<>());
            if (size < msgLen) {
                missingSize = msgLen - size;
            }
            updateMissingSizeInternal(missingSize, extraValues...);
        }
        return result;
    }

    template <typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readInternal(
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        DirectOpTag<>,
        TExtraValues... extraValues)
    {
        return readInternalDirect(msg, iter, size, extraValues...);
    }

    template <typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readInternal(
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        InterfaceOpTag<>,
        TExtraValues... extraValues)
    {
        static_assert(comms::isMessage<TMsg>(), "Must be interface class");
        static_assert(TMsg::hasRead(),
            "Message interface must support polymorphic read operation");

        return readInternalPolymorphic(msg, iter, size, extraValues...);
    }

    template <typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readInternal(
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        PointerOpTag<>,
        TExtraValues... extraValues)
    {
        return read(*msg, iter, size, extraValues...);
    }

    template <typename TMsg, typename TIter, typename... TExtraValues>
    static ErrorStatus readInternal(
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        OtherOpTag<>,
        TExtraValues... extraValues)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;
        static_assert(comms::details::hasElementType<MsgType>(),
            "Unsupported type of message object, expected to be either message itself, raw pointer or smart pointer");
        return read(*msg, iter, size, extraValues...);
    }

    template <typename TMsg, typename TIter, typename... TParams>
    static ErrorStatus writeInternal(
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        DirectOpTag<TParams...>)
    {
        return msg.doWrite(iter, size);
    }

    template <typename TMsg, typename TIter, typename... TParams>
    static ErrorStatus writeInternal(
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        InterfaceOpTag<TParams...>)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;

        static_assert(MsgType::hasWrite(),
            "Message interface must support polymorphic write operation");

        using IterType = typename std::decay<decltype(iter)>::type;

        static_assert(std::is_convertible<IterType, typename MsgType::WriteIterator>::value,
            "The provided iterator is not convertible to WriteIterator defined by Message class");

        using WriteIter = typename std::add_lvalue_reference<typename MsgType::WriteIterator>::type;

        return msg.write(static_cast<WriteIter>(iter), size);
    }

    template <typename TMsg, typename TIter, typename... TParams>
    static ErrorStatus writeInternal(
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        PointerOpTag<TParams...>)
    {
        return write(*msg, iter, size);
    }

    template <typename TMsg, typename TIter, typename... TParams>
    static ErrorStatus writeInternal(
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        OtherOpTag<TParams...>)
    {
        using MsgType = typename std::decay<decltype(msg)>::type;
        static_assert(comms::details::hasElementType<MsgType>(),
            "Unsupported type of message object, expected to be either message itself, raw pointer or smart pointer");

        return write(*msg, iter, size);
    }

    static constexpr bool missingSizeRequiredInternal()
    {
        return false;
    }

    template <typename... TExtraValues>
    static bool missingSizeRequiredInternal(details::MissingSizeRetriever<>, TExtraValues...)
    {
        return true;
    }

    template <typename T, typename... TExtraValues>
    static bool missingSizeRequiredInternal(T, TExtraValues... extraValues)
    {
        static_assert(
            !details::isMissingSizeRetriever<T>(),
            "Mustn't be missing size retriever");
        return missingSizeRequiredInternal(extraValues...);
    }

    static void updateMissingSizeInternal(std::size_t val)
    {
        static_cast<void>(val);
    }

    template <typename... TExtraValues>
    static void updateMissingSizeInternal(
        std::size_t val,
        details::MissingSizeRetriever<> retriever,
        TExtraValues... extraValues)
    {
        retriever.setValue(val);
        updateMissingSizeInternal(val, extraValues...);
    }

    template <typename T, typename... TExtraValues>
    static void updateMissingSizeInternal(std::size_t val, T retriever, TExtraValues... extraValues)
    {
        static_cast<void>(retriever);
        static_assert(
            !details::isMissingSizeRetriever<typename std::decay<decltype(retriever)>::type>(),
            "Mustn't be missing size retriever");
        updateMissingSizeInternal(val, extraValues...);
    }

    static constexpr bool setPayloadRequiredInternal()
    {
        return false;
    }

    template <typename TIter, typename... TExtraValues>
    static bool setPayloadRequiredInternal(details::MsgPayloadRetriever<TIter>, TExtraValues...)
    {
        return true;
    }

    template <typename T, typename... TExtraValues>
    static bool setPayloadRequiredInternal(T, TExtraValues... extraValues)
    {
        static_assert(
            !details::isMsgPayloadRetriever<T>(),
            "Mustn't be message payload retriever");
        return setPayloadRequiredInternal(extraValues...);
    }

    template <typename TIter>
    static void setMsgPayloadInternal(TIter iter, std::size_t len)
    {
        static_cast<void>(iter);
        static_cast<void>(len);
    }

    template <typename TIter, typename TOtherIter, typename... TExtraValues>
    static void setMsgPayloadInternal(
        TIter iter,
        std::size_t len,
        details::MsgPayloadRetriever<TOtherIter> retriever,
        TExtraValues... extraValues)
    {
        retriever.setValue(iter, len);
        setMsgPayloadInternal(iter, len, extraValues...);
    }

    template <typename TIter, typename T, typename... TExtraValues>
    static void setMsgPayloadInternal(
        TIter iter,
        std::size_t len,
        T retriever,
        TExtraValues... extraValues)
    {
        static_cast<void>(retriever);
        static_assert(
            !details::isMsgPayloadRetriever<typename std::decay<decltype(retriever)>::type>(),
            "Mustn't be message payload retriever");
        setMsgPayloadInternal(iter, len, extraValues...);
    }
};

namespace details
{
template <typename T>
struct MsgDataLayerCheckHelper
{
    static const bool Value = false;
};

template <typename... TExtraOpts>
struct MsgDataLayerCheckHelper<MsgDataLayer<TExtraOpts...> >
{
    static const bool Value = true;
};

} // namespace details

/// @brief Compile time check of whether the provided type is
///     a variant of @ref MsgDataLayer
/// @related MsgDataLayer
template <typename T>
constexpr bool isMsgDataLayer()
{
    return details::MsgDataLayerCheckHelper<T>::Value;
}

template <typename... TExtraOpts>
constexpr
MsgDataLayer<TExtraOpts...>&
toFrameLayerBase(MsgDataLayer<TExtraOpts...>& layer)
{
    return layer;
}

template <typename... TExtraOpts>
constexpr
const MsgDataLayer<TExtraOpts...>&
toFrameLayerBase(const MsgDataLayer<TExtraOpts...>& layer)
{
    return layer;
}

}  // namespace frame

}  // namespace comms



