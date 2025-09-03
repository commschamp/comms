//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of @ref comms::field::Variant

#pragma once

#include "comms/details/macro_common.h"
#include "comms/details/variant_access.h"
#include "comms/ErrorStatus.h"
#include "comms/field/basic/Variant.h"
#include "comms/field/details/AdaptBasicField.h"
#include "comms/options.h"
#include "comms/util/Tuple.h"

#include <utility>

namespace comms
{

namespace field
{

/// @brief Defines a "variant" field, that can contain any of the provided ones.
/// @details The @b Variant object contains uninitialised buffer that can
///     fit any of the provided field types (as second template parameter).
///     At any given point of time this space can be initialised and used to
///     contain <b>at most</b> one of the specified field types. It resembles
///     a classic @b union, but disallows set value of one field type and read
///     it as other. The @b Variant field abstraction provides
///     expected single field API functions, such as length(), read(), write(),
///     valid().
///
///     Refer to @ref sec_field_tutorial_variant for tutorial and usage examples.
/// @tparam TFieldBase Base class for this field, expected to be a variant of
///     comms::Field.
/// @tparam TMembers All supported field types bundled together in
///     <a href="http://en.cppreference.com/w/cpp/utility/tuple">std::tuple</a>.
///     This parameter is used to determine the size of the contained buffer
///     to be able to fit any of the specified types.
/// @tparam TOptions Zero or more options that modify/refine default behaviour
///     of the field.@n
///     Supported options are:
///     @li @ref comms::option::def::DefaultValueInitialiser - All wrapped fields may
///         specify their independent default value initialisers. It is
///         also possible to provide initialiser for the Variant field which
///         will set appropriate values to the fields based on some
///         internal logic.
///     @li @ref comms::option::def::DefaultVariantIndex - By default the Variant field
///         doesn't have any valid contents. This option may be used to specify
///         the index of the default member field.
///     @li @ref comms::option::def::EmptySerialization - Force empty serialization.
///     @li @ref comms::option::def::FieldType - Set actual field type
///     @li @ref comms::option::def::FixedValue
///     @li @ref comms::option::def::HasCustomRead - Mark field to have custom read
///         functionality
///     @li @ref comms::option::def::HasCustomRefresh - Mark field to have custom
///         refresh functionality.
///     @li @ref comms::option::def::HasName
///     @li @ref comms::option::def::HasVersionDependentMembers
///     @li @ref comms::option::def::VariantHasCustomResetOnDestruct - avoid calling
///         default @ref comms::field::Variant::reset() "reset()" on destruction, assume
///         it is called by the extending class destructor.
///     @li @ref comms::option::def::VersionStorage - Add version storage.
/// @extends comms::Field
/// @headerfile comms/field/Variant.h
/// @see COMMS_VARIANT_MEMBERS_NAMES()
/// @see COMMS_VARIANT_MEMBERS_ACCESS()
/// @see COMMS_VARIANT_MEMBERS_ACCESS_NOTEMPLATE()
template <typename TFieldBase, typename TMembers, typename... TOptions>
class Variant : public
    details::AdaptBasicFieldT<
        basic::Variant<
            TFieldBase, 
            details::OptionsParser<TOptions...>::ForcedMembersVersionDependency,
            TMembers
        >, 
        TOptions...>
{
    using BaseImpl = 
        details::AdaptBasicFieldT<
        basic::Variant<
            TFieldBase, 
            details::OptionsParser<TOptions...>::ForcedMembersVersionDependency,
            TMembers
        >, 
        TOptions...>;

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
    /// @details Type of the internal buffer used to store contained field,
    ///     should not be used in normal operation.
    using ValueType = typename BaseImpl::ValueType;

    /// @brief All the supported types.
    /// @details Same as @b TMemebers template argument, i.e. it is @b std::tuple
    ///     of all the wrapped fields.
    using Members = typename BaseImpl::Members;

    /// @brief Type of actual extending field specified via 
    ///     @ref comms::option::def::FieldType.
    /// @details @b void if @ref comms::option::def::FieldType hasn't been applied.
    using FieldType = typename ParsedOptions::FieldType;    

    /// @brief Default constructor
    /// @details Invokes default constructor of every wrapped field
    Variant() = default;

    /// @brief Constructor
    explicit Variant(const ValueType& val)
      : BaseImpl(val)
    {
    }

    /// @brief Constructor
    explicit Variant(ValueType&& val)
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

    /// @brief Get access to the internal storage buffer.
    /// @details Should not be used in normal operation.
    ValueType& value()
    {
        return BaseImpl::value();
    }

    /// @brief Get access to the internal storage buffer.
    /// @details Should not be used in normal operation.
    const ValueType& value() const
    {
        return BaseImpl::value();
    }

    /// @brief Get value
    /// @details Should not be used in normal operation
    const ValueType& getValue() const
    {
        return BaseImpl::getValue();
    }

    /// @brief Set value
    /// @details Should not be used in normal operation
    template <typename U>
    void setValue(U&& val)
    {
        BaseImpl::setValue(std::forward<U>(val));
    }          

    /// @brief Get length required to serialise contained fields.
    /// @details If the field doesn't contain a valid instance of other
    ///     field, the reported length is 0, otherwise the length of the
    ///     contained field is reported.
    /// @return Number of bytes it will take to serialise the field value.
    std::size_t length() const
    {
        return BaseImpl::length();
    }

    /// @brief Get minimal length that is required to serialise all possible contained fields.
    /// @return Always returns 0.
    static constexpr std::size_t minLength()
    {
        return BaseImpl::minLength();
    }

    /// @brief Get maximal length that is required to serialise all possible contained fields.
    /// @return Maximal number of bytes required serialise the field value.
    static constexpr std::size_t maxLength()
    {
        return BaseImpl::maxLength();
    }

    /// @brief Read field value from input data sequence
    /// @details Invokes read() member function over every possible field
    ///     in order of definition until comms::ErrorStatus::Success is returned.
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
        return false;
    }

