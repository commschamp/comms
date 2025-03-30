//
// Copyright 2015 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/Assert.h"
#include "comms/ErrorStatus.h"
#include "comms/Field.h"
#include "comms/field/OptionalMode.h"
#include "comms/field/tag.h"

namespace comms
{

namespace field
{

namespace basic
{

template <typename TField>
class Optional : public
        comms::Field<
            comms::option::def::Endian<typename TField::Endian>,
            comms::option::def::VersionType<typename TField::VersionType>
        >
{
    using BaseImpl =
        comms::Field<
            comms::option::def::Endian<typename TField::Endian>,
            comms::option::def::VersionType<typename TField::VersionType>
        >;

public:

    using Field = TField;
    using ValueType = TField;
    using Mode = field::OptionalMode;
    using VersionType = typename BaseImpl::VersionType;
    using CommsTag = comms::field::tag::Optional;

    Optional() = default;

    explicit Optional(const Field& fieldSrc, Mode mode = Mode::Tentative)
      : m_field(fieldSrc),
        m_mode(mode)
    {
    }

    explicit Optional(Field&& fieldSrc, Mode mode = Mode::Tentative)
      : m_field(std::move(fieldSrc)),
        m_mode(mode)
    {
    }

    Optional(const Optional&) = default;

    Optional(Optional&&) = default;

    ~Optional() noexcept = default;

    Optional& operator=(const Optional&) = default;

    Optional& operator=(Optional&&) = default;

    Field& field()
    {
        return m_field;
    }

    const Field& field() const
    {
        return m_field;
    }

    ValueType& value()
    {
        return field();
    }

    const ValueType& value() const
    {
        return field();
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

    Mode getMode() const
    {
        return m_mode;
    }

    void setMode(Mode val)
    {
        COMMS_ASSERT(val < Mode::NumOfModes);
        m_mode = val;
    }

    std::size_t length() const
    {
        if (m_mode != Mode::Exists) {
            return 0U;
        }

        return m_field.length();
    }

    static constexpr std::size_t minLength()
    {
        return 0U;
    }

    static constexpr std::size_t maxLength()
    {
        return Field::maxLength();
    }

    bool valid() const
    {
        if (m_mode == Mode::Missing) {
            return true;
        }

        return m_field.valid();
    }

    bool refresh() {
        if (m_mode != Mode::Exists) {
            return false;
        }
        return m_field.refresh();
    }

    template <typename TIter>
    ErrorStatus read(TIter& iter, std::size_t len)
    {
        if (m_mode == Mode::Missing) {
            return comms::ErrorStatus::Success;
        }

        if ((m_mode == Mode::Tentative) && (0U == len)) {
            m_mode = Mode::Missing;
            return comms::ErrorStatus::Success;
        }

        auto es = m_field.read(iter, len);
        if (es == comms::ErrorStatus::Success) {
            m_mode = Mode::Exists;
        }
        return es;
    }

    static constexpr bool hasReadNoStatus()
    {
        return false;
    }

    bool canWrite() const
    {
        if (m_mode != Mode::Exists) {
            return true;
        }

        return m_field.canWrite();
    }

    template <typename TIter>
    ErrorStatus write(TIter& iter, std::size_t len) const
    {
        if (m_mode != Mode::Exists) {
            return comms::ErrorStatus::Success;
        }

        return m_field.write(iter, len);
    }

    static constexpr bool hasWriteNoStatus()
    {
        return Field::hasWriteNoStatus();
    }

    template <typename TIter>
    void writeNoStatus(TIter& iter) const
    {
        if (m_mode != Mode::Exists) {
            return;
        }

        m_field.writeNoStatus(iter);
    }

    static constexpr bool isVersionDependent()
    {
        return BaseImpl::isVersionDependent() || Field::isVersionDependent();
    }

    static constexpr bool hasNonDefaultRefresh()
    {
        return BaseImpl::hasNonDefaultRefresh() || Field::hasNonDefaultRefresh();
    }

    bool setVersion(VersionType version)
    {
        return m_field.setVersion(static_cast<typename Field::VersionType>(version));
    }

private:
    Field m_field;
    Mode m_mode = Mode::Tentative;
};

}  // namespace basic

}  // namespace field

}  // namespace comms


