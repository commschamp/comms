//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once


#include "comms/CompileControl.h"
#include "comms/ErrorStatus.h"

#include <cstdint>
#include <limits>
#include <type_traits>

COMMS_MSVC_WARNING_PUSH
COMMS_MSVC_WARNING_DISABLE(4100) // Disable warning about unreferenced parameters

namespace comms
{

namespace field
{

namespace details
{

template <typename...>
struct FieldVersionDependentCheckHelper
{
    template <typename TField>
    constexpr bool operator()() const
    {
        return TField::isVersionDependent();
    }

    template <typename TField>
    constexpr bool operator()(bool soFar) const
    {
        return TField::isVersionDependent() || soFar;
    }    
};

template <typename...>
struct FieldMaxLengthCalcHelper
{
    template <typename TField>
    constexpr std::size_t operator()(std::size_t val) const
    {
        return val >= TField::maxLength() ? val : TField::maxLength();
    }
};

template <typename...>
struct FieldMinLengthSumCalcHelper
{
    template <typename TField>
    constexpr std::size_t operator()(std::size_t sum) const
    {
        return sum + TField::minLength();
    }
};

template <typename...>
struct FieldMaxLengthSumCalcHelper
{
    template <typename TField>
    constexpr std::size_t operator()(std::size_t sum) const
    {
        return sum + TField::maxLength();
    }
};

template <bool THasBitLengthLimit>
struct FieldBitLengthRetrieveHelper
{
    template <typename TField>
    using Type = 
        std::integral_constant<
            std::size_t,
            TField::ParsedOptions::FixedBitLength
        >;
};

template <>
struct FieldBitLengthRetrieveHelper<false>
{
    template <typename TField>
    using Type = 
        std::integral_constant<
            std::size_t,
            TField::maxLength() * std::numeric_limits<std::uint8_t>::digits
        >;
};


template <typename...>
struct FieldTotalBitLengthSumCalcHelper
{
    template <typename TField>
    constexpr std::size_t operator()(std::size_t sum) const
    {
        return sum + FieldBitLengthRetrieveHelper<TField::ParsedOptions::HasFixedBitLengthLimit>::template Type<TField>::value;
    }
};

template <typename...>
struct FieldLengthSumCalcHelper
{
    template <typename TField>
    constexpr std::size_t operator()(std::size_t sum, const TField& field) const
    {
        return sum + field.length();
    }
};

template <typename...>
struct FieldHasWriteNoStatusHelper
{
    constexpr FieldHasWriteNoStatusHelper() = default;

    template <typename TField>
    constexpr bool operator()(bool soFar) const
    {
        return TField::hasWriteNoStatus() && soFar;
    }
};

template<typename...>
struct FieldNonDefaultRefreshCheckHelper
{
    template <typename TField>
    constexpr bool operator()() const
    {
        return TField::hasNonDefaultRefresh();
    }

    template <typename TField>
    constexpr bool operator()(bool soFar) const
    {
        return TField::hasNonDefaultRefresh() || soFar;
    }
};

template<typename...>
struct FieldValidCheckHelper
{
    template <typename TField>
    constexpr bool operator()(bool soFar, const TField& field) const
    {
        return soFar && field.valid();
    }
};

template <typename...>
struct FieldRefreshHelper
{
    template <typename TField>
    bool operator()(bool soFar, TField& field) const
    {
        return field.refresh() || soFar;
    }
};

template <typename TIter>
class FieldReadHelper
{
public:
    FieldReadHelper(ErrorStatus& es, TIter& iter, std::size_t& len) :
        m_es(es),
        m_iter(iter),
        m_len(len)
    {
    }

    template <typename TField>
    void operator()(TField& field)
    {
        if (m_es != comms::ErrorStatus::Success) {
            return;
        }

        auto fromIter = m_iter;
        m_es = field.read(m_iter, m_len);
        if (m_es == comms::ErrorStatus::Success) {
            m_len -= static_cast<std::size_t>(std::distance(fromIter, m_iter));
        }
    }


private:
    ErrorStatus& m_es;
    TIter& m_iter;
    std::size_t& m_len;
};

template <typename TIter>
class FieldReadNoStatusHelper
{
public:
    FieldReadNoStatusHelper(TIter& iter)
      : m_iter(iter)
    {
    }

    template <typename TField>
    void operator()(TField& field)
    {
        field.readNoStatus(m_iter);
    }

private:
    TIter& m_iter;
};

template <typename TIter>
class FieldWriteHelper
{
public:
    FieldWriteHelper(ErrorStatus& es, TIter& iter, std::size_t len)
      : m_es(es),
        m_iter(iter),
        m_len(len)
    {
    }

    template <typename TField>
    void operator()(const TField& field)
    {
        if (m_es != comms::ErrorStatus::Success) {
            return;
        }

        m_es = field.write(m_iter, m_len);
        if (m_es == comms::ErrorStatus::Success) {
            m_len -= field.length();
        }
    }

private:
    ErrorStatus& m_es;
    TIter& m_iter;
    std::size_t m_len;
};

template <typename TIter>
class FieldWriteNoStatusHelper
{
public:
    FieldWriteNoStatusHelper(TIter& iter)
      : m_iter(iter)
    {
    }

    template <typename TField>
    void operator()(const TField& field)
    {
        field.writeNoStatus(m_iter);
    }

private:
    TIter& m_iter;
};

template <typename...>
struct FieldReadNoStatusDetectHelper
{
    template <typename TField>
    constexpr bool operator()() const
    {
        return TField::hasReadNoStatus();
    }    

    template <typename TField>
    constexpr bool operator()(bool soFar) const
    {
        return TField::hasReadNoStatus() && soFar;
    }
};

template<typename...>
struct FieldWriteNoStatusDetectHelper
{
    template <typename TField>
    constexpr bool operator()() const
    {
        return TField::hasWriteNoStatus();
    }

    template <typename TField>
    constexpr bool operator()(bool soFar) const
    {
        return TField::hasWriteNoStatus() && soFar;
    }
};

template <typename...>
struct FieldCanWriteCheckHelper
{
    template <typename TField>
    constexpr bool operator()(bool soFar, const TField& field) const
    {
        return soFar && field.canWrite();
    }
};

template <typename TVersionType>
struct FieldVersionUpdateHelper
{
    FieldVersionUpdateHelper(TVersionType version) : m_version(version) {}

    template <typename TField>
    bool operator()(bool updated, TField& field) const
    {
        using FieldVersionType = typename std::decay<decltype(field)>::type::VersionType;
        return field.setVersion(static_cast<FieldVersionType>(m_version)) || updated;
    }

private:
    const TVersionType m_version = static_cast<TVersionType>(0);
};
    
} // namespace details

} // namespace field

} // namespace comms

COMMS_MSVC_WARNING_POP