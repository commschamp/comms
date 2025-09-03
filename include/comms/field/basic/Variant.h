//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/CompileControl.h"
#include "comms/details/tag.h"
#include "comms/ErrorStatus.h"
#include "comms/field/basic/CommonFuncs.h"
#include "comms/field/details/FieldOpHelpers.h"
#include "comms/field/details/MembersVersionDependency.h"
#include "comms/field/details/VersionStorage.h"
#include "comms/field/tag.h"
#include "comms/util/Tuple.h"
#include "comms/util/type_traits.h"

#include <algorithm>
#include <type_traits>
#include <utility>

COMMS_MSVC_WARNING_PUSH
COMMS_MSVC_WARNING_DISABLE(4324) // Disable warning about alignment padding

namespace comms
{

namespace field
{

namespace basic
{

namespace details
{

template<typename...>
class VariantFieldConstructHelper
{
public:
    VariantFieldConstructHelper(void* storage) : m_storage(storage) {}

    template <std::size_t TIdx, typename TField>
    void operator()() const
    {
        new (m_storage) TField;
    }
private:
    void* m_storage = nullptr;
};

template<typename...>
class VariantLengthCalcHelper
{
public:
    VariantLengthCalcHelper(std::size_t& len, const void* storage) : 
        m_len(len),
        m_storage(storage)
    {
    }

    template <std::size_t TIdx, typename TField>
    void operator()()
    {
        m_len = reinterpret_cast<const TField*>(m_storage)->length();
    }

private:
    std::size_t& m_len;
    const void* m_storage;
};

template<typename...>
class VariantFieldCopyConstructHelper
{
public:
    VariantFieldCopyConstructHelper(void* storage, const void* other) : m_storage(storage), m_other(other) {}

    template <std::size_t TIdx, typename TField>
    void operator()() const
    {
        new (m_storage) TField(*(reinterpret_cast<const TField*>(m_other)));
    }

private:
    void* m_storage = nullptr;
    const void* m_other = nullptr;
};

template<typename...>
class VariantFieldMoveConstructHelper
{
public:
    VariantFieldMoveConstructHelper(void* storage, void* other) : m_storage(storage), m_other(other) {}

    template <std::size_t TIdx, typename TField>
    void operator()() const
    {
        new (m_storage) TField(std::move(*(reinterpret_cast<const TField*>(m_other))));
    }

private:
    void* m_storage = nullptr;
    void* m_other = nullptr;
};

template<typename...>
class VariantFieldDestructHelper
{
public:
    VariantFieldDestructHelper(void* storage) : m_storage(storage) {}

    template <std::size_t TIdx, typename TField>
    void operator()() const
    {
        reinterpret_cast<TField*>(m_storage)->~TField();
    }
private:
    void* m_storage = nullptr;
};

template <typename...>
class VariantFieldValidCheckHelper
{
public:
    VariantFieldValidCheckHelper(bool& result, const void* storage)
        : m_result(result),
        m_storage(storage)
    {
    }

    template <std::size_t TIdx, typename TField>
    void operator()()
    {
        m_result = reinterpret_cast<const TField*>(m_storage)->valid();
    }

private:
    bool& m_result;
    const void* m_storage;
};

template <typename...>
class VariantFieldRefreshHelper
{
public:
    VariantFieldRefreshHelper(bool& result, void* storage)
        : m_result(result),
        m_storage(storage)
    {
    }

    template <std::size_t TIdx, typename TField>
    void operator()()
    {
        m_result = reinterpret_cast<TField*>(m_storage)->refresh();
    }

private:
    bool& m_result;
    void* m_storage = nullptr;
};

template <typename TFunc>
class VariantExecHelper
{
    static_assert(std::is_lvalue_reference<TFunc>::value || std::is_rvalue_reference<TFunc>::value,
        "Wrong type of template parameter");
public:
    template <typename U>
    VariantExecHelper(void* storage, U&& func) : m_storage(storage), m_func(std::forward<U>(func)) {}

