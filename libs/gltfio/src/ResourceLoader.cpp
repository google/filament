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

#include <gltfio/ResourceLoader.h>

#include "FFilamentAsset.h"
#include "upcast.h"

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/VertexBuffer.h>

#include <geometry/SurfaceOrientation.h>

#include <math/quat.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/Log.h>

#include <cgltf.h>

#include <stb_image.h>

#include <tsl/robin_map.h>

#include <string>

using namespace filament;
using namespace filament::math;
using namespace utils;

static const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

namespace gltfio {

struct ResourceLoader::Impl {
    tsl::robin_map<std::string, BufferDescriptor> mResourceCache;
};

namespace details {

// The AssetPool tracks references to raw source data (cgltf hierarchies) and frees them
// appropriately. It releases all source assets only after the pending upload count is zero and the
// client has destroyed the ResourceLoader object. If the ResourceLoader is destroyed while uploads
// are still pending, then the AssetPool will stay alive until all uploads are complete.
class AssetPool {
public:
    AssetPool()  {}
    ~AssetPool() {
        for (auto asset : mAssets) {
            asset->releaseSourceAsset();
        }
    }
    void addAsset(FFilamentAsset* asset) {
        mAssets.push_back(asset);
        asset->acquireSourceAsset();
    }
    void addPendingUpload() {
        ++mPendingUploads;
    }
    static void onLoadedResource(void* buffer, size_t size, void* user) {
        auto pool = (AssetPool*) user;
        if (--pool->mPendingUploads == 0 && pool->mLoaderDestroyed) {
            delete pool;
        }
    }
    void onLoaderDestroyed() {
        if (mPendingUploads == 0) {
            delete this;
        } else {
            mLoaderDestroyed = true;
        }
    }
private:
    std::vector<FFilamentAsset*> mAssets;
    bool mLoaderDestroyed = false;
    int mPendingUploads = 0;
};

} // namespace details

using namespace details;

ResourceLoader::ResourceLoader(const ResourceConfiguration& config) :
        mPool(new AssetPool), mConfig(config), pImpl(new Impl) {}

ResourceLoader::~ResourceLoader() {
    mPool->onLoaderDestroyed();
    delete pImpl;
}

static void importSkinningData(Skin& dstSkin, const cgltf_skin& srcSkin) {
    const cgltf_accessor* srcMatrices = srcSkin.inverse_bind_matrices;
    dstSkin.inverseBindMatrices.resize(srcSkin.joints_count);
    if (srcMatrices) {
        auto dstMatrices = (uint8_t*) dstSkin.inverseBindMatrices.data();
        uint8_t* bytes = (uint8_t*) srcMatrices->buffer_view->buffer->data;
        auto srcBuffer = (void*) (bytes + srcMatrices->offset + srcMatrices->buffer_view->offset);
        memcpy(dstMatrices, srcBuffer, srcSkin.joints_count * sizeof(mat4f));
    }
}

void ResourceLoader::addResourceData(std::string url, BufferDescriptor&& buffer) {
    pImpl->mResourceCache.emplace(url, std::move(buffer));
}

static void convertBytesToShorts(uint16_t* dst, const uint8_t* src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

static void generateTrivialIndices(uint32_t* dst, size_t numVertices) {
    for (size_t i = 0; i < numVertices; ++i) {
        dst[i] = i;
    }
}

bool ResourceLoader::loadResources(FilamentAsset* asset) {
    FFilamentAsset* fasset = upcast(asset);
    if (fasset->mResourcesLoaded) {
        return false;
    }
    fasset->mResourcesLoaded = true;
    mPool->addAsset(fasset);
    auto gltf = (cgltf_data*) fasset->mSourceAsset;
    cgltf_options options {};

    // For emscripten builds we have a custom implementation of cgltf_load_buffers which looks
    // inside a cache of externally-supplied data blobs, rather than loading from the filesystem.

    #if defined(__EMSCRIPTEN__)

    if (gltf->buffers_count && !gltf->buffers[0].data && !gltf->buffers[0].uri && gltf->bin) {
        if (gltf->bin_size < gltf->buffers[0].size) {
            slog.e << "Bad size." << io::endl;
            return false;
        }
        gltf->buffers[0].data = (void*) gltf->bin;
    }

    for (cgltf_size i = 0; i < gltf->buffers_count; ++i) {
        if (gltf->buffers[i].data) {
            continue;
        }
        const char* uri = gltf->buffers[i].uri;
        if (uri == nullptr) {
            continue;
        }
        if (strncmp(uri, "data:", 5) == 0) {
            const char* comma = strchr(uri, ',');
            if (comma && comma - uri >= 7 && strncmp(comma - 7, ";base64", 7) == 0) {
                cgltf_result res = cgltf_load_buffer_base64(&options, gltf->buffers[i].size, comma + 1, &gltf->buffers[i].data);
                if (res != cgltf_result_success) {
                    slog.e << "Unable to load " << uri << io::endl;
                    return false;
                }
            } else {
                slog.e << "Unable to load " << uri << io::endl;
                return false;
            }
        } else if (strstr(uri, "://") == nullptr) {
            auto iter = pImpl->mResourceCache.find(uri);
            if (iter == pImpl->mResourceCache.end()) {
                slog.e << "Unable to load " << uri << io::endl;
                return false;
            }
            gltf->buffers[i].data = iter->second.buffer;
        } else {
            slog.e << "Unable to load " << uri << io::endl;
            return false;
        }
    }

    #else

    // Read data from the file system and base64 URLs.
    cgltf_result result = cgltf_load_buffers(&options, gltf, mConfig.gltfPath.c_str());
    if (result != cgltf_result_success) {
        slog.e << "Unable to load resources." << io::endl;
        return false;
    }

    #endif

    #ifndef NDEBUG
    if (cgltf_validate(gltf) != cgltf_result_success) {
        slog.e << "Failed cgltf validation." << io::endl;
        return false;
    }
    #endif

    // To be robust against the glTF conformance suite, we optionally ensure that skinning weights
    // sum to 1.0 at every vertex. Note that if the same weights buffer is shared in multiple
    // places, this will needlessly repeat the work. In the future we would like to remove this
    // feature, and instead simply require correct models. See also:
    // https://github.com/KhronosGroup/glTF-Sample-Models/issues/215
    if (mConfig.normalizeSkinningWeights) {
        normalizeSkinningWeights(fasset);
    }

    if (mConfig.recomputeBoundingBoxes) {
        updateBoundingBoxes(fasset);
    }

    // Upload data to the GPU.
    const BufferBinding* bindings = asset->getBufferBindings();
    bool needsTangents = false;
    bool needsSparseData = false;
    for (size_t i = 0, n = asset->getBufferBindingCount(); i < n; ++i) {
        auto bb = bindings[i];
        if (bb.vertexBuffer && bb.generateTangents) {
            needsTangents = true;
        } else if (bb.vertexBuffer && bb.sparseAccessor) {
            needsSparseData = true;
        } else if (bb.vertexBuffer && !bb.generateDummyData) {
            const uint8_t* data8 = bb.offset + (const uint8_t*) *bb.data;
            mPool->addPendingUpload();
            VertexBuffer::BufferDescriptor bd(data8, bb.size, AssetPool::onLoadedResource, mPool);
            bb.vertexBuffer->setBufferAt(*mConfig.engine, bb.bufferIndex, std::move(bd));
        } else if (bb.vertexBuffer) {
            uint32_t* dummyData = (uint32_t*) malloc(bb.size);
            memset(dummyData, 0xff, bb.size);
            VertexBuffer::BufferDescriptor bd(dummyData, bb.size, FREE_CALLBACK);
            bb.vertexBuffer->setBufferAt(*mConfig.engine, bb.bufferIndex, std::move(bd));
        } else if (bb.generateTrivialIndices) {
            uint32_t* data32 = (uint32_t*) malloc(bb.size);
            generateTrivialIndices(data32, bb.size / sizeof(uint32_t));
            IndexBuffer::BufferDescriptor bd(data32, bb.size, FREE_CALLBACK);
            bb.indexBuffer->setBuffer(*mConfig.engine, std::move(bd));
        } else if (bb.convertBytesToShorts) {
            const uint8_t* data8 = bb.offset + (const uint8_t*) *bb.data;
            size_t size16 = bb.size * 2;
            uint16_t* data16 = (uint16_t*) malloc(size16);
            convertBytesToShorts(data16, data8, bb.size);
            IndexBuffer::BufferDescriptor bd(data16, size16, FREE_CALLBACK);
            bb.indexBuffer->setBuffer(*mConfig.engine, std::move(bd));
        } else if (bb.indexBuffer) {
            const uint8_t* data8 = bb.offset + (const uint8_t*) *bb.data;
            mPool->addPendingUpload();
            IndexBuffer::BufferDescriptor bd(data8, bb.size, AssetPool::onLoadedResource, mPool);
            bb.indexBuffer->setBuffer(*mConfig.engine, std::move(bd));
        }
    }

    // Copy over the inverse bind matrices to allow users to destroy the source asset.
    for (cgltf_size i = 0, len = gltf->skins_count; i < len; ++i) {
        importSkinningData(fasset->mSkins[i], gltf->skins[i]);
    }

    // Apply sparse data modifications to base arrays, then upload the result.
    if (needsSparseData) {
        applySparseData(fasset);
    }

    // Compute surface orientation quaternions if necessary. This is similar to sparse data in that
    // we need to generate the contents of a GPU buffer by processing one or more CPU buffer(s).
    if (needsTangents) {
        computeTangents(fasset);
    }

    // Finally, load image files and create Filament Textures.
    return createTextures(fasset);
}

bool ResourceLoader::createTextures(details::FFilamentAsset* asset) const {
    // Define a simple functor that creates a Filament Texture from a blob of texels.
    // TODO: this could be optimized, e.g. do not generate mips if never mipmap-sampled, and use a
    // more compact format when possible.
    auto createTexture = [this, asset](stbi_uc* texels, uint32_t w, uint32_t h, bool srgb) {
        Texture *tex = Texture::Builder()
                .width(w)
                .height(h)
                .levels(0xff)
                .format(srgb ? Texture::InternalFormat::SRGB8_A8 : Texture::InternalFormat::RGBA8)
                .build(*mConfig.engine);

        Texture::PixelBufferDescriptor pbd(texels,
                size_t(w * h * 4),
                Texture::Format::RGBA,
                Texture::Type::UBYTE,
                [] (void* buffer, size_t, void*) { free(buffer); });

        tex->setImage(*mConfig.engine, 0, std::move(pbd));
        tex->generateMipmaps(*mConfig.engine);
        asset->mTextures.push_back(tex);
        return tex;
    };

    // Decode textures and associate them with material instance parameters.
    // To prevent needless re-decoding, we create a couple maps of Filament Texture objects where
    // the map keys are data pointers or URL strings.
    stbi_uc* texels;
    int width, height, comp;
    Texture* tex;

    tsl::robin_map<const void*, Texture*> bufTextures;
    tsl::robin_map<std::string, Texture*> urlTextures;

    const TextureBinding* texbindings = asset->getTextureBindings();
    for (size_t i = 0, n = asset->getTextureBindingCount(); i < n; ++i) {
        auto tb = texbindings[i];

        // Check if the texture binding uses BufferView data (i.e. it does not have a URL).
        if (tb.data) {
            const uint8_t* data8 = tb.offset + (const uint8_t*) *tb.data;
            tex = bufTextures[data8];
            if (!tex) {
                texels = stbi_load_from_memory(data8, tb.totalSize, &width, &height, &comp, 4);
                if (texels == nullptr) {
                    slog.e << "Unable to decode texture." << io::endl;
                    return false;
                }
                bufTextures[data8] = tex = createTexture(texels, width, height, tb.srgb);
            }
            tb.materialInstance->setParameter(tb.materialParameter, tex, tb.sampler);
            continue;
        }

        // Check if we already created a Texture object for this URL.
        tex = urlTextures[tb.uri];
        if (tex) {
            tb.materialInstance->setParameter(tb.materialParameter, tex, tb.sampler);
            continue;
        }

        // Check the resource cache for this URL, otherwise load it from the file system.
        auto iter = pImpl->mResourceCache.find(tb.uri);
        if (iter != pImpl->mResourceCache.end()) {
            const uint8_t* data8 = (const uint8_t*) iter->second.buffer;
            texels = stbi_load_from_memory(data8, iter->second.size, &width, &height, &comp, 4);
        } else {
            #if defined(__EMSCRIPTEN__)
                slog.e << "Unable to load texture: " << tb.uri << io::endl;
                return false;
            #else
                utils::Path fullpath = this->mConfig.gltfPath.getParent() + tb.uri;
                texels = stbi_load(fullpath.c_str(), &width, &height, &comp, 4);
            #endif
        }

        if (texels == nullptr) {
            slog.e << "Unable to decode texture: " << tb.uri << io::endl;
            return false;
        }

        urlTextures[tb.uri] = tex = createTexture(texels, width, height, tb.srgb);
        tb.materialInstance->setParameter(tb.materialParameter, tex, tb.sampler);
    }
    return true;
}

void ResourceLoader::applySparseData(FFilamentAsset* asset) const {
    auto uploadSparseData = [&](const cgltf_accessor* accessor, VertexBuffer* vb, uint8_t slot) {
        cgltf_size numFloats = accessor->count * cgltf_num_components(accessor->type);
        cgltf_size numBytes = sizeof(float) * numFloats;
        float* generated = (float*) malloc(numBytes);
        cgltf_accessor_unpack_floats(accessor, generated, numFloats);
        VertexBuffer::BufferDescriptor bd(generated, numBytes, FREE_CALLBACK);
        vb->setBufferAt(*mConfig.engine, slot, std::move(bd));
    };

    // Collect all vertex attribute slots that need to be populated.
    const BufferBinding* bindings = asset->getBufferBindings();
    tsl::robin_map<VertexBuffer*, uint8_t> sparseBuffers;
    for (size_t i = 0, n = asset->getBufferBindingCount(); i < n; ++i) {
        auto bb = bindings[i];
        if (bb.vertexBuffer && bb.sparseAccessor) {
            sparseBuffers[bb.vertexBuffer] = bb.bufferIndex;
        }
    }

    // Go through all cgltf accessors and apply sparse data if requested.
    for (cgltf_size index = 0; index < asset->mSourceAsset->accessors_count; index++) {
        const cgltf_accessor* accessor = asset->mSourceAsset->accessors + index;
        auto iter = asset->mAccessorMap.find(accessor);
        if (iter != asset->mAccessorMap.end()) {
            for (VertexBuffer* vb : iter->second) {
                auto iter = sparseBuffers.find(vb);
                if (iter != sparseBuffers.end()) {
                    uploadSparseData(accessor, vb, iter->second);
                }
            }
        }
    }
}

void ResourceLoader::computeTangents(FFilamentAsset* asset) const {
    // Declare vectors of normals and tangents, which we'll extract & convert from the source.
    std::vector<float3> fp32Normals;
    std::vector<float4> fp32Tangents;
    std::vector<float3> fp32Positions;
    std::vector<float2> fp32TexCoords;
    std::vector<uint3> ui32Triangles;

    constexpr int kMorphTargetUnused = -1;

    auto computeQuats = [&](const cgltf_primitive& prim, VertexBuffer* vb, uint8_t slot,
            int morphTargetIndex) {

        cgltf_size vertexCount = 0;

        // Build a mapping from cgltf_attribute_type to cgltf_accessor*.
        const int NUM_ATTRIBUTES = 8;
        const cgltf_accessor* accessors[NUM_ATTRIBUTES] = {};

        // Collect accessors for normals, tangents, etc.
        if (morphTargetIndex == kMorphTargetUnused) {
            for (cgltf_size aindex = 0; aindex < prim.attributes_count; aindex++) {
                const cgltf_attribute& attr = prim.attributes[aindex];
                if (attr.index == 0) {
                    accessors[attr.type] = attr.data;
                    vertexCount = attr.data->count;
                }
            }
        } else {
            const cgltf_morph_target& morphTarget = prim.targets[morphTargetIndex];
            for (cgltf_size aindex = 0; aindex < morphTarget.attributes_count; aindex++) {
                const cgltf_attribute& attr = prim.attributes[aindex];
                if (attr.index == 0) {
                    accessors[attr.type] = attr.data;
                    vertexCount = attr.data->count;
                }
            }
        }

        // At a minimum we need normals to generate tangents.
        auto normalsInfo = accessors[cgltf_attribute_type_normal];
        if (normalsInfo == nullptr || vertexCount == 0) {
            return;
        }

        short4* quats = (short4*) malloc(sizeof(short4) * vertexCount);

        geometry::SurfaceOrientation::Builder sob;
        sob.vertexCount(vertexCount);

        // Convert normals into packed floats.
        assert(normalsInfo->count == vertexCount);
        assert(normalsInfo->type == cgltf_type_vec3);
        fp32Normals.resize(vertexCount);
        cgltf_accessor_unpack_floats(normalsInfo, &fp32Normals[0].x, vertexCount * 3);
        sob.normals(fp32Normals.data());

        // Convert tangents into packed floats.
        auto tangentsInfo = accessors[cgltf_attribute_type_tangent];
        if (tangentsInfo) {
            if (tangentsInfo->count != vertexCount || tangentsInfo->type != cgltf_type_vec4) {
                slog.e << "Bad tangent count or type." << io::endl;
                return;
            }
            fp32Tangents.resize(vertexCount);
            cgltf_accessor_unpack_floats(tangentsInfo, &fp32Tangents[0].x, vertexCount * 4);
            sob.tangents(fp32Tangents.data());
        }

        auto positionsInfo = accessors[cgltf_attribute_type_position];
        if (positionsInfo) {
            if (positionsInfo->count != vertexCount || positionsInfo->type != cgltf_type_vec3) {
                slog.e << "Bad position count or type." << io::endl;
                return;
            }
            fp32Positions.resize(vertexCount);
            cgltf_accessor_unpack_floats(positionsInfo, &fp32Positions[0].x, vertexCount * 3);
            sob.positions(fp32Positions.data());
        }

        if (prim.indices) {
            size_t triangleCount = prim.indices->count / 3;
            ui32Triangles.resize(triangleCount);
            cgltf_size j = 0;
            for (auto& triangle : ui32Triangles) {
                triangle.x = cgltf_accessor_read_index(prim.indices, j++);
                triangle.y = cgltf_accessor_read_index(prim.indices, j++);
                triangle.z = cgltf_accessor_read_index(prim.indices, j++);
            }
        } else {
            size_t triangleCount = vertexCount / 3;
            ui32Triangles.resize(triangleCount);
            cgltf_size j = 0;
            for (auto& triangle : ui32Triangles) {
                triangle.x = j++;
                triangle.y = j++;
                triangle.z = j++;
            }
        }

        sob.triangleCount(ui32Triangles.size());
        sob.triangles(ui32Triangles.data());

        auto texcoordsInfo = accessors[cgltf_attribute_type_texcoord];
        if (texcoordsInfo) {
            if (texcoordsInfo->count != vertexCount || texcoordsInfo->type != cgltf_type_vec2) {
                slog.e << "Bad texcoord count or type." << io::endl;
                return;
            }
            fp32TexCoords.resize(vertexCount);
            cgltf_accessor_unpack_floats(texcoordsInfo, &fp32TexCoords[0].x, vertexCount * 2);
            sob.uvs(fp32TexCoords.data());
        }

        // Compute surface orientation quaternions.
        auto helper = sob.build();
        helper.getQuats(quats, vertexCount);

        // Upload quaternions to the GPU.
        VertexBuffer::BufferDescriptor bd(quats, vertexCount * sizeof(short4), FREE_CALLBACK);
        vb->setBufferAt(*mConfig.engine, slot, std::move(bd));
    };

    // Collect all TANGENT vertex attribute slots that need to be populated.
    tsl::robin_map<VertexBuffer*, uint8_t> baseTangents;
    tsl::robin_map<VertexBuffer*, uint8_t> morphTangents[4];
    const BufferBinding* bindings = asset->getBufferBindings();
    for (size_t i = 0, n = asset->getBufferBindingCount(); i < n; ++i) {
        auto bb = bindings[i];
        if (bb.vertexBuffer && bb.generateTangents) {
            if (bb.isMorphTarget) {
                morphTangents[bb.morphTargetIndex][bb.vertexBuffer] = bb.bufferIndex;
            } else {
                baseTangents[bb.vertexBuffer] = bb.bufferIndex;
            }
        }
    }

    // Go through all cgltf primitives and populate their tangents if requested.
    for (auto iter : asset->mNodeMap) {
        const cgltf_mesh* mesh = iter.first->mesh;
        if (mesh) {
            cgltf_size nprims = mesh->primitives_count;
            for (cgltf_size index = 0; index < nprims; ++index) {
                VertexBuffer* vb = asset->mPrimMap.at(mesh->primitives + index);
                auto iter = baseTangents.find(vb);
                if (iter != baseTangents.end()) {
                    computeQuats(mesh->primitives[index], vb, iter->second, kMorphTargetUnused);
                }
                for (int morphTarget = 0; morphTarget < 4; morphTarget++) {
                    const auto& tangents = morphTangents[morphTarget];
                    auto iter = tangents.find(vb);
                    if (iter != tangents.end()) {
                        computeQuats(mesh->primitives[index], vb, iter->second, morphTarget);
                    }
                }
            }
        }
    }
}

void ResourceLoader::normalizeSkinningWeights(details::FFilamentAsset* asset) const {
    auto normalize = [](cgltf_accessor* data) {
        if (data->type != cgltf_type_vec4 || data->component_type != cgltf_component_type_r_32f) {
            slog.w << "Cannot normalize weights, unsupported attribute type." << io::endl;
            return;
        }
        uint8_t* bytes = (uint8_t*) data->buffer_view->buffer->data;
        float4* floats = (float4*) (bytes + data->offset + data->buffer_view->offset);
        for (cgltf_size i = 0; i < data->count; ++i) {
            float4 weights = floats[i];
            float sum = weights.x + weights.y + weights.z + weights.w;
            floats[i] = weights / sum;
        }
    };
    const cgltf_data* gltf = asset->mSourceAsset;
    cgltf_size mcount = gltf->meshes_count;
    for (cgltf_size mindex = 0; mindex < mcount; ++mindex) {
        const cgltf_mesh& mesh = gltf->meshes[mindex];
        cgltf_size pcount = mesh.primitives_count;
        for (cgltf_size pindex = 0; pindex < pcount; ++pindex) {
            const cgltf_primitive& prim = mesh.primitives[pindex];
            cgltf_size acount = prim.attributes_count;
            for (cgltf_size aindex = 0; aindex < acount; ++aindex) {
                const auto& attr = prim.attributes[aindex];
                if (attr.type == cgltf_attribute_type_weights) {
                    normalize(attr.data);
                }
            }
        }
    }
}

void ResourceLoader::updateBoundingBoxes(details::FFilamentAsset* asset) const {
    auto& rm = mConfig.engine->getRenderableManager();
    auto& tm = mConfig.engine->getTransformManager();

    // The purpose of the root node is to give the client a place for custom transforms.
    // Since it is not part of the source model, it should be ignored when computing the
    // bounding box.
    TransformManager::Instance root = tm.getInstance(asset->getRoot());
    std::vector<Entity> modelRoots(tm.getChildCount(root));
    tm.getChildren(root, modelRoots.data(), modelRoots.size());
    for (auto e : modelRoots) {
        tm.setParent(tm.getInstance(e), 0);
    }

    auto computeBoundingBox = [&](const cgltf_primitive& prim) {
        Aabb aabb;
        for (cgltf_size slot = 0; slot < prim.attributes_count; slot++) {
            const cgltf_attribute& attr = prim.attributes[slot];
            const cgltf_accessor* accessor = attr.data;
            const size_t dim = cgltf_num_components(accessor->type);
            if (attr.type == cgltf_attribute_type_position && dim >= 3) {
                std::vector<float> unpacked(accessor->count * dim);
                cgltf_accessor_unpack_floats(accessor, unpacked.data(), unpacked.size());
                for (cgltf_size i = 0, j = 0, n = accessor->count; i < n; ++i, j += dim) {
                    float3 pt(unpacked[j + 0], unpacked[j + 1], unpacked[j + 2]);
                    aabb.min = min(aabb.min, pt);
                    aabb.max = max(aabb.max, pt);
                }
                break;
            }
        }
        return aabb;
    };

    Aabb assetBounds;
    for (auto iter : asset->mNodeMap) {
        const cgltf_mesh* mesh = iter.first->mesh;
        if (mesh) {
            // Find the object-space bounds for the renderable by unioning the bounds of each prim.
            Aabb aabb;
            for (cgltf_size index = 0, nprims = mesh->primitives_count; index < nprims; ++index) {
                Aabb primBounds = computeBoundingBox(mesh->primitives[index]);
                aabb.min = min(aabb.min, primBounds.min);
                aabb.max = max(aabb.max, primBounds.max);
            }
            auto renderable = rm.getInstance(iter.second);
            rm.setAxisAlignedBoundingBox(renderable, Box().set(aabb.min, aabb.max));

            // Transform this bounding box, then update the asset-level bounding box.
            auto transformable = tm.getInstance(iter.second);
            const mat4f worldTransform = tm.getWorldTransform(transformable);
            const Aabb transformed = aabb.transform(worldTransform);
            assetBounds.min = min(assetBounds.min, transformed.min);
            assetBounds.max = max(assetBounds.max, transformed.max);
        }
    }

    for (auto e : modelRoots) {
        tm.setParent(tm.getInstance(e), root);
    }

    asset->mBoundingBox = assetBounds;
}

} // namespace gltfio
