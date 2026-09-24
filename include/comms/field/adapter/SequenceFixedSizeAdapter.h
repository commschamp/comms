//
// Copyright 2015 - 2026 (C). Alex Robenko. All rights reserved.
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"
#include "comms/details/tag.h"
#include "comms/ErrorStatus.h"
#include "comms/field/basic/BasicCommonFuncs.h"
#include "comms/util/detect.h"
#include "comms/util/type_traits.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace comms
{

namespace field
{

namespace adapter
{

template <typename TBase>
class SequenceFixedSizeAdapterBase : public TBase
{
    using BaseImpl = TBase;

public:
    using ValueType = typename BaseImpl::ValueType;
    using ElementType = typename BaseImpl::ElementType;

    explicit SequenceFixedSizeAdapterBase(std::size_t maxSize)
      : m_fixedSize(maxSize)
    {
    }

    SequenceFixedSizeAdapterBase(std::size_t maxSize, const ValueType& val)
      : BaseImpl(val),
        m_fixedSize(maxSize)
    {
    }

    SequenceFixedSizeAdapterBase(std::size_t maxSize, ValueType&& val)
      : BaseImpl(std::move(val)),
        m_fixedSize(maxSize)
    {
    }

    SequenceFixedSizeAdapterBase(const SequenceFixedSizeAdapterBase&) = default;
    SequenceFixedSizeAdapterBase(SequenceFixedSizeAdapterBase&&) = default;
    SequenceFixedSizeAdapterBase& operator=(const SequenceFixedSizeAdapterBase&) = default;
    SequenceFixedSizeAdapterBase& operator=(SequenceFixedSizeAdapterBase&&) = default;

    std::size_t length() const
    {
        auto currSize = BaseImpl::getValue().size();
        if (currSize == m_fixedSize) {
            return BaseImpl::length();
        }

        if (currSize < m_fixedSize) {
            auto remSize = m_fixedSize - currSize;
            auto dummyElem = ElementType();
            return BaseImpl::length() + (remSize * BaseImpl::elementLength(dummyElem));
        }

        using Tag =
            typename comms::util::LazyShallowConditional<
                std::is_integral<ElementType>::value && (sizeof(ElementType) == sizeof(std::uint8_t))
            >::template Type<
                HasRawDataTag,
                HasFieldsTag
            >;

        return recalcLen(Tag());
    }

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        return BaseImpl::readN(m_fixedSize, iter, len);
    }

    template <typename TIter>
    void readNoStatus(TIter& iter)
    {
        return BaseImpl::readNoStatusN(m_fixedSize, iter);
    }

    template <typename TIter>
    comms::ErrorStatus write(TIter& iter, std::size_t len) const
    {
        auto writeCount = std::min(BaseImpl::getValue().size(), m_fixedSize);
        auto es = BaseImpl::writeN(writeCount, iter, len);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        auto remCount = m_fixedSize - writeCount;
        if (remCount == 0) {
            return es;
        }

        auto dummyElem = ElementType();
        while (0 < remCount) {
            es = BaseImpl::writeElement(dummyElem, iter, len);
            if (es != ErrorStatus::Success) {
                break;
            }

            --remCount;
        }

        return es;
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) const
    {
        auto writeCount = std::min(BaseImpl::getValue().size(), m_fixedSize);
        BaseImpl::writeNoStatusN(writeCount, iter);

        auto remCount = m_fixedSize - writeCount;
        if (remCount == 0) {
            return;
        }

        auto dummyElem = ElementType();
        while (0 < remCount) {
            BaseImpl::writeElementNoStatus(dummyElem, iter);
            --remCount;
        }
    }

    bool valid() const
    {
        return BaseImpl::valid() && (BaseImpl::getValue().size() <= m_fixedSize);
    }

    bool refresh()
    {
        if (!BaseImpl::refresh()) {
            return false;
        }

        using Tag =
            typename comms::util::LazyShallowConditional<
                comms::util::detect::hasResizeFunc<ElementType>()
            >::template Type<
                HasResizeTag,
                NoResizeTag
            >;

        return doRefresh(Tag());
    }

private:
    template <typename... TParams>
    using HasRawDataTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using HasFieldsTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    using HasFixedLengthElemsTag = comms::details::tag::Tag3<>;

    template <typename... TParams>
    using HasVarLengthElemsTag = comms::details::tag::Tag4<>;

    template <typename... TParams>
    using HasResizeTag = comms::details::tag::Tag5<>;

    template <typename... TParams>
    using NoResizeTag = comms::details::tag::Tag6<>;

    template <typename... TParams>
    std::size_t recalcLen(HasFieldsTag<TParams...>) const
    {
        using Tag =
            typename comms::util::LazyShallowConditional<
                ElementType::minLength() == ElementType::maxLength()
            >::template Type<
                HasFixedLengthElemsTag,
                HasVarLengthElemsTag
            >;
        return recalcLen(Tag());
    }

    template <typename... TParams>
    std::size_t recalcLen(HasRawDataTag<TParams...>) const
    {
        return m_fixedSize;
    }

    template <typename... TParams>
    std::size_t recalcLen(HasFixedLengthElemsTag<TParams...>) const
    {
        return m_fixedSize * ElementType::minLength();
    }

    template <typename... TParams>
    std::size_t recalcLen(HasVarLengthElemsTag<TParams...>) const
    {
        std::size_t result = 0U;
        auto count = m_fixedSize;
        for (auto& elem : BaseImpl::getValue()) {
            if (count == 0U) {
                break;
            }

            result += BaseImpl::elementLength(elem);
            --count;
        }
        return result;
    }

    template <typename... TParams>
    bool doRefresh(HasResizeTag<TParams...>)
    {
        if (BaseImpl::getValue().size() == m_fixedSize) {
            return false;
        }

        BaseImpl::value().resize(m_fixedSize);
        return true;
    }

    template <typename... TParams>
    static constexpr bool doRefresh(NoResizeTag<TParams...>)
    {
        return false;
    }

    std::size_t m_fixedSize = 0;
};

template <std::size_t TSize, typename TBase>
class SequenceFixedSizeAdapter : public SequenceFixedSizeAdapterBase<TBase>
{
    using BaseImpl = SequenceFixedSizeAdapterBase<TBase>;

public:
    using ValueType = typename BaseImpl::ValueType;
    using ElementType = typename BaseImpl::ElementType;

    explicit SequenceFixedSizeAdapter()
      : BaseImpl(TSize)
    {
    }

    explicit SequenceFixedSizeAdapter(const ValueType& val)
      : BaseImpl(TSize, val)
    {
    }

    SequenceFixedSizeAdapter(ValueType&& val)
      : BaseImpl(TSize, std::move(val))
    {
    }

    SequenceFixedSizeAdapter(const SequenceFixedSizeAdapter&) = default;
    SequenceFixedSizeAdapter(SequenceFixedSizeAdapter&&) = default;
    SequenceFixedSizeAdapter& operator=(const SequenceFixedSizeAdapter&) = default;
    SequenceFixedSizeAdapter& operator=(SequenceFixedSizeAdapter&&) = default;

    static constexpr std::size_t minLength()
    {
        return BaseImpl::minLength() + BaseImpl::minElementLength() * TSize;
    }

    static constexpr std::size_t maxLength()
    {
        return BaseImpl::minLength() + BaseImpl::maxElementLength() * TSize;
    }
};

}  // namespace adapter

}  // namespace field

}  // namespace comms

