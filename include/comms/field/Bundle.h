//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of @ref comms::field::Bundle

#pragma once

#include "comms/ErrorStatus.h"
#include "comms/field/basic/Bundle.h"
#include "comms/field/details/AdaptBasicField.h"
#include "comms/options.h"
#include "comms/util/Tuple.h"

#include <utility>

namespace comms
{

namespace field
{

/// @brief Bundles multiple fields into a single field.
/// @details The class wraps nicely multiple fields and provides
///     expected single field API functions, such as length(), read(), write(),
///     valid(). It may be useful when a collection (comms::field::ArrayList) of
///     complex fields is required.
///
///     Refer to @ref sec_field_tutorial_bundle for tutorial and usage examples.
/// @tparam TFieldBase Base class for this field, expected to be a variant of
///     comms::Field.
/// @tparam TMembers All wrapped fields bundled together in
///     <a href="http://en.cppreference.com/w/cpp/utility/tuple">std::tuple</a>.
/// @tparam TOptions Zero or more options that modify/refine default behaviour
///     of the field.@n
///     Supported options are:
///     @li @ref comms::option::def::DefaultValueInitialiser
///     @li @ref comms::option::def::EmptySerialization
///     @li @ref comms::option::def::FieldType
///     @li @ref comms::option::def::FixedValue
///     @li @ref comms::option::def::HasCustomRead
///     @li @ref comms::option::def::HasCustomRefresh
///     @li @ref comms::option::def::HasName
///     @li @ref comms::option::def::HasVersionDependentMembers
///     @li @ref comms::option::def::RemLengthMemberField
///     @li @ref comms::option::def::VersionStorage
/// @extends comms::Field
/// @headerfile comms/field/Bundle.h
/// @see @ref COMMS_FIELD_MEMBERS_NAMES()
/// @see @ref COMMS_FIELD_MEMBERS_ACCESS()
/// @see @ref COMMS_FIELD_MEMBERS_ACCESS_NOTEMPLATE()
/// @see @ref COMMS_FIELD_ALIAS()
template <typename TFieldBase, typename TMembers, typename... TOptions>
class Bundle : public 
    details::AdaptBasicFieldT<
        basic::Bundle<
            TFieldBase, 
            details::OptionsParser<TOptions...>::ForcedMembersVersionDependency,
            TMembers>, 
        TOptions...
    >
{
    using BaseImpl = 
        details::AdaptBasicFieldT<
            basic::Bundle<
                TFieldBase, 
                details::OptionsParser<TOptions...>::ForcedMembersVersionDependency,
                TMembers>, 
            TOptions...
        >;    
    static_assert(comms::util::IsTuple<TMembers>::Value,
        "TMembers is expected to be a tuple of std::tuple<...>");

    static_assert(
        1U <= std::tuple_size<TMembers>::value,
        "Number of members is expected to be at least 1.");

public:
    /// @brief Base class provided in the first template parameter.
    using FieldBase = TFieldBase;

    /// @brief Endian used for serialisation.
    using Endian = typename BaseImpl::Endian;

    /// @brief Version type
    using VersionType = typename BaseImpl::VersionType;

    /// @brief All the options provided to this class bundled into struct.
    using ParsedOptions = details::OptionsParser<TOptions...>;

    /// @brief Tag indicating type of the field
    using CommsTag = typename BaseImpl::CommsTag;

    /// @brief Value type.
    /// @details Same as TMemebers template argument, i.e. it is std::tuple
    ///     of all the wrapped fields.
    using ValueType = typename BaseImpl::ValueType;

    /// @brief Type of actual extending field specified via 
    ///     @ref comms::option::def::FieldType.
    /// @details @b void if @ref comms::option::def::FieldType hasn't been applied.
    using FieldType = typename ParsedOptions::FieldType;    

    /// @brief Default constructor
    /// @details Invokes default constructor of every wrapped field
    Bundle() = default;

    /// @brief Constructor
    explicit Bundle(const ValueType& val)
      : BaseImpl(val)
    {
    }

    /// @brief Constructor
    explicit Bundle(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    /// @brief Compile time inquiry of whether @ref comms::option::def::FailOnInvalid option
    ///     has been used.
    static constexpr bool hasFailOnInvalid()
    {
        return ParsedOptions::HasFailOnInvalid;
    }

    /// @brief Compile time inquiry of whether @ref comms::option::def::IgnoreInvalid option
    ///     has been used.
    static constexpr bool hasIgnoreInvalid()
    {
        return ParsedOptions::HasIgnoreInvalid;
    }

    /// @brief Compile time inquiry of whether @ref comms::option::def::EmptySerialization option
    ///     has been used.
    static constexpr bool hasEmptySerialization()
    {
        return ParsedOptions::HasEmptySerialization;
    }

    /// @brief Compile time inquiry of whether @ref comms::option::def::FieldType option
    ///     has been used.
    static constexpr bool hasFieldType()
    {
        return ParsedOptions::HasFieldType;
    }    

    /// @brief Compile time inquiry of whether @ref comms::option::def::FixedValue option
    ///     has been used.
    static constexpr bool hasFixedValue()
    {
        return ParsedOptions::HasFixedValue;
    }      

    /// @brief Compile time inquiry of whether @ref comms::option::def::HasName option
    ///     has been used.
    static constexpr bool hasName()
    {
        return ParsedOptions::HasName;
    } 

    /// @brief Get access to the stored tuple of fields.
    ValueType& value()
    {
        return BaseImpl::value();
    }

    /// @brief Get access to the stored tuple of fields.
    const ValueType& value() const
    {
        return BaseImpl::value();
    }

    /// @brief Get value
    /// @details Implemented by calling @b value(), but can be overriden in the derived class
    const ValueType& getValue() const
    {
        return BaseImpl::getValue();
    }

    /// @brief Set value
    /// @details Implemented as re-assigning to @b value(), but can be overriden in the derived class.
    template <typename U>
    void setValue(U&& val)
    {
        BaseImpl::setValue(std::forward<U>(val));
    }          

    /// @brief Get length required to serialise bundled fields.
    /// @details Summarises all the results returned by the call to length() for
    ///     every field in the bundle.
    /// @return Number of bytes it will take to serialise the field value.
    std::size_t length() const
    {
        return BaseImpl::length();
    }

    /// @brief Get length required to serialise specified bundled member fields.
    /// @details Summarises all the results returned by the call to length() for
    ///     every specified field in the bundle.
    /// @tparam TFromIdx Index of the field (included) from which the counting must start.
    /// @return Number of bytes it will take to serialise the specified member fields.
    /// @pre TFromIdx < std::tuple_size<ValueType>::value
    template <std::size_t TFromIdx>
    std::size_t lengthFrom() const
    {
        return BaseImpl::template lengthFrom<TFromIdx>();
    }

    /// @brief Get length required to serialise specified bundled member fields.
    /// @details Summarises all the results returned by the call to length() for
    ///     every specified field in the bundle.
    /// @tparam TUntilIdx Index of the field (not included) until which the counting must be performed.
    /// @return Number of bytes it will take to serialise the specified member fields.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>::value
    template <std::size_t TUntilIdx>
    std::size_t lengthUntil() const
    {
        return BaseImpl::template lengthUntil<TUntilIdx>();
    }

    /// @brief Get length required to serialise specified bundled member fields.
    /// @details Summarises all the results returned by the call to length() for
    ///     every specified field in the bundle.
    /// @tparam TFromIdx Index of the field (included) from which the counting must start.
    /// @tparam TUntilIdx Index of the field (not included) until which the counting must be performed.
    /// @return Number of bytes it will take to serialise the specified member fields.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>::value
    /// @pre TFromIdx < TUntilIdx
    template <std::size_t TFromIdx, std::size_t TUntilIdx>
    std::size_t lengthFromUntil() const
    {
        return BaseImpl::template lengthFromUntil<TFromIdx, TUntilIdx>();
    }

    /// @brief Get minimal length that is required to serialise all bundled fields.
    /// @return Minimal number of bytes required serialise the field value.
    static constexpr std::size_t minLength()
    {
        return BaseImpl::minLength();
    }

    /// @brief Get minimal length that is required to serialise specified bundled fields.
    /// @tparam TFromIdx Index of the field (included) from which the counting must start.
    /// @return Minimal number of bytes required to serialise the specified member fields.
    /// @pre TFromIdx < std::tuple_size<ValueType>::value
    template <std::size_t TFromIdx>
    static constexpr std::size_t minLengthFrom()
    {
        return BaseImpl::template minLengthFrom<TFromIdx>();
    }

    /// @brief Get minimal length that is required to serialise specified bundled fields.
    /// @tparam TUntilIdx Index of the field (not included) until which the counting must be performed.
    /// @return Minimal number of bytes required to serialise the specified member fields.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>::value
    template <std::size_t TUntilIdx>
    static constexpr std::size_t minLengthUntil()
    {
        return BaseImpl::template minLengthUntil<TUntilIdx>();
    }

    /// @brief Get minimal length that is required to serialise specified bundled fields.
    /// @tparam TFromIdx Index of the field (included) from which the counting must start.
    /// @tparam TUntilIdx Index of the field (not included) until which the counting must be performed.
    /// @return Minimal number of bytes required to serialise the specified member fields.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>::value
    /// @pre TFromIdx < TUntilIdx
    template <std::size_t TFromIdx, std::size_t TUntilIdx>
    static constexpr std::size_t minLengthFromUntil()
    {
        return BaseImpl::template minLengthFromUntil<TFromIdx, TUntilIdx>();
    }

    /// @brief Get maximal length that is required to serialise all bundled fields.
    /// @return Maximal number of bytes required serialise the field value.
    static constexpr std::size_t maxLength()
    {
        return BaseImpl::maxLength();
    }

    /// @brief Get maximal length that is required to serialise specified bundled fields.
    /// @tparam TFromIdx Index of the field (included) from which the counting must start.
    /// @return Minimal number of bytes required to serialise the specified member fields.
    /// @pre TFromIdx < std::tuple_size<ValueType>::value
    template <std::size_t TFromIdx>
    static constexpr std::size_t maxLengthFrom()
    {
        return BaseImpl::template maxLengthFrom<TFromIdx>();
    }

    /// @brief Get maximal length that is required to serialise specified bundled fields.
    /// @tparam TUntilIdx Index of the field (not included) until which the counting must be performed.
    /// @return Minimal number of bytes required to serialise the specified member fields.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>::value
    template <std::size_t TUntilIdx>
    static constexpr std::size_t maxLengthUntil()
    {
        return BaseImpl::template maxLengthUntil<TUntilIdx>();
    }

    /// @brief Get maximal length that is required to serialise specified bundled fields.
    /// @tparam TFromIdx Index of the field (included) from which the counting must start.
    /// @tparam TUntilIdx Index of the field (not included) until which the counting must be performed.
    /// @return Minimal number of bytes required to serialise the specified member fields.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>::value
    /// @pre TFromIdx < TUntilIdx
    template <std::size_t TFromIdx, std::size_t TUntilIdx>
    static constexpr std::size_t maxLengthFromUntil()
    {
        return BaseImpl::template maxLengthFromUntil<TFromIdx, TUntilIdx>();
    }

    /// @brief Read field value from input data sequence
    /// @details Invokes read() member function over every bundled field.
    /// @param[in, out] iter Iterator to read the data.
    /// @param[in] size Number of bytes available for reading.
    /// @return Status of read operation.
    /// @post Iterator is advanced.
    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t size)
    {
        return BaseImpl::read(iter, size);
    }

    /// @brief Read selected number of member fields (from specified index).
    /// @details Similar to @ref read(), but invokes @b read() member function
    ///     of only selected member fields.
    /// @tparam TFromIdx Index of the member field (included) from which the
    ///     "read" operation starts
    /// @param[in, out] iter Iterator to read the data.
    /// @param[in] len Number of bytes available for reading.
    /// @return Status of read operation.
    /// @pre TFromIdx < std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TFromIdx, typename TIter>
    ErrorStatus readFrom(TIter& iter, std::size_t len)
    {
        return BaseImpl::template readFrom<TFromIdx>(iter, len);
    }

    /// @brief Read selected number of member fields (from specified index) while
    ///     updating remaining length information.
    /// @details Similar to @ref readFrom(), but updates provided length information.
    ///     Number of consumed bytes are subsctruced from provided length value.
    /// @tparam TFromIdx Index of the member field (included) from which the
    ///     "read" operation starts
    /// @param[in, out] iter Iterator to read the data.
    /// @param[in, out] len Number of bytes available for reading.
    /// @return Status of read operation.
    /// @pre TFromIdx < std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TFromIdx, typename TIter>
    ErrorStatus readFromAndUpdateLen(TIter& iter, std::size_t& len)
    {
        return BaseImpl::template readFromAndUpdateLen<TFromIdx>(iter, len);
    }    

    /// @brief Read selected number of member fields (until specified index).
    /// @details Similar to @ref read(), but invokes @b read() member function
    ///     of only selected member fields.
    /// @tparam TUntilIdx Index of the member field (NOT included) until which the
    ///     "read" operation continues.
    /// @param[in, out] iter Iterator to read the data.
    /// @param[in] len Number of bytes available for reading.
    /// @return Status of read operation.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TUntilIdx, typename TIter>
    ErrorStatus readUntil(TIter& iter, std::size_t len)
    {
        return BaseImpl::template readUntil<TUntilIdx>(iter, len);
    }

    /// @brief Read selected number of member fields (until specified index) while
    ///     updating remaining length information.
    /// @details Similar to @ref readUntil(), but updates provided length information.
    ///     Number of consumed bytes are subsctruced from provided length value.
    /// @tparam TUntilIdx Index of the member field (NOT included) until which the
    ///     "read" operation continues.
    /// @param[in, out] iter Iterator to read the data.
    /// @param[in, out] len Number of bytes available for reading.
    /// @return Status of read operation.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TUntilIdx, typename TIter>
    ErrorStatus readUntilAndUpdateLen(TIter& iter, std::size_t& len)
    {
        return BaseImpl::template readUntilAndUpdateLen<TUntilIdx>(iter, len);
    }    

    /// @brief Read selected number of member fields (between specified indices).
    /// @details Similar to @ref read(), but invokes @b read() member function
    ///     of only selected member fields.
    /// @tparam TFromIdx Index of the member field (included) from which the
    ///     "read" operation starts
    /// @tparam TUntilIdx Index of the member field (NOT included) until which the
    ///     "read" operation continues.
    /// @param[in, out] iter Iterator to read the data.
    /// @param[in] len Number of bytes available for reading.
    /// @return Status of read operation.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>
    /// @pre TFromIdx < TUntilIdx
    /// @post Iterator is advanced.
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus readFromUntil(TIter& iter, std::size_t len)
    {
        return BaseImpl::template readFromUntil<TFromIdx, TUntilIdx>(iter, len);
    }

    /// @brief Read selected number of member fields (between specified indices) while
    ///     updating remaining length information.
    /// @details Similar to @ref readFromUntil(), but updates provided length information.
    ///     Number of consumed bytes are subsctruced from provided length value.
    /// @tparam TFromIdx Index of the member field (included) from which the
    ///     "read" operation starts
    /// @tparam TUntilIdx Index of the member field (NOT included) until which the
    ///     "read" operation continues.
    /// @param[in, out] iter Iterator to read the data.
    /// @param[in, out] len Number of bytes available for reading.
    /// @return Status of read operation.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>
    /// @pre TFromIdx < TUntilIdx
    /// @post Iterator is advanced.
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus readFromUntilAndUpdateLen(TIter& iter, std::size_t& len)
    {
        return BaseImpl::template readFromUntilAndUpdateLen<TFromIdx, TUntilIdx>(iter, len);
    }    

    /// @brief Compile time check of whether the field has @b proper
    ///     @ref readNoStatus() member function (or similar).
    static constexpr bool hasReadNoStatus()
    {
        return BaseImpl::hasReadNoStatus();
    }

    /// @brief Read field value from input data sequence without error check and status report.
    /// @details Similar to @ref read(), but doesn't perform any correctness
    ///     checks and doesn't report any failures.
    /// @param[in, out] iter Iterator to read the data.
    /// @post Iterator is advanced.
    template <typename TIter>
    void readNoStatus(TIter& iter)
    {
        BaseImpl::readNoStatus(iter);
    }

    /// @brief Read selected member fields from input data sequence
    ///     without error check and status report.
    /// @details Similar to @ref readNoStatus(), but invokes @b readNoStatus()
    ///     of only selected member fields.
    /// @tparam TFromIdx Index of the member field (included) from which the
    ///     "read" operation starts
    /// @param[in, out] iter Iterator to read the data.
    /// @pre TFromIdx < std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TFromIdx, typename TIter>
    void readFromNoStatus(TIter& iter)
    {
        BaseImpl::template readFromNoStatus<TFromIdx>(iter);
    }

    /// @brief Read selected member fields from input data sequence
    ///     without error check and status report.
    /// @details Similar to @ref readNoStatus(), but invokes @b readNoStatus()
    ///     of only selected member fields.
    /// @tparam TUntilIdx Index of the member field (NOT included) until which the
    ///     "read" operation continues.
    /// @param[in, out] iter Iterator to read the data.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TUntilIdx, typename TIter>
    void readUntilNoStatus(TIter& iter)
    {
        BaseImpl::template readUntilNoStatus<TUntilIdx>(iter);
    }

    /// @brief Read selected member fields from input data sequence
    ///     without error check and status report.
    /// @details Similar to @ref readNoStatus(), but invokes @b readNoStatus()
    ///     of only selected member fields.
    /// @tparam TFromIdx Index of the member field (included) from which the
    ///     "read" operation starts
    /// @tparam TUntilIdx Index of the member field (NOT included) until which the
    ///     "read" operation continues.
    /// @param[in, out] iter Iterator to read the data.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>
    /// @pre TFromIdx < TUntilIdx
    /// @post Iterator is advanced.
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    void readFromUntilNoStatus(TIter& iter)
    {
        BaseImpl::template readFromUntilNoStatus<TFromIdx, TUntilIdx>(iter);
    }

    /// @brief Check of whether the field has a consistent value for writing.
    bool canWrite() const
    {
        return BaseImpl::canWrite();
    }

    /// @brief Write current field value to output data sequence
    /// @details Invokes write() member function over every bundled field.
    /// @param[in, out] iter Iterator to write the data.
    /// @param[in] size Maximal number of bytes that can be written.
    /// @return Status of write operation.
    /// @post Iterator is advanced.
    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t size) const
    {
        return BaseImpl::write(iter, size);
    }

