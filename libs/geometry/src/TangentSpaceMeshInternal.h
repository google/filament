/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef TNT_GEOMETRY_TANGENTSPACEMESHIMPL_H
#define TNT_GEOMETRY_TANGENTSPACEMESHIMPL_H

#include <geometry/TangentSpaceMesh.h>

#include <math/mat3.h>
#include <math/norm.h>
#include <math/quat.h>

#include <utils/Panic.h>

#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace filament::geometry {

using namespace filament::math;

namespace {

using Algorithm = TangentSpaceMesh::Algorithm;
using AuxAttribute = TangentSpaceMesh::AuxAttribute;
using InData = TangentSpaceMesh::InData;

} // namespace

template<typename InputType>
inline const InputType* pointerAdd(InputType const* ptr, size_t index, size_t stride) noexcept {
    return (InputType*) (((uint8_t const*) ptr) + (index * stride));
}

template<typename InputType>
inline InputType* pointerAdd(InputType* ptr, size_t index, size_t stride) noexcept {
    return (InputType*) (((uint8_t*) ptr) + (index * stride));
}

// Defines the actual implementation used to compute the TBN, where as TangentSpaceMesh::Algorithm
// is a hint that the client can provide.
enum class AlgorithmImpl : uint8_t {
    INVALID = 0,

    MIKKTSPACE = 1,
    LENGYEL = 2,
    HUGHES_MOLLER = 3,
    FRISVAD = 4,

    // Generating flat shading will remesh the input
    FLAT_SHADING = 5,
    TANGENTS_PROVIDED = 6,
};

enum class AttributeImpl : uint8_t {
    UV1 = 0x0,
    COLORS = 0x1,
    JOINTS = 0x2,
    WEIGHTS = 0x3,
    NORMALS = 0x4,
    UV0 = 0x5,
    TANGENTS = 0x6,
    POSITIONS = 0x7,
    TANGENT_SPACE = 0x8,
};

#define DATA_TYPE_UV1 float2
#define DATA_TYPE_COLORS float4
#define DATA_TYPE_JOINTS ushort4
#define DATA_TYPE_WEIGHTS float4
#define DATA_TYPE_NORMALS float3
#define DATA_TYPE_UV0 float2
#define DATA_TYPE_TANGENTS float4
#define DATA_TYPE_POSITIONS float3
#define DATA_TYPE_TANGENT_SPACE quatf

#define AUX_ATTRIBUTE_MATCH(attrib) static_assert((uint8_t) AuxAttribute::attrib == (uint8_t) AttributeImpl::attrib)

// These enums are exposed in the public API, we need to make sure they match the internal enums.
AUX_ATTRIBUTE_MATCH(UV1);
AUX_ATTRIBUTE_MATCH(COLORS);
AUX_ATTRIBUTE_MATCH(JOINTS);
AUX_ATTRIBUTE_MATCH(WEIGHTS);

#undef AUX_ATTRIBUTE_MATCH

namespace {

// Maps an attribute to an array and stride.
struct AttributeDataStride {
    InData data;
    size_t stride = 0;
};

template<typename T>
class InternalArray {
public:
    void borrow(T const* ptr, size_t stride=sizeof(T)) {
        assert_invariant(mAllocated.empty());
        mBorrowed = ptr;
        mBorrowedStride = stride;
    }

    T* allocate(size_t size) {
        assert_invariant(!mBorrowed);
        mAllocated.resize(size);
        mAllocated.shrink_to_fit();
        return mAllocated.data();
    }

    explicit operator bool() const noexcept {
        return mBorrowed || !mAllocated.empty();
    }

    T const& operator [](int i) const {
        if (mBorrowed) {
            return *pointerAdd(mBorrowed, i, mBorrowedStride);
        }
        return mAllocated[i];
    }

