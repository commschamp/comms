//
// Copyright 2014 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of @ref comms::field::ArrayList

#pragma once

#include "basic/ArrayList.h"
#include "comms/CompileControl.h"
#include "comms/ErrorStatus.h"
#include "comms/options.h"
#include "comms/util/StaticVector.h"
#include "comms/util/ArrayView.h"
#include "comms/util/type_traits.h"
#include "details/AdaptBasicField.h"
#include "details/OptionsParser.h"

#include <vector>
#include <utility>

#if COMMS_HAS_CPP20_SPAN
#include <span>
#endif // #if COMMS_HAS_CPP20_SPAN

namespace comms
{

namespace field
{

namespace details
{

template <bool THasOrigDataViewStorage>
struct ArrayListOrigDataViewStorageType;

template <>
struct ArrayListOrigDataViewStorageType<true>
{
#if COMMS_HAS_CPP20_SPAN
    template <typename TElement>
    using Type = std::span<TElement>;
#else // #if COMMS_HAS_CPP20_SPAN    
    template <typename TElement>
    using Type = comms::util::ArrayView<TElement>;
#endif // #if COMMS_HAS_CPP20_SPAN        
};

template <>
struct ArrayListOrigDataViewStorageType<false>
{
    template <typename TElement>
    using Type = std::vector<TElement>;
};

template <bool THasSequenceFixedSizeUseFixedSizeStorage>
struct ArrayListSequenceFixedSizeUseFixedSizeStorageType;

template <>
struct ArrayListSequenceFixedSizeUseFixedSizeStorageType<true>
{
    template <typename TElement, typename TOpt>
    using Type = comms::util::StaticVector<TElement, TOpt::SequenceFixedSize>;
};

template <>
struct ArrayListSequenceFixedSizeUseFixedSizeStorageType<false>
{
    template <typename TElement, typename TOpt>
    using Type =
        typename ArrayListOrigDataViewStorageType<
            TOpt::HasOrigDataView && std::is_integral<TElement>::value && (sizeof(TElement) == sizeof(std::uint8_t))
        >::template Type<TElement>;
};

template <bool THasFixedSizeStorage>
struct ArrayListFixedSizeStorageType;

template <>
struct ArrayListFixedSizeStorageType<true>
{
    template <typename TElement, typename TOpt>
    using Type = comms::util::StaticVector<TElement, TOpt::FixedSizeStorage>;
};

template <>
struct ArrayListFixedSizeStorageType<false>
{
    template <typename TElement, typename TOpt>
    using Type =
        typename ArrayListSequenceFixedSizeUseFixedSizeStorageType<TOpt::HasSequenceFixedSizeUseFixedSizeStorage>
            ::template Type<TElement, TOpt>;
};

template <bool THasCustomStorage>
struct ArrayListCustomArrayListStorageType;

template <>
struct ArrayListCustomArrayListStorageType<true>
{
    template <typename TElement, typename TOpt>
    using Type = typename TOpt::CustomStorageType;
};

template <>
struct ArrayListCustomArrayListStorageType<false>
{
    template <typename TElement, typename TOpt>
    using Type =
        typename ArrayListFixedSizeStorageType<TOpt::HasFixedSizeStorage>::template Type<TElement, TOpt>;
};

template <typename TElement, typename TOpt>
using ArrayListStorageTypeT =
    typename ArrayListCustomArrayListStorageType<TOpt::HasCustomStorageType>::template Type<TElement, TOpt>;

template <typename TFieldBase, typename TElement, typename... TOptions>
using ArrayListBase =
    AdaptBasicFieldT<
        comms::field::basic::ArrayList<
            TFieldBase,
            ArrayListStorageTypeT<TElement, OptionsParser<TOptions...> >
        >,
        TOptions...
    >;

}  // namespace details

/// @brief Field that represents a sequential collection of fields.
/// @details By default uses
///     <a href="http://en.cppreference.com/w/cpp/container/vector">std::vector</a>,
///     for internal storage, unless @ref comms::option::app::FixedSizeStorage option is used,
///     which forces usage of comms::util::StaticVector instead.
/// @tparam TFieldBase Base class for this field, expected to be a variant of
///     comms::Field.
/// @tparam TElement Element of the collection, can be either basic integral value
///     (such as std::uint8_t) or any other field from comms::field namespace.@n
///     For example:
///     @code
///     using MyFieldBase = comms::Field<comms::option::def::BigEndian>;
///     using RawDataSeqField =
///         comms::field::ArrayList<
///             MyFieldBase,
///             std::uint8_t
///         >;
///     using CollectionOfBundlesField =
///         comms::field::ArrayList<
///             MyFieldBase,
///             std::field::Bundle<
///                 MyFieldBase,
///                 std::tuple<
///                     comms::field::IntValue<MyFieldBase, std::uint16_t>
///                     comms::field::IntValue<MyFieldBase, std::uint8_t>
///                     comms::field::IntValue<MyFieldBase, std::uint8_t>
///                 >
///             >
///         >;
///     @endcode
/// @tparam TOptions Zero or more options that modify/refine default behaviour
///     of the field.@n
///     Supported options are:
///     @li @ref comms::option::def::DefaultValueInitialiser
///     @li @ref comms::option::def::EmptySerialization
///     @li @ref comms::option::def::FailOnInvalid
///     @li @ref comms::option::def::FieldType
///     @li @ref comms::option::def::FixedValue
///     @li @ref comms::option::def::HasCustomRead
///     @li @ref comms::option::def::HasCustomRefresh
///     @li @ref comms::option::def::HasName
///     @li @ref comms::option::def::IgnoreInvalid
///     @li @ref comms::option::def::SequenceElemFixedSerLengthFieldPrefix
///     @li @ref comms::option::def::SequenceElemSerLengthFieldPrefix
///     @li @ref comms::option::def::SequenceFixedSize
///     @li @ref comms::option::def::SequenceLengthForcingEnabled
///     @li @ref comms::option::def::SequenceSerLengthFieldPrefix
///     @li @ref comms::option::def::SequenceSizeFieldPrefix
///     @li @ref comms::option::def::SequenceSizeForcingEnabled
///     @li @ref comms::option::def::SequenceTerminationFieldSuffix
///     @li @ref comms::option::def::SequenceTrailingFieldSuffix
///     @li @ref comms::option::def::VersionStorage
///     @li @ref comms::option::app::CustomStorageType
///     @li @ref comms::option::app::FixedSizeStorage
///     @li @ref comms::option::app::OrigDataView (valid only if TElement is integral type
///         of 1 byte size.

/// @extends comms::Field
/// @headerfile comms/field/ArrayList.h
template <typename TFieldBase, typename TElement, typename... TOptions>
class ArrayList : public details::ArrayListBase<TFieldBase, TElement, TOptions...>
{
    using BaseImpl = details::ArrayListBase<TFieldBase, TElement, TOptions...>;
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

