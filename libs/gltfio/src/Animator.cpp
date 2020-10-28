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

#include "FFilamentAsset.h"
#include "FFilamentInstance.h"
#include "math.h"
#include "upcast.h"

#include <filament/MaterialEnums.h>
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

namespace gltfio {

using TimeValues = std::map<float, size_t>;
using SourceValues = std::vector<float>;
using BoneVector = std::vector<filament::math::mat4f>;

struct Sampler {
    TimeValues times;
    SourceValues values;
    enum { LINEAR, STEP, CUBIC } interpolation;
};

struct Channel {
    const Sampler* sourceData;
    utils::Entity targetEntity;
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
    FFilamentAsset* asset = nullptr;
    FFilamentInstance* instance = nullptr;
    RenderableManager* renderableManager;
    TransformManager* transformManager;
};

static void createSampler(const cgltf_animation_sampler& src, Sampler& dst) {
    // Copy the time values into a red-black tree.
    const cgltf_accessor* timelineAccessor = src.input;
    const uint8_t* timelineBlob = (const uint8_t*) timelineAccessor->buffer_view->buffer->data;
    const float* timelineFloats = (const float*) (timelineBlob + timelineAccessor->offset +
            timelineAccessor->buffer_view->offset);
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
            slog.e << "Unknown animation type." << io::endl;
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
        case cgltf_animation_path_type_invalid:
            slog.e << "Unsupported channel path." << io::endl;
            break;
    }
}

Animator::Animator(FFilamentAsset* asset, FFilamentInstance* instance) {
    assert(asset->mResourcesLoaded && !asset->mIsReleased);
    mImpl = new AnimatorImpl();
    mImpl->asset = asset;
    mImpl->instance = instance;
    mImpl->renderableManager = &asset->mEngine->getRenderableManager();
    mImpl->transformManager = &asset->mEngine->getTransformManager();

    auto addChannels = [](const NodeMap& nodeMap, const cgltf_animation& srcAnim, Animation& dst) {
        cgltf_animation_channel* srcChannels = srcAnim.channels;
        cgltf_animation_sampler* srcSamplers = srcAnim.samplers;
        const Sampler* samplers = dst.samplers.data();
        for (cgltf_size j = 0, nchans = srcAnim.channels_count; j < nchans; ++j) {
            const cgltf_animation_channel& srcChannel = srcChannels[j];
            utils::Entity targetEntity = nodeMap.at(srcChannel.target_node);
            Channel dstChannel;
            dstChannel.sourceData = samplers + (srcChannel.sampler - srcSamplers);
            dstChannel.targetEntity = targetEntity;
            setTransformType(srcChannel, dstChannel);
            dst.channels.push_back(dstChannel);
        }
    };

    // Loop over the glTF animation definitions.
    const cgltf_data* srcAsset = asset->mSourceAsset;
    const cgltf_animation* srcAnims = srcAsset->animations;
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
            addChannels(instance->nodeMap, srcAnim, dstAnim);
        } else if (asset->mInstances.empty()) {
            addChannels(asset->mNodeMap, srcAnim, dstAnim);
        } else {
            for (FFilamentInstance* instance : asset->mInstances) {
                addChannels(instance->nodeMap, srcAnim, dstAnim);
            }
        }
    }
}

Animator::~Animator() {
    delete mImpl;
}

size_t Animator::getAnimationCount() const {
    return mImpl->animations.size();
}

