//
// Copyright 2015 - 2023 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "adapters.h"
#include "OptionsParser.h"

namespace comms
{

namespace field
{

namespace details
{

template<
    bool T1 = false,
    bool T2 = false,
    bool T3 = false,
    bool T4 = false,
    bool T5 = false,
    bool T6 = false>
struct FieldsOptionsCompatibilityCalc
{
    static const std::size_t Value =
        static_cast<std::size_t>(T1) +\
        static_cast<std::size_t>(T2) +
        static_cast<std::size_t>(T3) +
        static_cast<std::size_t>(T4) +
        static_cast<std::size_t>(T5) +
        static_cast<std::size_t>(T6);
};

//--

template <typename TBasic, typename... TOptions>
class AdaptBasicField
{
    using ParsedOptions = OptionsParser<TOptions...>;

    static const bool CustomReaderIncompatible =
            ParsedOptions::HasSerOffset ||
            ParsedOptions::HasFixedLengthLimit ||
            ParsedOptions::HasFixedBitLengthLimit ||
            ParsedOptions::HasVarLengthLimits ||
            ParsedOptions::HasAvailableLengthLimit ||
            ParsedOptions::HasSequenceElemLengthForcing ||
            ParsedOptions::HasSequenceSizeForcing ||
            ParsedOptions::HasSequenceLengthForcing ||
            ParsedOptions::HasSequenceFixedSize ||
            ParsedOptions::HasSequenceSizeFieldPrefix ||
            ParsedOptions::HasSequenceSerLengthFieldPrefix ||
            ParsedOptions::HasSequenceElemSerLengthFieldPrefix ||
            ParsedOptions::HasSequenceElemFixedSerLengthFieldPrefix ||
            ParsedOptions::HasSequenceTrailingFieldSuffix ||
            ParsedOptions::HasSequenceTerminationFieldSuffix ||
            ParsedOptions::HasEmptySerialization ||
            ParsedOptions::HasMissingOnReadFail;


    static const bool VarLengthIncompatible =
            ParsedOptions::HasFixedLengthLimit ||
            ParsedOptions::HasFixedBitLengthLimit ||
            ParsedOptions::HasAvailableLengthLimit;

    static_assert(
            (!ParsedOptions::HasVarLengthLimits) || (!VarLengthIncompatible),
            "VarLength option is incompatible with FixedLength, FixedBitLength "
            "and AvailableLengthLimit");

    static_assert(
            1U >= FieldsOptionsCompatibilityCalc<
                ParsedOptions::HasSequenceSizeFieldPrefix,
                ParsedOptions::HasSequenceSerLengthFieldPrefix,
                ParsedOptions::HasSequenceFixedSize,
                ParsedOptions::HasSequenceSizeForcing,
                ParsedOptions::HasSequenceLengthForcing,
                ParsedOptions::HasSequenceTerminationFieldSuffix>::Value,
            "The following options are incompatible, cannot be used together: "
            "SequenceSizeFieldPrefix, SequenceSerLengthFieldPrefix, "
            "SequenceFixedSize, SequenceSizeForcingEnabled, SequenceLengthForcingEnabled, "
            "SequenceTerminationFieldSuffix");

    static_assert(
            1U >= FieldsOptionsCompatibilityCalc<
                ParsedOptions::HasSequenceElemSerLengthFieldPrefix,
                ParsedOptions::HasSequenceElemFixedSerLengthFieldPrefix,
                ParsedOptions::HasSequenceTerminationFieldSuffix>::Value,
            "The following options are incompatible, cannot be used together: "
            "SequenceElemSerLengthFieldPrefix, SequenceElemFixedSerLengthFieldPrefix "
            "SequenceTerminationFieldSuffix");

    static_assert(
            (!ParsedOptions::HasSequenceTrailingFieldSuffix) ||
            (!ParsedOptions::HasSequenceTerminationFieldSuffix),
            "The following options are incompatible, cannot be used together: "
            "SequenceTrailingFieldSuffix, SequenceTerminationFieldSuffix");

    static_assert(
            (!ParsedOptions::HasFailOnInvalid) ||
            (!ParsedOptions::HasIgnoreInvalid),
            "The following options are incompatible, cannot be used together: "
            "FailOnInvalid, IgnoreInvalid");

    static_assert(
            1U >= FieldsOptionsCompatibilityCalc<
                ParsedOptions::HasCustomStorageType,
                ParsedOptions::HasFixedSizeStorage,
                ParsedOptions::HasOrigDataView>::Value,
            "The following options are incompatible, cannot be used together: "
            "CustomStorageType, FixedSizeStorage, OrigDataView");

    static_assert(
            (!ParsedOptions::HasSequenceFixedSizeUseFixedSizeStorage) ||
            (ParsedOptions::HasSequenceFixedSize),
            "The option SequenceFixedSizeUseFixedSizeStorage cannot be used without SequenceFixedSize.");

    static_assert(
            (!ParsedOptions::HasSequenceFixedSizeUseFixedSizeStorage) ||
            (!ParsedOptions::HasFixedSizeStorage),
            "The following options are incompatible, cannot be used together: "
            "SequenceFixedSizeUseFixedSizeStorage, FixedSizeStorage");

    using FieldTypeAdapted = 
        typename ParsedOptions::template AdaptFieldType<TBasic>;            