    T& operator [](int i) {
        if (mBorrowed) {
            return *pointerAdd(mBorrowed, i, mBorrowedStride);
        }
        return mAllocated[i];
    }

private:
    std::vector<T> mAllocated;
    T const* mBorrowed = nullptr;
    size_t mBorrowedStride = sizeof(T);
};

using ArrayType = std::variant<InternalArray<float2>, InternalArray<float3>, InternalArray<float4>,
        InternalArray<ushort3>, InternalArray<ushort4>, InternalArray<quatf>>;

ArrayType toArray(AttributeImpl attribute) {
    switch (attribute) {
        case AttributeImpl::UV1:
            return InternalArray<DATA_TYPE_UV1>{};
        case AttributeImpl::COLORS:
            return InternalArray<DATA_TYPE_COLORS>{};
        case AttributeImpl::JOINTS:
            return InternalArray<DATA_TYPE_JOINTS>{};
        case AttributeImpl::WEIGHTS:
            return InternalArray<DATA_TYPE_WEIGHTS>{};
        case AttributeImpl::NORMALS:
            return InternalArray<DATA_TYPE_NORMALS>{};
        case AttributeImpl::UV0:
            return InternalArray<DATA_TYPE_UV0>{};
        case AttributeImpl::TANGENTS:
            return InternalArray<DATA_TYPE_TANGENTS>{};
        case AttributeImpl::POSITIONS:
            return InternalArray<DATA_TYPE_POSITIONS>{};
        case AttributeImpl::TANGENT_SPACE:
            return InternalArray<DATA_TYPE_TANGENT_SPACE>{};
    }
}

} // namespace

struct TangentSpaceMeshInput {
    using AttributeMap = std::unordered_map<AttributeImpl, AttributeDataStride>;

    size_t vertexCount = 0;
    ushort3 const* triangles16 = nullptr;
    uint3 const* triangles32 = nullptr;

    size_t triangleCount = 0;

    inline float3 const* positions() const {
        return data<DATA_TYPE_POSITIONS>(AttributeImpl::POSITIONS);
    }

    inline size_t positionsStride() const {
        return strideSafe<DATA_TYPE_POSITIONS>(AttributeImpl::POSITIONS);
    }

    inline float2 const* uvs() const {
        return data<DATA_TYPE_UV0>(AttributeImpl::UV0);
    }

    inline size_t uvsStride() const {
        return strideSafe<DATA_TYPE_UV0>(AttributeImpl::UV0);
    }

    inline float4 const* tangents() const {
        return data<DATA_TYPE_TANGENTS>(AttributeImpl::TANGENTS);
    }

    inline size_t tangentsStride() const {
        return strideSafe<DATA_TYPE_TANGENTS>(AttributeImpl::TANGENTS);
    }

    inline float3 const* normals() const {
        return data<DATA_TYPE_NORMALS>(AttributeImpl::NORMALS);
    }

    inline size_t normalsStride() const {
        return strideSafe<DATA_TYPE_NORMALS>(AttributeImpl::NORMALS);
    }

    static inline size_t attributeSize(AttributeImpl attribute) {
        switch (attribute) {
            case AttributeImpl::UV1: return sizeof(DATA_TYPE_UV1);
            case AttributeImpl::COLORS: return sizeof(DATA_TYPE_COLORS);
            case AttributeImpl::JOINTS: return sizeof(DATA_TYPE_JOINTS);
            case AttributeImpl::WEIGHTS: return sizeof(DATA_TYPE_WEIGHTS);
            case AttributeImpl::NORMALS: return sizeof(DATA_TYPE_NORMALS);
            case AttributeImpl::UV0: return sizeof(DATA_TYPE_UV0);
            case AttributeImpl::TANGENTS: return sizeof(DATA_TYPE_TANGENTS);
            case AttributeImpl::POSITIONS: return sizeof(DATA_TYPE_POSITIONS);
            case AttributeImpl::TANGENT_SPACE:
                PANIC_POSTCONDITION("Invalid attribute found in input");
        }
    }

