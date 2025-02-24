include(CMakeFindDependencyMacro)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../cmake/modules")

find_dependency(PVRCore REQUIRED MODULES)
find_dependency(PVRAssets REQUIRED MODULES)
find_dependency(PVRVk REQUIRED MODULES)
find_dependency(glslang REQUIRED MODULES)
find_dependency(VulkanMemoryAllocator REQUIRED MODULES)

include("${CMAKE_CURRENT_LIST_DIR}/PVRUtilsVkTargets.cmake")