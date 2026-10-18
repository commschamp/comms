//
// Copyright 2016 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/details/base_detection.h"
#include "comms/details/gen_enum.h"
#include "comms/details/macro_common.h"

#ifdef COMMS_MUST_DEFINE_BASE
#define COMMS_AS_BITMASK_FUNC Base& asBitmask()
#define COMMS_AS_BITMASK_CONST_FUNC const Base& asBitmask() const
#else // #ifdef COMMS_MUST_DEFINE_BASE
#define COMMS_AS_BITMASK_FUNC FUNC_AUTO_REF_RETURN(asBitmask, decltype(comms::field::toFieldBase(*this)))
#define COMMS_AS_BITMASK_CONST_FUNC FUNC_AUTO_REF_RETURN_CONST(asBitmask, decltype(comms::field::toFieldBase(*this)))
#endif // #ifdef COMMS_MUST_DEFINE_BASE

#define COMMS_BIT_ACC_FUNC(f_, n_) \
    bool COMMS_CONCATENATE(getBitValue_, n_)() const {\
        return f_.getBitValue(COMMS_CONCATENATE(BitIdx_, n_)); \
    } \
    void COMMS_CONCATENATE(setBitValue_, n_)(bool val) {\
        f_.setBitValue(COMMS_CONCATENATE(BitIdx_, n_), val); \
    }

#define COMMS_BIT_ACC_FUNC_1(f_, n_) COMMS_BIT_ACC_FUNC(f_, n_)
#define COMMS_BIT_ACC_FUNC_2(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_1(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_3(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_2(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_4(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_3(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_5(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_4(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_6(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_5(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_7(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_6(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_8(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_7(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_9(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_8(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_10(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_9(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_11(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_10(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_12(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_11(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_13(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_12(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_14(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_13(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_15(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_14(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_16(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_15(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_17(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_16(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_18(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_17(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_19(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_18(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_20(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_19(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_21(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_20(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_22(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_21(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_23(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_22(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_24(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_23(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_25(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_24(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_26(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_25(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_27(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_26(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_28(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_27(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_29(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_28(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_30(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_29(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_31(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_30(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_32(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_31(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_33(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_32(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_34(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_33(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_35(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_34(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_36(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_35(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_37(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_36(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_38(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_37(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_39(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_38(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_40(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_39(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_41(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_40(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_42(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_41(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_43(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_42(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_44(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_43(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_45(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_44(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_46(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_45(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_47(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_46(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_48(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_47(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_49(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_48(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_50(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_49(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_51(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_50(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_52(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_51(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_53(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_52(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_54(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_53(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_55(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_54(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_56(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_55(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_57(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_56(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_58(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_57(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_59(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_58(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_60(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_59(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_61(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_60(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_62(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_61(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_63(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_62(f_, __VA_ARGS__))
#define COMMS_BIT_ACC_FUNC_64(f_, n_, ...) \
    COMMS_BIT_ACC_FUNC(f_, n_) \
    COMMS_EXPAND(COMMS_BIT_ACC_FUNC_63(f_, __VA_ARGS__))

#define COMMS_CHOOSE_BIT_ACC_FUNC_(N, f_, ...) COMMS_EXPAND(COMMS_BIT_ACC_FUNC_ ## N(f_, __VA_ARGS__))
#define COMMS_CHOOSE_BIT_ACC_FUNC(N, f_, ...) COMMS_EXPAND(COMMS_CHOOSE_BIT_ACC_FUNC_(N, f_, __VA_ARGS__))
#define COMMS_DO_BIT_ACC_FUNC(f_, ...) \
    COMMS_EXPAND(COMMS_CHOOSE_BIT_ACC_FUNC(COMMS_NUM_ARGS(__VA_ARGS__), f_, __VA_ARGS__))

// ---------------------------------------------------------------------

#define COMMS_WRAP_MASK_BIT(b_) (static_cast<ValueType>(1ULL << COMMS_CONCATENATE(BitIdx_, b_)))

