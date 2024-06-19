//
// Copyright 2019 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <limits>
#include <type_traits>
#include <iterator>

#include "comms/ErrorStatus.h"
#include "comms/field/tag.h"
#include "comms/util/type_traits.h"
#include "comms/details/tag.h"

namespace comms
{

namespace field
{

namespace adapter
{

template <std::size_t TLenFieldIdx, typename TBase>
class RemLengthMemberField : public TBase
{
    using BaseImpl = TBase;
public:
    using ValueType = typename BaseImpl::ValueType;

    static_assert(TLenFieldIdx < std::tuple_size<ValueType>::value, "Bad index");
    using LengthFieldType = typename std::tuple_element<TLenFieldIdx, ValueType>::type;
    using VersionType = typename BaseImpl::VersionType;

    RemLengthMemberField()
    {
        refreshLengthInternal();
    }

    static constexpr std::size_t maxLength()
    {
        return MaxPossibleLen;
    }

    template <std::size_t TFromIdx>
    static constexpr std::size_t maxLengthFrom()
    {
        using Tag = 
            typename comms::util::LazyShallowConditional<
                TLenFieldIdx < TFromIdx
            >::template Type<
                BaseRedirectTag,
                LocalTag
            >;
        return maxLengthFromInternal<TFromIdx>(Tag());
    }

    template <std::size_t TUntilIdx>
    static constexpr std::size_t maxLengthUntil()
    {
        using Tag = 
            typename comms::util::LazyShallowConditional<
                TUntilIdx <= TLenFieldIdx
            >::template Type<
                BaseRedirectTag,
                LocalTag
            >;

        return maxLengthUntilInternal<TUntilIdx>(Tag());
    }

    template <std::size_t TFromIdx, std::size_t TUntilIdx>
    static constexpr std::size_t maxLengthFromUntil()
    {
        using Tag = 
            typename comms::util::LazyShallowConditional<
                (TUntilIdx <= TLenFieldIdx) || (TLenFieldIdx < TFromIdx)
            >::template Type<
                BaseRedirectTag,
                LocalTag
            >;

        return maxLengthFromUntilInternal<TFromIdx, TUntilIdx>(Tag());
    }

    bool refresh()
    {
        bool updated = BaseImpl::refresh();
        return refreshLengthInternal() || updated;
    }

    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t len)
    {
        return readFromUntilAndUpdateLen<0, std::tuple_size<ValueType>::value>(iter, len);
    }

    template <std::size_t TFromIdx, typename TIter>
    ErrorStatus readFrom(TIter& iter, std::size_t len)
    {
        return readFromUntilAndUpdateLen<TFromIdx, std::tuple_size<ValueType>::value>(iter, len);
    }    

    template <std::size_t TFromIdx, typename TIter>
    ErrorStatus readFromAndUpdateLen(TIter& iter, std::size_t& len)
    {
        return readFromAndUpdateLen<TFromIdx, std::tuple_size<ValueType>::value>(iter, len);
    }       

    template <std::size_t TUntilIdx, typename TIter>
    ErrorStatus readUntil(TIter& iter, std::size_t len)
    {
        return readFromUntilAndUpdateLen<0U, TUntilIdx>(iter, len);
    }

    template <std::size_t TUntilIdx, typename TIter>
    ErrorStatus readUntilAndUpdateLen(TIter& iter, std::size_t& len)
    {
        return readFromUntilAndUpdateLen<0, TUntilIdx>(iter, len);
    }   

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus readFromUntil(TIter& iter, std::size_t len)
    {
        return readFromUntilAndUpdateLen<TFromIdx, TUntilIdx>(iter, len);
    }

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus readFromUntilAndUpdateLen(TIter& iter, std::size_t& len)
    {
        using Tag = 
            typename comms::util::LazyShallowConditional<
                (TUntilIdx <= TLenFieldIdx)
            >::template Type<
                BaseRedirectTag,
                LocalTag
            >;      
        return readFromUntilInternal<TFromIdx, TUntilIdx>(iter, len, Tag());
    }      

    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;

