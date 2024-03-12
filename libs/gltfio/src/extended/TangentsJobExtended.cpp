/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "TangentsJobExtended.h"

#include "AssetLoaderExtended.h"
#include "../GltfEnums.h"
#include "../FFilamentAsset.h"
#include "../Utility.h"

#include <geometry/TangentSpaceMesh.h>
#include <utils/Log.h>
#include <utils/StructureOfArrays.h>

#include <memory>
#include <unordered_map>

using namespace filament;
using namespace filament::gltfio;
using namespace filament::math;

namespace {

constexpr uint8_t POSITIONS_ID = 0;
constexpr uint8_t TANGENTS_ID = 1;
constexpr uint8_t COLORS_ID = 3;
constexpr uint8_t NORMALS_ID = 4;
constexpr uint8_t UV0_ID = 5;
constexpr uint8_t UV1_ID = 6;
constexpr uint8_t WEIGHTS_ID = 7;
constexpr uint8_t JOINTS_ID = 8;
constexpr uint8_t INVALID_ID = 0xFF;

using POSITIONS_TYPE = float3*;
using TANGENTS_TYPE = float4*;
using COLORS_TYPE = float4*;
using NORMALS_TYPE = float3*;
using UV0_TYPE = float2*;
using UV1_TYPE = float2*;
using WEIGHTS_TYPE = float4*;
using JOINTS_TYPE = ushort4*;

// Used in template specifier.
#define POSITIONS_T POSITIONS_TYPE, POSITIONS_ID
#define TANGENTS_T TANGENTS_TYPE, TANGENTS_ID
#define COLORS_T COLORS_TYPE, COLORS_ID
#define NORMALS_T NORMALS_TYPE, NORMALS_ID
#define UV0_T UV0_TYPE, UV0_ID
#define UV1_T UV1_TYPE, UV1_ID
#define WEIGHTS_T WEIGHTS_TYPE, WEIGHTS_ID
#define JOINTS_T JOINTS_TYPE, JOINTS_ID

using DataType = std::variant<float2*, float3*, float4*, ubyte4*, ushort4*>;
using AttributeDataMap = std::unordered_map<uint8_t, DataType>;

inline uint8_t toCode(Attribute attr, UvMap const& uvmap, bool hasUv0) {
    if (attr.type == cgltf_attribute_type_normal) {
        assert_invariant(attr.index == 0);
        return NORMALS_ID;
    }
    if (attr.type == cgltf_attribute_type_tangent) {
        assert_invariant(attr.index == 0);
        return TANGENTS_ID;
    }
    if (attr.type == cgltf_attribute_type_color) {
        assert_invariant(attr.index == 0);
        return COLORS_ID;
        PANIC_POSTCONDITION("Unexpected number of components in color attribute");
    }
    if (attr.type == cgltf_attribute_type_position) {
        assert_invariant(attr.index == 0);
        return POSITIONS_ID;
    }

    // This logic is replicating slot assignment in AssetLoaderExtended.cpp
    if (attr.type == cgltf_attribute_type_texcoord) {
        assert_invariant(attr.index < UvMapSize);
        UvSet uvset = uvmap[attr.index];
        switch (uvset) {
            case gltfio::UV0:
                return UV0_ID;
            case gltfio::UV1:
                return UV1_ID;
            case gltfio::UNUSED:
                // If we have a free slot, then include this unused UV set in the VertexBuffer.
                // This allows clients to swap the glTF material with a custom material.
                if (!hasUv0 && getNumUvSets(uvmap) == 0) {
                    return UV0_ID;
                }
        }
        utils::slog.w <<"Only two sets of UVs are available" << utils::io::endl;
        return INVALID_ID;
    }
    if (attr.type == cgltf_attribute_type_weights) {
        assert_invariant(attr.index == 0);
        return WEIGHTS_ID;
    }
    if (attr.type == cgltf_attribute_type_joints) {
        assert_invariant(attr.index == 0);
        return JOINTS_ID;
    }
    // Otherwise, this is not an attribute supported by Filament.
    return INVALID_ID;
}

// Wrapper around TangentSpaceMesh to allow for passthrough if unlit.
struct Mesh {
    using AuxType = geometry::TangentSpaceMesh::AuxAttribute;

