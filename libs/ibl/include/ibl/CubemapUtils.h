/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef IBL_CUBEMAP_UTILS_H
#define IBL_CUBEMAP_UTILS_H

#include <ibl/Cubemap.h>
#include <ibl/Image.h>

#include <functional>

namespace utils {
class JobSystem;
} // namespace utils

namespace filament {
namespace ibl {

class CubemapIBL;

/**
 * Create and convert Cubemap formats
 */
class CubemapUtils {
public:
    //! Creates a cubemap object and its backing Image
    static Cubemap create(Image& image, size_t dim, bool horizontal = true);

    struct EmptyState {
    };

    template<typename STATE>
    using ScanlineProc = std::function<
            void(STATE& state, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t width)>;

    template<typename STATE>
    using ReduceProc = std::function<void(STATE& state)>;

    //! process the cubemap using multithreading
    template<typename STATE>
    static void process(Cubemap& cm,
            utils::JobSystem& js,
            ScanlineProc<STATE> proc,
            ReduceProc<STATE> reduce = [](STATE&) {},
            const STATE& prototype = STATE());

    //! process the cubemap
    template<typename STATE>
    static void processSingleThreaded(Cubemap& cm,
            utils::JobSystem& js,
            ScanlineProc<STATE> proc,
            ReduceProc<STATE> reduce = [](STATE&) {},
            const STATE& prototype = STATE());

    //! Converts equirectangular Image to a Cubemap
    static void equirectangularToCubemap(utils::JobSystem& js, Cubemap& dst, const Image& src);

    //! Converts a Cubemap to an equirectangular Image
    static void cubemapToEquirectangular(utils::JobSystem& js, Image& dst, const Cubemap& src);

    //! Converts a Cubemap to an octahedron
    static void cubemapToOctahedron(utils::JobSystem& js, Image& dst, const Cubemap& src);

    //! Converts horizontal or vertical cross Image to a Cubemap
    static void crossToCubemap(utils::JobSystem& js, Cubemap& dst, const Image& src);

    //! clamps image to acceptable range
    static void clamp(Image& src);

    static void highlight(Image& src);

    //! Downsamples a cubemap by helf in x and y using a box filter
    static void downsampleCubemapLevelBoxFilter(utils::JobSystem& js, Cubemap& dst, const Cubemap& src);

    //! Return the name of a face (suitable for a file name)
    static const char* getFaceName(Cubemap::Face face);

    //! Sets a Cubemap faces from a cross image
    static void setAllFacesFromCross(Cubemap& cm, const Image& image);

    //! mirror the cubemap in the horizontal direction
    static void mirrorCubemap(utils::JobSystem& js, Cubemap& dst, const Cubemap& src);

    //! computes the solid angle of a pixel of a face of a cubemap
    static float solidAngle(size_t dim, size_t u, size_t v);

    //! generates a UV grid in the cubemap -- useful for debugging.
    static void generateUVGrid(utils::JobSystem& js, Cubemap& cml, size_t gridFrequencyX, size_t gridFrequencyY);

private:
    static void setFaceFromCross(Cubemap& cm, Cubemap::Face face, const Image& image);
    static Image createCubemapImage(size_t dim, bool horizontal = true);

    friend class CubemapIBL;
};


} // namespace ibl
} // namespace filament

#endif /* IBL_CUBEMAP_UTILS_H */
