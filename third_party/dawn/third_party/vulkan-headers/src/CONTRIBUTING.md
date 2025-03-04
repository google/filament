<!--
Copyright 2018-2023 The Khronos Group Inc.

SPDX-License-Identifier: Apache-2.0
-->

# CONTRIBUTING

Please note when contributing what files this repository actually is responsible for.

The majority for the Vulkan headers come from [Vulkan-Docs](https://github.com/KhronosGroup/Vulkan-Docs) or [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp)

### This repository (https://github.com/KhronosGroup/Vulkan-Headers)

* BUILD.gn
* BUILD.md
* CMakeLists.txt
* tests/*
* CODE_OF_CONDUCT.md
* LICENSE.txt
* README.md
* Non-API headers
  * include/vulkan/vk_icd.h
  * include/vulkan/vk_layer.h

### Specification repository (https://github.com/KhronosGroup/Vulkan-Docs)

* registry/*.py
* registry/spec_tools/*.py
* registry/profiles/*.json
* All files under include/vulkan/ which are *not* listed explicitly as originating from another repository.

### Vulkan C++ Binding Repository (https://github.com/KhronosGroup/Vulkan-Hpp)

As of the Vulkan-Docs 1.2.182 spec update, the Vulkan-Hpp headers have been
split into multiple files. All of those files are now included in this
repository.

* include/vulkan/*.hpp
* include/vulkan/*.cppm
