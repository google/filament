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
#include "upcast.h"

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

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__) || defined(IOS)
#define USE_FILESYSTEM 0
#else
#define USE_FILESYSTEM 1
#endif

using namespace filament;
using namespace filament::math;
using namespace utils;

using filament::geometry::Transcoder;
using filament::geometry::ComponentType;

static const auto FREE_CALLBACK = [](void* mem, size_t, void*) { free(mem); };

namespace gltfio {

using BufferTextureCache = tsl::robin_map<const void*, Texture*>;
using FilepathTextureCache = tsl::robin_map<std::string, Texture*>;
using UriDataCache = tsl::robin_map<std::string, gltfio::ResourceLoader::BufferDescriptor>;
using TextureProviderList = tsl::robin_map<std::string, TextureProvider*>;

struct ResourceLoader::Impl {
    Impl(const ResourceConfiguration& config) {
        mGltfPath = std::string(config.gltfPath ? config.gltfPath : "");
        mEngine = config.engine;
        mNormalizeSkinningWeights = config.normalizeSkinningWeights;
        mRecomputeBoundingBoxes = config.recomputeBoundingBoxes;
        mIgnoreBindTransform = config.ignoreBindTransform;
    }

    Engine* mEngine;
    bool mNormalizeSkinningWeights;
    bool mRecomputeBoundingBoxes;
    bool mIgnoreBindTransform;
    std::string mGltfPath;

    // This is used to calculate skinIndex when updateBoundingBoxes, so that the mapping between
    // cgltf_node* and FFilamentInstance::Skin can be retrieved. This pointer doesn't need to be freed.
    cgltf_skin* mSkinBaseAddress;

    // User-provided resource data with URI string keys, populated with addResourceData().
    // This is used on platforms without traditional file systems, such as Android, iOS, and WebGL.
    UriDataCache mUriDataCache;

    // User-provided mapping from mime types to texture providers.
    TextureProviderList mTextureProviders;

    // Avoid duplicated Texture objects via caches with two key types: buffer pointers and strings.
    BufferTextureCache mBufferTextureCache;
    FilepathTextureCache mFilepathTextureCache;

    FFilamentAsset* mAsyncAsset = nullptr;

