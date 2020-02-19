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

#include "MeshWriter.h"

#include <fstream>
#include <iostream>

#include <math/half.h>
#include <math/mat3.h>
#include <math/norm.h>
#include <math/quat.h>
#include <math/vec3.h>

#include <utils/algorithm.h>
#include <utils/Path.h>

#include <filameshio/filamesh.h>

#include <getopt/getopt.h>

using namespace filamesh;
using namespace filament::math;
using namespace utils;

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>

using Assimp::Importer;

// configuration
bool g_interleaved = false;
bool g_snormUVs = false;
bool g_compression = false;

Mesh g_mesh;
float2 g_minUV = float2(std::numeric_limits<float>::max());
float2 g_maxUV = float2(std::numeric_limits<float>::lowest());

template<bool SNORMUVS>
static ushort2 convertUV(float2 uv) {
    if (SNORMUVS) {
        short2 uvshort(packSnorm16(uv));
        return bit_cast<ushort2>(uvshort);
    } else {
        half2 uvhalf(uv);
        return bit_cast<ushort2>(uvhalf);
    }
}

template<typename VECTOR, typename INDEX>
static Box computeAABB(VECTOR const* positions, INDEX const* indices,
        size_t count, size_t stride) noexcept {
    filament::math::float3 bmin(std::numeric_limits<float>::max());
    filament::math::float3 bmax(std::numeric_limits<float>::lowest());
    for (size_t i = 0; i < count; ++i) {
        VECTOR const* p = reinterpret_cast<VECTOR const *>(
                (char const*) positions + indices[i] * stride);
        const filament::math::float3 v(p->x, p->y, p->z);
        bmin = min(bmin, v);
        bmax = max(bmax, v);
    }
    return Box().set(bmin, bmax);
}

void preprocessNode(const aiScene* scene, const aiNode* node) {
    for (size_t i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        if (!mesh->HasNormals()) {
            std::cerr << "Error: mesh " << i <<  " does not have normals" << std::endl;
            continue;
        }
        if (!mesh->HasTextureCoords(0)) {
            std::cerr << "Warning: mesh " << i <<  " does not have texture coordinates"
                    << std::endl;
            continue;
        }
        const float3* uv0 = reinterpret_cast<const float3*>(mesh->mTextureCoords[0]);
        const float3* uv1 = reinterpret_cast<const float3*>(mesh->mTextureCoords[1]);
        if (!mesh->HasTextureCoords(1)) {
            uv1 = nullptr;
        }
        const size_t numVertices = mesh->mNumVertices;
        const size_t numFaces = mesh->mNumFaces;
        if (numVertices == 0 || numFaces == 0) {
            continue;
        }
        for (size_t j = 0; j < numVertices; j++) {
            g_minUV = min(uv0[j].xy, g_minUV);
            g_maxUV = max(uv0[j].xy, g_maxUV);
            if (uv1) {
                g_minUV = min(uv1[j].xy, g_minUV);
                g_maxUV = max(uv1[j].xy, g_maxUV);
            }
        }
    }
    for (size_t i = 0; i < node->mNumChildren; ++i) {
        preprocessNode(scene, node->mChildren[i]);
    }
}

