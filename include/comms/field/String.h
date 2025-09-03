//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of @ref comms::field::String

#pragma once

#include "comms/CompileControl.h"
#include "comms/ErrorStatus.h"
#include "comms/details/tag.h"
#include "comms/field/basic/String.h"
#include "comms/field/details/AdaptBasicField.h"
#include "comms/field/details/OptionsParser.h"
#include "comms/options.h"
#include "comms/util/detect.h"
#include "comms/util/StaticString.h"
#include "comms/util/StringView.h"
#include "comms/util/type_traits.h"

#if COMMS_HAS_CPP17_STRING_VIEW
#include <string_view>
#endif // #if COMMS_HAS_CPP17_STRING_VIEW

#include <vector>
#include <utility>

namespace comms
{

namespace field
{

namespace details
{

template <bool THasOrigDataViewStorage>
struct StringOrigDataViewStorageType;

template <>
struct StringOrigDataViewStorageType<true>
{
#if COMMS_HAS_CPP17_STRING_VIEW
    using Type = std::string_view;
#else // #if COMMS_HAS_CPP17_STRING_VIEW
    using Type = comms::util::StringView;
#endif // #if COMMS_HAS_CPP17_STRING_VIEW
};

template <>
struct StringOrigDataViewStorageType<false>
{
    using Type = std::string;
};

template <bool THasSequenceFixedSizeUseFixedSizeStorage>
struct StringFixedSizeUseFixedSizeStorageType;

template <>
struct StringFixedSizeUseFixedSizeStorageType<true>
{
    template <typename TOpt>
    using Type = comms::util::StaticString<TOpt::SequenceFixedSize>;
};

template <>
struct StringFixedSizeUseFixedSizeStorageType<false>
{
    template <typename TOpt>
    using Type = typename StringOrigDataViewStorageType<TOpt::HasOrigDataView>::Type;
};

template <bool THasFixedSizeStorage>
struct StringFixedSizeStorageType;

template <>
struct StringFixedSizeStorageType<true>
{
    template <typename TOpt>
    using Type = comms::util::StaticString<TOpt::FixedSizeStorage>;
};

template <>
struct StringFixedSizeStorageType<false>
{
    template <typename TOpt>
    using Type = typename StringFixedSizeUseFixedSizeStorageType<TOpt::HasSequenceFixedSizeUseFixedSizeStorage>
        ::template Type<TOpt>;
};

template <bool THasCustomStorage>
struct StringCustomStringStorageType;

template <>
struct StringCustomStringStorageType<true>
{
    template <typename TOpt>
    using Type = typename TOpt::CustomStorageType;
};

template <>
struct StringCustomStringStorageType<false>
{
    template <typename TOpt>
    using Type =
        typename StringFixedSizeStorageType<TOpt::HasFixedSizeStorage>::template Type<TOpt>;
};

template <typename TOpt>
using StringStorageTypeT =
    typename StringCustomStringStorageType<TOpt::HasCustomStorageType>::template Type<TOpt>;

template <typename TFieldBase, typename... TOptions>
using StringBase =
    AdaptBasicFieldT<
        basic::String<TFieldBase, StringStorageTypeT<OptionsParser<TOptions...> > >,
        TOptions...
    >;

} // namespace details

/// @brief Field that represents a string.
/// @details By default uses
///     <a href="http://en.cppreference.com/w/cpp/string/basic_string">std::string</a>,
///     for internal storage, unless @ref comms::option::app::FixedSizeStorage option is used,
///     which forces usage of comms::util::StaticString instead.
/// @tparam TFieldBase Base class for this field, expected to be a variant of
///     comms::Field.
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
///     @li @ref comms::option::def::InvalidByDefault
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
///     @li @ref comms::option::app::OrigDataView
/// @extends comms::Field
/// @headerfile comms/field/String.h
template <typename TFieldBase, typename... TOptions>
class String : public details::StringBase<TFieldBase, TOptions...>
{
    using BaseImpl = details::StringBase<TFieldBase, TOptions...>;
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
    ///     ValueType is std::string, otherwise it becomes
    ///     comms::util::StaticString<TSize>, where TSize is a size
    ///     provided to @ref comms::option::app::FixedSizeStorage option.
    using ValueType = typename BaseImpl::ValueType;

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