#define COMMS_MASK_VAL_0(...)
#define COMMS_MASK_VAL_1(b_) COMMS_WRAP_MASK_BIT(b_)
#define COMMS_MASK_VAL_2(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_1(__VA_ARGS__))
#define COMMS_MASK_VAL_3(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_2(__VA_ARGS__))
#define COMMS_MASK_VAL_4(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_3(__VA_ARGS__))
#define COMMS_MASK_VAL_5(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_4(__VA_ARGS__))
#define COMMS_MASK_VAL_6(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_5(__VA_ARGS__))
#define COMMS_MASK_VAL_7(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_6(__VA_ARGS__))
#define COMMS_MASK_VAL_8(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_7(__VA_ARGS__))
#define COMMS_MASK_VAL_9(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_8(__VA_ARGS__))
#define COMMS_MASK_VAL_10(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_9(__VA_ARGS__))
#define COMMS_MASK_VAL_11(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_10(__VA_ARGS__))
#define COMMS_MASK_VAL_12(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_11(__VA_ARGS__))
#define COMMS_MASK_VAL_13(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_12(__VA_ARGS__))
#define COMMS_MASK_VAL_14(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_13(__VA_ARGS__))
#define COMMS_MASK_VAL_15(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_14(__VA_ARGS__))
#define COMMS_MASK_VAL_16(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_15(__VA_ARGS__))
#define COMMS_MASK_VAL_17(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_16(__VA_ARGS__))
#define COMMS_MASK_VAL_18(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_17(__VA_ARGS__))
#define COMMS_MASK_VAL_19(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_18(__VA_ARGS__))
#define COMMS_MASK_VAL_20(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_19(__VA_ARGS__))
#define COMMS_MASK_VAL_21(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_20(__VA_ARGS__))
#define COMMS_MASK_VAL_22(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_21(__VA_ARGS__))
#define COMMS_MASK_VAL_23(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_22(__VA_ARGS__))
#define COMMS_MASK_VAL_24(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_23(__VA_ARGS__))
#define COMMS_MASK_VAL_25(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_24(__VA_ARGS__))
#define COMMS_MASK_VAL_26(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_25(__VA_ARGS__))
#define COMMS_MASK_VAL_27(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_26(__VA_ARGS__))
#define COMMS_MASK_VAL_28(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_27(__VA_ARGS__))
#define COMMS_MASK_VAL_29(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_28(__VA_ARGS__))
#define COMMS_MASK_VAL_30(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_29(__VA_ARGS__))
#define COMMS_MASK_VAL_31(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_30(__VA_ARGS__))
#define COMMS_MASK_VAL_32(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_31(__VA_ARGS__))
#define COMMS_MASK_VAL_33(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_32(__VA_ARGS__))
#define COMMS_MASK_VAL_34(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_33(__VA_ARGS__))
#define COMMS_MASK_VAL_35(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_34(__VA_ARGS__))
#define COMMS_MASK_VAL_36(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_35(__VA_ARGS__))
#define COMMS_MASK_VAL_37(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_36(__VA_ARGS__))
#define COMMS_MASK_VAL_38(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_37(__VA_ARGS__))
#define COMMS_MASK_VAL_39(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_38(__VA_ARGS__))
#define COMMS_MASK_VAL_40(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_39(__VA_ARGS__))
#define COMMS_MASK_VAL_41(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_40(__VA_ARGS__))
#define COMMS_MASK_VAL_42(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_41(__VA_ARGS__))
#define COMMS_MASK_VAL_43(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_42(__VA_ARGS__))
#define COMMS_MASK_VAL_44(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_43(__VA_ARGS__))
#define COMMS_MASK_VAL_45(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_44(__VA_ARGS__))
#define COMMS_MASK_VAL_46(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_45(__VA_ARGS__))
#define COMMS_MASK_VAL_47(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_46(__VA_ARGS__))
#define COMMS_MASK_VAL_48(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_47(__VA_ARGS__))
#define COMMS_MASK_VAL_49(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_48(__VA_ARGS__))
#define COMMS_MASK_VAL_50(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_49(__VA_ARGS__))
#define COMMS_MASK_VAL_51(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_50(__VA_ARGS__))
#define COMMS_MASK_VAL_52(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_51(__VA_ARGS__))
#define COMMS_MASK_VAL_53(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_52(__VA_ARGS__))
#define COMMS_MASK_VAL_54(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_53(__VA_ARGS__))
#define COMMS_MASK_VAL_55(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_54(__VA_ARGS__))
#define COMMS_MASK_VAL_56(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_55(__VA_ARGS__))
#define COMMS_MASK_VAL_57(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_56(__VA_ARGS__))
#define COMMS_MASK_VAL_58(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_57(__VA_ARGS__))
#define COMMS_MASK_VAL_59(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_58(__VA_ARGS__))
#define COMMS_MASK_VAL_60(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_59(__VA_ARGS__))
#define COMMS_MASK_VAL_61(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_60(__VA_ARGS__))
#define COMMS_MASK_VAL_62(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_61(__VA_ARGS__))
#define COMMS_MASK_VAL_63(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_62(__VA_ARGS__))
#define COMMS_MASK_VAL_64(b_, ...) COMMS_WRAP_MASK_BIT(b_) | COMMS_EXPAND(COMMS_MASK_VAL_63(__VA_ARGS__))

#define COMMS_CHOOSE_MASK_(N, ...) COMMS_EXPAND(COMMS_MASK_VAL_ ## N(__VA_ARGS__))
#define COMMS_CHOOSE_MASK(N, ...) COMMS_EXPAND(COMMS_CHOOSE_MASK_(N, __VA_ARGS__))
#define COMMS_DO_MASK(...) COMMS_EXPAND(COMMS_CHOOSE_MASK(COMMS_NUM_ARGS(__VA_ARGS__), __VA_ARGS__))

#define COMMS_DEFINE_BIT_MASK(m_, ...) \
    static constexpr ValueType COMMS_CONCATENATE(BitMask_, m_) = \
        COMMS_EXPAND(COMMS_DO_MASK(__VA_ARGS__)) \
        ;

// ---------------------------------------------------------------------

#define COMMS_BIT_MASK_FUNC(n_) \
    bool COMMS_CONCATENATE(hasAllBitsSet_, n_)() const {\
        return asBitmask().hasAllBitsSet(COMMS_CONCATENATE(BitMask_, n_)); \
    } \
    bool COMMS_CONCATENATE(hasAnyBitsSet_, n_)() const {\
        return asBitmask().hasAnyBitsSet(COMMS_CONCATENATE(BitMask_, n_)); \
    } \
    bool COMMS_CONCATENATE(hasAllBitsCleared_, n_)() const {\
        return asBitmask().hasAllBitsCleared(COMMS_CONCATENATE(BitMask_, n_)); \
    } \
    void COMMS_CONCATENATE(setBits_, n_)() {\
        asBitmask().setBits(COMMS_CONCATENATE(BitMask_, n_)); \
    } \
    void COMMS_CONCATENATE(clearBits_, n_)() {\
        asBitmask().clearBits(COMMS_CONCATENATE(BitMask_, n_)); \
    }

#define COMMS_CHOOSE_MASK_FUNC(m_, ...) COMMS_BIT_MASK_FUNC(m_)
#define COMMS_DO_MASK_FUNC(...) COMMS_EXPAND(COMMS_CHOOSE_MASK_FUNC(__VA_ARGS__))
