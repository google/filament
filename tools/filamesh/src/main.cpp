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


#include <fstream>
#include <iostream>

#include <math/half.h>
#include <math/mat3.h>
#include <math/norm.h>
#include <math/quat.h>
#include <math/vec3.h>

#include <utils/Path.h>

#include <getopt/getopt.h>

#include "Box.h"

using namespace math;
using namespace utils;

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>

static const uint32_t VERSION = 1;

using Assimp::Importer;

struct Header {
    uint32_t version;
    uint32_t parts;
    Box      aabb;
    uint32_t interleaved;
    uint32_t offsetPosition;
    uint32_t stridePosition;
    uint32_t offsetTangents;
    uint32_t strideTangents;
    uint32_t offsetColor;
    uint32_t strideColor;
    uint32_t offsetUV0;
    uint32_t strideUV0;
    uint32_t offsetUV1;
    uint32_t strideUV1;
    uint32_t vertexCount;
    uint32_t vertexSize;
    uint32_t indexType;
    uint32_t indexCount;
    uint32_t indexSize;
};

struct Vertex {
    Vertex(const float3& position, const quatf& tangents, const float4& color, const float3& uv0):
            position(position, 1.0_h),
            tangents(packSnorm16(tangents.xyzw)),
            color(clamp(color, 0.0f, 1.0f) * 255.0f),
            uv0(uv0.xy) {
    }

    half4  position;
    short4 tangents;
    ubyte4 color;
    half2  uv0;
};

struct Mesh {
    Mesh(uint32_t offset, uint32_t count, uint32_t minIndex, uint32_t maxIndex,
            uint32_t material, const Box& aabb):
            offset(offset),
            count(count),
            minIndex(minIndex),
            maxIndex(maxIndex),
            material(material),
            aabb(aabb) {
    }

    uint32_t offset;
    uint32_t count;
    uint32_t minIndex;
    uint32_t maxIndex;
    uint32_t material;
    Box aabb;
};

// configuration
bool g_interleaved = false;

uint32_t g_vertexCount = 0;
std::vector<uint32_t> g_indices;
// interleaved
std::vector<Vertex> g_vertices;
// de-interleaved
std::vector<decltype(Vertex::position)>  g_positions;
std::vector<decltype(Vertex::tangents)>  g_tangents;
std::vector<decltype(Vertex::color)>     g_colors;
std::vector<decltype(Vertex::uv0)>       g_uv0;
std::vector<decltype(Vertex::uv0)>       g_uv1;

template<typename T>
void write(std::ofstream& out, const T& value) {
    out.write((const char*) &value, sizeof(T));
}

template<typename T>
void write(std::ofstream& out, const T* data, uint32_t count) {
    out.write((const char*) data, sizeof(T) * count);
}

template<typename VECTOR, typename INDEX>
static Box computeAABB(VECTOR const* positions, INDEX const* indices,
        size_t count, size_t stride) noexcept {
    math::float3 bmin(std::numeric_limits<float>::max());
    math::float3 bmax(std::numeric_limits<float>::lowest());
    for (size_t i = 0; i < count; ++i) {
        VECTOR const* p = reinterpret_cast<VECTOR const *>(
                (char const*) positions + indices[i] * stride);
        const math::float3 v(p->x, p->y, p->z);
        bmin = min(bmin, v);
        bmax = max(bmax, v);
    }
    return Box().set(bmin, bmax);
}

