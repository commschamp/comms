//
// Copyright 2015 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of @ref comms::frame::SyncPrefixLayer

#pragma once

#include "comms/CompileControl.h"
#include "comms/frame/details/SyncPrefixLayerBase.h"
#include "comms/frame/details/SyncPrefixLayerOptionsParser.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

COMMS_MSVC_WARNING_PUSH
COMMS_MSVC_WARNING_DISABLE(4189) // Disable erroneous initialized but not referenced variable warning

namespace comms
{

namespace frame
{

/// @brief Frame layer that uses "sync" field as a prefix to all the
///        subsequent data written by other (next) layers.
/// @details The main purpose of this layer is to provide a constant synchronisation
///     prefix to help identify the beginning of the serialised message.
///     This layer is a mid level layer, expects other mid level layer or
///     MsgDataLayer to be its next one.
/// @tparam TField Type of the field that is used as sync prefix. The "sync"
///     field type definition must use options (@ref comms::option::def::DefaultNumValue)
///     to specify its default value to be equal to the expected "sync" value.
/// @tparam TNextLayer Next transport layer in frame.
/// @tparam TOptions Extending functionality options. Supported options are:
///     @li  @ref comms::option::ExtendingClass - Use this option to provide a class
///         name of the extending class, which can be used to extend existing functionality.
///         See also @ref page_custom_sync_prefix_layer.
///     @li @ref comms::option::def::FrameLayerSeekField - By default, the
///         @b SyncSuffixLayer will invoke @b read operation of inner (wrapped) layers
///         and only if it is successful. Usage of @ref comms::option::def::FrameLayerSeekField
///         modifies the default behaviour by forcing layer to seek the field in the
///         buffer (until the successful read and verification)
///         prior to invocation of @b read operation in the wrapped layer(s).
/// @headerfile comms/frame/SyncPrefixLayer.h
/// @extends comms::frame::FrameLayerBase
template <typename TField, typename TNextLayer, typename... TOptions>
class SyncPrefixLayer : public comms::frame::details::SyncPrefixLayerBase<TField, TNextLayer, TOptions...>
{
    using BaseImpl = comms::frame::details::SyncPrefixLayerBase<TField, TNextLayer, TOptions...>;
    using ParsedOptionsInternal = comms::frame::details::SyncPrefixLayerOptionsParser<TOptions...>;

public:
    /// @brief Type of the field object used to read/write "sync" value.
    using Field = typename BaseImpl::Field;

    /// @brief Type of the escape field provided via @ref comms::option::def::FrameLayerSeekField option.
    /// @details Same as @b void if not provided via said option.
    using EscField = typename ParsedOptionsInternal::EscField;

    /// @brief Default constructor
    SyncPrefixLayer() = default;

    /// @brief Copy constructor.
    SyncPrefixLayer(const SyncPrefixLayer&) = default;

    /// @brief Move constructor.
    SyncPrefixLayer(SyncPrefixLayer&&) = default;

    /// @brief Destructor
    ~SyncPrefixLayer() noexcept = default;

    /// @brief Compile time inquiry of whether this class was extended via
    ///    @ref comms::option::ExtendingClass option.
    static constexpr bool hasExtendingClass()
    {
        return ParsedOptionsInternal::HasExtendingClass;
    }

