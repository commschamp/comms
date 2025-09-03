//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of @ref comms::field::Optional

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"
#include "comms/field/basic/Optional.h"
#include "comms/field/details/AdaptBasicField.h"
#include "comms/field/details/OptionsParser.h"
#include "comms/field/OptionalMode.h"

#include <utility>

namespace comms
{

namespace field
{

/// @brief Adaptor class to any other field, that makes the field optional.
/// @details When field is optional, it may either exist or not. The behaviour
///     of length(), read() and write() operations depends on the current field's mode.
/// @tparam TField Proper type of the field that needs to be optional.
/// @tparam TOptions Zero or more options that modify/refine default behaviour
///     of the field.@n
///     Supported options are:
///     @li @ref comms::option::def::DefaultValueInitialiser, @ref comms::option::def::DefaultOptionalMode,
///         @ref comms::option::def::OptionalMissingByDefault, or @ref comms::option::def::OptionalExistsByDefault.
///     @li @ref comms::option::def::FieldType
///     @li @ref comms::option::def::FixedValue
///     @li @ref comms::option::def::HasCustomRead
///     @li @ref comms::option::def::HasCustomRefresh
///     @li @ref comms::option::def::HasName
///     @li @ref comms::option::def::MissingOnInvalid
///     @li @ref comms::option::def::MissingOnReadFail
///     @li @ref comms::option::def::VersionStorage
/// @extends comms::Field
/// @headerfile comms/field/Optional.h
template <typename TField, typename... TOptions>
class Optional : public details::AdaptBasicFieldT<basic::Optional<TField>, TOptions...>
{
    using BaseImpl = details::AdaptBasicFieldT<basic::Optional<TField>, TOptions...>;
public:
    /// @brief Endian used for serialisation.
    using Endian = typename BaseImpl::Endian;

    /// @brief Version type
    using VersionType = typename BaseImpl::VersionType;

    /// @brief All the options provided to this class bundled into struct.
    using ParsedOptions = details::OptionsParser<TOptions...>;

    /// @brief Tag indicating type of the field
    using CommsTag = typename BaseImpl::CommsTag;

    /// @brief Type of the field.
    using Field = TField;

    /// @brief Value type of this field, equal to @ref Field
    using ValueType = Field;

    /// @brief Mode of the field.
    /// @see OptionalMode
    using Mode = OptionalMode;

    /// @brief Type of actual extending field specified via 
    ///     @ref comms::option::def::FieldType.
    /// @details @b void if @ref comms::option::def::FieldType hasn't been applied.
    using FieldType = typename ParsedOptions::FieldType;    

    /// @brief Default constructor
    /// @details The mode it is created in is OptionalMode::Tentative.
    Optional() = default;

    /// @brief Construct the field.
    /// @param[in] fieldSrc Field to be copied from during construction.
    explicit Optional(const Field& fieldSrc)
      : BaseImpl(fieldSrc)
    {
    }

    /// @brief Construct the field.
    /// @param[in] fieldSrc Field to be moved from during construction.
    explicit Optional(Field&& fieldSrc)
      : BaseImpl(std::move(fieldSrc))
    {
    }

    /// @brief Copy constructor
    Optional(const Optional&) = default;

    /// @brief Move constructor
    Optional(Optional&&) = default;

    /// @brief Destructor
    ~Optional() noexcept = default;

    /// @brief Copy assignment
    Optional& operator=(const Optional&) = default;

    /// @brief Move assignment
    Optional& operator=(Optional&&) = default;

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

    /// @brief Check whether mode is equivalent to Mode::Tentative
    /// @details Convenience wrapper for getMode(), equivalent to
    ///     @code return getMode() == Mode::Tentative; @endcode
    bool isTentative() const
    {
        return BaseImpl::getMode() == Mode::Tentative;
    }

    /// @brief Set mode to Mode::Tentative
    /// @details Convenience wrapper for setMode(), equivalent to
    ///     @code setMode(Mode::Tentative); @endcode
    void setTentative()
    {
        BaseImpl::setMode(Mode::Tentative);
    }

