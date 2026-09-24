//
// Copyright 2015 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"

#include <cstddef>
#include <limits>
#include <utility>

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TBase>
class SequenceSizeForcingAdapter : public TBase
{
    using BaseImpl = TBase;
public:
    using ValueType = typename BaseImpl::ValueType;
    using ElementType = typename BaseImpl::ElementType;

    SequenceSizeForcingAdapter() = default;

    explicit SequenceSizeForcingAdapter(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit SequenceSizeForcingAdapter(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    SequenceSizeForcingAdapter(const SequenceSizeForcingAdapter&) = default;
    SequenceSizeForcingAdapter(SequenceSizeForcingAdapter&&) = default;
    SequenceSizeForcingAdapter& operator=(const SequenceSizeForcingAdapter&) = default;
    SequenceSizeForcingAdapter& operator=(SequenceSizeForcingAdapter&&) = default;

    void forceReadElemCount(std::size_t val)
    {
        COMMS_ASSERT(val != Cleared);
        m_forced = val;
    }

    void clearReadElemCount()
    {
        m_forced = Cleared;
    }

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        if (m_forced == Cleared) {
            return BaseImpl::read(iter, len);
        }

        return BaseImpl::readN(m_forced, iter, len);
    }

    template <typename TIter>
    ErrorStatus readN(std::size_t count, TIter& iter, std::size_t& len) = delete;

    template <typename TIter>
    void readNoStatus(TIter& iter)
    {
        if (m_forced == Cleared) {
            BaseImpl::readNoStatus(iter);
            return;
        }

        BaseImpl::readNoStatusN(m_forced, iter);
    }

    template <typename TIter>
    void readNoStatusN(std::size_t count, TIter& iter) = delete;

private:
    static const std::size_t Cleared = std::numeric_limits<std::size_t>::max();
    std::size_t m_forced = Cleared;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

