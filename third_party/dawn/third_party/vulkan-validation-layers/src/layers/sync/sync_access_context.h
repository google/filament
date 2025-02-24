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

#include "sync/sync_common.h"
#include "sync/sync_access_state.h"

struct SubpassDependencyGraphNode;

namespace vvl {
class Buffer;
class VideoSession;
class VideoPictureResource;
class Bindable;
class Event;
}  // namespace vvl

namespace syncval_state {
class ImageState;
class ImageViewState;
}  // namespace syncval_state
bool SimpleBinding(const vvl::Bindable &bindable);
VkDeviceSize ResourceBaseAddress(const vvl::Buffer &buffer);

// ForEachEntryInRangesUntil -- Execute Action for each map entry in the generated ranges until it returns true
//
// Action is const w.r.t. map
// Action is allowed (assumed) to modify pos
// Action must not advance pos for ranges strictly < pos->first
// Action must handle range strictly less than pos->first correctly
// Action must handle pos == end correctly
// Action is assumed to only require invocation once per map entry
// RangeGen must be strictly monotonic
// Note: If Action invocations are heavyweight and inter-entry (gap) calls are not needed
//       add a template or function parameter to skip them. TBD.
template <typename RangeMap, typename RangeGen, typename Action>
bool ForEachEntryInRangesUntil(const RangeMap &map, RangeGen &range_gen, Action &action) {
    using RangeType = typename RangeGen::RangeType;
    using IndexType = typename RangeType::index_type;
    auto pos = map.lower_bound(*range_gen);
    const auto end = map.end();
    IndexType skip_limit = 0;
    for (; range_gen->non_empty() && pos != end; ++range_gen) {
        RangeType range = *range_gen;
        // See if a prev pos has covered this range
        if (range.end <= skip_limit) {
            // Since the map is const, we needn't call action on the same pos again
            continue;
        }

        //  If the current range was *partially* covered be a previous pos, trim, such that Action is only
        //  called once for a given range (and pos)
        if (range.begin < skip_limit) {
            range.begin = skip_limit;
        }

        // Now advance pos as needed to match range
        if (pos->first.strictly_less(range)) {
            ++pos;
            if (pos == end) break;
            if (pos->first.strictly_less(range)) {
                pos = map.lower_bound(range);
                if (pos == end) break;
            }
            assert(pos == map.lower_bound(range));
        }

        // If the range intersects pos->first, consider Action performed for that map entry, and
        // make sure not to call Action for this pos for any subsequent ranges
        skip_limit = range.end > pos->first.begin ? pos->first.end : 0U;

        // Action is allowed to alter pos but shouldn't do so if range is strictly < pos->first
        if (action(range, end, pos)) return true;
    }

    // Action needs to handle the "at end " condition (and can be useful for recursive actions)
    for (; range_gen->non_empty(); ++range_gen) {
        if (action(*range_gen, end, pos)) return true;
    }

    return false;
}

struct NoopBarrierAction {
    explicit NoopBarrierAction() {}
    void operator()(ResourceAccessState *access) const {}
    const bool layout_transition = false;
};

// This functor applies a collection of barriers, updating the "pending state" in each touched memory range, and optionally
// resolves the pending state. Suitable for processing Global memory barriers, or Subpass Barriers when the "final" barrier
// of a collection is known/present.
template <typename BarrierOp, typename OpVector = std::vector<BarrierOp>>
class ApplyBarrierOpsFunctor {
  public:
    using Iterator = ResourceAccessRangeMap::iterator;
    // Only called with a gap, and pos at the lower_bound(range)
    inline Iterator Infill(ResourceAccessRangeMap *accesses, const Iterator &pos, const ResourceAccessRange &range) const {
        if (!infill_default_) {
            return pos;
        }
        ResourceAccessState default_state;
        auto inserted = accesses->insert(pos, std::make_pair(range, default_state));
        return inserted;
    }

    void operator()(const Iterator &pos) const {
        auto &access_state = pos->second;
        for (const auto &op : barrier_ops_) {
            op(&access_state);
        }

        if (resolve_) {
            // If this is the last (or only) batch, we can do the pending resolve as the last step in this operation to avoid
            // another walk
            access_state.ApplyPendingBarriers(tag_);
        }
    }