template<bool INTERLEAVED, bool SNORMUVS>
void processNode(const aiScene* scene, const aiNode* node, std::vector<Part>& meshes) {
    for (size_t i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        if (!mesh->HasNormals()) {
            continue;
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
        if (!mesh->HasTextureCoords(0)) {
            uv0 = nullptr;
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
                uint32_t indicesOffset = g_mesh.vertexCount;
                g_mesh.vertexCount += numVertices;
                if (INTERLEAVED) {
                    g_mesh.vertices.reserve(g_mesh.vertexCount);
                } else {
                    g_mesh.positions.reserve(g_mesh.vertexCount);
                    g_mesh.tangents.reserve(g_mesh.vertexCount);
                    g_mesh.uv0.reserve(g_mesh.vertexCount);
                }

                for (size_t j = 0; j < numVertices; j++) {
                    quatf q;
                    if (uv0) {
                        q = mat3f::packTangentFrame({tangents[j], bitangents[j], normals[j]});
                    } else {
                        q = quatf(0, 0, 0, 1);
                    }
                    color = colors ? colors[j] : float4(1.0f);
                    Vertex vertex {
                        .position = half4(vertices[j], 1.0_h),
                        .tangents = short4(filament::math::packSnorm16(q.xyzw)),
                        .color = ubyte4(clamp(color, 0.0f, 1.0f) * 255.0f),
                        .uv0 = uv0 ? convertUV<SNORMUVS>(uv0[j].xy) : ushort2(0),
                    };
                    if (INTERLEAVED) {
                        g_mesh.vertices.emplace_back(vertex);
                    } else {
                        g_mesh.positions.emplace_back(vertex.position);
                        g_mesh.tangents.emplace_back(vertex.tangents);
                        g_mesh.colors.emplace_back(vertex.color);
                        g_mesh.uv0.emplace_back(vertex.uv0);
                        if (uv1 != nullptr) {
                            g_mesh.uv1.emplace_back(convertUV<SNORMUVS>(uv1[j].xy));
                        }
                    }
                }

                // all faces should be triangles since we configure assimp to triangulate faces
                uint32_t indicesCount = numFaces * faces[0].mNumIndices;
                uint32_t indexBufferOffset = g_mesh.indices.size();
                g_mesh.indices.reserve(g_mesh.indices.size() + indicesCount);

                for (size_t j = 0; j < numFaces; ++j) {
                    const aiFace& face = faces[j];
                    for (size_t k = 0; k < face.mNumIndices; ++k) {
                        g_mesh.indices.push_back(uint32_t(face.mIndices[k] + indicesOffset));
                    }
                }

                size_t stride = INTERLEAVED ? sizeof(Vertex) : sizeof(Vertex::position);
                const decltype(Vertex::position)* positions =
                        INTERLEAVED ? &g_mesh.vertices.data()->position : g_mesh.positions.data();
                const Box aabb(computeAABB(positions,
                        g_mesh.indices.data() + indexBufferOffset, indicesCount, stride));

                meshes.emplace_back(Part {
                    .offset = indexBufferOffset,
                    .indexCount = indicesCount,
                    .minIndex = indicesOffset,
                    .maxIndex = (indicesOffset + indicesCount - 1),
                    .material = mesh->mMaterialIndex,
                    .aabb = aabb
                });
            }
        }
    }

    for (size_t i = 0 ; i < node->mNumChildren ; ++i) {
        processNode<INTERLEAVED, SNORMUVS>(scene, node->mChildren[i], meshes);
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
                    "    FBX, OBJ\n"
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
                    "   --compress, -c\n"
                    "       enable compression\n\n"
    );

    const std::string from("FILAMESH");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), execName);
    }
    printf("%s", usage.c_str());
}

static void license() {
    static const char *license[] = {
        #include "licenses/licenses.inc"
        nullptr
    };

    const char **p = &license[0];
    while (*p)
        std::cout << *p++ << std::endl;
}

static int handleArguments(int argc, char* argv[]) {
    static constexpr const char* OPTSTR = "hilc";
    static const struct option OPTIONS[] = {
            { "help",        no_argument, 0, 'h' },
            { "license",     no_argument, 0, 'l' },
            { "interleaved", no_argument, 0, 'i' },
            { "compress",    no_argument, 0, 'c' },
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
            case 'c':
                g_compression = true;
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

    const aiNode* node = scene->mRootNode;

    // Check for acceptable assimp data and determine UV bounds.
    preprocessNode(scene, node);
    g_snormUVs = g_minUV.x >= -1.0f && g_minUV.x <= 1.0f && g_maxUV.x >= -1.0f && g_maxUV.x <= 1.0f &&
                 g_minUV.y >= -1.0f && g_minUV.y <= 1.0f && g_maxUV.y >= -1.0f && g_maxUV.y <= 1.0f;

    // Consume assimp data and produce filamesh data.
    if (g_interleaved) {
        if (g_snormUVs) {
            processNode<true, true>(scene, node, g_mesh.parts);
        } else {
            processNode<true, false>(scene, node, g_mesh.parts);
        }
    } else {
        if (g_snormUVs) {
            processNode<false, true>(scene, node, g_mesh.parts);
        } else {
            processNode<false, false>(scene, node, g_mesh.parts);
        }
    }

    uint32_t materialCount = scene->mNumMaterials;

    for (uint32_t i = 0; i < materialCount; i++) {
        const aiMaterial* material = scene->mMaterials[i];

        aiString name;
        if (material->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) {
            std::cerr << "Unnamed material replaced with 'default'" << std::endl;
            g_mesh.materials.emplace_back("default");
        } else {
            g_mesh.materials.emplace_back(name.C_Str());
        }
    }

    Path dst(argv[optionIndex + 1]);

    const Path outputDir(dst.getParent());
    if (!outputDir.exists()) {
        outputDir.mkdirRecursive();
    }

    std::ofstream out(dst, std::ios::binary | std::ios::trunc);
    if (!out.good()) {
        std::cerr << "Could not write to " << dst << std::endl;
        out.close();
        return 1;
    }

    uint32_t flags = 0;
    if (g_interleaved) {
        flags |= filamesh::INTERLEAVED;
    }
    if (g_snormUVs) {
        flags |= filamesh::TEXCOORD_SNORM16;
    }
    if (g_compression) {
        flags |= filamesh::COMPRESSION;
    }
    MeshWriter(flags).serialize(out, g_mesh);

    out.flush();
    out.close();

    return 0;
}