    /// @brief Type of termination field suffix specified via @ref comms::option::def::SequenceTerminationFieldSuffix.
    /// @details @b void if @ref comms::option::def::SequenceTerminationFieldSuffix is not used.
    using TerminationFieldSuffix = typename ParsedOptions::SequenceTerminationFieldSuffix;    

    /// @brief Type of trailing field suffix specified via @ref comms::option::def::SequenceTrailingFieldSuffix.
    /// @details @b void if @ref comms::option::def::SequenceTrailingFieldSuffix is not used.
    using TrailingFieldSuffix = typename ParsedOptions::SequenceTrailingFieldSuffix;         

    /// @brief Default constructor
    String() = default;

    /// @brief Constructor
    explicit String(const ValueType& val)
      : BaseImpl(val)
    {
    }

    /// @brief Constructor
    explicit String(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    /// @brief Constructor
    explicit String(const char* str)
    {
        setValue(str);
    }

    /// @brief Copy constructor
    String(const String&) = default;

    /// @brief Move constructor
    String(String&&) = default;

    /// @brief Destructor
    ~String() noexcept = default;

    /// @brief Copy assignment
    String& operator=(const String&) = default;

    /// @brief Move assignment
    String& operator=(String&&) = default;

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
        auto es = BaseImpl::read(iter, len);
        using TagTmp = 
            typename comms::util::LazyShallowConditional<
                ParsedOptions::HasSequenceFixedSize
            >::template Type<
                AdjustmentNeededTag,
                NoAdjustmentTag
            >;

        adjustValue(TagTmp());
        return es;
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
        using TagTmp = 
            typename comms::util::LazyShallowConditional<
                ParsedOptions::HasSequenceFixedSize
            >::template Type<
                AdjustmentNeededTag,
                NoAdjustmentTag
            >;

        adjustValue(TagTmp());
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
    std::size_t length() const
    {
        return BaseImpl::length();
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

    /// @brief Check of whether the field has a consistent value for writing.
    bool canWrite() const
    {
        return BaseImpl::canWrite();
    }

    /// @brief Write current field value to output data sequence
    /// @details By default, the write operation will write all the
    ///     characters the field contains. If @ref comms::option::def::SequenceFixedSize option
    ///     is used, the number of characters, that is going to be written, is
    ///     exactly as the option specifies. If underlying string storage
    ///     doesn't contain enough data, the '\0' characters will
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

    /// @brief Force number of characters that must be read in the next read()
    ///     invocation.
    /// @details Exists only if @ref comms::option::def::SequenceSizeForcingEnabled option has been
    ///     used.
    /// @param[in] count Number of elements to read during following read operation.
    void forceReadElemCount(std::size_t count)
    {
        BaseImpl::forceReadElemCount(count);
    }

    /// @brief Clear forcing of the number of characters that must be read in
    ///     the next read() invocation.
    /// @details Exists only if @ref comms::option::def::SequenceSizeForcingEnabled option has been
    ///     used.
    void clearReadElemCount()
    {
        BaseImpl::clearReadElemCount();
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
    template <typename... TParams>
    using NoAdjustmentTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using AdjustmentNeededTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    using HasResizeTag = comms::details::tag::Tag3<>;

    template <typename... TParams>
    using HasRemoveSuffixTag = comms::details::tag::Tag4<>;    

    template <typename... TParams>
    void adjustValue(NoAdjustmentTag<TParams...>)
    {
    }

    template <typename... TParams>
    void adjustValue(AdjustmentNeededTag<TParams...>)
    {
        std::size_t count = 0;
        for (auto iter = BaseImpl::value().begin(); iter != BaseImpl::value().end(); ++iter) {
            if (*iter == 0) {
                break;
            }
            ++count;
        }

        doResize(count);
    }

    void doResize(std::size_t count)
    {
        using TagTmp =
            typename comms::util::Conditional<
                comms::util::detect::hasResizeFunc<ValueType>()
            >::template Type<
                HasResizeTag<>,
                typename comms::util::Conditional<
                    comms::util::detect::hasRemoveSuffixFunc<ValueType>()
                >::template Type<
                    HasRemoveSuffixTag<>,
                    void
                >
            >;

        static_assert(!std::is_void<TagTmp>::value,
            "The string storage value type must have either resize() or remove_suffix() "
            "member functions");
        doResize(count, TagTmp());
    }

    template <typename... TParams>
    void doResize(std::size_t count, HasResizeTag<TParams...>)
    {
        BaseImpl::value().resize(count);
    }

    template <typename... TParams>
    void doResize(std::size_t count, HasRemoveSuffixTag<TParams...>)
    {
        BaseImpl::value().remove_suffix(BaseImpl::value().size() - count);
    }

    static_assert(!ParsedOptions::HasSerOffset,
        "comms::option::def::NumValueSerOffset option is not applicable to String field");
    static_assert(!ParsedOptions::HasFixedLengthLimit,
        "comms::option::def::FixedLength option is not applicable to String field");
    static_assert(!ParsedOptions::HasFixedBitLengthLimit,
        "comms::option::def::FixedBitLength option is not applicable to String field");
    static_assert(!ParsedOptions::HasVarLengthLimits,
        "comms::option::def::VarLength option is not applicable to String field");
    static_assert(!ParsedOptions::HasAvailableLengthLimit,
            "comms::option::def::AvailableLengthLimit option is not applicable to String field");
    static_assert(!ParsedOptions::HasScalingRatio,
        "comms::option::def::ScalingRatio option is not applicable to String field");
    static_assert(!ParsedOptions::HasUnits,
        "comms::option::def::Units option is not applicable to String field");
    static_assert(!ParsedOptions::HasMultiRangeValidation,
        "comms::option::def::ValidNumValueRange (or similar) option is not applicable to String field");
    static_assert(!ParsedOptions::HasSequenceElemSerLengthFieldPrefix,
        "comms::option::def::SequenceElemSerLengthFieldPrefix option is not applicable to String field");
    static_assert(!ParsedOptions::HasSequenceElemFixedSerLengthFieldPrefix,
        "comms::option::def::SequenceElemSerLengthFixedFieldPrefix option is not applicable to String field");
    static_assert(!ParsedOptions::HasVersionsRange,
        "comms::option::def::ExistsBetweenVersions (or similar) option is not applicable to String field");
    static_assert(!ParsedOptions::HasMissingOnReadFail,
            "comms::option::def::MissingOnReadFail option is not applicable to String field");           
    static_assert(!ParsedOptions::HasMissingOnInvalid,
            "comms::option::def::MissingOnInvalid option is not applicable to String field");  
};

/// @brief Equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case fields are equal, false otherwise.
/// @related String
template <typename TFieldBase, typename... TOptions>
bool operator==(
    const String<TFieldBase, TOptions...>& field1,
    const String<TFieldBase, TOptions...>& field2) noexcept
{
    return field1.value() == field2.value();
}

/// @brief Non-equality comparison operator.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case fields are NOT equal, false otherwise.
/// @related String
template <typename TFieldBase, typename... TOptions>
bool operator!=(
    const String<TFieldBase, TOptions...>& field1,
    const String<TFieldBase, TOptions...>& field2) noexcept
{
    return field1.value() != field2.value();
}

/// @brief Equivalence comparison operator.
/// @details Performs lexicographical compare of two string values.
/// @param[in] field1 First field.
/// @param[in] field2 Second field.
/// @return true in case first field is less than second field.
/// @related String
template <typename TFieldBase, typename... TOptions>
bool operator<(
    const String<TFieldBase, TOptions...>& field1,
    const String<TFieldBase, TOptions...>& field2) noexcept
{
    return field1.value() < field2.value();
}

/// @brief Compile time check function of whether a provided type is any
///     variant of comms::field::String.
/// @tparam T Any type.
/// @return true in case provided type is any variant of @ref String
/// @related comms::field::String
template <typename T>
constexpr bool isString()
{
    return std::is_same<typename T::CommsTag, tag::String>::value;
}

/// @brief Upcast type of the field definition to its parent comms::field::String type
///     in order to have access to its internal types.
/// @related comms::field::String
template <typename TFieldBase, typename... TOptions>
inline
String<TFieldBase, TOptions...>&
toFieldBase(String<TFieldBase, TOptions...>& field)
{
    return field;
}

/// @brief Upcast type of the field definition to its parent comms::field::String type
///     in order to have access to its internal types.
/// @related comms::field::String
template <typename TFieldBase, typename... TOptions>
inline
const String<TFieldBase, TOptions...>&
toFieldBase(const String<TFieldBase, TOptions...>& field)
{
    return field;
}

}  // namespace field

}  // namespace comms


