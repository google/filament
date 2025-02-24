/*
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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
#pragma once

#include "sync/sync_renderpass.h"
#include "sync/sync_reporting.h"
#include "state_tracker/cmd_buffer_state.h"

struct ReportKeyValues;
class SyncValidator;

namespace syncval {
class ErrorMessages;
}  // namespace syncval

class AlternateResourceUsage {
  public:
    struct RecordBase;
    struct RecordBase {
        using Record = std::unique_ptr<RecordBase>;
        virtual Record MakeRecord() const = 0;
        virtual std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const = 0;
        virtual vvl::Func GetCommand() const = 0;
        virtual ~RecordBase() {}
    };

    struct FormatterState {
        FormatterState(const SyncValidator &sync_state_, const AlternateResourceUsage &usage_)
            : sync_state(sync_state_), usage(usage_) {}
        const SyncValidator &sync_state;
        const AlternateResourceUsage &usage;
    };

    FormatterState Formatter(const SyncValidator &sync_state) const { return FormatterState(sync_state, *this); };

    std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const { return record_->Format(out, sync_state); };
    vvl::Func GetCommand() const { return record_->GetCommand(); }
    AlternateResourceUsage() = default;
    AlternateResourceUsage(const RecordBase &record) : record_(record.MakeRecord()) {}
    AlternateResourceUsage(const AlternateResourceUsage &other) : record_() {
        if (bool(other.record_)) {
            record_ = other.record_->MakeRecord();
        }
    }
    AlternateResourceUsage &operator=(const AlternateResourceUsage &other) {
        if (bool(other.record_)) {
            record_ = other.record_->MakeRecord();
        } else {
            record_.reset();
        }
        return *this;
    }

    operator bool() const { return bool(record_); }

  private:
    RecordBase::Record record_;
};

inline std::ostream &operator<<(std::ostream &out, const AlternateResourceUsage::FormatterState &formatter) {
    formatter.usage.Format(out, formatter.sync_state);
    return out;
}

template <typename State, typename T>
struct FormatterImpl {
    using That = T;
    friend T;
    const State &state;
    const That &that;

  private:
    // Only intended to be invoke with from That method
    FormatterImpl(const State &state_, const That &that_) : state(state_), that(that_) {}
};

// Vulkan handle and associated information.
// Command buffer context stores array of handles that are referenced by the tagged commands.
// VulkanTypedHandle is stored in unpacked form to avoid structure padding gaps.
struct HandleRecord {
    uint64_t handle = 0;
    VulkanObjectType type = kVulkanObjectTypeUnknown;
    uint32_t index = vvl::kNoIndex32;

    HandleRecord() = default;
    explicit HandleRecord(const VulkanTypedHandle &typed_handle, uint32_t index = vvl::kNoIndex32)
        : handle(typed_handle.handle), type(typed_handle.type), index(index) {}
    bool IsIndexed() const { return index != vvl::kNoIndex32; }

    VulkanTypedHandle TypedHandle() const {
        VulkanTypedHandle typed_handle;
        typed_handle.handle = handle;
        typed_handle.type = type;
        return typed_handle;
    }
    using FormatterState = FormatterImpl<SyncValidator, HandleRecord>;
    FormatterState Formatter(const SyncValidator &sync_state) const { return FormatterState(sync_state, *this); }
};

struct ResourceCmdUsageRecord {
    static constexpr auto kMaxIndex = std::numeric_limits<ResourceUsageTag>::max();
    enum class SubcommandType { kNone, kSubpassTransition, kLoadOp, kStoreOp, kResolveOp, kIndex };

    ResourceCmdUsageRecord() = default;
    ResourceCmdUsageRecord(vvl::Func command_, uint32_t seq_num_, SubcommandType sub_type_, uint32_t sub_command_,
                           const vvl::CommandBuffer *cb_state_, uint32_t reset_count_)
        : command(command_),
          seq_num(seq_num_),
          sub_command_type(sub_type_),
          sub_command(sub_command_),
          cb_state(cb_state_),
          reset_count(reset_count_) {}

    vvl::Func command = vvl::Func::Empty;
    uint32_t seq_num = 0U;
    SubcommandType sub_command_type = SubcommandType::kNone;
    uint32_t sub_command = 0U;

    // This is somewhat repetitive, but it prevents the need for Exec/Submit time touchup, after which usage records can be
    // from different command buffers and resets.
    // plain pointer as a shared pointer is held by the context storing this record
    const vvl::CommandBuffer *cb_state = nullptr;
    uint32_t reset_count = 0;

    uint32_t first_handle_index = vvl::kNoIndex32;
    uint32_t handle_count = 0;

    uint32_t label_command_index = vvl::kNoIndex32;
};

struct DebugNameProvider;

struct ResourceUsageRecord : public ResourceCmdUsageRecord {
    struct FormatterState {
        FormatterState(const SyncValidator &sync_state_, const ResourceUsageRecord &record_, const vvl::CommandBuffer *cb_state_,
                       const DebugNameProvider *debug_name_provider_, uint32_t handle_index)
            : sync_state(sync_state_),
              record(record_),
              ex_cb_state(cb_state_),
              debug_name_provider(debug_name_provider_),
              handle_index(handle_index) {}
        const SyncValidator &sync_state;
        const ResourceUsageRecord &record;
        const vvl::CommandBuffer *ex_cb_state;
        const DebugNameProvider *debug_name_provider;
        uint32_t handle_index;
    };
    FormatterState Formatter(const SyncValidator &sync_state, const vvl::CommandBuffer *ex_cb_state,
                             const DebugNameProvider *debug_name_provider, uint32_t handle_index) const {
        return FormatterState(sync_state, *this, ex_cb_state, debug_name_provider, handle_index);
    }

    AlternateResourceUsage alt_usage;

    ResourceUsageRecord() = default;
    ResourceUsageRecord(vvl::Func command_, uint32_t seq_num_, SubcommandType sub_type_, uint32_t sub_command_,
                        const vvl::CommandBuffer *cb_state_, uint32_t reset_count_)
        : ResourceCmdUsageRecord(command_, seq_num_, sub_type_, sub_command_, cb_state_, reset_count_) {}

    ResourceUsageRecord(const AlternateResourceUsage &other) : ResourceCmdUsageRecord(), alt_usage(other) {}
    ResourceUsageRecord(const ResourceUsageRecord &other) : ResourceCmdUsageRecord(other), alt_usage(other.alt_usage) {}
    ResourceUsageRecord &operator=(const ResourceUsageRecord &other) = default;
};

// Provides debug region name for the specified access log command.
// If empty name is returned it means the command is not inside debug region.
struct DebugNameProvider {
    virtual std::string GetDebugRegionName(const ResourceUsageRecord &record) const = 0;
};

// Command execution context is the base class for command buffer and queue contexts
class CommandExecutionContext {
  public:
    using AccessLog = std::vector<ResourceUsageRecord>;
    using CommandBufferSet = std::vector<std::shared_ptr<const vvl::CommandBuffer>>;
    CommandExecutionContext(const SyncValidator &sync_validator, VkQueueFlags queue_flags);
    virtual ~CommandExecutionContext() = default;

    // Are imported command buffers Submitted (QueueBatchContext), or Executed (CommandBufferAccessContext)
    enum ExecutionType : int {
        kExecuted = 0,  // Recorded contexts are integrated into context during vkCmdExecuteCommands
        kSubmitted = 1  // Recorded contexts are integrated into context during vkQueueSubmit (etc.)
    };

    virtual ExecutionType Type() const = 0;

    const char *ExecutionTypeString() const {
        const char *type_string[] = {"Executed", "Submitted"};
        return type_string[Type()];
    }
    const char *ExecutionUsageString() const {
        const char *usage_string[] = {"executed_usage", "submitted_usage"};
        return usage_string[Type()];
    }

    virtual AccessContext *GetCurrentAccessContext() = 0;
    virtual SyncEventsContext *GetCurrentEventsContext() = 0;
    virtual const AccessContext *GetCurrentAccessContext() const = 0;
    virtual const SyncEventsContext *GetCurrentEventsContext() const = 0;
    virtual QueueId GetQueueId() const = 0;
    virtual VulkanTypedHandle Handle() const = 0;
    virtual std::string FormatUsage(ResourceUsageTagEx tag_ex, ReportKeyValues &extra_properties) const = 0;
    virtual void AddUsageRecordExtraProperties(ResourceUsageTag tag, ReportKeyValues &extra_properties) const = 0;

    std::string FormatHazard(const HazardResult &hazard, ReportKeyValues &key_values) const;
    bool ValidForSyncOps() const;
    const SyncValidator &GetSyncState() const { return sync_state_; }

  protected:
    const SyncValidator &sync_state_;
    const syncval::ErrorMessages &error_messages_;
    const VkQueueFlags queue_flags_;
};

class CommandBufferAccessContext : public CommandExecutionContext, DebugNameProvider {
  public:
    using SyncOpPointer = std::shared_ptr<SyncOpBase>;
    constexpr static SyncAccessIndex kResolveRead = SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ;
    constexpr static SyncAccessIndex kResolveWrite = SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
    constexpr static SyncOrdering kColorResolveOrder = SyncOrdering::kColorAttachment;
    // Although depth resolve runs on the color attachment output stage and uses color accesses, depth accesses
    // still participate in the ordering. That's why using raster and not only color attachment ordering
    constexpr static SyncOrdering kDepthStencilResolveOrder = SyncOrdering::kRaster;

    constexpr static SyncOrdering kStoreOrder = SyncOrdering::kRaster;

    struct SyncOpEntry {
        ResourceUsageTag tag;
        SyncOpPointer sync_op;
        SyncOpEntry(ResourceUsageTag tag_, SyncOpPointer &&sync_op_) : tag(tag_), sync_op(std::move(sync_op_)) {}
        SyncOpEntry() = default;
        SyncOpEntry(const SyncOpEntry &other) = default;
    };

    CommandBufferAccessContext(SyncValidator &sync_validator, vvl::CommandBuffer *cb_state);

    struct AsProxyContext {};
    CommandBufferAccessContext(const CommandBufferAccessContext &real_context, AsProxyContext dummy);

    ~CommandBufferAccessContext() override;

    // NOTE: because this class is encapsulated in syncval_state::CommandBuffer, it isn't safe
    // to use shared_from_this from the constructor.
    void SetSelfReference() { cbs_referenced_->push_back(cb_state_->shared_from_this()); }

    void Destroy() {
        // the cb self reference must be cleared or the command buffer reference count will never go to 0
        cbs_referenced_.reset();
        cb_state_ = nullptr;
    }

    void Reset();

    ReportUsageInfo GetReportUsageInfo(ResourceUsageTagEx tag_ex) const;
    std::string FormatUsage(ResourceUsageTagEx tag_ex, ReportKeyValues &extra_properties) const override;
    void AddUsageRecordExtraProperties(ResourceUsageTag tag, ReportKeyValues &extra_properties) const override;
    std::string FormatUsage(const char *usage_string, const ResourceFirstAccess &access,
                            ReportKeyValues &key_values) const;  //  Only command buffers have "first usage"
    AccessContext *GetCurrentAccessContext() override { return current_context_; }
    SyncEventsContext *GetCurrentEventsContext() override { return &events_context_; }
    const AccessContext *GetCurrentAccessContext() const override { return current_context_; }
    const SyncEventsContext *GetCurrentEventsContext() const override { return &events_context_; }
    QueueId GetQueueId() const override;

    RenderPassAccessContext *GetCurrentRenderPassContext() { return current_renderpass_context_; }
    const RenderPassAccessContext *GetCurrentRenderPassContext() const { return current_renderpass_context_; }
    ResourceUsageTag RecordBeginRenderPass(vvl::Func command, const vvl::RenderPass &rp_state, const VkRect2D &render_area,
                                           const std::vector<const syncval_state::ImageViewState *> &attachment_views);

    bool ValidateBeginRendering(const ErrorObject &error_obj, syncval_state::BeginRenderingCmdState &cmd_state) const;
    void RecordBeginRendering(syncval_state::BeginRenderingCmdState &cmd_state, const RecordObject &record_obj);
    bool ValidateEndRendering(const ErrorObject &error_obj) const;
    void RecordEndRendering(const RecordObject &record_obj);
    bool ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint, const Location &loc) const;
    void RecordDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint, ResourceUsageTag tag);
    bool ValidateDrawVertex(std::optional<uint32_t> vertexCount, uint32_t firstVertex, const Location &loc) const;
    void RecordDrawVertex(std::optional<uint32_t> vertexCount, uint32_t firstVertex, ResourceUsageTag tag);
    bool ValidateDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, const Location &loc) const;
    void RecordDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, ResourceUsageTag tag);
    bool ValidateDrawAttachment(const Location &loc) const;
    bool ValidateDrawDynamicRenderingAttachment(const Location &loc) const;
    void RecordDrawAttachment(ResourceUsageTag tag);
    void RecordDrawDynamicRenderingAttachment(ResourceUsageTag tag);
    ClearAttachmentInfo GetClearAttachmentInfo(const VkClearAttachment &clear_attachment, const VkClearRect &rect) const;
    bool ValidateClearAttachment(const Location &loc, const VkClearAttachment &clear_attachment, const VkClearRect &rect) const;
    void RecordClearAttachment(ResourceUsageTag tag, const VkClearAttachment &clear_attachment, const VkClearRect &rect);

    ResourceUsageTag RecordNextSubpass(vvl::Func command);
    ResourceUsageTag RecordEndRenderPass(vvl::Func command);
    void RecordDestroyEvent(vvl::Event *event_state);

    void RecordExecutedCommandBuffer(const CommandBufferAccessContext &recorded_context);
    void ResolveExecutedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset);

    // TODO: what about using queue_flags directly from base class?
    VkQueueFlags GetQueueFlags() const { return cb_state_ ? cb_state_->GetQueueFlags() : 0; }

    ExecutionType Type() const override { return kExecuted; }
    size_t GetTagCount() const { return access_log_->size(); }
    VulkanTypedHandle Handle() const override {
        if (cb_state_) {
            return cb_state_->Handle();
        }
        return VulkanTypedHandle(static_cast<VkCommandBuffer>(VK_NULL_HANDLE), kVulkanObjectTypeCommandBuffer);
    }

    ResourceUsageTag NextCommandTag(vvl::Func command,
                                    ResourceUsageRecord::SubcommandType subcommand = ResourceUsageRecord::SubcommandType::kNone);
    ResourceUsageTag NextSubcommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand);

    ResourceUsageTagEx AddCommandHandle(ResourceUsageTag tag, const VulkanTypedHandle &typed_handle,
                                        uint32_t index = vvl::kNoIndex32);

    // Default subcommand behavior is that it references the same handles as the main command.
    // The following method allows to set subcommand handles independently of the main command.
    void AddSubcommandHandle(ResourceUsageTag tag, const VulkanTypedHandle &typed_handle, uint32_t index = vvl::kNoIndex32);

    const std::vector<HandleRecord> &GetHandleRecords() const { return handles_; }

    std::shared_ptr<const vvl::CommandBuffer> GetCBStateShared() const { return cb_state_->shared_from_this(); }

    const vvl::CommandBuffer &GetCBState() const {
        assert(cb_state_);
        return *cb_state_;
    }

    template <class T, class... Args>
    void RecordSyncOp(Args &&...args) {
        // T must be as derived from SyncOpBase or the compiler will flag the next line as an error.
        SyncOpPointer sync_op(std::make_shared<T>(std::forward<Args>(args)...));
        RecordSyncOp(std::move(sync_op));  // Call the non-template version
    }
    std::shared_ptr<AccessLog> GetAccessLogShared() const { return access_log_; }
    std::shared_ptr<CommandBufferSet> GetCBReferencesShared() const { return cbs_referenced_; }
    void ImportRecordedAccessLog(const CommandBufferAccessContext &cb_context);
    const std::vector<SyncOpEntry> &GetSyncOps() const { return sync_ops_; };

    // DebugNameProvider
    std::string GetDebugRegionName(const ResourceUsageRecord &record) const override;

    std::vector<vvl::LabelCommand> &GetProxyLabelCommands() { return proxy_label_commands_; }

  private:
    CommandBufferAccessContext(const SyncValidator &sync_validator, VkQueueFlags queue_flags);

    uint32_t AddHandle(const VulkanTypedHandle &typed_handle, uint32_t index);

    // As this is passing around a shared pointer to record, move to avoid needless atomics.
    void RecordSyncOp(SyncOpPointer &&sync_op);

    bool ValidateClearAttachment(const Location &loc, const ClearAttachmentInfo &info) const;
    void RecordClearAttachment(ResourceUsageTag tag, const ClearAttachmentInfo &clear_info);

    void CheckCommandTagDebugCheckpoint();

  private:
    // Note: since every CommandBufferAccessContext is encapsulated in its CommandBuffer object,
    // a reference count is not needed here.
    vvl::CommandBuffer *cb_state_;

    std::shared_ptr<AccessLog> access_log_;
    std::shared_ptr<CommandBufferSet> cbs_referenced_;
    uint32_t command_number_;
    uint32_t subcommand_number_;
    uint32_t reset_count_;

    // Handles referenced by the tagged commands
    std::vector<HandleRecord> handles_;

    // Location of the current command in the access log (it's not always the last element, there might be
    // subcommands that follow). The subcommands by default reference the same handles as the main command.
    ResourceUsageTag current_command_tag_ = vvl::kNoIndex32;

    AccessContext cb_access_context_;
    AccessContext *current_context_;
    SyncEventsContext events_context_;

    // Don't need the following for an active proxy cb context
    std::vector<std::unique_ptr<RenderPassAccessContext>> render_pass_contexts_;
    RenderPassAccessContext *current_renderpass_context_;
    std::vector<SyncOpEntry> sync_ops_;

    // State during dynamic rendering (dynamic rendering rendering passes must be
    // contained within a single command buffer)
    std::unique_ptr<syncval_state::DynamicRenderingInfo> dynamic_rendering_info_;

    // Secondary buffer validation uses proxy context and does local update (imitates Record).
    // Because in this case PreRecord is not called, the label state is not updated. We make
    // a copy of label state to update it locally together with proxy context.
    std::vector<vvl::LabelCommand> proxy_label_commands_;
};

namespace syncval_state {
class CommandBuffer : public vvl::CommandBuffer {
  public:
    CommandBufferAccessContext access_context;

    CommandBuffer(SyncValidator &dev, VkCommandBuffer handle, const VkCommandBufferAllocateInfo *allocate_info,
                  const vvl::CommandPool *pool);
    ~CommandBuffer() { Destroy(); }

    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    void Destroy() override;
    void Reset(const Location &loc) override;
};
}  // namespace syncval_state
