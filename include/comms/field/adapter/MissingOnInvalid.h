//
// Copyright 2019 - 2023 (C). Alex Robenko. All rights reserved.
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

template <typename TBase>
class MissingOnInvalid : public TBase
{
    using BaseImpl = TBase;
public:

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        auto iterTmp = iter;
        auto es = BaseImpl::read(iterTmp, len);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        if (!BaseImpl::valid()) {
            BaseImpl::setMode(comms::field::OptionalMode::Missing);
            return es;
        }

        iter = iterTmp;
        return es;
    }

    static constexpr bool hasReadNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;

    static constexpr bool hasNonDefaultRefresh()
    {
        return true;
    }

    bool refresh()
    {
        bool updated = BaseImpl::refresh();
        auto mode = BaseImpl::getMode();
        if (!BaseImpl::valid()) {
            mode = comms::field::OptionalMode::Missing;
        }

        if (mode == BaseImpl::getMode()) {
            return updated;
        }

        BaseImpl::setMode(mode);
        return true;
    }    
};

}  // namespace adapter

}  // namespace field

}  // namespace comms




