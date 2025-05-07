//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#pragma once

#include "comms/details/base_detection.h"
#include "comms/details/gen_enum.h"
#include "comms/details/macro_common.h"

#include <tuple>
#include <type_traits>

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

#define COMMS_CHOOSE_BIT_ACC_FUNC_(N, f_, ...) COMMS_EXPAND(COMMS_BIT_ACC_FUNC_ ## N(f_, __VA_ARGS__))
#define COMMS_CHOOSE_BIT_ACC_FUNC(N, f_, ...) COMMS_EXPAND(COMMS_CHOOSE_BIT_ACC_FUNC_(N, f_, __VA_ARGS__))
#define COMMS_DO_BIT_ACC_FUNC(f_, ...) \
    COMMS_EXPAND(COMMS_CHOOSE_BIT_ACC_FUNC(COMMS_NUM_ARGS(__VA_ARGS__), f_, __VA_ARGS__))