    /// @brief Read operation without error check and status report is not supported.
    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;

    /// @brief Check of whether the field has a consistent value for writing.
    bool canWrite() const
    {
        return BaseImpl::canWrite();
    }

    /// @brief Write current field value to output data sequence
    /// @details Invokes write() member function of the contained field if such
    ///     exists. If the Variant field doesn't contain any valid field, the
    ///     function doesn't advance the iterator, but returns comms::ErrorStatus::Success.
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

    /// @brief Check validity of all the contained field.
    /// @details Returns @b false if doesn't contain any field.
    bool valid() const
    {
        return BaseImpl::valid();
    }

    /// @brief Refresh the field's value
    /// @details Invokes refresh() member function of the current field
    ///     if such exists, otherwise returns false.
    /// @return @b true if the value has been updated, @b false otherwise
    bool refresh()
    {
        return BaseImpl::refresh();
    }

    /// @brief Get index of the current field (within the @ref Members tuple).
    /// @details If the Variant field doesn't contain any valid field, the
    ///     returned index is equivalent to size of the @ref Members tuple.
    std::size_t currentField() const
    {
        return BaseImpl::currentField();
    }

    /// @brief Select type of the variant field.
    /// @details If the same index has been selected before, the function does
    ///     nothing, otherwise the currently selected member field is destructed,
    ///     and the new one is default constructed.@n
    ///     If provided index is equal or exceeds the size of the @ref Members
    ///     tuple, no new field is constructed.
    /// @param[in] idx Index of the type within @ref Members tuple.
    void selectField(std::size_t idx)
    {
        BaseImpl::selectField(idx);
    }

