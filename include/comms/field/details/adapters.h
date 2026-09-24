//
// Copyright 2015 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/field/adapter/AvailableLengthAdapter.h"
#include "comms/field/adapter/CustomValidatorAdapter.h"
#include "comms/field/adapter/CustomReadWrapAdapter.h"
#include "comms/field/adapter/CustomRefreshWrapAdapter.h"
#include "comms/field/adapter/CustomWriteWrapAdapter.h"
#include "comms/field/adapter/DefaultValueInitialiserAdapter.h"
#include "comms/field/adapter/DisplayOffsetAdapter.h"
#include "comms/field/adapter/EmptySerializationAdapter.h"
#include "comms/field/adapter/ExistsBetweenVersionsAdapter.h"
#include "comms/field/adapter/FailOnInvalidAdapter.h"
#include "comms/field/adapter/FieldTypeAdapter.h"
#include "comms/field/adapter/FixedBitLengthAdapter.h"
#include "comms/field/adapter/FixedLengthAdapter.h"
#include "comms/field/adapter/FixedValueAdapter.h"
#include "comms/field/adapter/IgnoreInvalidAdapter.h"
#include "comms/field/adapter/InvalidByDefaultAdapter.h"
#include "comms/field/adapter/MissingOnInvalidAdapter.h"
#include "comms/field/adapter/MissingOnReadFailAdapter.h"
#include "comms/field/adapter/NumValueMultiRangeValidatorAdapter.h"
#include "comms/field/adapter/RemLengthMemberFieldAdapter.h"
#include "comms/field/adapter/SequenceElemFixedSerLengthFieldPrefixAdapter.h"
#include "comms/field/adapter/SequenceElemLengthForcingAdapter.h"
#include "comms/field/adapter/SequenceElemSerLengthFieldPrefixAdapter.h"
#include "comms/field/adapter/SequenceFixedSizeAdapter.h"
#include "comms/field/adapter/SequenceLengthForcingAdapter.h"
#include "comms/field/adapter/SequenceSerLengthFieldPrefixAdapter.h"
#include "comms/field/adapter/SequenceSizeFieldPrefixAdapter.h"
#include "comms/field/adapter/SequenceSizeForcingAdapter.h"
#include "comms/field/adapter/SequenceTrailingFieldSuffixAdapter.h"
#include "comms/field/adapter/SequenceTerminationFieldSuffixAdapter.h"
#include "comms/field/adapter/SerOffsetAdapter.h"
#include "comms/field/adapter/VariantResetOnDestructAdapter.h"
#include "comms/field/adapter/VarLengthAdapter.h"
#include "comms/field/adapter/VersionStorageAdapter.h"
