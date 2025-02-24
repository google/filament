#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
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
from generators.base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class PnextChainExtractionGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        # List of Vulkan structures that a pnext chain extraction function will be generated for
        self.target_structs = [
            'VkPhysicalDeviceImageFormatInfo2'
            ]

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2023-2025 The Khronos Group Inc.
            * Copyright (c) 2023-2025 Valve Corporation
            * Copyright (c) 2023-2025 LunarG, Inc.
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
            ****************************************************************************/\n\n''')
        out.append('// NOLINTBEGIN\n\n') # Wrap for clang-tidy to ignore

        if self.filename == 'pnext_chain_extraction.h':
            out.append(self.generateHeader())
        elif self.filename == 'pnext_chain_extraction.cpp':
            out.append(self.generateSource())
        else:
            out.append(f'\nFile name {self.filename} has no code to generate\n')

        out.append('\n// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write("".join(out))

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once

            #include <cassert>
            #include <tuple>

            #include "vulkan/vulkan.h"

            namespace vvl {

            // Add element to the end of a pNext chain
            void* PnextChainAdd(void *chain, void *new_struct);

            // Remove last element from a pNext chain
            void PnextChainRemoveLast(void *chain);
            
            // Free dynamically allocated pnext chain structs
            void PnextChainFree(void *chain);       

            // Helper class relying on RAII to help with adding and removing an element from a pNext chain
            class PnextChainScopedAdd {
            public:
                PnextChainScopedAdd(void *chain, void *new_struct) : chain(chain) {
                    PnextChainAdd(chain, new_struct);
                }
                ~PnextChainScopedAdd() {
                    PnextChainRemoveLast(chain);
                }

            private:
                void *chain = nullptr;
            };
            ''')

        out.append('''
// clang-format off

// Utility to make a selective copy of a pNext chain.
// Structs listed in the returned tuple type are the one extending some reference Vulkan structs, like VkPhysicalDeviceImageFormatInfo2.
// The copied structs are the one mentioned in the returned tuple type and found in the pNext chain `in_pnext_chain`.
// In the returned tuple, each struct is NOT a deep copy of the corresponding struct in in_pnext_chain,
// so be mindful of pointers copies.
// The first element of the extracted pNext chain is returned by this function. It can be nullptr.
template <typename T>
void *PnextChainExtract(const void */*in_pnext_chain*/, T &/*out*/) { assert(false); return nullptr; }

// Hereinafter are the available PnextChainExtract functions.
// To add a new one, find scripts/generators/pnext_chain_extraction_generator.py
// and add your reference struct to the `target_structs` list at the beginning of the file.
''')
        # Declare functions
        for struct_name in self.target_structs:
            struct = self.vk.structs[struct_name]
            out.append(f'\nusing PnextChain{struct_name} = std::tuple<\n\t')
            out.append(',\n\t'.join(struct.extendedBy))
            out.append('>;\n')
            out.append('template <>\n')
            out.append(f'void *PnextChainExtract(const void *in_pnext_chain, PnextChain{struct_name} &out);\n\n')

        out.append('}\n')
        out.append('// clang-format on\n')
        return "".join(out)

    def generateSource(self):
        out = []
        out.append('''
            #include "pnext_chain_extraction.h"

            #include <vulkan/utility/vk_struct_helper.hpp>

            namespace vvl {

            void* PnextChainAdd(void *chain, void *new_struct) {
                assert(chain);
                assert(new_struct);
                void *chain_end = vku::FindLastStructInPNextChain(chain);
                auto *vk_base_struct = static_cast<VkBaseOutStructure*>(chain_end);
                assert(!vk_base_struct->pNext);
                vk_base_struct->pNext = static_cast<VkBaseOutStructure*>(new_struct);
                return new_struct;
            }

            void PnextChainRemoveLast(void *chain) {
                if (!chain) {
                    return;
                }
                auto *current = static_cast<VkBaseOutStructure *>(chain);
                auto *prev = current;
                while (current->pNext) {
                    prev = current;
                    current = static_cast<VkBaseOutStructure *>(current->pNext);
                }
                prev->pNext = nullptr;
            }

            void PnextChainFree(void *chain) {
                if (!chain) return;       
                auto header = reinterpret_cast<VkBaseOutStructure *>(chain);
                switch (header->sType) {
            ''')
        
        guard_helper = PlatformGuardHelper()
        for struct in [x for x in self.vk.structs.values() if x.extends]:
            out.extend(guard_helper.add_guard(struct.protect))
            out.append(f'case {struct.sType}:\n')
            out.append('PnextChainFree(header->pNext);\n')
            out.append('header->pNext = nullptr;\n')
            out.append(f'delete reinterpret_cast<const {struct.name} *>(header);\n')
            out.append('break;\n')
        out.extend(guard_helper.add_guard(None))
        out.append('default:assert(false);break;')
        out.append('    }')
        out.append('}\n')

        # Define functions
        for struct_name in self.target_structs:
            struct = self.vk.structs[struct_name]
            out.append('\ntemplate <>\n')
            out.append(f'void *PnextChainExtract(const void *in_pnext_chain, PnextChain{struct_name} &out) {{')

            out.append('''
                void *chain_begin = nullptr;
                void *chain_end = nullptr;
                ''')

            # Add extraction logic for each struct extending target struct
            for extending_struct in struct.extendedBy:
                out.append(f'''
                    if (auto *chain_struct = vku::FindStructInPNextChain<{extending_struct}>(in_pnext_chain)) {{
                        auto &out_chain_struct = std::get<{extending_struct}>(out);
                        out_chain_struct = *chain_struct;
                        out_chain_struct.pNext = nullptr;
                        if (!chain_begin) {{
                            chain_begin = &out_chain_struct;
                            chain_end = chain_begin;
                        }} else {{
                            chain_end = PnextChainAdd(chain_end, &out_chain_struct);
                        }}
                    }}\n''')
            out.append('\n\treturn chain_begin;\n}\n')

        out.append('\n}\n')
        return "".join(out)
