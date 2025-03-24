//
// Copyright 2014 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of comms::field::EnumValue

#pragma once

#include "comms/field/basic/EnumValue.h"
#include "comms/field/details/AdaptBasicField.h"
#include "comms/field/details/OptionsParser.h"
#include "comms/options.h"

#include <type_traits>

namespace comms
{
namespace field
{

/// @brief Enumerator value field.
/// @details Sometimes dealing with enum values is much more convenient that
///     using integral values. comms::field::EnumValue is very similar to
///     comms::field::IntValue, but receives underlying enum type in its
///     template parameters instead of integral one.
/// @tparam TFieldBase Base class for this field, expected to be a variant of
///     comms::Field.
/// @tparam TEnum Enderlying enum type, can be either unscoped or scoped (enum class).
/// @tparam TOptions Zero or more options that modify/refine default behaviour
///     of the field. If no option is provided, the field's value is serialised as is,
///     where the length of the field is equal to the length of the underlying
///     enum type. For example:
///     @code
///         enum class MyEnum : std::uint16_t
///         {
///             Value1,
///             Value2,
///             Value3
///         }
///         using MyFieldBase = comms::Field<comms::option::def::BigEndian>;
///         using MyField =comms::field::EnumValue<MyFieldBase, MyEnum>;
///     @endcode
///     The serialised value of the field in the example above will consume
///     2 bytes, because the underlying type of MyEnum is
///     defined to be std::uint16_t. The value is serialised using big endian
///     notation because base field class receives comms::option::def::BigEndian option.@n
///     Supported options are:
///     @li @ref comms::option::def::AvailableLengthLimit
///     @li @ref comms::option::def::DefaultValueInitialiser or @ref comms::option::def::DefaultNumValue.
///     @li @ref comms::option::def::EmptySerialization
///     @li @ref comms::option::def::FailOnInvalid
///     @li @ref comms::option::def::FieldType
///     @li @ref comms::option::def::FixedBitLength
///     @li @ref comms::option::def::FixedLength
///     @li @ref comms::option::def::FixedValue
///     @li @ref comms::option::def::HasCustomRead
///     @li @ref comms::option::def::HasCustomRefresh
///     @li @ref comms::option::def::HasName
///     @li @ref comms::option::def::IgnoreInvalid
///     @li @ref comms::option::def::InvalidByDefault
///     @li @ref comms::option::def::NumValueSerOffset
///     @li @ref comms::option::def::ValidNumValueRange, @ref comms::option::def::ValidNumValue,
///         @ref comms::option::def::ValidBigUnsignedNumValueRange, @ref comms::option::def::ValidBigUnsignedNumValue
///     @li @ref comms::option::def::ValidRangesClear
///     @li @ref comms::option::def::VarLength
///     @li @ref comms::option::def::VersionStorage
/// @extends comms::Field
/// @headerfile comms/field/Bundle.h
template <typename TFieldBase, typename TEnum, typename... TOptions>
class EnumValue : public details::AdaptBasicFieldT<basic::EnumValue<TFieldBase, TEnum>, TOptions...>
{
    using BaseImpl = details::AdaptBasicFieldT<basic::EnumValue<TFieldBase, TEnum>, TOptions...>;
    static_assert(std::is_enum<TEnum>::value, "TEnum must be enum type");

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

    /// @brief Type of underlying enum value.
    /// @details Same as template parameter TEnum to this class.
    using ValueType = typename BaseImpl::ValueType;

    /// @brief Type of actual extending field specified via 
    ///     @ref comms::option::def::FieldType.
    /// @details @b void if @ref comms::option::def::FieldType hasn't been applied.
    using FieldType = typename ParsedOptions::FieldType;    

    /// @brief Default constructor.
    EnumValue() = default;

    /// @brief Constructor
    explicit EnumValue(const ValueType& val)
      : BaseImpl(val)
    {
    }

    /// @brief Copy constructor
    EnumValue(const EnumValue&) = default;

    /// @brief Destructor
    ~EnumValue() noexcept = default;

    /// @brief Copy assignment
    EnumValue& operator=(const EnumValue&) = default;

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

    /// @brief Get access to enum value storage.
    const ValueType& value() const
    {
        return BaseImpl::value();
    }

    /// @brief Get access to enum value storage.
    ValueType& value()
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

    /// @brief Get length required to serialise the current field value.
    /// @return Number of bytes it will take to serialise the field value.
    constexpr std::size_t length() const
    {
        return BaseImpl::length();
    }

    /// @brief Get minimal length that is required to serialise field of this type.
    /// @return Minimal number of bytes required serialise the field value.
    static constexpr std::size_t minLength()
    {
        return BaseImpl::minLength();
    }

    /// @brief Get maximal length that is required to serialise field of this type.
    /// @return Maximal number of bytes required serialise the field value.
    static constexpr std::size_t maxLength()
    {
        return BaseImpl::maxLength();
    }

