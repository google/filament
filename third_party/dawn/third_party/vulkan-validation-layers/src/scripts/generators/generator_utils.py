#!/usr/bin/python3 -i
#
# Copyright (c) 2023-2024 Valve Corporation
# Copyright (c) 2023-2024 LunarG, Inc.
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
import sys
import json

# Build a set of all vuid text strings found in validusage.json
def buildListVUID(valid_usage_file: str) -> set:

    # Walk the JSON-derived dict and find all "vuid" key values
    def ExtractVUIDs(vuid_dict):
        if hasattr(vuid_dict, 'items'):
            for key, value in vuid_dict.items():
                if key == "vuid":
                    yield value
                elif isinstance(value, dict):
                    for vuid in ExtractVUIDs(value):
                        yield vuid
                elif isinstance (value, list):
                    for listValue in value:
                        for vuid in ExtractVUIDs(listValue):
                            yield vuid

    valid_vuids = set()
    if not os.path.isfile(valid_usage_file):
        print(f'Error: Could not find, or error loading {valid_usage_file}')
        sys.exit(1)
    json_file = open(valid_usage_file, 'r', encoding='utf-8')
    vuid_dict = json.load(json_file)
    json_file.close()
    if len(vuid_dict) == 0:
        print(f'Error: Failed to load {valid_usage_file}')
        sys.exit(1)
    for json_vuid_string in ExtractVUIDs(vuid_dict):
        valid_vuids.add(json_vuid_string)

    return valid_vuids

# Will do a sanity check the VUID exists
def getVUID(valid_vuids: set, vuid: str, quotes: bool = True) -> str:
    if vuid not in valid_vuids:
        print(f'Warning: Could not find {vuid} in validusage.json')
        vuid = vuid.replace('VUID-', 'UNASSIGNED-')
    return vuid if not quotes else f'"{vuid}"'

class PlatformGuardHelper():
    """Used to elide platform guards together, so redundant #endif then #ifdefs are removed
    Note - be sure to call add_guard(None) when done to add a trailing #endif if needed
    """
    def __init__(self):
        self.current_guard = None

    def add_guard(self, guard, extra_newline = False):
        out = []
        if self.current_guard != guard and self.current_guard is not None:
            out.append(f'#endif  // {self.current_guard}\n')
        if extra_newline:
            out.append('\n')
        if self.current_guard != guard and guard is not None:
            out.append(f'#ifdef {guard}\n')
        self.current_guard = guard
        return out

# The SPIR-V grammar json doesn't have an easy way to detect these, so have listed by hand
# If we are missing one, its not critical, the goal of this list is to reduce the generated output size
def IsNonVulkanSprivCapability(capability):
    return capability in [
        'Kernel',
        'Vector16',
        'Float16Buffer',
        'ImageBasic',
        'ImageReadWrite',
        'ImageMipmap',
        'DeviceEnqueue',
        'SubgroupDispatch',
        'Pipes',
        'LiteralSampler',
        'NamedBarrier',
        'PipeStorage',
        'SubgroupShuffleINTEL',
        'SubgroupShuffleINTEL',
        'SubgroupBufferBlockIOINTEL',
        'SubgroupImageBlockIOINTEL',
        'SubgroupImageMediaBlockIOINTEL',
        'RoundToInfinityINTEL',
        'FloatingPointModeINTEL',
        'IndirectReferencesINTEL',
        'AsmINTEL',
        'VectorComputeINTEL',
        'VectorAnyINTEL',
        'SubgroupAvcMotionEstimationINTEL',
        'SubgroupAvcMotionEstimationIntraINTEL',
        'SubgroupAvcMotionEstimationChromaINTEL',
        'VariableLengthArrayINTEL',
        'FunctionFloatControlINTEL',
        'FPGAMemoryAttributesINTEL',
        'FPFastMathModeINTEL',
        'ArbitraryPrecisionIntegersINTEL',
        'ArbitraryPrecisionFloatingPointINTEL',
        'UnstructuredLoopControlsINTEL',
        'FPGALoopControlsINTEL',
        'KernelAttributesINTEL',
        'FPGAKernelAttributesINTEL',
        'FPGAMemoryAccessesINTEL',
        'FPGAClusterAttributesINTEL',
        'LoopFuseINTEL',
        'FPGADSPControlINTEL',
        'MemoryAccessAliasingINTEL',
        'FPGAInvocationPipeliningAttributesINTEL',
        'FPGABufferLocationINTEL',
        'ArbitraryPrecisionFixedPointINTEL',
        'USMStorageClassesINTEL',
        'RuntimeAlignedAttributeINTEL',
        'IOPipesINTEL',
        'BlockingPipesINTEL',
        'FPGARegINTEL',
        'LongCompositesINTEL',
        'OptNoneINTEL',
        'DebugInfoModuleINTEL',
        'BFloat16ConversionINTEL',
        'SplitBarrierINTEL',
        'FPGAClusterAttributesV2INTEL',
        'FPGAKernelAttributesv2INTEL',
        'FPMaxErrorINTEL',
        'FPGALatencyControlINTEL',
        'FPGAArgumentInterfacesINTEL',
        'GlobalVariableHostAccessINTEL',
        'GlobalVariableFPGADecorationsINTEL',
        'MaskedGatherScatterINTEL',
        'CacheControlsINTEL',
        'RegisterLimitsINTEL'
    ]