void Animator::applyAnimation(size_t animationIndex, float time) const {
    const Animation& anim = mImpl->animations[animationIndex];
    TransformManager* transformManager = mImpl->transformManager;
    RenderableManager* renderableManager = mImpl->renderableManager;
    time = fmod(time, anim.duration);
    for (const auto& channel : anim.channels) {
        const Sampler* sampler = channel.sourceData;
        if (sampler->times.size() < 2) {
            continue;
        }

        TransformManager::Instance node = transformManager->getInstance(channel.targetEntity);
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

        // Perform the interpolation. This is a simple but inefficient implementation; Filament
        // stores transforms as mat4's but glTF animation is based on TRS (translation rotation
        // scale).
        mat4f xform = transformManager->getTransform(node);
        float3 scale;
        quatf rotation;
        float3 translation;
        decomposeMatrix(xform, &translation, &rotation, &scale);

        if (sampler->interpolation == Sampler::STEP) {
            t = 0.0f;
        }

        switch (channel.transformType) {

            case Channel::SCALE: {
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
                break;
            }

            case Channel::TRANSLATION: {
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
                break;
            }

            case Channel::ROTATION: {
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
                break;
            }

            case Channel::WEIGHTS: {
                float4 weights(0, 0, 0, 0);
                const float* const samplerValues = sampler->values.data();
                assert(sampler->values.size() % times.size() == 0);
                const int valuesPerKeyframe = sampler->values.size() / times.size();

                if (sampler->interpolation == Sampler::CUBIC) {
                    assert(valuesPerKeyframe % 3 == 0);
                    const int numMorphTargets = valuesPerKeyframe / 3;
                    const float* const inTangents = samplerValues;
                    const float* const splineVerts = samplerValues + numMorphTargets;
                    const float* const outTangents = samplerValues + numMorphTargets * 2;

                    const int numComponents = std::min((int) MAX_MORPH_TARGETS, numMorphTargets);
                    for (int comp = 0; comp < numComponents; ++comp) {
                        float vert0 = splineVerts[comp + prevIndex * valuesPerKeyframe];
                        float tang0 = outTangents[comp + prevIndex * valuesPerKeyframe];
                        float tang1 = inTangents[comp + nextIndex * valuesPerKeyframe];
                        float vert1 = splineVerts[comp + nextIndex * valuesPerKeyframe];
                        weights[comp] = cubicSpline(vert0, tang0, vert1, tang1, t);
                    }
                } else {
                    const int numComponents = std::min((int) MAX_MORPH_TARGETS, valuesPerKeyframe);
                    for (int comp = 0; comp < numComponents; ++comp) {
                        float previous = samplerValues[comp + prevIndex * valuesPerKeyframe];
                        float current = samplerValues[comp + nextIndex * valuesPerKeyframe];
                        weights[comp] = (1 - t) * previous + t * current;
                    }
                }

                auto renderable = renderableManager->getInstance(channel.targetEntity);
                renderableManager->setMorphWeights(renderable, weights);
                continue;
            }
        }

        xform = composeMatrix(translation, rotation, scale);
        transformManager->setTransform(node, xform);
    }
}

void Animator::updateBoneMatrices() {
    auto renderableManager = mImpl->renderableManager;
    auto transformManager = mImpl->transformManager;

    auto update = [=](const SkinVector& skins, BoneVector& boneVector) {
        for (const auto& skin : skins) {
            size_t njoints = skin.joints.size();
            boneVector.resize(njoints);
            for (const auto& entity : skin.targets) {
                auto renderable = renderableManager->getInstance(entity);
                if (!renderable) {
                    continue;
                }
                mat4f inverseGlobalTransform;
                auto xformable = transformManager->getInstance(entity);
                if (xformable) {
                    inverseGlobalTransform = inverse(transformManager->getWorldTransform(xformable));
                }
                for (size_t boneIndex = 0; boneIndex < njoints; ++boneIndex) {
                    const auto& joint = skin.joints[boneIndex];
                    TransformManager::Instance jointInstance = transformManager->getInstance(joint);
                    mat4f globalJointTransform = transformManager->getWorldTransform(jointInstance);
                    boneVector[boneIndex] =
                            inverseGlobalTransform *
                            globalJointTransform *
                            skin.inverseBindMatrices[boneIndex];
                }
                renderableManager->setBones(renderable, boneVector.data(), boneVector.size());
            }
        }
    };

    if (mImpl->instance) {
        update(mImpl->instance->skins, mImpl->boneMatrices);
    } else if (mImpl->asset->mInstances.empty()) {
        update(mImpl->asset->mSkins, mImpl->boneMatrices);
    } else {
        for (FFilamentInstance* instance : mImpl->asset->mInstances) {
            update(instance->skins, mImpl->boneMatrices);
        }
    }
}

float Animator::getAnimationDuration(size_t animationIndex) const {
    return mImpl->animations[animationIndex].duration;
}

const char* Animator::getAnimationName(size_t animationIndex) const {
    return mImpl->animations[animationIndex].name.c_str();
}

} // namespace gltfio