    struct Passthrough {
        static constexpr int POSITION = 256;
        static constexpr int UV0 = 257;
        static constexpr int NORMALS = 258;
        static constexpr int TANGENTS = 259;
        static constexpr int TRIANGLES = 260;

        std::unordered_map<int, void const*> data;
        size_t vertexCount = 0;
        size_t triangleCount = 0;
    };

    struct Builder {
        Builder(bool isUnlit)
            : isUnlit(isUnlit) {}

        Builder& vertexCount(size_t count) noexcept {
            if (isUnlit) {
                passthrough.vertexCount = count;
            } else {
                tob.vertexCount(count);
            }
            return *this;
        }

        Builder& normals(float3 const* normals) noexcept {
            if (isUnlit) {
                passthrough.data[Passthrough::NORMALS] = normals;
            } else {
                tob.normals(normals);
            }
            return *this;
        }

        Builder& tangents(float4 const* tangents) noexcept {
            if (isUnlit) {
                passthrough.data[Passthrough::TANGENTS] = tangents;
            } else {
                tob.tangents(tangents);
            }
            return *this;

        }

        Builder& uvs(float2 const* uvs) noexcept {
            if (isUnlit) {
                passthrough.data[Passthrough::UV0] = uvs;
            } else {
                tob.uvs(uvs);
            }
            return *this;
        }

        Builder& positions(float3 const* positions) noexcept{
            if (isUnlit) {
                passthrough.data[Passthrough::POSITION] = positions;
            } else {
                tob.positions(positions);
            }
            return *this;
        }

        Builder& triangleCount(size_t triangleCount) noexcept {
            if (isUnlit) {
                passthrough.triangleCount = triangleCount;
            } else {
                tob.triangleCount(triangleCount);
            }
            return *this;
        }

        Builder& triangles(uint3 const* triangles) noexcept {
            if (isUnlit) {
                passthrough.data[Passthrough::TRIANGLES] = triangles;
            } else {
                tob.triangles(triangles);
            }
            return *this;
        }

        template<typename T>
        Builder& aux(AuxType type, T data) {
            if (isUnlit) {
                passthrough.data[static_cast<int>(type)] = (void*) data;
            } else {
                tob.aux(type, data);
            }
            return *this;
        }

        Mesh* build() {
            if (isUnlit) {
                return new Mesh(passthrough);
            } else {
                return new Mesh(tob.build());
            }
        }

    private:
        bool const isUnlit;
        Passthrough passthrough;
        geometry::TangentSpaceMesh::Builder tob;
    };

private:
    Mesh(geometry::TangentSpaceMesh* tsMesh)
        : tsMesh(tsMesh),
          vertexCount(tsMesh->getVertexCount()),
          triangleCount(tsMesh->getTriangleCount()) {}

    Mesh(Passthrough& passthrough)
        : passthrough(passthrough),
          vertexCount(passthrough.vertexCount),
          triangleCount(passthrough.triangleCount) {}

public:
    static void destroy(Mesh* mesh) {
        if (mesh->tsMesh) {
            geometry::TangentSpaceMesh::destroy(mesh->tsMesh);
        }
        // For the unlit case, the arrays are owned by another map, which will be cleaned up separately.
    }

    size_t getVertexCount() const noexcept {
        return vertexCount;
    }

    float3* getPositions() noexcept {
        size_t const nbytes = vertexCount * sizeof(float3);
        auto data = (float3*) malloc(nbytes);
        if (tsMesh) {
            tsMesh->getPositions(data);
            return data;
        }
        std::memcpy(data, passthrough.data[Passthrough::POSITION], nbytes);
        return data;
    }