    // A valid tag is required IFF layout_transition is true, as transitions are write ops
    ApplyBarrierOpsFunctor(bool resolve, typename OpVector::size_type size_hint, ResourceUsageTag tag)
        : resolve_(resolve), infill_default_(false), barrier_ops_(), tag_(tag) {
        barrier_ops_.reserve(size_hint);
    }
    void EmplaceBack(const BarrierOp &op) {
        barrier_ops_.emplace_back(op);
        infill_default_ |= op.layout_transition;
    }

  private:
    bool resolve_;
    bool infill_default_;
    OpVector barrier_ops_;
    const ResourceUsageTag tag_;
};

// This functor applies a single barrier, updating the "pending state" in each touched memory range, but does not
// resolve the pendinging state. Suitable for processing Image and Buffer barriers from PipelineBarriers or Events
template <typename BarrierOp>
class ApplyBarrierFunctor : public ApplyBarrierOpsFunctor<BarrierOp, small_vector<BarrierOp, 1>> {
    using Base = ApplyBarrierOpsFunctor<BarrierOp, small_vector<BarrierOp, 1>>;

  public:
    ApplyBarrierFunctor(const BarrierOp &barrier_op) : Base(false, 1, kInvalidTag) { Base::EmplaceBack(barrier_op); }
};

// This functor resolves the pendinging state.
class ResolvePendingBarrierFunctor : public ApplyBarrierOpsFunctor<NoopBarrierAction, small_vector<NoopBarrierAction, 1>> {
    using Base = ApplyBarrierOpsFunctor<NoopBarrierAction, small_vector<NoopBarrierAction, 1>>;

  public:
    ResolvePendingBarrierFunctor(ResourceUsageTag tag) : Base(true, 0, tag) {}
};

struct ApplySubpassTransitionBarriersAction {
    explicit ApplySubpassTransitionBarriersAction(const std::vector<SyncBarrier> &barriers_) : barriers(barriers_) {}
    void operator()(ResourceAccessState *access) const {
        assert(access);
        access->ApplyBarriers(barriers, true);
    }
    const std::vector<SyncBarrier> &barriers;
};

struct QueueTagOffsetBarrierAction {
    QueueTagOffsetBarrierAction(QueueId qid, ResourceUsageTag offset) : queue_id(qid), tag_offset(offset) {}
    void operator()(ResourceAccessState *access) const {
        access->OffsetTag(tag_offset);
        access->SetQueueId(queue_id);
    };
    QueueId queue_id;
    ResourceUsageTag tag_offset;
};

struct ApplyTrackbackStackAction {
    explicit ApplyTrackbackStackAction(const std::vector<SyncBarrier> &barriers_,
                                       const ResourceAccessStateFunction *previous_barrier_ = nullptr)
        : barriers(barriers_), previous_barrier(previous_barrier_) {}
    void operator()(ResourceAccessState *access) const {
        assert(access);
        assert(!access->HasPendingState());
        access->ApplyBarriers(barriers, false);
        // NOTE: We can use invalid tag, as these barriers do no include layout transitions (would assert in SetWrite)
        access->ApplyPendingBarriers(kInvalidTag);
        if (previous_barrier) {
            assert(bool(*previous_barrier));
            (*previous_barrier)(access);
        }
    }
    const std::vector<SyncBarrier> &barriers;
    const ResourceAccessStateFunction *previous_barrier;
};

template <typename SubpassNode>
struct SubpassBarrierTrackback {
    std::vector<SyncBarrier> barriers;
    const SubpassNode *source_subpass = nullptr;
    SubpassBarrierTrackback() = default;
    SubpassBarrierTrackback(const SubpassBarrierTrackback &) = default;
    SubpassBarrierTrackback(const SubpassNode *source_subpass_, VkQueueFlags queue_flags_,
                            const std::vector<const VkSubpassDependency2 *> &subpass_dependencies_)
        : barriers(), source_subpass(source_subpass_) {
        barriers.reserve(subpass_dependencies_.size());
        for (const VkSubpassDependency2 *dependency : subpass_dependencies_) {
            assert(dependency);
            barriers.emplace_back(queue_flags_, *dependency);
        }
    }
    SubpassBarrierTrackback(const SubpassNode *source_subpass_, const SyncBarrier &barrier_)
        : barriers(1, barrier_), source_subpass(source_subpass_) {}
    SubpassBarrierTrackback &operator=(const SubpassBarrierTrackback &) = default;
};