    /// @brief Customized read functionality, invoked by @ref comms::frame::FrameLayerBase::read() "read()".
    /// @details Reads the "sync" value from the input data. If the read value
    ///     is NOT as expected (doesn't equal to the default constructed
    ///     @ref Field), then comms::ErrorStatus::ProtocolError is returned.
    ////    If the read "sync" value as expected, the read() member function of
    ///     the next layer is called.
    /// @tparam TMsg Type of the @b msg parameter.
    /// @tparam TIter Type of iterator used for reading.
    /// @tparam TNextLayerReader Next layer reader object type.
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
    /// @pre Iterator must be valid and can be dereferenced and incremented at
    ///      least "size" times;
    /// @post The iterator will be advanced by the number of bytes was actually
    ///       read. In case of an error, distance between original position and
    ///       advanced will pinpoint the location of the error.
    template <typename TMsg, typename TIter, typename TNextLayerReader, typename... TExtraValues>
    comms::ErrorStatus doRead(
        Field& field,
        TMsg& msg,
        TIter& iter,
        std::size_t size,
        TNextLayerReader&& nextLayerReader,
        TExtraValues... extraValues)
    {
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
    /// @details The function will write proper "sync" value to the output
    ///     buffer, then call the write() function of the next layer.
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
    comms::ErrorStatus doWrite(
        Field& field,
        const TMsg& msg,
        TIter& iter,
        std::size_t size,
        TNextLayerWriter&& nextLayerWriter) const
    {
        auto& thisObj = BaseImpl::thisLayer();
        thisObj.prepareFieldForWrite(field);
        auto es = thisObj.doWriteField(&msg, field, iter, size);
        if (es != ErrorStatus::Success) {
            return es;
        }

        COMMS_ASSERT(field.length() <= size);
        return nextLayerWriter.write(msg, iter, size - field.length());
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

    /// @brief Verify the validity of the escapte field provided via @ref comms::option::def::FrameLayerSeekField option.
    /// @details Default implementation compares read field with default constructed type. @n
    ///     May be overridden by the extending class in case
    ///     more complex logic is required.
    /// @param[out] field Field that has been read.
    /// @note May be non-static in the extending class
    template <typename TEscField>
    static bool verifyEscFieldValue(const TEscField& field)
    {
        return field == TEscField();
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
    using EscapeSupportedTag = comms::details::tag::Tag3<>;

    template <typename... TParams>
    using NoEscapeTag = comms::details::tag::Tag4<>;

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
        using IterType = typename std::decay<decltype(iter)>::type;
        static_assert(std::is_same<typename std::iterator_traits<IterType>::iterator_category, std::random_access_iterator_tag>::value,
            "The read operation is expected to use random access iterator");

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
                (!fieldEscapedInternal(fromIter, iter)) &&
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

        auto remSize = size - static_cast<std::size_t>(std::distance(fromIter, iter));
        return nextLayerReader.read(msg, iter, remSize, extraValues...);
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
        auto& thisObj = BaseImpl::thisLayer();
        auto* msgPtr = BaseImpl::toMsgPtr(msg);
        auto beforeReadIter = iter;

        auto es = thisObj.doReadField(msgPtr, field, iter, size);
        if (es == comms::ErrorStatus::NotEnoughData) {
            BaseImpl::updateMissingSize(field, size, extraValues...);
        }

        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        bool verified = thisObj.verifyFieldValue(field);
        if (!verified) {
            return comms::ErrorStatus::ProtocolError;
        }

        auto fieldLen = static_cast<std::size_t>(std::distance(beforeReadIter, iter));
        return nextLayerReader.read(msg, iter, size - fieldLen, extraValues...);
    }

    template <typename TIter>
    bool fieldEscapedInternal(TIter from, TIter to, NoEscapeTag<>)
    {
        static_cast<void>(from);
        static_cast<void>(to);
        return false;
    }

    template <typename TIter>
    bool fieldEscapedInternal(TIter from, TIter to, EscapeSupportedTag<>)
    {
        unsigned escCount = 0U;
        auto& thisObj = BaseImpl::thisLayer();

        while (from != to) {
            auto dist = static_cast<std::size_t>(std::distance(from, to));
            dist = std::min(dist, EscField::maxLength());
            if (dist < EscField::minLength()) {
                break;
            }

            auto maxLenIter = to;
            std::advance(maxLenIter, -static_cast<int>(dist));

            auto iter = to;
            std::advance(iter, -static_cast<int>(EscField::minLength()));
            auto prevCount = escCount;
            while (true) {
                auto iterTmp = iter;
                EscField escField;
                auto len = static_cast<std::size_t>(std::distance(iterTmp, to));
                auto es = escField.read(iterTmp, len);
                if ((es == comms::ErrorStatus::Success) &&
                    (iterTmp == to) &&
                    (thisObj.verifyEscFieldValue(escField))) {
                    ++escCount;
                    break;
                }

                if (iter == maxLenIter) {
                    break;
                }

                std::advance(iter, -1);
            }

            if (prevCount == escCount) {
                // Failed to detect escField
                break;
            }

            // esc field was detected, trying again
            to = iter;
        }

        // Escaped only if odd number of esc field is detected
        return (escCount & 0x1) != 0U;
    }

    template <typename TIter>
    bool fieldEscapedInternal(TIter from, TIter to)
    {
        using EscTag =
            typename comms::util::LazyShallowConditional<
                std::is_same<EscField, void>::value
            >::template Type<
                NoEscapeTag,
                EscapeSupportedTag
            >;

        return fieldEscapedInternal(from, to, EscTag());
    }
};

namespace details
{
template <typename T>
struct SyncPrefixLayerCheckHelper
{
    static const bool Value = false;
};

template <typename TField, typename TNextLayer>
struct SyncPrefixLayerCheckHelper<SyncPrefixLayer<TField, TNextLayer> >
{
    static const bool Value = true;
};

} // namespace details

/// @brief Compile time check of whether the provided type is
///     a variant of @ref SyncPrefixLayer
/// @related SyncPrefixLayer
template <typename T>
constexpr bool isSyncPrefixLayer()
{
    return details::SyncPrefixLayerCheckHelper<T>::Value;
}

}  // namespace frame

}  // namespace comms

COMMS_MSVC_WARNING_POP
