//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"
#include "comms/field/basic/CommonFuncs.h"
#include "comms/field/details/FieldOpHelpers.h"
#include "comms/field/details/MembersVersionDependency.h"
#include "comms/field/tag.h"
#include "comms/util/Tuple.h"

#include <algorithm>
#include <type_traits>

namespace comms
{

namespace field
{

namespace basic
{

namespace details
{

template <comms::field::details::MembersVersionDependency TVersionDependency, typename... TMembers>
struct BundleVersionDependencyDetectHelper;

template <typename... TMembers>
struct BundleVersionDependencyDetectHelper<comms::field::details::MembersVersionDependency_NotSpecified, TMembers...>
{
    static constexpr bool Value = CommonFuncs::IsAnyFieldVersionDependentBoolType<TMembers...>::value;
};

template <typename... TMembers>
struct BundleVersionDependencyDetectHelper<comms::field::details::MembersVersionDependency_Independent, TMembers...>
{
    static constexpr bool Value = false;
};

template <typename... TMembers>
struct BundleVersionDependencyDetectHelper<comms::field::details::MembersVersionDependency_Dependent, TMembers...>
{
    static constexpr bool Value = true;
};

} // namespace details
    

template <typename TFieldBase, comms::field::details::MembersVersionDependency TVersionDependency, typename TMembers>
class Bundle;    

template <typename TFieldBase, comms::field::details::MembersVersionDependency TVersionDependency, typename... TMembers>
class Bundle<TFieldBase, TVersionDependency, std::tuple<TMembers...> > : public TFieldBase
{
public:
    using ValueType = std::tuple<TMembers...>;
    using Members = ValueType;
    using VersionType = typename TFieldBase::VersionType;
    using CommsTag = comms::field::tag::Bundle;

    Bundle() = default;
    explicit Bundle(const ValueType& val)
      : members_(val)
    {
    }

    explicit Bundle(ValueType&& val)
      : members_(std::move(val))
    {
    }

    Bundle(const Bundle&) = default;
    Bundle(Bundle&&) = default;
    ~Bundle() noexcept = default;

    Bundle& operator=(const Bundle&) = default;
    Bundle& operator=(Bundle&&) = default;

    const ValueType& value() const
    {
        return members_;
    }

    ValueType& value()
    {
        return members_;
    }

    const ValueType& getValue() const
    {
        return value();
    }

    template <typename T>
    void setValue(T&& val)
    {
        value() = std::forward<T>(val);
    }    

    constexpr std::size_t length() const
    {
        return comms::util::tupleAccumulate(
            value(), std::size_t(0), comms::field::details::FieldLengthSumCalcHelper<>());
    }

    template <std::size_t TFromIdx>
    constexpr std::size_t lengthFrom() const
    {
        return
            comms::util::tupleAccumulateFromUntil<TFromIdx, std::tuple_size<ValueType>::value>(
                value(),
                std::size_t(0),
                comms::field::details::FieldLengthSumCalcHelper<>());
    }

    template <std::size_t TUntilIdx>
    constexpr std::size_t lengthUntil() const
    {
        return
            comms::util::tupleAccumulateFromUntil<0, TUntilIdx>(
                value(),
                std::size_t(0),
                comms::field::details::FieldLengthSumCalcHelper<>());
    }

    template <std::size_t TFromIdx, std::size_t TUntilIdx>
    constexpr std::size_t lengthFromUntil() const
    {
        return
            comms::util::tupleAccumulateFromUntil<TFromIdx, TUntilIdx>(
                value(),
                std::size_t(0),
                comms::field::details::FieldLengthSumCalcHelper<>());
    }

    static constexpr std::size_t minLength()
    {
        return 
            comms::util::tupleTypeAccumulate<Members>(
                std::size_t(0), comms::field::details::FieldMinLengthSumCalcHelper<>());
    }

    template <std::size_t TFromIdx>
    static constexpr std::size_t minLengthFrom()
    {
        return 
            comms::util::tupleTypeAccumulateFromUntil<TFromIdx, std::tuple_size<ValueType>::value, Members>(
                std::size_t(0), comms::field::details::FieldMinLengthSumCalcHelper<>());
    }

    template <std::size_t TUntilIdx>
    static constexpr std::size_t minLengthUntil()
    {
        return 
            comms::util::tupleTypeAccumulateFromUntil<0, TUntilIdx, Members>(
                std::size_t(0), comms::field::details::FieldMinLengthSumCalcHelper<>());
    }

