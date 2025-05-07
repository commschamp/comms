//
// Copyright 2014 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file comms/traits.h
/// @brief This file contains all the classes necessary to properly
/// define message traits.


#pragma once

#include "comms/util/access.h"

#include <ratio>

namespace comms
{

namespace traits
{

namespace endian
{

/// @brief Empty class used in traits to indicate Big Endian.
using Big = util::traits::endian::Big;

/// @brief Empty class used in traits to indicate Little Endian.
using Little = util::traits::endian::Little;

}  // namespace endian

namespace units
{

/// @brief Tag class used to indicate time value
struct Time {};

/// @brief Tag class used to indicate distance value
struct Distance {};

/// @brief Tag class used to indicate speed value
struct Speed {};

/// @brief Tag class used to indicate frequency value
struct Frequency {};

/// @brief Tag class used to indicate angle value
struct Angle {};

/// @brief Tag class used to indicate electrical current value
struct Current {};

/// @brief Tag class used to indicate electrical voltage value
struct Voltage {};

/// @brief Tag class used to indicate memory size value
struct Memory {};

/// @brief Ratio for nanoseconds
using NanosecondsRatio = std::nano;

/// @brief Ratio for microseconds
using MicrosecondsRatio = std::micro;

/// @brief Ratio for milliseconds
using MillisecondsRatio = std::milli;

/// @brief Ratio for seconds
using SecondsRatio = std::ratio<1, 1>;

/// @brief Ratio for minutes
using MinutesRatio = std::ratio<60>;

/// @brief Ratio for hours
using HoursRatio = std::ratio<60 * 60>;

/// @brief Ratio for days
using DaysRatio = std::ratio<24L * 60 * 60>;

/// @brief Ratio for weeks
using WeeksRatio = std::ratio<7L * 24 * 60 * 60>;

/// @brief Ratio for nanometers
using NanometersRatio = std::nano;

/// @brief Ratio for micrometers
using MicrometersRatio = std::micro;

/// @brief Ratio for millimeters
using MillimetersRatio = std::milli;

/// @brief Ratio for centimeters
using CentimetersRatio = std::centi;

/// @brief Ratio for meters
using MetersRatio = std::ratio<1, 1>;

/// @brief Ratio for kilometers
using KilometersRatio = std::kilo;

/// @brief Ratio for nanometers / second
using NanometersPerSecondRatio =
    typename std::ratio_divide<NanometersRatio, SecondsRatio>::type;

/// @brief Ratio for micrometers / second
using MicrometersPerSecondRatio =
    typename std::ratio_divide<MicrometersRatio, SecondsRatio>::type;

/// @brief Ratio for millimeters / second
using MillimetersPerSecondRatio =
    typename std::ratio_divide<MillimetersRatio, SecondsRatio>::type;

/// @brief Ratio for centimeters / second
using CentimetersPerSecondRatio =
    typename std::ratio_divide<CentimetersRatio, SecondsRatio>::type;

/// @brief Ratio for meters / second
using MetersPerSecondRatio =
    typename std::ratio_divide<MetersRatio, SecondsRatio>::type;

/// @brief Ratio for kilometers / second
using KilometersPerSecondRatio =
    typename std::ratio_divide<KilometersRatio, SecondsRatio>::type;

/// @brief Ratio for kilometers / hour
using KilometersPerHourRatio =
    typename std::ratio_divide<KilometersRatio, HoursRatio>::type;

/// @brief Ratio for hertz
using HzRatio = std::ratio<1, 1>;

/// @brief Ratio for kilohertz
using KiloHzRatio = std::kilo;

/// @brief Ratio for megahertz
using MegaHzRatio = std::mega;

/// @brief Ratio for gigahertz
using GigaHzRatio = std::giga;

/// @brief Ratio for degrees
using DegreesRatio = std::ratio<1, 1>;

/// @brief Ratio for radians
using RadiansRatio = std::ratio<180, 1>;

/// @brief Ratio for nanoamperes
using NanoampsRatio = std::nano;

/// @brief Ratio for microamperes
using MicroampsRatio = std::micro;

/// @brief Ratio for milliamperes
using MilliampsRatio = std::milli;

/// @brief Ratio for amperes
using AmpsRatio = std::ratio<1, 1>;

/// @brief Ratio for kiloamperes
using KiloampsRatio = std::kilo;

/// @brief Ratio for nanovolts
using NanovoltsRatio = std::nano;

/// @brief Ratio for microvolts
using MicrovoltsRatio = std::micro;

/// @brief Ratio for millivolts
using MillivoltsRatio = std::milli;

/// @brief Ratio for volts
using VoltsRatio = std::ratio<1, 1>;

/// @brief Ratio for kilovolts
using KilovoltsRatio = std::kilo;

/// @brief Ratio for bytes
using BytesRatio = std::ratio<1, 1>;

/// @brief Ratio for kilobytes
using KilobytesRatio = std::ratio<1024, 1>;

/// @brief Ratio for megabytes
using MegabytesRatio = std::ratio<1024UL * 1024, 1>;

/// @brief Ratio for gigabytes
using GigabytesRatio = std::ratio<1024ULL * 1024 * 1024, 1>;

/// @brief Ratio for terabytes
using TerabytesRatio = std::ratio<1024ULL * 1024 * 1024 * 1024, 1>;

} // namespace units

namespace dispatch
{

/// @brief Tag class used to indicate polymorphic dispatch
struct Polymorphic {};

/// @brief Tag class used to indicate static binary search dispatch
struct StaticBinSearch {};

/// @brief Tag class used to indicate linear switch dispatch
struct LinearSwitch {};

} // namespace dispatch

}  // namespace traits

}  // namespace comms