    static inline bool isDataTypeCorrect(AttributeImpl attribute, InData indata) {
        switch (attribute) {
            case AttributeImpl::UV1:
                return std::holds_alternative<DATA_TYPE_UV1 const*>(indata);
            case AttributeImpl::COLORS:
                return std::holds_alternative<DATA_TYPE_COLORS const*>(indata);
            case AttributeImpl::JOINTS:
                return std::holds_alternative<DATA_TYPE_JOINTS const*>(indata);
            case AttributeImpl::WEIGHTS:
                return std::holds_alternative<DATA_TYPE_WEIGHTS const*>(indata);
            case AttributeImpl::NORMALS:
                return std::holds_alternative<DATA_TYPE_NORMALS const*>(indata);
            case AttributeImpl::UV0:
                return std::holds_alternative<DATA_TYPE_UV0 const*>(indata);
            case AttributeImpl::TANGENTS:
                return std::holds_alternative<DATA_TYPE_TANGENTS const*>(indata);
            case AttributeImpl::POSITIONS:
                return std::holds_alternative<DATA_TYPE_POSITIONS const*>(indata);
            case AttributeImpl::TANGENT_SPACE:
                PANIC_POSTCONDITION("Invalid attribute found in input");
        }
    }

    inline size_t stride(AttributeImpl attribute) const {
        auto res = attributeData.find(attribute);
        assert_invariant(res != attributeData.end());
        return res->second.stride ? res->second.stride : attributeSize(attribute);
    }

    template<typename DataType>
    inline size_t strideSafe(AttributeImpl attribute) const {
        auto res = attributeData.find(attribute);
        if (res == attributeData.end()) {
            return sizeof(DataType);

        }
        return res->second.stride ? res->second.stride : sizeof(DataType);
    }

    uint8_t const* raw(AttributeImpl attribute) const {
        switch (attribute) {
            case AttributeImpl::UV1:
                return (uint8_t const*) data<DATA_TYPE_UV1>(attribute);
            case AttributeImpl::COLORS:
                return (uint8_t const*) data<DATA_TYPE_COLORS>(attribute);
            case AttributeImpl::JOINTS:
                return (uint8_t const*) data<DATA_TYPE_JOINTS>(attribute);
            case AttributeImpl::WEIGHTS:
                return (uint8_t const*) data<DATA_TYPE_WEIGHTS>(attribute);
            case AttributeImpl::NORMALS:
                return (uint8_t const*) data<DATA_TYPE_NORMALS>(attribute);
            case AttributeImpl::UV0:
                return (uint8_t const*) data<DATA_TYPE_UV0>(attribute);
            case AttributeImpl::TANGENTS:
                return (uint8_t const*) data<DATA_TYPE_TANGENTS>(attribute);
            case AttributeImpl::POSITIONS:
                return (uint8_t const*) data<DATA_TYPE_POSITIONS>(attribute);
            case AttributeImpl::TANGENT_SPACE:
                PANIC_POSTCONDITION("Invalid attribute found in input");
        }
    }

    // Pass back the std::variant instead of the content.
    InData data(AttributeImpl attribute) const {
        auto res = attributeData.find(attribute);
        assert_invariant(res != attributeData.end());
        return res->second.data;
    }

    template<typename DataType>
    inline DataType const* data(AttributeImpl attribute) const {
        auto res = attributeData.find(attribute);
        if (res == attributeData.end()) {
            return nullptr;
        }
        return std::get<DataType const*>(res->second.data);
    }

    inline std::vector<AttributeImpl> getAuxAttributes() const {
        std::vector<AttributeImpl> ret;
        for (auto [attrib, data]: attributeData) {
            // TANGENT_SPACE is only used for output
            assert_invariant(attrib != AttributeImpl::TANGENT_SPACE);

            if (attrib == AttributeImpl::POSITIONS || attrib == AttributeImpl::TANGENTS ||
                    attrib == AttributeImpl::UV0 || attrib == AttributeImpl::NORMALS) {
                continue;
            }
            ret.push_back(attrib);
        }
        return ret;
    }

