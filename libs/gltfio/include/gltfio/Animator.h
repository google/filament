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
#include <gltfio/FilamentInstance.h>

namespace gltfio {

struct FFilamentAsset;
struct FFilamentInstance;
struct AnimatorImpl;

/**
 * \class Animator Animator.h gltfio/Animator.h
 * \brief Updates matrices according to glTF \c animation and \c skin definitions.
 *
 * Animator can be used for two things:
 * - Updating matrices in filament::TransformManager components according to glTF \c animation definitions.
 * - Updating bone matrices in filament::RenderableManager components according to glTF \c skin definitions.
 *
 * For a usage example, see the documentation for AssetLoader.
 */
class Animator {
public:
    /**
     * Applies rotation, translation, and scale to entities that have been targeted by the given
     * animation definition. Uses filament::TransformManager.
     *
     * @param animationIndex Zero-based index for the \c animation of interest.
     * @param time Elapsed time of interest in seconds.
     */
    void applyAnimation(size_t animationIndex, float time) const;

    /**
     * Computes root-to-node transforms for all bone nodes, then passes
     * the results into filament::RenderableManager::setBones.
     * Uses filament::TransformManager and filament::RenderableManager.
     *
     * NOTE: this operation is independent of \c animation.
     */
    void updateBoneMatrices();

    /** Returns the number of \c animation definitions in the glTF asset. */
    size_t getAnimationCount() const;

    /** Returns the duration of the specified glTF \c animation in seconds. */
    float getAnimationDuration(size_t animationIndex) const;

    /**
     * Returns a weak reference to the string name of the specified \c animation, or an
     * empty string if none was specified.
     */
    const char* getAnimationName(size_t animationIndex) const;

private:

    /*! \cond PRIVATE */
    friend struct FFilamentAsset;
    friend struct FFilamentInstance;
    /*! \endcond */

    Animator(FFilamentAsset* asset, FFilamentInstance* instance);
    ~Animator();
    AnimatorImpl* mImpl;
};

} // namespace gltfio

#endif // GLTFIO_ANIMATOR_H
