//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TTrailField, typename TBase>
class SequenceTrailingFieldSuffix : public TBase
{
    using BaseImpl = TBase;
    using TrailField = TTrailField;

    static_assert(!TrailField::isVersionDependent(),
            "Suffix fields must not be version dependent");

public:
    using ValueType = typename BaseImpl::ValueType;
    using ElementType = typename BaseImpl::ElementType;

    SequenceTrailingFieldSuffix() = default;

    explicit SequenceTrailingFieldSuffix(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit SequenceTrailingFieldSuffix(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    SequenceTrailingFieldSuffix(const SequenceTrailingFieldSuffix&) = default;
    SequenceTrailingFieldSuffix(SequenceTrailingFieldSuffix&&) = default;
    SequenceTrailingFieldSuffix& operator=(const SequenceTrailingFieldSuffix&) = default;
    SequenceTrailingFieldSuffix& operator=(SequenceTrailingFieldSuffix&&) = default;

    constexpr std::size_t length() const
    {
        return m_trailField.length() + BaseImpl::length();
    }

    static constexpr std::size_t minLength()
    {
        return TrailField::minLength() + BaseImpl::minLength();
    }

    static constexpr std::size_t maxLength()
    {
        return TrailField::maxLength() + BaseImpl::maxLength();
    }

    bool valid() const
    {
        return m_trailField.valid() && BaseImpl::valid();
    }

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        auto es = BaseImpl::read(iter, len - TrailField::minLength());
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        return m_trailField.read(iter, len - BaseImpl::length());
    }

    static constexpr bool hasReadNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;

    template <typename TIter>
    comms::ErrorStatus write(TIter& iter, std::size_t len) const
    {
        auto trailLen = m_trailField.length();
        auto es = BaseImpl::write(iter, len - trailLen);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        return m_trailField.write(iter, trailLen);
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) const
    {
        BaseImpl::writeNoStatus(iter);
        m_trailField.writeNoStatus(iter);
    }

private:
    TrailField m_trailField;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms




