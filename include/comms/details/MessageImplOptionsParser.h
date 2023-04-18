//
// Copyright 2015 - 2023 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/options.h"
#include "comms/util/type_traits.h"
#include "MessageImplBases.h"

namespace comms
{

namespace details
{

template <typename T, T TValue>
struct MessageImplValueWrapper
{
    static constexpr T Value = TValue;
};

template <typename... TOptions>
class MessageImplOptionsParser;

template <>
class MessageImplOptionsParser<>
{
public:
    static constexpr bool HasFieldsImpl = false;
    static constexpr bool HasMsgType = false;
    static constexpr bool HasNoReadImpl = false;
    static constexpr bool HasNoWriteImpl = false;
    static constexpr bool HasNoValidImpl = false;
    static constexpr bool HasNoLengthImpl = false;
    static constexpr bool HasNoRefreshImpl = false;
    static constexpr bool HasCustomRefresh = false;
    static constexpr bool HasVersionDependentFields = false;
    static constexpr bool HasFieldsWithNonDefaultRefresh = false;
    static constexpr bool HasNoDispatchImpl = false;
    static constexpr bool HasStaticMsgId = false;
    static constexpr bool HasDoGetId = false;
    static constexpr bool HasNoIdImpl = false;
    static constexpr bool HasName = false;
    static constexpr bool HasFailOnInvalid = false;

    using Fields = std::tuple<>;
    using MsgType = void;
    static constexpr std::intmax_t MsgId = std::numeric_limits<std::intmax_t>::max();
    using FailOnInvalidStatusWrapper = MessageImplValueWrapper<comms::ErrorStatus, comms::ErrorStatus::InvalidMsgData>;

    template <typename TBase>
    using BuildFieldsImpl = TBase;

    template <typename TBase>
    using BuildVersionImpl = TBase;

    template <typename TBase>
    using BuildReadImpl = TBase;

    template <typename TBase>
    using BuildWriteImpl = TBase;    

    template <typename TBase>
    using BuildValidImpl = TBase;     

    template <typename TBase>
    using BuildLengthImpl = TBase;      

    template <typename TBase>
    using BuildRefreshImpl = TBase; 

    template <typename TBase>
    using BuildDispatchImpl = TBase;   

    template <typename TBase>
    using BuildStaticMsgId = TBase;  

    template <typename TBase>
    using BuildMsgIdImpl = TBase;    

    template <typename TBase>
    using BuildNameImpl = TBase;     

    template <typename TBase>
    using BuildFailOnInvalidImpl = TBase;                   
};

template <std::intmax_t TId,
          typename... TOptions>
class MessageImplOptionsParser<
    comms::option::def::StaticNumIdImpl<TId>,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
    using BaseImpl = MessageImplOptionsParser<TOptions...>;

    static_assert(!BaseImpl::HasStaticMsgId,
        "comms::option::def::StaticNumIdImpl option is used more than once");
    static_assert(!BaseImpl::HasNoIdImpl,
        "comms::option::app::NoIdImpl and comms::option::def::StaticNumIdImpl options cannot be used together");
public:
    static constexpr bool HasStaticMsgId = true;
    static constexpr std::intmax_t MsgId = TId;

    template <typename TBase>
    using BuildStaticMsgId = 
        typename comms::util::Conditional<
            TBase::hasMsgIdType() // most likely to be true
        >::template Type<
            MessageImplStaticNumIdBase<TBase, MsgId>,
            TBase
        >;

    template <typename TBase>
    using BuildMsgIdImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasGetId() && (!BaseImpl::HasNoIdImpl)
        >::template Type<
            MessageImplPolymorhpicStaticNumIdBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType
        >;    
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::app::NoDispatchImpl,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
public:
    static constexpr bool HasNoDispatchImpl = true;

    template <typename TBase>
    using BuildDispatchImpl = TBase;
};

template <typename TFields,
          typename... TOptions>
class MessageImplOptionsParser<
    comms::option::def::FieldsImpl<TFields>,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
    using BaseImpl = MessageImplOptionsParser<TOptions...>;

    static_assert(!BaseImpl::HasFieldsImpl,
        "comms::option::def::FieldsImpl option is used more than once");
public:
    static constexpr bool HasFieldsImpl = true;
    using Fields = TFields;
    static constexpr bool HasVersionDependentFields = MessageImplFieldsContainer<Fields>::areFieldsVersionDependent();
    static constexpr bool HasFieldsWithNonDefaultRefresh = MessageImplFieldsContainer<Fields>::doFieldsHaveNonDefaultRefresh();

    template <typename TBase>
    using BuildFieldsImpl = MessageImplFieldsBase<TBase, Fields>;