class AttachmentViewGen {
  public:
    enum Gen { kViewSubresource = 0, kRenderArea = 1, kDepthOnlyRenderArea = 2, kStencilOnlyRenderArea = 3, kGenSize = 4 };
    AttachmentViewGen(const syncval_state::ImageViewState *image_view, const VkOffset3D &offset, const VkExtent3D &extent);
    AttachmentViewGen(const AttachmentViewGen &other) = default;
    AttachmentViewGen(AttachmentViewGen &&other) = default;
    const syncval_state::ImageViewState *GetViewState() const { return view_; }
    const std::optional<ImageRangeGen> &GetRangeGen(Gen type) const;
    bool IsValid() const { return gen_store_[Gen::kViewSubresource].has_value(); }
    Gen GetDepthStencilRenderAreaGenType(bool depth_op, bool stencil_op) const;

  private:
    const syncval_state::ImageViewState *view_ = nullptr;
    std::array<std::optional<ImageRangeGen>, Gen::kGenSize> gen_store_;
};

using AttachmentViewGenVector = std::vector<AttachmentViewGen>;

class AccessContext {
  public:
    using ImageState = syncval_state::ImageState;
    using ImageViewState = syncval_state::ImageViewState;
    using ScopeMap = ResourceAccessRangeMap;
    enum DetectOptions : uint32_t {
        kDetectPrevious = 1U << 0,
        kDetectAsync = 1U << 1,
        kDetectAll = (kDetectPrevious | kDetectAsync)
    };

    using TrackBack = SubpassBarrierTrackback<AccessContext>;

    HazardResult DetectHazard(const vvl::Buffer &buffer, SyncAccessIndex access_index, const ResourceAccessRange &range) const;
    HazardResult DetectHazard(const ImageState &image, SyncAccessIndex current_usage,
                              const VkImageSubresourceRange &subresource_range, bool is_depth_sliced) const;
    HazardResult DetectHazard(const ImageViewState &image_view, SyncAccessIndex current_usage) const;
    HazardResult DetectHazard(const ImageRangeGen &ref_range_gen, SyncAccessIndex current_usage,
                              SyncOrdering ordering_rule = SyncOrdering::kOrderingNone) const;
    HazardResult DetectHazard(const ImageViewState &image_view, const VkOffset3D &offset, const VkExtent3D &extent,
                              SyncAccessIndex current_usage, SyncOrdering ordering_rule) const;
    HazardResult DetectHazard(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, SyncAccessIndex current_usage,
                              SyncOrdering ordering_rule) const;
    HazardResult DetectHazard(const vvl::VideoSession &vs_state, const vvl::VideoPictureResource &resource,
                              SyncAccessIndex current_usage) const;
    HazardResult DetectHazard(const ImageState &image, const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                              const VkExtent3D &extent, bool is_depth_sliced, SyncAccessIndex current_usage,
                              SyncOrdering ordering_rule = SyncOrdering::kOrderingNone) const;

    HazardResult DetectImageBarrierHazard(const ImageState &image, const VkImageSubresourceRange &subresource_range,
                                          VkPipelineStageFlags2 src_exec_scope, const SyncAccessFlags &src_access_scope,
                                          QueueId queue_id, const ScopeMap &scope_map, ResourceUsageTag scope_tag,
                                          DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const AttachmentViewGen &attachment_view, const SyncBarrier &barrier,
                                          DetectOptions options) const;
    HazardResult DetectImageBarrierHazard(const ImageState &image, VkPipelineStageFlags2 src_exec_scope,
                                          const SyncAccessFlags &src_access_scope, const VkImageSubresourceRange &subresource_range,
                                          DetectOptions options) const;
    HazardResult DetectSubpassTransitionHazard(const TrackBack &track_back, const AttachmentViewGen &attach_view) const;

