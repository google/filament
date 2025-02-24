/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

#include <vulkan/vulkan.h>
#include "gpuav/debug_printf/debug_printf.h"
#include "chassis/dispatch_object.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/core/gpuav.h"
#include "state_tracker/shader_instruction.h"
#include "error_message/spirv_logging.h"

#include <iostream>

namespace gpuav {
namespace debug_printf {

enum NumericType {
    NumericTypeUnknown = 0,
    NumericTypeFloat = 1,
    NumericTypeSint = 2,
    NumericTypeUint = 4,
};

static NumericType NumericTypeLookup(char specifier) {
    switch (specifier) {
        case 'd':
        case 'i':
            return NumericTypeSint;
            break;

        case 'f':
        case 'F':
        case 'a':
        case 'A':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
            return NumericTypeFloat;
            break;

        case 'u':
        case 'x':
        case 'o':
        default:
            return NumericTypeUint;
            break;
    }
}

struct Substring {
    std::string string;
    bool needs_value = false;  // if value from buffer needed to print arguments
    NumericType type = NumericTypeUnknown;
    bool is_64_bit = false;
};

static std::vector<Substring> ParseFormatString(const std::string &format_string) {
    const char types[] = {'d', 'i', 'o', 'u', 'x', 'X', 'a', 'A', 'e', 'E', 'f', 'F', 'g', 'G', 'v', '\0'};
    std::vector<Substring> parsed_strings;
    size_t pos = 0;
    size_t begin = 0;
    size_t percent = 0;

    while (begin < format_string.length()) {
        Substring substring;

        // Find a percent sign
        pos = percent = format_string.find_first_of('%', pos);
        if (pos == std::string::npos) {
            // End of the format string   Push the rest of the characters
            substring.string = format_string.substr(begin, format_string.length());
            parsed_strings.emplace_back(substring);
            break;
        }
        pos++;
        if (format_string[pos] == '%') {
            pos++;
            continue;  // %% - skip it
        }
        // Find the type of the value
        pos = format_string.find_first_of(types, pos);
        if (pos == format_string.npos) {
            // This really shouldn't happen with a legal value string
            pos = format_string.length();
        } else {
            substring.needs_value = true;

            // We are just taking vector and creating a list of scalar that snprintf can handle
            if (format_string[pos] == 'v') {
                // Vector must be of size 2, 3, or 4
                // and format %v<size><type>
                std::string specifier = format_string.substr(percent, pos - percent);
                const int vec_size = atoi(&format_string[pos + 1]);
                pos += 2;

                // skip v<count>, handle long
                specifier.push_back(format_string[pos]);
                if (format_string[pos + 1] == 'l') {
                    // catches %ul
                    substring.is_64_bit = true;
                    specifier.push_back('l');
                    pos++;
                } else if (format_string[pos] == 'l') {
                    // catches %lu and lx
                    substring.is_64_bit = true;
                    specifier.push_back(format_string[pos + 1]);
                    pos++;
                }

                // Take the preceding characters, and the percent through the type
                substring.string = format_string.substr(begin, percent - begin);
                substring.string += specifier;
                substring.type = NumericTypeLookup(specifier.back());
                parsed_strings.emplace_back(substring);

                // Continue with a comma separated list
                char temp_string[32];
                snprintf(temp_string, sizeof(temp_string), ", %s", specifier.c_str());
                substring.string = temp_string;
                for (int i = 0; i < (vec_size - 1); i++) {
                    parsed_strings.emplace_back(substring);
                }
            } else {
                // Single non-vector value
                if (format_string[pos - 1] == 'l') {
                    substring.is_64_bit = true;  // finds %lu since we skipped the 'l' to find the 'u'
                } else if (format_string[pos + 1] == 'l') {
                    substring.is_64_bit = true;
                    pos++;  // Save long size
                }
                substring.string = format_string.substr(begin, pos - begin + 1);
                substring.type = NumericTypeLookup(format_string[pos]);
                parsed_strings.emplace_back(substring);
            }
            begin = pos + 1;
        }
    }
    return parsed_strings;
}

// GCC and clang don't like using variables as format strings in sprintf.
// #pragma GCC is recognized by both compilers
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

// The contents each "printf" is writting to the output buffer streams
struct OutputRecord {
    uint32_t size;
    uint32_t shader_id;
    uint32_t instruction_position;
    uint32_t format_string_id;
    uint32_t double_bitmask;     // used to distinguish if float is 1 or 2 dwords
    uint32_t signed_8_bitmask;   // used to distinguish if signed int is a int8_t
    uint32_t signed_16_bitmask;  // used to distinguish if signed int is a int16_t
    uint32_t stage_id;
    uint32_t stage_info_0;
    uint32_t stage_info_1;
    uint32_t stage_info_2;
    uint32_t values;  // place holder to be casted to get rest of items in record
};

void AnalyzeAndGenerateMessage(Validator &gpuav, VkCommandBuffer command_buffer, VkQueue queue,
                               gpuav::DebugPrintfBufferInfo &buffer_info, uint32_t *const debug_output_buffer,
                               const Location &loc) {
    uint32_t output_buffer_dwords_counts = debug_output_buffer[gpuav::kDebugPrintfOutputBufferDWordsCount];
    if (!output_buffer_dwords_counts) return;

    uint32_t output_record_i = gpuav::kDebugPrintfOutputBufferData;  // get first OutputRecord index
    while (debug_output_buffer[output_record_i]) {
        std::stringstream shader_message;

        OutputRecord *debug_record = reinterpret_cast<OutputRecord *>(&debug_output_buffer[output_record_i]);
        // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
        // by the instrumented shader.
        const gpuav::InstrumentedShader *instrumented_shader = nullptr;
        auto it = gpuav.instrumented_shaders_map_.find(debug_record->shader_id);
        if (it != gpuav.instrumented_shaders_map_.end()) {
            instrumented_shader = &it->second;
        }

        // without the instrumented spirv, there is nothing valuable to print out
        if (!instrumented_shader || instrumented_shader->instrumented_spirv.empty()) {
            gpuav.InternalWarning(queue, loc, "Can't find instructions from any handles in shader_map");
            return;
        }

        // Search through the shader source for the printf format string for this invocation
        std::string format_string = ::spirv::GetOpString(instrumented_shader->instrumented_spirv, debug_record->format_string_id);

        if (format_string.empty()) {
            // We have plumbed the OpString from the instrumented shader
            for (auto debug_instrumented_info : gpuav.intenral_only_debug_printf_) {
                if ((debug_instrumented_info.unique_shader_id == debug_record->shader_id) &&
                    (debug_record->format_string_id == debug_instrumented_info.op_string_id)) {
                    format_string = debug_instrumented_info.op_string_text;
                    break;
                }
            }
        }

        // Break the format string into strings with 1 or 0 value
        auto format_substrings = ParseFormatString(format_string);
        void *current_value = static_cast<void *>(&debug_record->values);
        // Sprintf each format substring into a temporary string then add that to the message
        for (size_t substring_i = 0; substring_i < format_substrings.size(); substring_i++) {
            auto &substring = format_substrings[substring_i];
            std::string temp_string;
            size_t needed = 0;

            if (substring.needs_value) {
                if (substring.is_64_bit) {
                    if (substring.type == NumericTypeUint) {
                        std::array<std::string_view, 3> format_strings = {{"%ul", "%lu", "%lx"}};
                        for (const auto &ul_string : format_strings) {
                            size_t ul_pos = substring.string.find(ul_string);
                            if (ul_pos == std::string::npos) continue;
                            if (ul_string != "%lu") {
                                substring.string.replace(ul_pos + 1, 2, PRIx64);
                            } else {
                                substring.string.replace(ul_pos + 1, 2, PRIu64);
                            }
                            break;
                        }

                        const uint64_t value = *static_cast<uint64_t *>(current_value);
                        // +1 for null terminator
                        needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                        temp_string.resize(needed);
                        std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                    } else if (substring.type == NumericTypeSint) {
                        size_t ld_pos = substring.string.find("%ld");
                        if (ld_pos != std::string::npos) {
                            substring.string.replace(ld_pos + 1, 2, PRId64);
                        } else {
                            gpuav.InternalWarning(command_buffer, loc,
                                                  "Trying to DebugPrintf a 64-bit signed int but not using \"%%ld\" to print it.");
                        }

                        const uint32_t *current_ptr = static_cast<uint32_t *>(current_value);
                        const uint32_t low = *current_ptr;
                        const uint32_t high = *(current_ptr + 1);
                        // Need to shift into uint before casting to signed int to avoid undefined behavior
                        // https://learn.microsoft.com/en-us/cpp/cpp/left-shift-and-right-shift-operators-input-and-output?view=msvc-170#footnotes
                        const uint64_t value_unsigned = (static_cast<uint64_t>(high) << 32) | low;
                        const int64_t value = static_cast<int64_t>(value_unsigned);

                        needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                        temp_string.resize(needed);
                        std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                    } else {
                        assert(false);  // non-supported type
                    }
                } else {
                    if (substring.type == NumericTypeUint) {
                        // +1 for null terminator
                        const uint32_t value = *static_cast<uint32_t *>(current_value);
                        needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                        temp_string.resize(needed);
                        std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);

                    } else if (substring.type == NumericTypeSint) {
                        // When dealing with signed int, we need to know which size the int was to print the correct value
                        if (debug_record->signed_8_bitmask & (1 << substring_i)) {
                            const int8_t value = *static_cast<int8_t *>(current_value);
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                        } else if (debug_record->signed_16_bitmask & (1 << substring_i)) {
                            const int16_t value = *static_cast<int16_t *>(current_value);
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                        } else {
                            const int32_t value = *static_cast<int32_t *>(current_value);
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                        }

                    } else if (substring.type == NumericTypeFloat) {
                        // On the CPU printf the "%f" is used for 16, 32, and 64-bit floats,
                        // but we need to store the 64-bit floats in 2 dwords in our GPU side buffer.
                        // Using the bitmask, we know if the incoming float was 64-bit or not.
                        // This is much simpler than enforcing a %lf which doesn't line up with how the CPU side works
                        if (debug_record->double_bitmask & (1 << substring_i)) {
                            substring.is_64_bit = true;
                            const double value = *static_cast<double *>(current_value);
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                        } else {
                            const float value = *static_cast<float *>(current_value);
                            needed = std::snprintf(nullptr, 0, substring.string.c_str(), value) + 1;
                            temp_string.resize(needed);
                            std::snprintf(&temp_string[0], needed, substring.string.c_str(), value);
                        }
                    }
                }

                const uint32_t offset = substring.is_64_bit ? 2 : 1;
                current_value = static_cast<uint32_t *>(current_value) + offset;

            } else {
                // incase where someone just printing a string with no arguments to it
                needed = std::snprintf(nullptr, 0, substring.string.c_str()) + 1;
                temp_string.resize(needed);
                std::snprintf(&temp_string[0], needed, substring.string.c_str());
            }

            shader_message << temp_string.c_str();
        }

        const bool use_stdout = gpuav.gpuav_settings.debug_printf_to_stdout;
        if (gpuav.gpuav_settings.debug_printf_verbose) {
            std::string debug_info_message = gpuav.GenerateDebugInfoMessage(
                command_buffer, debug_record->stage_id, debug_record->stage_info_0, debug_record->stage_info_1,
                debug_record->stage_info_2, debug_record->instruction_position, instrumented_shader, debug_record->shader_id,
                buffer_info.pipeline_bind_point, buffer_info.action_command_index);
            if (use_stdout) {
                std::cout << "VVL-DEBUG-PRINTF " << shader_message.str() << '\n' << debug_info_message;
            } else {
                LogObjectList objlist(queue, command_buffer);
                gpuav.LogInfo("VVL-DEBUG-PRINTF", objlist, loc, "%s\n%s", shader_message.str().c_str(), debug_info_message.c_str());
            }

        } else {
            if (use_stdout) {
                std::cout << shader_message.str();
            } else {
                // LogInfo will print out a lot of extra information (Object handles, VUID, hash, etc)
                // If the user doesn't set "verbose", but wants to use the debug callback, we should limit it to the bare minimum
                gpuav.debug_report->DebugLogMsg(kInformationBit, {}, shader_message.str().c_str(), "VVL-DEBUG-PRINTF");
            }
        }
        output_record_i += debug_record->size;
    }
    if ((output_record_i - gpuav::kDebugPrintfOutputBufferData) < output_buffer_dwords_counts) {
        std::stringstream message;
        message << "Debug Printf message was truncated due to a buffer size (" << gpuav.gpuav_settings.debug_printf_buffer_size
                << ") being too small for the messages. (This can be adjusted with VK_LAYER_PRINTF_BUFFER_SIZE or vkconfig)";
        gpuav.InternalWarning(command_buffer, loc, message.str().c_str());
    }

