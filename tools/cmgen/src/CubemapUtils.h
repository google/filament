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

#ifndef SRC_CUBEMAPUTILS_H_
#define SRC_CUBEMAPUTILS_H_

#include <utils/JobSystem.h>

#include "Cubemap.h"
#include "Image.h"

class CubemapUtils {
public:
    struct EmptyState { };

    static utils::JobSystem& getJobSystem() {
        static utils::JobSystem js;
        js.adopt();
        return js;
    }

    template<typename STATE>
    using ScanlineProc = std::function<
            void(STATE& state, size_t y, Cubemap::Face f, Cubemap::Texel* data, size_t width)>;

    template<typename STATE>
    using ReduceProc = std::function<void(STATE& state)>;

    template<typename STATE>
    static void process(
            const Cubemap& cm,
            ScanlineProc<STATE> proc,
            ReduceProc<STATE> reduce = [](STATE&){},
            const STATE& prototype = STATE());

    // Convert equirectangular Image to a Cubemap
    static void equirectangularToCubemap(Cubemap& dst, const Image& src);

    // clamp image to acceptable range
    static void clamp(Image& src);

    // Downsample a cubemap
    static void downsampleCubemapLevelBoxFilter(Cubemap& dst, const Cubemap& src);

    // Return the name of a face (suitable for a file name)
    static std::string getFaceName(Cubemap::Face face);

    // Create a cubemap object and its backing Image
    static Cubemap create(Image& image, size_t dim, bool horizontal = true);

    // Copy an image
    static void copyImage(Image& dst, const Image& src);

    // Sets a Cubemap faces from a cross image
    static void setAllFacesFromCross(Cubemap& cm, const Image& image);

    static void mirrorCubemap(Cubemap& dst, const Cubemap& src);

    static double solidAngle(size_t dim, size_t u, size_t v);

    static void generateUVGrid(Cubemap const& cml, size_t gridFrequency);

private:
    static void setFaceFromCross(Cubemap& cm, Cubemap::Face face, const Image& image);
    static Image createCubemapImage(size_t dim, bool horizontal = true);
};

// -----------------------------------------------------------------------------------------------

template<typename STATE>
void CubemapUtils::process( const Cubemap& cm,
        CubemapUtils::ScanlineProc<STATE> proc,
        ReduceProc<STATE> reduce,
        const STATE& prototype)
{
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
                [ faceIndex, &states, f, &cm, &dim, &proc ]
                        (utils::JobSystem& js, utils::JobSystem::Job* parent) {
                    STATE& s = states[faceIndex];
                    const Image& image(cm.getImageForFace(f));

                    auto parallelJobTask = [ &image, &proc, &s, dim, f ](size_t y0, size_t c) {
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
    js.reset();

    for (STATE& s : states) {
        reduce(s);
    }
}


#endif /* SRC_CUBEMAPUTILS_H_ */
