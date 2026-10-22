/*
 *
 * Copyright (c) 2021-2022 The Khronos Group Inc.
 * Copyright (c) 2021-2022 Valve Corporation
 * Copyright (c) 2021-2022 LunarG, Inc.
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
 *
 * Author: Mark Young <marky@lunarg.com>
 *
 */

#pragma once

#if defined(LOADER_ENABLE_LINUX_SORT)

#include "loader_common.h"

// This function allocates an array in sorted_devices which must be freed by the caller if not null
VkResult linux_read_sorted_physical_devices(struct loader_instance *inst, uint32_t icd_count,
                                            struct loader_icd_physical_devices *icd_devices, uint32_t phys_dev_count,
                                            struct loader_physical_device_term **sorted_device_term);

// This function sorts an array in physical device groups
VkResult linux_sort_physical_device_groups(struct loader_instance *inst, uint32_t group_count,
                                           struct loader_physical_device_group_term *sorted_group_term);

#endif  // LOADER_ENABLE_LINUX_SORT