    /// @brief Execute provided function object with current field as
    ///     parameter.
    /// @details The provided function object must define all the public @b operator()
    ///     member functions to handle all possible types.
    ///     @code
    ///     struct MyFunc
    ///     {
    ///         template <std::size_t TIdx>
    ///         void operator()(Type1& field) {...}
    ///
    ///         template <std::size_t TIdx>
    ///         void operator()(Type2& field) {...}
    ///         ...
    ///     }
    ///     @endcode
    ///     @b NOTE, that every @b operator() is expecting to receive
    ///     an index of the type within the holding tuple as a template
    ///     parameter. If the index information is not needed it may be
    ///     either ignored or static_assert-ed upon.
    ///
    ///     The @b operator() may also receive a member field type as a
    ///     template parameter.
    ///     @code
    ///     struct MyFunc
    ///     {
    ///         template <std::size_t TIdx, typename TField>
    ///         void operator()(TField& field)
    ///         {
    ///             ... // do somethign with the field
    ///         }
    ///     }
    ///     @endcode
    ///     The TField will be the actual type of the contained field.
    ///     If the Variant field doesn't contain any valid field, the functor
    ///     will @b NOT be called.
    template <typename TFunc>
    void currentFieldExec(TFunc&& func)
    {
        BaseImpl::currentFieldExec(std::forward<TFunc>(func));
    }

    /// @brief Execute provided function object with current field as
    ///     parameter (const variant).
    /// @details Similar to other currentFieldExec() variant, but with @b const.
    ///     Note, the constness of the parameter.
    ///     @code
    ///     struct MyFunc
    ///     {
    ///         template <std::size_t TIdx, typename TField>
    ///         void operator()(const TField& field)
    ///         {
    ///             ... // do somethign with the field
    ///         }
    ///     }
    ///     @endcode
    ///     The TField will be the actual type of the contained field.
    ///     If the Variant field doesn't contain any valid field, the functor
    ///     will @b NOT be called.
    template <typename TFunc>
    void currentFieldExec(TFunc&& func) const
    {
        BaseImpl::currentFieldExec(std::forward<TFunc>(func));
    }

    /// @brief Construct and initialise specified contained field in the
    ///     internal buffer.
    /// @details If the field already contains a valid field it must
    ///     be cleared via explicit @ref comms::field::Variant::deinitField() "deinitField()"
    ///     or @ref comms::field::Variant::reset() "reset()" calls.
    /// @tparam TIdx Index of the field type witin the @ref Members tuple.
    /// @tparam TArgs Types of the agurments for the field's constructor
    /// @param[in] args Arguments for the constructed field.
    /// @pre The field must NOT contain any other member field
    ///     @code
    ///     assert(!currentFieldValid());
    ///     @endcode
    /// @return Reference to the constructed field.
    template <std::size_t TIdx, typename... TArgs>
    typename std::tuple_element<TIdx, Members>::type& initField(TArgs&&... args)
    {
        return BaseImpl::template initField<TIdx>(std::forward<TArgs>(args)...);
    }

    /// @brief Destruct previously initialised (via @ref comms::field::Variant::initField() "initField()")
    ///     contained field.
    /// @tparam TIdx Index of the field type witin the @ref Members tuple.
    /// @pre The field must contain an expected member field
    ///     @code
    ///     assert(currentField() == TIdx);
    ///     @endcode
    template <std::size_t TIdx>
    void deinitField()
    {
        BaseImpl::template deinitField<TIdx>();
    }

    /// @brief Access already constructed field at specifed index (known at compile time).
    /// @details Use this function to get a reference to the contained field type
    /// @tparam TIdx Index of the field type witin the @ref Members tuple.
    /// @return Reference to the contained field.
    /// @pre @code currentField() == TIdx @endcode
    template <std::size_t TIdx>
    typename std::tuple_element<TIdx, Members>::type& accessField()
    {
        return BaseImpl::template accessField<TIdx>();
    }

    /// @brief Access already constructed field at specifed index (known at compile time).
    /// @details Use this function to get a const reference to the contained field type.
    /// @tparam TIdx Index of the field type witin the @ref Members tuple.
    /// @return Const reference to the contained field.
    /// @pre @code currentField() == TIdx @endcode
    template <std::size_t TIdx>
    const typename std::tuple_element<TIdx, Members>::type& accessField() const
    {
        return BaseImpl::template accessField<TIdx>();
    }