    // Only memset what is needed, in case we are only using a small portion of a large buffer_size.
    // At the same time we want to make sure we don't memset past the actual VkBuffer allocation
    uint32_t clear_size =
        sizeof(uint32_t) * (debug_output_buffer[gpuav::kDebugPrintfOutputBufferDWordsCount] + gpuav::kDebugPrintfOutputBufferData);
    clear_size = std::min(gpuav.gpuav_settings.debug_printf_buffer_size, clear_size);
    memset(debug_output_buffer, 0, clear_size);
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

bool UpdateInstrumentationDescSet(Validator &gpuav, CommandBuffer &cb_state, VkDescriptorSet instrumentation_desc_set,
                                  VkPipelineBindPoint bind_point, const Location &loc) {
    gpuav::vko::Buffer debug_printf_output_buffer(gpuav);

    // Allocate memory for the output block that the gpu will use to return values for printf
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = gpuav.gpuav_settings.debug_printf_buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    alloc_info.pool = gpuav.output_buffer_pool_;
    const bool success = debug_printf_output_buffer.Create(loc, &buffer_info, &alloc_info);
    if (!success) {
        return false;
    }

    // Clear the output block to zeros so that only printf values from the gpu will be present
    auto printf_output_ptr = (uint32_t *)debug_printf_output_buffer.MapMemory(loc);
    memset(printf_output_ptr, 0, gpuav.gpuav_settings.debug_printf_buffer_size);
    debug_printf_output_buffer.UnmapMemory();

    VkDescriptorBufferInfo debug_printf_desc_buffer_info = {};
    debug_printf_desc_buffer_info.range = gpuav.gpuav_settings.debug_printf_buffer_size;
    debug_printf_desc_buffer_info.buffer = debug_printf_output_buffer.VkHandle();
    debug_printf_desc_buffer_info.offset = 0;

    VkWriteDescriptorSet wds = vku::InitStructHelper();
    wds.dstBinding = glsl::kBindingInstDebugPrintf;
    wds.descriptorCount = 1;
    wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    wds.pBufferInfo = &debug_printf_desc_buffer_info;
    wds.dstSet = instrumentation_desc_set;

    DispatchUpdateDescriptorSets(gpuav.device, 1, &wds, 0, nullptr);

    cb_state.debug_printf_buffer_infos.emplace_back(debug_printf_output_buffer, bind_point, cb_state.action_command_count);
    return true;
}

}  // namespace debug_printf
}  // namespace gpuav
