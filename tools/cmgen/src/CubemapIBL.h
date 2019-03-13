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

#ifndef CUBEMAPIBL_H_
#define CUBEMAPIBL_H_

#include <functional>
#include <vector>

#include <stdint.h>
#include <stddef.h>

class Cubemap;
class Image;

class CubemapIBL {
public:
    /*
     * Compute roughness LOD using importance sampling GGX
     */

    using Progress = std::function<void(size_t, float)>;

    static void roughnessFilter(Cubemap& dst,
            const std::vector<Cubemap>& levels, double linearRoughness,
            size_t maxNumSamples = 1024,
            Progress progress = {});

    static void diffuseIrradiance(Cubemap& dst,
            const std::vector<Cubemap>& levels,
            size_t maxNumSamples = 1024,
            Progress progress = {});

    static void DFG(Image& dst, bool multiscatter, bool cloth);

    static void brdf(Cubemap& dst, double linearRoughness);
};

#endif /* CUBEMAPIBL_H_ */