    AttributeMap attributeData;

    Algorithm algorithm;
};

struct TangentSpaceMeshOutput {
    TangentSpaceMeshOutput() {
        for (AttributeImpl attrib:
                {AttributeImpl::TANGENT_SPACE, AttributeImpl::UV0, AttributeImpl::POSITIONS}) {
            attributeData.emplace(std::make_pair(attrib, toArray(attrib)));
        }
    }

    InternalArray<DATA_TYPE_TANGENT_SPACE>& tspace() {
        return data<quatf>(AttributeImpl::TANGENT_SPACE);
    }

    InternalArray<DATA_TYPE_UV0>& uvs() {
        return data<float2>(AttributeImpl::UV0);
    }

    InternalArray<DATA_TYPE_POSITIONS>& positions() {
        return data<float3>(AttributeImpl::POSITIONS);
    }

    template<typename DataType>
    InternalArray<DataType>& data(AttributeImpl attrib) {
        if (attributeData.find(attrib) == attributeData.end()) {
            attributeData.emplace(std::make_pair(attrib, toArray(attrib)));
        }
        return std::get<InternalArray<DataType>>(attributeData[attrib]);
    }

    void passthrough(TangentSpaceMeshInput::AttributeMap const& inAttributeMap,
            std::vector<AttributeImpl> const& attributes) {
        auto const borrow = [&inAttributeMap, this](AttributeImpl attrib) {
            auto ref = inAttributeMap.find(attrib);
            if (ref == inAttributeMap.end()) {
                return;
            }
            InData const& indata = ref->second.data;
            size_t const stride = ref->second.stride;
            switch (attrib) {
                case AttributeImpl::UV1:
                    data<DATA_TYPE_UV1>(attrib).borrow(std::get<DATA_TYPE_UV1 const*>(indata),
                            stride);
                    break;
                case AttributeImpl::COLORS:
                    data<DATA_TYPE_COLORS>(attrib).borrow(std::get<DATA_TYPE_COLORS const*>(indata),
                            stride);
                    break;
                case AttributeImpl::JOINTS:
                    data<DATA_TYPE_JOINTS>(attrib).borrow(std::get<DATA_TYPE_JOINTS const*>(indata),
                            stride);
                    break;
                case AttributeImpl::WEIGHTS:
                    data<DATA_TYPE_WEIGHTS>(attrib).borrow(
                            std::get<DATA_TYPE_WEIGHTS const*>(indata), stride);
                    break;
                case AttributeImpl::NORMALS:
                    data<DATA_TYPE_NORMALS>(attrib).borrow(
                            std::get<DATA_TYPE_NORMALS const*>(indata), stride);
                    break;
                case AttributeImpl::UV0:
                    data<DATA_TYPE_UV0>(attrib).borrow(std::get<DATA_TYPE_UV0 const*>(indata),
                            stride);
                    break;
                case AttributeImpl::TANGENTS:
                    data<DATA_TYPE_TANGENTS>(attrib).borrow(
                            std::get<DATA_TYPE_TANGENTS const*>(indata), stride);
                    break;
                case AttributeImpl::POSITIONS:
                    data<DATA_TYPE_POSITIONS>(attrib).borrow(
                            std::get<DATA_TYPE_POSITIONS const*>(indata), stride);
                    break;
                case AttributeImpl::TANGENT_SPACE:
                    PANIC_POSTCONDITION("Invalid attribute found in input");
                    break;
            }
        };

        for (AttributeImpl attrib: attributes) {
            borrow(attrib);
        }
    }

    AlgorithmImpl algorithm;

    size_t triangleCount = 0;
    size_t vertexCount = 0;

    InternalArray<uint3> triangles32;
    InternalArray<ushort3> triangles16;

    std::unordered_map<AttributeImpl, ArrayType> attributeData;
};

}// namespace filament::geometry

#endif//TNT_GEOMETRY_TANGENTSPACEMESHIMPL_H