    /// @brief Check whether the field contains a valid instance of other field.
    /// @details Returns @b true if and only if currentField() returns a valid
    ///     index inside the @ref Members tuple.
    bool currentFieldValid() const
    {
        return BaseImpl::currentFieldValid();
    }

    /// @brief Invalidate current state
    /// @details Destructs currently contained field if such exists.
    void reset()
    {
        BaseImpl::reset();
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
    /// @details Exists only if @ref comms::option::def::VersionStorage option has been provided
    ///     and/or any of the member fields is version dependent.
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
            "comms::option::def::NumValueSerOffset option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasFixedLengthLimit,
            "comms::option::def::FixedLength option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasFixedBitLengthLimit,
            "comms::option::def::FixedBitLength option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasVarLengthLimits,
            "comms::option::def::VarLength option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasAvailableLengthLimit,
            "comms::option::def::AvailableLengthLimit option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceElemLengthForcing,
            "comms::option::def::SequenceElemLengthForcingEnabled option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceSizeForcing,
            "comms::option::def::SequenceSizeForcingEnabled option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceLengthForcing,
            "comms::option::def::SequenceLengthForcingEnabled option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceFixedSize,
            "comms::option::def::SequenceFixedSize option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceFixedSizeUseFixedSizeStorage,
            "comms::option::app::SequenceFixedSizeUseFixedSizeStorage option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceSizeFieldPrefix,
            "comms::option::def::SequenceSizeFieldPrefix option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceSerLengthFieldPrefix,
            "comms::option::def::SequenceSerLengthFieldPrefix option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceElemSerLengthFieldPrefix,
            "comms::option::def::SequenceElemSerLengthFieldPrefix option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceElemFixedSerLengthFieldPrefix,
            "comms::option::def::SequenceElemFixedSerLengthFieldPrefix option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceTrailingFieldSuffix,
            "comms::option::def::SequenceTrailingFieldSuffix option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasSequenceTerminationFieldSuffix,
            "comms::option::def::SequenceTerminationFieldSuffix option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasFixedSizeStorage,
            "comms::option::app::FixedSizeStorage option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasCustomStorageType,
            "comms::option::app::CustomStorageType option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasScalingRatio,
            "comms::option::def::ScalingRatio option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasUnits,
            "comms::option::def::Units option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasOrigDataView,
            "comms::option::app::OrigDataView option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasMultiRangeValidation,
            "comms::option::def::ValidNumValueRange (or similar) option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasVersionsRange,
            "comms::option::def::ExistsBetweenVersions (or similar) option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasInvalidByDefault,
            "comms::option::def::InvalidByDefault option is not applicable to Variant field");
    static_assert(!ParsedOptions::HasMissingOnReadFail,
            "comms::option::def::MissingOnReadFail option is not applicable to Variant field");   
    static_assert(!ParsedOptions::HasMissingOnInvalid,
            "comms::option::def::MissingOnInvalid option is not applicable to Variant field");              
};

namespace details
{

template <typename TVar>
class VariantEqualityCompHelper
{
public:
    VariantEqualityCompHelper(const TVar& other, bool& result)
      : m_other(other),
        m_result(result)
    {}

    template <std::size_t TIdx, typename TField>
    void operator()(const TField& field)
    {
        m_result = (field == m_other.template accessField<TIdx>());
    }

private:
    const TVar& m_other;
    bool& m_result;

};

template <typename TVar>
VariantEqualityCompHelper<TVar> makeVariantEqualityCompHelper(TVar& other, bool& result)
{
    return VariantEqualityCompHelper<TVar>(other, result);
}

template <typename TVar>
class VariantLessCompHelper
{
public:
    VariantLessCompHelper(const TVar& other, bool& result)
      : m_other(other),
        m_result(result)
    {}

