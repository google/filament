# Copyright 2021 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

add_library(dawn_node_binding STATIC
    "AsyncRunner.cpp"
    "AsyncRunner.h"
    "Converter.cpp"
    "Converter.h"
    "Errors.cpp"
    "Errors.h"
    "Flags.cpp"
    "Flags.h"
    "GPU.cpp"
    "GPU.h"
    "GPUAdapter.cpp"
    "GPUAdapter.h"
    "GPUAdapterInfo.cpp"
    "GPUAdapterInfo.h"
    "GPUBindGroup.cpp"
    "GPUBindGroup.h"
    "GPUBindGroupLayout.cpp"
    "GPUBindGroupLayout.h"
    "GPUBuffer.cpp"
    "GPUBuffer.h"
    "GPUCommandBuffer.cpp"
    "GPUCommandBuffer.h"
    "GPUCommandEncoder.cpp"
    "GPUCommandEncoder.h"
    "GPUComputePassEncoder.cpp"
    "GPUComputePassEncoder.h"
    "GPUComputePipeline.cpp"
    "GPUComputePipeline.h"
    "GPUDevice.cpp"
    "GPUDevice.h"
    "GPUPipelineLayout.cpp"
    "GPUPipelineLayout.h"
    "GPUQuerySet.cpp"
    "GPUQuerySet.h"
    "GPUQueue.cpp"
    "GPUQueue.h"
    "GPURenderBundle.cpp"
    "GPURenderBundle.h"
    "GPURenderBundleEncoder.cpp"
    "GPURenderBundleEncoder.h"
    "GPURenderPassEncoder.cpp"
    "GPURenderPassEncoder.h"
    "GPURenderPipeline.cpp"
    "GPURenderPipeline.h"
    "GPUSampler.cpp"
    "GPUSampler.h"
    "GPUShaderModule.cpp"
    "GPUShaderModule.h"
    "GPUSupportedFeatures.cpp"
    "GPUSupportedFeatures.h"
    "GPUSupportedLimits.cpp"
    "GPUSupportedLimits.h"
    "GPUTexture.cpp"
    "GPUTexture.h"
    "GPUTextureView.cpp"
    "GPUTextureView.h"
    "IteratorHelper.h"
    "Split.cpp"
    "Split.h"
    "TogglesLoader.cpp"
    "TogglesLoader.h"
)
common_compile_options(dawn_node_binding)

target_include_directories(dawn_node_binding
    PRIVATE
        "${PROJECT_SOURCE_DIR}"
        "${NODE_ADDON_API_DIR}"
        "${NODE_API_HEADERS_DIR}/include"
        "${DAWN_NODE_GEN_DIR}"
)

target_link_libraries(dawn_node_binding
    PRIVATE
        dawncpp
        dawn_node_interop
)
