//
// Copyright 2017 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <utility>

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TBase>
class VersionStorageAdapter : public TBase
{
    using BaseImpl = TBase;
public:

    using ValueType = typename BaseImpl::ValueType;
    using VersionType = typename BaseImpl::VersionType;

    VersionStorageAdapter() = default;

    explicit VersionStorageAdapter(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit VersionStorageAdapter(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    VersionStorageAdapter(const VersionStorageAdapter&) = default;
    VersionStorageAdapter(VersionStorageAdapter&&) = default;
    VersionStorageAdapter& operator=(const VersionStorageAdapter&) = default;
    VersionStorageAdapter& operator=(VersionStorageAdapter&&) = default;

    static constexpr bool isVersionDependent()
    {
        return true;
    }

    VersionType getVersion() const
    {
        return m_version;
    }

    bool setVersion(VersionType val)
    {
        bool updated = BaseImpl::setVersion(val);
        if (m_version != val) {
            m_version = val;
            return true;
        }
        return updated;
    }

private:
    VersionType m_version = static_cast<VersionType>(0);
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