    template <std::size_t TFromIdx, typename TIter>
    void readFromNoStatus(TIter& iter) = delete;

    template <std::size_t TUntilIdx, typename TIter>
    void readUntilNoStatus(TIter& iter) = delete;

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    void readFromUntilNoStatus(TIter& iter) = delete;

    static constexpr bool hasNonDefaultRefresh()
    {
        return true;
    }

    bool setVersion(VersionType version)
    {
        bool updated = BaseImpl::setVersion(version);
        return refreshLengthInternal() || updated;
    }

    bool canWrite() const
    {
        if (!BaseImpl::canWrite()) {
            return false;
        }

        std::size_t expLen = BaseImpl::template lengthFrom<TLenFieldIdx + 1>();
        if (static_cast<std::size_t>(LengthFieldType::maxValue()) < expLen) {
            return false;
        }

        LengthFieldType lenField;
        lenField.setValue(expLen);
        return lenField.canWrite();
    }

    template <typename TIter>
    comms::ErrorStatus write(TIter& iter, std::size_t len) const
    {
        if (!canWrite()) {
            return comms::ErrorStatus::InvalidMsgData;
        }

        return BaseImpl::write(iter, len);
    }

    static constexpr bool hasWriteNoStatus()
    {
        return false;
    }

    template <typename TIter>
    comms::ErrorStatus writeNoStatus(TIter& iter, std::size_t len) const = delete;

    bool valid() const
    {
        return BaseImpl::valid() && canWrite();
    }

private:
    template <typename... TParams>
    using BaseRedirectTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using LocalTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    using PerformOpTag = comms::details::tag::Tag3<>;    

    template <typename... TParams>
    using SkipOpTag = comms::details::tag::Tag4<>;    

    template <std::size_t TFromIdx, typename... TParams>
    static constexpr std::size_t maxLengthFromInternal(BaseRedirectTag<TParams...>)
    {
        return BaseImpl::template maxLengthFrom<TFromIdx>();
    }

    template <std::size_t TFromIdx, typename... TParams>
    static constexpr std::size_t maxLengthFromInternal(LocalTag<TParams...>)
    {
        return MaxPossibleLen;
    }

    template <std::size_t TUntilIdx, typename... TParams>
    static constexpr std::size_t maxLengthUntilInternal(BaseRedirectTag<TParams...>)
    {
        return BaseImpl::template maxLengthUntil<TUntilIdx>();
    }    

    template <std::size_t TUntilIdx, typename... TParams>
    static constexpr std::size_t maxLengthUntilInternal(LocalTag<TParams...>)
    {
        return MaxPossibleLen;
    }

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename... TParams>
    static constexpr std::size_t maxLengthFromUntilInternal(BaseRedirectTag<TParams...>)
    {
        return BaseImpl::template maxLengthFromUntil<TFromIdx, TUntilIdx>();
    }    

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename... TParams>
    static constexpr std::size_t maxLengthFromUntilInternal(LocalTag<TParams...>)
    {
        return MaxPossibleLen;
    }       

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter, typename... TParams>
    ErrorStatus readFromUntilInternal(TIter& iter, std::size_t& len, BaseRedirectTag<TParams...>)
    {
        return BaseImpl::template readFromUntilAndUpdateLen<TFromIdx, TUntilIdx>(iter, len);
    }       

    template <std::size_t TFromIdx, typename TIter, typename... TParams>
    ErrorStatus readEarlierFieldsInternal(TIter& iter, std::size_t& len, PerformOpTag<TParams...>)
    {
        return BaseImpl::template readFromUntilAndUpdateLen<TFromIdx, TLenFieldIdx>(iter, len);
    }

    template <std::size_t TFromIdx, typename TIter, typename... TParams>
    ErrorStatus readEarlierFieldsInternal(TIter& iter, std::size_t& len, SkipOpTag<TParams...>)
    {
        static_cast<void>(iter);
        static_cast<void>(len);
        return ErrorStatus::Success;
    }   

