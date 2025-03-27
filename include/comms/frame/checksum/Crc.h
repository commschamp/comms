//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file 
/// @brief Contains definition of @ref comms::frame::checksum::Crc

#pragma once

#include "comms/cast.h"
#include "comms/details/tag.h"
#include "comms/util/type_traits.h"

#include <array>
#include <cstdint>
#include <limits>
#include <type_traits>


namespace comms
{

namespace frame
{

namespace checksum
{

namespace details
{

template <typename TResult>
using CrcInitTableType = std::array<TResult, 256>;

template <typename TResult, TResult TPoly>
struct CrcInitTable
{
    using Table = CrcInitTableType<TResult>;
    static const Table& get()
    {
        static Table table;
        static bool tableFilled = false;

        if (!tableFilled) {
            fillTable(table);
            tableFilled = true;
        }

        return table;
    }

private:
    static void fillTable(Table& table)
    {
        static const std::size_t Width =
            sizeof(TResult) * std::numeric_limits<std::uint8_t>::digits;
        static const auto Msb =
            static_cast<TResult>(1) << (Width - 1);

        for (unsigned idx = 0U; idx < table.size(); ++idx)
        {
            auto rem = static_cast<TResult>(idx << (Width - 8));

            for (auto bit = 8U; bit > 0U; --bit)
            {
                if ((rem & Msb) != 0)
                {
                    rem = (rem << 1) ^ TPoly;
                }
                else
                {
                    rem = (rem << 1);
                }
            }

            table[idx] = rem;
        }
    }
};

template <>
struct CrcInitTable<std::uint16_t, 0x1021>
{
    using Table = CrcInitTableType<std::uint16_t>;
    static const Table& get()
    {
        static const Table table = {{
            0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
            0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
            0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
            0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
            0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
            0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
            0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
            0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
            0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
            0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
            0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
            0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
            0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
            0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
            0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
            0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
            0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
            0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
            0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
            0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
            0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
            0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
            0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
            0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
            0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
            0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
            0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
            0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
            0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
            0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
            0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
            0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
        }};
        return table;
    }
};

template <>
struct CrcInitTable<std::uint16_t, 0x8005>
{
    using Table = CrcInitTableType<std::uint16_t>;
    static const Table& get()
    {
        static const Table table = {{
            0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011,
            0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022,
            0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072,
            0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041,
            0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2,
            0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1,
            0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
            0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082,
            0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192,
            0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1,
            0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1,
            0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2,
            0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151,
            0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
            0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132,
            0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101,
            0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312,
            0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321,
            0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371,
            0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342,
            0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
            0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2,
            0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2,
            0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381,
            0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291,
            0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2,
            0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2,
            0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
            0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252,
            0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261,
            0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231,
            0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202
        }};
        return table;
    }
};

template <>
struct CrcInitTable<std::uint32_t, 0x04c11db7>
{
    using Table = CrcInitTableType<std::uint32_t>;
    static const Table& get()
    {
        static const Table table = {{
            0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
            0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
            0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
            0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
            0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
            0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
            0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
            0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
            0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
            0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
            0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
            0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
            0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
            0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
            0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
            0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
            0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
            0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
            0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
            0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
            0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
            0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
            0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
            0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
            0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
            0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
            0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
            0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
            0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
            0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
            0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
            0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
            0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
            0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
            0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
            0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
            0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
            0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
            0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
            0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
            0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
            0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
            0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
            0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
            0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
            0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
            0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
            0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
            0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
            0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
            0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
            0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
            0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
            0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
            0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
            0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
            0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
            0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
            0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
            0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
            0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
            0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
            0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
            0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
        }};
        return table;
    }
};

}  // namespace details

/// @brief Calculate CRC values of all the bytes in the sequence.
/// @tparam TResult Type of the checksum result value.
/// @tparam TPoly Polynomial value
/// @tparam TInit Initial value
/// @tparam TFin Final XOR value
/// @tparam TReflect Perform reflection of every byte
/// @tparam TReflectRem Perform reflection of the final value
/// @headerfile comms/frame/checksum/Crc.h
/// @see Crc_CCITT
/// @see Crc_16
/// @see Crc_32
template <
    typename TResult,
    TResult TPoly,
    TResult TInit = 0,
    TResult TFin = 0,
    bool TReflect = false,
    bool TReflectRem = false
>
class Crc
{
    static_assert(std::is_unsigned<TResult>::value,
        "The TResult type is expected to be unsigned integral one");
public:
    /// @brief Operator that is invoked to calculate the checksum value
    /// @param[in, out] iter Input iterator,
    /// @param[in] len Number of bytes to summarise.
    /// @return The checksum value.
    /// @post The iterator is advanced by number of bytes read (len).
    template <typename TIter>
    TResult operator()(TIter& iter, std::size_t len) const
    {
        static const std::size_t Width =
            sizeof(TResult) * std::numeric_limits<std::uint8_t>::digits;

        TResult rem = TInit;
        auto& initTable = details::CrcInitTable<TResult, TPoly>::get();

        for (std::size_t byte = 0U; byte < len; ++byte)
        {
            using ByteType = typename std::make_unsigned<
                typename std::decay<decltype(*iter)>::type
            >::type;

            auto val = static_cast<std::uint8_t>(static_cast<ByteType>(*iter));
            comms::cast_assign(val) = reflect(val) ^ static_cast<decltype(val)>(rem >> (Width - 8));
            comms::cast_assign(rem) = initTable[val] ^ static_cast<decltype(rem)>(rem << 8);
            ++iter;
        }

        return (reflectRem(rem) ^ TFin);
    }

private:
    template <typename... TParams>
    using NoReflectTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using DoReflectTag = comms::details::tag::Tag2<>;

