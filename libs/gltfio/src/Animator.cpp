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
#include "math.h"
#include "upcast.h"

#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

#include <utils/Log.h>

#include <math/mat4.h>
#include <math/norm.h>
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

using namespace details;

using TimeValues = std::map<float, size_t>;
using SourceValues = std::vector<float>;

struct Sampler {
    TimeValues times;
    SourceValues values;
    enum { LINEAR, STEP, CUBIC } interpolation;
};

struct Channel {
    const Sampler* sourceData;
    utils::Entity targetEntity;
    enum { TRANSLATION, ROTATION, SCALE } transformType;
};

struct Animation {
    float duration;
    std::string name;
    std::vector<Sampler> samplers;
    std::vector<Channel> channels;
};

struct AnimatorImpl {
    std::vector<Animation> animations;
    FFilamentAsset* asset;
    RenderableManager* renderableManager;
    TransformManager* transformManager;
};

static int numComponents(cgltf_type type) {
    switch (type) {
        case cgltf_type_vec3: return 3;
        case cgltf_type_vec4: return 4;
        default: return 1;
    }
}

static void convert8(const cgltf_accessor* src, const uint8_t* srcBlob, SourceValues& dst) {
    srcBlob += src->buffer_view->offset + src->offset;
    dst.resize(src->count * numComponents(src->type));
    auto srcValues = (const int8_t*) srcBlob;
    for (size_t i = 0, n = dst.size(); i < n; ++i) {
        dst[i] = unpackSnorm8(srcValues[i]);
    }
}

static void convert8U(const cgltf_accessor* src, const uint8_t* srcBlob, SourceValues& dst) {
    srcBlob += src->buffer_view->offset + src->offset;
    dst.resize(src->count * numComponents(src->type));
    for (size_t i = 0, n = dst.size(); i < n; ++i) {
        dst[i] = unpackUnorm8(srcBlob[i]);
    }
}

static void convert16(const cgltf_accessor* src, const uint8_t* srcBlob, SourceValues& dst) {
    srcBlob += src->buffer_view->offset + src->offset;
    dst.resize(src->count * numComponents(src->type));
    auto srcValues = (const int16_t*) srcBlob;
    for (size_t i = 0, n = dst.size(); i < n; ++i) {
        dst[i] = unpackSnorm16(srcValues[i]);
    }
}

static void convert16U(const cgltf_accessor* src, const uint8_t* srcBlob, SourceValues& dst) {
    srcBlob += src->buffer_view->offset + src->offset;
    dst.resize(src->count * numComponents(src->type));
    auto srcValues = (const uint16_t*) srcBlob;
    for (size_t i = 0, n = dst.size(); i < n; ++i) {
        dst[i] = unpackUnorm16(srcValues[i]);
    }
}

static void convert32F(const cgltf_accessor* src, const uint8_t* srcBlob, SourceValues& dst) {
    srcBlob += src->buffer_view->offset + src->offset;
    dst.resize(src->count * numComponents(src->type));
    memcpy(dst.data(), srcBlob, dst.size() * sizeof(float));
}

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
    const uint8_t* valuesBlob = (const uint8_t*) valuesAccessor->buffer_view->buffer->data;
    switch (valuesAccessor->component_type) {
        case cgltf_component_type_r_8:
            convert8(valuesAccessor, valuesBlob, dst.values);
            break;
        case cgltf_component_type_r_8u:
            convert8U(valuesAccessor, valuesBlob, dst.values);
            break;
        case cgltf_component_type_r_16:
            convert16(valuesAccessor, valuesBlob, dst.values);
            break;
        case cgltf_component_type_r_16u:
            convert16U(valuesAccessor, valuesBlob, dst.values);
            break;
        case cgltf_component_type_r_32f:
            convert32F(valuesAccessor, valuesBlob, dst.values);
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
        case cgltf_animation_path_type_invalid:
        case cgltf_animation_path_type_weights:
            slog.e << "Unsupported channel path." << io::endl;
            break;
    }
}

