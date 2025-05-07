//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"

#include <algorithm>
#include <limits>

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TBase>
class SequenceLengthForcing : public TBase
{
    using BaseImpl = TBase;
public:
    using ValueType = typename BaseImpl::ValueType;
    using ElementType = typename BaseImpl::ElementType;

    SequenceLengthForcing() = default;

    explicit SequenceLengthForcing(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit SequenceLengthForcing(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    SequenceLengthForcing(const SequenceLengthForcing&) = default;
    SequenceLengthForcing(SequenceLengthForcing&&) = default;
    SequenceLengthForcing& operator=(const SequenceLengthForcing&) = default;
    SequenceLengthForcing& operator=(SequenceLengthForcing&&) = default;

    void forceReadLength(std::size_t val)
    {
        COMMS_ASSERT(val != Cleared);
        m_forced = val;
    }

    void clearReadLengthForcing()
    {
        m_forced = Cleared;
    }

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        if (m_forced == Cleared) {
            return BaseImpl::read(iter, len);
        }

        if (len < m_forced) {
            return comms::ErrorStatus::NotEnoughData;
        }

        return BaseImpl::read(iter, m_forced);
    }

    template <typename TIter>
    ErrorStatus readN(std::size_t count, TIter& iter, std::size_t& len)
    {
        if (m_forced == Cleared) {
            return BaseImpl::read(iter, len);
        }

        if (len < m_forced) {
            return comms::ErrorStatus::NotEnoughData;
        }

        return BaseImpl::readN(count, iter, m_forced);
    }

    static constexpr bool hasReadNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;

    template <typename TIter>
    void readNoStatusN(std::size_t count, TIter& iter) = delete;


private:
    static const std::size_t Cleared = std::numeric_limits<std::size_t>::max();
    std::size_t m_forced = Cleared;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms


