#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
# Copyright (c) 2023-2025 RasterGrid Kft.
#
# SPDX-License-Identifier: Apache-2.0

import os
import re
from vulkan_object import Struct, Member
from base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class SafeStructOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        # Will skip completly, mainly used for things that have no one consuming the safe struct
        self.no_autogen = [
            # These WSI struct have nothing deep to copy, except the WSI releated field which we can't make a copy
            'VkXlibSurfaceCreateInfoKHR',
            'VkXcbSurfaceCreateInfoKHR',
            'VkWaylandSurfaceCreateInfoKHR',
            'VkAndroidSurfaceCreateInfoKHR',
            'VkWin32SurfaceCreateInfoKHR',
            'VkIOSSurfaceCreateInfoMVK',
            'VkMacOSSurfaceCreateInfoMVK',
            'VkMetalSurfaceCreateInfoEXT'
        ]

        # Will skip generating the source logic (to be implemented in vk_safe_struct_manual.cpp)
        # The header will still be generated
        self.manual_source = [
            # This needs to know if we're doing a host or device build, logic becomes complex and very specialized
            'VkAccelerationStructureBuildGeometryInfoKHR',
            'VkAccelerationStructureGeometryKHR',
            # Have a pUsageCounts and ppUsageCounts that is not currently handled in the generated code
            'VkMicromapBuildInfoEXT',
            'VkAccelerationStructureTrianglesOpacityMicromapEXT',
            'VkAccelerationStructureTrianglesDisplacementMicromapNV',
             # Special case because its pointers may be non-null but ignored
            'VkGraphicsPipelineCreateInfo',
            # Special case because it has custom construct parameters
            'VkPipelineViewportStateCreateInfo',
        ]

        # For abstract types just want to save the pointer away
        # since we cannot make a copy.
        self.abstract_types = [
            'AHardwareBuffer',
            '_screen_context',
            '_screen_window',
            '_screen_buffer'
        ]

        # These 'data' union are decided by the 'type' in the same parent struct
        self.union_of_pointers = [
            'VkDescriptorDataEXT',
            'VkIndirectCommandsTokenDataEXT',
            'VkIndirectExecutionSetInfoEXT',
        ]
        self.union_of_pointer_callers = [
            'VkDescriptorGetInfoEXT',
        ]

        # Will update the the function interface
        self.custom_construct_params = {
            # vku::safe::GraphicsPipelineCreateInfo needs to know if subpass has color and\or depth\stencil attachments to use its pointers
            'VkGraphicsPipelineCreateInfo' :
                ', const bool uses_color_attachment, const bool uses_depthstencil_attachment',
            # vku::safe::PipelineViewportStateCreateInfo needs to know if viewport and scissor is dynamic to use its pointers
            'VkPipelineViewportStateCreateInfo' :
                ', const bool is_dynamic_viewports, const bool is_dynamic_scissors',
            # vku::safe::AccelerationStructureBuildGeometryInfoKHR needs to know if we're doing a host or device build
            'VkAccelerationStructureBuildGeometryInfoKHR' :
                ', const bool is_host, const VkAccelerationStructureBuildRangeInfoKHR *build_range_infos',
            # vku::safe::AccelerationStructureGeometryKHR needs to know if we're doing a host or device build
            'VkAccelerationStructureGeometryKHR' :
                ', const bool is_host, const VkAccelerationStructureBuildRangeInfoKHR *build_range_info',
        }

    def isInPnextChain(self, struct: Struct) -> bool:
        # Can appear in VkPipelineCreateInfoKHR::pNext even though it isn't listed in the xml structextends attribute
        # VUID-VkPipelineCreateInfoKHR-pNext-09604
        pipeline_create_infos = [
            'VkGraphicsPipelineCreateInfo',
            'VkExecutionGraphPipelineCreateInfoAMDX',
            'VkRayTracingPipelineCreateInfoKHR',
            'VkComputePipelineCreateInfo',
        ]
        return struct.extends or struct.name in pipeline_create_infos

    # Determine if a structure needs a safe_struct helper function
    # That is, it has an sType or one of its members is a pointer
    def needSafeStruct(self, struct: Struct) -> bool:
        if struct.name in self.no_autogen:
            return False
        if struct.name in self.union_of_pointers:
            return False
        if 'VkBase' in struct.name:
            return False #  Ingore structs like VkBaseOutStructure
        if struct.sType is not None:
            return True
        for member in struct.members:
            if member.pointer:
                return True
        # The VK_EXT_sample_locations design created edge case, easiest to handle here
        if struct.name == 'VkAttachmentSampleLocationsEXT' or struct.name == 'VkSubpassSampleLocationsEXT':
            return True
        return False

    def containsObjectHandle(self, member: Member) -> bool:
        if member.type in self.vk.handles:
            return True
        if member.type in self.vk.structs:
            for subMember in self.vk.structs[member.type].members:
                if self.containsObjectHandle(subMember):
                    return True
        return False

    def typeContainsObjectHandle(self, handle_type: str, dispatchable: bool) -> bool:
        if handle_type in self.vk.handles:
            if dispatchable == self.vk.handles[handle_type].dispatchable:
                return True
        # if handle_type is a struct, search its members
        if handle_type in self.vk.structs:
            struct = self.vk.structs[handle_type]
            for member in [x for x in struct.members if x.type in self.vk.handles]:
                if dispatchable == self.vk.handles[member.type].dispatchable:
                    return True
        return False

    def generate(self):
        # Should be fixed in 1.4.310 headers
        # https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7196
        manual_protect = ["VkCudaModuleNV", "VkCudaFunctionNV", "VkCudaModuleCreateInfoNV", "VkCudaFunctionCreateInfoNV", "VkCudaLaunchInfoNV", "VkPhysicalDeviceCudaKernelLaunchFeaturesNV", "VkPhysicalDeviceCudaKernelLaunchPropertiesNV", "VkSetPresentConfigNV", "VkPhysicalDevicePresentMeteringFeaturesNV"]
        for struct in [x for x in self.vk.structs.values() if x.name in manual_protect]:
            struct.protect = "VK_ENABLE_BETA_EXTENSIONS"

        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2025 The Khronos Group Inc.
            * Copyright (c) 2015-2025 Valve Corporation
            * Copyright (c) 2015-2025 LunarG, Inc.
            * Copyright (c) 2015-2025 Google Inc.
            *
            * SPDX-License-Identifier: Apache-2.0
            *
            ****************************************************************************/\n''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'vk_safe_struct.hpp':
            self.generateHeader()
        elif self.filename == 'vk_safe_struct_utils.cpp':
            self.generateUtil()
        elif self.filename.startswith('vk_safe_struct_'):
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def convertName(self, vk_name):
        return "safe_" + vk_name

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once
            #include <vulkan/vulkan.h>
            #include <algorithm>

            #include <vulkan/utility/vk_safe_struct_utils.hpp>

            namespace vku {

            // Mapping of unknown stype codes to structure lengths. This should be set up by the application
            // before vkCreateInstance() and not modified afterwards.
            std::vector<std::pair<uint32_t, uint32_t>>& GetCustomStypeInfo();
            \n''')

        guard_helper = PlatformGuardHelper()
        for struct in [x for x in self.vk.structs.values() if self.needSafeStruct(x)]:
            safe_name = self.convertName(struct.name)
            out.extend(guard_helper.add_guard(struct.protect))
            out.append(f'{"union" if struct.union else "struct"} {safe_name} {{\n')
            # Can only initialize first member of an Union
            canInitialize = True
            copy_pnext = ', bool copy_pnext = true' if struct.sType is not None else ''
            for member in struct.members:
                if member.type in self.vk.structs:
                    if self.needSafeStruct(self.vk.structs[member.type]):
                        safe_member_type = self.convertName(member.type)
                        if member.pointer:
                            pointer = '*' * member.cDeclaration.count('*')
                            brackets = '' if struct.union else '{}'
                            out.append(f'{safe_member_type}{pointer} {member.name}{brackets};\n')
                        else:
                            out.append(f'{safe_member_type} {member.name};\n')
                        continue

                explicitInitialize = member.pointer  and 'PFN_' not in member.type and canInitialize
                initialize = '{}' if explicitInitialize else ''
                # Prevents union from initializing agian
                canInitialize = not struct.union if explicitInitialize else canInitialize

                if member.length and self.containsObjectHandle(member) and not member.fixedSizeArray:
                    out.append(f'    {member.type}* {member.name}{initialize};\n')
                else:
                    out.append(f'{member.cDeclaration}{initialize};\n')

            constructParam = self.custom_construct_params.get(struct.name, '')
            out.append(f'''
                {safe_name}(const {struct.name}* in_struct{constructParam}, PNextCopyState* copy_state = {{}}{copy_pnext});
                {safe_name}(const {safe_name}& copy_src);
                {safe_name}& operator=(const {safe_name}& copy_src);
                {safe_name}();
                ~{safe_name}();
                void initialize(const {struct.name}* in_struct{constructParam}, PNextCopyState* copy_state = {{}});
                void initialize(const {safe_name}* copy_src, PNextCopyState* copy_state = {{}});
                {struct.name} *ptr() {{ return reinterpret_cast<{struct.name} *>(this); }}
                {struct.name} const *ptr() const {{ return reinterpret_cast<{struct.name} const *>(this); }}
                ''')

            if struct.name == 'VkShaderModuleCreateInfo':
                out.append('''
                    // Primarily intended for use by GPUAV when replacing shader module code with instrumented code
                    template<typename Container>
                    void SetCode(const Container &code) {
                        delete[] pCode;
                        codeSize = static_cast<uint32_t>(code.size() * sizeof(uint32_t));
                        pCode = new uint32_t[code.size()];
                        std::copy(&code.front(), &code.back() + 1, const_cast<uint32_t*>(pCode));
                    }
                    ''')
            out.append('};\n')
        out.extend(guard_helper.add_guard(None))

        out.append('''
                // Safe struct that spans NV and KHR VkRayTracingPipelineCreateInfo structures.
                // It is a VkRayTracingPipelineCreateInfoKHR and supports construction from
                // a VkRayTracingPipelineCreateInfoNV.
                class safe_VkRayTracingPipelineCreateInfoCommon : public safe_VkRayTracingPipelineCreateInfoKHR {
                public:
                    safe_VkRayTracingPipelineCreateInfoCommon() : safe_VkRayTracingPipelineCreateInfoKHR() {}
                    safe_VkRayTracingPipelineCreateInfoCommon(const VkRayTracingPipelineCreateInfoNV *pCreateInfo)
                        : safe_VkRayTracingPipelineCreateInfoKHR() {
                        initialize(pCreateInfo);
                    }
                    safe_VkRayTracingPipelineCreateInfoCommon(const VkRayTracingPipelineCreateInfoKHR *pCreateInfo)
                        : safe_VkRayTracingPipelineCreateInfoKHR(pCreateInfo) {}

                    void initialize(const VkRayTracingPipelineCreateInfoNV *pCreateInfo);
                    void initialize(const VkRayTracingPipelineCreateInfoKHR *pCreateInfo);
                    uint32_t maxRecursionDepth = 0;  // NV specific
                };
                ''')
        out.append('''
            } // namespace vku
        ''')
        self.write("".join(out))

    def generateUtil(self):
        out = []
        out.append('''
            #include <vulkan/vk_layer.h>
            #include <vulkan/utility/vk_safe_struct.hpp>

            #include <vector>
            #include <cstring>

            namespace vku {
            char *SafeStringCopy(const char *in_string) {
                if (nullptr == in_string) return nullptr;
                size_t len = std::strlen(in_string);
                char* dest = new char[len + 1];
                dest[len] = '\\0';
                std::memcpy(dest, in_string, len);
                return dest;
            }

            ''')
        out.append('''
// clang-format off
void *SafePnextCopy(const void *pNext, PNextCopyState* copy_state) {
    void *first_pNext{};
    VkBaseOutStructure *prev_pNext{};
    void *safe_pNext{};

    while (pNext) {
        const VkBaseOutStructure *header = reinterpret_cast<const VkBaseOutStructure *>(pNext);

        switch (header->sType) {
            // Add special-case code to copy beloved secret loader structs
            // Special-case Loader Instance Struct passed to/from layer in pNext chain
            case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO: {
                VkLayerInstanceCreateInfo *struct_copy = new VkLayerInstanceCreateInfo;
                // TODO: Uses original VkLayerInstanceLink* chain, which should be okay for our uses
                memcpy(struct_copy, pNext, sizeof(VkLayerInstanceCreateInfo));
                safe_pNext = struct_copy;
                break;
            }
            // Special-case Loader Device Struct passed to/from layer in pNext chain
            case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO: {
                VkLayerDeviceCreateInfo *struct_copy = new VkLayerDeviceCreateInfo;
                // TODO: Uses original VkLayerDeviceLink*, which should be okay for our uses
                memcpy(struct_copy, pNext, sizeof(VkLayerDeviceCreateInfo));
                safe_pNext = struct_copy;
                break;
            }
''')
        guard_helper = PlatformGuardHelper()
        for struct in filter(self.isInPnextChain, self.vk.structs.values()):
            safe_name = self.convertName(struct.name)
            out.extend(guard_helper.add_guard(struct.protect))
            out.append(f'            case {struct.sType}:\n')
            out.append(f'                safe_pNext = new {safe_name}(reinterpret_cast<const {struct.name} *>(pNext), copy_state, false);\n')
            out.append('                break;\n')
        out.extend(guard_helper.add_guard(None))

        out.append('''
            default: // Encountered an unknown sType -- skip (do not copy) this entry in the chain
                // If sType is in custom list, construct blind copy
                for (auto item : GetCustomStypeInfo()) {
                    if (item.first == static_cast<uint32_t>(header->sType)) {
                        safe_pNext = malloc(item.second);
                        memcpy(safe_pNext, header, item.second);
                    }
                }
                break;
        }
        if (!first_pNext) {
            first_pNext = safe_pNext;
        }
        pNext = header->pNext;
        if (prev_pNext && safe_pNext) {
            prev_pNext->pNext = (VkBaseOutStructure*)safe_pNext;
        }
        if (safe_pNext) {
            prev_pNext = (VkBaseOutStructure*)safe_pNext;
        }
        safe_pNext = nullptr;
    }

    return first_pNext;
}

void FreePnextChain(const void *pNext) {
    // The pNext parameter is const for convenience, since it is called by code
    // for many structures where the pNext field is const.
    void *current = const_cast<void*>(pNext);
    while (current) {
        auto header = reinterpret_cast<VkBaseOutStructure *>(current);
        void *next = header->pNext;
        // prevent destructors from recursing behind our backs.
        header->pNext = nullptr;

        switch (header->sType) {
            // Special-case Loader Instance Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO:
            delete reinterpret_cast<VkLayerInstanceCreateInfo *>(current);
            break;
        // Special-case Loader Device Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO:
            delete reinterpret_cast<VkLayerDeviceCreateInfo *>(current);
            break;
''')

        for struct in filter(self.isInPnextChain, self.vk.structs.values()):
            safe_name = self.convertName(struct.name)
            out.extend(guard_helper.add_guard(struct.protect))
            out.append(f'        case {struct.sType}:\n')
            out.append(f'            delete reinterpret_cast<{safe_name} *>(header);\n')
            out.append('            break;\n')
        out.extend(guard_helper.add_guard(None))

        out.append('''
        default: // Encountered an unknown sType
            // If sType is in custom list, free custom struct memory and clean up
            for (auto item : GetCustomStypeInfo()   ) {
                if (item.first == static_cast<uint32_t>(header->sType)) {
                    free(current);
                    break;
                }
            }
            break;
        }
        current = next;
    }
}''')
        out.append('// clang-format on\n')
        out.append('''
            } // namespace vku
        ''')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include <vulkan/utility/vk_safe_struct.hpp>
            #include <vulkan/utility/vk_struct_helper.hpp>

            #include <cstddef>
            #include <cstring>

            namespace vku {
            ''')

        custom_defeault_construct_txt = {}

        custom_construct_txt = {
                # VkWriteDescriptorSet is special case because pointers may be non-null but ignored
                'VkWriteDescriptorSet' : '''
                    switch (descriptorType) {
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:
                        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:
                        if (descriptorCount && in_struct->pImageInfo) {
                            pImageInfo = new VkDescriptorImageInfo[descriptorCount];
                            for (uint32_t i = 0; i < descriptorCount; ++i) {
                                pImageInfo[i] = in_struct->pImageInfo[i];
                            }
                        }
                        break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                        if (descriptorCount && in_struct->pBufferInfo) {
                            pBufferInfo = new VkDescriptorBufferInfo[descriptorCount];
                            for (uint32_t i = 0; i < descriptorCount; ++i) {
                                pBufferInfo[i] = in_struct->pBufferInfo[i];
                            }
                        }
                        break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                        if (descriptorCount && in_struct->pTexelBufferView) {
                            pTexelBufferView = new VkBufferView[descriptorCount];
                            for (uint32_t i = 0; i < descriptorCount; ++i) {
                                pTexelBufferView[i] = in_struct->pTexelBufferView[i];
                            }
                        }
                        break;
                        default:
                        break;
                    }
                ''',
                'VkShaderModuleCreateInfo' : '''
                    if (in_struct->pCode) {
                        pCode = reinterpret_cast<uint32_t *>(new uint8_t[codeSize]);
                        memcpy((void *)pCode, (void *)in_struct->pCode, codeSize);
                    }
                ''',
                # VkFrameBufferCreateInfo is special case because its pAttachments pointer may be non-null but ignored
                'VkFramebufferCreateInfo' : '''
                    if (attachmentCount && in_struct->pAttachments && !(flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {
                        pAttachments = new VkImageView[attachmentCount];
                        for (uint32_t i = 0; i < attachmentCount; ++i) {
                            pAttachments[i] = in_struct->pAttachments[i];
                        }
                    }
                ''',
                # VkDescriptorSetLayoutBinding is special case because its pImmutableSamplers pointer may be non-null but ignored
                'VkDescriptorSetLayoutBinding' : '''
                    const bool sampler_type = in_struct->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER || in_struct->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    if (descriptorCount && in_struct->pImmutableSamplers && sampler_type) {
                        pImmutableSamplers = new VkSampler[descriptorCount];
                        for (uint32_t i = 0; i < descriptorCount; ++i) {
                            pImmutableSamplers[i] = in_struct->pImmutableSamplers[i];
                        }
                    }
                ''',
                'VkPipelineRenderingCreateInfo': '''
                    bool custom_init = copy_state && copy_state->init;
                    if (custom_init) {
                        custom_init = copy_state->init(reinterpret_cast<VkBaseOutStructure*>(this), reinterpret_cast<const VkBaseOutStructure*>(in_struct));
                    }
                    if (!custom_init) {
                        // The custom iniitalization was not used, so do the regular initialization
                        if (in_struct->pColorAttachmentFormats) {
                            pColorAttachmentFormats = new VkFormat[in_struct->colorAttachmentCount];
                            memcpy ((void *)pColorAttachmentFormats, (void *)in_struct->pColorAttachmentFormats, sizeof(VkFormat)*in_struct->colorAttachmentCount);
                        }
                    }
                ''',
                # TODO: VkPushDescriptorSetWithTemplateInfo needs a custom constructor to handle pData
                # https://github.com/KhronosGroup/Vulkan-Utility-Libraries/issues/193
                'VkPushDescriptorSetWithTemplateInfo': '''
                ''',
            }

        custom_copy_txt = {
                'VkFramebufferCreateInfo' : '''
                    pNext = SafePnextCopy(copy_src.pNext);
                    if (attachmentCount && copy_src.pAttachments && !(flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {
                        pAttachments = new VkImageView[attachmentCount];
                        for (uint32_t i = 0; i < attachmentCount; ++i) {
                            pAttachments[i] = copy_src.pAttachments[i];
                        }
                    }
                ''',
                'VkPipelineRenderingCreateInfo': '''
                    if (copy_src.pColorAttachmentFormats) {
                        pColorAttachmentFormats = new VkFormat[copy_src.colorAttachmentCount];
                        memcpy ((void *)pColorAttachmentFormats, (void *)copy_src.pColorAttachmentFormats, sizeof(VkFormat)*copy_src.colorAttachmentCount);
                    }
                '''
            }

        custom_destruct_txt = {
                'VkShaderModuleCreateInfo' : '''
                    if (pCode)
                        delete[] reinterpret_cast<const uint8_t *>(pCode);
                ''',
            }

        member_init_transforms = {
            'queueFamilyIndexCount': lambda m: f'{m.name}(0)'
        }

        def qfi_construct(item, member):
            true_index_setter = lambda i: f'{i}queueFamilyIndexCount = in_struct->queueFamilyIndexCount;\n'
            false_index_setter = lambda i: f'{i}queueFamilyIndexCount = 0;\n'
            if item.name == 'VkSwapchainCreateInfoKHR':
                return (f'(in_struct->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->{member.name}', true_index_setter, false_index_setter)
            else:
                return (f'(in_struct->sharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->{member.name}', true_index_setter, false_index_setter)

        # map of:
        #  <member name>: function(item, member) -> (condition, true statement, false statement)
        member_construct_conditions = {
            'pQueueFamilyIndices': qfi_construct
        }

        # Find what types of safe structs need to be generated based on output file name
        splitRegex = r'.*';
        if self.filename.endswith('_khr.cpp'):
            splitRegex = r'.*KHR$'
        elif self.filename.endswith('_ext.cpp'):
            splitRegex = r'.*EXT$'
        elif self.filename.endswith('_vendor.cpp'):
            splitRegex = r'^(?!.*(KHR|EXT)$).*[A-Z]$' # Matches all words finishing with an upper case letter, but not ending with KHRor EXT
        else: # elif self.filename.endswith('_core.cpp'):
            splitRegex = r'.*[a-z0-9]$'

        guard_helper = PlatformGuardHelper()

        for struct in [x for x in self.vk.structs.values() if self.needSafeStruct(x) and x.name not in self.manual_source and re.match(splitRegex, x.name)]:
            out.extend(guard_helper.add_guard(struct.protect))

            init_list = ''          # list of members in struct constructor initializer
            default_init_list = ''  # Default constructor just inits ptrs to nullptr in initializer
            init_func_txt = ''      # Txt for initialize() function that takes struct ptr and inits members
            construct_txt = ''      # Body of constuctor as well as body of initialize() func following init_func_txt
            destruct_txt = ''

            has_pnext = struct.sType is not None
            copy_pnext = ''
            copy_pnext_if = ''
            copy_strings = ''
            for member in struct.members:
                m_type = member.type
                m_type_safe = False
                if member.name == 'pNext':
                    copy_pnext = 'pNext = SafePnextCopy(in_struct->pNext, copy_state);\n'
                    copy_pnext_if = '''
                    if (copy_pnext) {
                        pNext = SafePnextCopy(in_struct->pNext, copy_state);
                    }'''
                if member.type in self.vk.structs and self.needSafeStruct(self.vk.structs[member.type]):
                    m_type = self.convertName(member.type)
                    m_type_safe = True;

                if member.pointer and not m_type_safe and 'PFN_' not in member.type and not self.typeContainsObjectHandle(member.type, False):
                    # Ptr types w/o a safe_struct, for non-null case need to allocate new ptr and copy data in
                    if m_type in ['void', 'char']:
                        if member.name != 'pNext':
                            if m_type == 'char':
                                # Create deep copies of strings
                                if member.length:
                                    copy_strings += f'''
                                        char **tmp_{member.name} = new char *[in_struct->{member.length}];
                                        for (uint32_t i = 0; i < {member.length}; ++i) {{
                                            tmp_{member.name}[i] = SafeStringCopy(in_struct->{member.name}[i]);
                                        }}
                                        {member.name} = tmp_{member.name};'''

                                    destruct_txt += f'''
                                        if ({member.name}) {{
                                            for (uint32_t i = 0; i < {member.length}; ++i) {{
                                                delete [] {member.name}[i];
                                            }}
                                            delete [] {member.name};
                                        }}'''
                                else:
                                    copy_strings += f'{member.name} = SafeStringCopy(in_struct->{member.name});\n'
                                    destruct_txt += f'if ({member.name}) delete [] {member.name};\n'
                            else:
                                # We need a deep copy of pData / dataSize combos
                                if member.name == 'pData':
                                    init_list += f'\n    {member.name}(nullptr),'
                                    construct_txt += '''
                                        if (in_struct->pData != nullptr) {
                                            auto temp = new std::byte[in_struct->dataSize];
                                            std::memcpy(temp, in_struct->pData, in_struct->dataSize);
                                            pData = temp;
                                        }
                                        '''

                                    destruct_txt  += '''
                                        if (pData != nullptr) {
                                            auto temp = reinterpret_cast<const std::byte*>(pData);
                                            delete [] temp;
                                        }
                                        '''
                                else:
                                    init_list += f'\n{member.name}(in_struct->{member.name}),'
                                    init_func_txt += f'{member.name} = in_struct->{member.name};\n'
                        default_init_list += f'\n{member.name}(nullptr),'
                    else:
                        default_init_list += f'\n{member.name}(nullptr),'
                        init_list += f'\n{member.name}(nullptr),'
                        if m_type in self.abstract_types:
                            construct_txt += f'{member.name} = in_struct->{member.name};\n'
                        else:
                            init_func_txt += f'{member.name} = nullptr;\n'
                            if not member.fixedSizeArray and (member.length is None or '/' in member.length):
                                construct_txt += f'''
                                    if (in_struct->{member.name}) {{
                                            {member.name} = new {m_type}(*in_struct->{member.name});
                                        }}
                                    '''
                                destruct_txt += f'if ({member.name})\n'
                                destruct_txt += f'    delete {member.name};\n'
                            else:
                                # Prepend struct members with struct name
                                decorated_length = member.length
                                for other_member in struct.members:
                                    decorated_length = re.sub(r'\b({})\b'.format(other_member.name), r'in_struct->\1', decorated_length)
                                try:
                                    concurrent_clause = member_construct_conditions[member.name](struct, member)
                                except:
                                    concurrent_clause = (f'in_struct->{member.name}', lambda x: '')
                                construct_txt += f'''
                                    if ({concurrent_clause[0]}) {{
                                        {member.name} = new {m_type}[{decorated_length}];
                                        memcpy ((void *){member.name}, (void *)in_struct->{member.name}, sizeof({m_type})*{decorated_length});
                                        {concurrent_clause[1]('        ')}'''
                                if len(concurrent_clause) > 2:
                                    construct_txt += '} else {\n'
                                    construct_txt += concurrent_clause[2]('        ')
                                construct_txt += '}\n'
                                destruct_txt += f'if ({member.name})\n'
                                destruct_txt += f'    delete[] {member.name};\n'
                elif member.fixedSizeArray or member.length is not None:
                    if member.fixedSizeArray:
                        construct_txt += f'''
                            for (uint32_t i = 0; i < {member.fixedSizeArray[0]}; ++i) {{
                                    {member.name}[i] = in_struct->{member.name}[i];
                                }}
                            '''
                    else:
                        # Init array ptr to NULL
                        default_init_list += f'\n{member.name}(nullptr),'
                        init_list += f'\n{member.name}(nullptr),'
                        init_func_txt += f'{member.name} = nullptr;\n'
                        array_element = f'in_struct->{member.name}[i]'
                        if member.type in self.vk.structs and self.needSafeStruct(self.vk.structs[member.type]):
                            array_element = f'{member.type}(&in_struct->safe_{member.name}[i])'
                        construct_txt += f'if ({member.length} && in_struct->{member.name}) {{\n'
                        construct_txt += f'    {member.name} = new {m_type}[{member.length}];\n'
                        destruct_txt += f'if ({member.name})\n'
                        destruct_txt += f'    delete[] {member.name};\n'
                        construct_txt += f'for (uint32_t i = 0; i < {member.length}; ++i) {{\n'
                        if m_type_safe:
                            construct_txt += f'{member.name}[i].initialize(&in_struct->{member.name}[i]);\n'
                        else:
                            construct_txt += f'{member.name}[i] = {array_element};\n'
                        construct_txt += '}\n'
                        construct_txt += '}\n'
                elif member.pointer and 'PFN_' not in member.type:
                    default_init_list += f'\n{member.name}(nullptr),'
                    init_list += f'\n{member.name}(nullptr),'
                    init_func_txt += f'{member.name} = nullptr;\n'
                    construct_txt += f'if (in_struct->{member.name})\n'
                    construct_txt += f'    {member.name} = new {m_type}(in_struct->{member.name});\n'
                    destruct_txt += f'if ({member.name})\n'
                    destruct_txt += f'    delete {member.name};\n'
                elif m_type_safe and member.type in self.union_of_pointers:
                    init_list += f'\n{member.name}(&in_struct->{member.name}, in_struct->type),'
                    init_func_txt += f'{member.name}.initialize(&in_struct->{member.name}, in_struct->type);\n'
                elif m_type_safe:
                    init_list += f'\n{member.name}(&in_struct->{member.name}),'
                    init_func_txt += f'{member.name}.initialize(&in_struct->{member.name});\n'
                else:
                    try:
                        init_list += f'\n{member_init_transforms[member.name](member)},'
                    except:
                        init_list += f'\n{member.name}(in_struct->{member.name}),'
                        init_func_txt += f'{member.name} = in_struct->{member.name};\n'
                    if not struct.union:
                        if member.name == 'sType' and struct.sType:
                            default_init_list += f'\n{member.name}({struct.sType}),'
                        else:
                            default_init_list += f'\n{member.name}(),'
            if '' != init_list:
                init_list = init_list[:-1] # hack off final comma

            if struct.name in custom_construct_txt:
                construct_txt = custom_construct_txt[struct.name]

            construct_txt = copy_strings + construct_txt

            if struct.name in custom_destruct_txt:
                destruct_txt = custom_destruct_txt[struct.name]

            copy_pnext_param = ''
            if has_pnext:
                copy_pnext_param = ', bool copy_pnext'
                destruct_txt += '    FreePnextChain(pNext);\n'

            safe_name = self.convertName(struct.name)
            if struct.union:
                # Unions don't allow multiple members in the initialization list, so just call initialize
                out.append(f'''
                    {safe_name}::{safe_name}(const {struct.name}* in_struct{self.custom_construct_params.get(struct.name, '')}, PNextCopyState*)
                    {{
                        initialize(in_struct);
                    }}
                    ''')
            else:
                out.append(f'''
                    {safe_name}::{safe_name}(const {struct.name}* in_struct{self.custom_construct_params.get(struct.name, '')}, [[maybe_unused]] PNextCopyState* copy_state{copy_pnext_param}) :{init_list}
                    {{
                    {copy_pnext_if + construct_txt}}}
                    ''')
            if '' != default_init_list:
                # trim trailing comma from initializer list
                default_init_list = f' :{default_init_list[:-1]}'
                # truncate union initializer list to first element
                if struct.union:
                    default_init_list = default_init_list.split(',')[0]
            default_init_body = '\n' + custom_defeault_construct_txt[struct.name] if struct.name in custom_defeault_construct_txt else ''
            out.append(f'''
                {safe_name}::{safe_name}(){default_init_list}
                {{{default_init_body}}}
                ''')
            # Create slight variation of init and construct txt for copy constructor that takes a copy_src object reference vs. struct ptr
            construct_txt = copy_pnext + construct_txt
            copy_construct_init = init_func_txt.replace('in_struct->', 'copy_src.')
            copy_construct_init = copy_construct_init.replace(', copy_state', '')
            if struct.name in self.union_of_pointer_callers:
                copy_construct_init = copy_construct_init.replace(', copy_src.type', '')
            # Pass object to copy constructors
            copy_construct_txt = re.sub('(new \\w+)\\(in_struct->', '\\1(*copy_src.', construct_txt)
            # Modify remaining struct refs for copy_src object
            copy_construct_txt = copy_construct_txt.replace('in_struct->', 'copy_src.')
            # Modify remaining struct refs for copy_src object
            copy_construct_txt = copy_construct_txt .replace(', copy_state', '')
            if struct.name in custom_copy_txt:
                copy_construct_txt = custom_copy_txt[struct.name]
            copy_assign_txt = '    if (&copy_src == this) return *this;\n\n' + destruct_txt + '\n' + copy_construct_init + copy_construct_txt + '\n    return *this;'
            # Copy constructor
            out.append(f'''
                {safe_name}::{safe_name}(const {safe_name}& copy_src)
                {{
                {copy_construct_init}{copy_construct_txt}}}
                ''')
            # Copy assignment operator
            out.append(f'''
                {safe_name}& {safe_name}::operator=(const {safe_name}& copy_src)\n{{
                {copy_assign_txt}
                }}
                ''')
            out.append(f'''
                {safe_name}::~{safe_name}()
                {{
                {destruct_txt}}}
                ''')
            out.append(f'''
                void {safe_name}::initialize(const {struct.name}* in_struct{self.custom_construct_params.get(struct.name, '')}, [[maybe_unused]] PNextCopyState* copy_state)
                {{
                {destruct_txt}{init_func_txt}{construct_txt}}}
                ''')
            # Copy initializer uses same txt as copy constructor but has a ptr and not a reference
            init_copy = copy_construct_init.replace('copy_src.', 'copy_src->')
            # Replace '&copy_src' with 'copy_src' unless it's followed by a dereference
            init_copy = re.sub(r'&copy_src(?!->)', 'copy_src', init_copy)
            init_construct = copy_construct_txt.replace('copy_src.', 'copy_src->')
            # Replace '&copy_src' with 'copy_src' unless it's followed by a dereference
            init_construct = re.sub(r'&copy_src(?!->)', 'copy_src', init_construct)
            out.append(f'''
                void {safe_name}::initialize(const {safe_name}* copy_src, [[maybe_unused]] PNextCopyState* copy_state)
                {{
                {init_copy}{init_construct}}}
                ''')
        out.extend(guard_helper.add_guard(None))
        out.append('''
            } // namespace vku
        ''')

        self.write("".join(out))