    template <std::size_t TIdx, typename TField>
    void operator()(const TField& field)
    {
        m_result = (field < m_other.template accessField<TIdx>());
    }

private:
    const TVar& m_other;
    bool& m_result;
};

template <typename TVar>
VariantLessCompHelper<TVar> makeVariantLessCompHelper(TVar& other, bool& result)
{
    return VariantLessCompHelper<TVar>(other, result);
}

} // namespace details

/// @brief Equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case fields are equal, false otherwise.
/// @related Variant
template <typename TFieldBase, typename TMembers, typename... TOptions>
bool operator==(
    const Variant<TFieldBase, TMembers, TOptions...>& field1,
    const Variant<TFieldBase, TMembers, TOptions...>& field2)
{
    if (&field1 == &field2) {
        return true;
    }

    if (field1.currentFieldValid() != field2.currentFieldValid()) {
        return false;
    }

    if (!field1.currentFieldValid()) {
        return true;
    }

    if (field1.currentField() != field2.currentField()) {
        return false;
    }

    bool result = false;
    field1.currentFieldExec(details::makeVariantEqualityCompHelper(field2, result));
    return result;
}

/// @brief Non-equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case fields are NOT equal, false otherwise.
/// @related Variant
template <typename TFieldBase, typename TMembers, typename... TOptions>
bool operator!=(
    const Variant<TFieldBase, TMembers, TOptions...>& field1,
    const Variant<TFieldBase, TMembers, TOptions...>& field2)
{
    return !(field1 == field2);
}

/// @brief Order comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @related Variant
template <typename TFieldBase, typename TMembers, typename... TOptions>
bool operator<(
    const Variant<TFieldBase, TMembers, TOptions...>& field1,
    const Variant<TFieldBase, TMembers, TOptions...>& field2)
{
    if (!field1.currentFieldValid()) {
        return (!field2.currentFieldValid());
    }

    if (!field2.currentFieldValid()) {
        return false;
    }    

    if (field1.currentField() < field2.currentField()) {
        return true;
    }

    if (field1.currentField() != field2.currentField()) {
        return false;
    }

    if (&field1 == &field2) {
        return false;
    }    

    bool result = false;
    field1.currentFieldExec(details::makeVariantLessCompHelper(field2, result));
    return result;
}

/// @brief Compile time check function of whether a provided type is any
///     variant of comms::field::Variant.
/// @tparam T Any type.
/// @return true in case provided type is any variant of @ref Variant
/// @related comms::field::Variant
template <typename T>
constexpr bool isVariant()
{
    return std::is_same<typename T::CommsTag, tag::Variant>::value;
}

/// @brief Upcast type of the field definition to its parent comms::field::Variant type
///     in order to have access to its internal types.
/// @related comms::field::Variant
template <typename TFieldBase, typename TMembers, typename... TOptions>
inline
Variant<TFieldBase, TMembers, TOptions...>&
toFieldBase(Variant<TFieldBase, TMembers, TOptions...>& field)
{
    return field;
}

/// @brief Upcast type of the field definition to its parent comms::field::Variant type
///     in order to have access to its internal types.
/// @related comms::field::Variant
template <typename TFieldBase, typename TMembers, typename... TOptions>
inline
const Variant<TFieldBase, TMembers, TOptions...>&
toFieldBase(const Variant<TFieldBase, TMembers, TOptions...>& field)
{
    return field;
}

/// @brief Add convenience access enum and functions to the members of
///     @ref comms::field::Variant field.
/// @details Very similar to #COMMS_VARIANT_MEMBERS_NAMES(), but does @b NOT
///     require definition of @b Base inner member type (for some compilers) and does @b NOT
///     define inner @b Field_* types for used member fields.
/// @param[in] ... List of fields' names.
/// @related comms::field::Variant
/// @warning Some compilers, such as @b clang or early versions of @b g++
///     may have problems compiling code generated by this macro even
///     though it uses valid C++11 constructs in attempt to automatically identify the
///     type of the base class. If the compilation fails,
///     and this macro resides inside a @b NON-template class, please use
///     COMMS_VARIANT_MEMBERS_ACCESS_NOTEMPLATE() macro instead. In
///     case this macro needs to reside inside a @b template class, then
///     there is a need to define inner @b Base type, which specifies
///     exact type of the @ref comms::field::Variant class. For example:
///     @code
///     template <typename... TExtraOptions>
///     class MyField : public
///         comms::field::Variant<
///             MyFieldBase,
///             std::tuple<Field1, Field2, Field3>,
///             TExtraOptions...
///         >
///     {
///         // Duplicate the base class definition
///         using Base =
///             comms::field::Variant<
///                 MyFieldBase,
///                 std::tuple<Field1, Field2, Field3>,
///                 TExtraOptions...
///             >;
///     public:
///         COMMS_VARIANT_MEMBERS_ACCESS(member1, member2, member3);
///     };
///     @endcode
#define COMMS_VARIANT_MEMBERS_ACCESS(...) \
    COMMS_EXPAND(COMMS_DEFINE_FIELD_ENUM(__VA_ARGS__)) \
    COMMS_AS_VARIANT_FUNC { \
        auto& var = comms::field::toFieldBase(*this); \
        using Var = typename std::decay<decltype(var)>::type; \
        static_assert(std::tuple_size<typename Var::Members>::value == FieldIdx_numOfValues, \
            "Invalid number of names for variant field"); \
        return var; \
    }\
    COMMS_AS_VARIANT_CONST_FUNC { \
        auto& var = comms::field::toFieldBase(*this); \
        using Var = typename std::decay<decltype(var)>::type; \
        static_assert(std::tuple_size<typename Var::Members>::value == FieldIdx_numOfValues, \
            "Invalid number of names for variant field"); \
        return var; \
    } \
    COMMS_DO_VARIANT_MEM_ACC_FUNC(asVariant(), __VA_ARGS__)


/// @brief Similar to COMMS_VARIANT_MEMBERS_ACCESS(), but dedicated for
///     non-template classes.
/// @details The COMMS_VARIANT_MEMBERS_ACCESS() macro is a generic one,
///     which can be used in any class (template, or non-template). However,
///     some compilers (such as <b>g++-4.9</b> and below, @b clang-4.0 and below) may fail
///     to compile it even though it uses valid C++11 constructs. If the
///     compilation fails and the class it is being used in is @b NOT a
///     template one, please use @ref COMMS_VARIANT_MEMBERS_ACCESS_NOTEMPLATE()
///     instead.
/// @related comms::field::Variant
#define COMMS_VARIANT_MEMBERS_ACCESS_NOTEMPLATE(...) \
    COMMS_EXPAND(COMMS_DEFINE_FIELD_ENUM(__VA_ARGS__)) \
    COMMS_DO_VARIANT_MEM_ACC_FUNC_NOTEMPLATE(__VA_ARGS__)

/// @brief Provide names for member fields of @ref comms::field::Variant field.
/// @details All the possible field types the @ref comms::field::Variant field
///     can contain are bundled in
///     <a href="http://en.cppreference.com/w/cpp/utility/tuple">std::tuple</a>
///     and provided as a template parameter to the definition of the
///     @ref comms::field::Variant field.
///     @code
///     using MyFieldBase = comms::Field<comms::option::BigEndian>;
///     using Field1 = ...;
///     using Field2 = ...;
///     using Field3 = ...;
///     using MyField =
///         comms::field::Variant<
///             MyFieldBase,
///             std::tuple<Field1, Field2, Field3>
///         >;
///
///     MyField field;
///     auto& field1 = field.initField<0>(); // Initialise the field to contain Field1 value
///     field1.value() = ...;
///     @endcode
///     However, it would be convenient to provide names and easier access to
///     all the poisble variants. The #COMMS_VARIANT_MEMBERS_NAMES() macro does exactly
///     that when used inside the field class definition. Just inherit from
///     the @ref comms::field::Variant class and use the macro inside with the names for the
///     member fields:
///     @code
///     class MyField : public comms::field::Variant<...>
///     {
///         // (Re)definition of the base class as inner Base type is
///         // the required of COMMS_VARIANT_MEMBERS_NAMES() macro.
///         using Base = comms::field::Variant<...>;
///     public:
///         COMMS_FIELD_MEMBERS_NAMES(member1, member2, member3);
///     }
///     @endcode
///     @b NOTE that there is a required to have @b Base member type that
///     specifies base class used.
///
///     Usage of @ref COMMS_FIELD_MEMBERS_NAMES() macro is
///     equivalent to having the following types and functions
///     definitions:
///     @code
///     class MyField : public comms::field::Variant<...>
///     {
///     public:
///         // Access indices for member fields
///         enum FieldIdx {
///             FieldIdx_member1,
///             FieldIdx_member2,
///             FieldIdx_member3,
///             FieldIdx_numOfValues
///         };
///
///         // Initialise as first member (Field1)
///         template <typename... TArgs>
///         Field1& initField_member1(TArgs&&... args)
///         {
///             rerturn initField<FieldIdx_member1>(std::forward<TArgs>(args)...);
///         }
///
///         // De-initialise first member (Field1)
///         void deinitField_member1()
///         {
///             rerturn deinitField<FieldIdx_member1>();
///         }
///
///         // Accessor to the stored field as first member (Field1)
///         Field1& accessField_member1()
///         {
///             return accessField<FieldIdx_member1>();
///         }
///
///         // Const variant of the accessor to the stored field as first member (Field1)
///         const Field1& accessField_member1() const
///         {
///             return accessField<FieldIdx_member1>();
///         }
///
///         // Initialise as second member (Field2)
///         template <typename... TArgs>
///         Field2& initField_member2(TArgs&&... args)
///         {
///             rerturn initField<FieldIdx_member2>(std::forward<TArgs>(args)...);
///         }
///
///         // De-initialise second member (Field2)
///         void deinitField_member2()
///         {
///             rerturn deinitField<FieldIdx_member2>();
///         }
///
///         // Accessor to the stored field as second member (Field2)
///         Field2& accessField_member2()
///         {
///             return accessField<FieldIdx_member2>();
///         }
///
///         // Const variant of the accessor to the stored field as second member (Field2)
///         const Field2& accessField_member2() const
///         {
///             return accessField<FieldIdx_member2>();
///         }
///
///         // Initialise as third member (Field3)
///         template <typename... TArgs>
///         Field3& initField_member3(TArgs&&... args)
///         {
///             rerturn initField<FieldIdx_member3>(std::forward<TArgs>(args)...);
///         }
///
///         // De-initialise third member (Field3)
///         void deinitField_member3()
///         {
///             rerturn deinitField<FieldIdx_member3>();
///         }
///
///         // Accessor to the stored field as third member (Field3)
///         Field3& accessField_member3()
///         {
///             return accessField<FieldIdx_member3>();
///         }
///
///         // Const variant of the accessor to the stored field as third member (Field3)
///         const Field3& accessField_member3() const
///         {
///             return accessField<FieldIdx_member3>();
///         }
///
///         // Redefinition of the field types:
///         using Field_member1 = Field1;
///         using Field_member2 = Field2;
///         using Field_member3 = Field3;
///     };
///     @endcode
///     @b NOTE, that provided names @b member1, @b member2, and @b member3, have
///         found their way to the following definitions:
///     @li @b FieldIdx enum. The names are prefixed with @b FieldIdx_. The
///         @b FieldIdx_nameOfValues value is automatically added at the end.
///     @li Initialisation functions prefixed with @b initField_
///     @li De-initialisation functions prefixed with @b deinitField_
///     @li Accessor functions prefixed with @b accessField_
///     @li Types of fields prefixed with @b Field_*
///
///     See @ref sec_field_tutorial_variant for more examples and details
/// @param[in] ... List of member fields' names.
/// @related comms::field::Variant
/// @note Defined in "comms/field/Variant.h"
/// @see #COMMS_VARIANT_MEMBERS_ACCESS()
/// @see @ref sec_field_tutorial_variant
#define COMMS_VARIANT_MEMBERS_NAMES(...) \
    COMMS_EXPAND(COMMS_VARIANT_MEMBERS_ACCESS(__VA_ARGS__)) \
    COMMS_EXPAND(COMMS_DO_FIELD_TYPEDEF(typename Base::Members, Field_, FieldIdx_, __VA_ARGS__))


}  // namespace field

}  // namespace comms


