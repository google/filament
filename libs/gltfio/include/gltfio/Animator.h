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

#ifndef GLTFIO_ANIMATOR_H
#define GLTFIO_ANIMATOR_H

#include <gltfio/FilamentAsset.h>

namespace gltfio {

namespace details { class FFilamentAsset; }

struct AnimatorImpl;

/**
 * Animator can be used for two things: (1) updating matrices in Transform components
 * according to glTF animation definitions and (2) updating bone matrices in Renderable components
 * according to glTF skin definitions.
 *
 * For a usage example, see the comment block for AssetLoader.
 *
 * TODO: add support for morphing.
 */
class Animator {
public:
    /**
     * Uses TransformManager to apply rotation, translation, and scale to entities that have
     * been targeted by the given animation definition.
     */
    void applyAnimation(size_t animationIndex, float time) const;

    /**
     * Uses TransformManager to compute root-to-node transforms for all bone nodes, then passes
     * the results into RenderableManager::setBones.
     *
     * Note that this operation is actually independent of animation, but the Animator seems
     * like a reasonable place for a utility like this.
     */
    void updateBoneMatrices();

    size_t getAnimationCount() const;
    float getAnimationDuration(size_t animationIndex) const;
    const char* getAnimationName(size_t animationIndex) const;

private:
    friend class details::FFilamentAsset;
    Animator(FilamentAsset* asset);
    ~Animator();
    AnimatorImpl* mImpl;
};

} // namespace gltfio

#endif // GLTFIO_ANIMATOR_H