    /// @brief Write selected member fields to output data sequence.
    /// @details Similar to @ref write(), but invokes @b write member
    ///     function of only selected member fields.
    /// @tparam TFromIdx Index of the member field (included) from which the
    ///     "write" operation starts.
    /// @param[in, out] iter Iterator to write the data.
    /// @param[in] size Maximal number of bytes that can be written.
    /// @return Status of write operation.
    /// @pre TFromIdx < std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TFromIdx, typename TIter>
    ErrorStatus writeFrom(TIter& iter, std::size_t size) const
    {
        return BaseImpl::template writeFrom<TFromIdx>(iter, size);
    }

    /// @brief Write selected member fields to output data sequence.
    /// @details Similar to @ref write(), but invokes @b write member
    ///     function of only selected member fields.
    /// @tparam TUntilIdx Index of the member field (NOT included) until which the
    ///     "write" operation continues.
    /// @param[in, out] iter Iterator to write the data.
    /// @param[in] size Maximal number of bytes that can be written.
    /// @return Status of write operation.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TUntilIdx, typename TIter>
    ErrorStatus writeUntil(TIter& iter, std::size_t size) const
    {
        return BaseImpl::template writeUntil<TUntilIdx>(iter, size);
    }

    /// @brief Write selected member fields to output data sequence.
    /// @details Similar to @ref write(), but invokes @b write member
    ///     function of only selected member fields.
    /// @tparam TFromIdx Index of the member field (included) from which the
    ///     "write" operation starts.
    /// @tparam TUntilIdx Index of the member field (NOT included) until which the
    ///     "write" operation continues.
    /// @param[in, out] iter Iterator to write the data.
    /// @param[in] size Maximal number of bytes that can be written.
    /// @return Status of write operation.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>
    /// @pre TFromIdx < TUntilIdx
    /// @post Iterator is advanced.
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus writeFromUntil(TIter& iter, std::size_t size) const
    {
        return BaseImpl::template writeFromUntil<TFromIdx, TUntilIdx>(iter, size);
    }