    template <std::size_t TIdx, typename TField>
    void operator()()
    {
#if COMMS_IS_MSVC
        // VS compiler
        m_func.operator()<TIdx>(*(reinterpret_cast<TField*>(m_storage)));
#else // #if COMMS_IS_MSVC
        m_func.template operator()<TIdx>(*(reinterpret_cast<TField*>(m_storage)));
#endif // #if COMMS_IS_MSVC
    }
private:
    void* m_storage = nullptr;
    TFunc m_func;
};

template <typename TFunc>
class VariantConstExecHelper
{
    static_assert(std::is_lvalue_reference<TFunc>::value || std::is_rvalue_reference<TFunc>::value,
        "Wrong type of template parameter");
public:
    template <typename U>
    VariantConstExecHelper(const void* storage, U&& func) : m_storage(storage), m_func(std::forward<U>(func)) {}

    template <std::size_t TIdx, typename TField>
    void operator()()
    {
#if COMMS_IS_MSVC
        // VS compiler
        m_func.operator()<TIdx>(*(reinterpret_cast<const TField*>(m_storage)));
#else // #if COMMS_IS_MSVC
        m_func.template operator()<TIdx>(*(reinterpret_cast<const TField*>(m_storage)));
#endif // #if COMMS_IS_MSVC
    }
private:
    const void* m_storage = nullptr;
    TFunc m_func;
};


template <typename TIter, typename TVerBase, bool TVerDependent>
class VariantReadHelper
{
    template <typename... TParams>
    using VersionDependentTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using NoVersionDependencyTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    using VersionTag =
        typename comms::util::LazyShallowConditional<
            TVerDependent
        >::template Type<
            VersionDependentTag,
            NoVersionDependencyTag
        >;    
public:
    VariantReadHelper(
        std::size_t& idx,
        comms::ErrorStatus& es,
        TIter& iter,
        std::size_t len,
        void* storage,
        TVerBase& verBase)
        : m_idx(idx),
          m_es(es),
          m_iter(iter),
          m_len(len),
          m_storage(storage),
          m_verBase(verBase)
    {
        using IterType = typename std::decay<decltype(iter)>::type;
        using IterCategory = typename std::iterator_traits<IterType>::iterator_category;
        static_assert(std::is_base_of<std::random_access_iterator_tag, IterCategory>::value,
            "Variant field only supports read with random access iterators");

        m_es = comms::ErrorStatus::NumOfErrorStatuses;
    }

    template <typename TField>
    void operator()()
    {
        if (m_readComplete) {
            return;
        }

        auto* field = new (m_storage) TField;
        updateMemberVersionInternal(*field, VersionTag<>());

        auto iterTmp = m_iter;
        auto es = field->read(iterTmp, m_len);
        if (es == comms::ErrorStatus::Success) {
            m_iter = iterTmp;
            m_es = es;
            m_readComplete = true;
            return;
        }

        field->~TField();

        if ((m_es == comms::ErrorStatus::NumOfErrorStatuses) ||
            (es == comms::ErrorStatus::NotEnoughData)) {
            m_es = es;
        }

        ++m_idx;
    }

private:
    template <typename TField, typename... TParams>
    void updateMemberVersionInternal(TField& field, NoVersionDependencyTag<TParams...>)
    {
        static_cast<void>(field);
    }

    template <typename TField, typename... TParams>
    void updateMemberVersionInternal(TField& field, VersionDependentTag<TParams...>)
    {
        field.setVersion(m_verBase.getVersion());
    }

    std::size_t& m_idx;
    comms::ErrorStatus& m_es;
    TIter& m_iter;
    std::size_t m_len = 0;
    void* m_storage = nullptr;
    TVerBase& m_verBase;
    bool m_readComplete = false;
};

template <typename TIter>
class VariantFieldWriteHelper
{
public:
    VariantFieldWriteHelper(ErrorStatus& es, TIter& iter, std::size_t len, const void* storage)
      : m_es(es),
        m_iter(iter),
        m_len(len),
        m_storage(storage)
    {
    }

    template <std::size_t TIdx, typename TField>
    void operator()()
    {
        m_es = reinterpret_cast<const TField*>(m_storage)->write(m_iter, m_len);
    }

private:
    ErrorStatus& m_es;
    TIter& m_iter;
    std::size_t m_len = 0U;
    const void* m_storage = nullptr;
};

template <typename TIter>
class VariantWriteNoStatusHelper
{
public:
    VariantWriteNoStatusHelper(TIter& iter, const void* storage)
      : m_iter(iter),
        m_storage(storage)
    {
    }

