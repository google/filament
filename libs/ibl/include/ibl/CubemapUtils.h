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

#include <utils/JobSystem.h>

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
            ScanlineProc<STATE> proc,
            ReduceProc<STATE> reduce = [](STATE&) {},
            const STATE& prototype = STATE());

    //! Converts equirectangular Image to a Cubemap
    static void equirectangularToCubemap(Cubemap& dst, const Image& src);

    //! Converts a Cubemap to an equirectangular Image
    static void cubemapToEquirectangular(Image& dst, const Cubemap& src);

    //! Converts a Cubemap to an octahedron
    static void cubemapToOctahedron(Image& dst, const Cubemap& src);

    //! Converts horizontal or vertical cross Image to a Cubemap
    static void crossToCubemap(Cubemap& dst, const Image& src);

    //! clamps image to acceptable range for RGBM encoding.
    static void clamp(Image& src);

    //! Downsamples a cubemap by helf in x and y using a box filter
    static void downsampleCubemapLevelBoxFilter(Cubemap& dst, const Cubemap& src);

    //! Return the name of a face (suitable for a file name)
    static const char* getFaceName(Cubemap::Face face);

    //! Sets a Cubemap faces from a cross image
    static void setAllFacesFromCross(Cubemap& cm, const Image& image);

    //! mirror the cubemap in the horizontal direction
    static void mirrorCubemap(Cubemap& dst, const Cubemap& src);

    //! computes the solid angle of a pixel of a face of a cubemap
    static double solidAngle(size_t dim, size_t u, size_t v);

    //! generates a UV grid in the cubemap -- useful for debugging.
    static void generateUVGrid(Cubemap& cml, size_t gridFrequencyX, size_t gridFrequencyY);

private:
    static void setFaceFromCross(Cubemap& cm, Cubemap::Face face, const Image& image);
    static Image createCubemapImage(size_t dim, bool horizontal = true);

    friend class CubemapIBL;
    static utils::JobSystem& getJobSystem();
};

// -----------------------------------------------------------------------------------------------

template<typename STATE>
void CubemapUtils::process(
        Cubemap& cm,
        CubemapUtils::ScanlineProc<STATE> proc,
        ReduceProc<STATE> reduce,
        const STATE& prototype) {
    using namespace utils;

    JobSystem& js = getJobSystem();

    const size_t dim = cm.getDimensions();

    // multithread only on large-ish cubemaps
    STATE states[6];
    for (STATE& s : states) {
        s = prototype;
    }

    JobSystem::Job* parent = js.createJob();
    for (size_t faceIndex = 0; faceIndex < 6; faceIndex++) {
        const Cubemap::Face f = (Cubemap::Face)faceIndex;
        JobSystem::Job* face = jobs::createJob(js, parent,
                [faceIndex, &states, f, &cm, &dim, &proc]
                        (utils::JobSystem& js, utils::JobSystem::Job* parent) {
                    STATE& s = states[faceIndex];
                    Image& image(cm.getImageForFace(f));

                    auto parallelJobTask = [&image, &proc, &s, dim, f](size_t y0, size_t c) {
                        for (size_t y = y0; y < y0 + c; y++) {
                            Cubemap::Texel* data =
                                    static_cast<Cubemap::Texel*>(image.getPixelRef(0, y));
                            proc(s, y, f, data, dim);
                        }
                    };

                    if (std::is_same<STATE, CubemapUtils::EmptyState>::value) {
                        auto job = jobs::parallel_for(js, parent, 0, uint32_t(dim),
                                std::ref(parallelJobTask), jobs::CountSplitter<1, 8>());

                        // we need to wait here because parallelJobTask is passed by reference
                        js.runAndWait(job);
                    } else {
                        // if we have a per-thread STATE, we can't parallel_for()
                        parallelJobTask(0, dim);
                    }
                }, std::ref(js), parent);
        js.run(face);
    }
    // wait for all our threads to finish
    js.runAndWait(parent);

    for (STATE& s : states) {
        reduce(s);
    }
}

} // namespace ibl
} // namespace filament

#endif /* IBL_CUBEMAP_UTILS_H */
