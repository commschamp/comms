//
// Copyright 2019 - 2021 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <utility>

#include "comms/cast.h"

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

    auto getValueAdapted() const -> decltype(std::declval<TActField>().getValue())
    {
        return static_cast<const TActField*>(this)->getValue();
    }

    template <typename T>
    void setValueAdapted(T&& val)
    {
        static_cast<TActField*>(this)->setValue(std::forward<T>(val));
    }
};

}  // namespace adapter

}  // namespace field

}  // namespace comms