    template <std::size_t TIdx, typename TField>
    void operator()()
    {
        reinterpret_cast<const TField*>(m_storage)->writeNoStatus(m_iter);
    }

private:
    TIter& m_iter;
    const void* m_storage = nullptr;
};

template <typename TVersionType>
class VariantSetVersionHelper
{
public:
    VariantSetVersionHelper(TVersionType version, bool& updated, void* storage) :
        m_version(version), 
        m_updated(updated), 
        m_storage(storage)
    {
    }

    template <std::size_t TIdx, typename TField>
    void operator()()
    {
        m_updated = reinterpret_cast<TField*>(m_storage)->setVersion(m_version) || m_updated;
    }

private:
    TVersionType m_version = TVersionType();
    bool& m_updated;
    void* m_storage = nullptr;
};

template<typename...>
class VariantCanWriteHelper
{
public:
    VariantCanWriteHelper(bool& result, const void* storage)
        : m_result(result),
        m_storage(storage)
    {
    }

    template <std::size_t TIdx, typename TField>
    void operator()()
    {
        m_result = reinterpret_cast<const TField*>(m_storage)->canWrite();
    }

private:
    bool& m_result;
    const void* m_storage;
};

template <typename TFieldBase, comms::field::details::MembersVersionDependency TVersionDependency, typename... TMembers>
struct VariantVersionStorageBaseHelper;

template <typename TFieldBase, typename... TMembers>
struct VariantVersionStorageBaseHelper<TFieldBase, comms::field::details::MembersVersionDependency_NotSpecified, TMembers...>
{
    using Type = 
    typename comms::util::LazyShallowConditional<
        CommonFuncs::IsAnyFieldVersionDependentBoolType<TMembers...>::value
    >::template Type<
        comms::field::details::VersionStorage,
        comms::util::EmptyStruct,
        typename TFieldBase::VersionType
    >;    
};

template <typename TFieldBase, typename... TMembers>
struct VariantVersionStorageBaseHelper<TFieldBase, comms::field::details::MembersVersionDependency_Independent, TMembers...>
{
    using Type = comms::util::EmptyStruct<>;
};

template <typename TFieldBase, typename... TMembers>
struct VariantVersionStorageBaseHelper<TFieldBase, comms::field::details::MembersVersionDependency_Dependent, TMembers...>
{
    using Type = comms::field::details::VersionStorage<typename TFieldBase::VersionType>;
};

template <typename TFieldBase, comms::field::details::MembersVersionDependency TVersionDependency, typename... TMembers>
using VariantVersionStorageBase = 
    typename VariantVersionStorageBaseHelper<TFieldBase, TVersionDependency, TMembers...>::Type;

template <comms::field::details::MembersVersionDependency TVersionDependency, typename... TMembers>
struct VariantVersionDependencyDetectHelper;

template <typename... TMembers>
struct VariantVersionDependencyDetectHelper<comms::field::details::MembersVersionDependency_NotSpecified, TMembers...>
{
    static constexpr bool Value = CommonFuncs::IsAnyFieldVersionDependentBoolType<TMembers...>::value;
};

template <typename... TMembers>
struct VariantVersionDependencyDetectHelper<comms::field::details::MembersVersionDependency_Independent, TMembers...>
{
    static constexpr bool Value = false;
};

template <typename... TMembers>
struct VariantVersionDependencyDetectHelper<comms::field::details::MembersVersionDependency_Dependent, TMembers...>
{
    static constexpr bool Value = true;
};


} // namespace details

template <typename TFieldBase, comms::field::details::MembersVersionDependency TVersionDependency, typename TMembers>
class Variant;

template <typename TFieldBase, comms::field::details::MembersVersionDependency TVersionDependency, typename... TMembers>
class Variant<TFieldBase, TVersionDependency, std::tuple<TMembers...> > :
        public TFieldBase,
        public details::VariantVersionStorageBase<TFieldBase, TVersionDependency, TMembers...>
{
    using BaseImpl = TFieldBase;
    using VersionBaseImpl = details::VariantVersionStorageBase<TFieldBase, TVersionDependency, TMembers...>;

public:
    using Members = std::tuple<TMembers...>;
    using ValueType = comms::util::TupleAsAlignedUnionT<Members>;
    using VersionType = typename BaseImpl::VersionType;
    using CommsTag = comms::field::tag::Variant;

    static const std::size_t MembersCount = std::tuple_size<Members>::value;
    static_assert(0U < MembersCount, "ValueType must be non-empty tuple");

    Variant() = default;
    Variant(const ValueType& val)  : m_storage(val) {}
    Variant(ValueType&& val)  : m_storage(std::move(val)) {}

    Variant(const Variant& other)
    {
        if (!other.currentFieldValid()) {
            return;
        }

        comms::util::tupleForSelectedType<Members>(
            other.m_memIdx, details::VariantFieldCopyConstructHelper<>(&m_storage, &other.m_storage));

        m_memIdx = other.m_memIdx;
    }

    Variant(Variant&& other)
    {
        if (!other.currentFieldValid()) {
            return;
        }

        comms::util::tupleForSelectedType<Members>(
            other.m_memIdx, details::VariantFieldMoveConstructHelper<>(&m_storage, &other.m_storage));

        m_memIdx = other.m_memIdx;
    }

    ~Variant() noexcept
    {
        COMMS_ASSERT(!currentFieldValid());
    }

    Variant& operator=(const Variant& other)
    {
        if (this == &other) {
            return *this;
        }

        checkDestruct();
        if (!other.currentFieldValid()) {
            return *this;
        }

        comms::util::tupleForSelectedType<Members>(
            other.m_memIdx, details::VariantFieldCopyConstructHelper<>(&m_storage, &other.m_storage));

        m_memIdx = other.m_memIdx;
        return *this;
    }

    Variant& operator=(Variant&& other)
    {
        if (this == &other) {
            return *this;
        }

        checkDestruct();

        if (!other.currentFieldValid()) {
            return *this;
        }

        comms::util::tupleForSelectedType<Members>(
            other.m_memIdx, details::VariantFieldMoveConstructHelper<>(&m_storage, &other.m_storage));

        m_memIdx = other.m_memIdx;
        return *this;
    }

    const ValueType& value() const
    {
        return m_storage;
    }

    ValueType& value()
    {
        return m_storage;
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

    std::size_t length() const
    {
        if (!currentFieldValid()) {
            return 0U;
        }

        std::size_t len = std::numeric_limits<std::size_t>::max();
        comms::util::tupleForSelectedType<Members>(m_memIdx, details::VariantLengthCalcHelper<>(len, &m_storage));
        return len;
    }

    static constexpr std::size_t minLength()
    {
        return 0U;
    }

    static constexpr std::size_t maxLength()
    {
        return CommonFuncs::FieldSelectMaxLengthIntType<TMembers...>::value;
    }

    bool valid() const
    {
        if (!currentFieldValid()) {
            return false;
        }

        bool val = false;
        comms::util::tupleForSelectedType<Members>(
            m_memIdx, details::VariantFieldValidCheckHelper<>(val, &m_storage));
        return val;
    }

    static constexpr bool hasNonDefaultRefresh()
    {
        return CommonFuncs::AnyFieldHasNonDefaultRefreshBoolType<TMembers...>::value;
    }

    bool refresh()
    {
        if (!currentFieldValid()) {
            return false;
        }

        bool val = false;
        comms::util::tupleForSelectedType<Members>(
            m_memIdx, details::VariantFieldRefreshHelper<>(val, &m_storage));
        return val;
    }

    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t len)
    {
        checkDestruct();
        auto es = comms::ErrorStatus::NumOfErrorStatuses;
        comms::util::tupleForEachType<Members>(makeReadHelper(es, iter, len, &m_storage, static_cast<VersionBaseImpl&>(*this)));
        COMMS_ASSERT((es == comms::ErrorStatus::Success) || (MembersCount <= m_memIdx));
        COMMS_ASSERT((es != comms::ErrorStatus::Success) || (m_memIdx < MembersCount));

        return es;
    }

    static constexpr bool hasReadNoStatus()
    {
        return false;
    }

    template <typename TIter>
    void readNoStatus(TIter& iter) = delete;

    bool canWrite() const
    {
        if (!currentFieldValid()) {
            return true;
        }

        bool val = false;
        comms::util::tupleForSelectedType<Members>(
            m_memIdx, details::VariantCanWriteHelper<>(val, &m_storage));
        return val;
    }

    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t len) const
    {
        if (!currentFieldValid()) {
            return comms::ErrorStatus::Success;
        }

        auto es = ErrorStatus::NumOfErrorStatuses;
        comms::util::tupleForSelectedType<Members>(m_memIdx, makeWriteHelper(es, iter, len, &m_storage));
        return es;
    }

