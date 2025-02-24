#!/usr/bin/python3 -i
#
# Copyright (c) 2021-2024 The Khronos Group Inc.
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

import sys
import os
import re
import json
from generators.base_generator import BaseGenerator
from generators.generator_utils import IsNonVulkanSprivCapability

#
# Generate SPIR-V grammar helper for SPIR-V opcodes, enums, etc
# Has zero relationship to the Vulkan API (doesn't use vk.xml)
class SpirvGrammarHelperOutputGenerator(BaseGenerator):
    def __init__(self,
                 grammar):
        BaseGenerator.__init__(self)

        self.opcodes = dict()
        self.opnames = []
        self.atomicsOps = []
        self.groupOps = []
        self.imageGatherOps = []
        self.imageSampleOps = []
        self.imageFetchOps = []
        self.typeOps = [] # OpType*
        self.storageClassList = [] # list of storage classes
        self.executionModelList = []
        self.executionModeList = []
        self.decorationList = []
        self.builtInList = []
        self.dimList = []
        self.cooperativeMatrixList = []
        self.hasType = []
        self.hasResult = []
        self.provisionalList = []
        # Need range to be large as largest possible operand index
        # This is done to make it easier to group switch case of same value
        self.imageOperandsParamCount = [[] for i in range(3)]
        self.memoryScopePosition = [[] for i in range(5)]
        self.executionScopePosition = [[] for i in range(4)]
        self.imageOperandsPosition = [[] for i in range(8)]
        self.imageAccessOperand = [[] for i in range(4)]

        self.kindId = [] # "category" : "Id"
        self.kindLiteral = [] # "category" : "Literal"
        self.kindComposite = [] # "category" : "Composite"
        self.kindValueEnum = [] # "category" : "ValueEnum"
        self.kindBitEnum = [] # "category" : "BitEnum"

        self.parseGrammar(grammar)

    def addToStringList(self, operandKind, kind, list, ignoreList = []):
        if operandKind['kind'] == kind:
            for enum in operandKind['enumerants']:
                if enum['enumerant'] not in ignoreList:
                    list.append(enum['enumerant'])

    #
    # Takes the SPIR-V Grammar JSON and parses it
    # Emulates the gen*() functions the vk.xml calls
    #
    # In the future, IF more then this generator wants to use the grammar
    # it would be better to move the file opening to run_generators.py
    def parseGrammar(self, grammar):
        with open(grammar, 'r') as jsonFile:
            data = json.load(jsonFile)
            instructions = data['instructions']
            operandKinds = data['operand_kinds']

            for operandKind in operandKinds:
                kind = operandKind['kind']
                category = operandKind['category']

                if category == 'Id':
                    self.kindId.append(kind)
                elif category == 'Literal':
                    self.kindLiteral.append(kind)
                elif category == 'Composite':
                    self.kindComposite.append(kind)
                elif category == 'ValueEnum':
                    self.kindValueEnum.append(kind)
                elif category == 'BitEnum':
                    self.kindBitEnum.append(kind)

                if kind == 'ImageOperands':
                    for enum in operandKind['enumerants']:
                        count = 0  if 'parameters' not in enum else len(enum['parameters'])
                        self.imageOperandsParamCount[count].append(enum['enumerant'])

                self.addToStringList(operandKind, 'StorageClass', self.storageClassList)
                self.addToStringList(operandKind, 'ExecutionModel', self.executionModelList)
                self.addToStringList(operandKind, 'ExecutionMode', self.executionModeList)
                self.addToStringList(operandKind, 'Decoration', self.decorationList)
                self.addToStringList(operandKind, 'BuiltIn', self.builtInList)
                self.addToStringList(operandKind, 'Dim', self.dimList)
                self.addToStringList(operandKind, 'CooperativeMatrixOperands', self.cooperativeMatrixList, ['NoneKHR'])

                if 'enumerants' in operandKind:
                    for enum in operandKind['enumerants']:
                        if 'provisional' in enum:
                            self.provisionalList.append(enum['enumerant'])

            for instruction in instructions:
                opname = instruction['opname']
                opcode = instruction['opcode']

                if 'provisional' in instruction:
                    self.provisionalList.append(opname)

                if 'capabilities' in instruction:
                    notSupported = True
                    for capability in instruction['capabilities']:
                        if not IsNonVulkanSprivCapability(capability):
                            notSupported = False
                            break
                    if notSupported:
                        continue # If just 'Kernel' capabilites then it's ment for OpenCL and skip instruction

                self.opnames.append(opname)

                self.opcodes[opcode] = {
                    'imageRefPosition' : 0,
                    'sampledImageRefPosition' : 0,

                    'opname' : opname,
                    'operands' : [],
                    'hasOptional' : False,
                    'hasVariableLength' : False,
                }

                if instruction['class'] == 'Atomic':
                    self.atomicsOps.append(opname)
                if instruction['class'] == 'Non-Uniform':
                    self.groupOps.append(opname)
                if re.search("OpImage.*Gather", opname) is not None:
                    self.imageGatherOps.append(opname)
                if re.search("OpImageFetch.*", opname) is not None:
                    self.imageFetchOps.append(opname)
                if re.search("OpImageSample.*", opname) is not None:
                    self.imageSampleOps.append(opname)
                if re.search("OpType.*", opname) is not None:
                    # Currently this is for GPU-AV which doesn't supporrt provisional extensions
                    if opname not in self.provisionalList:
                        self.typeOps.append(opname)
                if 'operands' in instruction:
                    for index, operand in enumerate(instruction['operands']):
                        kind = operand['kind']
                        if kind == 'IdResultType':
                            self.hasType.append(opname)
                        elif kind == 'IdResult':
                            self.hasResult.append(opname)
                        else:
                            # Operands are anything that isn't a result or result type
                            if 'quantifier' in operand:
                                if operand['quantifier'] == '?':
                                    self.opcodes[opcode]['hasOptional'] = True
                                if operand['quantifier'] == '*':
                                    self.opcodes[opcode]['hasVariableLength'] = True

                            operands = self.opcodes[opcode]['operands']
                            if kind in self.kindId:
                                operands.append('Id')
                            elif kind in self.kindLiteral:
                                if kind == 'LiteralString':
                                    operands.append('LiteralString')
                                else:
                                    operands.append('Literal')
                            elif kind in self.kindComposite:
                                operands.append('Composite')
                            elif kind in self.kindValueEnum:
                                operands.append('ValueEnum')
                            elif kind in self.kindBitEnum:
                                operands.append('BitEnum')

                        # some instructions have both types of IdScope
                        # OpReadClockKHR has the wrong 'name' as 'Scope'
                        if kind == 'IdScope':
                            if operand['name'] == '\'Execution\'' or operand['name'] == '\'Scope\'':
                                self.executionScopePosition[index + 1].append(opname)
                            elif operand['name'] == '\'Memory\'':
                                self.memoryScopePosition[index + 1].append(opname)
                            elif operand['name'] == '\'Visibility\'':
                                continue # ignore
                            else:
                                print(f'Error: unknown operand {opname} with IdScope {operand["name"]} not handled correctly\n')
                                sys.exit(1)
                        if kind == 'ImageOperands':
                            self.imageOperandsPosition[index + 1].append(opname)
                        if kind == 'IdRef':
                            if operand['name'] == '\'Image\'':
                                self.opcodes[opcode]['imageRefPosition'] = index + 1
                            elif operand['name'] == '\'Sampled Image\'':
                                self.opcodes[opcode]['sampledImageRefPosition'] = index + 1

                if re.search("OpImage*", opname) is not None:
                    info = self.opcodes[opcode]
                    imageRef = info['imageRefPosition']
                    sampledImageRef = info['sampledImageRefPosition']
                    if imageRef == 0 and sampledImageRef == 0:
                        # things like OpImageSparseTexelsResident don't do an actual image operation
                        continue
                    elif imageRef != 0 and sampledImageRef != 0:
                        print("Error: unknown opcode {} not handled correctly\n".format(opname))
                        sys.exit(1)
                    elif imageRef != 0:
                        self.imageAccessOperand[imageRef].append(opname)
                    elif sampledImageRef != 0:
                        self.imageAccessOperand[sampledImageRef].append(opname)
                # exceptions that don't fit the OpImage naming
                if opname == 'OpFragmentFetchAMD' or opname == 'OpFragmentMaskFetchAMD':
                    self.imageAccessOperand[3].append(opname)

                # We want to manually mark "Label" if an ID is used for Control Flow
                # It is easier to manage the few cases here then complex the operand logic above
                if opname == 'OpLoopMerge':
                    self.opcodes[opcode]['operands'] = ['Label', 'Label', 'BitEnum']
                if opname == 'OpSelectionMerge':
                    self.opcodes[opcode]['operands'] = ['Label', 'BitEnum']
                if opname == 'OpBranch':
                    self.opcodes[opcode]['operands'] = ['Label']
                if opname == 'OpBranchConditional':
                    self.opcodes[opcode]['operands'] = ['Id', 'Label', 'Label', 'Literal']
                if opname == 'OpSwitch':
                    self.opcodes[opcode]['operands'] = ['Id', 'Label', 'Label']

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2021-2024 The Khronos Group Inc.
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
            * This file is related to anything that is found in the SPIR-V grammar
            * file found in the SPIRV-Headers. Mainly used for SPIR-V util functions.
            *
            ****************************************************************************/\n''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'spirv_grammar_helper.h':
            self.generateHeader()
        elif self.filename == 'spirv_grammar_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once
            #include <cstdint>
            #include <string>
            #include <vector>
            #include <spirv/unified1/spirv.hpp>

            const char* string_SpvOpcode(uint32_t opcode);
            const char* string_SpvStorageClass(uint32_t storage_class);
            const char* string_SpvExecutionModel(uint32_t execution_model);
            const char* string_SpvExecutionMode(uint32_t execution_mode);
            const char* string_SpvDecoration(uint32_t decoration);
            const char* string_SpvBuiltIn(uint32_t built_in);
            const char* string_SpvDim(uint32_t dim);
            std::string string_SpvCooperativeMatrixOperands(uint32_t mask);
            ''')

        hasTypeCase = "\n".join([f"        case spv::{f}:" for f in self.hasType if f not in self.provisionalList])
        hasTypeCaseProvisional = "\n".join([f"        case spv::{f}:" for f in self.hasType if f in self.provisionalList])
        hasResultCase = "\n".join([f"        case spv::{f}:" for f in self.hasResult if f not in self.provisionalList])
        hasResultCaseProvisional = "\n".join([f"        case spv::{f}:" for f in self.hasResult if f in self.provisionalList])
        out.append(f'''
            static constexpr bool OpcodeHasType(uint32_t opcode) {{
                switch (opcode) {{
            {hasTypeCase}
            #ifdef VK_ENABLE_BETA_EXTENSIONS
            {hasTypeCaseProvisional}
            #endif
                        return true;
                    default:
                        return false;
                }}
            }}

            static constexpr bool OpcodeHasResult(uint32_t opcode) {{
                switch (opcode) {{
            {hasResultCase}
            #ifdef VK_ENABLE_BETA_EXTENSIONS
            {hasResultCaseProvisional}
            #endif
                        return true;
                    default:
                        return false;
                }}
            }}
            ''')

        # \n is not allowed in f-string until 3.12
        atomicCase = "\n".join([f"        case spv::{f}:" for f in self.atomicsOps])
        groupCase = "\n".join([f"        case spv::{f}:" for f in self.groupOps])
        out.append(f'''
            // Any non supported operation will be covered with other VUs
            static constexpr bool AtomicOperation(uint32_t opcode) {{
                switch (opcode) {{
            {atomicCase}
                        return true;
                    default:
                        return false;
                }}
            }}

            // Any non supported operation will be covered with other VUs
            static constexpr bool GroupOperation(uint32_t opcode) {{
                switch (opcode) {{
            {groupCase}
                        return true;
                    default:
                        return false;
                }}
            }}
            ''')

        imageGatherOpsCase = "\n".join([f"        case spv::{f}:" for f in self.imageGatherOps])
        imageFetchOpsCase = "\n".join([f"        case spv::{f}:" for f in self.imageFetchOps])
        imageSampleOpsCase = "\n".join([f"        case spv::{f}:" for f in self.imageSampleOps])
        out.append(f'''
            static constexpr bool ImageGatherOperation(uint32_t opcode) {{
                switch (opcode) {{
            {imageGatherOpsCase}
                        return true;
                    default:
                        return false;
                }}
            }}

            static constexpr bool ImageFetchOperation(uint32_t opcode) {{
                switch (opcode) {{
            {imageFetchOpsCase}
                        return true;
                    default:
                        return false;
                }}
            }}

            static constexpr bool ImageSampleOperation(uint32_t opcode) {{
                switch (opcode) {{
            {imageSampleOpsCase}
                        return true;
                    default:
                        return false;
                }}
            }}
            ''')

        out.append('''
            // Return number of optional parameter from ImageOperands
            static constexpr uint32_t ImageOperandsParamCount(uint32_t image_operand) {
                uint32_t count = 0;
                switch (image_operand) {
            ''')

        for index, operands in enumerate(self.imageOperandsParamCount):
            for operand in operands:
                if operand == 'None': # not sure why header is not consistent with this
                    out.append(f'        case spv::ImageOperandsMask{operand}:\n')
                else:
                    out.append(f'        case spv::ImageOperands{operand}Mask:\n')
            if len(operands) != 0:
                out.append(f'            return {index};\n')
        out.append('''
                    default:
                        break;
                }
                return count;
            }
            ''')

        out.append('''
            // Return operand position of Memory Scope <ID> or zero if there is none
            static constexpr uint32_t OpcodeMemoryScopePosition(uint32_t opcode) {
                uint32_t position = 0;
                switch (opcode) {
            ''')
        for index, opcodes in enumerate(self.memoryScopePosition):
            for opcode in opcodes:
                out.append(f'        case spv::{opcode}:\n')
            if len(opcodes) != 0:
                out.append(f'            return {index};\n')
        out.append('''
                    default:
                        break;
                }
                return position;
            }
            ''')

        out.append('''
            // Return operand position of Execution Scope <ID> or zero if there is none
            static constexpr uint32_t OpcodeExecutionScopePosition(uint32_t opcode) {
                uint32_t position = 0;
                switch (opcode) {
            ''')
        for index, opcodes in enumerate(self.executionScopePosition):
            for opcode in opcodes:
                out.append(f'        case spv::{opcode}:\n')
            if len(opcodes) != 0:
                out.append(f'            return {index};\n')
        out.append('''
                    default:
                        break;
                }
                return position;
            }
            ''')

        out.append('''
            // Return operand position of Image Operands <ID> or zero if there is none
            static constexpr uint32_t OpcodeImageOperandsPosition(uint32_t opcode) {
                uint32_t position = 0;
                switch (opcode) {
            ''')
        for index, opcodes in enumerate(self.imageOperandsPosition):
            for opcode in opcodes:
                out.append(f'        case spv::{opcode}:\n')
            if len(opcodes) != 0:
                out.append(f'            return {index};\n')
        out.append('''
                    default:
                        break;
                }
                return position;
            }
            ''')

        out.append('''
            // Return operand position of 'Image' or 'Sampled Image' IdRef or zero if there is none.
            static constexpr uint32_t OpcodeImageAccessPosition(uint32_t opcode) {
                uint32_t position = 0;
                switch (opcode) {
            ''')
        for index, opcodes in enumerate(self.imageAccessOperand):
            for opcode in opcodes:
                out.append(f'        case spv::{opcode}:\n')
            if len(opcodes) != 0:
                out.append(f'            return {index};\n')
        out.append('''
                    default:
                        break;
                }
                return position;
            }
            ''')

        out.append('''
            // All valid OpType*
            enum class SpvType {
                Empty = 0,
            ''')
        for type in self.typeOps:
            out.append(f'k{type[6:]},\n')
        out.append("};\n")

        typeCase = "\n".join([f"case spv::{f}: return SpvType::k{f[6:]};" for f in self.typeOps])
        out.append(f'''
            static constexpr SpvType GetSpvType(uint32_t opcode) {{
                switch (opcode) {{
                    {typeCase}
                    default:
                        return SpvType::Empty;
                }}
            }}
            ''')

        out.append('''
            enum class OperandKind {
                Invalid = 0,
                Id,
                Label, // Id but for Control Flow
                Literal,
                LiteralString,
                Composite,
                ValueEnum,
                BitEnum,
            };

            struct OperandInfo {
                std::vector<OperandKind> types;
            };

            const OperandInfo& GetOperandInfo(uint32_t opcode);
            ''')

        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include "containers/custom_containers.h"
            #include "spirv_grammar_helper.h"
            ''')

        out.append(f'''
            const char* string_SpvOpcode(uint32_t opcode) {{
                switch(opcode) {{
            {"".join([f"""        case spv::{x}:
                        return "{x}";
            """ for x in self.opnames if x not in self.provisionalList])}
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {"".join([f"""        case spv::{x}:
                        return "{x}";
            """ for x in self.opnames if x in self.provisionalList])}#endif
                    default:
                        return "Unknown Opcode";
                }}
            }}

            const char* string_SpvStorageClass(uint32_t storage_class) {{
                switch(storage_class) {{
            {"".join([f"""        case spv::StorageClass{x}:
                        return "{x}";
            """ for x in self.storageClassList if x not in self.provisionalList])}
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {"".join([f"""        case spv::StorageClass{x}:
                        return "{x}";
            """ for x in self.storageClassList if x in self.provisionalList])}#endif
                    default:
                        return "Unknown Storage Class";
                }}
            }}

            const char* string_SpvExecutionModel(uint32_t execution_model) {{
                switch(execution_model) {{
            {"".join([f"""        case spv::ExecutionModel{x}:
                        return "{x}";
            """ for x in self.executionModelList if x not in self.provisionalList])}
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {"".join([f"""        case spv::ExecutionModel{x}:
                        return "{x}";
            """ for x in self.executionModelList if x in self.provisionalList])}#endif
                    default:
                        return "Unknown Execution Model";
                }}
            }}

            const char* string_SpvExecutionMode(uint32_t execution_mode) {{
                switch(execution_mode) {{
            {"".join([f"""        case spv::ExecutionMode{x}:
                        return "{x}";
            """ for x in self.executionModeList if x not in self.provisionalList])}
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {"".join([f"""        case spv::ExecutionMode{x}:
                        return "{x}";
            """ for x in self.executionModeList if x in self.provisionalList])}#endif
                    default:
                        return "Unknown Execution Mode";
                }}
            }}

            const char* string_SpvDecoration(uint32_t decoration) {{
                switch(decoration) {{
            {"".join([f"""        case spv::Decoration{x}:
                        return "{x}";
            """ for x in self.decorationList if x not in self.provisionalList])}
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {"".join([f"""        case spv::Decoration{x}:
                        return "{x}";
            """ for x in self.decorationList if x in self.provisionalList])}#endif
                    default:
                        return "Unknown Decoration";
                }}
            }}

            const char* string_SpvBuiltIn(uint32_t built_in) {{
                switch(built_in) {{
            {"".join([f"""        case spv::BuiltIn{x}:
                        return "{x}";
            """ for x in self.builtInList if x not in self.provisionalList])}
#ifdef VK_ENABLE_BETA_EXTENSIONS
            {"".join([f"""        case spv::BuiltIn{x}:
                        return "{x}";
            """ for x in self.builtInList if x in self.provisionalList])}#endif
                    default:
                        return "Unknown BuiltIn";
                }}
            }}

            const char* string_SpvDim(uint32_t dim) {{
                switch(dim) {{
            {"".join([f"""        case spv::Dim{x}:
                        return "{x}";
            """ for x in self.dimList if x not in self.provisionalList])}
                    default:
                        return "Unknown Dim";
                }}
            }}

            static const char* string_SpvCooperativeMatrixOperandsMask(spv::CooperativeMatrixOperandsMask mask) {{
                switch(mask) {{
                    case spv::CooperativeMatrixOperandsMaskNone:
                        return "NoneKHR";
            {"".join([f"""        case spv::CooperativeMatrixOperands{x}Mask:
                        return "{x}";
            """ for x in self.cooperativeMatrixList if x not in self.provisionalList])}
                    default:
                        return "Unknown CooperativeMatrixOperandsMask";
                }}
            }}

            std::string string_SpvCooperativeMatrixOperands(uint32_t mask) {{
                std::string ret;
                while(mask) {{
                    if (mask & 1) {{
                        if(!ret.empty()) ret.append("|");
                        ret.append(string_SpvCooperativeMatrixOperandsMask(static_cast<spv::CooperativeMatrixOperandsMask>(1U << mask)));
                    }}
                    mask >>= 1;
                }}
                if (ret.empty()) ret.append("CooperativeMatrixOperandsMask(0)");
                return ret;
            }}
            ''')


        out.append('''
            const OperandInfo& GetOperandInfo(uint32_t opcode) {
                static const vvl::unordered_map<uint32_t, OperandInfo> kOperandTable {
                // clang-format off\n''')
        for info in self.opcodes.values():
            opname = info['opname']
            if opname in self.provisionalList:
                continue # Currently this is for GPU-AV which doesn't supporrt provisional extensions
            kinds = ", ".join([f"OperandKind::{f}" for f in info['operands']])
            out.append(f'        {{spv::{opname}, {{{{{kinds}}}}}}},\n')
        out.append('''    }; // clang-format on

                auto info = kOperandTable.find(opcode);
                if (info != kOperandTable.end()) {
                    return info->second;
                }
                return kOperandTable.find(spv::OpNop)->second;
            }
            ''')
        self.write("".join(out))