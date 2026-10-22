/*
 * Copyright (c) 2022 The Khronos Group Inc.
 * Copyright (c) 2022 Valve Corporation
 * Copyright (c) 2022 LunarG, Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Young <marky@lunarg.com>
 * Author: Lenny Komow <lenny@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

#pragma once

#include "loader_common.h"

void loader_init_dispatch_dev_ext(struct loader_instance *inst, struct loader_device *dev);
void *loader_dev_ext_gpa_tramp(struct loader_instance *inst, const char *funcName);
void *loader_dev_ext_gpa_term(struct loader_instance *inst, const char *funcName);

void *loader_phys_dev_ext_gpa_tramp(struct loader_instance *inst, const char *funcName);
void *loader_phys_dev_ext_gpa_term(struct loader_instance *inst, const char *funcName);

void loader_free_dev_ext_table(struct loader_instance *inst);
void loader_free_phys_dev_ext_table(struct loader_instance *inst);
