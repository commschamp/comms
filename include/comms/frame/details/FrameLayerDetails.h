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
    MissingSizeRetriever(std::size_t& val) : m_value(val) {}

    void setValue(std::size_t val)
    {
        m_value = val;
    }

private:
    std::size_t& m_value;
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
    MsgIdRetriever(TId& val) : m_value(val) {}

    template <typename U>
    void setValue(U&& val)
    {
        m_value = static_cast<TId>(val);
    }

private:
    TId& m_value;
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
    MsgIndexRetriever(std::size_t& val) : m_value(val) {}

    void setValue(std::size_t val)
    {
        m_value = val;
    }

private:
    std::size_t& m_value;
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

template <typename... TRetrievers>
struct MsgIndexRetrieverDetector;

template <typename T, typename... TRetrievers>
struct MsgIndexRetrieverDetector<T, TRetrievers...>
{
    static const bool Detected = isMsgIndexRetriever<typename std::decay<T>::type>() || MsgIndexRetrieverDetector<TRetrievers...>::Detected;
};

template <typename... TRetrievers>
constexpr bool hasMsgIndexRetriever()
{
    return MsgIndexRetrieverDetector<TRetrievers...>::Detected;
}

template <>
struct MsgIndexRetrieverDetector<>
{
    static const bool Detected = false;
};

template <typename TIter>
class MsgPayloadRetriever
{
public:
    MsgPayloadRetriever(TIter& iter, std::size_t& len) : m_iter(iter), m_len(len) {}

    template <typename TOtherIter>
    void setValue(TOtherIter iter, std::size_t len)
    {
        m_iter = static_cast<TIter>(iter);
        m_len = len;
    }

private:
    TIter& m_iter;
    std::size_t& m_len;
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
