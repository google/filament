//===--- VkConstantsTables.h --- Implict Vulkan Constants Tables ---C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file contains information about implictly-defined vulkan constants.
//  These constants will be added to the AST under the "vk" namespace.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SEMA_VKCONSTANTSTABLES_H
#define LLVM_CLANG_LIB_SEMA_VKCONSTANTSTABLES_H

#include <string>
#include <utility>
#include <vector>

std::vector<std::pair<std::string, uint32_t>> GetVkIntegerConstants() {
  return {
      {"CrossDeviceScope", 0u}, {"DeviceScope", 1u},
      {"WorkgroupScope", 2u},   {"SubgroupScope", 3u},
      {"InvocationScope", 4u},  {"QueueFamilyScope", 5u},
  };
}

#endif // LLVM_CLANG_LIB_SEMA_VKCONSTANTSTABLES_H