    /// @brief Check whether mode is equivalent to Mode::Missing
    /// @details Convenience wrapper for getMode(), equivalent to
    ///     @code return getMode() == Mode::Missing; @endcode
    bool isMissing() const
    {
        return BaseImpl::getMode() == Mode::Missing;
    }

    /// @brief Set mode to Mode::Missing
    /// @details Convenience wrapper for setMode(), equivalent to
    ///     @code setMode(Mode::Missing); @endcode
    void setMissing()
    {
        BaseImpl::setMode(Mode::Missing);
    }

    /// @brief Check whether mode is equivalent to Mode::Exists
    /// @details Convenience wrapper for getMode(), equivalent to
    ///     @code return getMode() == Mode::Exists; @endcode
    bool doesExist() const
    {
        return BaseImpl::getMode() == Mode::Exists;
    }

    /// @brief Set mode to Mode::Exists
    /// @details Convenience wrapper for setMode(), equivalent to
    ///     @code setMode(Mode::Exists); @endcode
    void setExists()
    {
        BaseImpl::setMode(Mode::Exists);
    }

    /// @brief Get an access to the wrapped field object
    Field& field()
    {
        return BaseImpl::field();
    }

    /// @brief Get an access to the wrapped field object
    const Field& field() const
    {
        return BaseImpl::field();
    }

    /// @brief Get an access to the wrapped field object
    ValueType& value()
    {
        return BaseImpl::value();
    }

    /// @brief Get an access to the wrapped field object
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

    /// @brief Get current optional mode
    Mode getMode() const
    {
        return BaseImpl::getMode();
    }

    /// @brief Get optional mode
    void setMode(Mode val)
    {
        BaseImpl::setMode(val);
    }

    /// @brief Get length required to serialise the current field value.
    /// @return If current mode is OptionalMode::Exists, then the function
    ///     returns whatever length() member function of the wrapped field
    ///     returns. Otherwise (for both OptionalMode::Missing and
    ///     OptionalMode::Tentative) 0 is returned.
    std::size_t length() const
    {
        return BaseImpl::length();
    }

    /// @brief Get minimal length that is required to serialise field of this type.
    /// @return Same as Field::minLength()
    static constexpr std::size_t minLength()
    {
        return BaseImpl::minLength();
    }

    /// @brief Get maximal length that is required to serialise field of this type.
    /// @return Same as Field::maxLength()
    static constexpr std::size_t maxLength()
    {
        return BaseImpl::maxLength();
    }

    /// @brief Check validity of the field value.
    /// @return If field is marked to be missing (mode is OptionalMode::Missing),
    ///     "true" is returned, otherwise valid() member function of the wrapped
    ///     field is called.
    bool valid() const
    {
        return BaseImpl::valid();
    }

    /// @brief Refresh the field's value
    /// @details Will invoke the refresh() member function of the contained
    ///     field, only if it is marked as "exists", otherwise @b false will be
    ///     returned.
    /// @return @b true if the value has been updated, @b false otherwise
    bool refresh()
    {
        return BaseImpl::refresh();
    }

    /// @brief Read field value from input data sequence
    /// @details If field is marked as missing (mode is OptionalMode::Missing),
    ///     function returns comms::ErrorStatus::Success without advancing iterator.@n
    ///     If field is marked as existing (mode is OptionalMode::Exists) the
    ///     read() member function of the wrapped field object is invoked.@n
    ///     If field is marked to be tentative (mode is OptionalMode::Tentative),
    ///     the call redirected to wrapped field's read() member function if
    ///     value of the "len" parameter is greater than 0, i.e. there are
    ///     still bytes available for reading, and field itself is marked as
    ///     existing.@n Otherwise, field is marked as missing and
    ///     comms::ErrorStatus::Success is returned.
    /// @param[in, out] iter Iterator to read the data.
    /// @param[in] len Number of bytes available for reading.
    /// @return Status of read operation.
    /// @post Iterator is advanced.
    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t len)
    {
        return BaseImpl::read(iter, len);
    }

    /// @brief Compile time check of whether the field has @b proper
    ///     @ref readNoStatus() member function.
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

