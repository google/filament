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
#include <gltfio/TextureProvider.h>

#include "GltfEnums.h"
#include "FFilamentAsset.h"
#include "TangentsJob.h"
#include "downcast.h"

#include <filament/BufferObject.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/VertexBuffer.h>
#include <filament/MorphTargetBuffer.h>

#include <geometry/Transcoder.h>

#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Systrace.h>
#include <utils/Path.h>

#include <cgltf.h>

#include <math/quat.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <tsl/robin_map.h>

#include <string>
#include <fstream>

using namespace filament;
using namespace filament::math;
using namespace utils;

using filament::geometry::ComponentType;

static const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

namespace filament::gltfio {

using BufferTextureCache = tsl::robin_map<const void*, Texture*>;
using FilepathTextureCache = tsl::robin_map<std::string, Texture*>;
using UriDataCache = tsl::robin_map<std::string, gltfio::ResourceLoader::BufferDescriptor>;
using TextureProviderList = tsl::robin_map<std::string, TextureProvider*>;

enum class CacheResult {
    ERROR,
    NOT_READY,
    FOUND,
    MISS,
};

struct ResourceLoader::Impl {
    Impl(const ResourceConfiguration& config) :
        mEngine(config.engine),
        mNormalizeSkinningWeights(config.normalizeSkinningWeights),
        mGltfPath(config.gltfPath ? config.gltfPath : "") {}

    Engine* const mEngine;
    const bool mNormalizeSkinningWeights;
    const std::string mGltfPath;

    // User-provided resource data with URI string keys, populated with addResourceData().
    // This is used on platforms without traditional file systems, such as Android, iOS, and WebGL.
    UriDataCache mUriDataCache;

    // User-provided mapping from mime types to texture providers.
    TextureProviderList mTextureProviders;

    // Avoid duplicated Texture objects via caches with two key types: buffer pointers and strings.
    BufferTextureCache mBufferTextureCache;
    FilepathTextureCache mFilepathTextureCache;

    FFilamentAsset* mAsyncAsset = nullptr;
    size_t mRemainingTextureDownloads = 0;