template<bool INTERLEAVED>
void processNode(const aiScene* scene, const aiNode* node, std::vector<Mesh>& meshes) {
    for (size_t i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        if (!mesh->HasNormals() || !mesh->HasTextureCoords(0)) {
            std::cerr << "The mesh must have texture coordinates" << std::endl;
            exit(1);
        }

        const float3* vertices = reinterpret_cast<const float3*>(mesh->mVertices);
        const float3* tangents = reinterpret_cast<const float3*>(mesh->mTangents);
        const float3* bitangents = reinterpret_cast<const float3*>(mesh->mBitangents);
        const float3* normals = reinterpret_cast<const float3*>(mesh->mNormals);
        const float4* colors = reinterpret_cast<const float4*>(mesh->mColors[0]);
        const float3* uv0 = reinterpret_cast<const float3*>(mesh->mTextureCoords[0]);
        const float3* uv1 = reinterpret_cast<const float3*>(mesh->mTextureCoords[1]);

        if (!mesh->HasVertexColors(0)) {
            colors = nullptr;
        }
        if (!mesh->HasTextureCoords(1)) {
            uv1 = nullptr;
        }

        float4 color = {};

        const size_t numVertices = mesh->mNumVertices;
        if (numVertices > 0) {
            const aiFace* faces = mesh->mFaces;
            const size_t numFaces = mesh->mNumFaces;

            if (numFaces > 0) {
                size_t indicesOffset = g_vertexCount;
                g_vertexCount += numVertices;
                if (INTERLEAVED) {
                    g_vertices.reserve(g_vertexCount);
                } else {
                    g_positions.reserve(g_vertexCount);
                    g_tangents.reserve(g_vertexCount);
                    g_uv0.reserve(g_vertexCount);
                }

                for (size_t j = 0; j < numVertices; j++) {
                    quatf q = mat3f::packTangentFrame({tangents[j], bitangents[j], normals[j]});

                    color = colors ? colors[j] : float4(1.0f);
                    if (INTERLEAVED) {
                        g_vertices.emplace_back(vertices[j], q, color, uv0[j]);
                    } else {
                        // use the same conversions as in the interleaved case
                        Vertex v(vertices[j], q, color, uv0[j]);
                        g_positions.emplace_back(v.position);
                        g_tangents.emplace_back(v.tangents);
                        g_colors.emplace_back(v.color);
                        g_uv0.emplace_back(v.uv0);
                        if (uv1 != nullptr) {
                            g_uv1.emplace_back(uv1[j].xy);
                        }
                    }
                }

                // all faces should be triangles since we configure assimp to triangulate faces
                size_t indicesCount = numFaces * faces[0].mNumIndices;
                size_t indexBufferOffset = g_indices.size();
                g_indices.reserve(g_indices.size() + indicesCount);

                for (size_t j = 0; j < numFaces; ++j) {
                    const aiFace& face = faces[j];
                    for (size_t k = 0; k < face.mNumIndices; ++k) {
                        g_indices.push_back(uint32_t(face.mIndices[k] + indicesOffset));
                    }
                }

                size_t stride = INTERLEAVED ? sizeof(Vertex) : sizeof(Vertex::position);
                const decltype(Vertex::position)* positions =
                        INTERLEAVED ? &g_vertices.data()->position : g_positions.data();
                const Box aabb(computeAABB(positions,
                        g_indices.data() + indexBufferOffset, indicesCount, stride));

                meshes.emplace_back(indexBufferOffset, indicesCount, indicesOffset,
                        indicesOffset + indicesCount - 1, mesh->mMaterialIndex, aabb);
            }
        }
    }

    for (size_t i=0 ; i<node->mNumChildren ; ++i) {
        processNode<INTERLEAVED>(scene, node->mChildren[i], meshes);
    }
}

static void printUsage(const char* name) {
    std::string execName(utils::Path(name).getName());
    std::string usage(
            "FILAMESH is a tool to convert meshes into an optimized binary format\n"
                    "Usage:\n"
                    "    FILAMESH [options] <source mesh> <destination file>\n"
                    "\n"
                    "Supported mesh formats:\n"
                    "    COLLADA, FBX, OBJ\n"
                    "\n"
                    "Input meshes must have texture coordinates.\n"
                    "\n"
                    "Options:\n"
                    "   --help, -h\n"
                    "       print this message\n\n"
                    "   --license\n"
                    "       Print copyright and license information\n\n"
                    "   --interleaved, -i\n"
                    "       interleaves mesh attributes\n\n"
    );

    const std::string from("FILAMESH");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), execName);
    }
    printf("%s", usage.c_str());
}

static void license() {
    std::cout <<
    #include "licenses/licenses.inc"
    ;
}

static int handleArguments(int argc, char* argv[]) {
    static constexpr const char* OPTSTR = "hil";
    static const struct option OPTIONS[] = {
            { "help",        no_argument, 0, 'h' },
            { "license",     no_argument, 0, 'l' },
            { "interleaved", no_argument, 0, 'i' },
            { 0, 0, 0, 0 }  // termination of the option list
    };

    int opt;
    int optionIndex = 0;

    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        // std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
                // break;
            case 'l':
                license();
                exit(0);
                // break;
            case 'i':
                g_interleaved = true;
                break;
        }
    }

    return optind;
}