    float2* getUVs() noexcept {
        size_t const nbytes = vertexCount * sizeof(float2);
        auto data = (float2*) malloc(nbytes);
        if (tsMesh) {
            tsMesh->getUVs(data);
            return data;
        }
        std::memcpy(data, passthrough.data[Passthrough::UV0], nbytes);
        return data;
    }

    short4* getQuats() noexcept {
        size_t const nbytes = vertexCount * sizeof(short4);
        auto data = (short4*) malloc(nbytes);
        if (tsMesh) {
            tsMesh->getQuats(data);
            return data;
        }
        std::memcpy(data, passthrough.data[Passthrough::TANGENTS], nbytes);
        return data;
    }

    uint3* getTriangles() {
        size_t const nbytes = triangleCount * sizeof(uint3);
        auto data = (uint3*) malloc(nbytes);
        if (tsMesh) {
            tsMesh->getTriangles(data);
            return data;
        }
        std::memcpy(data, passthrough.data[Passthrough::TRIANGLES], nbytes);
        return data;
    }

    template<typename T>
    using is_supported_aux_t =
            typename std::enable_if<std::is_same<float2*, T>::value ||
                                    std::is_same<float3*, T>::value ||
                                    std::is_same<float4*, T>::value ||
                                    std::is_same<ushort3*, T>::value ||
                                    std::is_same<ushort4*, T>::value>::type;
    template<typename T, typename = is_supported_aux_t<T>>
    T getAux(AuxType attribute) noexcept {
        size_t const nbytes = vertexCount * sizeof(std::remove_pointer_t<T>);
        auto data = (T) malloc(nbytes);
        if (tsMesh) {
            tsMesh->getAux(attribute, data);
            return data;
        }
        std::memcpy(data, (T) passthrough.data[static_cast<int>(attribute)], nbytes);
        return data;
    }