Animator::Animator(FilamentAsset* publicAsset) {
    mImpl = new AnimatorImpl();
    FFilamentAsset* asset = mImpl->asset = upcast(publicAsset);
    mImpl->renderableManager = &asset->mEngine->getRenderableManager();
    mImpl->transformManager = &asset->mEngine->getTransformManager();

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
        cgltf_animation_channel* srcChannels = srcAnim.channels;
        dstAnim.channels.resize(srcAnim.channels_count);
        auto& transformManager = *mImpl->transformManager;
        for (cgltf_size j = 0, nchans = srcAnim.channels_count; j < nchans; ++j) {
            const cgltf_animation_channel& srcChannel = srcChannels[j];
            utils::Entity targetEntity = asset->mNodeMap[srcChannel.target_node];
            Channel& dstChannel = dstAnim.channels[j];
            dstChannel.sourceData = &dstAnim.samplers[srcChannel.sampler - srcSamplers];
            dstChannel.targetEntity = targetEntity;
            setTransformType(srcChannel, dstChannel);
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
    time = fmod(time, anim.duration);
    for (const auto& channel : anim.channels) {
        const Sampler* sampler = channel.sourceData;
        if (sampler->times.size() < 2) {
            continue;
        }

        int cindex = &channel - &anim.channels.front();
        int sindex = sampler - &anim.samplers.front();

        TransformManager::Instance node = mImpl->transformManager->getInstance(channel.targetEntity);
        const TimeValues& times = sampler->times;

        // Find the first keyframe after the given time, or the keyframe that matches it exactly.
        TimeValues::const_iterator iter = times.lower_bound(time);

        // Find the two values that we will interpolate between.
        TimeValues::const_iterator prevIter;
        TimeValues::const_iterator nextIter;
        if (iter == times.end()) {
            prevIter = --times.end();
            nextIter = times.begin();
        } else if (iter == times.begin()) {
            prevIter = nextIter = iter;
        } else {
            nextIter = iter;
            prevIter = --iter;
        }

        // Compute the interpolant between 0 and 1.
        float prevTime = prevIter->first;
        float nextTime = nextIter->first;
        float interval = nextTime - prevTime;
        if (interval < 0) {
            interval += anim.duration;
        }
        float t = interval == 0 ? 0.0f : ((time - prevTime) / interval);

        // Perform the interpolation. This is a simple but inefficient implementation; Filament
        // stores transforms as mat4's but glTF animation is based on TRS (translation rotation
        // scale). TODO: honor channel.transformType and support STEP / CUBIC interpolation
        size_t prevIndex = prevIter->second;
        size_t nextIndex = nextIter->second;

        mat4f xform = mImpl->transformManager->getTransform(node);
        float3 scale;
        quatf rotation;
        float3 translation;
        decomposeMatrix(xform, &translation, &rotation, &scale);

        switch (channel.transformType) {
            case Channel::SCALE: {
                const float3* srcVec3 = (const float3*) sampler->values.data();
                scale = ((1 - t) * srcVec3[prevIndex]) + (t * srcVec3[nextIndex]);
                break;
            }
            case Channel::TRANSLATION: {
                const float3* srcVec3 = (const float3*) sampler->values.data();
                translation = ((1 - t) * srcVec3[prevIndex]) + (t * srcVec3[nextIndex]);
                break;
            }
            case Channel::ROTATION: {
                const quatf* srcQuat = (const quatf*) sampler->values.data();
                rotation = slerp(srcQuat[prevIndex], srcQuat[nextIndex], t);
                break;
            }
        }

        xform = composeMatrix(translation, rotation, scale);
        mImpl->transformManager->setTransform(node, xform);
    }
}

void Animator::updateBoneMatrices() {
    vector<mat4f> boneMatrices;
    FFilamentAsset* asset = mImpl->asset;
    auto renderableManager = mImpl->renderableManager;
    auto transformManager = mImpl->transformManager;
    for (const auto& skin : asset->mSkins) {
        size_t njoints = skin.joints.size();
        boneMatrices.resize(njoints);
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
                boneMatrices[boneIndex] =
                        inverseGlobalTransform *
                        globalJointTransform *
                        skin.inverseBindMatrices[boneIndex];
            }
            renderableManager->setBones(renderable, boneMatrices.data(), boneMatrices.size());
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