    /// @brief Type of underlying value.
    /// @details If @ref comms::option::app::FixedSizeStorage option is NOT used, the
    ///     ValueType is std::vector<TElement>, otherwise it becomes
    ///     comms::util::StaticVector<TElement, TSize>, where TSize is a size
    ///     provided to @ref comms::option::app::FixedSizeStorage option.
    using ValueType = typename BaseImpl::ValueType;

    /// @brief Type of the element.
    using ElementType = typename BaseImpl::ElementType;

    /// @brief Type of actual extending field specified via 
    ///     @ref comms::option::def::FieldType.
    /// @details @b void if @ref comms::option::def::FieldType hasn't been applied.
    using FieldType = typename ParsedOptions::FieldType;    

    /// @brief Type of size field prefix specified via @ref comms::option::def::SequenceSizeFieldPrefix.
    /// @details @b void if @ref comms::option::def::SequenceSizeFieldPrefix is not used.
    using SizeFieldPrefix = typename ParsedOptions::SequenceSizeFieldPrefix;

    /// @brief Type of length field prefix specified via @ref comms::option::def::SequenceSerLengthFieldPrefix.
    /// @details @b void if @ref comms::option::def::SequenceSerLengthFieldPrefix is not used.
    using SerLengthFieldPrefix = typename ParsedOptions::SequenceSerLengthFieldPrefix;    

    /// @brief Type of element's length field prefix specified via @ref comms::option::def::SequenceElemSerLengthFieldPrefix.
    /// @details @b void if @ref comms::option::def::SequenceElemSerLengthFieldPrefix is not used.
    using ElemSerLengthFieldPrefix = typename ParsedOptions::SequenceElemSerLengthFieldPrefix;    

    /// @brief Type of element's fixed length field prefix specified via @ref comms::option::def::SequenceElemFixedSerLengthFieldPrefix.
    /// @details @b void if @ref comms::option::def::SequenceElemFixedSerLengthFieldPrefix is not used.
    using ElemFixedSerLengthFieldPrefix = typename ParsedOptions::SequenceElemFixedSerLengthFieldPrefix;    

