//
// Copyright 2023 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <limits>
#include <type_traits>

#include "comms/details/tag.h"
#include "comms/util/type_traits.h"
#include "comms/util/detect.h"

namespace comms
{

namespace util
{

namespace details
{

class MaxSizeOfHelper
{
public:
    template <typename T>
    static std::size_t maxSizeOf(const T& val)
    {
        using DecayedType = typename std::decay<decltype(val)>::type;
        using Tag = 
            typename comms::util::LazyShallowConditional<
                comms::util::detect::hasMaxSizeFunc<DecayedType>()
            >::template Type<
                HasMaxSizeTag,
                NoMaxSizeTag
            >;

        return maxSizeOfInternal(val, Tag());
    }

private:
    template <typename... TParams>
    using HasMaxSizeTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using NoMaxSizeTag = comms::details::tag::Tag2<>;    

    template <typename T, typename... TParams>
    static std::size_t maxSizeOfInternal(const T& val, HasMaxSizeTag<>)
    {
        return val.max_size();
    }    

    template <typename T, typename... TParams>
    static std::size_t maxSizeOfInternal(const T& val, NoMaxSizeTag<>)
    {
        static_cast<void>(val);
        return std::numeric_limits<std::size_t>::max();
    }       
};

} // namespace details

/// @cond SKIP_DOC
template <typename T>
std::size_t maxSizeOf(const T& val)
{
    return details::MaxSizeOfHelper::maxSizeOf(val);
}

/// @endcond

}  // namespace util

}  // namespace comms


