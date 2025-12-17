//
// Copyright 2025 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of @ref comms::frame::SyncSuffixLayer

#pragma once

#include "comms/cast.h"
#include "comms/CompileControl.h"
#include "comms/details/tag.h"
#include "comms/field/IntValue.h"
#include "comms/frame/details/SyncSuffixLayerBase.h"
#include "comms/frame/details/SyncSuffixLayerOptionsParser.h"
#include "comms/util/type_traits.h"

#include <iterator>
#include <type_traits>
#include <utility>

COMMS_MSVC_WARNING_PUSH
COMMS_MSVC_WARNING_DISABLE(4189) // Disable erroneous initialized but not referenced variable warning

namespace comms
{

namespace frame
{
/// @brief Frame layer that uses "sync" field as a suffix (after message payload) to all the
///        preceding data written by other (next) layers.
/// @details The main purpose of this layer is to provide a constant synchronisation
///     suffix to help identify the end of the serialised message.
///     This layer is a mid level layer, expects other mid level layer or
///     MsgDataLayer to be its next one.
/// @tparam TField Type of the field that is used as to represent checksum value.
/// @tparam TNextLayer Next transport layer in frame.
/// @tparam TOptions Extending functionality options. Supported options are:
///     @li  @ref comms::option::ExtendingClass - Use this option to provide a class
///         name of the extending class, which can be used to extend existing functionality.
///         See also @ref page_custom_checksum_layer tutorial page.
///     @li @ref comms::option::def::FrameLayerVerifyBeforeRead - By default, the
///         @b SyncSuffixLayer will invoke @b read operation of inner (wrapped) layers
///         and only if it is successful. Usage of @ref comms::option::def::FrameLayerVerifyBeforeRead
///         modifies the default behaviour by forcing the field value verification
///         prior to invocation of @b read operation in the wrapped layer(s).
///     @li @ref comms::option::def::FrameLayerSeekField - By default, the
///         @b SyncSuffixLayer will invoke @b read operation of inner (wrapped) layers
///         and only if it is successful. Usage of @ref comms::option::def::FrameLayerSeekField
///         modifies the default behaviour by forcing layer to seek the field in the
///         buffer (until the successful read and verification)
///         prior to invocation of @b read operation in the wrapped layer(s).
/// @headerfile comms/frame/SyncSuffixLayer.h
/// @extends comms::frame::FrameLayerBase
template <typename TField, typename TNextLayer, typename... TOptions>
class SyncSuffixLayer : public comms::frame::details::SyncSuffixLayerBase<TField, TNextLayer, TOptions...>
{
    using BaseImpl = comms::frame::details::SyncSuffixLayerBase<TField, TNextLayer, TOptions...>;
    using ParsedOptionsInternal = details::SyncSuffixLayerOptionsParser<TOptions...>;

public:
    /// @brief Type of the field object used to read/write checksum value.
    using Field = typename BaseImpl::Field;

    /// @brief Default constructor.
    SyncSuffixLayer() = default;

    /// @brief Copy constructor
    SyncSuffixLayer(const SyncSuffixLayer&) = default;

    /// @brief Move constructor
    SyncSuffixLayer(SyncSuffixLayer&&) = default;

    /// @brief Destructor.
    ~SyncSuffixLayer() noexcept = default;

    /// @brief Copy assignment
    SyncSuffixLayer& operator=(const SyncSuffixLayer&) = default;

    /// @brief Move assignment
    SyncSuffixLayer& operator=(SyncSuffixLayer&&) = default;

    /// @brief Compile time inquiry of whether this class was extended via
    ///    @ref comms::option::ExtendingClass option.
    /// @details If @b true is returned, the @ref SyncPrefixLayer::ExtendingClass "ExtendingClass"
    ///     type aliasing the real layer type.
    static constexpr bool hasExtendingClass()
    {
        return ParsedOptionsInternal::HasExtendingClass;
    }

    /// @brief Compile time inquiry of whether @ref comms::option::def::FrameLayerVerifyBeforeRead
    ///     options has been used.
    static constexpr bool hasVerifyBeforeRead()
    {
        return ParsedOptionsInternal::HasVerifyBeforeRead;
    }

    /// @brief Compile time inquiry of whether @ref comms::option::def::FrameLayerSeekField
    ///     options has been used.
    static constexpr bool hasSeekField()
    {
        return ParsedOptionsInternal::HasSeekField;
    }