    /// @brief Read field value from input data sequence
    /// @param[in, out] iter Iterator to read the data.
    /// @param[in] size Number of bytes available for reading.
    /// @return Status of read operation.
    /// @post Iterator is advanced.
    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t size)
    {
        return BaseImpl::read(iter, size);
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
    /// @param[in, out] iter Iterator to write the data.
    /// @param[in] size Maximal number of bytes that can be written.
    /// @return Status of write operation.
    /// @post Iterator is advanced.
    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t size) const
    {
        return BaseImpl::write(iter, size);
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

    /// @brief Check validity of the field value.
    bool valid() const
    {
        return BaseImpl::valid();
    }

    /// @brief Refresh the field's value
    /// @return @b true if the value has been updated, @b false otherwise
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

    /// @brief Force serialization length of the field.
    /// @details Available only when @ref comms::option::def::AvailableLengthLimit
    ///     option is used for field definition.
    /// @param[in] len Forced serialization length. 
    ///     @li 0 means default serialization length determined by the storage type
    ///     @li positive value means limit of the serialization length
    ///     @li negative value means the length is determined by the stored value
    void setForcedLength(int len)
    {
        BaseImpl::setForcedLength(len);
    }

    /// @brief Get forced serialization length
    /// @see @ref setForcedLength()
    int getForcedLength() const
    {
        return BaseImpl::getForcedLength();
    }    

protected:
    using BaseImpl::readData;
    using BaseImpl::writeData;
private:
    static_assert(!ParsedOptions::HasSequenceElemLengthForcing,
        "comms::option::def::SequenceElemLengthForcingEnabled option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasSequenceSizeForcing,
        "comms::option::def::SequenceSizeForcingEnabled option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasSequenceLengthForcing,
        "comms::option::def::SequenceLengthForcingEnabled option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasSequenceFixedSize,
        "comms::option::def::SequenceFixedSize option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasSequenceFixedSizeUseFixedSizeStorage,
        "comms::option::app::SequenceFixedSizeUseFixedSizeStorage option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasSequenceSizeFieldPrefix,
        "comms::option::def::SequenceSizeFieldPrefix option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasSequenceSerLengthFieldPrefix,
        "comms::option::def::SequenceSerLengthFieldPrefix option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasSequenceElemSerLengthFieldPrefix,
        "comms::option::def::SequenceElemSerLengthFieldPrefix option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasSequenceElemFixedSerLengthFieldPrefix,
        "comms::option::def::SequenceElemSerLengthFixedFieldPrefix option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasSequenceTrailingFieldSuffix,
        "comms::option::def::SequenceTrailingFieldSuffix option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasSequenceTerminationFieldSuffix,
        "comms::option::def::SequenceTerminationFieldSuffix option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasFixedSizeStorage,
        "comms::option::app::FixedSizeStorage option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasCustomStorageType,
        "comms::option::app::CustomStorageType option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasScalingRatio,
        "comms::option::def::ScalingRatio option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasUnits,
        "comms::option::def::Units option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasOrigDataView,
        "comms::option::app::OrigDataView option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasVersionsRange,
        "comms::option::def::ExistsBetweenVersions (or similar) option is not applicable to EnumValue field");
    static_assert(!ParsedOptions::HasMissingOnReadFail,
            "comms::option::def::MissingOnReadFail option is not applicable to EnumValue field");           
    static_assert(!ParsedOptions::HasMissingOnInvalid,
            "comms::option::def::MissingOnInvalid option is not applicable to EnumValue field");                
};

// Implementation

/// @brief Equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case fields are equal, false otherwise.
/// @related EnumValue
template <typename TFieldBase, typename TEnum, typename... TOptions>
bool operator==(
    const EnumValue<TFieldBase, TEnum, TOptions...>& field1,
    const EnumValue<TFieldBase, TEnum, TOptions...>& field2) noexcept
{
    return field1.value() == field2.value();
}

/// @brief Non-equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case fields are NOT equal, false otherwise.
/// @related EnumValue
template <typename TFieldBase, typename TEnum, typename... TOptions>
bool operator!=(
    const EnumValue<TFieldBase, TEnum, TOptions...>& field1,
    const EnumValue<TFieldBase, TEnum, TOptions...>& field2) noexcept
{
    return field1.value() != field2.value();
}

/// @brief Equivalence comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case value of the first field is lower than than the value of the second.
/// @related EnumValue
template <typename TFieldBase, typename TEnum, typename... TOptions>
bool operator<(
    const EnumValue<TFieldBase, TEnum, TOptions...>& field1,
    const EnumValue<TFieldBase, TEnum, TOptions...>& field2) noexcept
{
    return field1.value() < field2.value();
}

/// @brief Compile time check function of whether a provided type is any
///     variant of comms::field::EnumValue.
/// @tparam T Any type.
/// @return true in case provided type is any variant of @ref EnumValue
/// @related comms::field::EnumValue
template <typename T>
constexpr bool isEnumValue()
{
    return std::is_same<typename T::CommsTag, tag::Enum>::value;
}

/// @brief Upcast type of the field definition to its parent comms::field::EnumValue type
///     in order to have access to its internal types.
/// @related comms::field::EnumValue
template <typename TFieldBase, typename TEnum, typename... TOptions>
inline
EnumValue<TFieldBase, TEnum, TOptions...>&
toFieldBase(EnumValue<TFieldBase, TEnum, TOptions...>& field)
{
    return field;
}

/// @brief Upcast type of the field definition to its parent comms::field::EnumValue type
///     in order to have access to its internal types.
/// @related comms::field::EnumValue
template <typename TFieldBase, typename TEnum, typename... TOptions>
inline
const EnumValue<TFieldBase, TEnum, TOptions...>&
toFieldBase(const EnumValue<TFieldBase, TEnum, TOptions...>& field)
{
    return field;
}

}  // namespace field

}  // namespace comms