    HazardResult DetectFirstUseHazard(QueueId queue_id, const ResourceUsageRange &tag_range,
                                      const AccessContext &access_context) const;

    const TrackBack &GetDstExternalTrackBack() const { return dst_external_; }
    void Reset() {
        prev_.clear();
        prev_by_subpass_.clear();
        async_.clear();
        src_external_ = nullptr;
        dst_external_ = TrackBack();
        start_tag_ = ResourceUsageTag();
        access_state_map_.clear();
    }

    void ResolvePreviousAccesses();
    void ResolveFromContext(const AccessContext &from);

    template <typename ResolveOp>
    void ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context,
                            const ResourceAccessState *infill_state = nullptr, bool recur_to_infill = false);
    template <typename ResolveOp, typename RangeGenerator>
    void ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context, RangeGenerator range_gen,
                            const ResourceAccessState *infill_state = nullptr, bool recur_to_infill = false);

    void UpdateAccessState(const vvl::Buffer &buffer, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           const ResourceAccessRange &range, ResourceUsageTagEx tag_ex);
    void UpdateAccessState(const ImageState &image, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           const VkImageSubresourceRange &subresource_range, const ResourceUsageTag &tag);
    void UpdateAccessState(const ImageState &image, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset, const VkExtent3D &extent,
                           ResourceUsageTagEx tag_ex);
    void UpdateAccessState(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, SyncAccessIndex current_usage,
                           SyncOrdering ordering_rule, ResourceUsageTag tag);
    void UpdateAccessState(const ImageViewState &image_view, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           const VkOffset3D &offset, const VkExtent3D &extent, ResourceUsageTagEx tag_ex);
    void UpdateAccessState(const ImageViewState &image_view, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           ResourceUsageTagEx tag_ex);
    void UpdateAccessState(const ImageRangeGen &range_gen, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           ResourceUsageTagEx tag_ex);
    void UpdateAccessState(ImageRangeGen &range_gen, SyncAccessIndex current_usage, SyncOrdering ordering_rule,
                           ResourceUsageTagEx tag_ex);
    void UpdateAccessState(const vvl::VideoSession &vs_state, const vvl::VideoPictureResource &resource,
                           SyncAccessIndex current_usage, ResourceUsageTag tag);
    void ResolveChildContexts(const std::vector<AccessContext> &contexts);

    void ImportAsyncContexts(const AccessContext &from);
    void ClearAsyncContexts() { async_.clear(); }
    template <typename Action>
    void ApplyUpdateAction(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, const Action &action);
    template <typename Action>
    void ApplyToContext(const Action &barrier_action);

    AccessContext(uint32_t subpass, VkQueueFlags queue_flags, const std::vector<SubpassDependencyGraphNode> &dependencies,
                  const std::vector<AccessContext> &contexts, const AccessContext *external_context);

    AccessContext() { Reset(); }
    AccessContext(const AccessContext &copy_from) = default;
    void Trim();
    void TrimAndClearFirstAccess();
    void AddReferencedTags(ResourceUsageTagSet &referenced) const;

    ResourceAccessRangeMap &GetAccessStateMap() { return access_state_map_; }
    const ResourceAccessRangeMap &GetAccessStateMap() const { return access_state_map_; }
    const TrackBack *GetTrackBackFromSubpass(uint32_t subpass) const {
        if (subpass == VK_SUBPASS_EXTERNAL) {
            return src_external_;
        } else {
            assert(subpass < prev_by_subpass_.size());
            return prev_by_subpass_[subpass];
        }
    }

    void SetStartTag(ResourceUsageTag tag) { start_tag_ = tag; }
    ResourceUsageTag StartTag() const { return start_tag_; }

    template <typename Action>
    void ForAll(Action &&action);
    template <typename Action>
    void ConstForAll(Action &&action) const;
    template <typename Predicate>
    void EraseIf(Predicate &&pred);

    // For use during queue submit building up the QueueBatchContext AccessContext for validation, otherwise clear.
    void AddAsyncContext(const AccessContext *context, ResourceUsageTag tag, QueueId queue_id);

    class AsyncReference {
      public:
        AsyncReference(const AccessContext &async_context, ResourceUsageTag async_tag, QueueId queue_id)
            : context_(&async_context), tag_(async_tag), queue_id_(queue_id) {}
        const AccessContext &Context() const { return *context_; }
        // For RenderPass time validation this is "start tag", for QueueSubmit, this is the earliest
        // unsynchronized tag for the Queue being tested against (max synchrononous + 1, perhaps)
        ResourceUsageTag StartTag() const;
        QueueId GetQueueId() const { return queue_id_; }

      protected:
        const AccessContext *context_;
        ResourceUsageTag tag_;  // Start of open ended asynchronous range
        QueueId queue_id_;
    };

    template <typename Action, typename RangeGen>
    void UpdateMemoryAccessState(const Action &action, RangeGen &range_gen);

  private:
    template <typename Action>
    static void UpdateMemoryAccessRangeState(ResourceAccessRangeMap &accesses, Action &action, const ResourceAccessRange &range);

    struct UpdateMemoryAccessStateFunctor {
        using Iterator = ResourceAccessRangeMap::iterator;
        Iterator Infill(ResourceAccessRangeMap *accesses, const Iterator &pos, const ResourceAccessRange &range) const;
        void operator()(const Iterator &pos) const;
        UpdateMemoryAccessStateFunctor(const AccessContext &context_, SyncAccessIndex usage_, SyncOrdering ordering_rule_,
                                       ResourceUsageTagEx tag_ex)
            : context(context_), usage_info(SyncStageAccess::AccessInfo(usage_)), ordering_rule(ordering_rule_), tag_ex(tag_ex) {}
        const AccessContext &context;
        const SyncAccessInfo &usage_info;
        const SyncOrdering ordering_rule;
        const ResourceUsageTagEx tag_ex;
    };

    // Follow the context previous to access the access state, supporting "lazy" import into the context. Not intended for
    // subpass layout transition, as the pending state handling is more complex
    // TODO: See if returning the lower_bound would be useful from a performance POV -- look at the lower_bound overhead
    // Would need to add a "hint" overload to parallel_iterator::invalidate_[AB] call, if so.
    void ResolvePreviousAccess(const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                               const ResourceAccessState *infill_state,
                               const ResourceAccessStateFunction *previous_barrier = nullptr) const;
    template <typename BarrierAction>
    void ResolvePreviousAccessStack(const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                                    const ResourceAccessState *infill_state, const BarrierAction &previous_barrie) const;
    template <typename BarrierAction>
    void ResolveAccessRange(const ResourceAccessRange &range, BarrierAction &barrier_action, ResourceAccessRangeMap *resolve_map,
                            const ResourceAccessState *infill_state, bool recur_to_infill = true) const;

    template <typename Detector>
    HazardResult DetectHazardRange(Detector &detector, const ResourceAccessRange &range, DetectOptions options) const;
    template <typename Detector, typename RangeGen>
    HazardResult DetectHazardGeneratedRanges(Detector &detector, RangeGen &range_gen, DetectOptions options) const;
    template <typename Detector, typename RangeGen>
    HazardResult DetectHazardGeneratedRanges(Detector &detector, const RangeGen &range_gen, DetectOptions options) const {
        RangeGen mutable_gen(range_gen);
        return DetectHazardGeneratedRanges<Detector, RangeGen>(detector, mutable_gen, options);
    }
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                              DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const ImageState &image, const VkImageSubresourceRange &subresource_range,
                              const VkOffset3D &offset, const VkExtent3D &extent, bool is_depth_sliced,
                              DetectOptions options) const;
    template <typename Detector>
    HazardResult DetectHazard(Detector &detector, const ImageState &image, const VkImageSubresourceRange &subresource_range,
                              bool is_depth_sliced, DetectOptions options) const;

    template <typename Detector>
    HazardResult DetectHazardOneRange(Detector &detector, bool detect_prev, ResourceAccessRangeMap::const_iterator &pos,
                                      const ResourceAccessRangeMap::const_iterator &the_end,
                                      const ResourceAccessRange &range) const;

    template <typename Detector, typename RangeGen>
    HazardResult DetectAsyncHazard(const Detector &detector, const RangeGen &const_range_gen, ResourceUsageTag async_tag,
                                   QueueId async_queue_id) const;

    template <typename NormalizeOp>
    void Trim(NormalizeOp &&normalize);

    template <typename Detector>
    HazardResult DetectPreviousHazard(Detector &detector, const ResourceAccessRange &range) const;

    ResourceAccessRangeMap access_state_map_;
    std::vector<TrackBack> prev_;
    std::vector<TrackBack *> prev_by_subpass_;
    // These contexts *must* have the same lifespan as this context, or be cleared, before the referenced contexts can expire
    std::vector<AsyncReference> async_;
    TrackBack *src_external_;
    TrackBack dst_external_;
    ResourceUsageTag start_tag_;
};

// The semantics of the InfillUpdateOps of infill_update_range are slightly different than for the UpdateMemoryAccessState Action
// operations, as this simplifies the generic traversal.  So we wrap them in a semantics Adapter to get the same effect.
template <typename Action>
struct ActionToOpsAdapter {
    using Map = ResourceAccessRangeMap;
    using Range = typename Map::key_type;
    using Iterator = typename Map::iterator;
    using IndexType = typename Map::index_type;

    void infill(Map &accesses, const Iterator &pos, const Range &infill_range) const {
        // Combine Infill and update operations to make the generic implementation simpler
        Iterator infill = action.Infill(&accesses, pos, infill_range);
        if (infill == accesses.end()) return;  // Allow action to 'pass' on filling in the blanks

        // Need to apply the action to the Infill.  'infill_update_range' expect ops.infill to be completely done with
        // the infill_range, where as Action::Infill assumes the caller will apply the action() logic to the infill_range
        for (; infill != pos; ++infill) {
            assert(infill != accesses.end());
            action(infill);
        }
    }
    void update(const Iterator &pos) const { action(pos); }
    const Action &action;
};

template <typename Action>
void AccessContext::ApplyToContext(const Action &barrier_action) {
    // Note: Barriers do *not* cross context boundaries, applying to accessess within.... (at least for renderpass subpasses)
    UpdateMemoryAccessRangeState(access_state_map_, barrier_action, kFullRange);
}

