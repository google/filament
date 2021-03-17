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
#include "TangentsJob.h"
#include "upcast.h"

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/VertexBuffer.h>

#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Systrace.h>

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
    JobSystem::Job* mDecoderRootJob = nullptr;
    FFilamentAsset* mCurrentAsset = nullptr;

    void computeTangents(FFilamentAsset* asset);
    bool createTextures(bool async);
    void cancelTextureDecoding();
    void addTextureCacheEntry(const TextureSlot& tb);
    void bindTextureToMaterial(const TextureSlot& tb);
    void decodeSingleTexture();
    void uploadPendingTextures();
    void releasePendingTextures();
    ~Impl();
};

uint32_t computeBindingSize(const cgltf_accessor* accessor);
uint32_t computeBindingOffset(const cgltf_accessor* accessor);

// This little struct holds a shared_ptr that wraps cgltf_data (and, potentially, glb data) while
// uploading vertex buffer data to the GPU.
struct UploadEvent {
    FFilamentAsset::SourceHandle handle;
};

UploadEvent* uploadUserdata(FFilamentAsset* asset) {
    return new UploadEvent({ asset->mSourceAsset });
}

static void uploadCallback(void* buffer, size_t size, void* user) {
    auto event = (UploadEvent*) user;
    delete event;
}

void importSkins(const cgltf_data* gltf, const NodeMap& nodeMap, SkinVector& dstSkins) {
    dstSkins.resize(gltf->skins_count);
    for (cgltf_size i = 0, len = gltf->nodes_count; i < len; ++i) {
        const cgltf_node& node = gltf->nodes[i];
        if (node.skin) {
            int skinIndex = node.skin - &gltf->skins[0];
            Entity entity = nodeMap.at(&node);
            dstSkins[skinIndex].targets.push_back(entity);
        }
    }
    for (cgltf_size i = 0, len = gltf->skins_count; i < len; ++i) {
        Skin& dstSkin = dstSkins[i];
        const cgltf_skin& srcSkin = gltf->skins[i];
        if (srcSkin.name) {
            dstSkin.name = srcSkin.name;
        }

        // Build a list of transformables for this skin, one for each joint.
        // TODO: We've seen models with joint nodes that do not belong to the scene's node graph.
        // e.g. BrainStem after Draco compression. That's why we have a fallback here. AssetManager
        // should maybe create an Entity for every glTF node, period. (regardless of hierarchy)
        // https://github.com/CesiumGS/gltf-pipeline/issues/532
        dstSkin.joints.resize(srcSkin.joints_count);
        for (cgltf_size i = 0, len = srcSkin.joints_count; i < len; ++i) {
            auto iter = nodeMap.find(srcSkin.joints[i]);
            if (iter == nodeMap.end()) {
                dstSkin.joints[i] = nodeMap.begin()->second;
            } else {
                dstSkin.joints[i] = iter->second;
            }
        }

        // Retain a copy of the inverse bind matrices because the source blob could be evicted later.
        const cgltf_accessor* srcMatrices = srcSkin.inverse_bind_matrices;
        dstSkin.inverseBindMatrices.resize(srcSkin.joints_count);
        if (srcMatrices) {
            auto dstMatrices = (uint8_t*) dstSkin.inverseBindMatrices.data();
            uint8_t* bytes = (uint8_t*) srcMatrices->buffer_view->buffer->data;
            if (!bytes) {
                slog.w << "Empty animation buffer, have resources been loaded yet?" << io::endl;
                continue;
            }
            auto srcBuffer = (void*) (bytes + srcMatrices->offset + srcMatrices->buffer_view->offset);
            memcpy(dstMatrices, srcBuffer, srcSkin.joints_count * sizeof(mat4f));
        }
    }
}