    /// @brief Compile time check of whether the field has @b proper
    ///     @ref writeNoStatus() member function.
    static constexpr bool hasWriteNoStatus()
    {
        return BaseImpl::hasWriteNoStatus();
    }

    /// @brief Write current field value to output data sequence  without error check and status report.
    /// @details Similar to @ref write(), but doesn't perform any correctness
    ///     checks and doesn't report any failures.
    /// @param[in, out] iter Iterator to write the data.
    /// @post Iterator is advanced.
    template <typename TIter>
    void writeNoStatus(TIter& iter) const
    {
        BaseImpl::writeNoStatus(iter);
    }

    /// @brief Write selected member fields to output data sequence
    ///     without error check and status report.
    /// @details Similar to @ref writeNoStatus(), but invokes @b writeNoStatus()
    ///     of only selected member fields.
    /// @tparam TFromIdx Index of the member field (included) from which the
    ///     "write" operation starts
    /// @param[in, out] iter Iterator to read the data.
    /// @pre TFromIdx < std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TFromIdx, typename TIter>
    void writeFromNoStatus(TIter& iter)
    {
        BaseImpl::template writeFromNoStatus<TFromIdx>(iter);
    }

    /// @brief Write selected member fields to output data sequence
    ///     without error check and status report.
    /// @details Similar to @ref writeNoStatus(), but invokes @b writeNoStatus()
    ///     of only selected member fields.
    /// @tparam TUntilIdx Index of the member field (NOT included) until which the
    ///     "write" operation continues.
    /// @param[in, out] iter Iterator to read the data.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TUntilIdx, typename TIter>
    void writeUntilNoStatus(TIter& iter)
    {
        BaseImpl::template writeUntilNoStatus<TUntilIdx>(iter);
    }

