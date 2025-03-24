//
// Copyright 2019 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/cast.h"

#include <utility>

namespace comms
{

namespace field
{

namespace adapter
{


template <typename TBase>
class MissingOnReadFail : public TBase
{
    using BaseImpl = TBase;
public:

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        BaseImpl tmp;
        auto iterTmp = iter;
        auto es = tmp.read(iterTmp, len);
        if (es != comms::ErrorStatus::Success) {
            BaseImpl::setMode(comms::field::OptionalMode::Missing);
            return comms::ErrorStatus::Success;
        }

        static_cast<BaseImpl&>(*this) = std::move(tmp);
        iter = iterTmp;
        return es;
    }

    static constexpr bool hasReadNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms




