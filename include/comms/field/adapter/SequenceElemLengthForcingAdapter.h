//
// Copyright 2017 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"
#include "comms/field/basic/BasicCommonFuncs.h"

#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TBase>
class SequenceElemLengthForcingAdapter : public TBase
{
    using BaseImpl = TBase;
public:
    using ValueType = typename BaseImpl::ValueType;
    using ElementType = typename BaseImpl::ElementType;

    SequenceElemLengthForcingAdapter() = default;

    explicit SequenceElemLengthForcingAdapter(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit SequenceElemLengthForcingAdapter(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    SequenceElemLengthForcingAdapter(const SequenceElemLengthForcingAdapter&) = default;
    SequenceElemLengthForcingAdapter(SequenceElemLengthForcingAdapter&&) = default;
    SequenceElemLengthForcingAdapter& operator=(const SequenceElemLengthForcingAdapter&) = default;
    SequenceElemLengthForcingAdapter& operator=(SequenceElemLengthForcingAdapter&&) = default;

    void forceReadElemLength(std::size_t val)
    {
        COMMS_ASSERT(val != Cleared);
        m_forced = val;
    }

    void clearReadElemLengthForcing()
    {
        m_forced = Cleared;
    }

    std::size_t length() const
    {
        if (m_forced != Cleared) {
            return BaseImpl::getValue().size() * m_forced;
        }

        return BaseImpl::length();
    }

    std::size_t elementLength(const ElementType& elem) const
    {
        if (m_forced != Cleared) {
            return m_forced;
        }
        return BaseImpl::elementLength(elem);
    }

    static constexpr std::size_t maxElementLength()
    {
        return basic::BasicCommonFuncs::maxSupportedLength();
    }

    template <typename TIter>
    ErrorStatus readElement(ElementType& elem, TIter& iter, std::size_t& len) const
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using IterTag = typename std::iterator_traits<IterType>::iterator_category;
        static_assert(std::is_base_of<std::random_access_iterator_tag, IterTag>::value,
            "Only random access iterator for reading is supported with comms::option::def::SequenceElemLengthForcingEnabled option");

        if (m_forced == Cleared) {
            return BaseImpl::readElement(elem, iter, len);
        }

        if (len < m_forced) {
            return comms::ErrorStatus::NotEnoughData;
        }

        auto iterTmp = iter;
        auto remLen = m_forced;
        std::advance(iter, m_forced);
        len -= m_forced;
        return BaseImpl::readElement(elem, iterTmp, remLen);
    }

    template <typename TIter>
    void readElementNoStatus(ElementType& elem, TIter& iter) const
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using IterTag = typename std::iterator_traits<IterType>::iterator_category;
        static_assert(std::is_base_of<std::random_access_iterator_tag, IterTag>::value,
            "Only random access iterator for reading is supported with comms::option::def::SequenceElemLengthForcingEnabled option");

        if (m_forced == Cleared) {
            return BaseImpl::readElementNoStatus(elem, iter);
        }

        auto fromIter = iter;
        auto es = BaseImpl::readElementNoStatus(elem, iter);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        auto consumed = std::distance(fromIter, iter);
        if (consumed < m_forced) {
            std::advance(iter, m_forced - consumed);
        }
    }

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        return basic::BasicCommonFuncs::readSequence(*this, iter, len);
    }

    static constexpr bool hasReadNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;

    template <typename TIter>
    ErrorStatus readN(std::size_t count, TIter& iter, std::size_t& len)
    {
        return basic::BasicCommonFuncs::readSequenceN(*this, count, iter, len);
    }

    template <typename TIter>
    void readNoStatusN(std::size_t count, TIter& iter) = delete;
//    {
//        basic::BasicCommonFuncs::readSequenceNoStatusN(*this, count, iter);
//    }

private:

    static const std::size_t Cleared = std::numeric_limits<std::size_t>::max();
    std::size_t m_forced = Cleared;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