    template <std::size_t TFromIdx, std::size_t TUntilIdx>
    static constexpr std::size_t minLengthFromUntil()
    {
        return 
            comms::util::tupleTypeAccumulateFromUntil<TFromIdx, TUntilIdx, Members>(
                std::size_t(0), comms::field::details::FieldMinLengthSumCalcHelper<>());
    }

    static constexpr std::size_t maxLength()
    {
        return 
            comms::util::tupleTypeAccumulate<Members>(
                std::size_t(0), comms::field::details::FieldMaxLengthSumCalcHelper<>());        
    }

    template <std::size_t TFromIdx>
    static constexpr std::size_t maxLengthFrom()
    {
        return 
            comms::util::tupleTypeAccumulateFromUntil<TFromIdx, std::tuple_size<ValueType>::value, Members>(
                std::size_t(0), comms::field::details::FieldMaxLengthSumCalcHelper<>());      
    }

    template <std::size_t TUntilIdx>
    static constexpr std::size_t maxLengthUntil()
    {
        return 
            comms::util::tupleTypeAccumulateFromUntil<0, TUntilIdx, Members>(
                std::size_t(0), comms::field::details::FieldMaxLengthSumCalcHelper<>());
    }

    template <std::size_t TFromIdx, std::size_t TUntilIdx>
    static constexpr std::size_t maxLengthFromUntil()
    {
        return 
            comms::util::tupleTypeAccumulateFromUntil<TFromIdx, TUntilIdx, Members>(
                std::size_t(0), comms::field::details::FieldMaxLengthSumCalcHelper<>());
    }

    constexpr bool valid() const
    {
        return comms::util::tupleAccumulate(value(), true, comms::field::details::FieldValidCheckHelper<>());
    }

    bool refresh()
    {
        return comms::util::tupleAccumulate(value(), false, comms::field::details::FieldRefreshHelper<>());
    }

    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t len)
    {
        auto es = ErrorStatus::Success;
        comms::util::tupleForEach(value(), makeReadHelper(es, iter, len));
        return es;
    }

    template <std::size_t TFromIdx, typename TIter>
    ErrorStatus readFrom(TIter& iter, std::size_t len)
    {
        return readFromAndUpdateLen<TFromIdx>(iter, len);
    }

    template <std::size_t TFromIdx, typename TIter>
    ErrorStatus readFromAndUpdateLen(TIter& iter, std::size_t& len)
    {
        auto es = ErrorStatus::Success;
        comms::util::template tupleForEachFrom<TFromIdx>(value(), makeReadHelper(es, iter, len));
        return es;
    }    

    template <std::size_t TUntilIdx, typename TIter>
    ErrorStatus readUntil(TIter& iter, std::size_t len)
    {
        return readUntilAndUpdateLen<TUntilIdx>(iter, len);
    }

    template <std::size_t TUntilIdx, typename TIter>
    ErrorStatus readUntilAndUpdateLen(TIter& iter, std::size_t& len)
    {
        auto es = ErrorStatus::Success;
        comms::util::template tupleForEachUntil<TUntilIdx>(value(), makeReadHelper(es, iter, len));
        return es;
    }    

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus readFromUntil(TIter& iter, std::size_t len)
    {
        return readFromUntilAndUpdateLen<TFromIdx, TUntilIdx>(iter, len);
    }

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus readFromUntilAndUpdateLen(TIter& iter, std::size_t& len)
    {
        auto es = ErrorStatus::Success;
        comms::util::template tupleForEachFromUntil<TFromIdx, TUntilIdx>(value(), makeReadHelper(es, iter, len));
        return es;
    }    

    static constexpr bool hasReadNoStatus()
    {
        return CommonFuncs::AllFieldsHaveReadNoStatusBoolType<TMembers...>::value;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter)
    {
        comms::util::tupleForEach(value(), makeReadNoStatusHelper(iter));
    }

    template <std::size_t TFromIdx, typename TIter>
    void readFromNoStatus(TIter& iter)
    {
        comms::util::template tupleForEachFrom<TFromIdx>(value(), makeReadNoStatusHelper(iter));
    }

