//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace comms
{

namespace field
{

namespace details
{

template <typename TVersionType>
struct VersionStorage
{
    TVersionType getVersion() const
    {
        return m_version;
    }
    
protected:
    TVersionType m_version = TVersionType();
};

} // namespace details

} // namespace field

} // namespace comms
