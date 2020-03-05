/*
 * Copyright (C) 2020 The Android Open Source Project
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
#include <gltfio/Image.h>

#include "FFilamentAsset.h"
#include "upcast.h"

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/VertexBuffer.h>

#include <geometry/SurfaceOrientation.h>

#include <utils/JobSystem.h>
#include <utils/Log.h>

#include <cgltf.h>

#include <math/quat.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <tsl/robin_map.h>

#include <string>

#if defined(__EMSCRIPTEN__) || defined(ANDROID)
#define USE_FILESYSTEM 0
#else
#define USE_FILESYSTEM 1
#include <utils/Path.h>
#endif

using namespace filament;
using namespace filament::math;
using namespace utils;

static const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

namespace {
    struct TextureCacheEntry {
        Texture* texture;
        std::atomic<stbi_uc*> texels;
        uint32_t bufferSize;
        int width;
        int height;
        int numComponents;
        bool srgb;
        bool completed;
    };

    using BufferTextureCache = tsl::robin_map<const void*, std::unique_ptr<TextureCacheEntry>>;
    using UriTextureCache = tsl::robin_map<std::string, std::unique_ptr<TextureCacheEntry>>;
    using UriDataCache = tsl::robin_map<std::string, gltfio::ResourceLoader::BufferDescriptor>;
}

namespace gltfio {

struct ResourceLoader::Impl {
    Impl(const ResourceConfiguration& config) {
        mGltfPath = std::string(config.gltfPath ? config.gltfPath : "");
        mEngine = config.engine;
        mNormalizeSkinningWeights = config.normalizeSkinningWeights;
        mRecomputeBoundingBoxes = config.recomputeBoundingBoxes;
    }

    Engine* mEngine;
    bool mNormalizeSkinningWeights;
    bool mRecomputeBoundingBoxes;
    std::string mGltfPath;

    // User-provided resource data with URI string keys, populated with addResourceData().
    // This is used on platforms without traditional file systems, such as Android and WebGL.
    UriDataCache mUriDataCache;

    // The two texture caches are populated while textures are being decoded, and they are no longer
    // used after all textures have been finalized. Since multiple glTF textures might be loaded
    // from a single URI or buffer pointer, these caches prevent needless re-decoding. There are
    // two caches: one for URI-based textures and one for buffer-based textures.
    BufferTextureCache mBufferTextureCache;
    UriTextureCache mUriTextureCache;
    int mNumDecoderTasks;
    int mNumDecoderTasksFinished;
    utils::JobSystem::Job* mDecoderRootJob = nullptr;
    details::FFilamentAsset* mCurrentAsset;

    bool createTextures(bool async);
    void addTextureCacheEntry(const details::TextureSlot& tb);
    void bindTextureToMaterial(const details::TextureSlot& tb);
    void decodeSingleTexture();
    void uploadPendingTextures();
    ~Impl();
};

namespace details {

uint32_t computeBindingSize(const cgltf_accessor* accessor);
uint32_t computeBindingOffset(const cgltf_accessor* accessor);

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

static void convertBytesToShorts(uint16_t* dst, const uint8_t* src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

ResourceLoader::ResourceLoader(const ResourceConfiguration& config) :
        mPool(new AssetPool), pImpl(new Impl(config)) { }

ResourceLoader::~ResourceLoader() {
    mPool->onLoaderDestroyed();
    delete pImpl;
}

void ResourceLoader::addResourceData(const char* uri, BufferDescriptor&& buffer) {
    pImpl->mUriDataCache.emplace(uri, std::move(buffer));
}

bool ResourceLoader::hasResourceData(const char* uri) const {
    return pImpl->mUriDataCache.find(uri) != pImpl->mUriDataCache.end();
}

bool ResourceLoader::loadResources(FilamentAsset* asset) {
    FFilamentAsset* fasset = upcast(asset);
    return loadResources(fasset, false);
}

bool ResourceLoader::loadResources(FFilamentAsset* asset, bool async) {
    if (asset->mResourcesLoaded) {
        return false;
    }
    asset->mResourcesLoaded = true;
    mPool->addAsset(asset);
    auto gltf = (cgltf_data*) asset->mSourceAsset;
    cgltf_options options {};

    // For emscripten and Android builds we have a custom implementation of cgltf_load_buffers which
    // looks inside a cache of externally-supplied data blobs, rather than loading from the
    // filesystem.

    #if !USE_FILESYSTEM

    if (gltf->buffers_count && !gltf->buffers[0].data && !gltf->buffers[0].uri && gltf->bin) {
        if (gltf->bin_size < gltf->buffers[0].size) {
            slog.e << "Bad size." << io::endl;
            return false;
        }
        gltf->buffers[0].data = (void*) gltf->bin;
    }

    bool missingResources = false;

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
            auto iter = pImpl->mUriDataCache.find(uri);
            if (iter == pImpl->mUriDataCache.end()) {
                slog.e << "Unable to load external resource: " << uri << io::endl;
                missingResources = true;
            }
            // Make a copy to allow cgltf_free() to work as expected and prevent a double-free.
            // TODO: Future versions of CGLTF will make this easier, see the following ticket.
            // https://github.com/jkuhlmann/cgltf/issues/94
            gltf->buffers[i].data = malloc(iter->second.size);
            memcpy(gltf->buffers[i].data, iter->second.buffer, iter->second.size);
        } else {
            slog.e << "Unable to load " << uri << io::endl;
            return false;
        }
    }

    if (missingResources) {
        slog.e << "Some external resources have not been added via addResourceData()" << io::endl;
        return false;
    }

    #else

    // Read data from the file system and base64 URIs.
    cgltf_result result = cgltf_load_buffers(&options, gltf, pImpl->mGltfPath.c_str());
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
    if (pImpl->mNormalizeSkinningWeights) {
        normalizeSkinningWeights(asset);
    }

    if (pImpl->mRecomputeBoundingBoxes) {
        updateBoundingBoxes(asset);
    }

    Engine& engine = *pImpl->mEngine;

    // Upload VertexBuffer and IndexBuffer data to the GPU.

    for (auto slot : asset->mBufferSlots) {
        const cgltf_accessor* accessor = slot.accessor;
        if (!accessor->buffer_view) {
            continue;
        }
        auto bufferData = (const uint8_t*) accessor->buffer_view->buffer->data;
        const uint8_t* data = computeBindingOffset(accessor) + bufferData;
        const uint32_t size = computeBindingSize(accessor);
        if (slot.vertexBuffer) {
            mPool->addPendingUpload();
            VertexBuffer::BufferDescriptor bd(data, size, AssetPool::onLoadedResource, mPool);
            slot.vertexBuffer->setBufferAt(engine, slot.bufferIndex, std::move(bd));
            continue;
        }
        assert(slot.indexBuffer);
        if (accessor->component_type == cgltf_component_type_r_8u) {
            const size_t size16 = size * 2;
            uint16_t* data16 = (uint16_t*) malloc(size16);
            convertBytesToShorts(data16, data, size);
            IndexBuffer::BufferDescriptor bd(data16, size16, FREE_CALLBACK);
            slot.indexBuffer->setBuffer(engine, std::move(bd));
            continue;
        }
        mPool->addPendingUpload();
        IndexBuffer::BufferDescriptor bd(data, size, AssetPool::onLoadedResource, mPool);
        slot.indexBuffer->setBuffer(engine, std::move(bd));
    }
    // Copy over the inverse bind matrices.
    for (cgltf_size i = 0, len = gltf->skins_count; i < len; ++i) {
        importSkinningData(asset->mSkins[i], gltf->skins[i]);
    }

    // Apply sparse data modifications to base arrays, then upload the result.
    applySparseData(asset);

    // Compute surface orientation quaternions if necessary. This is similar to sparse data in that
    // we need to generate the contents of a GPU buffer by processing one or more CPU buffer(s).
    computeTangents(asset);

    // Non-textured renderables are now considered ready, so notify the dep graph.
    asset->mDependencyGraph.finalize();
    pImpl->mCurrentAsset = asset;

    // Finally, load image files and create Filament Textures.
    return pImpl->createTextures(async);
}

bool ResourceLoader::asyncBeginLoad(FilamentAsset* asset) {
    return loadResources(upcast(asset), true);
}

float ResourceLoader::asyncGetLoadProgress() const {
    const float finished = pImpl->mNumDecoderTasksFinished;
    const float total = pImpl->mNumDecoderTasks;
    return total == 0 ? 0 : finished / total;
}

void ResourceLoader::asyncUpdateLoad() {
    if (!UTILS_HAS_THREADING) {
        pImpl->decodeSingleTexture();
    }
    pImpl->uploadPendingTextures();
}

void ResourceLoader::Impl::decodeSingleTexture() {
    assert(!UTILS_HAS_THREADING);
    int w, h, c;

    // Check if any buffer-based textures haven't been decoded yet.
    for (auto& pair : mBufferTextureCache) {
        const uint8_t* sourceData = (const uint8_t*) pair.first;
        TextureCacheEntry* entry = pair.second.get();
        if (entry->texels) {
            continue;
        }
        entry->texels = stbi_load_from_memory(sourceData, entry->bufferSize, &w, &h, &c, 4);
        return;
    }

    // Check if any URI-based textures haven't been decoded yet.
    for (auto& pair : mUriTextureCache) {
        auto uri = pair.first;
        TextureCacheEntry* entry = pair.second.get();
        if (entry->texels) {
            continue;
        }

        // First, check the user-supplied resource cache for this URI.
        auto iter = mUriDataCache.find(uri);
        if (iter != mUriDataCache.end()) {
            const uint8_t* sourceData = (const uint8_t*) iter->second.buffer;
            entry->texels = stbi_load_from_memory(sourceData, iter->second.size, &w, &h, &c, 4);
            return;
        }

        // Otherwise load it from the file system if this platform supports it.
        #if !USE_FILESYSTEM
            slog.e << "Unable to load texture: " << uri << io::endl;
            entry->completed = true;
            mNumDecoderTasksFinished++;
            return;
        #else
            utils::Path fullpath = utils::Path(mGltfPath).getParent() + uri;
            entry->texels = stbi_load(fullpath.c_str(), &w, &h, &c, 4);
            return;
        #endif
    }
}

void ResourceLoader::Impl::uploadPendingTextures() {
    auto upload = [this](TextureCacheEntry* entry, Engine& engine) {
        Texture* texture = entry->texture;
        uint8_t* texels = entry->texels;
        if (texture && texels && !entry->completed) {
            Texture::PixelBufferDescriptor pbd(texels,
                    texture->getWidth() * texture->getHeight() * 4,
                    Texture::Format::RGBA, Texture::Type::UBYTE, FREE_CALLBACK);
            texture->setImage(engine, 0, std::move(pbd));
            texture->generateMipmaps(engine);
            entry->completed = true;
            mNumDecoderTasksFinished++;
            mCurrentAsset->mDependencyGraph.markAsReady(texture);
        }
    };
    for (auto& pair : mBufferTextureCache) upload(pair.second.get(), *mEngine);
    for (auto& pair : mUriTextureCache) upload(pair.second.get(), *mEngine);
}

void ResourceLoader::Impl::addTextureCacheEntry(const TextureSlot& tb) {
    TextureCacheEntry* entry = nullptr;

    const cgltf_texture* srcTexture = tb.texture;
    const cgltf_buffer_view* bv = srcTexture->image->buffer_view;
    const char* uri = srcTexture->image->uri;
    const uint32_t totalSize = uint32_t(bv ? bv->size : 0);
    void** data = bv ? &bv->buffer->data : nullptr;
    const size_t offset = bv ? bv->offset : 0;

    // Check if the texture binding uses BufferView data (i.e. it does not have a URI).
    if (data) {
        const uint8_t* sourceData = offset + (const uint8_t*) *data;
        entry = mBufferTextureCache[sourceData] ? mBufferTextureCache[sourceData].get() : nullptr;
        if (entry) {
            return;
        }
        entry = (mBufferTextureCache[sourceData] = std::make_unique<TextureCacheEntry>()).get();
        entry->srgb = tb.srgb;
        stbi_info_from_memory(sourceData, totalSize, &entry->width, &entry->height,
                &entry->numComponents);
        entry->bufferSize = totalSize;
        return;
    }

    // Check if we already created a Texture object for this URI.
    entry = mUriTextureCache[uri] ? mUriTextureCache[uri].get() : nullptr;
    if (entry) {
        return;
    }

    entry = (mUriTextureCache[uri] = std::make_unique<TextureCacheEntry>()).get();
    entry->srgb = tb.srgb;

    // Check the user-supplied resource cache for this URI, otherwise peek at the file.
    auto iter = mUriDataCache.find(uri);
    if (iter != mUriDataCache.end()) {
        const uint8_t* sourceData = (const uint8_t*) iter->second.buffer;
        stbi_info_from_memory(sourceData, iter->second.size, &entry->width,
                &entry->height, &entry->numComponents);
        return;
    }
    #if !USE_FILESYSTEM
        slog.e << "Unable to load texture: " << uri << io::endl;
    #else
        utils::Path fullpath = utils::Path(mGltfPath).getParent() + uri;
        stbi_info(fullpath.c_str(), &entry->width, &entry->height, &entry->numComponents);
    #endif
}

void ResourceLoader::Impl::bindTextureToMaterial(const TextureSlot& tb) {
    details::FFilamentAsset* asset = mCurrentAsset;

    const cgltf_texture* srcTexture = tb.texture;
    const cgltf_buffer_view* bv = srcTexture->image->buffer_view;
    const char* uri = srcTexture->image->uri;
    void** data = bv ? &bv->buffer->data : nullptr;
    const size_t offset = bv ? bv->offset : 0;

    // First check if this is a buffer-based texture.
    if (data) {
        const uint8_t* sourceData = offset + (const uint8_t*) *data;
        auto& entry = mBufferTextureCache[sourceData];
        if (entry.get() && entry->texture) {
            asset->bindTexture(tb, entry->texture);
        }
        return;
    }

    // Next check if this is a URI-based texture.
    auto& entry = mUriTextureCache[uri];
    if (entry.get() && entry->texture) {
        asset->bindTexture(tb, entry->texture);
    }
}

bool ResourceLoader::Impl::createTextures(bool async) {
    // If any decoding jobs are still underway, wait for them to finish.
    utils::JobSystem* js = utils::JobSystem::getJobSystem();
    if (mDecoderRootJob) {
        js->waitAndRelease(mDecoderRootJob);
        mDecoderRootJob = nullptr;
    }

    mBufferTextureCache.clear();
    mUriTextureCache.clear();

    // First, determine texture dimensions and create texture cache entries.
    details::FFilamentAsset* asset = mCurrentAsset;
    for (auto slot : asset->mTextureSlots) {
        addTextureCacheEntry(slot);
    }

    // Tally up the total number of textures that need to be decoded. Zero textures is a special
    // case that needs to report 100% progress right away, so we set NumDecoderTasks and Finished
    // both to 1. If they were both 0, this would indicate that loading has not started.
    mNumDecoderTasks = mBufferTextureCache.size() + mUriTextureCache.size();
    if (mNumDecoderTasks == 0) {
        mNumDecoderTasks = 1;
        mNumDecoderTasksFinished = 1;
    } else {
        mNumDecoderTasksFinished = 0;
    }

    // Next create blank Filament textures.
    auto createTexture = [=](TextureCacheEntry* entry) {
        entry->texture = Texture::Builder()
            .width(entry->width)
            .height(entry->height)
            .levels(0xff)
            .format(entry->srgb ? Texture::InternalFormat::SRGB8_A8 : Texture::InternalFormat::RGBA8)
            .build(*mEngine);
        asset->takeOwnership(entry->texture);
    };
    for (auto& pair : mBufferTextureCache) createTexture(pair.second.get());
    for (auto& pair : mUriTextureCache) createTexture(pair.second.get());

    // Bind the textures to material instances.
    for (auto slot : asset->mTextureSlots) {
        bindTextureToMaterial(slot);
    }

    // Before creating jobs for PNG / JPEG decoding, we might need to return early. On single
    // threaded systems, it is usually fine to create jobs because the job system will simply
    // execute serially. However if the client requests async behavior, then we need to wait
    // until subsequent calls to asyncUpdateLoad().
    if (!UTILS_HAS_THREADING && async) {
        return true;
    }

    utils::JobSystem::Job* parent = js->createJob();

    // Kick off jobs that decode texels from buffer pointers.
    for (auto& pair : mBufferTextureCache) {
        const uint8_t* sourceData = (const uint8_t*) pair.first;
        TextureCacheEntry* entry = pair.second.get();
        utils::JobSystem::Job* decode = utils::jobs::createJob(*js, parent, [=] {
            int width, height, comp;
            entry->texels = stbi_load_from_memory(sourceData, entry->bufferSize,
                    &width, &height, &comp, 4);
        });
        js->run(decode);
    }

    // Kick off jobs that decode texels from URI strings.
    for (auto& pair : mUriTextureCache) {
        auto uri = pair.first;
        TextureCacheEntry* entry = pair.second.get();

        // First, check the user-supplied resource cache for this URI.
        auto iter = mUriDataCache.find(uri);
        if (iter != mUriDataCache.end()) {
            const uint8_t* sourceData = (const uint8_t*) iter->second.buffer;
            utils::JobSystem::Job* decode = utils::jobs::createJob(*js, parent, [=] {
                int width, height, comp;
                entry->texels = stbi_load_from_memory(sourceData, iter->second.size, &width,
                        &height, &comp, 4);
            });
            js->run(decode);
            continue;
        }

        // Otherwise load it from the file system if this platform supports it.
        #if !USE_FILESYSTEM
            slog.e << "Unable to load texture: " << uri << io::endl;
            return false;
        #else
            utils::Path fullpath = utils::Path(mGltfPath).getParent() + uri;
            utils::JobSystem::Job* decode = utils::jobs::createJob(*js, parent, [=] {
                int width, height, comp;
                entry->texels = stbi_load(fullpath.c_str(), &width, &height, &comp, 4);
            });
            js->run(decode);
        #endif
    }

    if (async) {
        mDecoderRootJob = js->runAndRetain(parent);
        return true;
    }

    // Wait for decoding to finish.
    js->runAndWait(parent);

    // Finally, upload texels to the GPU and generate mipmaps.
    mCurrentAsset = asset;
    uploadPendingTextures();

    return true;
}

ResourceLoader::Impl::~Impl() {
    utils::JobSystem* js = utils::JobSystem::getJobSystem();
    if (mDecoderRootJob) {
        js->waitAndRelease(mDecoderRootJob);
    }
}

void ResourceLoader::applySparseData(FFilamentAsset* asset) const {
    for (auto slot : asset->mBufferSlots) {
        const cgltf_accessor* accessor = slot.accessor;
        if (!accessor->is_sparse) {
            continue;
        }
        cgltf_size numFloats = accessor->count * cgltf_num_components(accessor->type);
        cgltf_size numBytes = sizeof(float) * numFloats;
        float* generated = (float*) malloc(numBytes);
        cgltf_accessor_unpack_floats(accessor, generated, numFloats);
        VertexBuffer::BufferDescriptor bd(generated, numBytes, FREE_CALLBACK);
        slot.vertexBuffer->setBufferAt(*pImpl->mEngine, slot.bufferIndex, std::move(bd));
    }
}

void ResourceLoader::computeTangents(FFilamentAsset* asset) const {
    const cgltf_accessor* kGenerateTangents = &asset->mGenerateTangents;
    const cgltf_accessor* kGenerateNormals = &asset->mGenerateNormals;

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
        if (vertexCount == 0) {
            return;
        }

        short4* quats = (short4*) malloc(sizeof(short4) * vertexCount);

        geometry::SurfaceOrientation::Builder sob;
        sob.vertexCount(vertexCount);

        // Convert normals into packed floats.
        if (normalsInfo) {
            assert(normalsInfo->count == vertexCount);
            assert(normalsInfo->type == cgltf_type_vec3);
            fp32Normals.resize(vertexCount);
            cgltf_accessor_unpack_floats(normalsInfo, &fp32Normals[0].x, vertexCount * 3);
            sob.normals(fp32Normals.data());
        }

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
        geometry::SurfaceOrientation* helper = sob.build();
        helper->getQuats(quats, vertexCount);
        delete helper;

        // Upload quaternions to the GPU.
        VertexBuffer::BufferDescriptor bd(quats, vertexCount * sizeof(short4), FREE_CALLBACK);
        vb->setBufferAt(*pImpl->mEngine, slot, std::move(bd));
    };

    // Collect all TANGENT vertex attribute slots that need to be populated.
    tsl::robin_map<VertexBuffer*, uint8_t> baseTangents;
    tsl::robin_map<VertexBuffer*, uint8_t> morphTangents[4];
    for (auto slot : asset->mBufferSlots) {
        if (slot.accessor != kGenerateTangents && slot.accessor != kGenerateNormals) {
            continue;
        }
        if (slot.morphTarget) {
            morphTangents[slot.morphTarget - 1][slot.vertexBuffer] = slot.bufferIndex;
            continue;
        }
        baseTangents[slot.vertexBuffer] = slot.bufferIndex;
    }

    // Go through all cgltf primitives and populate their tangents if requested.
    for (auto pair : asset->mPrimitives) {
        const cgltf_primitive& prim = *pair.first;
        VertexBuffer* vb = pair.second;
        auto iter = baseTangents.find(vb);
        if (iter != baseTangents.end()) {
            computeQuats(prim, vb, iter->second, kMorphTargetUnused);
        }
        for (int morphTarget = 0; morphTarget < 4; morphTarget++) {
            const auto& tangents = morphTangents[morphTarget];
            auto iter = tangents.find(vb);
            if (iter != tangents.end()) {
                computeQuats(prim, vb, iter->second, morphTarget);
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
    auto& rm = pImpl->mEngine->getRenderableManager();
    auto& tm = pImpl->mEngine->getTransformManager();

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