    static constexpr bool hasWriteNoStatus()
    {
        return comms::util::tupleTypeAccumulate<Members>(
            true, comms::field::details::FieldHasWriteNoStatusHelper<>());
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) const
    {
        if (!currentFieldValid()) {
            return;
        }

        comms::util::tupleForSelectedType<Members>(m_memIdx, makeWriteNoStatusHelper(iter, &m_storage));
    }


    std::size_t currentField() const
    {
        return m_memIdx;
    }

    void selectField(std::size_t idx)
    {
        if (idx == m_memIdx) {
            return;
        }

        checkDestruct();
        if (!isIdxValid(idx)) {
            return;
        }

        comms::util::tupleForSelectedType<Members>(
            idx, details::VariantFieldConstructHelper<>(&m_storage));
        m_memIdx = idx;
    }

    template <typename TFunc>
    void currentFieldExec(TFunc&& func)
    {
        if (!currentFieldValid()) {
            static constexpr bool Invalid_field_execution = false;
            static_cast<void>(Invalid_field_execution);
            COMMS_ASSERT(Invalid_field_execution);
            return;
        }

        comms::util::tupleForSelectedType<Members>(m_memIdx, makeExecHelper(std::forward<TFunc>(func)));
    }

    template <typename TFunc>
    void currentFieldExec(TFunc&& func) const
    {
        if (!currentFieldValid()) {
            static constexpr bool Invalid_field_execution = false;
            static_cast<void>(Invalid_field_execution);
            COMMS_ASSERT(Invalid_field_execution);
            return;
        }

        comms::util::tupleForSelectedType<Members>(m_memIdx, makeConstExecHelper(std::forward<TFunc>(func)));
    }

