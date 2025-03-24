//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/details/tag.h"
#include "comms/ErrorStatus.h"
#include "comms/field/basic/CommonFuncs.h"
#include "comms/util/type_traits.h"

#include <algorithm>
#include <iterator>
#include <limits>

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TLenField, comms::ErrorStatus TStatus, typename TBase>
class SequenceElemSerLengthFieldPrefix : public TBase
{
    using BaseImpl = TBase;
    using LenField = TLenField;
    static const std::size_t MaxAllowedElemLength =
            static_cast<std::size_t>(LenField::maxValue());

    static_assert(!LenField::isVersionDependent(),
            "Prefix fields must not be version dependent");

public:
    using ValueType = typename BaseImpl::ValueType;
    using ElementType = typename BaseImpl::ElementType;

    SequenceElemSerLengthFieldPrefix() = default;

    explicit SequenceElemSerLengthFieldPrefix(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit SequenceElemSerLengthFieldPrefix(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    SequenceElemSerLengthFieldPrefix(const SequenceElemSerLengthFieldPrefix&) = default;
    SequenceElemSerLengthFieldPrefix(SequenceElemSerLengthFieldPrefix&&) = default;
    SequenceElemSerLengthFieldPrefix& operator=(const SequenceElemSerLengthFieldPrefix&) = default;
    SequenceElemSerLengthFieldPrefix& operator=(SequenceElemSerLengthFieldPrefix&&) = default;

    std::size_t length() const
    {
        using ElemLengthTag =
            typename comms::util::LazyShallowConditional<
                BaseImpl::minElementLength() == BaseImpl::maxElementLength()
            >::template Type<
                FixedLengthElemTag,
                VarLengthElemTag
            >;
        return lengthInternal(LenFieldLengthTag<>(), ElemLengthTag());
    }

    std::size_t elementLength(const ElementType& elem) const
    {
        return elementLengthInternal(elem, LenFieldLengthTag<>());
    }

    static constexpr std::size_t minLength()
    {
        return LenField::minLength();
    }    

    static constexpr std::size_t maxLength()
    {
        return basic::CommonFuncs::maxSupportedLength();
    }

    static constexpr std::size_t minElementLength()
    {
        return LenField::minLength() + BaseImpl::minElementLength();
    }

    static constexpr std::size_t maxElementLength()
    {
        return LenField::maxLength() + BaseImpl::maxElementLength();
    }

    template <typename TIter>
    ErrorStatus readElement(ElementType& elem, TIter& iter, std::size_t& len) const
    {
        auto fromIter = iter;
        LenField lenField;
        auto es = lenField.read(iter, len);
        if (es != ErrorStatus::Success) {
            return es;
        }

        auto diff = static_cast<std::size_t>(std::distance(fromIter, iter));
        COMMS_ASSERT(diff <= len);
        len -= diff;
        if (len < lenField.getValue()) {
            return comms::ErrorStatus::NotEnoughData;
        }

        const auto reqLen = static_cast<std::size_t>(lenField.getValue());
        std::size_t elemLen = reqLen;
        es = BaseImpl::readElement(elem, iter, elemLen);
        if (es == ErrorStatus::NotEnoughData) {
            return TStatus;
        }

        if (es != ErrorStatus::Success) {
            return es;
        }

        COMMS_ASSERT(elemLen <= reqLen);
        std::advance(iter, elemLen);
        len -= reqLen;
        return ErrorStatus::Success;
    }

    template <typename TIter>
    void readElementNoStatus(ElementType& elem, TIter& iter) const = delete;

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        return basic::CommonFuncs::readSequence(*this, iter, len);
    }

    static constexpr bool hasReadNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;

    template <typename TIter>
    ErrorStatus readN(std::size_t count, TIter& iter, std::size_t& len)
    {
        return basic::CommonFuncs::readSequenceN(*this, count, iter, len);
    }

    template <typename TIter>
    void readNoStatusN(std::size_t count, TIter& iter) = delete;

    bool canWriteElement(const ElementType& elem) const
    {
        if (!BaseImpl::canWriteElement(elem)) {
            return false;
        }

        auto elemLen = elementLength(elem);
        if (MaxAllowedElemLength < elemLen) {
            return false;
        }

        LenField lenField;
        lenField.setValue(elemLen);
        return lenField.canWrite();
    }

    template <typename TIter>
    ErrorStatus writeElement(const ElementType& elem, TIter& iter, std::size_t& len) const
    {
        if (!canWriteElement(elem)) {
            return ErrorStatus::InvalidMsgData;
        }

        auto elemLength = BaseImpl::elementLength(elem);
        LenField lenField;
        lenField.setValue(elemLength);
        auto es = lenField.write(iter, len);
        if (es != ErrorStatus::Success) {
            return es;
        }

        len -= lenField.length();
        return BaseImpl::writeElement(elem, iter, len);
    }

    template <typename TIter>
    static void writeElementNoStatus(const ElementType& elem, TIter& iter) = delete;

    bool canWrite() const
    {
        return basic::CommonFuncs::canWriteSequence(*this);
    }

    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t len) const
    {
        return basic::CommonFuncs::writeSequence(*this, iter, len);
    }

