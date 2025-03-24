//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"
#include "comms/field/basic/CommonFuncs.h"

#include <iterator>
#include <limits>

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TBase>
class SequenceElemLengthForcing : public TBase
{
    using BaseImpl = TBase;
public:
    using ValueType = typename BaseImpl::ValueType;
    using ElementType = typename BaseImpl::ElementType;

    SequenceElemLengthForcing() = default;

    explicit SequenceElemLengthForcing(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit SequenceElemLengthForcing(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    SequenceElemLengthForcing(const SequenceElemLengthForcing&) = default;
    SequenceElemLengthForcing(SequenceElemLengthForcing&&) = default;
    SequenceElemLengthForcing& operator=(const SequenceElemLengthForcing&) = default;
    SequenceElemLengthForcing& operator=(SequenceElemLengthForcing&&) = default;

    void forceReadElemLength(std::size_t val)
    {
        COMMS_ASSERT(val != Cleared);
        forced_ = val;
    }

    void clearReadElemLengthForcing()
    {
        forced_ = Cleared;
    }

    std::size_t length() const
    {
        if (forced_ != Cleared) {
            return BaseImpl::getValue().size() * forced_;
        }

        return BaseImpl::length();
    }

    std::size_t elementLength(const ElementType& elem) const
    {
        if (forced_ != Cleared) {
            return forced_;
        }
        return BaseImpl::elementLength(elem);
    }

    static constexpr std::size_t maxElementLength()
    {
        return basic::CommonFuncs::maxSupportedLength();
    }

    template <typename TIter>
    ErrorStatus readElement(ElementType& elem, TIter& iter, std::size_t& len) const
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using IterTag = typename std::iterator_traits<IterType>::iterator_category;
        static_assert(std::is_base_of<std::random_access_iterator_tag, IterTag>::value,
            "Only random access iterator for reading is supported with comms::option::def::SequenceElemLengthForcingEnabled option");

        if (forced_ == Cleared) {
            return BaseImpl::readElement(elem, iter, len);
        }

        if (len < forced_) {
            return comms::ErrorStatus::NotEnoughData;
        }

        auto iterTmp = iter;
        auto remLen = forced_;
        std::advance(iter, forced_);
        len -= forced_;
        return BaseImpl::readElement(elem, iterTmp, remLen);
    }

    template <typename TIter>
    void readElementNoStatus(ElementType& elem, TIter& iter) const
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using IterTag = typename std::iterator_traits<IterType>::iterator_category;
        static_assert(std::is_base_of<std::random_access_iterator_tag, IterTag>::value,
            "Only random access iterator for reading is supported with comms::option::def::SequenceElemLengthForcingEnabled option");

        if (forced_ == Cleared) {
            return BaseImpl::readElementNoStatus(elem, iter);
        }

        auto fromIter = iter;
        auto es = BaseImpl::readElementNoStatus(elem, iter);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        auto consumed = std::distance(fromIter, iter);
        if (consumed < forced_) {
            std::advance(iter, forced_ - consumed);
        }
    }

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        return basic::CommonFuncs::readSequence(*this, iter, len);
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
        return basic::CommonFuncs::readSequenceN(*this, count, iter, len);
    }

    template <typename TIter>
    void readNoStatusN(std::size_t count, TIter& iter) = delete;
//    {
//        basic::CommonFuncs::readSequenceNoStatusN(*this, count, iter);
//    }

private:

    static const std::size_t Cleared = std::numeric_limits<std::size_t>::max();
    std::size_t forced_ = Cleared;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms


