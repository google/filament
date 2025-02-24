/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include "gpuav/resources/gpuav_state_trackers.h"

#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_constants.h"
#include "gpuav/descriptor_validation/gpuav_image_layout.h"
#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/debug_printf/debug_printf.h"

#include "profiling/profiling.h"

namespace gpuav {

Buffer::Buffer(vvl::Device &dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo, DescriptorHeap &desc_heap_)
    : vvl::Buffer(dev_data, buff, pCreateInfo),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(buff, kVulkanObjectTypeBuffer))) {}

void Buffer::Destroy() {
    desc_heap.DeleteId(id);
    vvl::Buffer::Destroy();
}

void Buffer::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::Buffer::NotifyInvalidate(invalid_nodes, unlink);
}

BufferView::BufferView(const std::shared_ptr<vvl::Buffer> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci,
                       VkFormatFeatureFlags2KHR buf_ff, DescriptorHeap &desc_heap_)
    : vvl::BufferView(bf, bv, ci, buf_ff),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(bv, kVulkanObjectTypeBufferView))) {}

void BufferView::Destroy() {
    desc_heap.DeleteId(id);
    vvl::BufferView::Destroy();
}

void BufferView::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::BufferView::NotifyInvalidate(invalid_nodes, unlink);
}

ImageView::ImageView(const std::shared_ptr<vvl::Image> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
                     VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props,
                     DescriptorHeap &desc_heap_)
    : vvl::ImageView(image_state, iv, ci, ff, cubic_props),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(iv, kVulkanObjectTypeImageView))) {}

void ImageView::Destroy() {
    desc_heap.DeleteId(id);
    vvl::ImageView::Destroy();
}

void ImageView::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::ImageView::NotifyInvalidate(invalid_nodes, unlink);
}

Sampler::Sampler(const VkSampler s, const VkSamplerCreateInfo *pci, DescriptorHeap &desc_heap_)
    : vvl::Sampler(s, pci), desc_heap(desc_heap_), id(desc_heap.NextId(VulkanTypedHandle(s, kVulkanObjectTypeSampler))) {}

void Sampler::Destroy() {
    desc_heap.DeleteId(id);
    vvl::Sampler::Destroy();
}

void Sampler::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::Sampler::NotifyInvalidate(invalid_nodes, unlink);
}

AccelerationStructureKHR::AccelerationStructureKHR(VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR *ci,
                                                   std::shared_ptr<vvl::Buffer> &&buf_state, DescriptorHeap &desc_heap_)
    : vvl::AccelerationStructureKHR(as, ci, std::move(buf_state)),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureKHR))) {}

void AccelerationStructureKHR::Destroy() {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureKHR::Destroy();
}

void AccelerationStructureKHR::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureKHR::NotifyInvalidate(invalid_nodes, unlink);
}

AccelerationStructureNV::AccelerationStructureNV(VkDevice device, VkAccelerationStructureNV as,
                                                 const VkAccelerationStructureCreateInfoNV *ci, DescriptorHeap &desc_heap_)
    : vvl::AccelerationStructureNV(device, as, ci),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureNV))) {}

void AccelerationStructureNV::Destroy() {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureNV::Destroy();
}

void AccelerationStructureNV::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    vvl::AccelerationStructureNV::NotifyInvalidate(invalid_nodes, unlink);
}

CommandBuffer::CommandBuffer(Validator &gpuav, VkCommandBuffer handle, const VkCommandBufferAllocateInfo *pCreateInfo,
                             const vvl::CommandPool *pool)
    : vvl::CommandBuffer(gpuav, handle, pCreateInfo, pool),
      gpu_resources_manager(*gpuav.desc_set_manager_),
      state_(gpuav),
      error_output_buffer_(gpuav),
      cmd_errors_counts_buffer_(gpuav),
      bda_ranges_snapshot_(gpuav) {
    Location loc(vvl::Func::vkAllocateCommandBuffers);
    AllocateResources(loc);
}

