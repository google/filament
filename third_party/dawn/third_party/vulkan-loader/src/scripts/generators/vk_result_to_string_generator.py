#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2021 The Khronos Group Inc.
# Copyright (c) 2015-2021 Valve Corporation
# Copyright (c) 2015-2021 LunarG, Inc.
# Copyright (c) 2015-2021 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Author: Mark Lobodzinski <mark@lunarg.com>
# Author: Charles Giessen <charles@lunarg.com>

import os
from base_generator import BaseGenerator

class VkResultToStringGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []

        out.append(f'''#pragma once
// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications

/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

 */

#include <ostream>
#include <string>

#include <vulkan/vulkan.h>

''')

        self.OutputVkResultToStringHelper(out)
        out.append('\n')

        self.write(''.join(out))

    # Create a dispatch table from the corresponding table_type and append it to out
    def OutputVkResultToStringHelper(self, out: list):
        out.append('inline std::ostream& operator<<(std::ostream& os, const VkResult& result) {\n')
        out.append('   switch (result) {\n')
        for field in self.vk.enums['VkResult'].fields:
            out.append(f'        case({field.name}):\n')
            out.append(f'            return os << "{field.name}";\n')
        out.append('        default:\n')
        out.append('           return os << static_cast<int32_t>(result);\n')
        out.append('       }\n')
        out.append('   }\n')
