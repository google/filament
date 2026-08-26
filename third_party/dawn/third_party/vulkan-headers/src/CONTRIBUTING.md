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
* CONTRIBUTING.md
* LICENSE.txt
* README.md
* Non-API headers
  * include/vulkan/vk_icd.h
  * include/vulkan/vk_layer.h

### Specification repository (https://github.com/KhronosGroup/Vulkan-Docs)

* registry/*.py
* registry/*.xml
* registry/spec_tools/*.py
* registry/profiles/*.json
* All files under include/vulkan/ which are *not* listed explicitly as originating from another repository.

### Vulkan C++ Binding Repository (https://github.com/KhronosGroup/Vulkan-Hpp)

As of the Vulkan-Docs 1.2.182 spec update, the Vulkan-Hpp headers have been
split into multiple files. All of those files are now included in this
repository.

* include/vulkan/*.hpp
* include/vulkan/*.cppm

### **Contributor License Agreement (CLA)**

You will be prompted with a one-time "click-through" CLA dialog as part of submitting your pull request
or other contribution to GitHub.

### **AI-Assisted Contributions**

By submitting a Contribution to this repository, you additionally represent 
that, to the extent any of Your Contributions were developed with the 
assistance of artificial intelligence tools or AI-generated code, You have 
exercised sufficient review, judgment, and creative direction over such tools 
and resulting material to reasonably consider it Your original creation, and 
You are not aware of any third-party license, intellectual property claim, or 
other restriction arising from such use that is associated with any part of 
Your Contribution or use thereof.