    template <typename...>
    using ReflectTag = 
        typename comms::util::LazyShallowConditional<
            TReflect
        >::template Type<
            DoReflectTag,
            NoReflectTag
        >;

    template <typename...>
    using ReflectRemTag = 
        typename comms::util::LazyShallowConditional<
            TReflectRem
        >::template Type<
            DoReflectTag,
            NoReflectTag
        >;

    static std::uint8_t reflect(std::uint8_t byte)
    {
        return reflectInternal(byte, 8U, ReflectTag<>());
    }

    static TResult reflectRem(TResult value)
    {
        static const std::size_t Width =
            sizeof(TResult) * std::numeric_limits<std::uint8_t>::digits;

        return reflectInternal(value, Width, ReflectRemTag<>());
    }

    template <typename TVal, typename... TParams>
    static TVal reflectInternal(TVal value, std::size_t bitsCount, DoReflectTag<TParams...>)
    {
        return static_cast<TVal>(doReflect(value, bitsCount));
    }

    template <typename TVal, typename... TParams>
    static constexpr TVal reflectInternal(TVal value, std::size_t, NoReflectTag<TParams...>)
    {
        return value;
    }

    static TResult doReflect(TResult value, std::size_t bitsCount)
    {
        TResult reflection = 0U;
        for (auto bit = 0U; bit < bitsCount; ++bit)
        {
            if (value & 0x01)
            {
                comms::cast_assign(reflection) = 
                    reflection | 
                    static_cast<decltype(reflection)>(1 << ((bitsCount - 1) - bit));
            }

            value = static_cast<decltype(value)>(value >> 1);
        }

        return (reflection);
    }

};

/// @brief Alias to @ref Crc checksum calculator for CRC-CCITT.
/// @details Defines:
///     @li Result type is @b std::uint16_t
///     @li Polynomial is @b 0x1021
///     @li Initial value is @b 0xffff
///     @li Final XOR value is @b 0x0000
///     @li @b NOT using reflection
///     @li @b NOT using reflection for final value
using Crc_CCITT = Crc<std::uint16_t, 0x1021, 0xffff>;

/// @brief Alias to @ref Crc checksum calculator for standard CRC-16.
/// @details Defines:
///     @li Result type is @b std::uint16_t
///     @li Polynomial is @b 0x8005
///     @li Initial value is @b 0x0000
///     @li Final XOR value is @b 0x0000
///     @li @b Using reflection
///     @li @b Using reflection for final value
using Crc_16 = Crc<std::uint16_t, 0x8005, 0, 0, true, true>;

/// @brief Alias to @ref Crc checksum calculator for standard CRC-32.
/// @details Defines:
///     @li Result type is @b std::uint32_t
///     @li Polynomial is @b 0x04c11db7
///     @li Initial value is @b 0xffffffff
///     @li Final XOR value is @b 0xffffffff
///     @li @b Using reflection
///     @li @b Using reflection for final value
using Crc_32 = Crc<std::uint32_t, 0x04c11db7, 0xffffffff, 0xffffffff, true, true>;

}  // namespace checksum

}  // namespace frame

}  // namespace comms