    void addResourceData(const char* uri, BufferDescriptor&& buffer);
    void computeTangents(FFilamentAsset* asset);
    void createTextures(FFilamentAsset* asset, bool async);
    void cancelTextureDecoding();
    std::pair<Texture*, CacheResult> getOrCreateTexture(FFilamentAsset* asset, size_t textureIndex,
            TextureProvider::TextureFlags flags);
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

static void convertBytesToShorts(uint16_t* dst, const uint8_t* src, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

static bool requiresConversion(const cgltf_accessor* accessor) {
    if (UTILS_UNLIKELY(accessor->is_sparse)) {
        return true;
    }
    const cgltf_type type = accessor->type;
    const cgltf_component_type ctype = accessor->component_type;
    filament::VertexBuffer::AttributeType permitted;
    filament::VertexBuffer::AttributeType actual;
    bool supported = getElementType(type, ctype, &permitted, &actual);
    return supported && permitted != actual;
}

static bool requiresPacking(const cgltf_accessor* accessor) {
    if (requiresConversion(accessor)) {
        return true;
    }
    const size_t dim = cgltf_num_components(accessor->type);
    switch (accessor->component_type) {
        case cgltf_component_type_r_8:
        case cgltf_component_type_r_8u:
            return accessor->stride != dim;
        case cgltf_component_type_r_16:
        case cgltf_component_type_r_16u:
            return accessor->stride != dim * 2;
        case cgltf_component_type_r_32u:
        case cgltf_component_type_r_32f:
            return accessor->stride != dim * 4;
        default:
            assert_invariant(false);
            return true;
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
    for (auto& [prim, vertexBuffer] : asset->mPrimitives) {
        if (!prim->has_draco_mesh_compression) {
            continue;
        }

        const cgltf_draco_mesh_compression& draco = prim->draco_mesh_compression;

        // If an error occurs, we can simply set the primitive's associated VertexBuffer to null.
        // This does not cause a leak because it is a weak reference.

        // Check if we have already decoded this mesh.
        DracoMesh* mesh = dracoCache->findOrCreateMesh(draco.buffer_view);
        if (!mesh) {
            slog.e << "Cannot decompress mesh, Draco decoding error." << io::endl;
            vertexBuffer = nullptr;
            continue;
        }

        // Copy over the decompressed data, converting the data type if necessary.
        if (prim->indices && !mesh->getFaceIndices(prim->indices)) {
            vertexBuffer = nullptr;
            continue;
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
            if (!mesh->getVertexAttributes(id, accessor)) {
                vertexBuffer = nullptr;
                break;
            }
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
    pImpl->addResourceData(uri, std::move(buffer));
}

static bool endsWith(std::string_view expr, std::string_view ending) {
    if (expr.length() >= ending.length()) {
        return (expr.compare(expr.length() - ending.length(), ending.length(), ending) == 0);
    }
    return false;
}

// TODO: This is not a great way to determine if a resource is a texture, but we can remove it after
// gltfio gains support for concurrent downloading of vertex data:
// https://github.com/google/filament/issues/5909
static bool isTexture(const char* uri) {
    using namespace std::literals;
    std::string_view urisv(uri);
    if (endsWith(urisv, ".png"sv)) {
        return true;
    }
    if (endsWith(urisv, ".ktx2"sv)) {
        return true;
    }
    if (endsWith(urisv, ".jpg"sv) || endsWith(urisv, ".jpeg"sv)) {
        return true;
    }
    return false;
}

void ResourceLoader::Impl::addResourceData(const char* uri, BufferDescriptor&& buffer) {
    // Start an async marker the first time this is called and end it when
    // finalization begins. This marker provides a rough indicator of how long
    // the client is taking to load raw data blobs from storage.
    if (mUriDataCache.empty()) {
        SYSTRACE_CONTEXT();
        SYSTRACE_ASYNC_BEGIN("addResourceData", 1);
    }
    // NOTE: replacing an existing item in a robin map does not seem to behave as expected.
    // To work around this, we explicitly erase the old element if it already exists.
    auto iter = mUriDataCache.find(uri);
    if (iter != mUriDataCache.end()) {
        mUriDataCache.erase(iter);
    }
    mUriDataCache.emplace(uri, std::move(buffer));

    // If this is a texture and async loading has already started, add a new decoder job.
    if (isTexture(uri) && mAsyncAsset && mRemainingTextureDownloads > 0) {
        createTextures(mAsyncAsset, true);
    }
}

bool ResourceLoader::hasResourceData(const char* uri) const {
    return pImpl->mUriDataCache.find(uri) != pImpl->mUriDataCache.end();
}

void ResourceLoader::evictResourceData() {
    // Note that this triggers BufferDescriptor callbacks.
    pImpl->mUriDataCache.clear();
}

bool ResourceLoader::loadResources(FilamentAsset* asset) {
    FFilamentAsset* fasset = downcast(asset);
    return loadResources(fasset, false);
}

bool ResourceLoader::loadResources(FFilamentAsset* asset, bool async) {
    SYSTRACE_CONTEXT();
    SYSTRACE_ASYNC_END("addResourceData", 1);

    if (asset->mResourcesLoaded) {
        return false;
    }
    asset->mResourcesLoaded = true;

    // At this point, any entities that are created in the future (i.e. dynamically added instances)
    // will not need the progressive feature to be enabled. This simplifies the dependency graph and
    // prevents it from growing.
    asset->mDependencyGraph.disableProgressiveReveal();

    // Clear our texture caches. Previous calls to loadResources may have populated these, but the
    // Texture objects could have since been destroyed.
    pImpl->mBufferTextureCache.clear();
    pImpl->mFilepathTextureCache.clear();

    const cgltf_data* gltf = asset->mSourceAsset->hierarchy;
    cgltf_options options {};

    // For emscripten and Android builds we supply a custom file reader callback that looks inside a
    // cache of externally-supplied data blobs, rather than loading from the filesystem.

    SYSTRACE_NAME_BEGIN("Load buffers");
    #if !GLTFIO_USE_FILESYSTEM

    struct Closure {
        Impl* impl;
        const cgltf_data* gltf;
    };

    Closure closure = { pImpl, gltf };

    options.file.user_data = &closure;

    options.file.read = [](const cgltf_memory_options* memoryOpts,
            const cgltf_file_options* fileOpts, const char* path, cgltf_size* size, void** data) {
        Closure* closure = (Closure*) fileOpts->user_data;
        auto& uriDataCache = closure->impl->mUriDataCache;

        if (auto iter = uriDataCache.find(path); iter != uriDataCache.end()) {
            *size = iter->second.size;
            *data = iter->second.buffer;
        } else {
            // Even if we don't find the given resource in the cache, we still return a successful
            // error code, because we allow downloads to finish after the decoding work starts.
           *size = 0;
           *data = 0;
        }

        return cgltf_result_success;
    };

    #endif

    // Read data from the file system and base64 URIs.
    cgltf_result result = cgltf_load_buffers(&options, (cgltf_data*) gltf, pImpl->mGltfPath.c_str());
    if (result != cgltf_result_success) {
        slog.e << "Unable to load resources." << io::endl;
        return false;
    }

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

    // For each skin, optionally normalize skinning weights and store a copy of the bind matrices.
    if (gltf->skins_count > 0) {
        if (pImpl->mNormalizeSkinningWeights) {
            normalizeSkinningWeights(asset);
        }
        asset->mSkins.reserve(gltf->skins_count);
        for (cgltf_size i = 0, len = gltf->skins_count; i < len; ++i) {
            const cgltf_skin& srcSkin = gltf->skins[i];
            CString name;
            if (srcSkin.name) {
                name = CString(srcSkin.name);
            }
            const cgltf_accessor* srcMatrices = srcSkin.inverse_bind_matrices;
            FixedCapacityVector<mat4f> inverseBindMatrices(srcSkin.joints_count);
            if (srcMatrices) {
                uint8_t* bytes = (uint8_t*) srcMatrices->buffer_view->buffer->data;
                assert_invariant(bytes);
                uint8_t* srcBuffer = bytes + srcMatrices->offset + srcMatrices->buffer_view->offset;
                memcpy((uint8_t*) inverseBindMatrices.data(),
                        (const void*) srcBuffer, srcSkin.joints_count * sizeof(mat4f));
            }
            FFilamentAsset::Skin skin {
                .name = std::move(name),
                .inverseBindMatrices = std::move(inverseBindMatrices),
            };
            asset->mSkins.emplace_back(std::move(skin));
        }
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
            if (requiresConversion(accessor)) {
                const size_t floatsCount = accessor->count * cgltf_num_components(accessor->type);
                const size_t floatsByteCount = sizeof(float) * floatsCount;
                float* floatsData = (float*) malloc(floatsByteCount);
                cgltf_accessor_unpack_floats(accessor, floatsData, floatsCount);
                BufferObject* bo = BufferObject::Builder().size(floatsByteCount).build(engine);
                asset->mBufferObjects.push_back(bo);
                bo->setBuffer(engine, BufferDescriptor(floatsData, floatsByteCount, FREE_CALLBACK));
                slot.vertexBuffer->setBufferObjectAt(engine, slot.bufferIndex, bo);
                continue;
            }
            BufferObject* bo = BufferObject::Builder().size(size).build(engine);
            asset->mBufferObjects.push_back(bo);
            bo->setBuffer(engine, BufferDescriptor(data, size,
                    uploadCallback, uploadUserdata(asset)));
            slot.vertexBuffer->setBufferObjectAt(engine, slot.bufferIndex, bo);
            continue;
        } else if (slot.indexBuffer) {
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
            continue;
        }

        // If the buffer slot does not have an associated VertexBuffer or IndexBuffer, then this
        // must be a morph target.
        assert(slot.morphTargetBuffer);

        if (requiresPacking(accessor)) {
            const size_t floatsCount = accessor->count * cgltf_num_components(accessor->type);
            const size_t floatsByteCount = sizeof(float) * floatsCount;
            float* floatsData = (float*) malloc(floatsByteCount);
            cgltf_accessor_unpack_floats(accessor, floatsData, floatsCount);
            if (accessor->type == cgltf_type_vec3) {
                slot.morphTargetBuffer->setPositionsAt(engine, slot.bufferIndex,
                        (const float3*) floatsData, slot.morphTargetBuffer->getVertexCount());
            } else {
                slot.morphTargetBuffer->setPositionsAt(engine, slot.bufferIndex,
                        (const float4*) data, slot.morphTargetBuffer->getVertexCount());
            }
            free(floatsData);
            continue;
        }

        if (accessor->type == cgltf_type_vec3) {
            slot.morphTargetBuffer->setPositionsAt(engine, slot.bufferIndex,
                    (const float3*) data, slot.morphTargetBuffer->getVertexCount());
        } else {
            assert_invariant(accessor->type == cgltf_type_vec4);
            slot.morphTargetBuffer->setPositionsAt(engine, slot.bufferIndex,
                    (const float4*) data, slot.morphTargetBuffer->getVertexCount());
        }
    }

    // Compute surface orientation quaternions if necessary. This is similar to sparse data in that
    // we need to generate the contents of a GPU buffer by processing one or more CPU buffer(s).
    pImpl->computeTangents(asset);

    asset->mBufferSlots = {};
    asset->mPrimitives = {};

    // If any decoding jobs are still underway from a previous load, wait for them to finish.
    for (const auto& iter : pImpl->mTextureProviders) {
        iter.second->waitForCompletion();
        iter.second->updateQueue();
    }

    // Finally, create Filament Textures and begin loading image files.
    pImpl->createTextures(asset, async);

    // Non-textured renderables are now considered ready, and we can guarantee that no new
    // materials or textures will be added. Notify the dependency graph.
    asset->mDependencyGraph.commitEdges();

    for (FFilamentInstance* instance : asset->mInstances) {
        instance->createAnimator();
    }

    return true;
}

bool ResourceLoader::asyncBeginLoad(FilamentAsset* asset) {
    pImpl->mAsyncAsset = downcast(asset);
    return loadResources(downcast(asset), true);
}

void ResourceLoader::asyncCancelLoad() {
    pImpl->cancelTextureDecoding();
    pImpl->mAsyncAsset = nullptr;
    pImpl->mEngine->flushAndWait();
}

void ResourceLoader::addTextureProvider(const char* mimeType, TextureProvider* provider) {
    pImpl->mTextureProviders[mimeType] = provider;
}

float ResourceLoader::asyncGetLoadProgress() const {
    if (pImpl->mTextureProviders.empty() || !pImpl->mAsyncAsset) {
        return 0;
    }
    size_t pushedCount = 0;
    size_t poppedCount = 0;
    for (const auto& iter : pImpl->mTextureProviders) {
        pushedCount += iter.second->getPushedCount();
        poppedCount += iter.second->getPoppedCount();
    }

    // Textures that haven't been fully downloaded are not yet pushed into one of the
    // decoding queues, so here we include them in the total "pending" count.
    const size_t pendingCount = pushedCount + pImpl->mRemainingTextureDownloads;

    return pendingCount == 0 ? 1 : (float(poppedCount) / pendingCount);
}

void ResourceLoader::asyncUpdateLoad() {
    if (!pImpl->mAsyncAsset) {
        return;
    }
    for (const auto& iter : pImpl->mTextureProviders) {
        iter.second->updateQueue();
        while (Texture* texture = iter.second->popTexture()) {
            pImpl->mAsyncAsset->mDependencyGraph.markAsReady(texture);
        }
    }
}

std::pair<Texture*, CacheResult> ResourceLoader::Impl::getOrCreateTexture(FFilamentAsset* asset,
        size_t textureIndex, TextureProvider::TextureFlags flags) {
    const cgltf_texture& srcTexture = asset->mSourceAsset->hierarchy->textures[textureIndex];
    const cgltf_image* image = srcTexture.basisu_image ?
            srcTexture.basisu_image : srcTexture.image;
    const cgltf_buffer_view* bv = image->buffer_view;
    const char* uri = image->uri;

    std::string mime = image->mime_type ? image->mime_type : "";
    size_t dataUriSize;
    const uint8_t* dataUriContent = uri ? parseDataUri(uri, &mime, &dataUriSize) : nullptr;

    if (mime.empty()) {
        assert_invariant(uri && "Non-URI images must supply a mime type.");
        const std::string extension = Path(uri).getExtension();
        mime = extension == "jpg" ? "image/jpeg" : "image/" + extension;
    }

    TextureProvider* provider = mTextureProviders[mime];
    if (!provider) {
        slog.e << "Missing texture provider for " << mime << io::endl;
        return {};
    }

    // Check if the texture slot uses BufferView data.
    if (void** bufferViewData = bv ? &bv->buffer->data : nullptr; bufferViewData) {
        assert_invariant(!dataUriContent);
        const size_t offset = bv ? bv->offset : 0;
        const uint8_t* sourceData = offset + (const uint8_t*) *bufferViewData;
        if (auto iter = mBufferTextureCache.find(sourceData); iter != mBufferTextureCache.end()) {
            return {iter->second, CacheResult::FOUND};
        }
        const uint32_t totalSize = uint32_t(bv ? bv->size : 0);
        if (Texture* texture = provider->pushTexture(sourceData, totalSize, mime.c_str(), flags); texture) {
            mBufferTextureCache[sourceData] = texture;
            return {texture, CacheResult::MISS};
        }
    }

    // Check if the texture slot is a data URI.
    // Note that this is a data URI in an image, not a buffer. Data URI's in buffers are decoded
    // by the cgltf_load_buffers() function.
    else if (dataUriContent) {
        if (auto iter = mBufferTextureCache.find(uri); iter != mBufferTextureCache.end()) {
            free((void*)dataUriContent);
            return {iter->second, CacheResult::FOUND};
        }
        if (Texture* texture = provider->pushTexture(dataUriContent, dataUriSize, mime.c_str(), flags); texture) {
            free((void*)dataUriContent);
            mBufferTextureCache[uri] = texture;
            return {texture, CacheResult::MISS};
        }
        free((void*)dataUriContent);
    }

    // Check the user-supplied resource cache for this URI.
    else if (auto iter = mUriDataCache.find(uri); iter != mUriDataCache.end()) {
        const uint8_t* sourceData = (const uint8_t*) iter->second.buffer;
        if (auto iter = mBufferTextureCache.find(sourceData); iter != mBufferTextureCache.end()) {
            return {iter->second, CacheResult::FOUND};
        }
        if (Texture* texture = provider->pushTexture(sourceData, iter->second.size, mime.c_str(), flags); texture) {
            mBufferTextureCache[sourceData] = texture;
            return {texture, CacheResult::MISS};
        }
    }

    // Finally, try the file system.
    else if constexpr (GLTFIO_USE_FILESYSTEM) {
        if (auto iter = mFilepathTextureCache.find(uri); iter != mFilepathTextureCache.end()) {
            return {iter->second, CacheResult::FOUND};
        }
        Path fullpath = Path(mGltfPath).getParent() + uri;
        if (!fullpath.exists()) {
            slog.e << "Unable to open " << fullpath << io::endl;
            return {};
        }
        using namespace std;
        ifstream filest(fullpath, std::ifstream::in | std::ifstream::binary);
        vector<uint8_t> buffer;
        filest.seekg(0, ios::end);
        buffer.reserve((size_t) filest.tellg());
        filest.seekg(0, ios::beg);
        buffer.assign((istreambuf_iterator<char>(filest)), istreambuf_iterator<char>());
        if (Texture* texture = provider->pushTexture(buffer.data(), buffer.size(), mime.c_str(), flags); texture) {
            mFilepathTextureCache[uri] = texture;
            return {texture, CacheResult::MISS};
        }

    } else {
        // If we reach here, the app has not yet called addResourceData() for this texture,
        // perhaps because it is still being downloaded.
        return {nullptr, CacheResult::NOT_READY};
    }

    const char* name = srcTexture.name ? srcTexture.name : uri;
    slog.e << "Unable to create texture " << name << ": " << provider->getPushMessage() << io::endl;
    return {};
}

void ResourceLoader::Impl::cancelTextureDecoding() {
    for (const auto& iter : mTextureProviders) {
        iter.second->cancelDecoding();
    }
    mAsyncAsset = nullptr;
}

void ResourceLoader::Impl::createTextures(FFilamentAsset* asset, bool async) {
    mRemainingTextureDownloads = 0;

    // Create new texture objects if they are not cached and kick off decoding jobs.
    for (size_t textureIndex = 0, n = asset->mTextures.size(); textureIndex < n; ++textureIndex) {
        FFilamentAsset::TextureInfo& info = asset->mTextures[textureIndex];
        auto [texture, cacheResult] = getOrCreateTexture(asset, textureIndex, info.flags);
        if (texture == nullptr) {
            if (cacheResult == CacheResult::NOT_READY) {
                mRemainingTextureDownloads++;
            }
            continue;
        }

        // If this cgtf_texture slot is being initialized, copy the Texture into the slot
        // and note if the Texture was created or re-used.
        if (info.texture == nullptr) {
            info.texture = texture;
            info.isOwner = cacheResult == CacheResult::MISS;
        }

        // For each binding to a material instance, call setParameter(...) on the material.
        for (const TextureSlot& slot : info.bindings) {
            asset->applyTextureBinding(textureIndex, slot);
        }
    }

    // Non-threaded systems are required to use the asynchronous API.
    assert_invariant(UTILS_HAS_THREADING || async);

    if (async) {
        return;
    }

    for (const auto& iter : mTextureProviders) {
        iter.second->waitForCompletion();
        iter.second->updateQueue();
    }
}

void ResourceLoader::Impl::computeTangents(FFilamentAsset* asset) {
    SYSTRACE_CALL();

    const cgltf_accessor* kGenerateTangents = &asset->mGenerateTangents;
    const cgltf_accessor* kGenerateNormals = &asset->mGenerateNormals;

    // Collect all TANGENT vertex attribute slots that need to be populated.
    tsl::robin_map<VertexBuffer*, uint8_t> baseTangents;
    for (auto slot : asset->mBufferSlots) {
        if (slot.accessor != kGenerateTangents && slot.accessor != kGenerateNormals) {
            continue;
        }
        baseTangents[slot.vertexBuffer] = slot.bufferIndex;
    }

    // Create a job description for each triangle-based primitive.
    using Params = TangentsJob::Params;
    std::vector<Params> jobParams;
    for (auto [prim, vb] : asset->mPrimitives) {
        if (UTILS_UNLIKELY(prim->type != cgltf_primitive_type_triangles)) {
            continue;
        }
        auto iter = baseTangents.find(vb);
        if (iter != baseTangents.end()) {
            jobParams.emplace_back(Params {{ prim }, {vb, nullptr, iter->second }});
        }
    }

    // Create a job description for morph targets.
    for (size_t i = 0, n = asset->mSourceAsset->hierarchy->meshes_count; i < n; ++i) {
        const cgltf_mesh& mesh = asset->mSourceAsset->hierarchy->meshes[i];
        const FixedCapacityVector<Primitive>& prims = asset->mMeshCache[i];
        if (0 == mesh.weights_count) {
            continue;
        }
        for (cgltf_size pindex = 0, pcount = mesh.primitives_count; pindex < pcount; ++pindex) {
            const cgltf_primitive& prim = mesh.primitives[pindex];
            MorphTargetBuffer* tb = prims[pindex].targets;
            for (cgltf_size tindex = 0, tcount = prim.targets_count; tindex < tcount; ++tindex) {
                const cgltf_morph_target& target = prim.targets[tindex];
                bool hasNormals = false;
                for (cgltf_size aindex = 0; aindex < target.attributes_count; aindex++) {
                    const cgltf_attribute& attribute = target.attributes[aindex];
                    const cgltf_attribute_type atype = attribute.type;
                    if (atype != cgltf_attribute_type_tangent) {
                        continue;
                    }
                    hasNormals = true;
                    jobParams.emplace_back(Params { { &prim, (int) tindex },
                                                    { nullptr, tb, (uint8_t) pindex } });
                    break;
                }
                // Generate flat normals if necessary.
                if (!hasNormals && !prim.material->unlit) {
                    jobParams.emplace_back(Params { { &prim, (int) tindex },
                                                    { nullptr, tb, (uint8_t) pindex } });
                }
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
        if (params.context.vb) {
            BufferObject* bo = BufferObject::Builder()
                    .size(params.out.vertexCount * sizeof(short4)).build(*mEngine);
            asset->mBufferObjects.push_back(bo);
            bo->setBuffer(*mEngine, BufferDescriptor(
                    params.out.results, bo->getByteCount(), FREE_CALLBACK));
            params.context.vb->setBufferObjectAt(*mEngine, params.context.slot, bo);
        } else {
            assert_invariant(params.context.tb);
            params.context.tb->setTangentsAt(*mEngine, params.in.morphTargetIndex,
                    params.out.results, params.out.vertexCount);
            free(params.out.results);
        }
    }
}

ResourceLoader::Impl::~Impl() {
    for (const auto& iter : mTextureProviders) {
        iter.second->cancelDecoding();
    }
}

void ResourceLoader::normalizeSkinningWeights(FFilamentAsset* asset) const {
    auto normalize = [](cgltf_accessor* data) {
        if (data->type != cgltf_type_vec4 || data->component_type != cgltf_component_type_r_32f) {
            slog.w << "Cannot normalize weights, unsupported attribute type." << io::endl;
            return;
        }
        uint8_t* bytes = (uint8_t*) data->buffer_view->buffer->data;
        bytes += data->offset + data->buffer_view->offset;
        for (cgltf_size i = 0, n = data->count; i < n; ++i, bytes += data->stride) {
            float4* weights = (float4*) bytes;
            const float sum = weights->x + weights->y + weights->z + weights->w;
            *weights /= sum;
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

} // namespace filament::gltfio
