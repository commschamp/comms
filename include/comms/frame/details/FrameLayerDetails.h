//
// Copyright 2014 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/details/detect.h"

namespace comms
{

namespace frame
{

namespace details
{

template <typename T, bool THasImpl>
struct FrameLayerHasFieldsImplHelper;

template <typename T>
struct FrameLayerHasFieldsImplHelper<T, true>
{
    static const bool Value = T::hasFields();
};

template <typename T>
struct FrameLayerHasFieldsImplHelper<T, false>
{
    static const bool Value = false;
};

template <typename T>
struct FrameLayerHasFieldsImpl
{
    static const bool Value =
        FrameLayerHasFieldsImplHelper<T, comms::details::hasImplOptions<T>()>::Value;
};

template <class T>
constexpr bool frameLayerHasFieldsImpl()
{
    return FrameLayerHasFieldsImpl<T>::Value;
}

template <typename T, bool THasImpl>
struct FrameLayerHasDoGetIdHelper;

template <typename T>
struct FrameLayerHasDoGetIdHelper<T, true>
{
    static const bool Value = T::hasStaticMsgId();
};

template <typename T>
struct FrameLayerHasDoGetIdHelper<T, false>
{
    static const bool Value = false;
};

template <typename T>
struct FrameLayerHasDoGetId
{
    static const bool Value =
        FrameLayerHasDoGetIdHelper<T, comms::details::hasImplOptions<T>()>::Value;
};

template <typename T>
constexpr bool frameLayerHasDoGetId()
{
    return FrameLayerHasDoGetId<T>::Value;
}

template <class T, class R = void>
struct FrameLayerEnableIfHasMsgPtr { using Type = R; };

template <class T, class Enable = void>
struct FrameLayerMsgPtr
{
    using Type = void;
};

template <class T>
struct FrameLayerMsgPtr<T, typename FrameLayerEnableIfHasMsgPtr<typename T::MsgPtr>::Type>
{
    using Type = typename T::MsgPtr;
};

template<typename...>
class MissingSizeRetriever
{
public:
    MissingSizeRetriever(std::size_t& val) : value_(val) {}

    void setValue(std::size_t val)
    {
        value_ = val;
    }

private:
    std::size_t& value_;
};

template <typename T>
struct IsMissingSizeRetrieverHelper
{
    static const bool Value = false;
};

template <typename... TParams>
struct IsMissingSizeRetrieverHelper<MissingSizeRetriever<TParams...> >
{
    static const bool Value = true;
};

template <typename T>
constexpr bool isMissingSizeRetriever()
{
    return IsMissingSizeRetrieverHelper<T>::Value;
}

template <typename TId>
class MsgIdRetriever
{
public:
    MsgIdRetriever(TId& val) : value_(val) {}

    template <typename U>
    void setValue(U&& val)
    {
        value_ = static_cast<TId>(val);
    }

private:
    TId& value_;
};

template <typename T>
struct IsMsgIdRetrieverHelper
{
    static const bool Value = false;
};

template <typename TId>
struct IsMsgIdRetrieverHelper<MsgIdRetriever<TId> >
{
    static const bool Value = true;
};

template <typename T>
constexpr bool isMsgIdRetriever()
{
    return IsMsgIdRetrieverHelper<T>::Value;
}

class MsgIndexRetriever
{
public:
    MsgIndexRetriever(std::size_t& val) : value_(val) {}

    void setValue(std::size_t val)
    {
        value_ = val;
    }

private:
    std::size_t& value_;
};

template <typename T>
struct IsMsgIndexRetrieverHelper
{
    static const bool Value = false;
};

template <>
struct IsMsgIndexRetrieverHelper<MsgIndexRetriever>
{
    static const bool Value = true;
};

template <typename T>
constexpr bool isMsgIndexRetriever()
{
    return IsMsgIndexRetrieverHelper<T>::Value;
}

template <typename TIter>
class MsgPayloadRetriever
{
public:
    MsgPayloadRetriever(TIter& iter, std::size_t& len) : iter_(iter), len_(len) {}

    template <typename TOtherIter>
    void setValue(TOtherIter iter, std::size_t len)
    {
        iter_ = static_cast<TIter>(iter);
        len_ = len;
    }

private:
    TIter& iter_;
    std::size_t& len_;
};

template <typename TITer>
struct IsMsgPayloadRetrieverHelper
{
    static const bool Value = false;
};

template <typename TIter>
struct IsMsgPayloadRetrieverHelper<MsgPayloadRetriever<TIter> >
{
    static const bool Value = true;
};

template <typename T>
constexpr bool isMsgPayloadRetriever()
{
    return IsMsgPayloadRetrieverHelper<T>::Value;
}

} // namespace details

} // namespace frame

} // namespace comms
