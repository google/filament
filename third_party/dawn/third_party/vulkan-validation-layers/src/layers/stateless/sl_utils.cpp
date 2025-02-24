/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
 */

#include "stateless/stateless_validation.h"

namespace stateless {
bool Instance::CheckPromotedApiAgainstVulkanVersion(VkInstance instance, const Location &loc,
                                                    const uint32_t promoted_version) const {
    bool skip = false;
    if (api_version < promoted_version) {
        skip |= LogError("UNASSIGNED-API-Version-Violation", instance, loc,
                         "Attempted to call with an effective API version of %s"
                         "but this API was not promoted until version %s.",
                         StringAPIVersion(api_version).c_str(), StringAPIVersion(promoted_version).c_str());
    }
    return skip;
}

bool Instance::CheckPromotedApiAgainstVulkanVersion(VkPhysicalDevice pdev, const Location &loc,
                                                    const uint32_t promoted_version) const {
    bool skip = false;
    const auto &target_pdev = physical_device_properties_map.find(pdev);
    if (target_pdev != physical_device_properties_map.end()) {
        auto effective_api_version = std::min(APIVersion(target_pdev->second->apiVersion), api_version);
        if (effective_api_version < promoted_version) {
            skip |= LogError(
                "UNASSIGNED-API-Version-Violation", instance, loc,
                "Attempted to call with an effective API version of %s, "
                "which is the minimum of version requested in pApplicationInfo (%s) and supported by this physical device (%s), "
                "but this API was not promoted until version %s.",
                StringAPIVersion(effective_api_version).c_str(), StringAPIVersion(api_version).c_str(),
                StringAPIVersion(target_pdev->second->apiVersion).c_str(), StringAPIVersion(promoted_version).c_str());
        }
    }
    return skip;
}

bool Instance::OutputExtensionError(const Location &loc, const vvl::Extensions &exentsions) const {
    return LogError("UNASSIGNED-GeneralParameterError-ExtensionNotEnabled", instance, loc,
                    "function required extension %s which has not been enabled.\n", String(exentsions).c_str());
}
bool Device::OutputExtensionError(const Location &loc, const vvl::Extensions &exentsions) const {
    return LogError("UNASSIGNED-GeneralParameterError-ExtensionNotEnabled", device, loc,
                    "function required extension %s which has not been enabled.\n", String(exentsions).c_str());
}

static const uint8_t kUtF8OneByteCode = 0xC0;
static const uint8_t kUtF8OneByteMask = 0xE0;
static const uint8_t kUtF8TwoByteCode = 0xE0;
static const uint8_t kUtF8TwoByteMask = 0xF0;
static const uint8_t kUtF8ThreeByteCode = 0xF0;
static const uint8_t kUtF8ThreeByteMask = 0xF8;
static const uint8_t kUtF8DataByteCode = 0x80;
static const uint8_t kUtF8DataByteMask = 0xC0;

static VkStringErrorFlags ValidateVkString(const int max_length, const char *utf8) {
    VkStringErrorFlags result = VK_STRING_ERROR_NONE;
    int num_char_bytes = 0;
    int i, j;

    for (i = 0; i <= max_length; i++) {
        if (utf8[i] == 0) {
            break;
        } else if (i == max_length) {
            result |= VK_STRING_ERROR_LENGTH;
            break;
        } else if ((utf8[i] >= 0xa) && (utf8[i] < 0x7f)) {
            num_char_bytes = 0;
        } else if ((utf8[i] & kUtF8OneByteMask) == kUtF8OneByteCode) {
            num_char_bytes = 1;
        } else if ((utf8[i] & kUtF8TwoByteMask) == kUtF8TwoByteCode) {
            num_char_bytes = 2;
        } else if ((utf8[i] & kUtF8ThreeByteMask) == kUtF8ThreeByteCode) {
            num_char_bytes = 3;
        } else {
            result |= VK_STRING_ERROR_BAD_DATA;
            break;
        }

        // Validate the following num_char_bytes of data
        for (j = 0; (j < num_char_bytes) && (i < max_length); j++) {
            if (++i == max_length) {
                result |= VK_STRING_ERROR_LENGTH;
                break;
            }
            if ((utf8[i] & kUtF8DataByteMask) != kUtF8DataByteCode) {
                result |= VK_STRING_ERROR_BAD_DATA;
                break;
            }
        }
        if (result != VK_STRING_ERROR_NONE) break;
    }
    return result;
}

static const int kMaxParamCheckerStringLength = 256;
bool Context::ValidateString(const Location &loc, const char *vuid, const char *validate_string) const {
    bool skip = false;

    VkStringErrorFlags result = ValidateVkString(kMaxParamCheckerStringLength, validate_string);

    if (result == VK_STRING_ERROR_NONE) {
        return skip;
    } else if (result & VK_STRING_ERROR_LENGTH) {
        skip |= log.LogError(vuid, error_obj.handle, loc, "exceeds max length %" PRIu32 ".", kMaxParamCheckerStringLength);
    } else if (result & VK_STRING_ERROR_BAD_DATA) {
        skip |= log.LogError(vuid, error_obj.handle, loc, "contains invalid characters or is badly formed.");
    }
    return skip;
}

bool Context::ValidateNotZero(bool is_zero, const char *vuid, const Location &loc) const {
    bool skip = false;
    if (is_zero) {
        skip |= log.LogError(vuid, error_obj.handle, loc, "is zero.");
    }
    return skip;
}

bool Context::ValidateRequiredPointer(const Location &loc, const void *value, const char *vuid) const {
    bool skip = false;
    if (value == nullptr) {
        skip |= log.LogError(vuid, error_obj.handle, loc, "is NULL.");
    }
    return skip;
}

bool Context::ValidateAllocationCallbacks(const VkAllocationCallbacks &callback, const Location &loc) const {
    bool skip = false;
    skip |= ValidateRequiredPointer(loc.dot(Field::pfnAllocation), reinterpret_cast<const void *>(callback.pfnAllocation),
                                    "VUID-VkAllocationCallbacks-pfnAllocation-00632");

    skip |= ValidateRequiredPointer(loc.dot(Field::pfnReallocation), reinterpret_cast<const void *>(callback.pfnReallocation),
                                    "VUID-VkAllocationCallbacks-pfnReallocation-00633");

    skip |= ValidateRequiredPointer(loc.dot(Field::pfnFree), reinterpret_cast<const void *>(callback.pfnFree),
                                    "VUID-VkAllocationCallbacks-pfnFree-00634");

    if (callback.pfnInternalAllocation) {
        skip |=
            ValidateRequiredPointer(loc.dot(Field::pfnInternalAllocation), reinterpret_cast<const void *>(callback.pfnInternalFree),
                                    "VUID-VkAllocationCallbacks-pfnInternalAllocation-00635");
    }

    if (callback.pfnInternalFree) {
        skip |=
            ValidateRequiredPointer(loc.dot(Field::pfnInternalFree), reinterpret_cast<const void *>(callback.pfnInternalAllocation),
                                    "VUID-VkAllocationCallbacks-pfnInternalAllocation-00635");
    }
    return skip;
}

bool Context::ValidateStringArray(const Location &count_loc, const Location &array_loc, uint32_t count, const char *const *array,
                                  bool count_required, bool array_required, const char *count_required_vuid,
                                  const char *array_required_vuid) const {
    bool skip = false;

    if ((array == nullptr) || (count == 0)) {
        skip |= ValidateArray(count_loc, array_loc, count, &array, count_required, array_required, count_required_vuid,
                              array_required_vuid);
    } else {
        // Verify that strings in the array are not NULL
        for (uint32_t i = 0; i < count; ++i) {
            if (array[i] == nullptr) {
                skip |= log.LogError(array_required_vuid, error_obj.handle, array_loc.dot(i), "is NULL.");
            }
        }
    }

    return skip;
}

bool Context::ValidateStructPnext(const Location &loc, const void *next, size_t allowed_type_count,
                                  const VkStructureType *allowed_types, uint32_t header_version, const char *pnext_vuid,
                                  const char *stype_vuid, const bool is_const_param) const {
    bool skip = false;

    if (next != nullptr) {
        vvl::unordered_set<const void *> cycle_check;
        vvl::unordered_set<VkStructureType, vvl::hash<int>> unique_stype_check;
        const char *disclaimer =
            "This error is based on the Valid Usage documentation for version %" PRIu32
            " of the Vulkan header.  It is possible that "
            "you are using a struct from a private extension or an extension that was added to a later version of the Vulkan "
            "header, in which case the use of %s is undefined and may not work correctly with validation enabled";

        const Location pNext_loc = loc.dot(Field::pNext);
        if ((allowed_type_count == 0) && (GetCustomStypeInfo().empty())) {
            std::string message = "must be NULL. ";
            message += disclaimer;
            skip |=
                log.LogError(pnext_vuid, error_obj.handle, pNext_loc, message.c_str(), header_version, pNext_loc.Fields().c_str());
        } else {
            const VkStructureType *start = allowed_types;
            const VkStructureType *end = allowed_types + allowed_type_count;
            const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(next);

            while (current != nullptr) {
                if ((loc.function != Func::vkCreateInstance || (current->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO)) &&
                        (loc.function != Func::vkCreateDevice || (current->sType != VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO))) {
                    std::string type_name = string_VkStructureType(current->sType);
                    if (unique_stype_check.find(current->sType) != unique_stype_check.end() && !IsDuplicatePnext(current->sType)) {
                        // stype_vuid will only be null if there are no listed pNext and will hit disclaimer check
                        skip |=
                            log.LogError(stype_vuid, error_obj.handle, pNext_loc,
                                    "chain contains duplicate structure types: %s appears multiple times.", type_name.c_str());
                    } else {
                        unique_stype_check.insert(current->sType);
                    }

                    // Search custom stype list -- if sType found, skip this entirely
                    bool custom = false;
                    for (const auto &item : GetCustomStypeInfo()) {
                        if (item.first == current->sType) {
                            custom = true;
                            break;
                        }
                    }
                    if (!custom) {
                        if (std::find(start, end, current->sType) == end) {
                            // String returned by string_VkStructureType for an unrecognized type.
                            if (type_name.compare("Unhandled VkStructureType") == 0) {
                                std::string message = "chain includes a structure with unknown VkStructureType (%" PRIu32 "). ";
                                message += disclaimer;
                                skip |= log.LogError(pnext_vuid, error_obj.handle, pNext_loc, message.c_str(), current->sType,
                                        header_version, pNext_loc.Fields().c_str());
                            } else {
                                std::string message = "chain includes a structure with unexpected VkStructureType %s. ";
                                message += disclaimer;
                                skip |= log.LogError(pnext_vuid, error_obj.handle, pNext_loc, message.c_str(), type_name.c_str(),
                                        header_version, pNext_loc.Fields().c_str());
                            }
                        }
                        // Send Location without pNext field so the pNext() connector can be used
                        skip |= ValidatePnextStructContents(loc, current, pnext_vuid, is_const_param);
                        // pNext contents for vkGetPhysicalDeviceProperties2KHR() is no longer checked.
                        if (loc.function == Func::vkGetPhysicalDeviceFeatures2 ||
                            loc.function == Func::vkGetPhysicalDeviceFeatures2KHR || loc.function == Func::vkCreateDevice) {
                            skip |= ValidatePnextFeatureStructContents(loc, current, pnext_vuid, is_const_param);
                        }
                    }
                }
                current = reinterpret_cast<const VkBaseOutStructure *>(current->pNext);
            }
        }
    }

    return skip;
}

bool Context::ValidateBool32(const Location &loc, VkBool32 value) const {
    bool skip = false;
    if ((value != VK_TRUE) && (value != VK_FALSE)) {
        skip |= log.LogError("UNASSIGNED-GeneralParameterError-UnrecognizedBool32", error_obj.handle, loc,
                             "(%" PRIu32
                             ") is neither VK_TRUE nor VK_FALSE. Applications MUST not pass any other "
                             "values than VK_TRUE or VK_FALSE into a Vulkan implementation where a VkBool32 is expected.",
                             value);
    }
    return skip;
}

bool Context::ValidateBool32Array(const Location &count_loc, const Location &array_loc, uint32_t count, const VkBool32 *array,
                                  bool count_required, bool array_required, const char *count_required_vuid,
                                  const char *array_required_vuid) const {
    bool skip = false;

    if ((array == nullptr) || (count == 0)) {
        skip |= ValidateArray(count_loc, array_loc, count, &array, count_required, array_required, count_required_vuid,
                              array_required_vuid);
    } else {
        for (uint32_t i = 0; i < count; ++i) {
            if ((array[i] != VK_TRUE) && (array[i] != VK_FALSE)) {
                skip |= log.LogError(array_required_vuid, error_obj.handle, array_loc.dot(i),
                                     "(%" PRIu32
                                     ") is neither VK_TRUE nor VK_FALSE. Applications MUST not pass any other "
                                     "values than VK_TRUE or VK_FALSE into a Vulkan implementation where a VkBool32 is expected.",
                                     array[i]);
            }
        }
    }

    return skip;
}

bool Context::ValidateReservedFlags(const Location &loc, VkFlags value, const char *vuid) const {
    bool skip = false;
    if (value != 0) {
        skip |= log.LogError(vuid, error_obj.handle, loc, "is %" PRIu32 ", but must be 0.", value);
    }
    return skip;
}

// helper to implement validation of both 32 bit and 64 bit flags.
template <typename FlagTypedef>
bool Context::ValidateFlagsImplementation(const Location &loc, vvl::FlagBitmask flag_bitmask, FlagTypedef all_flags,
                                          FlagTypedef value, const FlagType flag_type, const char *vuid,
                                          const char *flags_zero_vuid) const {
    bool skip = false;

    const bool required = flag_type == kRequiredFlags || flag_type == kRequiredSingleBit;
    const char *zero_vuid = flag_type == kRequiredFlags ? flags_zero_vuid : vuid;
    if (required && value == 0) {
        skip |= log.LogError(zero_vuid, error_obj.handle, loc, "is zero.");
    }

    const auto HasMaxOneBitSet = [](const FlagTypedef f) {
        // Decrement flips bits from right upto first 1.
        // Rest stays same, and if there was any other 1s &ded together they would be non-zero. QED
        return f == 0 || !(f & (f - 1));
    };

    const bool is_bits_type = flag_type == kRequiredSingleBit || flag_type == kOptionalSingleBit;
    if (is_bits_type && !HasMaxOneBitSet(value)) {
        skip |= log.LogError(vuid, error_obj.handle, loc, "contains multiple members of %s when only a single value is allowed.",
                             String(flag_bitmask));
    }

    return skip;
}

bool Context::ValidateFlags(const Location &loc, vvl::FlagBitmask flag_bitmask, VkFlags all_flags, VkFlags value,
                            const FlagType flag_type, const char *vuid, const char *flags_zero_vuid) const {
    bool skip = false;
    skip |= ValidateFlagsImplementation<VkFlags>(loc, flag_bitmask, all_flags, value, flag_type, vuid, flags_zero_vuid);

    if (ignore_unknown_enums) {
        return skip;
    }

    if ((value & ~all_flags) != 0) {
        skip |=
            log.LogError(vuid, error_obj.handle, loc, "contains flag bits (0x%" PRIx32 ") which are not recognized members of %s.",
                         value, String(flag_bitmask));
    }

    if (!skip && value != 0) {
        vvl::Extensions required = IsValidFlagValue(flag_bitmask, value);
        if (!required.empty()) {
            skip |=
                log.LogError(vuid, error_obj.handle, loc, "has %s values (%s) that requires the extensions %s.",
                             String(flag_bitmask), DescribeFlagBitmaskValue(flag_bitmask, value).c_str(), String(required).c_str());
        }
    }
    return skip;
}

bool Context::ValidateFlags(const Location &loc, vvl::FlagBitmask flag_bitmask, VkFlags64 all_flags, VkFlags64 value,
                            const FlagType flag_type, const char *vuid, const char *flags_zero_vuid) const {
    bool skip = false;
    skip |= ValidateFlagsImplementation<VkFlags64>(loc, flag_bitmask, all_flags, value, flag_type, vuid, flags_zero_vuid);

    if (ignore_unknown_enums) {
        return skip;
    }

    if ((value & ~all_flags) != 0) {
        skip |=
            log.LogError(vuid, error_obj.handle, loc, "contains flag bits (0x%" PRIx64 ") which are not recognized members of %s.",
                         value, String(flag_bitmask));
    }

    if (!skip && value != 0) {
        vvl::Extensions required = IsValidFlag64Value(flag_bitmask, value);
        if (!required.empty()) {
            skip |= log.LogError(vuid, error_obj.handle, loc, "has %s values (%s) that requires the extensions %s.",
                                 String(flag_bitmask), DescribeFlagBitmaskValue64(flag_bitmask, value).c_str(),
                                 String(required).c_str());
        }
    }
    return skip;
}

bool Context::ValidateFlagsArray(const Location &count_loc, const Location &array_loc, vvl::FlagBitmask flag_bitmask,
                                 VkFlags all_flags, uint32_t count, const VkFlags *array, bool count_required,
                                 const char *count_required_vuid, const char *array_required_vuid) const {
    bool skip = false;

    if ((array == nullptr) || (count == 0)) {
        // Flag arrays always need to have a valid array
        skip |= ValidateArray(count_loc, array_loc, count, &array, count_required, true, count_required_vuid, array_required_vuid);
    } else {
        // Verify that all VkFlags values in the array
        for (uint32_t i = 0; i < count; ++i) {
            if ((array[i] & (~all_flags)) != 0) {
                skip |= log.LogError(array_required_vuid, error_obj.handle, array_loc.dot(i),
                                     "contains flag bits that are not recognized members of %s.", String(flag_bitmask));
            }
        }
    }

    return skip;
}
}  // namespace stateless