    void computeTangents(FFilamentAsset* asset);
    bool createTextures(FFilamentAsset* asset, bool async);
    void cancelTextureDecoding();
    Texture* getOrCreateTexture(FFilamentAsset* asset, const TextureSlot& tb);
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
            dstSkin.name = CString(srcSkin.name);
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

static ComponentType getComponentType(const cgltf_accessor* accessor) {
    switch (accessor->component_type) {
        case cgltf_component_type_r_8: return ComponentType::BYTE;
        case cgltf_component_type_r_8u: return ComponentType::UBYTE;
        case cgltf_component_type_r_16: return ComponentType::SHORT;
        case cgltf_component_type_r_16u: return ComponentType::USHORT;
        default:
            // This should be unreachable because other types do not require conversion.
            assert_invariant(false);
            return {};
    }
}

static void convertToFloats(float* dest, const cgltf_accessor* accessor) {
    const uint32_t dim = cgltf_num_components(accessor->type);
    const size_t floatsSize = accessor->count * sizeof(float) * dim;
    Transcoder transcode({
        .componentType = getComponentType(accessor),
        .normalized = bool(accessor->normalized),
        .componentCount = dim,
        .inputStrideBytes = uint32_t(accessor->stride)
    });
    auto bufferData = (const uint8_t*) accessor->buffer_view->buffer->data;
    const uint8_t* source = computeBindingOffset(accessor) + bufferData;
    transcode(dest, source, accessor->count);
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
    for (auto& pair : asset->mPrimitives) {
        const cgltf_primitive* prim = pair.first;
        if (!prim->has_draco_mesh_compression) {
            continue;
        }

        const cgltf_draco_mesh_compression& draco = prim->draco_mesh_compression;

        // If an error occurs, we can simply set the primitive's associated VertexBuffer to null.
        // This does not cause a leak because it is a weak reference.
        auto& vertexBuffer = pair.second;

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
                cgltf_result res = cgltf_load_buffer_base64(&options, gltf->buffers[i].size,
                        comma + 1, &gltf->buffers[i].data);
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
        // asset->mSkins is unused for instanced assets
        if (!pImpl->mIgnoreBindTransform) {
            pImpl->mIgnoreBindTransform = asset->isInstanced();
        }
        pImpl->mSkinBaseAddress = &gltf->skins[0];
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
            if (requiresConversion(accessor->type, accessor->component_type)) {
                const size_t dim = cgltf_num_components(accessor->type);
                const size_t floatsSize = accessor->count * sizeof(float) * dim;
                float* floatsData = (float*) malloc(floatsSize);
                convertToFloats(floatsData, accessor);
                BufferObject* bo = BufferObject::Builder().size(floatsSize).build(engine);
                asset->mBufferObjects.push_back(bo);
                bo->setBuffer(engine, BufferDescriptor(floatsData, floatsSize, FREE_CALLBACK));
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
        assert(slot.morphTargetBuffer);
        if (accessor->type == cgltf_type_vec3) {
            slot.morphTargetBuffer->setPositionsAt(engine, slot.bufferIndex,
                    (const float3*) data, slot.morphTargetBuffer->getVertexCount());
        } else {
            assert_invariant(accessor->type == cgltf_type_vec4);
            slot.morphTargetBuffer->setPositionsAt(engine, slot.bufferIndex,
                    (const float4*) data, slot.morphTargetBuffer->getVertexCount());
        }
    }

    // Apply sparse data modifications to base arrays, then upload the result.
    applySparseData(asset);

    // Compute surface orientation quaternions if necessary. This is similar to sparse data in that
    // we need to generate the contents of a GPU buffer by processing one or more CPU buffer(s).
    pImpl->computeTangents(asset);

    // Finally, create Filament Textures and begin loading image files.
    asset->mResourcesLoaded = pImpl->createTextures(asset, async);

    // Non-textured renderables are now considered ready, and we can guarantee that no new
    // materials or textures will be added. notify the dependency graph.
    asset->mDependencyGraph.finalize();

    asset->createAnimators();

    return asset->mResourcesLoaded;
}

bool ResourceLoader::asyncBeginLoad(FilamentAsset* asset) {
    pImpl->mAsyncAsset = upcast(asset);
    return loadResources(upcast(asset), true);
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
    return pushedCount == 0 ? 1 : (float(poppedCount) / pushedCount);
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

Texture* ResourceLoader::Impl::getOrCreateTexture(FFilamentAsset* asset, const TextureSlot& tb) {
    const cgltf_texture* srcTexture = tb.texture;
    const cgltf_image* image = srcTexture->image;
    const cgltf_buffer_view* bv = image->buffer_view;
    const char* uri = image->uri;

    TextureProvider::FlagBits flags = {};
    if (tb.srgb) {
        flags |= int(TextureProvider::Flags::sRGB);
    }

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
        asset->mDependencyGraph.markAsError(tb.materialInstance);
        return nullptr;
    }

    Texture* texture = nullptr;

    // Check if the texture slot uses BufferView data.
    if (void** bufferViewData = bv ? &bv->buffer->data : nullptr; bufferViewData) {
        assert_invariant(!dataUriContent);
        const size_t offset = bv ? bv->offset : 0;
        const uint8_t* sourceData = offset + (const uint8_t*) *bufferViewData;
        if (auto iter = mBufferTextureCache.find(sourceData); iter != mBufferTextureCache.end()) {
            return iter->second;
        }
        const uint32_t totalSize = uint32_t(bv ? bv->size : 0);
        if ((texture = provider->pushTexture(sourceData, totalSize, mime.c_str(), flags))) {
            mBufferTextureCache[sourceData] = texture;
        }
    }

    // Check if the texture slot is a data URI.
    else if (dataUriContent) {
        if (auto iter = mBufferTextureCache.find(uri); iter != mBufferTextureCache.end()) {
            free((void*)dataUriContent);
            return iter->second;
        }
        if ((texture = provider->pushTexture(dataUriContent, dataUriSize, mime.c_str(), flags))) {
            mBufferTextureCache[uri] = texture;
        }
        free((void*)dataUriContent);
    }

    // Check the user-supplied resource cache for this URI.
    else if (auto iter = mUriDataCache.find(uri); iter != mUriDataCache.end()) {
        const uint8_t* sourceData = (const uint8_t*) iter->second.buffer;
        if (auto iter = mBufferTextureCache.find(sourceData); iter != mBufferTextureCache.end()) {
            return iter->second;
        }
        if ((texture = provider->pushTexture(sourceData, iter->second.size, mime.c_str(), flags))) {
            mBufferTextureCache[sourceData] = texture;
        }
    }

    // Finally, try the file system.
    else if constexpr (USE_FILESYSTEM) {
        if (auto iter = mFilepathTextureCache.find(uri); iter != mFilepathTextureCache.end()) {
            return iter->second;
        }
        Path fullpath = Path(mGltfPath).getParent() + uri;
        if (!fullpath.exists()) {
            slog.e << "Unable to open " << fullpath << io::endl;
            asset->mDependencyGraph.markAsError(tb.materialInstance);
            return nullptr;
        }
        using namespace std;
        ifstream filest(fullpath);
        vector<uint8_t> buffer;
        filest.seekg(0, ios::end);
        buffer.reserve((size_t) filest.tellg());
        filest.seekg(0, ios::beg);
        buffer.assign((istreambuf_iterator<char>(filest)), istreambuf_iterator<char>());
        if ((texture = provider->pushTexture(buffer.data(), buffer.size(), mime.c_str(), flags))) {
            mFilepathTextureCache[uri] = texture;
        }

    // If the platform does not have a filesystem, emit an error and move on.
    } else {
        slog.e << "Unable to load " << uri << io::endl;
        asset->mDependencyGraph.markAsError(tb.materialInstance);
        return nullptr;
    }

    if (!texture) {
        const char* name = srcTexture->name ? srcTexture->name : uri;
        slog.e << "Unable to create texture " << name << ": "
                << provider->getPushMessage() << io::endl;
        asset->mDependencyGraph.markAsError(tb.materialInstance);
    } else {
        asset->takeOwnership(texture);
    }

    return texture;
}

void ResourceLoader::Impl::cancelTextureDecoding() {
    for (const auto& iter : mTextureProviders) {
        iter.second->cancelDecoding();
    }
    mAsyncAsset = nullptr;
}

bool ResourceLoader::Impl::createTextures(FFilamentAsset* asset, bool async) {
    // If any decoding jobs are still underway, wait for them to finish.
    for (const auto& iter : mTextureProviders) {
        iter.second->waitForCompletion();
        iter.second->updateQueue();
    }

    // Create new texture objects if they are not cached.
    for (auto slot : asset->mTextureSlots) {
        if (Texture* texture = getOrCreateTexture(asset, slot)) {
            asset->bindTexture(slot, texture);
        }
    }

    // Non-threaded systems are required to use the asynchronous API.
    assert_invariant(UTILS_HAS_THREADING || async);

    if (async) {
        return true;
    }

    for (const auto& iter : mTextureProviders) {
        iter.second->waitForCompletion();
        iter.second->updateQueue();
    }

    return true;
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
    for (auto pair : asset->mPrimitives) {
        if (UTILS_UNLIKELY(pair.first->type != cgltf_primitive_type_triangles)) {
            continue;
        }
        VertexBuffer* vb = pair.second;
        auto iter = baseTangents.find(vb);
        if (iter != baseTangents.end()) {
            jobParams.emplace_back(Params {{ pair.first }, {vb, nullptr, iter->second }});
        }
    }
    // Create a job description for morph targets.
    NodeMap& nodeMap = asset->isInstanced() ? asset->mInstances[0]->nodeMap : asset->mNodeMap;
    for (auto iter : nodeMap) {
        cgltf_node const* node = iter.first;
        cgltf_mesh const* mesh = node->mesh;
        if (UTILS_UNLIKELY(!mesh || !mesh->weights_count)) {
            continue;
        }
        cgltf_primitive const* prims = mesh->primitives;
        for (cgltf_size pindex = 0, pcount = mesh->primitives_count; pindex < pcount; ++pindex) {
            const cgltf_primitive& prim = mesh->primitives[pindex];
            const auto& gltfioPrim = asset->mMeshCache.at(mesh)[pindex];
            MorphTargetBuffer* tb = gltfioPrim.targets;
            for (cgltf_size tindex = 0, tcount = prim.targets_count; tindex < tcount; ++ tindex) {
                const cgltf_morph_target& target = prim.targets[tindex];
                bool hasNormals = false;
                for (cgltf_size aindex = 0; aindex < target.attributes_count; aindex++) {
                    const cgltf_attribute& attribute = target.attributes[aindex];
                    const cgltf_accessor* accessor = attribute.data;
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
        BufferObject* bo = BufferObject::Builder().size(numBytes).build(*asset->mEngine);
        asset->mBufferObjects.push_back(bo);
        bo->setBuffer(*pImpl->mEngine, BufferDescriptor(generated, numBytes, FREE_CALLBACK));
        slot.vertexBuffer->setBufferObjectAt(*pImpl->mEngine, slot.bufferIndex, bo);
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

    const size_t posAttrSize = cgltf_num_components(cgltf_type_vec3);
    const size_t skinningAttrSize = cgltf_num_components(cgltf_type_vec4);
    auto computeBoundingBoxSkinned = [&](const cgltf_primitive* prim, const Skin* skin, Aabb* result) {
        Aabb aabb;
        std::vector<mat4f> inverseGlobalTransforms(skin->targets.size());
        for (size_t i = 0; i < skin->targets.size(); i++) {
            auto xformable = tm.getInstance(skin->targets[i]);
            if (xformable) {
                inverseGlobalTransforms[i] = inverse(tm.getWorldTransform(xformable));
            }
        }
        std::vector<float> verts;
        std::vector<float> rawJoints;
        std::vector<float> weights;
        for (cgltf_size slot = 0; slot < prim->attributes_count; slot++) {
            const cgltf_attribute& attr = prim->attributes[slot];
            const cgltf_accessor* accessor = attr.data;
            if (attr.type == cgltf_attribute_type_position &&
                    cgltf_num_components(accessor->type) >= posAttrSize) {
                verts.resize(accessor->count * posAttrSize);
                cgltf_accessor_unpack_floats(accessor, &verts[0], accessor->count * posAttrSize);
            }
            if (attr.type == cgltf_attribute_type_joints &&
                    cgltf_num_components(accessor->type) >= skinningAttrSize) {
                rawJoints.resize(accessor->count * skinningAttrSize);
                cgltf_accessor_unpack_floats(accessor, &rawJoints[0], accessor->count * skinningAttrSize);
            }
            if (attr.type == cgltf_attribute_type_weights &&
                    cgltf_num_components(accessor->type) >= skinningAttrSize) {
                weights.resize(accessor->count * skinningAttrSize);
                cgltf_accessor_unpack_floats(accessor, &weights[0], accessor->count * skinningAttrSize);
            }
        }
        std::vector<size_t> jointIndices(rawJoints.begin(), rawJoints.end());
        auto primitiveCount = static_cast<size_t>(verts.size() / posAttrSize);
        for (size_t i = 0; i < primitiveCount; i++) {
            float3 point(verts[posAttrSize * i], verts[posAttrSize * i + 1], verts[posAttrSize * i + 2]);
            mat4f tmp = mat4f(0.0f);
            for (size_t j = 0; j < skinningAttrSize; j++) {
                size_t jointIndex = jointIndices[skinningAttrSize * i + j];
                float jointWeight = weights[skinningAttrSize * i + j];
                Entity jointEntity = skin->joints[jointIndex];
                mat4f globalJointTransform = tm.getWorldTransform(tm.getInstance(jointEntity));
                mat4f inverseBindMatrix = skin->inverseBindMatrices[jointIndex];
                tmp += jointWeight * globalJointTransform * inverseBindMatrix;
            }
            for (const auto& inverseGlobalTransform: inverseGlobalTransforms) {
                mat4f skinMatrix = inverseGlobalTransform * tmp;
                if (!pImpl->mNormalizeSkinningWeights) {
                    skinMatrix /= skinMatrix[3].w;
                }
                float3 skinnedPoint = (point.x * skinMatrix[0] +
                        point.y * skinMatrix[1] + point.z * skinMatrix[2] + skinMatrix[3]).xyz;
                aabb.min = min(aabb.min, skinnedPoint);
                aabb.max = max(aabb.max, skinnedPoint);
            }
        }
        *result = aabb;
    };

    // Collect all mesh primitives that we wish to find bounds for. For each mesh primitive, we also
    // collect the skin it bound to (nullptr if not skinned) for bounds computation.
    std::vector<std::pair<cgltf_primitive*, const Skin*>> primitives;
    for (auto iter : nodeMap) {
        const Skin* skin = nullptr;
        cgltf_skin* const cgltfSkin = iter.first->skin;
        if (cgltfSkin) {
            // importSkins unpacked cgltfSkin into FFilamentInstance::SkinVector bijectively so that
            // the unpacked Skin can be retrieved given cgltfSkin index
            int skinIndex = cgltfSkin - pImpl->mSkinBaseAddress;
            skin = &asset->mSkins[skinIndex];
        }
        const cgltf_mesh* cgltfMesh = iter.first->mesh;
        if (cgltfMesh) {
            for (cgltf_size index = 0, nprims = cgltfMesh->primitives_count; index < nprims; ++index) {
                primitives.push_back({&cgltfMesh->primitives[index], skin});
            }
        }
    }

    // Kick off a bounding box job for every primitive.
    std::vector<Aabb> bounds(primitives.size());
    JobSystem* js = &pImpl->mEngine->getJobSystem();
    JobSystem::Job* parent = js->createJob();
    for (size_t i = 0; i < primitives.size(); ++i) {
        Aabb* result = &bounds[i];
        if (pImpl->mIgnoreBindTransform || !primitives[i].second) {
            cgltf_primitive const* prim = primitives[i].first;
            js->run(jobs::createJob(*js, parent, [prim, result, computeBoundingBox] {
                computeBoundingBox(prim, result);
            }));
        } else {
            std::pair<cgltf_primitive*, const Skin*> skinnedPrimitive = primitives[i];
            js->run(jobs::createJob(*js, parent, [skinnedPrimitive, result, computeBoundingBoxSkinned] {
                computeBoundingBoxSkinned(skinnedPrimitive.first, skinnedPrimitive.second, result);
            }));
        }
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