static bool AllocateErrorLogsBuffer(Validator &gpuav, VkCommandBuffer command_buffer, vko::Buffer &error_output_buffer,
                                    const Location &loc) {
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = glsl::kErrorBufferByteSize;
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    alloc_info.pool = gpuav.output_buffer_pool_;
    const bool success = error_output_buffer.Create(loc, &buffer_info, &alloc_info);
    if (!success) {
        return false;
    }

    auto output_buffer_ptr = (uint32_t *)error_output_buffer.MapMemory(loc);

    memset(output_buffer_ptr, 0, glsl::kErrorBufferByteSize);
    if (gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        output_buffer_ptr[cst::stream_output_flags_offset] = cst::inst_buffer_oob_enabled;
    }

    error_output_buffer.UnmapMemory();
    return true;
}

void CommandBuffer::AllocateResources(const Location &loc) {
    auto gpuav = static_cast<Validator *>(&dev_data);

    VkResult result = VK_SUCCESS;

    // Instrumentation descriptor set layout
    if (instrumentation_desc_set_layout_ == VK_NULL_HANDLE) {
        assert(!gpuav->instrumentation_bindings_.empty());
        VkDescriptorSetLayoutCreateInfo instrumentation_desc_set_layout_ci = vku::InitStructHelper();
        instrumentation_desc_set_layout_ci.bindingCount = static_cast<uint32_t>(gpuav->instrumentation_bindings_.size());
        instrumentation_desc_set_layout_ci.pBindings = gpuav->instrumentation_bindings_.data();
        result = DispatchCreateDescriptorSetLayout(gpuav->device, &instrumentation_desc_set_layout_ci, nullptr,
                                                   &instrumentation_desc_set_layout_);
        if (result != VK_SUCCESS) {
            gpuav->InternalError(gpuav->device, loc, "Unable to create instrumentation descriptor set layout.");
            return;
        }
    }

    // Error output buffer
    if (!AllocateErrorLogsBuffer(*gpuav, VkHandle(), error_output_buffer_, loc)) {
        return;
    }

    // Commands errors counts buffer
    {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.size = GetCmdErrorsCountsBufferByteSize();
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        alloc_info.pool = gpuav->output_buffer_pool_;
        const bool success = cmd_errors_counts_buffer_.Create(loc, &buffer_info, &alloc_info);
        if (!success) {
            return;
        }

        ClearCmdErrorsCountsBuffer(loc);
        if (gpuav->aborted_) return;
    }

    // BDA snapshot
    if (gpuav->gpuav_settings.shader_instrumentation.buffer_device_address) {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        buffer_info.size = GetBdaRangesBufferByteSize();
        // This buffer could be very large if an application uses many buffers. Allocating it as HOST_CACHED
        // and manually flushing it at the end of the state updates is faster than using HOST_COHERENT.
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        bool success = bda_ranges_snapshot_.Create(loc, &buffer_info, &alloc_info);
        if (!success) {
            return;
        }
    }

    // Update validation commands common descriptor set
    {
        const std::vector<VkDescriptorSetLayoutBinding> validation_cmd_bindings = {
            // Error output buffer
            {glsl::kBindingDiagErrorBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
            // Buffer holding action command index in command buffer
            {glsl::kBindingDiagActionIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
            // Buffer holding a resource index from the per command buffer command resources list
            {glsl::kBindingDiagCmdResourceIndex, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
            // Commands errors counts buffer
            {glsl::kBindingDiagCmdErrorsCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        };

        if (error_logging_desc_set_layout_ == VK_NULL_HANDLE) {
            VkDescriptorSetLayoutCreateInfo validation_cmd_desc_set_layout_ci = vku::InitStructHelper();
            validation_cmd_desc_set_layout_ci.bindingCount = static_cast<uint32_t>(validation_cmd_bindings.size());
            validation_cmd_desc_set_layout_ci.pBindings = validation_cmd_bindings.data();
            result = DispatchCreateDescriptorSetLayout(gpuav->device, &validation_cmd_desc_set_layout_ci, nullptr,
                                                       &error_logging_desc_set_layout_);
            if (result != VK_SUCCESS) {
                gpuav->InternalError(gpuav->device, loc, "Unable to create descriptor set layout used for validation commands.");
                return;
            }
        }

        assert(validation_cmd_desc_pool_ == VK_NULL_HANDLE);
        assert(error_logging_desc_set_ == VK_NULL_HANDLE);
        result = gpuav->desc_set_manager_->GetDescriptorSet(&validation_cmd_desc_pool_, error_logging_desc_set_layout_,
                                                            &error_logging_desc_set_);
        if (result != VK_SUCCESS) {
            gpuav->InternalError(gpuav->device, loc, "Unable to create descriptor set used for validation commands.");
            return;
        }

        std::array<VkWriteDescriptorSet, 4> validation_cmd_descriptor_writes = {};
        assert(validation_cmd_bindings.size() == validation_cmd_descriptor_writes.size());

        VkDescriptorBufferInfo error_output_buffer_desc_info = {};

        assert(!error_output_buffer_.IsDestroyed());
        error_output_buffer_desc_info.buffer = error_output_buffer_.VkHandle();
        error_output_buffer_desc_info.offset = 0;
        error_output_buffer_desc_info.range = VK_WHOLE_SIZE;

        validation_cmd_descriptor_writes[0] = vku::InitStructHelper();
        validation_cmd_descriptor_writes[0].dstBinding = glsl::kBindingDiagErrorBuffer;
        validation_cmd_descriptor_writes[0].descriptorCount = 1;
        validation_cmd_descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        validation_cmd_descriptor_writes[0].pBufferInfo = &error_output_buffer_desc_info;
        validation_cmd_descriptor_writes[0].dstSet = GetErrorLoggingDescSet();

        VkDescriptorBufferInfo cmd_indices_buffer_desc_info = {};

        assert(!gpuav->indices_buffer_.IsDestroyed());
        cmd_indices_buffer_desc_info.buffer = gpuav->indices_buffer_.VkHandle();
        cmd_indices_buffer_desc_info.offset = 0;
        cmd_indices_buffer_desc_info.range = sizeof(uint32_t);

        validation_cmd_descriptor_writes[1] = vku::InitStructHelper();
        validation_cmd_descriptor_writes[1].dstBinding = glsl::kBindingDiagActionIndex;
        validation_cmd_descriptor_writes[1].descriptorCount = 1;
        validation_cmd_descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        validation_cmd_descriptor_writes[1].pBufferInfo = &cmd_indices_buffer_desc_info;
        validation_cmd_descriptor_writes[1].dstSet = GetErrorLoggingDescSet();

        validation_cmd_descriptor_writes[2] = validation_cmd_descriptor_writes[1];
        validation_cmd_descriptor_writes[2].dstBinding = glsl::kBindingDiagCmdResourceIndex;

        VkDescriptorBufferInfo cmd_errors_count_buffer_desc_info = {};
        cmd_errors_count_buffer_desc_info.buffer = GetCmdErrorsCountsBuffer();
        cmd_errors_count_buffer_desc_info.offset = 0;
        cmd_errors_count_buffer_desc_info.range = VK_WHOLE_SIZE;

        validation_cmd_descriptor_writes[3] = vku::InitStructHelper();
        validation_cmd_descriptor_writes[3].dstBinding = glsl::kBindingDiagCmdErrorsCount;
        validation_cmd_descriptor_writes[3].descriptorCount = 1;
        validation_cmd_descriptor_writes[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        validation_cmd_descriptor_writes[3].pBufferInfo = &cmd_errors_count_buffer_desc_info;
        validation_cmd_descriptor_writes[3].dstSet = GetErrorLoggingDescSet();

        DispatchUpdateDescriptorSets(gpuav->device, static_cast<uint32_t>(validation_cmd_descriptor_writes.size()),
                                     validation_cmd_descriptor_writes.data(), 0, NULL);
    }
}

bool CommandBuffer::UpdateBdaRangesBuffer(const Location &loc) {
    auto gpuav = static_cast<Validator *>(&dev_data);

    // By supplying a "date"
    if (!gpuav->gpuav_settings.shader_instrumentation.buffer_device_address ||
        bda_ranges_snapshot_version_ == gpuav->buffer_device_address_ranges_version) {
        return true;
    }

    // Update buffer device address table
    // ---
    auto bda_table_ptr = (VkDeviceAddress *)bda_ranges_snapshot_.MapMemory(loc);

    // Buffer device address table layout
    // Ranges are sorted from low to high, and do not overlap
    // QWord 0 | Number of *ranges* (1 range occupies 2 QWords)
    // QWord 1 | Range 1 begin
    // QWord 2 | Range 1 end
    // QWord 3 | Range 2 begin
    // QWord 4 | Range 2 end
    // QWord 5 | ...

    const size_t max_recordable_ranges =
        static_cast<size_t>((GetBdaRangesBufferByteSize() - sizeof(uint64_t)) / (2 * sizeof(VkDeviceAddress)));
    auto bda_ranges = reinterpret_cast<vvl::Device::BufferAddressRange *>(bda_table_ptr + 1);
    const auto [ranges_to_update_count, total_address_ranges_count] =
        gpuav->GetBufferAddressRanges(bda_ranges, max_recordable_ranges);
    bda_table_ptr[0] = ranges_to_update_count;

    if (total_address_ranges_count > size_t(gpuav->gpuav_settings.max_bda_in_use)) {
        std::ostringstream problem_string;
        problem_string << "Number of buffer device addresses ranges in use (" << total_address_ranges_count
                       << ") is greater than khronos_validation.gpuav_max_buffer_device_addresses ("
                       << gpuav->gpuav_settings.max_bda_in_use
                       << "). Truncating buffer device address table could result in invalid validation.";
        gpuav->InternalError(gpuav->device, loc, problem_string.str().c_str());
        return false;
    }

    // Post update cleanups
    // ---
    // Flush the BDA buffer before un-mapping so that the new state is visible to the GPU
    bda_ranges_snapshot_.FlushAllocation(loc);
    bda_ranges_snapshot_.UnmapMemory();
    bda_ranges_snapshot_version_ = gpuav->buffer_device_address_ranges_version;

    return true;
}

VkDeviceSize CommandBuffer::GetBdaRangesBufferByteSize() const {
    auto gpuav = static_cast<Validator *>(&dev_data);
    return (1                                           // 1 QWORD for the number of address ranges
            + 2 * gpuav->gpuav_settings.max_bda_in_use  // 2 QWORDS per address range
            ) *
           8;
}

CommandBuffer::~CommandBuffer() { Destroy(); }

void CommandBuffer::Destroy() {
    {
        auto guard = WriteLock();
        ResetCBState();
    }
    vvl::CommandBuffer::Destroy();
}

void CommandBuffer::Reset(const Location &loc) {
    vvl::CommandBuffer::Reset(loc);
    ResetCBState();
    // TODO: Calling AllocateResources in Reset like so is a kind of a hack,
    // relying on CommandBuffer internal logic to work.
    // Tried to call it in ResetCBState, hang on command buffer mutex :/
    AllocateResources(loc);
}

void CommandBuffer::ResetCBState() {
    auto gpuav = static_cast<Validator *>(&dev_data);

    // Free the device memory and descriptor set(s) associated with a command buffer.
    for (DebugPrintfBufferInfo &printf_buffer_info : debug_printf_buffer_infos) {
        printf_buffer_info.output_mem_buffer.Destroy();
    }
    debug_printf_buffer_infos.clear();

    // Free the device memory and descriptor set(s) associated with a command buffer.
    gpu_resources_manager.DestroyResources();
    per_command_error_loggers.clear();

    for (DescriptorCommandBinding &descriptor_command_binding : descriptor_command_bindings) {
        descriptor_command_binding.descritpor_state_ssbo_buffer.Destroy();
        descriptor_command_binding.post_process_ssbo_buffer.Destroy();
    }
    descriptor_command_bindings.clear();
    action_command_snapshots.clear();
    descriptor_indexing_buffer = VK_NULL_HANDLE;
    post_process_buffer_lut = VK_NULL_HANDLE;

    error_output_buffer_.Destroy();
    cmd_errors_counts_buffer_.Destroy();
    bda_ranges_snapshot_.Destroy();
    bda_ranges_snapshot_version_ = 0;

    if (validation_cmd_desc_pool_ != VK_NULL_HANDLE && error_logging_desc_set_ != VK_NULL_HANDLE) {
        gpuav->desc_set_manager_->PutBackDescriptorSet(validation_cmd_desc_pool_, error_logging_desc_set_);
        validation_cmd_desc_pool_ = VK_NULL_HANDLE;
        error_logging_desc_set_ = VK_NULL_HANDLE;
    }

    if (instrumentation_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav->device, instrumentation_desc_set_layout_, nullptr);
        instrumentation_desc_set_layout_ = VK_NULL_HANDLE;
    }

    if (error_logging_desc_set_layout_ != VK_NULL_HANDLE) {
        DispatchDestroyDescriptorSetLayout(gpuav->device, error_logging_desc_set_layout_, nullptr);
        error_logging_desc_set_layout_ = VK_NULL_HANDLE;
    }

    draw_index = 0;
    compute_index = 0;
    trace_rays_index = 0;
    action_command_count = 0;
}

void CommandBuffer::ClearCmdErrorsCountsBuffer(const Location &loc) const {
    auto cmd_errors_counts_buffer_ptr = (uint32_t *)cmd_errors_counts_buffer_.MapMemory(loc);
    std::memset(cmd_errors_counts_buffer_ptr, 0, static_cast<size_t>(GetCmdErrorsCountsBufferByteSize()));
    cmd_errors_counts_buffer_.UnmapMemory();
}

void CommandBuffer::IncrementCommandCount(VkPipelineBindPoint bind_point) {
    action_command_count++;
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        draw_index++;
    } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
        compute_index++;
    } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        trace_rays_index++;
    }
}

std::string CommandBuffer::GetDebugLabelRegion(uint32_t label_command_i,
                                               const std::vector<std::string> &initial_label_stack) const {
    std::string debug_region_name;
    if (label_command_i != vvl::kU32Max) {
        debug_region_name = GetDebugRegionName(GetLabelCommands(), label_command_i, initial_label_stack);
    } else {
        // label_command_i == vvl::kU32Max => when the instrumented command was recorded,
        // no debug label region was yet opened in the corresponding command buffer,
        // but still a region might have been started in another previously submitted
        // command buffer. So just compute region name from initial_label_stack.
        for (const std::string &label_name : initial_label_stack) {
            if (!debug_region_name.empty()) {
                debug_region_name += "::";
            }
            debug_region_name += label_name;
        }
    }
    return debug_region_name;
}

bool CommandBuffer::PreProcess(const Location &loc) {
    auto gpuav = static_cast<Validator *>(&dev_data);

    bool succeeded = descriptor::UpdateDescriptorStateSSBO(*gpuav, *this, loc);
    if (!succeeded) {
        return false;
    }

    succeeded = UpdateBdaRangesBuffer(loc);
    if (!succeeded) {
        return false;
    }

    return !per_command_error_loggers.empty() || has_build_as_cmd;
}

bool CommandBuffer::NeedsPostProcess() { return !error_output_buffer_.IsDestroyed(); }

// For the given command buffer, map its debug data buffers and read their contents for analysis.
void CommandBuffer::PostProcess(VkQueue queue, const std::vector<std::string> &initial_label_stack, const Location &loc) {
    auto gpuav = static_cast<Validator *>(&dev_data);

    // For the given command buffer, map its debug data buffers and read their contents for analysis.
    for (DebugPrintfBufferInfo &printf_buffer_info : debug_printf_buffer_infos) {
        auto printf_output_ptr = (char *)printf_buffer_info.output_mem_buffer.MapMemory(loc);
        debug_printf::AnalyzeAndGenerateMessage(*gpuav, VkHandle(), queue, printf_buffer_info, (uint32_t *)printf_output_ptr, loc);
        printf_buffer_info.output_mem_buffer.UnmapMemory();
    }

    // CommandBuffer::Destroy can happen on an other thread,
    // so when getting here after acquiring command buffer's lock,
    // make sure there are still things to process
    if (!NeedsPostProcess()) {
        return;
    }

    bool skip = false;
    {
        auto error_output_buffer_ptr = (uint32_t *)error_output_buffer_.MapMemory(loc);

        // The second word in the debug output buffer is the number of words that would have
        // been written by the shader instrumentation, if there was enough room in the buffer we provided.
        // The number of words actually written by the shaders is determined by the size of the buffer
        // we provide via the descriptor. So, we process only the number of words that can fit in the
        // buffer.
        const uint32_t total_words = error_output_buffer_ptr[cst::stream_output_size_offset];

        // A zero here means that the shader instrumentation didn't write anything.
        if (total_words != 0) {
            uint32_t *const error_records_start = &error_output_buffer_ptr[cst::stream_output_data_offset];
            assert(glsl::kErrorBufferByteSize > cst::stream_output_data_offset);
            uint32_t *const error_records_end =
                error_output_buffer_ptr + (glsl::kErrorBufferByteSize - cst::stream_output_data_offset);

            uint32_t *error_record_ptr = error_records_start;
            uint32_t record_size = error_record_ptr[glsl::kHeaderErrorRecordSizeOffset];
            assert(record_size == glsl::kErrorRecordSize);

            while (record_size > 0 && (error_record_ptr + record_size) <= error_records_end) {
                const uint32_t error_logger_i = error_record_ptr[glsl::kHeaderCommandResourceIdOffset];
                assert(error_logger_i < per_command_error_loggers.size());
                auto &error_logger = per_command_error_loggers[error_logger_i];
                const LogObjectList objlist(queue, VkHandle());
                skip |= error_logger(*gpuav, *this, error_record_ptr, objlist, initial_label_stack);

                // Next record
                error_record_ptr += record_size;
                record_size = error_record_ptr[glsl::kHeaderErrorRecordSizeOffset];
            }

            VVL_TracyPlot("GPU-AV errors count", int64_t(total_words / glsl::kErrorRecordSize));

            // Clear the written size and any error messages. Note that this preserves the first word, which contains flags.
            assert(glsl::kErrorBufferByteSize > cst::stream_output_data_offset);
            memset(&error_output_buffer_ptr[cst::stream_output_data_offset], 0,
                   glsl::kErrorBufferByteSize - cst::stream_output_data_offset * sizeof(uint32_t));
        }
        error_output_buffer_ptr[cst::stream_output_size_offset] = 0;
        error_output_buffer_.UnmapMemory();
    }

    ClearCmdErrorsCountsBuffer(loc);
    if (gpuav->aborted_) return;

    // If instrumentation found an error, skip post processing. Errors detected by instrumentation are usually
    // very serious, such as a prematurely destroyed resource and the state needed below is likely invalid.
    bool gpuav_success = false;
    if (!skip && gpuav->gpuav_settings.shader_instrumentation.post_process_descriptor_index) {
        gpuav_success = ValidateBindlessDescriptorSets(loc);
    }

    if (gpuav_success) {
        UpdateCmdBufImageLayouts(state_, *this);
    }
}

Queue::Queue(Validator &gpuav, VkQueue q, uint32_t family_index, uint32_t queue_index, VkDeviceQueueCreateFlags flags,
             const VkQueueFamilyProperties &queueFamilyProperties, bool timeline_khr)
    : vvl::Queue(gpuav, q, family_index, queue_index, flags, queueFamilyProperties), state_(gpuav), timeline_khr_(timeline_khr) {}

Queue::~Queue() {
    if (barrier_command_buffer_) {
        DispatchFreeCommandBuffers(state_.device, barrier_command_pool_, 1, &barrier_command_buffer_);
        barrier_command_buffer_ = VK_NULL_HANDLE;
    }
    if (barrier_command_pool_) {
        DispatchDestroyCommandPool(state_.device, barrier_command_pool_, nullptr);
        barrier_command_pool_ = VK_NULL_HANDLE;
    }
    if (barrier_sem_) {
        DispatchDestroySemaphore(state_.device, barrier_sem_, nullptr);
        barrier_sem_ = VK_NULL_HANDLE;
    }
}

// Submit a memory barrier on graphics queues.
// Lazy-create and record the needed command buffer.
void Queue::SubmitBarrier(const Location &loc, uint64_t seq) {
    if (barrier_command_pool_ == VK_NULL_HANDLE) {
        VkResult result = VK_SUCCESS;

        VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
        pool_create_info.queueFamilyIndex = queue_family_index;
        result = DispatchCreateCommandPool(state_.device, &pool_create_info, nullptr, &barrier_command_pool_);
        if (result != VK_SUCCESS) {
            state_.InternalError(vvl::Queue::VkHandle(), loc, "Unable to create command pool for barrier CB.");
            barrier_command_pool_ = VK_NULL_HANDLE;
            return;
        }

        VkCommandBufferAllocateInfo buffer_alloc_info = vku::InitStructHelper();
        buffer_alloc_info.commandPool = barrier_command_pool_;
        buffer_alloc_info.commandBufferCount = 1;
        buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        result = DispatchAllocateCommandBuffers(state_.device, &buffer_alloc_info, &barrier_command_buffer_);
        if (result != VK_SUCCESS) {
            state_.InternalError(vvl::Queue::VkHandle(), loc, "Unable to create barrier command buffer.");
            DispatchDestroyCommandPool(state_.device, barrier_command_pool_, nullptr);
            barrier_command_pool_ = VK_NULL_HANDLE;
            barrier_command_buffer_ = VK_NULL_HANDLE;
            return;
        }

        VkSemaphoreTypeCreateInfo semaphore_type_create_info = vku::InitStructHelper();
        semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphore_type_create_info.initialValue = 0;

        VkSemaphoreCreateInfo semaphore_create_info = vku::InitStructHelper(&semaphore_type_create_info);

        result = DispatchCreateSemaphore(state_.device, &semaphore_create_info, nullptr, &barrier_sem_);
        if (result != VK_SUCCESS) {
            state_.InternalError(state_.device, loc, "Unable to create barrier semaphore.");
            DispatchDestroyCommandPool(state_.device, barrier_command_pool_, nullptr);
            barrier_command_pool_ = VK_NULL_HANDLE;
            barrier_command_buffer_ = VK_NULL_HANDLE;
            return;
        }

        // Hook up command buffer dispatch
        state_.vk_set_device_loader_data_(state_.device, barrier_command_buffer_);

        // Record a global memory barrier to force availability of device memory operations to the host domain.
        VkCommandBufferBeginInfo barrier_cmd_buffer_begin_info = vku::InitStructHelper();
        barrier_cmd_buffer_begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        result = DispatchBeginCommandBuffer(barrier_command_buffer_, &barrier_cmd_buffer_begin_info);
        if (result == VK_SUCCESS) {
            VkMemoryBarrier memory_barrier = vku::InitStructHelper();
            memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            memory_barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
            DispatchCmdPipelineBarrier(barrier_command_buffer_, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                                       1, &memory_barrier, 0, nullptr, 0, nullptr);
            DispatchEndCommandBuffer(barrier_command_buffer_);
        }
    }

    if (barrier_command_buffer_ != VK_NULL_HANDLE) {
        VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = vku::InitStructHelper();
        timeline_semaphore_submit_info.signalSemaphoreValueCount = 1;
        timeline_semaphore_submit_info.pSignalSemaphoreValues = &seq;

        VkSubmitInfo submit_info = vku::InitStructHelper(&timeline_semaphore_submit_info);

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &barrier_command_buffer_;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &barrier_sem_;

        DispatchQueueSubmit(vvl::Queue::VkHandle(), 1, &submit_info, VK_NULL_HANDLE);
    }
}

vvl::PreSubmitResult Queue::PreSubmit(std::vector<vvl::QueueSubmission> &&submissions) {
    for (const auto &submission : submissions) {
        auto loc = submission.loc.Get();
        for (auto &cb_submission : submission.cb_submissions) {
            auto gpu_cb = std::static_pointer_cast<CommandBuffer>(cb_submission.cb);
            auto guard = gpu_cb->ReadLock();
            gpu_cb->PreProcess(loc);
            for (auto *secondary_cb : gpu_cb->linkedCommandBuffers) {
                auto secondary_guard = secondary_cb->ReadLock();
                auto *secondary_gpu_cb = static_cast<CommandBuffer *>(secondary_cb);
                secondary_gpu_cb->PreProcess(loc);
            }
        }
    }
    return vvl::Queue::PreSubmit(std::move(submissions));
}

void Queue::PostSubmit(vvl::QueueSubmission &submission) {
    vvl::Queue::PostSubmit(submission);
    if (submission.end_batch) {
        auto loc = submission.loc.Get();
        SubmitBarrier(loc, submission.seq);
    }
}

void Queue::Retire(vvl::QueueSubmission &submission) {
    vvl::Queue::Retire(submission);
    if (submission.loc.Get().function == vvl::Func::vkQueuePresentKHR) {
        // Present batch does not have any GPU-AV work to post process, skip it.
        // This is also needed for correctness. QueuePresent does not have a PostSubmit call
        // that signals barrier_sem_. The following timeline wait must not be called.
        return;
    }
    retiring_.emplace_back(submission.cb_submissions);
    if (submission.end_batch) {
        VkSemaphoreWaitInfo wait_info = vku::InitStructHelper();
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &barrier_sem_;
        wait_info.pValues = &submission.seq;

        if (timeline_khr_) {
            DispatchWaitSemaphoresKHR(state_.device, &wait_info, 1'000'000'000);
        } else {
            DispatchWaitSemaphores(state_.device, &wait_info, 1'000'000'000);
        }

        for (std::vector<vvl::CommandBufferSubmission> &cb_submissions : retiring_) {
            for (vvl::CommandBufferSubmission &cb_submission : cb_submissions) {
                auto gpu_cb = std::static_pointer_cast<CommandBuffer>(cb_submission.cb);
                auto guard = gpu_cb->WriteLock();
                auto loc = submission.loc.Get();
                gpu_cb->PostProcess(VkHandle(), cb_submission.initial_label_stack, loc);
                for (vvl::CommandBuffer *secondary_cb : gpu_cb->linkedCommandBuffers) {
                    auto secondary_gpu_cb = static_cast<CommandBuffer *>(secondary_cb);
                    auto secondary_guard = secondary_gpu_cb->WriteLock();
                    secondary_gpu_cb->PostProcess(VkHandle(), cb_submission.initial_label_stack, loc);
                }
            }
        }
        retiring_.clear();
    }
}

}  // namespace gpuav