    template <typename TBase>
    using BuildFailOnInvalidImpl = 
        typename comms::util::LazyShallowDeepConditional<
            BaseImpl::HasFailOnInvalid
        >::template Type<
            MessageImplFailOnInvalidBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType, typename BaseImpl::FailOnInvalidStatusWrapper
        >;       

    template <typename TBase>
    using BuildVersionImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasVersionInTransportFields()
        >::template Type<
            MessageImplVersionBase,
            comms::util::TypeDeepWrap,
            TBase
        >;

    template <typename TBase>
    using BuildReadImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasRead() && (!BaseImpl::HasNoReadImpl)
        >::template Type<
            MessageImplFieldsReadImplBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType
        >;    

    template <typename TBase>
    using BuildWriteImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasWrite() && (!BaseImpl::HasNoWriteImpl)
        >::template Type<
            MessageImplFieldsWriteImplBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType
        >;      

    template <typename TBase>
    using BuildValidImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasValid() && (!BaseImpl::HasNoValidImpl)
        >::template Type<
            MessageImplFieldsValidBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType
        >;   

    template <typename TBase>
    using BuildLengthImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasLength() && (!BaseImpl::HasNoLengthImpl)
        >::template Type<
            MessageImplFieldsLengthBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType
        >;      

    template <typename TBase>
    using BuildRefreshImpl = 
        typename comms::util::LazyShallowDeepConditional<
            (TBase::hasRefresh()) &&
            (!BaseImpl::HasNoRefreshImpl) &&
            (
                    BaseImpl::HasCustomRefresh ||
                    HasFieldsWithNonDefaultRefresh ||
                    (TBase::hasVersionInTransportFields() && HasVersionDependentFields)
            )
        >::template Type<
            MessageImplRefreshBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType
        >;
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::def::NoIdImpl,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
    using BaseImpl = MessageImplOptionsParser<TOptions...>;

    static_assert(!BaseImpl::HasNoIdImpl,
        "comms::option::def::NoIdImpl option is used more than once");
    static_assert(!BaseImpl::HasStaticMsgId,
        "comms::option::def::NoIdImpl and comms::option::def::StaticNumIdImpl options cannot be used together");
public:
    static constexpr bool HasNoIdImpl = true;

    template <typename TBase>
    using BuildMsgIdImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasGetId()
        >::template Type<
            MessageImplNoIdBase,
            comms::util::TypeDeepWrap,
            TBase
        >; 
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::app::NoReadImpl,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
public:
    static constexpr bool HasNoReadImpl = true;

    template <typename TBase>
    using BuildReadImpl = TBase;
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::app::NoWriteImpl,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
public:
    static constexpr bool HasNoWriteImpl = true;

    template <typename TBase>
    using BuildWriteImpl = TBase;    
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::app::NoLengthImpl,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
public:
    static constexpr bool HasNoLengthImpl = true;

    template <typename TBase>
    using BuildLengthImpl = TBase;     
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::app::NoValidImpl,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
public:
    static constexpr bool HasNoValidImpl = true;

    template <typename TBase>
    using BuildValidImpl = TBase;   
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::app::NoRefreshImpl,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
public:
    static constexpr bool HasNoRefreshImpl = true;

    template <typename TBase>
    using BuildRefreshImpl = TBase;
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::def::HasCustomRefresh,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
    using BaseImpl = MessageImplOptionsParser<TOptions...>;
public:
    static constexpr bool HasCustomRefresh = true;

    template <typename TBase>
    using BuildRefreshImpl = 
        typename comms::util::LazyShallowDeepConditional<
            (TBase::hasRefresh()) && (!BaseImpl::HasNoRefreshImpl) 
        >::template Type<
            MessageImplRefreshBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType
        >;       
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::def::HasName,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
    using BaseImpl = MessageImplOptionsParser<TOptions...>;

public:
    static constexpr bool HasName = true;

    template <typename TBase>
    using BuildNameImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasName() && BaseImpl::HasMsgType
        >::template Type<
            MessageImplNameBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType
        >;         
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::def::HasDoGetId,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
    using BaseImpl = MessageImplOptionsParser<TOptions...>;
public:
    static constexpr bool HasDoGetId = true;

    template <typename TBase>
    using BuildMsgIdImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasGetId() && (!BaseImpl::HasNoIdImpl) &&
                (BaseImpl::HasStaticMsgId || BaseImpl::HasMsgType)
        >::template Type<
            MessageImplPolymorhpicStaticNumIdBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType
        >;     
};

template <typename TMsgType,
          typename... TOptions>
