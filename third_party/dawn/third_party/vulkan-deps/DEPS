# This file is used to manage Vulkan dependencies for several repos. It is
# used by gclient to determine what version of each dependency to check out, and
# where.

# Avoids the need for a custom root variable.
use_relative_paths = True
git_dependencies = 'SYNC'

vars = {
  'chromium_git': 'https://chromium.googlesource.com',

  # Current revision of glslang, the Khronos SPIRV compiler.
  'glslang_revision': '8a6b381c9afbfaf9fff851582539fab15ea480c0',

  # Current revision of Lunarg VulkanTools
  'lunarg_vulkantools_revision': 'a15ec3f54859037512aac22e58ea45aef09cde40',

  # Current revision of spirv-cross, the Khronos SPIRV cross compiler.
  'spirv_cross_revision': 'b8fcf307f1f347089e3c46eb4451d27f32ebc8d3',

  # Current revision fo the SPIRV-Headers Vulkan support library.
  'spirv_headers_revision': '09913f088a1197aba4aefd300a876b2ebbaa3391',

  # Current revision of SPIRV-Tools for Vulkan.
  'spirv_tools_revision': '682bcd51548e670811f1d03511968bb59a1157ce',

  # Current revision of Khronos Vulkan-Headers.
  'vulkan_headers_revision': '234c4b7370a8ea3239a214c9e871e4b17c89f4ab',

  # Current revision of Khronos Vulkan-Loader.
  'vulkan_loader_revision': 'fde0f9718bd60b49cf8efc80d3fb7a093c309ac0',

  # Current revision of Khronos Vulkan-Tools.
  'vulkan_tools_revision': 'a9f88a232a4cd6634e47f0f93c8fac5789387a7a',

  # Current revision of Khronos Vulkan-Utility-Libraries.
  'vulkan_utility_libraries_revision': 'fe7a09b13899c5c77d956fa310286f7a7eb2c4ed',

  # Current revision of Khronos Vulkan-ValidationLayers.
  'vulkan_validation_revision': 'e00009bc3f550898c92e96a77ae4dca963778301',
}

deps = {
  'glslang/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/glslang@{glslang_revision}',
  },

  'lunarg-vulkantools/src': {
    'url': '{chromium_git}/external/github.com/LunarG/VulkanTools@{lunarg_vulkantools_revision}',
  },

  'spirv-cross/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Cross@{spirv_cross_revision}',
  },

  'spirv-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Headers@{spirv_headers_revision}',
  },

  'spirv-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Tools@{spirv_tools_revision}',
  },

  'vulkan-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Headers@{vulkan_headers_revision}',
  },

  'vulkan-loader/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Loader@{vulkan_loader_revision}',
  },

  'vulkan-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Tools@{vulkan_tools_revision}',
  },

  'vulkan-utility-libraries/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Utility-Libraries@{vulkan_utility_libraries_revision}',
  },

  'vulkan-validation-layers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-ValidationLayers@{vulkan_validation_revision}',
  },
}
