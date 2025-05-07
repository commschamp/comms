//
// Copyright 2017 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace comms
{

namespace frame
{

namespace details
{

template <typename TBase>
class TransportValueLayerPseudoBase : public TBase
{
    using BaseImpl = TBase;
    using FieldImpl = typename BaseImpl::Field;
public:
    FieldImpl& pseudoField()
    {
        return m_pseudoField;
    }

    const FieldImpl& pseudoField() const
    {
        return m_pseudoField;
    }

private:
    FieldImpl m_pseudoField;
};

} // namespace details

} // namespace frame

} // namespace comms