    /// @brief Customized read functionality, invoked by @ref read().
    /// @details First, executes the read() member function of the next layer.
    ///     If the call returns comms::ErrorStatus::Success, it reads and verifies the
    ///     field value. If the field's value is as expected,
    ///     comms::ErrorStatus::Success is returned, otherwise
    ///     function returns comms::ErrorStatus::ProtocolError.
    /// @tparam TMsg Type of @b msg parameter.
    /// @tparam TIter Type of iterator used for reading.
    /// @tparam TNextLayerReader next layer reader object type.
    /// @param[out] field Field object to read.
    /// @param[in, out] msg Reference to smart pointer, that already holds or
    ///     will hold allocated message object, or reference to actual message
    ///     object (which extends @ref comms::MessageBase).
    /// @param[in, out] iter Input iterator used for reading.
    /// @param[in] size Size of the data in the sequence
    /// @param[in] nextLayerReader Reader object, needs to be invoked to
    ///     forward read operation to the next layer.
    /// @param[out] extraValues Variadic extra output parameters passed to the
    ///     "read" operatation of the frame (see
    ///     @ref comms::frame::FrameLayerBase::read() "read()" and
    ///     @ref comms::frame::FrameLayerBase::readFieldsCached() "readFieldsCached()").
    ///     Need to passed on as variadic arguments to the @b nextLayerReader.
    /// @return Status of the read operation.
    /// @pre Iterator must be a "random access" one.
    /// @pre Iterator must be valid and can be dereferenced and incremented at
    ///      least "size" times;
    /// @post The iterator will be advanced by the number of bytes was actually
    ///       read. In case of an error, distance between original position and
    ///       advanced will pinpoint the location of the error.
    template <typename TMsg, typename TIter, typename TNextLayerReader, typename... TExtraValues>
    ErrorStatus doRead(
        Field& field,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TNextLayerReader&& nextLayerReader,
        TExtraValues... extraValues)
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        static_assert(std::is_same<typename std::iterator_traits<IterType>::iterator_category, std::random_access_iterator_tag>::value,
            "The read operation is expected to use random access iterator");

        using SeekTag =
            typename comms::util::LazyShallowConditional<
                ParsedOptionsInternal::HasSeekField
            >::template Type<
                SeekFieldTag,
                InstantOpTag
            >;

        return
            readInternal(
                field,
                msg,
                iter,
                size,
                std::forward<TNextLayerReader>(nextLayerReader),
                SeekTag(),
                extraValues...);
    }

    /// @brief Customized write functionality, invoked by @ref comms::frame::FrameLayerBase::write() "write()".
    /// @details The function will call the write() function of the next layer and then will write its sync field value.
    /// @tparam TMsg Type of message object.
    /// @tparam TIter Type of iterator used for writing.
    /// @tparam TNextLayerWriter next layer writer object type.
    /// @param[out] field Field object to update and write.
    /// @param[in] msg Reference to message object
    /// @param[in, out] iter Output iterator.
    /// @param[in] size Max number of bytes that can be written.
    /// @param[in] nextLayerWriter Next layer writer object.
    /// @return Status of the write operation.
    /// @pre Iterator must be valid and can be dereferenced and incremented at
    ///      least "size" times;
    /// @post The iterator will be advanced by the number of bytes was actually
    ///       written. In case of an error, distance between original position
    ///       and advanced will pinpoint the location of the error.
    template <typename TMsg, typename TIter, typename TNextLayerWriter>
    ErrorStatus doWrite(
        Field& field,
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        TNextLayerWriter&& nextLayerWriter) const
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using Tag = typename std::iterator_traits<IterType>::iterator_category;

        return writeInternal(field, msg, iter, size, std::forward<TNextLayerWriter>(nextLayerWriter), Tag());
    }

protected:
    /// @brief Verify the validity of the field.
    /// @details Default implementation compares read field with default constructed Field type. @n
    ///     May be overridden by the extending class in case
    ///     more complex logic is required.
    /// @param[out] field Field that has been read.
    /// @note May be non-static in the extending class
    static bool verifyFieldValue(const Field& field)
    {
        return field == Field();
    }

    /// @brief Prepare field for writing.
    /// @details Default implementation does nothing. @n
    ///     May be overridden by the extending class in case
    ///     more complex logic is required.
    /// @param[out] field Field, default value of which needs to be (re)populated
    /// @note May be non-static in the extending class, but must be const, use
    ///     mutable member data in case it needs to be updated.
    static void prepareFieldForWrite(Field& field)
    {
        static_cast<void>(field);
    }

private:
    template <typename... TParams>
    using SeekFieldTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using InstantOpTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    using VerifyBeforeReadTag = comms::details::tag::Tag3<>;

    template <typename... TParams>
    using VerifyAfterReadTag = comms::details::tag::Tag4<>;