    /// @brief Write selected member fields to output data sequence
    ///     without error check and status report.
    /// @details Similar to @ref writeNoStatus(), but invokes @b writeNoStatus()
    ///     of only selected member fields.
    /// @tparam TFromIdx Index of the member field (included) from which the
    ///     "write" operation starts
    /// @tparam TUntilIdx Index of the member field (NOT included) until which the
    ///     "write" operation continues.
    /// @param[in, out] iter Iterator to read the data.
    /// @pre TUntilIdx <= std::tuple_size<ValueType>
    /// @post Iterator is advanced.
    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    void writeFromUntilNoStatus(TIter& iter)
    {
        BaseImpl::template writeFromUntilNoStatus<TFromIdx, TUntilIdx>(iter);
    }

    /// @brief Check validity of all the bundled fields.
    bool valid() const
    {
        return BaseImpl::valid();
    }

    /// @brief Refresh the field's contents
    /// @details Calls refresh() member function on every member field, will
    ///     return @b true if any of the calls returns @b true.
    bool refresh()
    {
        return BaseImpl::refresh();
    }

    /// @brief Compile time check if this class is version dependent
    static constexpr bool isVersionDependent()
    {
        return ParsedOptions::HasCustomVersionUpdate || BaseImpl::isVersionDependent();
    }