    size_t getTriangleCount() const noexcept {
        if (tsMesh) {
            return tsMesh->getTriangleCount();
        }
        return passthrough.triangleCount;
    }

private:
    geometry::TangentSpaceMesh* tsMesh = nullptr;
    Passthrough passthrough;
    size_t const vertexCount;
    size_t const triangleCount;
};

namespace data {

template<typename T, uint8_t attrib>
inline T get(AttributeDataMap const& data) {
    auto iter = data.find(attrib);
    if (iter != data.end()) {
        return std::get<T>(iter->second);
    }
    return nullptr;
}

template<typename T, uint8_t attrib>
void allocate(AttributeDataMap& data, size_t count) {
    assert_invariant(data.find(attrib) == data.end());
    data[attrib]  = (T) malloc(sizeof(std::remove_pointer_t<T>) * count);
}

template<typename T, uint8_t attrib>
void free(AttributeDataMap& data) {
    if (data.find(attrib) == data.end()) {
        return;
    }
    std::free(std::get<T>(data[attrib]));
    data.erase(attrib);
}

constexpr uint8_t const UBYTE_TYPE = 1;
constexpr uint8_t const USHORT_TYPE = 2;
constexpr uint8_t const FLOAT_TYPE = 3;

template<typename T>
constexpr uint8_t componentType() {
    if constexpr (std::is_same_v<T, float2*> ||
            std::is_same_v<T, float3*> ||
            std::is_same_v<T, float4*>) {
        return FLOAT_TYPE;
    } else if constexpr (std::is_same_v<T, ubyte2*> ||
            std::is_same_v<T, ubyte3*> ||
            std::is_same_v<T, ubyte4*>) {
        return UBYTE_TYPE;
    } else if constexpr (std::is_same_v<T, ushort2*> ||
            std::is_same_v<T, ushort3*> ||
            std::is_same_v<T, ushort4*>) {
        return USHORT_TYPE;
    }
    return 0;
}

template<typename T>
constexpr size_t byteCount() {
    return sizeof(std::remove_pointer_t<T>);
}

template<typename T>
constexpr size_t componentCount() {
    if constexpr (std::is_same_v<T, float2*> || std::is_same_v<T, ubyte2*> ||
                  std::is_same_v<T, ushort2*>) {
        return 2;
    } else if constexpr (std::is_same_v<T, float3*> || std::is_same_v<T, ubyte3*> ||
                         std::is_same_v<T, ushort3*>) {
        return 3;
    } else if constexpr (std::is_same_v<T, float4*> || std::is_same_v<T, ubyte4*> ||
                         std::is_same_v<T, ushort4*>) {
        return 4;
    }
    return 0;
}

template<typename InDataType, typename OutDataType>
void copy(InDataType in, size_t inStride, OutDataType out, size_t outStride, size_t count) {
    uint8_t* inBytes = (uint8_t*) in;
    uint8_t* outBytes = (uint8_t*) out;

    if constexpr (componentType<InDataType>() == componentType<OutDataType>()) {
        if constexpr (componentCount<InDataType>() == 3 && componentCount<OutDataType>() == 4) {
            for (size_t i = 0; i < count; ++i) {
                *((OutDataType) (outBytes + (i * outStride))) = std::remove_pointer_t<OutDataType>(
                        *((InDataType) (inBytes + (i * inStride))), 1);
            }
            return;
        } else if constexpr (componentCount<InDataType>() == componentCount<OutDataType>()) {
            if (inStride == outStride) {
                std::memcpy(out, in, data::byteCount<InDataType>() * count);
            } else {
                for (size_t i = 0; i < count; ++i) {
                    *((OutDataType) (outBytes + (i * outStride))) =
                            std::remove_pointer_t<OutDataType>(
                                    *((InDataType) (inBytes + (i * inStride))));
                }
            }
            return;
        }

        PANIC_POSTCONDITION("Invalid component count in conversion");
    } else if constexpr (componentCount<InDataType>() == componentCount<OutDataType>()) {
        // byte to float conversion
        constexpr size_t const compCount = componentCount<InDataType>();
        if constexpr (componentType<InDataType>() == UBYTE_TYPE &&
                      componentType<OutDataType>() == FLOAT_TYPE) {
            for (size_t i = 0; i < count; ++i) {
                for (size_t j = 0; j < compCount; ++j) {
                    *(((float*) (outBytes + (i * outStride))) + j) =
                            *(((uint8_t*) (inBytes + (i * inStride))) + j) / 255.0f;
                }
            }
            return;
        } else if constexpr (componentType<InDataType>() == UBYTE_TYPE &&
                             componentType<OutDataType>() == USHORT_TYPE) {
            for (size_t i = 0; i < count; ++i) {
                for (size_t j = 0; j < compCount; ++j) {
                    *(((uint16_t*) (outBytes + (i * outStride))) + j) =
                            *(((uint8_t*) (inBytes + (i * inStride))) + j);
                }
            }
            return;
        }
    }
    PANIC_POSTCONDITION("Invalid conversion");
}

template<typename T>
void unpack(cgltf_accessor const* accessor, size_t const vertexCount, T out,
        bool isMorphTarget = false) {
    assert_invariant(accessor->count == vertexCount);
    uint8_t const* data = nullptr;
    if (accessor->buffer_view->has_meshopt_compression) {
        data = (uint8_t const*) accessor->buffer_view->data + accessor->offset;
    } else {
        data = (uint8_t const*) accessor->buffer_view->buffer->data +
               utility::computeBindingOffset(accessor);
    }
    auto componentType = accessor->component_type;
    size_t const inDim = cgltf_num_components(accessor->type);
    size_t const outDim = componentCount<T>();

    if (componentType == cgltf_component_type_r_32f) {
        assert_invariant(accessor->buffer_view);
        assert_invariant(data::componentType<T>() == FLOAT_TYPE);
        size_t const elementCount = outDim * vertexCount;

        if ((isMorphTarget && utility::requiresPacking(accessor)) ||
                utility::requiresConversion(accessor)) {
            cgltf_accessor_unpack_floats(accessor, (float*) out, elementCount);
            return;
        } else {
            if (inDim == 3 && outDim == 4) {
                data::copy<float3*, T>((float3*) data, accessor->stride, (T) out,
                        data::byteCount<T>(), vertexCount);
                return;
            } else {
                assert_invariant(inDim == outDim);
                data::copy<T, T>((T) data, accessor->stride, (T) out, data::byteCount<T>(),
                        vertexCount);
                return;
            }
        }
    } else {
        assert_invariant(outDim == inDim);
        if (componentType == cgltf_component_type_r_8u) {
            if (inDim == 2) {
                data::copy<ubyte2*, T>((ubyte2*) data, accessor->stride, (T) out,
                        data::byteCount<T>(), vertexCount);
                return;
            } else if (inDim == 3) {
                data::copy<ubyte3*, T>((ubyte3*) data, accessor->stride, (T) out,
                        data::byteCount<T>(), vertexCount);
                return;
            } else if (inDim == 4) {
                data::copy<ubyte4*, T>((ubyte4*) data, accessor->stride, (T) out,
                        data::byteCount<T>(), vertexCount);
                return;
            }
        } else if (componentType == cgltf_component_type_r_16u) {
            if (inDim == 2) {
                data::copy<ushort2*, T>((ushort2*) data, accessor->stride, (T) out,
                        data::byteCount<T>(), vertexCount);
                return;
            } else if (inDim == 3) {
                data::copy<ushort3*, T>((ushort3*) data, accessor->stride, (T) out,
                        data::byteCount<T>(), vertexCount);
                return;
            } else if (inDim == 4) {
                data::copy<ushort4*, T>((ushort4*) data, accessor->stride, (T) out,
                        data::byteCount<T>(), vertexCount);
                return;
            }
        }

        PANIC_POSTCONDITION("Only ubyte or ushort accepted as input");
    }
}

template<typename T, uint8_t attrib>
void unpack(cgltf_accessor const* accessor, AttributeDataMap& data, size_t const vertexCount,
        bool isMorphTarget = false) {
    assert_invariant(accessor->count == vertexCount);
    assert_invariant(data.find(attrib) != data.end());

    unpack(accessor, vertexCount, data::get<T, attrib>(data), isMorphTarget);
}

template<typename T, uint8_t attrib>
void add(AttributeDataMap& data, size_t const vertexCount, float3* addition) {
    assert_invariant(data.find(attrib) != data.end());

    T datav = std::get<T>(data[attrib]);
    for (size_t i = 0; i < vertexCount; ++i) {
        if constexpr(std::is_same_v<T, float2*>) {
            datav[i] += addition[i].xy;
        } else if constexpr(std::is_same_v<T, float3*>) {
            datav[i] += addition[i];
        } else if constexpr(std::is_same_v<T, float4*>) {
            datav[i].xyz += addition[i];
        }
    }
}

} // namespace data

void destroy(AttributeDataMap& data) {
    data::free<POSITIONS_T>(data);
    data::free<TANGENTS_T>(data);
    data::free<COLORS_T>(data);
    data::free<NORMALS_T>(data);
    data::free<UV0_T>(data);
    data::free<UV1_T>(data);
    data::free<WEIGHTS_T>(data);
    data::free<JOINTS_T>(data);
}

} // anonymous namespace