    template <typename TMsg, typename TIter, typename TReader, typename... TExtraValues>
    ErrorStatus verifyRead(
        Field& field,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TReader&& nextLayerReader,
        TExtraValues... extraValues)
    {
        auto fromIter = iter;
        auto* msgPtr = BaseImpl::toMsgPtr(msg);
        auto fieldLen = Field::minLength();

        static_assert(Field::minLength() == Field::maxLength(), "Cannot verify variable length suffix before reading rest of the message frame");
        auto& thisObj = BaseImpl::thisLayer();
        if (msgPtr != nullptr) {
            fieldLen = thisObj.doFieldLength(*msgPtr);
        }
        auto toIter = fromIter + (size - fieldLen);

        auto fieldEs = thisObj.doReadField(msgPtr, field, toIter, fieldLen);
        if (fieldEs != ErrorStatus::Success) {
            return fieldEs;
        }

        bool verified = thisObj.verifyFieldValue(field);
        if (!verified) {
            return comms::ErrorStatus::ProtocolError;
        }

        auto es = nextLayerReader.read(msg, iter, size - fieldLen, extraValues...);
        if (es == ErrorStatus::Success) {
            iter = toIter;
        }

        return es;
    }

    template <typename TMsg, typename TIter, typename TReader, typename... TExtraValues>
    ErrorStatus readVerify(
        Field& field,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TReader&& nextLayerReader,
        TExtraValues... extraValues)
    {
        auto fromIter = iter;

        auto es = nextLayerReader.read(msg, iter, size, extraValues...);
        if ((es == ErrorStatus::NotEnoughData) ||
            (es == ErrorStatus::ProtocolError)) {
            return es;
        }

        auto len = static_cast<std::size_t>(std::distance(fromIter, iter));
        COMMS_ASSERT(len <= size);
        auto remSize = size - len;
        auto* msgPtr = BaseImpl::toMsgPtr(msg);
        auto& thisObj = BaseImpl::thisLayer();
        auto fieldEs = thisObj.doReadField(msgPtr, field, iter, remSize);
        if (fieldEs == ErrorStatus::NotEnoughData) {
            BaseImpl::updateMissingSize(field, remSize, extraValues...);
        }

        if (fieldEs != ErrorStatus::Success) {
            BaseImpl::resetMsg(msg);
            return fieldEs;
        }

        bool verified = thisObj.verifyFieldValue(field);
        if (!verified) {
            return comms::ErrorStatus::ProtocolError;
        }

        return es;
    }

    template <typename TMsg, typename TIter, typename TReader, typename... TExtraValues>
    ErrorStatus seekFieldRead(
        Field& field,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TReader&& nextLayerReader,
        TExtraValues... extraValues)
    {
        auto& thisObj = BaseImpl::thisLayer();
        auto* msgPtr = BaseImpl::toMsgPtr(msg);

        auto fromIter = iter;
        std::size_t consumed = 0U;
        while (consumed < size) {
            auto iterTmp = iter;
            auto remSize = size - consumed;

            auto fieldEs = thisObj.doReadField(msgPtr, field, iterTmp, remSize);
            if (fieldEs == ErrorStatus::NotEnoughData) {
                BaseImpl::updateMissingSize(field, remSize, extraValues...);
                BaseImpl::resetMsg(msg);
                return fieldEs;
            }

            if ((fieldEs == ErrorStatus::Success) &&
                (thisObj.verifyFieldValue(field))) {
                // Set iter to point after field
                iter = iterTmp;
                break;
            }

            ++iter;
            ++consumed;
        }

        if (size <= consumed) {
            // Field hasn't been recognized
            return ErrorStatus::NotEnoughData;
        }

        // iter is already past the sync field
        return nextLayerReader.read(msg, fromIter, consumed, extraValues...);
    }

    template <typename TMsg, typename TIter, typename TReader, typename... TExtraValues>
    ErrorStatus readInternal(
        Field& field,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TReader&& nextLayerReader,
        SeekFieldTag<>,
        TExtraValues... extraValues)
    {
        return
            seekFieldRead(
                field,
                msg,
                iter,
                size,
                std::forward<TReader>(nextLayerReader),
                extraValues...);
    }

    template <typename TMsg, typename TIter, typename TReader, typename... TExtraValues>
    ErrorStatus readInternal(
        Field& field,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TReader&& nextLayerReader,
        InstantOpTag<>,
        TExtraValues... extraValues)
    {
        using VerifyTag =
            typename comms::util::LazyShallowConditional<
                ParsedOptionsInternal::HasVerifyBeforeRead
            >::template Type<
                VerifyBeforeReadTag,
                VerifyAfterReadTag
            >;

        return
            readInternal(
                field,
                msg,
                iter,
                size,
                std::forward<TReader>(nextLayerReader),
                VerifyTag(),
                extraValues...);
    }

    template <typename TMsg, typename TIter, typename TReader, typename... TExtraValues>
    ErrorStatus readInternal(
        Field& field,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TReader&& nextLayerReader,
        VerifyBeforeReadTag<>,
        TExtraValues... extraValues)
    {
        return
            verifyRead(
                field,
                msg,
                iter,
                size,
                std::forward<TReader>(nextLayerReader),
                extraValues...);
    }

