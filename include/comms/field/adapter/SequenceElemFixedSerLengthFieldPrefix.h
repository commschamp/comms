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
class SequenceElemFixedSerLengthFieldPrefix : public TBase
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

    SequenceElemFixedSerLengthFieldPrefix() = default;

    explicit SequenceElemFixedSerLengthFieldPrefix(const ValueType& val)
      : BaseImpl(val)
    {
    }

    explicit SequenceElemFixedSerLengthFieldPrefix(ValueType&& val)
      : BaseImpl(std::move(val))
    {
    }

    SequenceElemFixedSerLengthFieldPrefix(const SequenceElemFixedSerLengthFieldPrefix&) = default;
    SequenceElemFixedSerLengthFieldPrefix(SequenceElemFixedSerLengthFieldPrefix&&) = default;
    SequenceElemFixedSerLengthFieldPrefix& operator=(const SequenceElemFixedSerLengthFieldPrefix&) = default;
    SequenceElemFixedSerLengthFieldPrefix& operator=(SequenceElemFixedSerLengthFieldPrefix&&) = default;

    std::size_t length() const
    {

        using LenFieldLengthTag =
            typename comms::util::LazyShallowConditional<
                LenField::minLength() == LenField::maxLength()
            >::template Type<
                FixedLengthLenFieldTag,
                VarLengthLenFieldTag
            >;
                    
        return lengthInternal(LenFieldLengthTag());
    }

    static constexpr std::size_t minLength()
    {
        return LenField::minLength();
    }

    static constexpr std::size_t maxLength()
    {
        return basic::CommonFuncs::maxSupportedLength();
    }

    template <typename TIter>
    ErrorStatus readElement(ElementType& elem, TIter& iter, std::size_t& len) const
    {
        COMMS_ASSERT(elemLen_ < MaxLengthLimit);

        if (len < elemLen_) {
            return ErrorStatus::NotEnoughData;
        }

        std::size_t elemLen = elemLen_;
        auto es = BaseImpl::readElement(elem, iter, elemLen);
        if (es == ErrorStatus::NotEnoughData) {
            return TStatus;
        }

        if (es != ErrorStatus::Success) {
            return es;
        }

        COMMS_ASSERT(elemLen <= elemLen_);
        std::advance(iter, elemLen);
        len -= elemLen_;
        return ErrorStatus::Success;
    }

    template <typename TIter>
    void readElementNoStatus(ElementType& elem, TIter& iter) const = delete;

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        if (len == 0U) {
            elemLen_ = 0U;
            return ErrorStatus::Success;
        }
                
        auto es = readLen(iter, len);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

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
        if (0U < count) {
            auto es = readLen(iter, len);
            if (es != comms::ErrorStatus::Success) {
                return es;
            }
        }
        else {
            elemLen_ = 0U;
        }
        return basic::CommonFuncs::readSequenceN(*this, count, iter, len);
    }

    template <typename TIter>
    void readNoStatusN(std::size_t count, TIter& iter) = delete;

    bool canWriteElement(const ElementType& elem) const
    {
        if (!BaseImpl::canWriteElement(elem)) {
            return false;
        }

        auto elemLen = elem.length();
        if (MaxAllowedElemLength < elemLen) {
            return false;
        }

        LenField lenField;
        lenField.setValue(elemLen);
        return lenField.canWrite();
    }

    bool canWrite() const
    {
        if (BaseImpl::getValue().empty()) {
            return BaseImpl::canWrite();
        }

        return BaseImpl::canWrite() && canWriteElement(BaseImpl::getValue().front());
    }

    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t len) const
    {
        if (!BaseImpl::getValue().empty()) {
            if (!canWriteElement(BaseImpl::getValue().front())) {
                return ErrorStatus::InvalidMsgData;
            }

            auto es = writeLen(iter, len); // len is updated
            if (es != comms::ErrorStatus::Success) {
                return es;
            }
        }

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
        if (0U < count) {
            COMMS_ASSERT(!BaseImpl::getValue().empty());
            if (!canWriteElement(BaseImpl::getValue().front())) {
                return ErrorStatus::InvalidMsgData;
            }

            auto es = writeLen(iter, len); // len is updated
            if (es != comms::ErrorStatus::Success) {
                return es;
            }
        }

        return basic::CommonFuncs::writeSequenceN(*this, count, iter, len);
    }

    template <typename TIter>
    void writeNoStatusN(std::size_t count, TIter& iter) const = delete;

    bool valid() const
    {
        return BaseImpl::valid() && canWrite();
    }

private:

    template <typename... TParams>
    using FixedLengthLenFieldTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using VarLengthLenFieldTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    std::size_t lengthInternal(FixedLengthLenFieldTag<TParams...>) const
    {
        std::size_t prefixLen = 0U;
        if (!BaseImpl::getValue().empty()) {
            prefixLen = LenField::minLength();
        }
        return (prefixLen + BaseImpl::length());
    }

    template <typename... TParams>
    std::size_t lengthInternal(VarLengthLenFieldTag<TParams...>) const
    {
        std::size_t prefixLen = 0U;
        if (!BaseImpl::getValue().empty()) {
            LenField lenField;
            lenField.setValue( 
                    std::min(BaseImpl::minElementLength(), std::size_t(MaxAllowedElemLength)));
            prefixLen = lenField.length();
        }

        return (prefixLen + BaseImpl::length());
    }

    template <typename TIter>
    static void advanceWriteIterator(TIter& iter, std::size_t len)
    {
        basic::CommonFuncs::advanceWriteIterator(iter, len);
    }

    template <typename TIter>
    ErrorStatus readLen(TIter& iter, std::size_t& len)
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

        elemLen_ = static_cast<std::size_t>(lenField.getValue());
        if (elemLen_ == MaxLengthLimit) {
            return TStatus;
        }

        return ErrorStatus::Success;
    }

    template <typename TIter>
    ErrorStatus writeLen(TIter& iter, std::size_t& len) const
    {
        auto elemLength = BaseImpl::minElementLength();
        LenField lenField;
        lenField.setValue(elemLength);
        auto es = lenField.write(iter, len);
        if (es != ErrorStatus::Success) {
            return es;
        }

        len -= lenField.length();
        return es;
    }

    template <typename TIter>
    void writeLenNoStatus(TIter& iter) const
    {
        auto elemLength = BaseImpl::minElementLength();
        LenField lenField;
        lenField.setValue(elemLength);
        lenField.writeNoStatus(iter);
    }

    static_assert(BaseImpl::minElementLength() == BaseImpl::maxElementLength(),
            "Option SequenceElemFixedSerLengthFieldPrefix can be used only with fixed length "
            "elements.");
    static_assert(1U <= LenField::minLength(), "Invalid min length assumption");

    static const std::size_t MaxLengthLimit =
        std::numeric_limits<std::size_t>::max();
    std::size_t elemLen_ = MaxLengthLimit;
};

}  // namespace adapter

}  // namespace field

}  // namespace comms




