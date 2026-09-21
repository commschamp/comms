//
// Copyright 2014 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains functions for raw data access / (de)serialization.

#pragma once

#include "comms/traits.h"
#include "comms/util/details/access_internal.h"

#include <cstddef>
#include <type_traits>

namespace comms
{

namespace util
{

/// @brief Write part of integral value into the output area using big
///     endian notation.
/// @tparam TSize Number of bytes to write.
/// @param[in] value Integral type value to be written.
/// @param[in, out] iter Output iterator.
/// @pre TSize <= sizeof(T).
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least TSize times.
/// @post The iterator is advanced.
template <std::size_t TSize, typename T, typename TIter>
void writeBig(T value, TIter& iter)
{
    comms::util::details::WriteHelper<>::template write<comms::traits::endian::Big>(value, TSize, iter);
}

/// @brief Write part of integral value into the output area using big
///     endian notation.
/// @param[in] value Integral type value to be written.
/// @param[in] size Number of bytes to write.
/// @param[in, out] iter Output iterator.
/// @pre TSize <= sizeof(T).
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least TSize times.
/// @post The iterator is advanced.
template <typename T, typename TIter>
void writeBig(T value, std::size_t size, TIter& iter)
{
    comms::util::details::WriteHelper<>::template write<comms::traits::endian::Big>(value, size, iter);
}

/// @brief Write integral value into the output area using big
///     endian notation.
/// @param[in] value Integral type value to be written.
/// @param[in, out] iter Output iterator.
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least sizeof(T) times.
/// @post The iterator is advanced.
template <typename T, typename TIter>
void writeBig(T value, TIter& iter)
{
    using ValueType = typename std::decay<T>::type;
    writeBig<sizeof(ValueType)>(static_cast<ValueType>(value), iter);
}

/// @brief Read part of integral value from the input area using big
///     endian notation.
/// @tparam T Type to read.
/// @tparam TSize Number of bytes to read.
/// @param[in, out] iter Input iterator.
/// @return Read value
/// @pre TSize <= sizeof(T).
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least TSize times.
/// @post The iterator is advanced.
template <typename T, std::size_t TSize, typename TIter>
T readBig(TIter& iter)
{
    return comms::util::details::ReadHelper<>::template read<comms::traits::endian::Big, T, TSize>(iter);
}

/// @brief Read integral value from the input area using big
///     endian notation.
/// @tparam T Type to read.
/// @param[in, out] iter Input iterator.
/// @return Read value
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least sizeof(T) times.
/// @post The iterator is advanced.
template <typename T, typename TIter>
T readBig(TIter& iter)
{
    using ValueType = typename std::decay<T>::type;
    return static_cast<T>(readBig<ValueType, sizeof(ValueType)>(iter));
}

/// @brief Read integral value from the input area using big
///     endian notation.
/// @tparam T Type to read.
/// @param[in, out] iter Input iterator.
/// @param[in] size length to read
/// @return Read value
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least sizeof(T) times.
/// @post The iterator is advanced.
template <typename T, typename TIter>
T readBig(TIter& iter, std::size_t size)
{
    return comms::util::details::ReadHelper<>::template read<comms::traits::endian::Big, T>(size, iter);
}

/// @brief Write part of integral value into the output area using little
///     endian notation.
/// @tparam TSize Number of bytes to write.
/// @param[in] value Integral type value to be written.
/// @param[in, out] iter Output iterator.
/// @pre TSize <= sizeof(T).
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least TSize times.
/// @post The iterator is advanced.
template <std::size_t TSize, typename T, typename TIter>
void writeLittle(T value, TIter& iter)
{
    comms::util::details::WriteHelper<>::template write<comms::traits::endian::Little>(value, TSize, iter);
}

/// @brief Write part of integral value into the output area using little
///     endian notation.
/// @param[in] value Integral type value to be written.
/// @param[in] size Number of bytes to write.
/// @param[in, out] iter Output iterator.
/// @pre TSize <= sizeof(T).
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least TSize times.
/// @post The iterator is advanced.
template <typename T, typename TIter>
void writeLittle(T value, std::size_t size, TIter& iter)
{
    comms::util::details::WriteHelper<>::template write<comms::traits::endian::Little>(value, size, iter);
}

/// @brief Write integral value into the output area using big
///     endian notation.
/// @param[in] value Integral type value to be written.
/// @param[in, out] iter Output iterator.
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least sizeof(T) times.
/// @post The iterator is advanced.
template <typename T, typename TIter>
void writeLittle(T value, TIter& iter)
{
    using ValueType = typename std::decay<T>::type;
    writeLittle<sizeof(ValueType)>(static_cast<ValueType>(value), iter);
}

/// @brief Read part of integral value from the input area using little
///     endian notation.
/// @tparam T Type to read.
/// @tparam TSize Number of bytes to read.
/// @param[in, out] iter Input iterator.
/// @return Read value
/// @pre TSize <= sizeof(T).
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least TSize times.
/// @post The iterator is advanced.
template <typename T, std::size_t TSize, typename TIter>
T readLittle(TIter& iter)
{
    return comms::util::details::ReadHelper<>::template read<comms::traits::endian::Little, T, TSize>(iter);
}

/// @brief Read integral value from the input area using little
///     endian notation.
/// @tparam T Type to read.
/// @param[in, out] iter Input iterator.
/// @return Read value
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least sizeof(T) times.
/// @post The iterator is advanced.
template <typename T, typename TIter>
T readLittle(TIter& iter)
{
    using ValueType = typename std::decay<T>::type;
    return static_cast<T>(readLittle<ValueType, sizeof(ValueType)>(iter));
}

/// @brief Read integral value from the input area using little
///     endian notation.
/// @tparam T Type to read.
/// @param[in, out] iter Input iterator.
/// @param[in] size length to read
/// @return Read value
/// @pre The iterator must be valid and can be successfully dereferenced
///      and incremented at least sizeof(T) times.
/// @post The iterator is advanced.
template <typename T, typename TIter>
T readLittle(TIter& iter, std::size_t size)
{
    return comms::util::details::ReadHelper<>::template read<comms::traits::endian::Little, T>(size, iter);
}

/// @brief Same as writeBig<T, TIter>()
template <typename T, typename TIter>
void writeData(T value, TIter& iter, const comms::traits::endian::Big& endian)
{
    static_cast<void>(endian);
    writeBig(value, iter);
}

/// @brief Same as writeBig<TSize, T, TIter>()
template <std::size_t TSize, typename T, typename TIter>
void writeData(T value, TIter& iter, const comms::traits::endian::Big& endian)
{
    static_cast<void>(endian);
    writeBig<TSize>(value, iter);
}

/// @brief Same as writeBig<T, TIter>()
template <typename T, typename TIter>
void writeData(T value, std::size_t size, TIter& iter, const comms::traits::endian::Big& endian)
{
    static_cast<void>(endian);
    writeBig(value, size, iter);
}

/// @brief Same as writeLittle<T, TIter>()
template <typename T, typename TIter>
void writeData(T value, TIter& iter, const comms::traits::endian::Little& endian)
{
    static_cast<void>(endian);
    writeLittle(value, iter);
}

/// @brief Same as writeLittle<TSize, T, TIter>()
template <std::size_t TSize, typename T, typename TIter>
void writeData(T value, TIter& iter, const comms::traits::endian::Little& endian)
{
    static_cast<void>(endian);
    return writeLittle<TSize>(value, iter);
}

/// @brief Same as writeLittle<T, TIter>()
template <typename T, typename TIter>
void writeData(T value, std::size_t size, TIter& iter, const comms::traits::endian::Little& endian)
{
    static_cast<void>(endian);
    writeLittle(value, size, iter);
}

/// @brief Same as readBig<T, TIter>()
template <typename T, typename TIter>
T readData(TIter& iter, const comms::traits::endian::Big& endian)
{
    static_cast<void>(endian);
    return readBig<T>(iter);
}

/// @brief Same as readBig<T, TIter>()
template <typename T, typename TIter>
T readData(TIter& iter, std::size_t size, const comms::traits::endian::Big& endian)
{
    static_cast<void>(endian);
    return readBig<T>(iter, size);
}

/// @brief Same as readBig<T, TSize, TIter>()
template <typename T, std::size_t TSize, typename TIter>
T readData(TIter& iter, const comms::traits::endian::Big& endian)
{
    static_cast<void>(endian);
    return readBig<T, TSize>(iter);
}

/// @brief Same as readLittle<T, TIter>()
template <typename T, typename TIter>
T readData(TIter& iter, const comms::traits::endian::Little& endian)
{
    static_cast<void>(endian);
    return readLittle<T>(iter);
}

/// @brief Same as readData<T, TSize, TIter>()
template <typename T, std::size_t TSize, typename TIter>
T readData(TIter& iter, const comms::traits::endian::Little& endian)
{
    static_cast<void>(endian);
    return readLittle<T, TSize>(iter);
}

/// @brief Same as readBig<T, TIter>()
template <typename T, typename TIter>
T readData(TIter& iter, std::size_t size, const comms::traits::endian::Little& endian)
{
    static_cast<void>(endian);
    return readLittle<T>(iter, size);
}

}  // namespace util

}  // namespace comms
