//
// Copyright 2014 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace comms
{

namespace frame
{

namespace details
{

template <bool TValidPtr>
struct MsgSizeLayerConstNullPtrCastHelper
{
    template <typename TPtr>
    using Type = const typename TPtr::element_type*;
};

template <>
struct MsgSizeLayerConstNullPtrCastHelper<false>
{
    template <typename TPtr>
    using Type = const void*;
};

} // namespace details

} // namespace frame

} // namespace comms
