#!/usr/bin/python3 -i
#
# Copyright (c) 2025 The Khronos Group Inc.
# Copyright (c) 2025 Valve Corporation
# Copyright (c) 2025 LunarG, Inc.
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

import os
from collections import defaultdict
from generators.base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class ValidFlagValuesOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)
        self.ignoreList = [
            'VkInstanceCreateFlagBits', # handled else where

            # TODO - will cause many tests to fail if any codec extension is not enabled
            'VkVideoCodecOperationFlagBitsKHR',
        ]

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
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
            ****************************************************************************/\n''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'valid_flag_values.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateSource(self):
        guard_helper = PlatformGuardHelper()
        out = []
        out.append('''
            #include "stateless/stateless_validation.h"
            #include <vulkan/vk_enum_string_helper.h>

            // For flags, we can't use the VkFlag as it can't be templated (since it all resolves to a int).
            // It is simpler for the caller to already check for both
            //    - if zero is valid value or not
            //    - if the value is even found in the API
            // so the this file is only focused on checking for extensions being supported
            ''')

        out.append('''
            vvl::Extensions stateless::Context::IsValidFlagValue(vvl::FlagBitmask flag_bitmask, VkFlags value) const {
                switch(flag_bitmask) {
            ''')
        for bitmask in [x for x in self.vk.bitmasks.values() if x.name not in self.ignoreList and not x.returnedOnly and x.bitWidth == 32]:
            # Build up list of expressions so can check together
            expressionMap = defaultdict(list)
            for flag in [x for x in bitmask.flags if len(x.extensions) > 0]:
                # Ignore the base extensions needed to use the flag, only focus on the flag specific extensions
                extensions = [x.name for x in flag.extensions if x not in bitmask.extensions]
                if (len(extensions) == 0):
                    continue
                expression = ",".join(extensions)
                expressionMap[expression].append(flag.name)
            if (len(expressionMap) == 0):
                continue

            out.extend(guard_helper.add_guard(bitmask.protect))
            out.append(f'case vvl::FlagBitmask::{bitmask.name}:\n')

            for flag in [x for x in bitmask.flags if x.multiBit]:
                out.append(f'if (value == {flag.name}) {{ return {{}}; }}\n')

            for expression, names in expressionMap.items():
                extensions = expression.split(',')
                checkExpression = []
                for extension in extensions:
                    checkExpression.append(f'!IsExtEnabled(extensions.{extension.lower()})')
                checkExpression = " && ".join(checkExpression)
                resultExpression = []
                for extension in extensions:
                    resultExpression.append(f'vvl::Extension::_{extension}')
                resultExpression = ", ".join(resultExpression)

                out.append(f'if (value & ({" | ".join(names)})) {{\n')
                out.append(f'   if ({checkExpression}) {{\n')
                out.append(f'       return {{{resultExpression}}};\n')
                out.append('    }')
                out.append('}')

            out.append('   return {};\n')
            # out.append('\n')
        out.extend(guard_helper.add_guard(None))
        out.append('''default: return {};
                }
            }
            ''')

        out.append('''
            vvl::Extensions stateless::Context::IsValidFlag64Value(vvl::FlagBitmask flag_bitmask, VkFlags64 value) const {
                switch(flag_bitmask) {
            ''')
        for bitmask in [x for x in self.vk.bitmasks.values() if x.name not in self.ignoreList and not x.returnedOnly and x.bitWidth == 64]:
            # Build up list of expressions so can check together
            expressionMap = defaultdict(list)
            for flag in [x for x in bitmask.flags if len(x.extensions) > 0]:
                # Ignore the base extensions needed to use the flag, only focus on the flag specific extensions
                extensions = [x.name for x in flag.extensions if x not in bitmask.extensions]
                if (len(extensions) == 0):
                    continue
                expression = ",".join(extensions)
                expressionMap[expression].append(flag.name)
            if (len(expressionMap) == 0):
                continue

            out.extend(guard_helper.add_guard(bitmask.protect))
            out.append(f'case vvl::FlagBitmask::{bitmask.name}:\n')

            for flag in [x for x in bitmask.flags if x.multiBit]:
                out.append(f'if (value == {flag.name}) {{ return {{}}; }}\n')

            for expression, names in expressionMap.items():
                extensions = expression.split(',')
                checkExpression = []
                for extension in extensions:
                    checkExpression.append(f'!IsExtEnabled(extensions.{extension.lower()})')
                checkExpression = " && ".join(checkExpression)
                resultExpression = []
                for extension in extensions:
                    resultExpression.append(f'vvl::Extension::_{extension}')
                resultExpression = ", ".join(resultExpression)

                out.append(f'if (value & ({" | ".join(names)})) {{\n')
                out.append(f'   if ({checkExpression}) {{\n')
                out.append(f'       return {{{resultExpression}}};\n')
                out.append('    }')
                out.append('}')

            out.append('   return {};\n')
        out.extend(guard_helper.add_guard(None))
        out.append('''default: return {};
                }
            }

            std::string stateless::Context::DescribeFlagBitmaskValue(vvl::FlagBitmask flag_bitmask, VkFlags value) const {
                switch(flag_bitmask) {
            ''')
        for bitmask in [x for x in self.vk.bitmasks.values() if x.name not in self.ignoreList and not x.returnedOnly and len(x.flags) > 0 and x.bitWidth == 32]:
            out.extend(guard_helper.add_guard(bitmask.protect))
            out.append(f'case vvl::FlagBitmask::{bitmask.name}:\n')
            out.append(f'return string_{bitmask.flagName}(value);\n')
        out.extend(guard_helper.add_guard(None))
        out.append('''
                    default:
                        std::stringstream ss;
                        ss << "0x" << std::hex << value;
                        return ss.str();
                }
            }

            std::string stateless::Context::DescribeFlagBitmaskValue64(vvl::FlagBitmask flag_bitmask, VkFlags64 value) const {
                switch(flag_bitmask) {
            ''')
        for bitmask in [x for x in self.vk.bitmasks.values() if x.name not in self.ignoreList and not x.returnedOnly and len(x.flags) > 0 and x.bitWidth == 64]:
            out.extend(guard_helper.add_guard(bitmask.protect))
            out.append(f'case vvl::FlagBitmask::{bitmask.name}:\n')
            out.append(f'return string_{bitmask.flagName}(value);\n')
        out.extend(guard_helper.add_guard(None))
        out.append('''
                    default:
                        std::stringstream ss;
                        ss << "0x" << std::hex << value;
                        return ss.str();
                }
            }
        ''')
        self.write(''.join(out))