class MessageImplOptionsParser<
    comms::option::def::MsgType<TMsgType>,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
    using BaseImpl = MessageImplOptionsParser<TOptions...>;

    static_assert(!BaseImpl::HasMsgType,
        "comms::option::def::MsgType option is used more than once");
public:
    static constexpr bool HasMsgType = true;
    using MsgType = TMsgType;

    template <typename TBase>
    using BuildFailOnInvalidImpl = 
        typename comms::util::LazyShallowDeepConditional<
            BaseImpl::HasFieldsImpl && BaseImpl::HasFailOnInvalid
        >::template Type<
            MessageImplFailOnInvalidBase,
            comms::util::TypeDeepWrap,
            TBase, MsgType, typename BaseImpl::FailOnInvalidStatusWrapper
        >;       

    template <typename TBase>
    using BuildReadImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasRead() && (!BaseImpl::HasNoReadImpl)
        >::template Type<
            MessageImplFieldsReadImplBase,
            comms::util::TypeDeepWrap,
            TBase, MsgType
        >;     

    template <typename TBase>
    using BuildWriteImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasWrite() && (!BaseImpl::HasNoWriteImpl)
        >::template Type<
            MessageImplFieldsWriteImplBase,
            comms::util::TypeDeepWrap,
            TBase, MsgType
        >;    

    template <typename TBase>
    using BuildValidImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasValid() && (!BaseImpl::HasNoValidImpl)
        >::template Type<
            MessageImplFieldsValidBase,
            comms::util::TypeDeepWrap,
            TBase, MsgType
        >;         

    template <typename TBase>
    using BuildLengthImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasLength() && (!BaseImpl::HasNoLengthImpl)
        >::template Type<
            MessageImplFieldsLengthBase,
            comms::util::TypeDeepWrap,
            TBase, MsgType
        >;     

    template <typename TBase>
    using BuildRefreshImpl = 
        typename comms::util::LazyShallowDeepConditional<
            (TBase::hasRefresh()) &&
            (!BaseImpl::HasNoRefreshImpl) &&
            (
                    BaseImpl::HasCustomRefresh ||
                    BaseImpl::HasFieldsWithNonDefaultRefresh ||
                    (TBase::hasVersionInTransportFields() && BaseImpl::HasVersionDependentFields)
            )
        >::template Type<
            MessageImplRefreshBase,
            comms::util::TypeDeepWrap,
            TBase, MsgType
        >;   

    template <typename TBase>
    using BuildDispatchImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasDispatch() && (!BaseImpl::HasNoDispatchImpl)
        >::template Type<
            MessageImplDispatchBase,
            comms::util::TypeDeepWrap,
            TBase, MsgType
        >;       

    template <typename TBase>
    using BuildMsgIdImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasGetId() && (!BaseImpl::HasNoIdImpl) &&
                (BaseImpl::HasStaticMsgId || BaseImpl::HasDoGetId)
        >::template Type<
            MessageImplPolymorhpicStaticNumIdBase,
            comms::util::TypeDeepWrap,
            TBase, MsgType
        >;    

    template <typename TBase>
    using BuildNameImpl = 
        typename comms::util::LazyShallowDeepConditional<
            TBase::hasName() && BaseImpl::HasName
        >::template Type<
            MessageImplNameBase,
            comms::util::TypeDeepWrap,
            TBase, MsgType
        >; 
};

template <comms::ErrorStatus TStatus, typename... TOptions>
class MessageImplOptionsParser<
    comms::option::def::FailOnInvalid<TStatus>,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
    using BaseImpl = MessageImplOptionsParser<TOptions...>;

public:
    static constexpr bool HasFailOnInvalid = true;
    using FailOnInvalidStatusWrapper = MessageImplValueWrapper<comms::ErrorStatus, TStatus>;

    template <typename TBase>
    using BuildFailOnInvalidImpl = 
        typename comms::util::LazyShallowDeepConditional<
            BaseImpl::HasFieldsImpl
        >::template Type<
            MessageImplFailOnInvalidBase,
            comms::util::TypeDeepWrap,
            TBase, typename BaseImpl::MsgType, FailOnInvalidStatusWrapper
        >;         
};

template <typename... TOptions>
class MessageImplOptionsParser<
    comms::option::app::EmptyOption,
    TOptions...> : public MessageImplOptionsParser<TOptions...>
{
};

template <typename... TBundledOptions, typename... TOptions>
class MessageImplOptionsParser<
    std::tuple<TBundledOptions...>,
    TOptions...> : public MessageImplOptionsParser<TBundledOptions..., TOptions...>
{
};

}  // namespace details

}  // namespace comms