    template <typename TIter, typename... TParams>
    ErrorStatus readRemLengthFieldInternal(TIter& iter, std::size_t& len, std::size_t& remLen, PerformOpTag<TParams...>)
    {
        auto& mems = BaseImpl::value();
        auto& lenField = std::get<TLenFieldIdx>(mems);

        auto beforeLenReadIter = iter;
        auto es = lenField.read(iter, len);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        auto lenFieldLen = static_cast<std::size_t>(std::distance(beforeLenReadIter, iter));
        COMMS_ASSERT(lenFieldLen <= len);
        len -= lenFieldLen;

        remLen = static_cast<std::size_t>(lenField.getValue());
        return ErrorStatus::Success;
    }

    template <typename TIter, typename... TParams>
    ErrorStatus readRemLengthFieldInternal(TIter& iter, std::size_t& len, std::size_t& remLen, SkipOpTag<TParams...>)
    {
        static_cast<void>(iter);
        static_cast<void>(len);
        auto& mems = BaseImpl::value();
        auto& lenField = std::get<TLenFieldIdx>(mems);
        remLen = lenField.value();
        return ErrorStatus::Success;
    }    

    template <std::size_t TUntilIdx, typename... TParams>
    void skipUntilFieldInternal(std::size_t& reqLen, PerformOpTag<TParams...>)
    {
        static_assert(TLenFieldIdx < TUntilIdx, "Invalid assumption");
        auto fieldsLen = BaseImpl::template lengthFromUntil<TLenFieldIdx + 1, TUntilIdx>();
        COMMS_ASSERT(fieldsLen <= reqLen);
        reqLen -= fieldsLen;
    }

    template <std::size_t TUntilIdx, typename... TParams>
    void skipUntilFieldInternal(std::size_t& reqLen, SkipOpTag<TParams...>)
    {
        static_cast<void>(reqLen);
    }    

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter, typename... TParams>
    ErrorStatus readFromUntilInternal(TIter& iter, std::size_t& len, LocalTag<TParams...>)
    {
        static_assert(TLenFieldIdx < TUntilIdx, "Invalid function invocation");
        using EarlierFieldsTag = 
            typename comms::util::LazyShallowConditional<
                (TFromIdx < TLenFieldIdx)
            >::template Type<
                PerformOpTag,
                SkipOpTag
            >; 

        auto es = readEarlierFieldsInternal<TFromIdx>(iter, len, EarlierFieldsTag());
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        using LenTag = 
            typename comms::util::LazyShallowConditional<
                (TFromIdx <= TLenFieldIdx)
            >::template Type<
                PerformOpTag,
                SkipOpTag
            >;         

        std::size_t reqLen = 0U;
        es = readRemLengthFieldInternal(iter, len, reqLen, LenTag());
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        using SkipTag = 
            typename comms::util::LazyShallowConditional<
                (TLenFieldIdx < TFromIdx)
            >::template Type<
                PerformOpTag,
                SkipOpTag
            >;    

        skipUntilFieldInternal<TFromIdx>(reqLen, SkipTag());

        static const std::size_t NextIdx = (TFromIdx <= TLenFieldIdx) ? TLenFieldIdx + 1 : TFromIdx;

        auto remLen = std::min(len, reqLen);
        es = BaseImpl::template readFromUntilAndUpdateLen<NextIdx, TUntilIdx>(iter, remLen);
        auto consumed = reqLen - remLen;
        len -= consumed;

        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        if (std::tuple_size<ValueType>::value <= TUntilIdx) {
            len -= remLen;
            std::advance(iter, remLen);
        }

        return es;
    }       

    bool refreshLengthInternal()
    {
        auto& mems = BaseImpl::value();
        auto& lenField = std::get<TLenFieldIdx>(mems);
        std::size_t expLen = BaseImpl::template lengthFrom<TLenFieldIdx + 1>();
        std::size_t actLen = static_cast<std::size_t>(lenField.getValue());
        if (expLen == actLen) {
            return false;
        }

        lenField.setValue(expLen);
        return true;
    }


    static const std::size_t MaxPossibleLen = 0xffff;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms



