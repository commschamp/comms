//
// Copyright 2022 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of comms::MsgFactoryCreateFailureReason enum.

#pragma once

namespace comms
{

// @brief Reason for message creation failure
enum class MsgFactoryCreateFailureReason
{
    None, ///< No reason
    InvalidId, ///< Invalid message id
    AllocFailure, ///< Allocation of the object has failied
    NumOfValues ///< Number of available values, must be last
};

} // namespace comms

