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

#include <filamentapp/IcoSphere.h>

#include <array>

static constexpr float X = .525731112119133606f;
static constexpr float Z = .850650808352039932f;
static constexpr float N = 0.f;

const IcoSphere::VertexList IcoSphere::sVertices = {
        { -X, N,  Z }, { X,  N,  Z }, { -X, N,  -Z }, { X,  N,  -Z },
        { N,  Z,  X }, { N,  Z,  -X }, { N,  -Z, X }, { N,  -Z, -X },
        { Z,  X,  N }, { -Z, X,  N }, { Z,  -X, N }, { -Z, -X, N }
};

const IcoSphere::TriangleList IcoSphere::sTriangles = {
        {  1,   4, 0 }, {  4,  9,  0 }, { 4,   5, 9 }, { 8, 5,   4 }, {  1,  8,  4 },
        {  1,  10, 8 }, { 10,  3,  8 }, { 8,   3, 5 }, { 3, 2,   5 }, {  3,  7,  2 },
        {  3,  10, 7 }, { 10,  6,  7 }, { 6,  11, 7 }, { 6, 0,  11 }, {  6,  1,  0 },
        { 10,   1, 6 }, { 11,  0,  9 }, { 2,  11, 9 }, { 5, 2,   9 }, { 11,  2,  7 }
};

IcoSphere::IcoSphere(size_t subdivisions) {
    mMesh = make_icosphere(subdivisions);
}

IcoSphere::Index IcoSphere::vertex_for_edge(
        Lookup& lookup, VertexList& vertices, Index first, Index second) {
    Lookup::key_type key(first, second);
    if (key.first > key.second) {
        std::swap(key.first, key.second);
    }

    auto inserted = lookup.insert({ key, (Lookup::mapped_type)vertices.size() });
    if (inserted.second) {
        auto edge0 = vertices[first];
        auto edge1 = vertices[second];
        auto point = normalize(edge0 + edge1);
        vertices.push_back(point);
    }

    return inserted.first->second;
}

IcoSphere::TriangleList IcoSphere::subdivide(VertexList& vertices, TriangleList const& triangles) {
    Lookup lookup;
    TriangleList result;
    for (Triangle const& each : triangles) {
        std::array<Index, 3> mid;
        mid[0] = vertex_for_edge(lookup, vertices, each.vertex[0], each.vertex[1]);
        mid[1] = vertex_for_edge(lookup, vertices, each.vertex[1], each.vertex[2]);
        mid[2] = vertex_for_edge(lookup, vertices, each.vertex[2], each.vertex[0]);
        result.push_back({ each.vertex[0], mid[0], mid[2] });
        result.push_back({ each.vertex[1], mid[1], mid[0] });
        result.push_back({ each.vertex[2], mid[2], mid[1] });
        result.push_back({ mid[0], mid[1], mid[2] });
    }
    return result;
}

IcoSphere::IndexedMesh IcoSphere::make_icosphere(int subdivisions) {
    VertexList vertices = sVertices;
    TriangleList triangles = sTriangles;
    for (int i = 0; i < subdivisions; ++i) {
        triangles = subdivide(vertices, triangles);
    }
    return { vertices, triangles };
}
