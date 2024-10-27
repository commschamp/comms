//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once


namespace comms
{

namespace field
{

namespace adapter
{


template <typename TBase>
class FixedValue : public TBase
{
    using BaseImpl = TBase;
public:
    using ValueType = typename BaseImpl::ValueType;

    using BaseImpl::value;

    ValueType& value() = delete;

    template <typename U>
    void setValue(U&& val) = delete;    

    static constexpr bool fixedValue()
    {
        return true;
    }        

private:
    using BaseImpl::setValue;

};

}  // namespace adapter

}  // namespace field

}  // namespace comms