    /// @brief Check of whether the field has a consistent value for writing.
    bool canWrite() const
    {
        return BaseImpl::canWrite();
    }

    /// @brief Write current field value to output data sequence
    /// @details If field is marked as missing (mode is OptionalMode::Missing) or
    ///     tentative (mode is OptionalMode::Tentative) the
    ///     function returns comms::ErrorStatus::Success without advancing iterator.@n
    ///     If field is marked as existing (mode is OptionalMode::Exists) the
    ///     write() member function of the wrapped field object is invoked.
    /// @param[in, out] iter Iterator to write the data.
    /// @param[in] len Maximal number of bytes that can be written.
    /// @return Status of write operation.
    /// @post Iterator is advanced.
    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t len) const
    {
        return BaseImpl::write(iter, len);
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
    static_assert(!ParsedOptions::HasInvalidByDefault,
        "comms::option::def::InvalidByDefault option is not applicable to Optional field");
};

/// @brief Equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return Result of the equality comparison of the contained fields.
/// @related Optional
template <typename TField, typename... TOptions>
bool operator==(
    const Optional<TField, TOptions...>& field1,
    const Optional<TField, TOptions...>& field2) noexcept
{
    if (field1.getMode() != field2.getMode()) {
        return false;
    }
    
    if (field1.isMissing()) {
        return true;
    }
    
    return field1.field() == field2.field();
}

/// @brief Non-equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return Result of the non-equality comparison of the contained fields.
/// @related Optional
template <typename TField, typename... TOptions>
bool operator!=(
    const Optional<TField, TOptions...>& field1,
    const Optional<TField, TOptions...>& field2) noexcept
{
    return !(field1 == field2);
}

/// @brief Equivalence comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return Result of the equivalence comparison of the contained fields.
/// @related Optional
template <typename TField, typename... TOptions>
bool operator<(
    const Optional<TField, TOptions...>& field1,
    const Optional<TField, TOptions...>& field2) noexcept
{
    if (field1.isMissing()) {
        return !field2.isMissing();
    }
    
    if (field2.isMissing()) {
        return false;
    }
    
    return field1.field() < field2.field();
}

/// @brief Equivalence comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return Result of the equivalence comparison of the contained fields.
/// @related Optional
template <typename TField, typename... TOptions>
bool operator>(
    const Optional<TField, TOptions...>& field1,
    const Optional<TField, TOptions...>& field2) noexcept
{
    return (field2 < field1);
}

/// @brief Equivalence comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return Result of the equivalence comparison of the contained fields.
/// @related Optional
template <typename TField, typename... TOptions>
bool operator<=(
    const Optional<TField, TOptions...>& field1,
    const Optional<TField, TOptions...>& field2) noexcept
{
    return (field1 < field2) || (field1 == field2);
}

/// @brief Equivalence comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return Result of the equivalence comparison of the contained fields.
/// @related Optional
template <typename TField, typename... TOptions>
bool operator>=(
    const Optional<TField, TOptions...>& field1,
    const Optional<TField, TOptions...>& field2) noexcept
{
    return field2 <= field1;
}

/// @brief Compile time check function of whether a provided type is any
///     variant of comms::field::Optional.
/// @tparam T Any type.
/// @return true in case provided type is any variant of @ref Optional
/// @related comms::field::Optional
template <typename T>
constexpr bool isOptional()
{
    return std::is_same<typename T::CommsTag, tag::Optional>::value;
}

/// @brief Upcast type of the field definition to its parent comms::field::Optional type
///     in order to have access to its internal types.
/// @related comms::field::Optional
template <typename TField, typename... TOptions>
inline
Optional<TField, TOptions...>&
toFieldBase(Optional<TField, TOptions...>& field)
{
    return field;
}

/// @brief Upcast type of the field definition to its parent comms::field::Optional type
///     in order to have access to its internal types.
/// @related comms::field::Optional
template <typename TField, typename... TOptions>
inline
const Optional<TField, TOptions...>&
toFieldBase(const Optional<TField, TOptions...>& field)
{
    return field;
}

}  // namespace field

}  // namespace comms