    /// @brief Compile time check if this class has non-default refresh functionality
    static constexpr bool hasNonDefaultRefresh()
    {
        return BaseImpl::hasNonDefaultRefresh();
    }

    /// @brief Get version of the field.
    /// @details Exists only if @ref comms::option::def::VersionStorage option has been provided.
    VersionType getVersion() const
    {
        return BaseImpl::getVersion();
    }

    /// @brief Default implementation of version update.
    /// @return @b true in case the field contents have changed, @b false otherwise
    bool setVersion(VersionType version)
    {
        return BaseImpl::setVersion(version);
    }

protected:
    using BaseImpl::readData;
    using BaseImpl::writeData;

private:
    static_assert(!ParsedOptions::HasSerOffset,
        "comms::option::def::NumValueSerOffset option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasFixedLengthLimit,
        "comms::option::def::FixedLength option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasFixedBitLengthLimit,
        "comms::option::def::FixedBitLength option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasVarLengthLimits,
        "comms::option::def::VarLength option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasAvailableLengthLimit,
            "comms::option::def::AvailableLengthLimit option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceElemLengthForcing,
        "comms::option::def::SequenceElemLengthForcingEnabled option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceSizeForcing,
        "comms::option::def::SequenceSizeForcingEnabled option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceLengthForcing,
        "comms::option::def::SequenceLengthForcingEnabled option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceFixedSize,
        "comms::option::def::SequenceFixedSize option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceFixedSizeUseFixedSizeStorage,
        "comms::option::app::SequenceFixedSizeUseFixedSizeStorage option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceSizeFieldPrefix,
        "comms::option::def::SequenceSizeFieldPrefix option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceSerLengthFieldPrefix,
        "comms::option::def::SequenceSerLengthFieldPrefix option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceElemSerLengthFieldPrefix,
        "comms::option::def::SequenceElemSerLengthFieldPrefix option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceElemFixedSerLengthFieldPrefix,
        "comms::option::def::SequenceElemSerLengthFixedFieldPrefix option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceTrailingFieldSuffix,
        "comms::option::def::SequenceTrailingFieldSuffix option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasSequenceTerminationFieldSuffix,
        "comms::option::def::SequenceTerminationFieldSuffix option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasFixedSizeStorage,
        "comms::option::app::FixedSizeStorage option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasCustomStorageType,
        "comms::option::app::CustomStorageType option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasScalingRatio,
        "comms::option::def::ScalingRatio option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasUnits,
        "comms::option::def::Units option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasOrigDataView,
        "comms::option::app::OrigDataView option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasMultiRangeValidation,
        "comms::option::def::ValidNumValueRange (or similar) option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasVersionsRange,
        "comms::option::def::ExistsBetweenVersions (or similar) option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasInvalidByDefault,
        "comms::option::def::InvalidByDefault option is not applicable to Bundle field");
    static_assert(!ParsedOptions::HasMissingOnReadFail,
            "comms::option::def::MissingOnReadFail option is not applicable to Bundle field");     
    static_assert(!ParsedOptions::HasMissingOnInvalid,
            "comms::option::def::MissingOnInvalid option is not applicable to Bundle field");          
};

/// @brief Equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case fields are equal, false otherwise.
/// @related Bundle
template <typename TFieldBase, typename TMembers, typename... TOptions>
bool operator==(
    const Bundle<TFieldBase, TMembers, TOptions...>& field1,
    const Bundle<TFieldBase, TMembers, TOptions...>& field2) noexcept
{
    return field1.value() == field2.value();
}

/// @brief Non-equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case fields are NOT equal, false otherwise.
/// @related Bundle
template <typename TFieldBase, typename TMembers, typename... TOptions>
bool operator!=(
    const Bundle<TFieldBase, TMembers, TOptions...>& field1,
    const Bundle<TFieldBase, TMembers, TOptions...>& field2) noexcept
{
    return field1.value() != field2.value();
}

/// @brief Equivalence comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @related Bundle
template <typename TFieldBase, typename TMembers, typename... TOptions>
bool operator<(
    const Bundle<TFieldBase, TMembers, TOptions...>& field1,
    const Bundle<TFieldBase, TMembers, TOptions...>& field2) noexcept
{
    return field1.value() < field2.value();
}

/// @brief Equivalence comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @related Bundle
template <typename TFieldBase, typename TMembers, typename... TOptions>
bool operator<=(
    const Bundle<TFieldBase, TMembers, TOptions...>& field1,
    const Bundle<TFieldBase, TMembers, TOptions...>& field2) noexcept
{
    return field1.value() <= field2.value();
}

/// @brief Equivalence comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @related Bundle
template <typename TFieldBase, typename TMembers, typename... TOptions>
bool operator>(
    const Bundle<TFieldBase, TMembers, TOptions...>& field1,
    const Bundle<TFieldBase, TMembers, TOptions...>& field2) noexcept
{
    return field1.value() > field2.value();
}

/// @brief Equivalence comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @related Bundle
template <typename TFieldBase, typename TMembers, typename... TOptions>
bool operator>=(
    const Bundle<TFieldBase, TMembers, TOptions...>& field1,
    const Bundle<TFieldBase, TMembers, TOptions...>& field2) noexcept
{
    return field1.value() >= field2.value();
}

/// @brief Compile time check function of whether a provided type is any
///     variant of comms::field::Bundle.
/// @tparam T Any type.
/// @return true in case provided type is any variant of @ref Bundle
/// @related comms::field::Bundle
template <typename T>
constexpr bool isBundle()
{
    return std::is_same<typename T::CommsTag, tag::Bundle>::value;
}

/// @brief Upcast type of the field definition to its parent comms::field::Bundle type
///     in order to have access to its internal types.
template <typename TFieldBase, typename TMembers, typename... TOptions>
inline
Bundle<TFieldBase, TMembers, TOptions...>&
toFieldBase(Bundle<TFieldBase, TMembers, TOptions...>& field)
{
    return field;
}

/// @brief Upcast type of the field definition to its parent comms::field::Bundle type
///     in order to have access to its internal types.
template <typename TFieldBase, typename TMembers, typename... TOptions>
inline
const Bundle<TFieldBase, TMembers, TOptions...>&
toFieldBase(const Bundle<TFieldBase, TMembers, TOptions...>& field)
{
    return field;
}

}  // namespace field

}  // namespace comms


