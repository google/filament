#!/usr/bin/env python3 -i
#
# Copyright 2021-2026 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

# Generic alias for working group-specific API conventions interface.

# This import should be changed at the repository / working group level to
# specify the correct API's conventions.


import os

defaultAPI = 'vulkan'

VulkanAPI = os.getenv('VULKAN_API', default=defaultAPI)

if VulkanAPI == 'vulkansc':
    from vkconventions import VulkanSCConventions as APIConventions
elif VulkanAPI == 'vulkanbase':
    from vkconventions import VulkanBaseConventions as APIConventions
else:
    from vkconventions import VulkanConventions as APIConventions