    static constexpr bool hasWriteNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) const = delete;

    template <typename TIter>
    ErrorStatus writeN(std::size_t count, TIter& iter, std::size_t& len) const
    {
        return basic::CommonFuncs::writeSequenceN(*this, count, iter, len);
    }

    template <typename TIter>
    void writeNoStatusN(std::size_t count, TIter& iter) const = delete;

    bool valid() const
    {
        if (!BaseImpl::valid()) {
            return false;
        }

        auto& vec = BaseImpl::getValue();
        for (auto& elem : vec) {
            auto elemLen = BaseImpl::elementLength(elem);
            if (MaxAllowedElemLength < elemLen) {
                return false;
            }
        }
        return true;
    }

private:

    template <typename... TParams>
    using FixedLengthLenFieldTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using VarLengthLenFieldTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    using FixedLengthElemTag = comms::details::tag::Tag3<>;

    template <typename... TParams>
    using VarLengthElemTag = comms::details::tag::Tag4<>;    

    template <typename...>
    using LenFieldLengthTag =
        typename comms::util::LazyShallowConditional<
            LenField::minLength() == LenField::maxLength()
        >::template Type<
            FixedLengthLenFieldTag,
            VarLengthLenFieldTag
        >;

    template<typename... TParams>
    std::size_t lengthInternal(FixedLengthLenFieldTag<TParams...>, FixedLengthElemTag<TParams...>) const
    {
        return (LenField::minLength() + BaseImpl::minElementLength()) * BaseImpl::getValue().size();
    }

    template<typename... TParams>
    std::size_t lengthInternal(FixedLengthLenFieldTag<TParams...>, VarLengthElemTag<TParams...>) const
    {
        return lengthInternalIterative();
    }

    template <typename... TParams>
    std::size_t lengthInternal(VarLengthLenFieldTag<TParams...>, FixedLengthElemTag<TParams...>) const
    {
        auto origElemLen = BaseImpl::minElementLength();
        auto elemLen = std::min(origElemLen, std::size_t(MaxAllowedElemLength));
        LenField lenField;
        lenField.setValue(elemLen);
        return (lenField.length() + origElemLen) * BaseImpl::getValue().size();
    }

    template <typename... TParams>
    std::size_t lengthInternal(VarLengthLenFieldTag<TParams...>, VarLengthElemTag<TParams...>) const
    {
        return lengthInternalIterative();
    }

    std::size_t lengthInternalIterative() const
    {
        std::size_t result = 0U;
        for (auto& elem : BaseImpl::getValue()) {
            result += elementLength(elem);
        }
        return result;
    }

    template<typename... TParams>
    std::size_t elementLengthInternal(const ElementType& elem, FixedLengthLenFieldTag<TParams...>) const
    {
        return LenField::minLength() + BaseImpl::elementLength(elem);
    }

    template <typename... TParams>
    std::size_t elementLengthInternal(const ElementType& elem, VarLengthLenFieldTag<TParams...>) const
    {
        LenField lenField;
        auto origElemLength = BaseImpl::elementLength(elem);
        auto elemLength = std::min(origElemLength, std::size_t(MaxAllowedElemLength));
        lenField.setValue(elemLength);
        return lenField.length() + origElemLength;
    }

    template <typename TIter>
    static void advanceWriteIterator(TIter& iter, std::size_t len)
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using ByteType = typename std::iterator_traits<IterType>::value_type;
        while (len > 0U) {
            *iter = ByteType();
            ++iter;
            --len;
        }
    }
};

}  // namespace adapter

}  // namespace field

}  // namespace comms