    using InvalidByDefaultAdapted = 
        typename ParsedOptions::template AdaptInvalidByDefault<FieldTypeAdapted>;
        
    using VersionStorageAdapted = 
        typename ParsedOptions::template AdaptVersionStorage<InvalidByDefaultAdapted>;
    
    using SerOffsetAdapted = 
        typename ParsedOptions::template AdaptSerOffset<VersionStorageAdapted>;

    using VersionsRangeAdapted = 
        typename ParsedOptions::template AdaptVersionsRange<SerOffsetAdapted>;

    using FixedLengthLimitAdapted = 
        typename ParsedOptions::template AdaptFixedLengthLimit<VersionsRangeAdapted>;

    using FixedBitLengthLimitAdapted = 
        typename ParsedOptions::template AdaptFixedBitLengthLimit<FixedLengthLimitAdapted>;

    using VarLengthLimitsAdapted = 
        typename ParsedOptions::template AdaptVarLengthLimits<FixedBitLengthLimitAdapted>;

    using AvailableLengthLimitAdapted =
        typename ParsedOptions::template AdaptAvailableLengthLimit<VarLengthLimitsAdapted>;       

    using SequenceElemLengthForcingAdapted = 
        typename ParsedOptions::template AdaptSequenceElemLengthForcing<AvailableLengthLimitAdapted>;
    
    using SequenceElemSerLengthFieldPrefixAdapted = 
        typename ParsedOptions::template AdaptSequenceElemSerLengthFieldPrefix<SequenceElemLengthForcingAdapted>;

    using SequenceElemFixedSerLengthFieldPrefixAdapted = 
        typename ParsedOptions::template AdaptSequenceElemFixedSerLengthFieldPrefix<SequenceElemSerLengthFieldPrefixAdapted>;

    using SequenceSizeForcingAdapted = 
        typename ParsedOptions::template AdaptSequenceSizeForcing<SequenceElemFixedSerLengthFieldPrefixAdapted>;

    using SequenceLengthForcingAdapted = 
        typename ParsedOptions::template AdaptSequenceLengthForcing<SequenceSizeForcingAdapted>;
        
    using SequenceFixedSizeAdapted = 
        typename ParsedOptions::template AdaptSequenceFixedSize<SequenceLengthForcingAdapted>;

    using SequenceSizeFieldPrefixAdapted = 
        typename ParsedOptions::template AdaptSequenceSizeFieldPrefix<SequenceFixedSizeAdapted>;

    using SequenceSerLengthFieldPrefixAdapted = 
        typename ParsedOptions::template AdaptSequenceSerLengthFieldPrefix<SequenceSizeFieldPrefixAdapted>;
    
    using SequenceTrailingFieldSuffixAdapted = 
        typename ParsedOptions::template AdaptSequenceTrailingFieldSuffix<SequenceSerLengthFieldPrefixAdapted>;
    
    using SequenceTerminationFieldSuffixAdapted = 
        typename ParsedOptions::template AdaptSequenceTerminationFieldSuffix<SequenceTrailingFieldSuffixAdapted>;
    
     using RemLengthMemberFieldAdapted = 
        typename ParsedOptions::template AdaptRemLengthMemberField<SequenceTerminationFieldSuffixAdapted>;

    using DefaultValueInitialiserAdapted = 
        typename ParsedOptions::template AdaptDefaultValueInitialiser<RemLengthMemberFieldAdapted>;

    using MultiRangeValidationAdapted = 
        typename ParsedOptions::template AdaptMultiRangeValidation<DefaultValueInitialiserAdapted>;
    
    using CustomValidatorAdapted = 
        typename ParsedOptions::template AdaptCustomValidator<MultiRangeValidationAdapted>;

    using FailOnInvalidAdapted = 
        typename ParsedOptions::template AdaptFailOnInvalid<CustomValidatorAdapted>;

    using IgnoreInvalidAdapted = 
        typename ParsedOptions::template AdaptIgnoreInvalid<FailOnInvalidAdapted>;

    using EmptySerializationAdapted = 
        typename ParsedOptions::template AdaptEmptySerialization<IgnoreInvalidAdapted>;

    using CustomReadWrapAdapted = 
        typename ParsedOptions::template AdaptCustomRead<EmptySerializationAdapted>;

    using CustomRefreshWrapAdapted = 
        typename ParsedOptions::template AdaptCustomRefresh<CustomReadWrapAdapted>;

    using CustomWriteWrapAdapted = 
        typename ParsedOptions::template AdaptCustomWrite<CustomRefreshWrapAdapted>;

    using MissingOnInvalidAdapted = 
        typename ParsedOptions::template AdaptMissingOnInvalid<CustomWriteWrapAdapted>;

    using MissingOnReadFailAdapted = 
        typename ParsedOptions::template AdaptMissingOnReadFail<MissingOnInvalidAdapted>;

    using VariantResetOnDestructAdapted = 
        typename ParsedOptions::template AdaptVariantResetOnDestruct<MissingOnReadFailAdapted>;

public:
    using Type = VariantResetOnDestructAdapted;
};

template <typename TBasic, typename... TOptions>
using AdaptBasicFieldT = typename AdaptBasicField<TBasic, TOptions...>::Type;

}  // namespace details

}  // namespace field

}  // namespace comms


