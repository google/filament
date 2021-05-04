hunter_config(civetweb VERSION 1.11-p0
             CMAKE_ARGS CIVETWEB_ENABLE_WEBSOCKETS=ON)

hunter_config(SPIRV-Headers VERSION 1.5.1.corrected)

hunter_config(SPIRV-Tools VERSION 2020.1-p0)

hunter_config(spirv-cross VERSION 20210115)

hunter_config(glslang VERSION 8.13.3743-9eef54b2-p0
              CMAKE_ARGS ENABLE_HLSL=OFF ENABLE_GLSLANG_BINARIES=OFF ENABLE_OPT=OFF BUILD_TESTING=OFF)

hunter_config(astc-encoder VERSION 1.3-a47b80f-p1)

hunter_config(VulkanMemoryAllocator VERSION 2.3.0-p0)

hunter_config(cgltf VERSION 1.10-p0)
