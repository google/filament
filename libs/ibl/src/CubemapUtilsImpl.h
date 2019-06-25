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

#ifndef IBL_CUBEMAPUTILSIMPL_H
#define IBL_CUBEMAPUTILSIMPL_H

#include <ibl/CubemapUtils.h>

#include <utils/JobSystem.h>

namespace filament {
namespace ibl {

template<typename STATE>
void CubemapUtils::process(
        Cubemap& cm,
        utils::JobSystem& js,
        CubemapUtils::ScanlineProc<STATE> proc,
        ReduceProc<STATE> reduce,
        const STATE& prototype) {
    using namespace utils;

    const size_t dim = cm.getDimensions();

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
                                std::ref(parallelJobTask), jobs::CountSplitter<64, 8>());

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

template<typename STATE>
void CubemapUtils::processSingleThreaded(
        Cubemap& cm,
        utils::JobSystem& js,
        CubemapUtils::ScanlineProc<STATE> proc,
        ReduceProc<STATE> reduce,
        const STATE& prototype) {
    using namespace utils;

    const size_t dim = cm.getDimensions();

    STATE s;
    for (size_t faceIndex = 0; faceIndex < 6; faceIndex++) {
        const Cubemap::Face f = (Cubemap::Face)faceIndex;
        Image& image(cm.getImageForFace(f));
        for (size_t y = 0; y < dim; y++) {
            Cubemap::Texel* data = static_cast<Cubemap::Texel*>(image.getPixelRef(0, y));
            proc(s, y, f, data, dim);
        }
    }
    reduce(s);
}


} // namespace ibl
} // namespace filament

#endif // IBL_CUBEMAPUTILSIMPL_H