template <typename Action>
void AccessContext::UpdateMemoryAccessRangeState(ResourceAccessRangeMap &accesses, Action &action,
                                                 const ResourceAccessRange &range) {
    ActionToOpsAdapter<Action> ops{action};
    infill_update_range(accesses, range, ops);
}

template <typename Action, typename RangeGen>
void AccessContext::UpdateMemoryAccessState(const Action &action, RangeGen &range_gen) {
    ActionToOpsAdapter<Action> ops{action};
    infill_update_rangegen(access_state_map_, range_gen, ops);
}

template <typename Action>
void AccessContext::ApplyUpdateAction(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, const Action &action) {
    const std::optional<ImageRangeGen> &ref_range_gen = view_gen.GetRangeGen(gen_type);
    if (ref_range_gen) {
        ImageRangeGen range_gen(*ref_range_gen);
        UpdateMemoryAccessState(action, range_gen);
    }
}

// A non recursive range walker for the asynchronous contexts (those we have no barriers with)
template <typename Detector, typename RangeGen>
HazardResult AccessContext::DetectAsyncHazard(const Detector &detector, const RangeGen &const_range_gen, ResourceUsageTag async_tag,
                                              QueueId async_queue_id) const {
    using RangeType = typename RangeGen::RangeType;
    using ConstIterator = ResourceAccessRangeMap::const_iterator;
    RangeGen range_gen(const_range_gen);

    HazardResult hazard;

    auto do_async_hazard_check = [&detector, async_tag, async_queue_id, &hazard](const RangeType &range, const ConstIterator &end,
                                                                                 ConstIterator &pos) {
        while (pos != end && pos->first.begin < range.end) {
            hazard = detector.DetectAsync(pos, async_tag, async_queue_id);
            if (hazard.IsHazard()) return true;
            ++pos;
        }
        return false;
    };

    ForEachEntryInRangesUntil(access_state_map_, range_gen, do_async_hazard_check);

    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectHazardOneRange(Detector &detector, bool detect_prev, ResourceAccessRangeMap::const_iterator &pos,
                                                 const ResourceAccessRangeMap::const_iterator &the_end,
                                                 const ResourceAccessRange &range) const {
    HazardResult hazard;
    ResourceAccessRange gap = {range.begin, range.begin};

    while (pos != the_end && pos->first.begin < range.end) {
        // Cover any leading gap, or gap between entries
        if (detect_prev) {
            // TODO: After profiling we may want to change the descent logic such that we don't recur per gap...
            // Cover any leading gap, or gap between entries
            gap.end = pos->first.begin;  // We know this begin is < range.end
            if (gap.non_empty()) {
                // Recur on all gaps
                hazard = DetectPreviousHazard(detector, gap);
                if (hazard.IsHazard()) return hazard;
            }
            // Set up for the next gap.  If pos..end is >= range.end, loop will exit, and trailing gap will be empty
            gap.begin = pos->first.end;
        }

        hazard = detector.Detect(pos);
        if (hazard.IsHazard()) return hazard;
        ++pos;
    }

    if (detect_prev) {
        // Detect in the trailing empty as needed
        gap.end = range.end;
        if (gap.non_empty()) {
            hazard = DetectPreviousHazard(detector, gap);
        }
    }

    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectHazardRange(Detector &detector, const ResourceAccessRange &range, DetectOptions options) const {
    SingleRangeGenerator range_gen(range);
    return DetectHazardGeneratedRanges(detector, range_gen, options);
}

template <typename BarrierAction>
void AccessContext::ResolveAccessRange(const ResourceAccessRange &range, BarrierAction &barrier_action,
                                       ResourceAccessRangeMap *resolve_map, const ResourceAccessState *infill_state,
                                       bool recur_to_infill) const {
    if (!range.non_empty()) return;

    ResourceRangeMergeIterator current(*resolve_map, access_state_map_, range.begin);
    while (current->range.non_empty() && range.includes(current->range.begin)) {
        const auto current_range = current->range & range;
        if (current->pos_B->valid) {
            const auto &src_pos = current->pos_B->lower_bound;
            ResourceAccessState access(src_pos->second);  // intentional copy
            barrier_action(&access);
            if (current->pos_A->valid) {
                const auto trimmed = sparse_container::split(current->pos_A->lower_bound, *resolve_map, current_range);
                trimmed->second.Resolve(access);
                current.invalidate_A(trimmed);
            } else {
                auto inserted = resolve_map->insert(current->pos_A->lower_bound, std::make_pair(current_range, access));
                current.invalidate_A(inserted);  // Update the parallel iterator to point at the insert segment
            }
        } else {
            // we have to descend to fill this gap
            if (recur_to_infill) {
                ResourceAccessRange recurrence_range = current_range;
                // The current context is empty for the current range, so recur to fill the gap.
                // Since we will be recurring back up the DAG, expand the gap descent to cover the full range for which B
                // is not valid, to minimize that recurrence
                if (current->pos_B.at_end()) {
                    // Do the remainder here....
                    recurrence_range.end = range.end;
                } else {
                    // Recur only over the range until B becomes valid (within the limits of range).
                    recurrence_range.end = std::min(range.end, current->pos_B->lower_bound->first.begin);
                }
                ResolvePreviousAccessStack(recurrence_range, resolve_map, infill_state, barrier_action);

                // Given that there could be gaps we need to seek carefully to not repeatedly search the same gaps in the next
                // iterator of the outer while.

                // Set the parallel iterator to the end of this range s.t. ++ will move us to the next range whether or
                // not the end of the range is a gap.  For the seek to work, first we need to warn the parallel iterator
                // we stepped on the dest map
                const auto seek_to = recurrence_range.end - 1;  // The subtraction is safe as range can't be empty (loop condition)
                current.invalidate_A();                         // Changes current->range
                current.seek(seek_to);
            } else if (!current->pos_A->valid && infill_state) {
                // If we didn't find anything in the current range, and we aren't reccuring... we infill if required
                auto inserted = resolve_map->insert(current->pos_A->lower_bound, std::make_pair(current->range, *infill_state));
                current.invalidate_A(inserted);  // Update the parallel iterator to point at the correct segment after insert
            }
        }
        if (current->range.non_empty()) {
            ++current;
        }
    }

    // Infill if range goes passed both the current and resolve map prior contents
    if (recur_to_infill && (current->range.end < range.end)) {
        ResourceAccessRange trailing_fill_range = {current->range.end, range.end};
        ResolvePreviousAccessStack<BarrierAction>(trailing_fill_range, resolve_map, infill_state, barrier_action);
    }
}

// A recursive range walker for hazard detection, first for the current context and the (DetectHazardRecur) to walk
// the DAG of the contexts (for example subpasses)
template <typename Detector, typename RangeGen>
HazardResult AccessContext::DetectHazardGeneratedRanges(Detector &detector, RangeGen &range_gen, DetectOptions options) const {
    HazardResult hazard;

    // Do this before range_gen is incremented s.t. the copies used will be correct
    if (static_cast<uint32_t>(options) & DetectOptions::kDetectAsync) {
        // Async checks don't require recursive lookups, as the async lists are exhaustive for the top-level context
        // so we'll check these first
        for (const auto &async_ref : async_) {
            hazard = async_ref.Context().DetectAsyncHazard(detector, range_gen, async_ref.StartTag(), async_ref.GetQueueId());
            if (hazard.IsHazard()) return hazard;
        }
    }

    const bool detect_prev = (static_cast<uint32_t>(options) & DetectOptions::kDetectPrevious) != 0;

    using RangeType = typename RangeGen::RangeType;
    using ConstIterator = ResourceAccessRangeMap::const_iterator;
    auto do_detect_hazard_range = [this, &detector, &hazard, detect_prev](const RangeType &range, const ConstIterator &end,
                                                                          ConstIterator &pos) {
        hazard = DetectHazardOneRange(detector, detect_prev, pos, end, range);
        return hazard.IsHazard();
    };

    ForEachEntryInRangesUntil(access_state_map_, range_gen, do_detect_hazard_range);

    return hazard;
}

template <typename Detector>
HazardResult AccessContext::DetectPreviousHazard(Detector &detector, const ResourceAccessRange &range) const {
    ResourceAccessRangeMap descent_map;
    ResolvePreviousAccess(range, &descent_map, nullptr);

    for (auto prev = descent_map.begin(); prev != descent_map.end(); ++prev) {
        HazardResult hazard = detector.Detect(prev);
        if (hazard.IsHazard()) {
            return hazard;
        }
    }
    return {};
}

template <typename Predicate>
void AccessContext::EraseIf(Predicate &&pred) {
    // Note: Don't forward, we don't want r-values moved, since we're going to make multiple calls.
    vvl::EraseIf(access_state_map_, pred);
}

template <typename ResolveOp>
void AccessContext::ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context,
                                       const ResourceAccessState *infill_state, bool recur_to_infill) {
    from_context.ResolveAccessRange(kFullRange, resolve_op, &access_state_map_, infill_state, recur_to_infill);
}

template <typename ResolveOp, typename RangeGenerator>
void AccessContext::ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context, RangeGenerator range_gen,
                                       const ResourceAccessState *infill_state, bool recur_to_infill) {
    for (; range_gen->non_empty(); ++range_gen) {
        from_context.ResolveAccessRange(*range_gen, resolve_op, &access_state_map_, infill_state, recur_to_infill);
    }
}

template <typename BarrierAction>
void AccessContext::ResolvePreviousAccessStack(const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                                               const ResourceAccessState *infill_state,
                                               const BarrierAction &previous_barrier) const {
    ResourceAccessStateFunction stacked_barrier(std::ref(previous_barrier));
    ResolvePreviousAccess(range, descent_map, infill_state, &stacked_barrier);
}
