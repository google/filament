# Copyright 2020 The Dawn & Tint Authors
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

DawnJSONGenerator(
    TARGET "wire"
    PRINT_NAME "Dawn wire"
    OUTPUT_HEADERS DAWN_WIRE_GEN_HEADERS
    OUTPUT_SOURCES DAWN_WIRE_GEN_SOURCES
)

set(headers
    "${DAWN_INCLUDE_DIR}/dawn/wire/Wire.h"
    "${DAWN_INCLUDE_DIR}/dawn/wire/WireClient.h"
    "${DAWN_INCLUDE_DIR}/dawn/wire/WireServer.h"
    "${DAWN_INCLUDE_DIR}/dawn/wire/dawn_wire_export.h"
)

set(private_headers
    "${DAWN_WIRE_GEN_HEADERS}"
    "BufferConsumer_impl.h"
    "BufferConsumer.h"
    "ChunkedCommandHandler.h"
    "ChunkedCommandSerializer.h"
    "client/Adapter.h"
    "client/ApiObjects.h"
    "client/Buffer.h"
    "client/Client.h"
    "client/Device.h"
    "client/EventManager.h"
    "client/Instance.h"
    "client/LimitsAndFeatures.h"
    "client/ObjectBase.h"
    "client/ObjectStore.h"
    "client/QuerySet.h"
    "client/Queue.h"
    "client/ShaderModule.h"
    "client/Surface.h"
    "client/Texture.h"
    "ObjectHandle.h"
    "server/ObjectStorage.h"
    "server/Server.h"
    "SupportedFeatures.h"
    "WireDeserializeAllocator.h"
    "WireResult.h"
)

set(sources
    "${DAWN_WIRE_GEN_SOURCES}"
    "ChunkedCommandHandler.cpp"
    "ChunkedCommandSerializer.cpp"
    "client/Adapter.cpp"
    "client/Buffer.cpp"
    "client/Client.cpp"
    "client/ClientDoers.cpp"
    "client/ClientInlineMemoryTransferService.cpp"
    "client/Device.cpp"
    "client/EventManager.cpp"
    "client/Instance.cpp"
    "client/LimitsAndFeatures.cpp"
    "client/ObjectBase.cpp"
    "client/ObjectStore.cpp"
    "client/QuerySet.cpp"
    "client/Queue.cpp"
    "client/ShaderModule.cpp"
    "client/Surface.cpp"
    "client/Texture.cpp"
    "ObjectHandle.cpp"
    "server/Server.cpp"
    "server/ServerAdapter.cpp"
    "server/ServerBuffer.cpp"
    "server/ServerDevice.cpp"
    "server/ServerInlineMemoryTransferService.cpp"
    "server/ServerInstance.cpp"
    "server/ServerQueue.cpp"
    "server/ServerShaderModule.cpp"
    "server/ServerSurface.cpp"
    "SupportedFeatures.cpp"
    "Wire.cpp"
    "WireClient.cpp"
    "WireDeserializeAllocator.cpp"
    "WireServer.cpp"
)

dawn_add_library(
  dawn_wire
  UTILITY_TARGET  dawn_internal_config
  HEADERS
    ${headers}
  PRIVATE_HEADERS
    ${private_headers}
  SOURCES
    ${sources}
  DEPENDS
    dawn::dawn_headers
  PRIVATE_DEPENDS
    absl::flat_hash_map
    absl::flat_hash_set
    dawn::dawn_common
    dawn::partition_alloc
    tint_lang_wgsl_features
)
target_compile_definitions(dawn_wire PRIVATE "DAWN_WIRE_IMPLEMENTATION")
if(BUILD_SHARED_LIBS)
    target_compile_definitions(dawn_wire PRIVATE "DAWN_WIRE_SHARED_LIBRARY")
endif()