    /// @brief Type of termination field suffix specified via @ref comms::option::def::SequenceTerminationFieldSuffix.
    /// @details @b void if @ref comms::option::def::SequenceTerminationFieldSuffix is not used.
    using TerminationFieldSuffix = typename ParsedOptions::SequenceTerminationFieldSuffix;    

    /// @brief Type of trailing field suffix specified via @ref comms::option::def::SequenceTrailingFieldSuffix.
    /// @details @b void if @ref comms::option::def::SequenceTrailingFieldSuffix is not used.
    using TrailingFieldSuffix = typename ParsedOptions::SequenceTrailingFieldSuffix;      

        /// @brief Default constructor
    ArrayList() = default;

    /// @brief Value constructor
    explicit ArrayList(const ValueType& val)
      : BaseImpl(val)
    {
    }

    /// @brief Value constructor
    explicit ArrayList(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    /// @brief Copy constructor
    ArrayList(const ArrayList&) = default;

    /// @brief Move constructor
    ArrayList(ArrayList&&) = default;

    /// @brief Destructor
    ~ArrayList() noexcept = default;

    /// @brief Copy assignment
    ArrayList& operator=(const ArrayList&) = default;

    /// @brief Move assignment
    ArrayList& operator=(ArrayList&&) = default;

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

    /// @brief Compile time inquiry of whether @ref comms::option::def::SequenceSizeFieldPrefix option
    ///     has been used.
    static constexpr bool hasSizeFieldPrefix()
    {
        return ParsedOptions::HasSequenceSizeFieldPrefix;
    }

    /// @brief Compile time inquiry of whether @ref comms::option::def::SequenceSerLengthFieldPrefix option
    ///     has been used.
    static constexpr bool hasSerLengthFieldPrefix()
    {
        return ParsedOptions::HasSequenceSerLengthFieldPrefix;
    }    

    /// @brief Compile time inquiry of whether @ref comms::option::def::SequenceElemSerLengthFieldPrefix option
    ///     has been used.
    static constexpr bool hasElemSerLengthFieldPrefix()
    {
        return ParsedOptions::HasSequenceElemSerLengthFieldPrefix;
    }

    /// @brief Compile time inquiry of whether @ref comms::option::def::SequenceElemFixedSerLengthFieldPrefix option
    ///     has been used.
    static constexpr bool hasElemFixedSerLengthFieldPrefix()
    {
        return ParsedOptions::HasSequenceElemFixedSerLengthFieldPrefix;
    }    

    /// @brief Compile time inquiry of whether @ref comms::option::def::SequenceTerminationFieldSuffix option
    ///     has been used.
    static constexpr bool hasTerminationFieldSuffix()
    {
        return ParsedOptions::HasSequenceTerminationFieldSuffix;
    }      

    /// @brief Compile time inquiry of whether @ref comms::option::def::SequenceTrailingFieldSuffix option
    ///     has been used.
    static constexpr bool hasTrailingFieldSuffix()
    {
        return ParsedOptions::HasSequenceTrailingFieldSuffix;
    }    

    /// @brief Compile time inquiry of whether @ref comms::option::def::SequenceFixedSize option
    ///     has been used.
    static constexpr bool hasFixedSize()
    {
        return ParsedOptions::HasSequenceFixedSize;
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

    /// @brief Compile time inquiry of fixed size provided via @ref comms::option::def::SequenceFixedSize option.
    /// @details If the @ref comms::option::def::SequenceFixedSize option hasn't been used
    ///     <b>std::numeric_limits&lt;std::size_t&gt;\::max()</b> is returned.
    static constexpr std::size_t fixedSize()
    {
        return ParsedOptions::SequenceFixedSize;
    }           

    /// @brief Get access to the value storage.
    ValueType& value()
    {
        return BaseImpl::value();
    }

    /// @brief Get access to the value storage.
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

    /// @brief Get length of serialised data
    constexpr std::size_t length() const
    {
        return BaseImpl::length();
    }

    /// @brief Read field value from input data sequence
    /// @details By default, the read operation will try to consume all the
    ///     data available, unless size limiting option (such as
    ///     @ref comms::option::def::SequenceSizeFieldPrefix, @ref comms::option::def::SequenceFixedSize,
    ///     @ref comms::option::def::SequenceSizeForcingEnabled,
    ///     @ref comms::option::def::SequenceLengthForcingEnabled) is used.
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
    /// @details By default, the write operation will write all the
    ///     elements the field contains. If @ref comms::option::def::SequenceFixedSize option
    ///     is used, the number of elements, that is going to be written, is
    ///     exactly as the option specifies. If underlying vector storage
    ///     doesn't contain enough data, the default constructed elements will
    ///     be appended to the written sequence until the required amount of
    ///     elements is reached.
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

    /// @brief Check validity of the field value.
    /// @details The collection is valid if all the elements are valid. 
    /// @return true in case the field's value is valid, false otherwise.
    bool valid() const
    {
        return BaseImpl::valid();
    }

    /// @brief Refresh the field.
    /// @details Calls refresh() on all the elements (if they are fields and not raw bytes).
    /// @brief Returns true if any of the elements has been updated, false otherwise.
    bool refresh()
    {
        return BaseImpl::refresh();
    }

    /// @brief Get minimal length that is required to serialise field of this type.
    static constexpr std::size_t minLength()
    {
        return BaseImpl::minLength();
    }

    /// @brief Get maximal length that is required to serialise field of this type.
    static constexpr std::size_t maxLength()
    {
        return BaseImpl::maxLength();
    }

    /// @brief Force number of elements that must be read in the next read()
    ///     invocation.
    /// @details Exists only if @ref comms::option::def::SequenceSizeForcingEnabled option has been
    ///     used.
    /// @param[in] count Number of elements to read during following read operation.
    void forceReadElemCount(std::size_t count)
    {
        return BaseImpl::forceReadElemCount(count);
    }

    /// @brief Clear forcing of the number of elements that must be read in the next read()
    ///     invocation.
    /// @details Exists only if @ref comms::option::def::SequenceSizeForcingEnabled option has been
    ///     used.
    void clearReadElemCount()
    {
        return BaseImpl::clearReadElemCount();
    }

    /// @brief Force available length for the next read() invocation.
    /// @details Exists only if @ref comms::option::def::SequenceLengthForcingEnabled option has been
    ///     used.
    /// @param[in] count Number of elements to read during following read operation.
    void forceReadLength(std::size_t count)
    {
        return BaseImpl::forceReadLength(count);
    }

    /// @brief Clear forcing of the available length in the next read()
    ///     invocation.
    /// @details Exists only if @ref comms::option::def::SequenceLengthForcingEnabled option has been
    ///     used.
    void clearReadLengthForcing()
    {
        return BaseImpl::clearReadLengthForcing();
    }

    /// @brief Force serialisation length of a single element.
    /// @details The function can be used to force a serialisation length of a
    ///     single element within the ArrayList.
    ///     Exists only if @ref comms::option::def::SequenceElemLengthForcingEnabled option has been
    ///     used.
    /// @param[in] count Number of elements to read during following read operation.
    void forceReadElemLength(std::size_t count)
    {
        return BaseImpl::forceReadElemLength(count);
    }

    /// @brief Clear forcing the serialisation length of the single element.
    /// @details Exists only if @ref comms::option::def::SequenceElemLengthForcingEnabled option has been
    ///     used.
    void clearReadElemLengthForcing()
    {
        return BaseImpl::clearReadElemLengthForcing();
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

#ifdef FOR_DOXYGEN_DOC_ONLY
    /// @brief Access list termination field 
    /// @details Exists only if @ref comms::option::def::SequenceTerminationFieldSuffix option has been
    ///     used. The function can be used to access the termination field value after
    ///     the @b read and/or before the @b write operations.
    /// @pre @ref hasTerminationFieldSuffix() returns true.
    auto terminationFieldSuffix() -> decltype(BaseImpl::terminationFieldSuffix())
    {
        return BaseImpl::terminationFieldSuffix();
    }    

    /// @brief Access list termination field (const variant)
    /// @details Exists only if @ref comms::option::def::SequenceTerminationFieldSuffix option has been
    ///     used. The function can be used to access the termination field value after
    ///     the @b read and/or before the @b write operations.
    /// @pre @ref hasTerminationFieldSuffix() returns true.
    auto terminationFieldSuffix() const -> decltype(BaseImpl::terminationFieldSuffix())
    {
        return BaseImpl::terminationFieldSuffix();
    }     

#endif // #ifdef FOR_DOXYGEN_DOC_ONLY    

protected:
    using BaseImpl::readData;
    using BaseImpl::writeData;

private:
    static_assert(!ParsedOptions::HasSerOffset,
            "comms::option::def::NumValueSerOffset option is not applicable to ArrayList field");
    static_assert(!ParsedOptions::HasFixedLengthLimit,
            "comms::option::def::FixedLength option is not applicable to ArrayList field");
    static_assert(!ParsedOptions::HasFixedBitLengthLimit,
            "comms::option::def::FixedBitLength option is not applicable to ArrayList field");
    static_assert(!ParsedOptions::HasVarLengthLimits,
            "comms::option::def::VarLength option is not applicable to ArrayList field");
    static_assert(!ParsedOptions::HasAvailableLengthLimit,
            "comms::option::def::AvailableLengthLimit option is not applicable to ArrayList field");
    static_assert(!ParsedOptions::HasScalingRatio,
            "comms::option::def::ScalingRatio option is not applicable to ArrayList field");
    static_assert(!ParsedOptions::HasUnits,
            "comms::option::def::Units option is not applicable to ArrayList field");
    static_assert(!ParsedOptions::HasMultiRangeValidation,
            "comms::option::def::ValidNumValueRange (or similar) option is not applicable to ArrayList field");
    static_assert((!ParsedOptions::HasOrigDataView) || (std::is_integral<TElement>::value && (sizeof(TElement) == sizeof(std::uint8_t))),
        "Usage of comms::option::app::OrigDataView option is allowed only for raw binary data (std::uint8_t) types.");
    static_assert(!ParsedOptions::HasVersionsRange,
            "comms::option::def::ExistsBetweenVersions (or similar) option is not applicable to ArrayList field");
    static_assert(!ParsedOptions::HasInvalidByDefault,
            "comms::option::def::InvalidByDefault option is not applicable to ArrayList field");
    static_assert(!ParsedOptions::HasMissingOnReadFail,
            "comms::option::def::MissingOnReadFail option is not applicable to ArrayList field");     
    static_assert(!ParsedOptions::HasMissingOnInvalid,
            "comms::option::def::MissingOnInvalid option is not applicable to ArrayList field");                       
};

/// @brief Equivalence comparison operator.
/// @details Performs lexicographical compare of two array fields.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case first field is less than second field.
/// @related ArrayList
template <typename TFieldBase, typename TElement, typename... TOptions>
bool operator<(
    const ArrayList<TFieldBase, TElement, TOptions...>& field1,
    const ArrayList<TFieldBase, TElement, TOptions...>& field2) noexcept
{
    return std::lexicographical_compare(
                field1.value().begin(), field1.value().end(),
                field2.value().begin(), field2.value().end());
}

/// @brief Equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case fields are equal, false otherwise.
/// @related ArrayList
template <typename TFieldBase, typename TElement, typename... TOptions>
bool operator==(
    const ArrayList<TFieldBase, TElement, TOptions...>& field1,
    const ArrayList<TFieldBase, TElement, TOptions...>& field2) noexcept
{
    auto& vec1 = field1.value();
    auto& vec2 = field2.value();
    if (vec1.size() != vec2.size()) {
        return false;
    }

    for (auto idx = 0U; idx < vec1.size(); ++idx) {
        if (vec1[idx] != vec2[idx]) {
            return false;
        }
    }
    return true;
}

/// @brief Non-equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case fields are NOT equal, false otherwise.
/// @related ArrayList
template <typename TFieldBase, typename TElement, typename... TOptions>
bool operator!=(
    const ArrayList<TFieldBase, TElement, TOptions...>& field1,
    const ArrayList<TFieldBase, TElement, TOptions...>& field2) noexcept
{
    return !(field1 == field2);
}

/// @brief Compile time check function of whether a provided type is any
///     variant of comms::field::ArrayList.
/// @tparam T Any type.
/// @return true in case provided type is any variant of @ref ArrayList
/// @related comms::field::ArrayList
template <typename T>
constexpr bool isArrayList()
{
    return 
        std::is_same<typename T::CommsTag, tag::ArrayList>::value || 
        std::is_same<typename T::CommsTag, tag::RawArrayList>::value;
}

/// @brief Upcast type of the field definition to its parent comms::field::ArrayList type
///     in order to have access to its internal types.
/// @related comms::field::ArrayList
template <typename TFieldBase, typename TElement, typename... TOptions>
inline
ArrayList<TFieldBase, TElement, TOptions...>&
toFieldBase(ArrayList<TFieldBase, TElement, TOptions...>& field)
{
    return field;
}

/// @brief Upcast type of the field definition to its parent comms::field::ArrayList type
///     in order to have access to its internal types.
/// @related comms::field::ArrayList
template <typename TFieldBase, typename TElement, typename... TOptions>
inline
const ArrayList<TFieldBase, TElement, TOptions...>&
toFieldBase(const ArrayList<TFieldBase, TElement, TOptions...>& field)
{
    return field;
}


}  // namespace field

}  // namespace comms

