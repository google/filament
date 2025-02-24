// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See pnext_chain_extraction_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/

// NOLINTBEGIN

#pragma once

#include <cassert>
#include <tuple>

#include "vulkan/vulkan.h"

namespace vvl {

// Add element to the end of a pNext chain
void *PnextChainAdd(void *chain, void *new_struct);

// Remove last element from a pNext chain
void PnextChainRemoveLast(void *chain);

// Free dynamically allocated pnext chain structs
void PnextChainFree(void *chain);

// Helper class relying on RAII to help with adding and removing an element from a pNext chain
class PnextChainScopedAdd {
  public:
    PnextChainScopedAdd(void *chain, void *new_struct) : chain(chain) { PnextChainAdd(chain, new_struct); }
    ~PnextChainScopedAdd() { PnextChainRemoveLast(chain); }

  private:
    void *chain = nullptr;
};

// clang-format off

// Utility to make a selective copy of a pNext chain.
// Structs listed in the returned tuple type are the one extending some reference Vulkan structs, like VkPhysicalDeviceImageFormatInfo2.
// The copied structs are the one mentioned in the returned tuple type and found in the pNext chain `in_pnext_chain`.
// In the returned tuple, each struct is NOT a deep copy of the corresponding struct in in_pnext_chain,
// so be mindful of pointers copies.
// The first element of the extracted pNext chain is returned by this function. It can be nullptr.
template <typename T>
void *PnextChainExtract(const void */*in_pnext_chain*/, T &/*out*/) { assert(false); return nullptr; }

// Hereinafter are the available PnextChainExtract functions.
// To add a new one, find scripts/generators/pnext_chain_extraction_generator.py
// and add your reference struct to the `target_structs` list at the beginning of the file.

using PnextChainVkPhysicalDeviceImageFormatInfo2 = std::tuple<
	VkImageCompressionControlEXT,
	VkImageFormatListCreateInfo,
	VkImageStencilUsageCreateInfo,
	VkOpticalFlowImageFormatInfoNV,
	VkPhysicalDeviceExternalImageFormatInfo,
	VkPhysicalDeviceImageDrmFormatModifierInfoEXT,
	VkPhysicalDeviceImageViewImageFormatInfoEXT,
	VkVideoProfileListInfoKHR>;
template <>
void *PnextChainExtract(const void *in_pnext_chain, PnextChainVkPhysicalDeviceImageFormatInfo2 &out);

}
// clang-format on

// NOLINTEND