    template <std::size_t TUntilIdx, typename TIter>
    void readUntilNoStatus(TIter& iter)
    {
        comms::util::template tupleForEachUntil<TUntilIdx>(value(), makeReadNoStatusHelper(iter));
    }

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    void readFromUntilNoStatus(TIter& iter)
    {
        comms::util::template tupleForEachFromUntil<TFromIdx, TUntilIdx>(value(), makeReadNoStatusHelper(iter));
    }

    bool canWrite() const
    {
        return 
            comms::util::tupleAccumulate(
                value(), true, comms::field::details::FieldCanWriteCheckHelper<>());
    }

    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t len) const
    {
        auto es = ErrorStatus::Success;
        comms::util::tupleForEach(value(), makeWriteHelper(es, iter, len));
        return es;
    }

    template <std::size_t TFromIdx, typename TIter>
    ErrorStatus writeFrom(TIter& iter, std::size_t len) const
    {
        auto es = ErrorStatus::Success;
        comms::util::template tupleForEachFrom<TFromIdx>(value(), makeWriteHelper(es, iter, len));
        return es;
    }

    template <std::size_t TUntilIdx, typename TIter>
    ErrorStatus writeUntil(TIter& iter, std::size_t len) const
    {
        auto es = ErrorStatus::Success;
        comms::util::template tupleForEachUntil<TUntilIdx>(value(), makeWriteHelper(es, iter, len));
        return es;
    }

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    ErrorStatus writeFromUntil(TIter& iter, std::size_t len) const
    {
        auto es = ErrorStatus::Success;
        comms::util::template tupleForEachFromUntil<TFromIdx, TUntilIdx>(value(), makeWriteHelper(es, iter, len));
        return es;
    }

    static constexpr bool hasWriteNoStatus()
    {
        return CommonFuncs::AllFieldsHaveWriteNoStatusBoolType<TMembers...>::value;
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) const
    {
        comms::util::tupleForEach(value(), makeWriteNoStatusHelper(iter));
    }

    template <std::size_t TFromIdx, typename TIter>
    void writeFromNoStatus(TIter& iter) const
    {
        comms::util::template tupleForEachFrom<TFromIdx>(value(), makeWriteNoStatusHelper(iter));
    }

    template <std::size_t TUntilIdx, typename TIter>
    void writeUntilNoStatus(TIter& iter) const
    {
        comms::util::template tupleForEachUntil<TUntilIdx>(value(), makeWriteNoStatusHelper(iter));
    }

    template <std::size_t TFromIdx, std::size_t TUntilIdx, typename TIter>
    void writeFromUntilNoStatus(TIter& iter) const
    {
        comms::util::template tupleForEachFromUntil<TFromIdx, TUntilIdx>(value(), makeWriteNoStatusHelper(iter));
    }

    static constexpr bool isVersionDependent()
    {
        return details::BundleVersionDependencyDetectHelper<TVersionDependency, TMembers...>::Value;
    }

    static constexpr bool hasNonDefaultRefresh()
    {
        return CommonFuncs::AnyFieldHasNonDefaultRefreshBoolType<TMembers...>::value;
    }

    bool setVersion(VersionType version)
    {
        return CommonFuncs::setVersionForMembers(value(), version);
    }

private:
    template <typename TIter>
    static comms::field::details::FieldReadHelper<TIter> makeReadHelper(comms::ErrorStatus& es, TIter& iter, std::size_t& len)
    {
        return comms::field::details::FieldReadHelper<TIter>(es, iter, len);
    }

    template <typename TIter>
    static comms::field::details::FieldReadNoStatusHelper<TIter> makeReadNoStatusHelper(TIter& iter)
    {
        return comms::field::details::FieldReadNoStatusHelper<TIter>(iter);
    }

    template <typename TIter>
    static comms::field::details::FieldWriteHelper<TIter> makeWriteHelper(ErrorStatus& es, TIter& iter, std::size_t len)
    {
        return comms::field::details::FieldWriteHelper<TIter>(es, iter, len);
    }

    template <typename TIter>
    static comms::field::details::FieldWriteNoStatusHelper<TIter> makeWriteNoStatusHelper(TIter& iter)
    {
        return comms::field::details::FieldWriteNoStatusHelper<TIter>(iter);
    }

    static_assert(comms::util::IsTuple<ValueType>::Value, "ValueType must be tuple");
    ValueType members_;
};

}  // namespace basic

}  // namespace field

}  // namespace comms


