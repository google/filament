/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gltfio/Animator.h>
#include <gltfio/math.h>

#include "FFilamentAsset.h"
#include "FFilamentInstance.h"
#include "FTrsTransformManager.h"
#include "downcast.h"

#include <filament/VertexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

#include <utils/Log.h>

#include <math/mat4.h>
#include <math/quat.h>
#include <math/scalar.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <map>
#include <string>
#include <vector>

using namespace filament;
using namespace filament::math;
using namespace std;
using namespace utils;

namespace filament::gltfio {

using TimeValues = map<float, size_t>;
using SourceValues = vector<float>;
using BoneVector = vector<mat4f>;

struct Sampler {
    TimeValues times;
    SourceValues values;
    enum { LINEAR, STEP, CUBIC } interpolation;
};

struct Channel {
    const Sampler* sourceData;
    Entity targetEntity;
    enum { TRANSLATION, ROTATION, SCALE, WEIGHTS } transformType;
};

struct Animation {
    float duration;
    std::string name;
    vector<Sampler> samplers;
    vector<Channel> channels;
};

struct AnimatorImpl {
    vector<Animation> animations;
    BoneVector boneMatrices;
    FFilamentAsset const* asset = nullptr;
    FFilamentInstance* instance = nullptr;
    RenderableManager* renderableManager;
    TransformManager* transformManager;
    TrsTransformManager* trsTransformManager;
    vector<float> weights;
    FixedCapacityVector<mat4f> crossFade;
    void addChannels(const FixedCapacityVector<Entity>& nodeMap, const cgltf_animation& srcAnim,
            Animation& dst);
    void applyAnimation(const Channel& channel, float t, size_t prevIndex, size_t nextIndex);
    void stashCrossFade();
    void applyCrossFade(float alpha);
    void resetBoneMatrices(FFilamentInstance* instance);
    void updateBoneMatrices(FFilamentInstance* instance);
};

static void createSampler(const cgltf_animation_sampler& src, Sampler& dst) {
    // Copy the time values into a red-black tree.
    const cgltf_accessor* timelineAccessor = src.input;
    const uint8_t* timelineBlob = nullptr;
    const float* timelineFloats = nullptr;
    if (timelineAccessor->buffer_view->has_meshopt_compression) {
        timelineBlob = (const uint8_t*) timelineAccessor->buffer_view->data;
        timelineFloats = (const float*) (timelineBlob + timelineAccessor->offset);
    } else {
        timelineBlob = (const uint8_t*) timelineAccessor->buffer_view->buffer->data;
        timelineFloats = (const float*) (timelineBlob + timelineAccessor->offset +
                timelineAccessor->buffer_view->offset);
    }
    for (size_t i = 0, len = timelineAccessor->count; i < len; ++i) {
        dst.times[timelineFloats[i]] = i;
    }

    // Convert source data to float.
    const cgltf_accessor* valuesAccessor = src.output;
    switch (valuesAccessor->type) {
        case cgltf_type_scalar:
            dst.values.resize(valuesAccessor->count);
            cgltf_accessor_unpack_floats(src.output, &dst.values[0], valuesAccessor->count);
            break;
        case cgltf_type_vec3:
            dst.values.resize(valuesAccessor->count * 3);
            cgltf_accessor_unpack_floats(src.output, &dst.values[0], valuesAccessor->count * 3);
            break;
        case cgltf_type_vec4:
            dst.values.resize(valuesAccessor->count * 4);
            cgltf_accessor_unpack_floats(src.output, &dst.values[0], valuesAccessor->count * 4);
            break;
        default:
            GLTFIO_WARN("Unknown animation type.");
            return;
    }

    switch (src.interpolation) {
        case cgltf_interpolation_type_linear:
            dst.interpolation = Sampler::LINEAR;
            break;
        case cgltf_interpolation_type_step:
            dst.interpolation = Sampler::STEP;
            break;
        case cgltf_interpolation_type_cubic_spline:
            dst.interpolation = Sampler::CUBIC;
            break;
        case cgltf_interpolation_type_max_enum:
            break;
    }
}

static void setTransformType(const cgltf_animation_channel& src, Channel& dst) {
    switch (src.target_path) {
        case cgltf_animation_path_type_translation:
            dst.transformType = Channel::TRANSLATION;
            break;
        case cgltf_animation_path_type_rotation:
            dst.transformType = Channel::ROTATION;
            break;
        case cgltf_animation_path_type_scale:
            dst.transformType = Channel::SCALE;
            break;
        case cgltf_animation_path_type_weights:
            dst.transformType = Channel::WEIGHTS;
            break;
        case cgltf_animation_path_type_max_enum:
        case cgltf_animation_path_type_invalid:
            GLTFIO_WARN("Unsupported channel path.");
            break;
    }
}

static bool validateAnimation(const cgltf_animation& anim) {
    for (cgltf_size j = 0; j < anim.channels_count; ++j) {
        const cgltf_animation_channel& channel = anim.channels[j];
        const cgltf_animation_sampler* sampler = channel.sampler;
        if (!channel.target_node) {
            continue;
        }
        if (!channel.sampler) {
            return false;
        }
        cgltf_size components = 1;
        if (channel.target_path == cgltf_animation_path_type_weights) {
            if (!channel.target_node->mesh || !channel.target_node->mesh->primitives_count) {
                return false;
            }
            components = channel.target_node->mesh->primitives[0].targets_count;
        }
        cgltf_size values = sampler->interpolation == cgltf_interpolation_type_cubic_spline ? 3 : 1;
        if (sampler->input->count * components * values != sampler->output->count) {
            return false;
        }
    }
    return true;
}

Animator::Animator(FFilamentAsset const* asset, FFilamentInstance* instance) {
    assert(asset->mResourcesLoaded && asset->mSourceAsset);
    mImpl = new AnimatorImpl();
    mImpl->asset = asset;
    mImpl->instance = instance;
    mImpl->renderableManager = &asset->mEngine->getRenderableManager();
    mImpl->transformManager = &asset->mEngine->getTransformManager();
    mImpl->trsTransformManager = asset->getTrsTransformManager();

    const cgltf_data* srcAsset = asset->mSourceAsset->hierarchy;
    const cgltf_animation* srcAnims = srcAsset->animations;
    for (cgltf_size i = 0, len = srcAsset->animations_count; i < len; ++i) {
        const cgltf_animation& anim = srcAnims[i];
        if (!validateAnimation(anim)) {
            GLTFIO_WARN("Disabling animation due to validation failure.");
            return;
        }
    }

    // Loop over the glTF animation definitions.
    mImpl->animations.resize(srcAsset->animations_count);
    for (cgltf_size i = 0, len = srcAsset->animations_count; i < len; ++i) {
        const cgltf_animation& srcAnim = srcAnims[i];
        Animation& dstAnim = mImpl->animations[i];
        dstAnim.duration = 0;
        if (srcAnim.name) {
            dstAnim.name = srcAnim.name;
        }

        // Import each glTF sampler into a custom data structure.
        cgltf_animation_sampler* srcSamplers = srcAnim.samplers;
        dstAnim.samplers.resize(srcAnim.samplers_count);
        for (cgltf_size j = 0, nsamps = srcAnim.samplers_count; j < nsamps; ++j) {
            const cgltf_animation_sampler& srcSampler = srcSamplers[j];
            Sampler& dstSampler = dstAnim.samplers[j];
            createSampler(srcSampler, dstSampler);
            if (dstSampler.times.size() > 1) {
                float maxtime = (--dstSampler.times.end())->first;
                dstAnim.duration = std::max(dstAnim.duration, maxtime);
            }
        }

        // Import each glTF channel into a custom data structure.
        if (instance) {
            mImpl->addChannels(instance->mNodeMap, srcAnim, dstAnim);
        } else {
            for (FFilamentInstance* instance : asset->mInstances) {
                mImpl->addChannels(instance->mNodeMap, srcAnim, dstAnim);
            }
        }
    }
}

void Animator::applyCrossFade(size_t previousAnimIndex, float previousAnimTime, float alpha) {
    mImpl->stashCrossFade();
    applyAnimation(previousAnimIndex, previousAnimTime);
    mImpl->applyCrossFade(alpha);
}

void Animator::addInstance(FFilamentInstance* instance) {
    const cgltf_data* srcAsset = mImpl->asset->mSourceAsset->hierarchy;
    const cgltf_animation* srcAnims = srcAsset->animations;
    for (cgltf_size i = 0, len = srcAsset->animations_count; i < len; ++i) {
        const cgltf_animation& srcAnim = srcAnims[i];
        Animation& dstAnim = mImpl->animations[i];
        mImpl->addChannels(instance->mNodeMap, srcAnim, dstAnim);
    }
}

Animator::Animator(FilamentAsset *asset, FilamentInstance *instance) : Animator(reinterpret_cast<FFilamentAsset*>(asset), reinterpret_cast<FFilamentInstance*>(instance)) {
}

Animator::~Animator() {
    delete mImpl;
}

size_t Animator::getAnimationCount() const {
    return mImpl->animations.size();
}

void Animator::applyAnimation(size_t animationIndex, float time) const {
    const Animation& anim = mImpl->animations[animationIndex];
    time = time == anim.duration ? time : fmod(time, anim.duration);
    TransformManager& transformManager = *mImpl->transformManager;
    transformManager.openLocalTransformTransaction();
    for (const auto& channel : anim.channels) {
        const Sampler* sampler = channel.sourceData;
        if (sampler->times.size() < 2) {
            continue;
        }

        const TimeValues& times = sampler->times;

        // Find the first keyframe after the given time, or the keyframe that matches it exactly.
        TimeValues::const_iterator iter = times.lower_bound(time);

        // Compute the interpolant (between 0 and 1) and determine the keyframe pair.
        float t = 0.0f;
        size_t nextIndex;
        size_t prevIndex;
        if (iter == times.end()) {
            nextIndex = times.size() - 1;
            prevIndex = nextIndex;
        } else if (iter == times.begin()) {
            nextIndex = 0;
            prevIndex = 0;
        } else {
            TimeValues::const_iterator prev = iter; --prev;
            nextIndex = iter->second;
            prevIndex = prev->second;
            const float nextTime = iter->first;
            const float prevTime = prev->first;
            float deltaTime = nextTime - prevTime;
            assert(deltaTime >= 0);
            if (deltaTime > 0) {
                t = (time - prevTime) / deltaTime;
            }
        }

        if (sampler->interpolation == Sampler::STEP) {
            t = 0.0f;
        }

        mImpl->applyAnimation(channel, t, prevIndex, nextIndex);
    }
    transformManager.commitLocalTransformTransaction();
}

void Animator::resetBoneMatrices() {
    // If this is a single-instance animator, then reset only this instance.
    if (mImpl->instance) {
        mImpl->resetBoneMatrices(mImpl->instance);
        return;
    }

    // If this is a broadcast animator, then reset all instances.
    for (FFilamentInstance* instance : mImpl->asset->mInstances) {
        mImpl->resetBoneMatrices(instance);
    }
}

void Animator::updateBoneMatrices() {
    // If this is a single-instance animator, then update only this instance.
    if (mImpl->instance) {
        mImpl->updateBoneMatrices(mImpl->instance);
        return;
    }

    // If this is a broadcast animator, then update all instances.
    for (FFilamentInstance* instance : mImpl->asset->mInstances) {
        mImpl->updateBoneMatrices(instance);
    }
}

float Animator::getAnimationDuration(size_t animationIndex) const {
    return mImpl->animations[animationIndex].duration;
}

const char* Animator::getAnimationName(size_t animationIndex) const {
    return mImpl->animations[animationIndex].name.c_str();
}

void AnimatorImpl::stashCrossFade() {
    using Instance = TransformManager::Instance;
    auto& tm = *this->transformManager;
    auto& stash = this->crossFade;

    // Count the total number of transformable nodes to preallocate the stash memory.
    // We considered caching this count, but the cache would need to be invalidated when entities
    // are added into the hierarchy.
    auto recursiveCount = [&tm](Instance node, size_t count, auto& fn) -> size_t {
        ++count;
        for (auto iter = tm.getChildrenBegin(node); iter != tm.getChildrenEnd(node); ++iter) {
            count = fn(*iter, count, fn);
        }
        return count;
    };

    auto recursiveStash = [&tm, &stash](Instance node, size_t index, auto& fn) -> size_t {
        stash[index++] = tm.getTransform(node);
        for (auto iter = tm.getChildrenBegin(node); iter != tm.getChildrenEnd(node); ++iter) {
            index = fn(*iter, index, fn);
        }
        return index;
    };

    const Entity rootEntity = instance ? instance->getRoot() : asset->mRoot;
    const Instance root = tm.getInstance(rootEntity);
    const size_t count = recursiveCount(root, 0, recursiveCount);
    crossFade.reserve(count);
    crossFade.resize(count);
    recursiveStash(root, 0, recursiveStash);
}

void AnimatorImpl::applyCrossFade(float alpha) {
    using Instance = TransformManager::Instance;
    auto& tm = *this->transformManager;
    auto& stash = this->crossFade;
    auto recursiveFn = [&tm, &stash, alpha](Instance node, size_t index, auto& fn) -> size_t {
        float3 scale0, scale1;
        quatf rotation0, rotation1;
        float3 translation0, translation1;
        decomposeMatrix(stash[index++], &translation1, &rotation1, &scale1);
        decomposeMatrix(tm.getTransform(node), &translation0, &rotation0, &scale0);
        const float3 scale = mix(scale0, scale1, alpha);
        const quatf rotation = slerp(rotation0, rotation1, alpha);
        const float3 translation = mix(translation0, translation1, alpha);
        tm.setTransform(node, composeMatrix(translation, rotation, scale));
        for (auto iter = tm.getChildrenBegin(node); iter != tm.getChildrenEnd(node); ++iter) {
            index = fn(*iter, index, fn);
        }
        return index;
    };
    const Entity rootEntity = instance ? instance->getRoot() : asset->mRoot;
    const Instance root = tm.getInstance(rootEntity);
    recursiveFn(root, 0, recursiveFn);
}

void AnimatorImpl::addChannels(const FixedCapacityVector<Entity>& nodeMap,
        const cgltf_animation& srcAnim, Animation& dst) {
    const cgltf_animation_channel* srcChannels = srcAnim.channels;
    const cgltf_animation_sampler* srcSamplers = srcAnim.samplers;
    const cgltf_node* nodes = asset->mSourceAsset->hierarchy->nodes;
    const Sampler* samplers = dst.samplers.data();
    for (cgltf_size j = 0, nchans = srcAnim.channels_count; j < nchans; ++j) {
        const cgltf_animation_channel& srcChannel = srcChannels[j];
        if (!srcChannel.target_node) {
            continue;
        }
        Entity targetEntity = nodeMap[srcChannel.target_node - nodes];
        if (UTILS_UNLIKELY(!targetEntity)) {
            if (GLTFIO_VERBOSE) {
                slog.w << "No scene root contains node ";
                if (srcChannel.target_node->name) {
                    slog.w << "'" << srcChannel.target_node->name << "' ";
                }
                slog.w << "for animation ";
                if (srcAnim.name) {
                    slog.w << "'" << srcAnim.name << "' ";
                }
                slog.w << "in channel " << j << io::endl;
            }
            continue;
        }
        Channel dstChannel;
        dstChannel.sourceData = samplers + (srcChannel.sampler - srcSamplers);
        dstChannel.targetEntity = targetEntity;
        setTransformType(srcChannel, dstChannel);
        dst.channels.push_back(dstChannel);
    }
}

void AnimatorImpl::applyAnimation(const Channel& channel, float t, size_t prevIndex,
        size_t nextIndex) {
    const Sampler* sampler = channel.sourceData;
    const TimeValues& times = sampler->times;
    TrsTransformManager::Instance trsNode = trsTransformManager->getInstance(channel.targetEntity);
    TransformManager::Instance node = transformManager->getInstance(channel.targetEntity);
    switch (channel.transformType) {

        case Channel::SCALE: {
            float3 scale;
            const float3* srcVec3 = (const float3*) sampler->values.data();
            if (sampler->interpolation == Sampler::CUBIC) {
                float3 vert0 = srcVec3[prevIndex * 3 + 1];
                float3 tang0 = srcVec3[prevIndex * 3 + 2];
                float3 tang1 = srcVec3[nextIndex * 3];
                float3 vert1 = srcVec3[nextIndex * 3 + 1];
                scale = cubicSpline(vert0, tang0, vert1, tang1, t);
            } else {
                scale = ((1 - t) * srcVec3[prevIndex]) + (t * srcVec3[nextIndex]);
            }
            trsTransformManager->setScale(trsNode, scale);
            break;
        }

        case Channel::TRANSLATION: {
            float3 translation;
            const float3* srcVec3 = (const float3*) sampler->values.data();
            if (sampler->interpolation == Sampler::CUBIC) {
                float3 vert0 = srcVec3[prevIndex * 3 + 1];
                float3 tang0 = srcVec3[prevIndex * 3 + 2];
                float3 tang1 = srcVec3[nextIndex * 3];
                float3 vert1 = srcVec3[nextIndex * 3 + 1];
                translation = cubicSpline(vert0, tang0, vert1, tang1, t);
            } else {
                translation = ((1 - t) * srcVec3[prevIndex]) + (t * srcVec3[nextIndex]);
            }
            trsTransformManager->setTranslation(trsNode, translation);
            break;
        }

        case Channel::ROTATION: {
            quatf rotation;
            const quatf* srcQuat = (const quatf*) sampler->values.data();
            if (sampler->interpolation == Sampler::CUBIC) {
                quatf vert0 = srcQuat[prevIndex * 3 + 1];
                quatf tang0 = srcQuat[prevIndex * 3 + 2];
                quatf tang1 = srcQuat[nextIndex * 3];
                quatf vert1 = srcQuat[nextIndex * 3 + 1];
                rotation = normalize(cubicSpline(vert0, tang0, vert1, tang1, t));
            } else {
                rotation = slerp(srcQuat[prevIndex], srcQuat[nextIndex], t);
            }
            trsTransformManager->setRotation(trsNode, rotation);
            break;
        }

        case Channel::WEIGHTS: {
            const float* const samplerValues = sampler->values.data();
            assert(sampler->values.size() % times.size() == 0);
            const int valuesPerKeyframe = sampler->values.size() / times.size();

            if (sampler->interpolation == Sampler::CUBIC) {
                assert(valuesPerKeyframe % 3 == 0);
                const int numMorphTargets = valuesPerKeyframe / 3;
                const float* const inTangents = samplerValues;
                const float* const splineVerts = samplerValues + numMorphTargets;
                const float* const outTangents = samplerValues + numMorphTargets * 2;

                weights.resize(numMorphTargets);
                for (int comp = 0; comp < numMorphTargets; ++comp) {
                    float vert0 = splineVerts[comp + prevIndex * valuesPerKeyframe];
                    float tang0 = outTangents[comp + prevIndex * valuesPerKeyframe];
                    float tang1 = inTangents[comp + nextIndex * valuesPerKeyframe];
                    float vert1 = splineVerts[comp + nextIndex * valuesPerKeyframe];
                    weights[comp] = cubicSpline(vert0, tang0, vert1, tang1, t);
                }
            } else {
                weights.resize(valuesPerKeyframe);
                for (int comp = 0; comp < valuesPerKeyframe; ++comp) {
                    float previous = samplerValues[comp + prevIndex * valuesPerKeyframe];
                    float current = samplerValues[comp + nextIndex * valuesPerKeyframe];
                    weights[comp] = (1 - t) * previous + t * current;
                }
            }

            auto ci = renderableManager->getInstance(channel.targetEntity);
            renderableManager->setMorphWeights(ci, weights.data(), weights.size());
            return;
        }
    }

    transformManager->setTransform(node, trsTransformManager->getTransform(trsNode));
}

void AnimatorImpl::resetBoneMatrices(FFilamentInstance* instance) {
    for (const auto& skin : instance->mSkins) {
        size_t njoints = skin.joints.size();
        boneMatrices.resize(njoints);
        for (const auto& entity : skin.targets) {
            auto renderable = renderableManager->getInstance(entity);
            if (renderable) {
                for (size_t boneIndex = 0; boneIndex < njoints; ++boneIndex) {
                    boneMatrices[boneIndex] = mat4f();
                }
                renderableManager->setBones(renderable, boneMatrices.data(), boneMatrices.size());
            }
        }
    }
}

void AnimatorImpl::updateBoneMatrices(FFilamentInstance* instance) {
    assert_invariant(instance->mSkins.size() == asset->mSkins.size());
    size_t skinIndex = 0;
    for (const auto& skin : instance->mSkins) {
        const auto& assetSkin = asset->mSkins[skinIndex++];
        size_t njoints = skin.joints.size();
        boneMatrices.resize(njoints);
        for (Entity entity : skin.targets) {
            auto renderable = renderableManager->getInstance(entity);
            if (!renderable) {
                continue;
            }
            mat4 inverseGlobalTransform;
            auto xformable = transformManager->getInstance(entity);
            if (xformable) {
                inverseGlobalTransform = inverse(transformManager->getWorldTransformAccurate(xformable));
            }
            for (size_t boneIndex = 0; boneIndex < njoints; ++boneIndex) {
                const auto& joint = skin.joints[boneIndex];
                assert_invariant(assetSkin.inverseBindMatrices.size() > boneIndex);
                const mat4f& inverseBindMatrix = assetSkin.inverseBindMatrices[boneIndex];
                TransformManager::Instance jointInstance = transformManager->getInstance(joint);
                mat4 globalJointTransform = transformManager->getWorldTransformAccurate(jointInstance);
                assert_invariant(boneMatrices.size() > boneIndex);
                boneMatrices[boneIndex] =
                        mat4f{ inverseGlobalTransform * globalJointTransform } *
                        inverseBindMatrix;
            }
            renderableManager->setBones(renderable, boneMatrices.data(), boneMatrices.size());
        }
    }
}

} // namespace filament::gltfio