    template <std::size_t TIdx, typename... TArgs>
    typename std::tuple_element<TIdx, Members>::type& initField(TArgs&&... args)
    {
        static_assert(isIdxValid(TIdx), "Only valid field index can be used");
        COMMS_ASSERT(!currentFieldValid());

        using FieldType = typename std::tuple_element<TIdx, Members>::type;
        new (&m_storage) FieldType(std::forward<TArgs>(args)...);
        m_memIdx = TIdx;
        updateVersionInternal(VersionTag<>());
        return reinterpret_cast<FieldType&>(m_storage);
    }

    template <std::size_t TIdx>
    void deinitField()
    {
        static_assert(isIdxValid(TIdx), "Only valid field index can be used");
        COMMS_ASSERT(m_memIdx == TIdx);

        using FieldType = typename std::tuple_element<TIdx, Members>::type;
        reinterpret_cast<FieldType*>(&m_storage)->~FieldType();
        m_memIdx = MembersCount;
    }    

    template <std::size_t TIdx>
    typename std::tuple_element<TIdx, Members>::type& accessField()
    {
        static_assert(isIdxValid(TIdx), "Only valid field index can be used");
        COMMS_ASSERT(TIdx == m_memIdx); // Accessing non initialised field

        using FieldType = typename std::tuple_element<TIdx, Members>::type;
        return reinterpret_cast<FieldType&>(m_storage);
    }

    template <std::size_t TIdx>
    const typename std::tuple_element<TIdx, Members>::type& accessField() const
    {
        static_assert(isIdxValid(TIdx), "Something is wrong");
        COMMS_ASSERT(TIdx == m_memIdx); // Accessing non initialised field

        using FieldType = typename std::tuple_element<TIdx, Members>::type;
        return reinterpret_cast<const FieldType&>(m_storage);
    }

    bool currentFieldValid() const
    {
        return isIdxValid(m_memIdx);
    }

    void reset()
    {
        checkDestruct();
        COMMS_ASSERT(!currentFieldValid());
    }

    static constexpr bool isVersionDependent()
    {
        return details::VariantVersionDependencyDetectHelper<TVersionDependency, TMembers...>::Value;
    }

    bool setVersion(VersionType version)
    {
        return setVersionInternal(version, VersionTag<>());
    }

