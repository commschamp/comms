//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>
#include <utility>

#include "comms/cast.h"
#include "comms/util/ScopeGuard.h"

namespace comms
{

namespace field
{

namespace adapter
{


template <typename TActField, typename TBase>
class FieldType : public TBase
{
    using BaseImpl = TBase;
public:
    using ValueType = typename BaseImpl::ValueType;

    bool valid() const
    {
        if (m_entered) {
            return BaseImpl::valid();
        }

        m_entered = true;
        auto onExit =
            comms::util::makeScopeGuard(
                [this]()
                {
                    m_entered = false;
                });
                
        return static_cast<const TActField*>(this)->valid();
    }

private:
    mutable bool m_entered = false;    
};

}  // namespace adapter

}  // namespace field

}  // namespace comms




