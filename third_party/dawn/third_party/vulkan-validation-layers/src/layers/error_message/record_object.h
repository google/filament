/* Copyright (c) 2023-2024 LunarG, Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "chassis/chassis_handle_data.h"
#include "generated/error_location_helper.h"

// Contains all information for each PreCallRecord / PostCallRecord function
struct RecordObject {
    const Location location; // starting location (Always the function entrypoint)

    // not const as we will want to update these after the command is ran, this struct should passed around as a const ref anyway
    VkResult result = VK_RESULT_MAX_ENUM;  // Not all items return a VkResult
    VkDeviceAddress device_address = 0;

    const chassis::HandleData* handle_data;

    RecordObject(vvl::Func command_, const chassis::HandleData* handle_data_ = nullptr)
        : location(Location(command_)), handle_data(handle_data_) {}
    RecordObject(vvl::Func command_, VkResult result_, const chassis::HandleData* handle_data_ = nullptr)
        : location(Location(command_)), result(result_), handle_data(handle_data_) {}
    RecordObject(vvl::Func command_, VkDeviceAddress device_address_, const chassis::HandleData* handle_data_ = nullptr)
        : location(Location(command_)), device_address(device_address_), handle_data(handle_data_) {}

    bool HasResult() { return result != VK_RESULT_MAX_ENUM; }
};
