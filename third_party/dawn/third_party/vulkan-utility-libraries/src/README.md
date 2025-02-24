<!--
Copyright 2023 The Khronos Group Inc.
Copyright 2023 Valve Corporation
Copyright 2023 LunarG, Inc.

SPDX-License-Identifier: Apache-2.0
-->

# Vulkan-Utility-Libraries

This repo was created to share code across various Vulkan repositories, solving long standing issues for Vulkan 
SDK developers and users.

## Historical Context

The `Vulkan-ValidationLayers` contained many libraries and utilities that were useful for other Vulkan repositories, and became the primary mechanism for code sharing in the Vulkan ecosystem.

This caused the `Vulkan-ValidationLayers` to have to maintain and export source code which was never intended for that purpose. This not only hindered development of the `Vulkan-ValidationLayers`, but would frequently break anyone depending on the source code due to the poorly located nature of it. On top of numerous other issues.

This repository was created to facilitate official source deliverables that can be reliably used by developers.

## Vulkan::LayerSettings

The `Vulkan::LayerSettings` library was created to standardize layer configuration code for various SDK layer deliverables.

- [Vulkan Validation Layers](https://github.com/KhronosGroup/Vulkan-ValidationLayers)
- [Vulkan Extension Layer](https://github.com/KhronosGroup/Vulkan-ExtensionLayer/)
- [Vulkan Profiles](https://github.com/KhronosGroup/Vulkan-Profiles)
- [LunarG Tool Layers](https://github.com/LunarG/VulkanTools)
    EX: `VK_LAYER_LUNARG_api_dump`, `VK_LAYER_LUNARG_screenshot` and `VK_LAYER_LUNARG_monitor`

This is to ensure they all worked consistently with the 3 main methods of layer configuration.

For more information see [layer_configuration.md](docs/layer_configuration.md).

## Vulkan::UtilityHeaders

The `Vulkan::UtilityHeaders` library contains header only files that provide useful functionality to developers:

- `vk_dispatch_table.h`: Initializing instance/device dispatch tables
- `vk_format_utils.h`: Utilities for `VkFormat`
- `vk_struct_helper.hpp`: Utilities for vulkan structs
- `vk_enum_string_helper.h`: Converts Vulkan enums into strings
