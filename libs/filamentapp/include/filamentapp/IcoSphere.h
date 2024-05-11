/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_SAMPLE_ICOSPHERE_H
#define TNT_FILAMENT_SAMPLE_ICOSPHERE_H

#include <math/vec3.h>

#include <map>
#include <vector>

class IcoSphere {
public:
    using Index = uint16_t;

    struct Triangle {
        Index vertex[3];
    };

    using TriangleList = std::vector<Triangle>;
    using VertexList = std::vector<filament::math::float3>;
    using IndexedMesh = std::pair<VertexList, TriangleList>;

    explicit IcoSphere(size_t subdivisions);

    IndexedMesh const& getMesh() const { return mMesh; }
    VertexList const& getVertices() const { return mMesh.first; }
    TriangleList const& getIndices() const { return mMesh.second; }

private:
    static const IcoSphere::VertexList sVertices;
    static const IcoSphere::TriangleList sTriangles;

    using Lookup = std::map<std::pair<Index, Index>, Index>;
    Index vertex_for_edge(Lookup& lookup, VertexList& vertices, Index first, Index second);
    TriangleList subdivide(VertexList& vertices, TriangleList const& triangles);
    IndexedMesh make_icosphere(int subdivisions);

    IndexedMesh mMesh;
};

#endif //TNT_FILAMENT_SAMPLE_ICOSPHERE_H