namespace filament::gltfio {

void TangentsJobExtended::run(Params* params) {
    cgltf_primitive const& prim = *params->in.prim;
    int const morphTargetIndex = params->in.morphTargetIndex;
    bool const isMorphTarget = morphTargetIndex != kMorphTargetUnused;

    bool const isUnlit = prim.material ? prim.material->unlit : false;

    // Extract the vertex count from the first attribute. All attributes must have the same count.
    assert_invariant(prim.attributes_count > 0);
    auto const vertexCount = prim.attributes[0].data->count;
    assert_invariant(vertexCount > 0);

    std::unordered_map<uint8_t, cgltf_accessor const*> accessors;
    std::unordered_map<uint8_t, cgltf_accessor const*> morphAccessors;
    AttributeDataMap attributes;

    std::unique_ptr<uint3[]> unpackedTriangles;
    bool hasUV0 = false;
    for (cgltf_size aindex = 0; aindex < prim.attributes_count; aindex++) {
        cgltf_attribute const& attr = prim.attributes[aindex];
        cgltf_accessor* accessor = attr.data;
        assert_invariant(accessor);
        if (auto const attrCode = toCode({attr.type, attr.index}, params->in.uvmap, hasUV0);
                attrCode != INVALID_ID) {
            hasUV0 = hasUV0 || attrCode == UV0_ID;
            accessors[attrCode] = accessor;
        }
    }

    std::vector<float3> morphDelta;
    if (isMorphTarget) {
        auto const& morphTarget = prim.targets[morphTargetIndex];
        decltype(params->in.uvmap) tmpUvmap; // just a placeholder since we don't consider morph target uvs.
        for (cgltf_size aindex = 0; aindex < morphTarget.attributes_count; aindex++) {
            cgltf_attribute const& attr = morphTarget.attributes[aindex];
            if (auto const attrCode = toCode({attr.type, attr.index}, tmpUvmap, false);
                    attrCode != INVALID_ID) {
                assert_invariant(accessors.find(attrCode) != accessors.end() &&
                                 "Morph target data has no corresponding base vertex data.");
                morphAccessors[attrCode] = attr.data;
            }
        }
        morphDelta.resize(vertexCount);
    }
    using AuxType = Mesh::AuxType;
    Mesh::Builder tob(isUnlit);
    tob.vertexCount(vertexCount);

    for (auto [attr, accessor]: accessors) {
        switch (attr) {
            case POSITIONS_ID:
                data::allocate<POSITIONS_T>(attributes, vertexCount);
                data::unpack<POSITIONS_T>(accessor, attributes, vertexCount);
                if (auto itr = morphAccessors.find(attr); itr != morphAccessors.end()) {
                    data::unpack<float3*>(itr->second, vertexCount, morphDelta.data(),
                            isMorphTarget);
                    data::add<POSITIONS_T>(attributes, vertexCount, morphDelta.data());

                    // We stash the positions as colors so that they can be retrieved without change
                    // after the TBN algo, which might have remeshed the input.
                    data::allocate<COLORS_T>(attributes, vertexCount);
                    float4* storage = data::get<COLORS_T>(attributes);
                    for (size_t i = 0; i < vertexCount; i++) {
                        storage[i] = float4{morphDelta[i], 0.0};
                    }
                    tob.aux(AuxType::COLORS, storage);
                }
                tob.positions(data::get<POSITIONS_T>(attributes));
                break;
            case TANGENTS_ID:
                data::allocate<TANGENTS_T>(attributes, vertexCount);
                data::unpack<TANGENTS_T>(accessor, attributes, vertexCount);
                if (auto itr = morphAccessors.find(attr); itr != morphAccessors.end()) {
                    data::unpack<float3*>(itr->second, vertexCount, morphDelta.data());
                    data::add<TANGENTS_T>(attributes, vertexCount, morphDelta.data());
                }
                tob.tangents(data::get<TANGENTS_T>(attributes));
                break;
            case NORMALS_ID:
                data::allocate<NORMALS_T>(attributes, vertexCount);
                data::unpack<NORMALS_T>(accessor, attributes, vertexCount);
                if (auto itr = morphAccessors.find(attr); itr != morphAccessors.end()) {
                    data::unpack<float3*>(itr->second, vertexCount, morphDelta.data());
                    data::add<NORMALS_T>(attributes, vertexCount, morphDelta.data());
                }
                tob.normals(data::get<NORMALS_T>(attributes));
                break;
            case COLORS_ID:
                data::allocate<COLORS_T>(attributes, vertexCount);
                data::unpack<COLORS_T>(accessor, attributes, vertexCount);
                tob.aux(AuxType::COLORS, data::get<COLORS_T>(attributes));
                break;
            case UV0_ID:
                data::allocate<UV0_T>(attributes, vertexCount);
                data::unpack<UV0_T>(accessor, attributes, vertexCount);
                tob.uvs(data::get<UV0_T>(attributes));
                break;
            case UV1_ID:
                data::allocate<UV1_T>(attributes, vertexCount);
                data::unpack<UV1_T>(accessor, attributes, vertexCount);
                tob.aux(AuxType::UV1, data::get<UV1_T>(attributes));
                break;
            case WEIGHTS_ID:
                data::allocate<WEIGHTS_T>(attributes, vertexCount);
                data::unpack<WEIGHTS_T>(accessor, attributes, vertexCount);
                tob.aux(AuxType::WEIGHTS, data::get<WEIGHTS_T>(attributes));
                break;
            case JOINTS_ID:
                data::allocate<JOINTS_T>(attributes, vertexCount);
                data::unpack<JOINTS_T>(accessor, attributes, vertexCount);
                tob.aux(AuxType::JOINTS, data::get<JOINTS_T>(attributes));
                break;
            default:
                break;
        }
    }

    size_t const triangleCount = prim.indices ? (prim.indices->count / 3) : (vertexCount / 3);
    unpackedTriangles.reset(new uint3[triangleCount]);

    // TODO, this is slow. We might be able to skip the manual read if the indices are already in
    // the right format.
    if (prim.indices) {
        for (size_t tri = 0, j = 0; tri < triangleCount; ++tri) {
            auto& triangle = unpackedTriangles[tri];
            triangle.x = cgltf_accessor_read_index(prim.indices, j++);
            triangle.y = cgltf_accessor_read_index(prim.indices, j++);
            triangle.z = cgltf_accessor_read_index(prim.indices, j++);
        }
    } else {
        for (size_t tri = 0, j = 0; tri < triangleCount; ++tri) {
            auto& triangle = unpackedTriangles[tri];
            triangle.x = j++;
            triangle.y = j++;
            triangle.z = j++;
        }
    }

    tob.triangleCount(triangleCount);
    tob.triangles(unpackedTriangles.get());
    auto const mesh = tob.build();

    auto& out = params->out;
    out.vertexCount = mesh->getVertexCount();

    auto const cleanup = [&]() {
        destroy(attributes);
        Mesh::destroy(mesh);
    };

    out.triangleCount = mesh->getTriangleCount();
    out.triangles = mesh->getTriangles();

    if (!isUnlit) {
        out.tbn = mesh->getQuats();
    }

    if (isMorphTarget) {
        // For morph targets, we need to retrieve the positions, but note that the unadjusted
        // positions are stored as colors.
        // For morph targets, we use COLORS as a way to store the original positions.
        auto data = mesh->getAux<COLORS_TYPE>(AuxType::COLORS);
        out.positions = (float3*) malloc(sizeof(float3) * out.vertexCount);
        for (size_t i = 0; i < out.vertexCount; ++i) {
            out.positions[i] = data[i].xyz;
        }
        free(data);
    } else {
        for (auto [attr, data]: attributes) {
            switch (attr) {
                case POSITIONS_ID:
                    out.positions = mesh->getPositions();
                    break;
                case COLORS_ID:
                    out.colors = mesh->getAux<COLORS_TYPE>(AuxType::COLORS);
                    break;
                case UV0_ID:
                    out.uv0 = mesh->getUVs();
                    break;
                case UV1_ID:
                    out.uv1 = mesh->getAux<UV1_TYPE>(AuxType::UV1);
                    break;
                case WEIGHTS_ID:
                    out.weights = mesh->getAux<WEIGHTS_TYPE>(AuxType::WEIGHTS);
                    break;
                case JOINTS_ID:
                    out.joints = mesh->getAux<JOINTS_TYPE>(AuxType::JOINTS);
                    break;
                default:
                    break;
            }
        }
    }


    cleanup();
}

} // namespace filament::gltfio