static void convertBytesToShorts(uint16_t* dst, const uint8_t* src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

static void decodeDracoMeshes(FFilamentAsset* asset) {
    DracoCache* dracoCache = &asset->mSourceAsset->dracoCache;

    // For a given primitive and attribute, find the corresponding accessor.
    auto findAccessor = [](const cgltf_primitive* prim, cgltf_attribute_type type, cgltf_int idx) {
        for (cgltf_size i = 0; i < prim->attributes_count; i++) {
            const cgltf_attribute& attr = prim->attributes[i];
            if (attr.type == type && attr.index == idx) {
                return attr.data;
            }
        }
        return (cgltf_accessor*) nullptr;
    };

    // Go through every primitive and check if it has a Draco mesh.
    for (auto pair : asset->mPrimitives) {
        const cgltf_primitive* prim = pair.first;
        VertexBuffer* vb = pair.second;
        if (!prim->has_draco_mesh_compression) {
            continue;
        }

        const cgltf_draco_mesh_compression& draco = prim->draco_mesh_compression;

        // Check if we have already decoded this mesh.
        DracoMesh* mesh = dracoCache->findOrCreateMesh(draco.buffer_view);
        if (!mesh) {
            slog.w << "Cannot decompress mesh, Draco decoding error." << io::endl;
            continue;
        }

        // Copy over the decompressed data, converting the data type if necessary.
        if (prim->indices) {
            mesh->getFaceIndices(prim->indices);
        }

        // Go through each attribute in the decompressed mesh.
        for (cgltf_size i = 0; i < draco.attributes_count; i++) {

            // In cgltf, each Draco attribute's data pointer is an attribute id, not an accessor.
            const uint32_t id = draco.attributes[i].data - asset->mSourceAsset->hierarchy->accessors;

            // Find the destination accessor; this contains the desired component type, etc.
            const cgltf_attribute_type type = draco.attributes[i].type;
            const cgltf_int index = draco.attributes[i].index;
            cgltf_accessor* accessor = findAccessor(prim, type, index);
            if (!accessor) {
                slog.w << "Cannot find matching accessor for Draco id " << id << io::endl;
                continue;
            }

            // Copy over the decompressed data, converting the data type if necessary.
            mesh->getVertexAttributes(id, accessor);
        }
    }
}

// Parses a data URI and returns a blob that gets malloc'd in cgltf, which the caller must free.
// (implementation snarfed from meshoptimizer)
static const uint8_t* parseDataUri(const char* uri, std::string* mimeType, size_t* psize) {
    if (strncmp(uri, "data:", 5) != 0) {
        return nullptr;
    }
    const char* comma = strchr(uri, ',');
    if (comma && comma - uri >= 7 && strncmp(comma - 7, ";base64", 7) == 0) {
        const char* base64 = comma + 1;
        const size_t base64Size = strlen(base64);
        size_t size = base64Size - base64Size / 4;
        if (base64Size >= 2) {
            size -= base64[base64Size - 2] == '=';
            size -= base64[base64Size - 1] == '=';
        }
        void* data = 0;
        cgltf_options options = {};
        cgltf_result result = cgltf_load_buffer_base64(&options, size, base64, &data);
        if (result != cgltf_result_success) {
            return nullptr;
        }
        *mimeType = std::string(uri + 5, comma - 7);
        *psize = size;
        return (const uint8_t*) data;
    }
    return nullptr;
}

ResourceLoader::ResourceLoader(const ResourceConfiguration& config) : pImpl(new Impl(config)) { }

ResourceLoader::~ResourceLoader() {
    delete pImpl;
}

void ResourceLoader::addResourceData(const char* uri, BufferDescriptor&& buffer) {
    // Start an async marker the first time this is called and end it when
    // finalization begins. This marker provides a rough indicator of how long
    // the client is taking to load raw data blobs from storage.
    if (pImpl->mUriDataCache.empty()) {
        SYSTRACE_CONTEXT();
        SYSTRACE_ASYNC_BEGIN("addResourceData", 1);
    }
    // NOTE: replacing an existing item in a robin map does not seem to behave as expected.
    // To work around this, we explicitly erase the old element if it already exists.
    auto iter = pImpl->mUriDataCache.find(uri);
    if (iter != pImpl->mUriDataCache.end()) {
        pImpl->mUriDataCache.erase(iter);
    }
    pImpl->mUriDataCache.emplace(uri, std::move(buffer));
}

bool ResourceLoader::hasResourceData(const char* uri) const {
    return pImpl->mUriDataCache.find(uri) != pImpl->mUriDataCache.end();
}

void ResourceLoader::evictResourceData() {
    // Note that this triggers BufferDescriptor callbacks.
    pImpl->mUriDataCache.clear();
}

bool ResourceLoader::loadResources(FilamentAsset* asset) {
    FFilamentAsset* fasset = upcast(asset);
    return loadResources(fasset, false);
}

bool ResourceLoader::loadResources(FFilamentAsset* asset, bool async) {
    SYSTRACE_CONTEXT();
    SYSTRACE_ASYNC_END("addResourceData", 1);

    if (asset->mResourcesLoaded) {
        return false;
    }
    const cgltf_data* gltf = asset->mSourceAsset->hierarchy;
    cgltf_options options {};

    // For emscripten and Android builds we have a custom implementation of cgltf_load_buffers which
    // looks inside a cache of externally-supplied data blobs, rather than loading from the
    // filesystem.

    SYSTRACE_NAME_BEGIN("Load buffers");
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
    cgltf_result result = cgltf_load_buffers(&options, (cgltf_data*) gltf, pImpl->mGltfPath.c_str());
    if (result != cgltf_result_success) {
        slog.e << "Unable to load resources." << io::endl;
        return false;
    }

    #endif
    SYSTRACE_NAME_END();

    #ifndef NDEBUG
    if (cgltf_validate((cgltf_data*) gltf) != cgltf_result_success) {
        slog.e << "Failed cgltf validation." << io::endl;
        return false;
    }
    #endif

    // Decompress Draco meshes early on, which allows us to exploit subsequent processing such as
    // tangent generation.
    decodeDracoMeshes(asset);

    // Normalize skinning weights, then "import" each skin into the asset by building a mapping of
    // skins to their affected entities.
    if (gltf->skins_count > 0) {
        if (pImpl->mNormalizeSkinningWeights) {
            normalizeSkinningWeights(asset);
        }
        if (!asset->isInstanced()) {
            importSkins(gltf, asset->mNodeMap, asset->mSkins);
        } else {
            // NOTE: This takes care of up-front instances, but dynamically added instances also
            // need to import the skin data, which is done in AssetLoader.
            for (FFilamentInstance* instance : asset->mInstances) {
                importSkins(gltf, instance->nodeMap, instance->skins);
            }
        }
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
            VertexBuffer::BufferDescriptor bd(data, size, uploadCallback, uploadUserdata(asset));
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
        IndexBuffer::BufferDescriptor bd(data, size, uploadCallback, uploadUserdata(asset));
        slot.indexBuffer->setBuffer(engine, std::move(bd));
    }

    // Apply sparse data modifications to base arrays, then upload the result.
    applySparseData(asset);

    // Compute surface orientation quaternions if necessary. This is similar to sparse data in that
    // we need to generate the contents of a GPU buffer by processing one or more CPU buffer(s).
    pImpl->computeTangents(asset);

    // Non-textured renderables are now considered ready, so notify the dependency graph.
    asset->mDependencyGraph.finalize();
    pImpl->mCurrentAsset = asset;

    // Finally, create Filament Textures and begin loading image files.
    asset->mResourcesLoaded = pImpl->createTextures(async);
    return asset->mResourcesLoaded;
}

bool ResourceLoader::asyncBeginLoad(FilamentAsset* asset) {
    return loadResources(upcast(asset), true);
}

void ResourceLoader::asyncCancelLoad() {
    pImpl->cancelTextureDecoding();
    pImpl->mEngine->flushAndWait();
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
            Path fullpath = Path(mGltfPath).getParent() + uri;
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

void ResourceLoader::Impl::releasePendingTextures() {
    auto release = [this](TextureCacheEntry* entry, Engine& engine) {
        Texture* texture = entry->texture;
        uint8_t* texels = entry->texels;
        if (texture && texels && !entry->completed) {
            // Normally the ownership of these texels is transferred to PixelBufferDescriptor, but
            // if uploads have been cancelled then we need to free them explicitly.
            free(texels);
        }
    };
    for (auto& pair : mBufferTextureCache) release(pair.second.get(), *mEngine);
    for (auto& pair : mUriTextureCache) release(pair.second.get(), *mEngine);
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
        if (!stbi_info_from_memory(sourceData, totalSize, &entry->width, &entry->height,
                &entry->numComponents)) {
            slog.e << "Unable to decode BufferView texture: " << stbi_failure_reason() << io::endl;
            mBufferTextureCache.erase(sourceData);
            return;
        }
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

    // Check if this is a data URI. We don't care about the MIME type since stb can infer it.
    std::string mimeType;
    size_t dataUriSize;
    const uint8_t* dataUriContent = parseDataUri(uri, &mimeType, &dataUriSize);
    if (dataUriContent) {
        BufferDescriptor buffer(dataUriContent, dataUriSize, FREE_CALLBACK);
        mUriDataCache.emplace(uri, std::move(buffer));
    }

    // Check the user-supplied resource cache for this URI, otherwise peek at the file.
    auto iter = mUriDataCache.find(uri);
    if (iter != mUriDataCache.end()) {
        const uint8_t* sourceData = (const uint8_t*) iter->second.buffer;
        if (!stbi_info_from_memory(sourceData, iter->second.size, &entry->width,
                &entry->height, &entry->numComponents)) {
            slog.e << "Unable to decode " << uri << " : " << stbi_failure_reason() << io::endl;
            mUriTextureCache.erase(uri);
        }
        return;
    }
    #if !USE_FILESYSTEM
        slog.e << "Unable to load texture: " << uri << io::endl;
    #else
        Path fullpath = Path(mGltfPath).getParent() + uri;
        if (!stbi_info(fullpath.c_str(), &entry->width, &entry->height, &entry->numComponents)) {
            slog.e << "Unable to decode " << fullpath.c_str() << " : " << stbi_failure_reason()
                    << io::endl;
            mUriTextureCache.erase(uri);
        }
    #endif
}

void ResourceLoader::Impl::bindTextureToMaterial(const TextureSlot& tb) {
    FFilamentAsset* asset = mCurrentAsset;

    const cgltf_texture* srcTexture = tb.texture;
    const cgltf_buffer_view* bv = srcTexture->image->buffer_view;
    const char* uri = srcTexture->image->uri;
    void** data = bv ? &bv->buffer->data : nullptr;
    const size_t offset = bv ? bv->offset : 0;

    // First check if this is a buffer-based texture.
    if (data) {
        const uint8_t* sourceData = offset + (const uint8_t*) *data;
        if (auto iter = mBufferTextureCache.find(sourceData); iter != mBufferTextureCache.end()) {
            auto& entry = iter->second;
            if (entry.get() && entry->texture) {
                asset->bindTexture(tb, entry->texture);
            }
        }
        return;
    }

    // Next check if this is a URI-based texture.
    if (auto iter = mUriTextureCache.find(uri); iter != mUriTextureCache.end()) {
        auto& entry = iter->second;
        if (entry.get() && entry->texture) {
            asset->bindTexture(tb, entry->texture);
        }
    }
}

void ResourceLoader::Impl::cancelTextureDecoding() {
    JobSystem* js = &mEngine->getJobSystem();
    if (mDecoderRootJob) {
        js->waitAndRelease(mDecoderRootJob);
        mDecoderRootJob = nullptr;
    }
    releasePendingTextures();
    mBufferTextureCache.clear();
    mUriTextureCache.clear();
    mCurrentAsset = nullptr;
    mNumDecoderTasksFinished = 0;
    mNumDecoderTasks = 0;
}

bool ResourceLoader::Impl::createTextures(bool async) {
    // If any decoding jobs are still underway, wait for them to finish.
    JobSystem* js = &mEngine->getJobSystem();
    if (mDecoderRootJob) {
        js->waitAndRelease(mDecoderRootJob);
        mDecoderRootJob = nullptr;
    }

    mBufferTextureCache.clear();
    mUriTextureCache.clear();

    // First, determine texture dimensions and create texture cache entries.
    FFilamentAsset* asset = mCurrentAsset;
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

    JobSystem::Job* parent = js->createJob();

    // Create a copy of the shared_ptr to the source data to prevent it from being freed during
    // the texture decoding process.
    FFilamentAsset::SourceHandle retainSourceAsset = asset->mSourceAsset;

    // Kick off jobs that decode texels from buffer pointers.
    for (auto& pair : mBufferTextureCache) {
        const uint8_t* sourceData = (const uint8_t*) pair.first;
        TextureCacheEntry* entry = pair.second.get();
        JobSystem::Job* decode = jobs::createJob(*js, parent, [retainSourceAsset, entry, sourceData] {
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
            JobSystem::Job* decode = jobs::createJob(*js, parent, [retainSourceAsset, entry, sourceData, iter] {
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
            Path fullpath = Path(mGltfPath).getParent() + uri;
            JobSystem::Job* decode = jobs::createJob(*js, parent, [retainSourceAsset, entry, fullpath] {
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

void ResourceLoader::Impl::computeTangents(FFilamentAsset* asset) {
    SYSTRACE_CALL();

    const cgltf_accessor* kGenerateTangents = &asset->mGenerateTangents;
    const cgltf_accessor* kGenerateNormals = &asset->mGenerateNormals;

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

    // Create a job description for each primitive.
    using Params = TangentsJob::Params;
    std::vector<Params> jobParams;
    for (auto pair : asset->mPrimitives) {
        VertexBuffer* vb = pair.second;
        auto iter = baseTangents.find(vb);
        if (iter != baseTangents.end()) {
            jobParams.emplace_back(Params {{ pair.first }, {vb, iter->second }});
        }
        for (int morphTarget = 0; morphTarget < 4; morphTarget++) {
            const auto& tangents = morphTangents[morphTarget];
            auto iter = tangents.find(vb);
            if (iter != tangents.end()) {
                jobParams.emplace_back(Params {{ pair.first, morphTarget }, {vb, iter->second }});
            }
        }
    }

    // Kick off jobs for computing tangent frames.
    JobSystem* js = &mEngine->getJobSystem();
    JobSystem::Job* parent = js->createJob();
    for (Params& params : jobParams) {
        Params* pptr = &params;
        js->run(jobs::createJob(*js, parent, [pptr] { TangentsJob::run(pptr); }));
    }
    js->runAndWait(parent);

    // Finally, upload quaternions to the GPU from the main thread.
    for (Params& params : jobParams) {
        VertexBuffer::BufferDescriptor bd(params.out.results,
                params.out.vertexCount * sizeof(short4), FREE_CALLBACK);
        params.context.vb->setBufferAt(*mEngine, params.context.slot, std::move(bd));
    }
}

ResourceLoader::Impl::~Impl() {
    if (mDecoderRootJob) {
        mEngine->getJobSystem().waitAndRelease(mDecoderRootJob);
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

void ResourceLoader::normalizeSkinningWeights(FFilamentAsset* asset) const {
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
    const cgltf_data* gltf = asset->mSourceAsset->hierarchy;
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

void ResourceLoader::updateBoundingBoxes(FFilamentAsset* asset) const {
    SYSTRACE_CALL();
    auto& rm = pImpl->mEngine->getRenderableManager();
    auto& tm = pImpl->mEngine->getTransformManager();
    NodeMap& nodeMap = asset->isInstanced() ? asset->mInstances[0]->nodeMap : asset->mNodeMap;

    // The purpose of the root node is to give the client a place for custom transforms.
    // Since it is not part of the source model, it should be ignored when computing the
    // bounding box.
    TransformManager::Instance root = tm.getInstance(asset->getRoot());
    std::vector<Entity> modelRoots(tm.getChildCount(root));
    tm.getChildren(root, modelRoots.data(), modelRoots.size());
    for (auto e : modelRoots) {
        tm.setParent(tm.getInstance(e), 0);
    }

    auto computeBoundingBox = [](const cgltf_primitive* prim, Aabb* result) {
        Aabb aabb;
        for (cgltf_size slot = 0; slot < prim->attributes_count; slot++) {
            const cgltf_attribute& attr = prim->attributes[slot];
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
        *result = aabb;
    };

    // Collect all mesh primitives that we wish to find bounds for.
    std::vector<cgltf_primitive const*> prims;
    for (auto iter : nodeMap) {
        const cgltf_mesh* mesh = iter.first->mesh;
        if (mesh) {
            for (cgltf_size index = 0, nprims = mesh->primitives_count; index < nprims; ++index) {
                prims.push_back(&mesh->primitives[index]);
            }
        }
    }

    // Kick off a bounding box job for every primitive.
    std::vector<Aabb> bounds(prims.size());
    JobSystem* js = &pImpl->mEngine->getJobSystem();
    JobSystem::Job* parent = js->createJob();
    for (size_t i = 0; i < prims.size(); ++i) {
        cgltf_primitive const* prim = prims[i];
        Aabb* result = &bounds[i];
        js->run(jobs::createJob(*js, parent, [prim, result, computeBoundingBox] {
            computeBoundingBox(prim, result);
        }));
    }
    js->runAndWait(parent);

    // Compute the asset-level bounding box.
    size_t primIndex = 0;
    Aabb assetBounds;
    for (auto iter : nodeMap) {
        const cgltf_mesh* mesh = iter.first->mesh;
        if (mesh) {
            // Find the object-space bounds for the renderable by unioning the bounds of each prim.
            Aabb aabb;
            for (cgltf_size index = 0, nprims = mesh->primitives_count; index < nprims; ++index) {
                Aabb primBounds = bounds[primIndex++];
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
