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

#ifndef GLTFIO_ANIMATIONHELPER_H
#define GLTFIO_ANIMATIONHELPER_H

#include <gltfio/FilamentAsset.h>

namespace gltfio {

struct AnimationImpl;

/**
 * AnimationHelper uses TransformManager to apply prescribed rotation, translation, and scale to the
 * entities that have been targeted by the animation definitions for a particular FilamentAsset.
 * For a usage example, see the comment block for AssetLoader.
 * TODO: this supports skinning but not morphing.
 */
class AnimationHelper {
public:
    AnimationHelper(FilamentAsset* asset);
    ~AnimationHelper();
    size_t getAnimationCount() const;
    void applyAnimation(size_t animationIndex, float time) const;
    float getAnimationDuration(size_t animationIndex) const;
    const char* getAnimationName(size_t animationIndex) const;
private:
    AnimationImpl* mImpl;
};

} // namespace gltfio

#endif // GLTFIO_ANIMATIONHELPER_H