    VersionType getVersion() const
    {
        return getVersionInternal(VersionTag<>());
    }

private:
    template <typename... TParams>
    using VersionDependentTag = comms::details::tag::Tag1<>;

    template <typename... TParams>
    using NoVersionDependencyTag = comms::details::tag::Tag2<>;

    template <typename... TParams>
    using ForcedVersionDependencyTag = comms::details::tag::Tag3<>;

    template <typename... TParams>
    using NoForcedVersionDependencyTag = comms::details::tag::Tag4<>;    

    template <typename... TParams>
    using VersionTag =
        typename comms::util::LazyShallowConditional<
            details::VariantVersionDependencyDetectHelper<TVersionDependency, TMembers...>::Value
        >::template Type<
            VersionDependentTag,
            NoVersionDependencyTag
        >;

    template <typename TFunc>
    auto makeExecHelper(TFunc&& func) -> details::VariantExecHelper<decltype(std::forward<TFunc>(func))>
    {
        using FuncType = decltype(std::forward<TFunc>(func));
        return details::VariantExecHelper<FuncType>(&m_storage, std::forward<TFunc>(func));
    }

    template <typename TFunc>
    auto makeConstExecHelper(TFunc&& func) const -> details::VariantConstExecHelper<decltype(std::forward<TFunc>(func))>
    {
        using FuncType = decltype(std::forward<TFunc>(func));
        return details::VariantConstExecHelper<FuncType>(&m_storage, std::forward<TFunc>(func));
    }

    template <typename TIter, typename TVerBase>
    details::VariantReadHelper<TIter, TVerBase, details::VariantVersionDependencyDetectHelper<TVersionDependency, TMembers...>::Value> 
    makeReadHelper(
        comms::ErrorStatus& es,
        TIter& iter,
        std::size_t len,
        void* storage,
        TVerBase& verBase)
    {
        m_memIdx = 0;
        static constexpr bool VerDependent = isVersionDependent();
        return 
            details::VariantReadHelper<TIter, TVerBase, VerDependent>(
                m_memIdx, es, iter, len, storage, verBase);
    }

    template <typename TIter>
    static details::VariantFieldWriteHelper<TIter> makeWriteHelper(comms::ErrorStatus& es, TIter& iter, std::size_t len, const void* storage)
    {
        return details::VariantFieldWriteHelper<TIter>(es, iter, len, storage);
    }

    template <typename TIter>
    static details::VariantWriteNoStatusHelper<TIter> makeWriteNoStatusHelper(TIter& iter, const void* storage)
    {
        return details::VariantWriteNoStatusHelper<TIter>(iter, storage);
    }

    void checkDestruct()
    {
        if (currentFieldValid()) {
            comms::util::tupleForSelectedType<Members>(
                m_memIdx, details::VariantFieldDestructHelper<>(&m_storage));
            m_memIdx = MembersCount;
        }
    }

    static constexpr bool isIdxValid(std::size_t idx)
    {
        return idx < MembersCount;
    }

    template <typename... TParams>
    bool setVersionInternal(VersionType version, NoVersionDependencyTag<TParams...>)
    {
        static_cast<void>(version);
        return false;
    }

    template <typename... TParams>
    bool setVersionInternal(VersionType version, VersionDependentTag<TParams...>)
    {
        VersionBaseImpl::m_version = version;
        bool updated = false;
        if (currentFieldValid()) {
            comms::util::tupleForSelectedType<Members>(
                m_memIdx, details::VariantSetVersionHelper<VersionType>(version, updated, &m_storage));
        }
        return updated;
    }


    template <typename... TParams>
    VersionType getVersionInternal(VersionDependentTag<TParams...>) const
    {
        return VersionBaseImpl::m_version;;
    }

    template <typename... TParams>
    void updateVersionInternal(NoVersionDependencyTag<TParams...>)
    {
    }

    template <typename... TParams>
    void updateVersionInternal(VersionDependentTag<TParams...>)
    {
        setVersion(VersionBaseImpl::m_version);
    }

    alignas(8) ValueType m_storage;
    std::size_t m_memIdx = MembersCount;
};

}  // namespace basic

}  // namespace field

}  // namespace comms

COMMS_MSVC_WARNING_POP
