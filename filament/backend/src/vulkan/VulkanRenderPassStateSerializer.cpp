/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fstream>

#include "VulkanRenderPassStateSerializer.h"
#include <utils/Log.h>
#include <utils/Panic.h>

#include "VulkanConstants.h"
#include "VulkanHandles.h"

#if defined(__clang__)
// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#endif

using namespace bluevk;

namespace filament::backend {

VulkanRenderPassStateSerializer::VulkanRenderPassStateSerializer(
        const VkRenderPassCreateInfo& info, uint32_t key) {
    std::stringstream filename;
    filename << "render_pass_" << key << ".json";

    std::ofstream file(filename.str());
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << "{" << std::endl;
        buffer << "\"attachments\":[" << std::endl;
        for (uint32_t i = 0; i < info.attachmentCount; ++i) {
            buffer << "{" << std::endl;
            buffer << "\"flags\":" << info.pAttachments[i].flags << "," << std::endl;
            buffer << "\"format\":" << info.pAttachments[i].format << "," << std::endl;
            buffer << "\"samples\":" << info.pAttachments[i].samples << "," << std::endl;
            buffer << "\"load_op\":" << info.pAttachments[i].loadOp << "," << std::endl;
            buffer << "\"store_op\":" << info.pAttachments[i].storeOp << "," << std::endl;
            buffer << "\"stencil_load_op\":" << info.pAttachments[i].stencilLoadOp << "," << std::endl;
            buffer << "\"stencil_store_op\":" << info.pAttachments[i].stencilStoreOp << "," << std::endl;
            buffer << "\"initial_layout\":" << info.pAttachments[i].initialLayout << "," << std::endl;
            buffer << "\"final_layout\":" << info.pAttachments[i].finalLayout << std::endl;
            buffer << "}";
            if (i < info.attachmentCount - 1) {
                buffer << ",";
            }
            buffer << std::endl;
        }
        buffer << "]," << std::endl;

        buffer << "\"subpasses\":[" << std::endl;
        for (uint32_t i = 0; i < info.subpassCount; ++i) {
            buffer << "{" << std::endl;
            buffer << "\"flags\":" << info.pSubpasses[i].flags << "," << std::endl;
            buffer << "\"pipeline_bind_point\":" << info.pSubpasses[i].pipelineBindPoint;
            
            //------------depth stencil-------------------
            if (info.pSubpasses[i].pDepthStencilAttachment) {
                buffer << "," << std::endl;
                buffer << "\"depth_stencil_attachment\":{" << std::endl;
                buffer << "\"attachement\":"
                       << info.pSubpasses[i].pDepthStencilAttachment->attachment << ","
                       << std::endl;
                buffer << "\"layout\":" << info.pSubpasses[i].pDepthStencilAttachment->layout
                       << std::endl;
                buffer << "}";
            }

            //------------Resolve-------------------
            if (info.pSubpasses[i].pResolveAttachments) {
                buffer << "," << std::endl;
                buffer << "\"resolve_attachments\":[" << std::endl;
                for (uint32_t j = 0; j < info.pSubpasses[i].colorAttachmentCount; ++j) {
                    buffer << "{" << std::endl;
                    buffer << "\"attachement\":"
                           << info.pSubpasses[i].pResolveAttachments[j].attachment << ","
                           << std::endl;
                    buffer << "\"layout\":" << info.pSubpasses[i].pResolveAttachments[j].layout
                           << std::endl;
                    buffer << "}" << std::endl;
                    if (j < info.pSubpasses[i].colorAttachmentCount - 1) {
                        buffer << ",";
                    }
                    buffer << std::endl;
                }
                buffer << "]";
            }

            //------------Color-------------------
            if (info.pSubpasses[i].pColorAttachments) {
                buffer << "," << std::endl;
                buffer << "\"color_attachments\":[" << std::endl;
                for (uint32_t j = 0; j < info.pSubpasses[i].colorAttachmentCount; ++j) {
                    buffer << "{" << std::endl;
                    buffer << "\"attachement\":"
                           << info.pSubpasses[i].pColorAttachments[j].attachment << ","
                           << std::endl;
                    buffer << "\"layout\":" << info.pSubpasses[i].pColorAttachments[j].layout
                           << std::endl;
                    buffer << "}" << std::endl;
                    if (j < info.pSubpasses[i].colorAttachmentCount - 1) {
                        buffer << ",";
                    }
                    buffer << std::endl;
                }
                buffer << "]";
            }

            //------------Input-------------------
            if (info.pSubpasses[i].pInputAttachments) {
                buffer << "," << std::endl;
                buffer << "\"input_attachments\":[" << std::endl;
                for (uint32_t j = 0; j < info.pSubpasses[i].inputAttachmentCount; ++j) {
                    buffer << "{" << std::endl;
                    buffer << "\"attachement\":"
                           << info.pSubpasses[i].pInputAttachments[j].attachment << ","
                           << std::endl;
                    buffer << "\"layout\":" << info.pSubpasses[i].pInputAttachments[j].layout
                           << std::endl;
                    buffer << "}" << std::endl;
                    if (j < info.pSubpasses[i].inputAttachmentCount - 1) {
                        buffer << ",";
                    }
                    buffer << std::endl;
                }
                buffer << "]" << std::endl;
            }

            buffer << "}";
            if (i < info.subpassCount - 1) {
                buffer << ",";
            }
            buffer << std::endl;
        }
        buffer << "]" << std::endl;

        buffer << "}";
        file << buffer.str();
    }
    file.close();
}
}
