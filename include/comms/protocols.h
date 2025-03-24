//
// Copyright 2014 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief This file provides all the definitions from comms::protocol namespace.

#pragma once

#include "comms/protocol/MsgDataLayer.h"
#include "comms/protocol/MsgIdLayer.h"
#include "comms/protocol/MsgSizeLayer.h"
#include "comms/protocol/SyncPrefixLayer.h"
#include "comms/protocol/ChecksumLayer.h"
#include "comms/protocol/ChecksumPrefixLayer.h"
#include "comms/protocol/TransportValueLayer.h"

#include "comms/protocol/checksum/BasicSum.h"
#include "comms/protocol/checksum/BasicXor.h"
#include "comms/protocol/checksum/Crc.h"