int main(int argc, char* argv[]) {
    int optionIndex = handleArguments(argc, argv);

    int numArgs = argc - optionIndex;
    if (numArgs < 2) {
        printUsage(argv[0]);
        return 1;
    }

    Path src(argv[optionIndex]);
    if (!src.exists()) {
        std::cerr << "The source mesh " << src << " does not exist." << std::endl;
        return 1;
    }

    Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
            aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, true);
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_KEEP_HIERARCHY, true);

    const aiScene* scene = importer.ReadFile(src,
            // normals and tangents
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            // topology optimization
            aiProcess_FindInstances |
            aiProcess_OptimizeMeshes |
            aiProcess_JoinIdenticalVertices |
            // misc optimization
            aiProcess_ImproveCacheLocality |
            aiProcess_PreTransformVertices |
            aiProcess_SortByPType |
            // we only support triangles
            aiProcess_Triangulate);

    if (!scene) {
        std::cerr << "Unknown mesh format in " << src << std::endl;
        return 1;
    }

    std::vector<Mesh> meshes;

    const aiNode* node = scene->mRootNode;

    if (g_interleaved) {
        processNode<true>(scene, node, meshes);
    } else {
        processNode<false>(scene, node, meshes);
    }

    Path dst(argv[optionIndex + 1]);
    std::ofstream out(dst, std::ios::binary | std::ios::trunc);
    if (!out.good()) {
        std::cerr << "Could not write to " << dst << std::endl;
        out.close();
        return 1;
    }

    const bool hasIndex16 = g_vertexCount < std::numeric_limits<uint16_t>::max();
    const bool hasUV1 = !g_uv1.empty();

    Box aabb = meshes.at(0).aabb;
    for (size_t i = 1; i < meshes.size(); i++) {
        aabb.unionSelf(meshes.at(i).aabb);
    }

    write(out, "FILAMESH", 8 * sizeof(char));

    Header header;
    header.version = VERSION;
    header.parts = uint32_t(meshes.size());
    header.aabb = aabb;
    header.interleaved = uint32_t(g_interleaved ? 1 : 0);
    if (g_interleaved) {
        header.offsetPosition = offsetof(Vertex, position);
        header.offsetTangents = offsetof(Vertex, tangents);
        header.offsetColor    = offsetof(Vertex, color);
        header.offsetUV0      = offsetof(Vertex, uv0);
        header.offsetUV1      = std::numeric_limits<uint32_t>::max();
        header.stridePosition = sizeof(Vertex);
        header.strideTangents = sizeof(Vertex);
        header.strideColor    = sizeof(Vertex);
        header.strideUV0      = sizeof(Vertex);
        header.strideUV1      = std::numeric_limits<uint32_t>::max();
    } else {
        header.offsetPosition = 0;
        header.offsetTangents = g_vertexCount * sizeof(Vertex::position);
        header.offsetColor    = header.offsetTangents + g_vertexCount * sizeof(Vertex::tangents);
        header.offsetUV0      = header.offsetColor + g_vertexCount * sizeof(Vertex::color);
        header.offsetUV1      = std::numeric_limits<uint32_t>::max();
        header.stridePosition = 0;
        header.strideTangents = 0;
        header.strideColor    = 0;
        header.strideUV0      = 0;
        header.strideUV1      = std::numeric_limits<uint32_t>::max();

        if (hasUV1) {
            header.offsetUV1  = header.offsetUV0 + g_vertexCount * sizeof(Vertex::uv0);
            header.strideUV1  = 0;
        }
    }
    header.vertexCount = g_vertexCount;
    header.vertexSize = g_vertexCount * sizeof(Vertex);
    header.indexType = uint32_t(hasIndex16 ? 1 : 0);
    header.indexCount = g_indices.size();
    header.indexSize = g_indices.size() * (hasIndex16 ? sizeof(uint16_t) : sizeof(uint32_t));

    write(out, header);

    if (g_interleaved) {
        write(out, g_vertices.data(), uint32_t(g_vertices.size()));
    } else {
        write(out, g_positions.data(), uint32_t(g_positions.size()));
        write(out, g_tangents.data(),  uint32_t(g_tangents.size()));
        write(out, g_colors.data(), uint32_t(g_colors.size()));
        write(out, g_uv0.data(), uint32_t(g_uv0.size()));
        if (hasUV1) {
            write(out, g_uv1.data(), uint32_t(g_uv1.size()));
        }
    }

    if (!hasIndex16) {
        write(out, g_indices.data(), uint32_t(g_indices.size()));
    } else {
        std::vector<uint16_t> smallIndices;
        smallIndices.resize(g_indices.size());
        for (size_t i = 0; i < g_indices.size(); i++) {
            smallIndices[i] = static_cast<uint16_t>(g_indices[i]);
        }
        write(out, smallIndices.data(), uint32_t(smallIndices.size()));
    }

    write(out, meshes.data(), header.parts);

    uint32_t materialCount = scene->mNumMaterials;
    write(out, materialCount);

    for (uint32_t i = 0; i < materialCount; i++) {
        const aiMaterial* material = scene->mMaterials[i];

        aiString name;
        if (material->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) {
            std::cerr << "Unnamed material replaced with 'default'" << std::endl;
            write(out, uint32_t(7));
            write(out, "default\0", uint32_t(8));
        } else {
            write(out, uint32_t(name.length));
            write(out, name.C_Str(), uint32_t(name.length));
            write(out, char(0));
        }
    }

    out.flush();
    out.close();

    return 0;
}