    template <typename TMsg, typename TIter, typename TReader, typename... TExtraValues>
    ErrorStatus readInternal(
        Field& field,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TReader&& nextLayerReader,
        VerifyAfterReadTag<>,
        TExtraValues... extraValues)
    {
        return
            readVerify(
                field,
                msg,
                iter,
                size,
                std::forward<TReader>(nextLayerReader),
                extraValues...);
    }

    template <typename TMsg, typename TIter, typename TWriter>
    ErrorStatus writeInternalRandomAccess(
        Field& field,
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        TWriter&& nextLayerWriter) const
    {
        auto fromIter = iter;
        auto es = nextLayerWriter.write(msg, iter, size);
        if ((es != comms::ErrorStatus::Success) &&
            (es != comms::ErrorStatus::UpdateRequired)) {
            return es;
        }

        COMMS_ASSERT(fromIter <= iter);
        auto len = static_cast<std::size_t>(std::distance(fromIter, iter));
        auto remSize = size - len;
        auto& thisObj = BaseImpl::thisLayer();

        thisObj.prepareFieldForWrite(field);
        auto esTmp = thisObj.doWriteField(&msg, field, iter, remSize);
        if (esTmp != comms::ErrorStatus::Success) {
            return esTmp;
        }

        return es;
    }

    template <typename TMsg, typename TIter, typename TWriter>
    ErrorStatus writeInternalOutput(
        Field& field,
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        TWriter&& nextLayerWriter) const
    {
        auto& thisObj = BaseImpl::thisLayer();
        auto fieldLen = thisObj.doFieldLength(msg);
        auto es = nextLayerWriter.write(msg, iter, size - fieldLen);
        if ((es != comms::ErrorStatus::Success) &&
            (es != comms::ErrorStatus::UpdateRequired)) {
            return es;
        }

        auto esTmp = thisObj.writeField(&msg, field, iter, fieldLen);
        static_cast<void>(esTmp);
        COMMS_ASSERT(esTmp == comms::ErrorStatus::Success);
        return comms::ErrorStatus::UpdateRequired;
    }

    template <typename TMsg, typename TIter, typename TWriter>
    ErrorStatus writeInternal(
        Field& field,
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        TWriter&& nextLayerWriter,
        std::random_access_iterator_tag) const
    {
        return writeInternalRandomAccess(field, msg, iter, size, std::forward<TWriter>(nextLayerWriter));
    }

    template <typename TMsg, typename TIter, typename TWriter>
    ErrorStatus writeInternal(
        Field& field,
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        TWriter&& nextLayerWriter,
        std::output_iterator_tag) const
    {
        return writeInternalOutput(field, msg, iter, size, std::forward<TWriter>(nextLayerWriter));
    }

    template <typename TMsg, typename TIter>
    ErrorStatus fieldUpdateInternal(const TMsg* msgPtr, TIter from, TIter to, std::size_t size, Field& field) const
    {
        COMMS_ASSERT(from <= to);
        auto len = static_cast<std::size_t>(std::distance(from, to));
        auto& thisObj = BaseImpl::thisLayer();
        if (msgPtr != nullptr) {
            COMMS_ASSERT(len == (size - thisObj.doFieldLength(*msgPtr)));
        }
        else {
            COMMS_ASSERT(len == (size - Field::maxLength()));
        }
        auto remSize = size - len;

        bool checksumValid = false;
        auto checksum =
            thisObj.calculateSyncSuffix(
                msgPtr,
                from,
                len,
                checksumValid);

        if (!checksumValid) {
            return comms::ErrorStatus::ProtocolError;
        }

        thisObj.prepareFieldForWrite(checksum, msgPtr, field);
        return thisObj.writeField(msgPtr, field, to, remSize);
    }

    static_assert(!(hasVerifyBeforeRead() && hasSeekField()),
        "Usage of both comms::option::def::FrameLayerVerifyBeforeRead and comms::option::def::FrameLayerSeekField at the same time are not allowed");
};

namespace details
{
template <typename T>
struct SyncSuffixLayerCheckHelper
{
    static const bool Value = false;
};

template <typename TField, typename TNextLayer, typename... TOptions>
struct SyncSuffixLayerCheckHelper<SyncSuffixLayer<TField, TNextLayer, TOptions...> >
{
    static const bool Value = true;
};

} // namespace details

/// @brief Compile time check of whether the provided type is
///     a variant of @ref SyncSuffixLayer
/// @related SyncSuffixLayer
template <typename T>
constexpr bool isSyncSuffixLayer()
{
    return details::SyncSuffixLayerCheckHelper<T>::Value;
}

}  // namespace frame

}  // namespace comms

COMMS_MSVC_WARNING_POP
