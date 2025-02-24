<!--
Copyright 2023 The Khronos Group Inc.
Copyright 2023 Valve Corporation
Copyright 2023 LunarG, Inc.

SPDX-License-Identifier: Apache-2.0
-->

# Library integration testing

In order to avoid disruption of downstream users. It's important to test how this
repository is consumed.

1. Self contained headers

It's easy to write header files that aren't self contained. By compiling 
a single source file that includes a single header we ensure a smooth experience for
downstream users.

2. Ensure C compatibility of C header files

It's VERY easy to write invalid C code. Especially for experience C++ programmers.

## tests/find_package

Test find_package support. The intent is to ensure we properly install files.

Used by system/language package managers and the Vulkan SDK packaging.

## tests/add_subdirectory

1. Test add_subdirectory support

While we don't have to support add_subdirectory it is a common feature request for CMake projects.

2. Ensure file name consistency of header files we install

All header files we ship will have the `vk_` prefix

This convention was originally established in VulkanHeaders for files created by LunarG. 
- EX: `vk_icd.h`, `vk_layer.h`, `vk_platform.h`
