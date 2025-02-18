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

/*
 * JS BINDINGS DESIGN
 * ------------------
 *
 * The purpose of filament-js is to offer a first-class JavaScript interface to the core Filament
 * classes: Engine, Renderer, Texture, etc.
 *
 * Emscripten offers two ways to binding JavaScript to C++: embind and WebIDL. We chose embind.
 *
 * With WebIDL, we would need to author WebIDL files and generate opaque C++ from the IDL, which
 * complicates the build process and ultimately results in the same amount of code. Using embind is
 * more direct and controllable.
 *
 * For nested classes, we use $ as the separator character because embind does not support nested
 * classes and it would transform dot separators into $ anyway. By using $, we at least make
 * this explicit rather than mysterious.
 */

#include <filameshio/MeshReader.h>

#include <filament/BufferObject.h>
#include <filament/Camera.h>
#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/Frustum.h>
#include <filament/IndexBuffer.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/RenderTarget.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/SwapChain.h>
#include <filament/Texture.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <geometry/SurfaceOrientation.h>

#include <viewer/ViewerGui.h>

#include <gltfio/Animator.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/FilamentInstance.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <materials/uberarchive.h>

#include <ktxreader/Ktx1Reader.h>
#include <ktxreader/Ktx2Reader.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat4.h>

#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>
#include <utils/Log.h>

#include <stb_image.h>

#include <emscripten.h>
#include <emscripten/bind.h>

// Avoid warnings for deprecated Filament APIs.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

using namespace emscripten;
using namespace filament;
using namespace filamesh;
using namespace geometry;
using namespace filament::gltfio;
using namespace image;
using namespace ktxreader;

using namespace filament::viewer;

namespace em = emscripten;

#if __has_feature(cxx_rtti)
#error Filament JS bindings require RTTI to be disabled.
#endif

// Many methods require a thin layer of C++ glue which is elegantly expressed with a lambda.
// However, passing a bare lambda into embind's daisy chain requires a cast to a function pointer.
#define EMBIND_LAMBDA(retval, arglist, impl) (retval (*) arglist) [] arglist impl

// Builder functions that return "this" have verbose binding declarations, this macro reduces
// the amount of boilerplate.
#define BUILDER_FUNCTION(name, btype, arglist, impl) \
        function(name, EMBIND_LAMBDA(btype*, arglist, impl), allow_raw_pointers())

// Explicit instantiation of emscripten::internal::raw_destructor is required for binding classes
// that have non-public destructors. To prevent leaks, do not include pass-by-value types here.
#define BIND(T) template<> void raw_destructor<T>(T* ptr) {}
namespace emscripten {
    namespace internal {
        BIND(Animator)
        BIND(AssetLoader)
        BIND(BufferObject)
        BIND(Camera)
        BIND(ColorGrading)
        BIND(Engine)
        BIND(FilamentAsset)
        BIND(FilamentInstance)
        BIND(IndexBuffer)
        BIND(IndirectLight)
        BIND(LightManager)
        BIND(Material)
        BIND(MaterialInstance)
        BIND(RenderableManager)
        BIND(Renderer)
        BIND(RenderTarget)
        BIND(Scene)
        BIND(Skybox)
        BIND(SwapChain)
        BIND(Texture)
        BIND(TransformManager)
        BIND(utils::EntityManager)
        BIND(VertexBuffer)
        BIND(View)
    }
}
#undef BIND

namespace {

// For convenience, declare terse private aliases to nested types. This lets us avoid extremely
// verbose binding declarations.
using ColorBuilder = ColorGrading::Builder;
using IblBuilder = IndirectLight::Builder;
using IndexBuilder = IndexBuffer::Builder;
using LightBuilder = LightManager::Builder;
using MatBuilder = Material::Builder;
using RenderableBuilder = RenderableManager::Builder;
using RenderTargetBuilder = RenderTarget::Builder;
using SkyBuilder = Skybox::Builder;
using SurfaceBuilder = SurfaceOrientation::Builder;
using TexBuilder = Texture::Builder;
using VertexBuilder = VertexBuffer::Builder;
using BufferBuilder = BufferObject::Builder;

// We avoid directly exposing backend::BufferDescriptor because embind does not support move
// semantics and void* doesn't make sense to JavaScript anyway. This little wrapper class is exposed
// to JavaScript as "driver$BufferDescriptor", but clients will normally use our "Filament.Buffer"
// helper function (implemented in utilities.js)
struct BufferDescriptor {
    BufferDescriptor() {}
    // This form is used when JavaScript sends a buffer into WASM.
    BufferDescriptor(uint32_t byteLength) {
        this->bd.reset(new backend::BufferDescriptor(malloc(byteLength), byteLength,
                [](void* buffer, size_t size, void* user) { free(buffer); }));
    }
    // This form is used when WASM needs to return a buffer to JavaScript.
    BufferDescriptor(uint8_t* data, uint32_t size) {
        this->bd.reset(new backend::BufferDescriptor(data, size));
    }
    val getBytes() {
        unsigned char *byteBuffer = (unsigned char*) bd->buffer;
        size_t bufferLength = bd->size;
        return val(typed_memory_view(bufferLength, byteBuffer));
    }
    // In order to match its JavaScript counterpart, the Buffer wrapper needs to use reference
    // counting, and the easiest way to achieve that is with shared_ptr.
    std::shared_ptr<backend::BufferDescriptor> bd;
};

// Exposed to JavaScript as "driver$PixelBufferDescriptor", but clients will normally use the
// PixelBuffer or CompressedPixelBuffer helper functions (implemented in utilities.js)
struct PixelBufferDescriptor {
    PixelBufferDescriptor(uint32_t byteLength, backend::PixelDataFormat fmt, backend::PixelDataType dtype) {
        this->pbd.reset(new backend::PixelBufferDescriptor(malloc(byteLength), byteLength,
                fmt, dtype, [](void* buffer, size_t size, void* user) { free(buffer); }));
    }
    // Note that embind allows overloading based on number of arguments, but not on types.
    // It's fine to have two constructors but they can't both have the same number of arguments.
    PixelBufferDescriptor(uint32_t byteLength, backend::CompressedPixelDataType cdtype, int imageSize,
            bool compressed) {
        assert(compressed == true);
        // For compressed cubemaps, the image size should be one-sixth the size of the entire blob.
        assert(imageSize == byteLength || imageSize == byteLength / 6);
        this->pbd.reset(new backend::PixelBufferDescriptor(malloc(byteLength), byteLength,
                cdtype, imageSize, [](void* buffer, size_t size, void* user) { free(buffer); }));
    }
    val getBytes() {
        unsigned char* byteBuffer = (unsigned char*) pbd->buffer;
        size_t bufferLength = pbd->size;
        return val(typed_memory_view(bufferLength, byteBuffer));
    }
    // In order to match its JavaScript counterpart, the Buffer wrapper needs to use reference
    // counting, and the easiest way to achieve that is with shared_ptr.
    std::shared_ptr<backend::PixelBufferDescriptor> pbd;
};

// Small structure whose sole purpose is to return decoded image data to JavaScript.
struct DecodedImage {
    int width;
    int height;
    int encoded_ncomp;
    int decoded_ncomp;
    BufferDescriptor decoded_data;
};

// JavaScript clients should call [createTextureFromPng] rather than calling this directly.
DecodedImage decodeImage(BufferDescriptor encoded_data, int requested_ncomp) {
    DecodedImage result;
    stbi_uc* decoded_data = stbi_load_from_memory(
            (stbi_uc const *) encoded_data.bd->buffer,
            encoded_data.bd->size,
            &result.width,
            &result.height,
            &result.encoded_ncomp,
            requested_ncomp);
    const uint32_t decoded_size = result.width * result.height * requested_ncomp;
    result.decoded_data = BufferDescriptor(decoded_data, decoded_size);
    result.decoded_data.bd->setCallback([](void* buffer, size_t size, void* user) {
        stbi_image_free(buffer);
    });
    result.decoded_ncomp = requested_ncomp;
    return result;
}

} // anonymous namespace

EMSCRIPTEN_BINDINGS(jsbindings) {

// MATH TYPES
// ----------
// Individual JavaScript objects for math types would be too heavy, so instead we simply accept
// array-like data using embind's "value_array" feature. We do not expose all our math functions
// under the assumption that JS clients will use glMatrix or something similar for math.

value_array<filament::math::float2>("float2")
    .element(&filament::math::float2::x)
    .element(&filament::math::float2::y);

value_array<filament::math::float3>("float3")
    .element(&filament::math::float3::x)
    .element(&filament::math::float3::y)
    .element(&filament::math::float3::z);

value_array<filament::math::float4>("float4")
    .element(&filament::math::float4::x)
    .element(&filament::math::float4::y)
    .element(&filament::math::float4::z)
    .element(&filament::math::float4::w);

value_array<filament::math::double2>("double2")
    .element(&filament::math::double2::x)
    .element(&filament::math::double2::y);

value_array<filament::math::double3>("double3")
    .element(&filament::math::double3::x)
    .element(&filament::math::double3::y)
    .element(&filament::math::double3::z);

value_array<filament::math::double4>("double4")
    .element(&filament::math::double4::x)
    .element(&filament::math::double4::y)
    .element(&filament::math::double4::z)
    .element(&filament::math::double4::w);

value_array<filament::math::quat>("quat")
    .element(&filament::math::quat::x)
    .element(&filament::math::quat::y)
    .element(&filament::math::quat::z)
    .element(&filament::math::quat::w);

value_array<Viewport>("Viewport")
    .element(&Viewport::left)
    .element(&Viewport::bottom)
    .element(&Viewport::width)
    .element(&Viewport::height);

value_array<KtxBlobIndex>("KtxBlobIndex")
    .element(&KtxBlobIndex::mipLevel)
    .element(&KtxBlobIndex::arrayIndex)
    .element(&KtxBlobIndex::cubeFace);

value_object<Box>("Box")
    .field("center", &Box::center)
    .field("halfExtent", &Box::halfExtent);

value_object<filament::Aabb>("Aabb")
    .field("min", &filament::Aabb::min)
    .field("max", &filament::Aabb::max);

value_object<filament::Renderer::ClearOptions>("Renderer$ClearOptions")
    .field("clearColor", &filament::Renderer::ClearOptions::clearColor)
    .field("clear", &filament::Renderer::ClearOptions::clear)
    .field("discard", &filament::Renderer::ClearOptions::discard);

value_object<LightManager::ShadowOptions>("LightManager$ShadowOptions")
    .field("mapSize", &LightManager::ShadowOptions::mapSize)
    .field("shadowCascades", &LightManager::ShadowOptions::shadowCascades)
    .field("constantBias", &LightManager::ShadowOptions::constantBias)
    .field("normalBias", &LightManager::ShadowOptions::normalBias)
    .field("shadowFar", &LightManager::ShadowOptions::shadowFar)
    .field("shadowNearHint", &LightManager::ShadowOptions::shadowNearHint)
    .field("shadowFarHint", &LightManager::ShadowOptions::shadowFarHint)
    .field("stable", &LightManager::ShadowOptions::stable)
    .field("lispsm", &LightManager::ShadowOptions::lispsm)
    .field("polygonOffsetConstant", &LightManager::ShadowOptions::polygonOffsetConstant)
    .field("polygonOffsetSlope", &LightManager::ShadowOptions::polygonOffsetSlope)
    .field("screenSpaceContactShadows", &LightManager::ShadowOptions::screenSpaceContactShadows)
    .field("stepCount", &LightManager::ShadowOptions::stepCount)
    .field("maxShadowDistance", &LightManager::ShadowOptions::maxShadowDistance)
    .field("shadowBulbRadius", &LightManager::ShadowOptions::shadowBulbRadius)
    .field("transform", &LightManager::ShadowOptions::transform);

// In JavaScript, a flat contiguous representation is best for matrices (see gl-matrix) so we
// need to define a small wrapper here.

struct flatmat4 {
    filament::math::mat4f m;
    float& operator[](int i) { return m[i / 4][i % 4]; }
};

value_array<flatmat4>("mat4")
    .element(em::index< 0>()).element(em::index< 1>()).element(em::index< 2>()).element(em::index< 3>())
    .element(em::index< 4>()).element(em::index< 5>()).element(em::index< 6>()).element(em::index< 7>())
    .element(em::index< 8>()).element(em::index< 9>()).element(em::index<10>()).element(em::index<11>())
    .element(em::index<12>()).element(em::index<13>()).element(em::index<14>()).element(em::index<15>());

struct flatmat3 {
    filament::math::mat3f m;
    float& operator[](int i) { return m[i / 3][i % 3]; }
};

value_array<flatmat3>("mat3")
    .element(em::index<0>()).element(em::index<1>()).element(em::index<2>())
    .element(em::index<3>()).element(em::index<4>()).element(em::index<5>())
    .element(em::index<6>()).element(em::index<7>()).element(em::index<8>());

value_object<RenderableManager::Bone>("RenderableManager$Bone")
    .field("unitQuaternion", &RenderableManager::Bone::unitQuaternion)
    .field("translation", &RenderableManager::Bone::translation);

// VECTOR TYPES
// ------------

using EntityVector = std::vector<utils::Entity>;

register_vector<std::string>("RegistryKeys");
register_vector<utils::Entity>("EntityVector");
register_vector<allow_raw_pointer<FilamentInstance*>>("AssetInstanceVector");
register_vector<allow_raw_pointer<MaterialInstance*>>("MaterialInstanceVector");

// CORE FILAMENT CLASSES
// ---------------------

/// Engine ::core class:: Central manager and resource owner.
class_<Engine>("Engine")
    .class_function("_create", (Engine* (*)()) [] {
        EM_ASM_INT({
            const options = window.filament_glOptions;
            const context = window.filament_glContext;
            const handle = GL.registerContext(context, options);
            window.filament_contextHandle = handle;
            GL.makeContextCurrent(handle);
        });
        return Engine::create();
    }, allow_raw_pointers())

    .class_function("getSteadyClockTimeNano", &Engine::getSteadyClockTimeNano)

    .function("unprotected", &Engine::unprotected)

    .function("enableAccurateTranslations", &Engine::enableAccurateTranslations)

    .function("setAutomaticInstancingEnabled", &Engine::setAutomaticInstancingEnabled)

    .function("isAutomaticInstancingEnabled", &Engine::isAutomaticInstancingEnabled)

    .function("getSupportedFeatureLevel", &Engine::getSupportedFeatureLevel)

    .function("setActiveFeatureLevel", &Engine::setActiveFeatureLevel)

    .function("getActiveFeatureLevel", &Engine::getActiveFeatureLevel)

    .class_function("getMaxStereoscopicEyes", &Engine::getMaxStereoscopicEyes)

    .function("_execute", EMBIND_LAMBDA(void, (Engine* engine), {
        EM_ASM_INT({
            const handle = window.filament_contextHandle;
            GL.makeContextCurrent(handle);
        });
        engine->execute();
    }), allow_raw_pointers())

    /// destroy ::static method:: Destroys an engine instance and cleans up resources.
    /// engine ::argument:: the instance to destroy
    .class_function("destroy", (void (*)(Engine*)) []
            (Engine* engine) { Engine::destroy(&engine); }, allow_raw_pointers())

    /// getTransformManager ::method::
    /// ::retval:: an instance of [TransformManager]
    .function("getTransformManager", EMBIND_LAMBDA(TransformManager*, (Engine* engine), {
        return &engine->getTransformManager();
    }), allow_raw_pointers())

    /// getLightManager ::method::
    /// ::retval:: an instance of [LightManager]
    .function("getLightManager", EMBIND_LAMBDA(LightManager*, (Engine* engine), {
        return &engine->getLightManager();
    }), allow_raw_pointers())

    /// getRenderableManager ::method::
    /// ::retval:: an instance of [RenderableManager]
    .function("getRenderableManager", EMBIND_LAMBDA(RenderableManager*, (Engine* engine), {
        return &engine->getRenderableManager();
    }), allow_raw_pointers())

    /// getEntityManager ::method::
    /// ::retval:: an instance of [utils::EntityManager]
    .function("getEntityManager", EMBIND_LAMBDA(utils::EntityManager*, (Engine* engine), {
        return &engine->getEntityManager();
    }), allow_raw_pointers())

    /// createSwapChain ::method::
    /// ::retval:: an instance of [SwapChain]
    .function("createSwapChain", (SwapChain* (*)(Engine*)) []
            (Engine* engine) { return engine->createSwapChain(nullptr); },
            allow_raw_pointers())
    /// destroySwapChain ::method::
    /// swapChain ::argument:: an instance of [SwapChain]
    .function("destroySwapChain", (void (*)(Engine*, SwapChain*)) []
            (Engine* engine, SwapChain* swapChain) { engine->destroy(swapChain); },
            allow_raw_pointers())

    /// createRenderer ::method::
    /// ::retval:: an instance of [Renderer]
    .function("createRenderer", &Engine::createRenderer, allow_raw_pointers())
    /// destroyRenderer ::method::
    /// renderer ::argument:: an instance of [Renderer]
    .function("destroyRenderer", (void (*)(Engine*, Renderer*)) []
            (Engine* engine, Renderer* renderer) { engine->destroy(renderer); },
            allow_raw_pointers())

    /// createView ::method::
    /// ::retval:: an instance of [View]
    .function("createView", &Engine::createView, allow_raw_pointers())
    /// destroyView ::method::
    /// view ::argument:: an instance of [View]
    .function("destroyView", (void (*)(Engine*, View*)) []
            (Engine* engine, View* view) { engine->destroy(view); },
            allow_raw_pointers())

    /// createScene ::method::
    /// ::retval:: an instance of [Scene]
    .function("createScene", &Engine::createScene, allow_raw_pointers())
    /// destroyScene ::method::
    /// scene ::argument:: an instance of [Scene]
    .function("destroyScene", (void (*)(Engine*, Scene*)) []
            (Engine* engine, Scene* scene) { engine->destroy(scene); },
            allow_raw_pointers())

    /// createCamera ::method::
    /// entity ::argument:: the [Entity] to add the camera component to
    /// ::retval:: an instance of [Camera]
    .function("createCamera", select_overload<Camera*(utils::Entity entity)>(&Engine::createCamera),
            allow_raw_pointers())
    /// getCameraComponent ::method::
    /// ::retval:: an instance of [Camera]
    .function("getCameraComponent", &Engine::getCameraComponent, allow_raw_pointers())
    /// destroyCameraComponent ::method::
    /// camera ::argument:: an [Entity] with a camera component
    .function("destroyCameraComponent", (void (*)(Engine*, utils::Entity)) []
            (Engine* engine, utils::Entity camera) { engine->destroyCameraComponent(camera); },
            allow_raw_pointers())

    .function("_createMaterial", EMBIND_LAMBDA(Material*, (Engine* engine, BufferDescriptor mbd), {
        return Material::Builder().package(mbd.bd->buffer, mbd.bd->size).build(*engine);
    }), allow_raw_pointers())
    /// destroyMaterial ::method::
    /// material ::argument:: an instance of [Material]
    .function("destroyMaterial", (void (*)(Engine*, Material*)) []
            (Engine* engine, Material* mat) { engine->destroy(mat); },
            allow_raw_pointers())

    /// destroyEntity ::method::
    /// entity ::argument:: an [Entity]
    .function("destroyEntity", (void (*)(Engine*, utils::Entity)) []
            (Engine* engine, utils::Entity entity) { engine->destroy(entity); },
            allow_raw_pointers())
    /// destroyColorGrading ::method::
    /// entity ::argument:: an [ColorGrading]
    .function("destroyColorGrading", (void (*)(Engine*, ColorGrading*)) []
            (Engine* engine, ColorGrading* colorGrading) { engine->destroy(colorGrading); },
            allow_raw_pointers())
    /// destroyIndexBuffer ::method::
    /// ib ::argument:: the [IndexBuffer] to destroy
    .function("destroyIndexBuffer", (void (*)(Engine*, IndexBuffer*)) []
            (Engine* engine, IndexBuffer* ib) { engine->destroy(ib); },
            allow_raw_pointers())
    /// destroyIndirectLight ::method::
    /// light ::argument:: the [IndirectLight] to destroy
    .function("destroyIndirectLight", (void (*)(Engine*, IndirectLight*)) []
            (Engine* engine, IndirectLight* light) { engine->destroy(light); },
            allow_raw_pointers())
    /// destroyMaterial ::method::
    /// instance ::argument:: the [MaterialInstance] to destroy
    .function("destroyMaterialInstance", (void (*)(Engine*, MaterialInstance*)) []
            (Engine* engine, MaterialInstance* mi) { engine->destroy(mi); },
            allow_raw_pointers())
    /// destroyRenderTarget ::method::
    /// rt ::argument:: the [RenderTarget] to destroy
    .function("destroyRenderTarget", (void (*)(Engine*, RenderTarget*)) []
            (Engine* engine, RenderTarget* rt) { engine->destroy(rt); },
            allow_raw_pointers())
    /// destroySkybox ::method::
    /// skybox ::argument:: the [Skybox] to destroy
    .function("destroySkybox", (void (*)(Engine*, Skybox*)) []
            (Engine* engine, Skybox* sky) { engine->destroy(sky); },
            allow_raw_pointers())
    /// destroyTexture ::method::
    /// texture ::argument:: the [Texture] to destroy
    .function("destroyTexture", (void (*)(Engine*, Texture*)) []
            (Engine* engine, Texture* tex) { engine->destroy(tex); },
            allow_raw_pointers())
    /// destroyVertexBuffer ::method::
    /// vb ::argument:: the [VertexBuffer] to destroy
    .function("destroyVertexBuffer", (void (*)(Engine*, VertexBuffer*)) []
            (Engine* engine, VertexBuffer* vb) { engine->destroy(vb); },
            allow_raw_pointers())

    .function("isValidRenderer", EMBIND_LAMBDA(bool, (Engine* engine, Renderer* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidView", EMBIND_LAMBDA(bool, (Engine* engine, View* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidScene", EMBIND_LAMBDA(bool, (Engine* engine, Scene* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidFence", EMBIND_LAMBDA(bool, (Engine* engine, Fence* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidStream", EMBIND_LAMBDA(bool, (Engine* engine, Stream* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidIndexBuffer", EMBIND_LAMBDA(bool, (Engine* engine, IndexBuffer* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidVertexBuffer", EMBIND_LAMBDA(bool, (Engine* engine, VertexBuffer* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidSkinningBuffer", EMBIND_LAMBDA(bool, (Engine* engine, SkinningBuffer* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidIndirectLight", EMBIND_LAMBDA(bool, (Engine* engine, IndirectLight* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidMaterial", EMBIND_LAMBDA(bool, (Engine* engine, Material* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidMaterialInstance", EMBIND_LAMBDA(bool, (Engine* engine, Material* ma, MaterialInstance* mi), {
                return engine->isValid(ma, mi);
            }), allow_raw_pointers())
    .function("isValidExpensiveMaterialInstance", EMBIND_LAMBDA(bool, (Engine* engine, MaterialInstance* object), {
                return engine->isValidExpensive(object);
            }), allow_raw_pointers())
    .function("isValidSkybox", EMBIND_LAMBDA(bool, (Engine* engine, Skybox* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidColorGrading", EMBIND_LAMBDA(bool, (Engine* engine, ColorGrading* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidTexture", EMBIND_LAMBDA(bool, (Engine* engine, Texture* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidRenderTarget", EMBIND_LAMBDA(bool, (Engine* engine, RenderTarget* object), {
                return engine->isValid(object);
            }), allow_raw_pointers())
    .function("isValidSwapChain", EMBIND_LAMBDA(bool, (Engine* engine, SwapChain* object), {
                return engine->isValid(object);
            }), allow_raw_pointers());

/// SwapChain ::core class:: Represents the platform's native rendering surface.
/// See also the [Engine] methods `createSwapChain` and `destroySwapChain`.
class_<SwapChain>("SwapChain")
        .class_function("isSRGBSwapChainSupported", &SwapChain::isSRGBSwapChainSupported);

/// Renderer ::core class:: Represents the platform's native window.
/// See also the [Engine] methods `createRenderer` and `destroyRenderer`.
class_<Renderer>("Renderer")
    .function("renderView", &Renderer::render, allow_raw_pointers())
    .function("renderStandaloneView", &Renderer::renderStandaloneView, allow_raw_pointers())
    /// render ::method:: requests rendering for a single frame on the given [View]
    /// swapChain ::argument:: the [SwapChain] corresponding to the canvas
    /// view ::argument:: the [View] corresponding to the canvas
    .function("render", EMBIND_LAMBDA(void, (Renderer* self, SwapChain* swapChain, View* view), {
        auto engine = self->getEngine();
        if (self->beginFrame(swapChain)) {
            self->render(view);
            self->endFrame();
        }
        engine->execute();
    }), allow_raw_pointers())
    .function("_setClearOptions", &Renderer::setClearOptions, allow_raw_pointers())
    .function("getClearOptions", &Renderer::getClearOptions)
    .function("setPresentationTime", &Renderer::setPresentationTime)
    .function("setVsyncTime", &Renderer::setVsyncTime)
    .function("skipFrame", &Renderer::skipFrame)
    .function("beginFrame", EMBIND_LAMBDA(bool, (Renderer* self, SwapChain* swapChain), {
        return self->beginFrame(swapChain);
    }), allow_raw_pointers())
    .function("endFrame", &Renderer::endFrame, allow_raw_pointers());

/// View ::core class:: Encompasses all the state needed for rendering a Scene.
/// A view is associated with a particular [Scene], [Camera], and viewport.
/// See also the [Engine] methods `createView` and `destroyView`.
class_<View>("View")
    .function("pick", EMBIND_LAMBDA(void, (View* self, uint32_t x, uint32_t y, val cb), {
        self->pick(x, y, [cb](const View::PickingQueryResult& result) {
            EM_ASM_ARGS({
                const fn = Emval.toValue($0);
                fn({
                    "renderable": Emval.toValue($1),
                    "depth": $2,
                    "fragCoords": [$3, $4, $5],
                });
            }, cb.as_handle(), val(result.renderable).as_handle(), result.depth,
                result.fragCoords.x, result.fragCoords.y, result.fragCoords.z);
        });
    }), allow_raw_pointers())

    .function("setScene", &View::setScene, allow_raw_pointers())
    .function("setCamera", &View::setCamera, allow_raw_pointers())
    .function("hasCamera", &View::hasCamera)
    .function("setColorGrading", &View::setColorGrading, allow_raw_pointers())
    .function("setBlendMode", &View::setBlendMode)
    .function("getBlendMode", &View::getBlendMode)
    .function("setViewport", &View::setViewport)
    .function("getViewport", &View::getViewport)
    .function("setVisibleLayers", &View::setVisibleLayers)
    .function("setPostProcessingEnabled", &View::setPostProcessingEnabled)
    .function("setDithering", &View::setDithering)
    .function("_setAmbientOcclusionOptions", &View::setAmbientOcclusionOptions)
    .function("_setDepthOfFieldOptions", &View::setDepthOfFieldOptions)
    .function("_setMultiSampleAntiAliasingOptions", &View::setMultiSampleAntiAliasingOptions)
    .function("_setTemporalAntiAliasingOptions", &View::setTemporalAntiAliasingOptions)
    .function("_setScreenSpaceReflectionsOptions", &View::setScreenSpaceReflectionsOptions)
    .function("_setBloomOptions", &View::setBloomOptions)
    .function("_setFogOptions", &View::setFogOptions)
    .function("_setVignetteOptions", &View::setVignetteOptions)
    .function("_setGuardBandOptions", &View::setGuardBandOptions)
    .function("_setStereoscopicOptions", &View::setStereoscopicOptions)
    .function("setAmbientOcclusion", &View::setAmbientOcclusion)
    .function("getAmbientOcclusion", &View::getAmbientOcclusion)
    .function("setAntiAliasing", &View::setAntiAliasing)
    .function("getAntiAliasing", &View::getAntiAliasing)
    .function("setSampleCount", &View::setSampleCount)
    .function("getSampleCount", &View::getSampleCount)
    .function("setRenderTarget", EMBIND_LAMBDA(void, (View* self, RenderTarget* renderTarget), {
        self->setRenderTarget(renderTarget);
    }), allow_raw_pointers())
    .function("setTransparentPickingEnabled", &View::setTransparentPickingEnabled)
    .function("isTransparentPickingEnabled", &View::isTransparentPickingEnabled)
    .function("setStencilBufferEnabled", &View::setStencilBufferEnabled)
    .function("isStencilBufferEnabled", &View::isStencilBufferEnabled)
    .function("setMaterialGlobal", &View::setMaterialGlobal)
    .function("getMaterialGlobal", &View::getMaterialGlobal)
    .function("getFogEntity", &View::getFogEntity)
    .function("clearFrameHistory", &View::clearFrameHistory);

/// Scene ::core class:: Flat container of renderables and lights.
/// See also the [Engine] methods `createScene` and `destroyScene`.
class_<Scene>("Scene")
    .function("addEntity", &Scene::addEntity)

    .function("_addEntities", EMBIND_LAMBDA(void, (Scene* self, EntityVector entities), {
        self->addEntities(entities.data(), entities.size());
    }), allow_raw_pointers())

    .function("_removeEntities", EMBIND_LAMBDA(void, (Scene* self, EntityVector entities), {
        self->removeEntities(entities.data(), entities.size());
    }), allow_raw_pointers())

    .function("hasEntity", &Scene::hasEntity)
    .function("remove", &Scene::remove)
    .function("setSkybox", &Scene::setSkybox, allow_raw_pointers())
    .function("getSkybox", &Scene::getSkybox, allow_raw_pointers())
    .function("setIndirectLight", &Scene::setIndirectLight, allow_raw_pointers())
    .function("getIndirectLight", &Scene::getIndirectLight, allow_raw_pointers())
    .function("getEntityCount", &Scene::getEntityCount)
    .function("getRenderableCount", &Scene::getRenderableCount)
    .function("getLightCount", &Scene::getLightCount);

/// Frustum ::core class:: Represents the six planes of a truncated viewing pyramid
class_<Frustum>("Frustum")
    .constructor(EMBIND_LAMBDA(Frustum*, (flatmat4 m), {
        return new Frustum(m.m);
    }), allow_raw_pointers())

    .function("setProjection",  EMBIND_LAMBDA(void, (Frustum* self, flatmat4 m), {
        self->setProjection(m.m);
    }), allow_raw_pointers())

    .function("getNormalizedPlane", &Frustum::getNormalizedPlane)

    .function("intersectsBox", EMBIND_LAMBDA(bool, (Frustum* self, const Box& box), {
        return self->intersects(box);
    }), allow_raw_pointers())

    .function("intersectsSphere", EMBIND_LAMBDA(bool, (Frustum* self, const filament::math::float4& sphere), {
        return self->intersects(sphere);
    }), allow_raw_pointers());

/// Camera ::core class:: Represents the eye through which the scene is viewed.
/// See also the [Engine] methods `createCamera` and `destroyCamera`.
class_<Camera>("Camera")
    .function("setProjection", EMBIND_LAMBDA(void, (Camera* self, Camera::Projection projection,
            double left, double right, double bottom, double top, double near, double far), {
        self->setProjection(projection, left, right, bottom, top, near, far);
    }), allow_raw_pointers())

    .function("setProjectionFov", EMBIND_LAMBDA(void, (Camera* self,
            double fovInDegrees, double aspect, double near, double far, Camera::Fov direction), {
        self->setProjection(fovInDegrees, aspect, near, far, direction);
    }), allow_raw_pointers())

    .function("setLensProjection", &Camera::setLensProjection)

    .function("setCustomProjection", EMBIND_LAMBDA(void, (Camera* self,
            flatmat4 m, double near, double far), {
        self->setCustomProjection(filament::math::mat4(m.m), near, far);
    }), allow_raw_pointers())

    .function("setScaling", EMBIND_LAMBDA(void, (Camera* self, math::double2 scaling), {
        self->setScaling(scaling);
    }), allow_raw_pointers())

    .function("getProjectionMatrix", EMBIND_LAMBDA(flatmat4, (Camera* self), {
        return flatmat4 { filament::math::mat4f(self->getProjectionMatrix()) };
    }), allow_raw_pointers())

    .function("getCullingProjectionMatrix", EMBIND_LAMBDA(flatmat4, (Camera* self), {
        return flatmat4 { filament::math::mat4f(self->getCullingProjectionMatrix()) };
    }), allow_raw_pointers())

    .function("getScaling", &Camera::getScaling)

    .function("getNear", &Camera::getNear)
    .function("getCullingFar", &Camera::getCullingFar)

    .function("setModelMatrix", EMBIND_LAMBDA(void, (Camera* self, flatmat4 m), {
        self->setModelMatrix(m.m);
    }), allow_raw_pointers())

    .function("lookAt", EMBIND_LAMBDA(void, (Camera* self,
            const math::float3& eye,
            const math::float3& center,
            const math::float3& up), {
        self->lookAt(eye, center, up);
    }), allow_raw_pointers())

    .function("getModelMatrix", EMBIND_LAMBDA(flatmat4, (Camera* self), {
        return flatmat4 { (filament::math::mat4f)self->getModelMatrix() };
    }), allow_raw_pointers())

    .function("getViewMatrix", EMBIND_LAMBDA(flatmat4, (Camera* self), {
        return flatmat4 { (filament::math::mat4f)self->getViewMatrix() };
    }), allow_raw_pointers())

    .function("getPosition", &Camera::getPosition)
    .function("getLeftVector", &Camera::getLeftVector)
    .function("getUpVector", &Camera::getUpVector)
    .function("getForwardVector", &Camera::getForwardVector)
    .function("getFrustum", &Camera::getFrustum)
    .function("setExposure", (void(Camera::*)(float, float, float)) &Camera::setExposure)
    .function("setExposureDirect", (void(Camera::*)(float)) &Camera::setExposure)
    .function("getAperture", &Camera::getAperture)
    .function("getShutterSpeed", &Camera::getShutterSpeed)
    .function("getSensitivity", &Camera::getSensitivity)
    .function("setFocusDistance", &Camera::setFocusDistance)
    .function("getFocusDistance", &Camera::getFocusDistance)
    .function("getFocalLength", &Camera::getFocalLength)

    .class_function("computeEffectiveFocalLength", &Camera::computeEffectiveFocalLength)
    .class_function("computeEffectiveFov", &Camera::computeEffectiveFov)

    .class_function("inverseProjection",  (flatmat4 (*)(flatmat4)) [] (flatmat4 m) {
        return flatmat4 { filament::math::mat4f(Camera::inverseProjection(m.m)) };
    }, allow_raw_pointers());

class_<ColorGrading>("ColorGrading")
    .class_function("Builder", (ColorBuilder (*)()) [] { return ColorBuilder(); });

class_<ColorBuilder>("ColorGrading$Builder")
    .function("_build", EMBIND_LAMBDA(ColorGrading*, (ColorBuilder* builder, Engine* engine), {
        return builder->build(*engine);
    }), allow_raw_pointers())

    .BUILDER_FUNCTION("quality", ColorBuilder, (ColorBuilder* builder,
            ColorGrading::QualityLevel ql), {
        return &builder->quality(ql);
    })

    .BUILDER_FUNCTION("format", ColorBuilder, (ColorBuilder* builder,
            ColorGrading::LutFormat format), {
        return &builder->format(format);
    })

    .BUILDER_FUNCTION("dimensions", ColorBuilder, (ColorBuilder* builder, uint8_t dim), {
        return &builder->dimensions(dim);
    })

    .BUILDER_FUNCTION("toneMapping", ColorBuilder, (ColorBuilder* builder,
            ColorGrading::ToneMapping tm), {
        return &builder->toneMapping(tm);
    })

    .BUILDER_FUNCTION("luminanceScaling", ColorBuilder, (ColorBuilder* builder,
            bool luminanceScaling), {
        return &builder->luminanceScaling(luminanceScaling);
    })

    .BUILDER_FUNCTION("gamutMapping", ColorBuilder, (ColorBuilder* builder,
            bool gamutMapping), {
        return &builder->gamutMapping(gamutMapping);
    })

    .BUILDER_FUNCTION("exposure", ColorBuilder, (ColorBuilder* builder,
            float exposure), {
        return &builder->exposure(exposure);
    })

    .BUILDER_FUNCTION("nightAdaptation", ColorBuilder, (ColorBuilder* builder,
            float adaptation), {
        return &builder->nightAdaptation(adaptation);
    })

    .BUILDER_FUNCTION("whiteBalance", ColorBuilder, (ColorBuilder* builder, float temp,
            float tint), {
        return &builder->whiteBalance(temp, tint);
    })

    .BUILDER_FUNCTION("channelMixer", ColorBuilder, (ColorBuilder* builder,
            filament::math::float3 red,
            filament::math::float3 green,
            filament::math::float3 blue), {
        return &builder->channelMixer(red, green, blue);
    })

    .BUILDER_FUNCTION("shadowsMidtonesHighlights", ColorBuilder, (ColorBuilder* builder,
            filament::math::float4 shadows,
            filament::math::float4 midtones,
            filament::math::float4 highlights,
            filament::math::float4 ranges), {
        return &builder->shadowsMidtonesHighlights(shadows, midtones, highlights, ranges);
    })

    .BUILDER_FUNCTION("slopeOffsetPower", ColorBuilder, (ColorBuilder* builder,
            math::float3 slope, math::float3 offset, math::float3 power), {
        return &builder->slopeOffsetPower(slope, offset, power);
    })

    .BUILDER_FUNCTION("contrast", ColorBuilder, (ColorBuilder* builder, float contrast), {
        return &builder->contrast(contrast);
    })

    .BUILDER_FUNCTION("vibrance", ColorBuilder, (ColorBuilder* builder, float vibrance), {
        return &builder->vibrance(vibrance);
    })

    .BUILDER_FUNCTION("saturation", ColorBuilder, (ColorBuilder* builder, float saturation), {
        return &builder->saturation(saturation);
    })

    .BUILDER_FUNCTION("curves", ColorBuilder, (ColorBuilder* builder, math::float3 shadowGamma,
            math::float3 midPoint, math::float3 highlightScale), {
        return &builder->curves(shadowGamma, midPoint, highlightScale);
    });

class_<RenderTargetBuilder>("RenderTarget$Builder")
    .BUILDER_FUNCTION("texture", RenderTargetBuilder, (RenderTargetBuilder* builder,
            RenderTarget::AttachmentPoint attachment, Texture* tex), {
        return &builder->texture(attachment, tex); })

    .BUILDER_FUNCTION("mipLevel", RenderTargetBuilder, (RenderTargetBuilder* builder,
            RenderTarget::AttachmentPoint attachment, uint8_t level), {
        return &builder->mipLevel(attachment, level); })

    .BUILDER_FUNCTION("face", RenderTargetBuilder, (RenderTargetBuilder* builder,
            RenderTarget::AttachmentPoint attachment, Texture::CubemapFace face), {
        return &builder->face(attachment, face); })

    .BUILDER_FUNCTION("layer", RenderTargetBuilder, (RenderTargetBuilder* builder,
            RenderTarget::AttachmentPoint attachment, uint32_t layer), {
        return &builder->layer(attachment, layer); })

    .function("_build", EMBIND_LAMBDA(RenderTarget*, (RenderTargetBuilder* builder, Engine* engine), {
        return builder->build(*engine);
    }), allow_raw_pointers());

class_<RenderTarget>("RenderTarget")
    .class_function("Builder", (RenderTargetBuilder (*)()) [] () {
        return RenderTarget::Builder();
    })
    .function("getMipLevel", &RenderTarget::getMipLevel)
    .function("getFace", &RenderTarget::getFace)
    .function("getLayer", &RenderTarget::getLayer);

class_<RenderableBuilder>("RenderableManager$Builder")
    .BUILDER_FUNCTION("geometry", RenderableBuilder, (RenderableBuilder* builder,
            size_t index,
            RenderableManager::PrimitiveType type,
            VertexBuffer* vertices,
            IndexBuffer* indices), {
        return &builder->geometry(index, type, vertices, indices); })

    .BUILDER_FUNCTION("geometryOffset", RenderableBuilder, (RenderableBuilder* builder,
            size_t index,
            RenderableManager::PrimitiveType type,
            VertexBuffer* vertices,
            IndexBuffer* indices,
            size_t offset,
            size_t count), {
        return &builder->geometry(index, type, vertices, indices, offset, count); })

    .BUILDER_FUNCTION("geometryMinMax", RenderableBuilder, (RenderableBuilder* builder,
            size_t index,
            RenderableManager::PrimitiveType type,
            VertexBuffer* vertices,
            IndexBuffer* indices,
            size_t offset,
            size_t minIndex,
            size_t maxIndex,
            size_t count), {
        return &builder->geometry(index, type, vertices, indices, offset, minIndex, maxIndex, count); })

    .BUILDER_FUNCTION("geometryType", RenderableBuilder, (RenderableBuilder* builder,
            RenderableManager::Builder::GeometryType type), {
        return &builder->geometryType(type); })

    .BUILDER_FUNCTION("material", RenderableBuilder, (RenderableBuilder* builder,
            size_t index, MaterialInstance* mi), {
        return &builder->material(index, mi); })

    .BUILDER_FUNCTION("boundingBox", RenderableBuilder, (RenderableBuilder* builder, Box box), {
        return &builder->boundingBox(box); })

    .BUILDER_FUNCTION("layerMask", RenderableBuilder, (RenderableBuilder* builder, uint8_t select,
            uint8_t values), {
        return &builder->layerMask(select, values); })

    .BUILDER_FUNCTION("priority", RenderableBuilder, (RenderableBuilder* builder, uint8_t value), {
        return &builder->priority(value); })

    .BUILDER_FUNCTION("channel", RenderableBuilder, (RenderableBuilder* builder, uint8_t value), {
        return &builder->channel(value); })

    .BUILDER_FUNCTION("culling", RenderableBuilder, (RenderableBuilder* builder, bool enable), {
        return &builder->culling(enable); })

    .BUILDER_FUNCTION("lightChannel", RenderableBuilder,
            (RenderableBuilder* builder, unsigned int channel, bool enable), {
        return &builder->lightChannel(channel, enable); })

    .BUILDER_FUNCTION("castShadows", RenderableBuilder, (RenderableBuilder* builder, bool enable), {
        return &builder->castShadows(enable); })

    .BUILDER_FUNCTION("receiveShadows", RenderableBuilder, (RenderableBuilder* builder, bool enable), {
        return &builder->receiveShadows(enable); })

    .BUILDER_FUNCTION("screenSpaceContactShadows", RenderableBuilder,
            (RenderableBuilder* builder, bool enable), {
        return &builder->screenSpaceContactShadows(enable); })

    .BUILDER_FUNCTION("fog", RenderableBuilder, (RenderableBuilder* builder, bool enable), {
        return &builder->fog(enable); })

    .BUILDER_FUNCTION("skinning", RenderableBuilder, (RenderableBuilder* builder, size_t boneCount), {
        return &builder->skinning(boneCount); })

    .BUILDER_FUNCTION("skinningBones", RenderableBuilder, (RenderableBuilder* builder,
            emscripten::val transforms), {
        auto nbones = transforms["length"].as<size_t>();
        std::vector<RenderableManager::Bone> bones(nbones);
        for (size_t i = 0; i < nbones; i++) {
            bones[i] = transforms[i].as<RenderableManager::Bone>();
        }
        return &builder->skinning(bones.size(), bones.data());
    })

    .BUILDER_FUNCTION("skinningMatrices", RenderableBuilder, (RenderableBuilder* builder,
            emscripten::val transforms), {
        auto nbones = transforms["length"].as<size_t>();
        std::vector<filament::math::mat4f> matrices(nbones);
        for (size_t i = 0; i < nbones; i++) {
            matrices[i] = transforms[i].as<flatmat4>().m;
        }
        return &builder->skinning(matrices.size(), matrices.data());
    })

    .BUILDER_FUNCTION("morphing", RenderableBuilder, (RenderableBuilder* builder, bool enable), {
        return &builder->morphing(enable); })

    .BUILDER_FUNCTION("blendOrder", RenderableBuilder,
            (RenderableBuilder* builder, size_t index, uint16_t order), {
        return &builder->blendOrder(index, order); })

    .BUILDER_FUNCTION("globalBlendOrderEnabled", RenderableBuilder,
            (RenderableBuilder* builder, size_t index, bool enabled), {
        return &builder->globalBlendOrderEnabled(index, enabled); })

    .BUILDER_FUNCTION("instances", RenderableBuilder,
            (RenderableBuilder* builder, size_t instanceCount), {
        return &builder->instances(instanceCount); })

    .function("_build", EMBIND_LAMBDA(int, (RenderableBuilder* builder,
            Engine* engine, utils::Entity entity), {
        return (int) builder->build(*engine, entity);
    }), allow_raw_pointers());

/// RenderableManager ::core class:: Allows access to properties of drawable objects.
class_<RenderableManager>("RenderableManager")
    .function("hasComponent", &RenderableManager::hasComponent)

    /// getInstance ::method:: Gets an instance of the renderable component for an entity.
    /// entity ::argument:: an [Entity]
    /// ::retval:: a renderable component
    .function("getInstance", &RenderableManager::getInstance)

    .class_function("Builder", (RenderableBuilder (*)(int)) [] (int n) {
        return RenderableBuilder(n);
    }, return_value_policy::take_ownership())

    .function("destroy", &RenderableManager::destroy)
    .function("setAxisAlignedBoundingBox", &RenderableManager::setAxisAlignedBoundingBox)
    .function("setLayerMask", &RenderableManager::setLayerMask)
    .function("setPriority", &RenderableManager::setPriority)
    .function("setChannel", &RenderableManager::setChannel)
    .function("setCastShadows", &RenderableManager::setCastShadows)
    .function("setReceiveShadows", &RenderableManager::setReceiveShadows)
    .function("isShadowCaster", &RenderableManager::isShadowCaster)
    .function("isShadowReceiver", &RenderableManager::isShadowReceiver)
    .function("setLightChannel", &RenderableManager::setLightChannel)
    .function("getLightChannel", &RenderableManager::getLightChannel)
    .function("setFogEnabled", &RenderableManager::setFogEnabled)
    .function("getFogEnabled", &RenderableManager::getFogEnabled)

    .function("setBones", EMBIND_LAMBDA(void, (RenderableManager* self,
            RenderableManager::Instance instance, emscripten::val transforms, size_t offset), {
        auto nbones = transforms["length"].as<size_t>();
        std::vector<RenderableManager::Bone> bones(nbones);
        for (size_t i = 0; i < nbones; i++) {
            bones[i] = transforms[i].as<RenderableManager::Bone>();
        }
        self->setBones(instance, bones.data(), bones.size(), offset);
    }), allow_raw_pointers())

    .function("setBonesFromMatrices", EMBIND_LAMBDA(void, (RenderableManager* self,
            RenderableManager::Instance instance, emscripten::val transforms, size_t offset), {
        auto nbones = transforms["length"].as<size_t>();
        std::vector<filament::math::mat4f> bones(nbones);
        for (size_t i = 0; i < nbones; i++) {
            bones[i] = transforms[i].as<flatmat4>().m;
        }
        self->setBones(instance, bones.data(), bones.size(), offset);
    }), allow_raw_pointers())

    .function("setMorphWeights", EMBIND_LAMBDA(void, (RenderableManager* self,
            RenderableManager::Instance instance, emscripten::val weights), {
        auto nfloats = weights["length"].as<size_t>();
        std::vector<float> floats(nfloats);
        for (size_t i = 0; i < nfloats; i++) {
            floats[i] = weights[i].as<float>();
        }
        self->setMorphWeights(instance, floats.data(), nfloats);
    }), allow_raw_pointers())

    .function("getAxisAlignedBoundingBox", &RenderableManager::getAxisAlignedBoundingBox)
    .function("getPrimitiveCount", &RenderableManager::getPrimitiveCount)
    .function("setMaterialInstanceAt", &RenderableManager::setMaterialInstanceAt,
            allow_raw_pointers())
    .function("clearMaterialInstanceAt", &RenderableManager::clearMaterialInstanceAt)
    .function("getMaterialInstanceAt", &RenderableManager::getMaterialInstanceAt,
            allow_raw_pointers())

    .function("setGeometryAt", EMBIND_LAMBDA(void, (RenderableManager* self,
            RenderableManager::Instance instance, size_t primitiveIndex,
            RenderableManager::PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices,
            size_t offset, size_t count), {
        self->setGeometryAt(instance, primitiveIndex, type, vertices, indices, offset, count);
    }), allow_raw_pointers())

    .function("setBlendOrderAt", &RenderableManager::setBlendOrderAt)

    .function("setGlobalBlendOrderEnabledAt", &RenderableManager::setGlobalBlendOrderEnabledAt)

    .function("getEnabledAttributesAt", EMBIND_LAMBDA(uint32_t, (RenderableManager* self,
            RenderableManager::Instance instance, size_t primitiveIndex), {
        return self->getEnabledAttributesAt(instance, primitiveIndex).getValue();
    }), allow_raw_pointers());

/// RenderableManager$Instance ::class:: Component instance returned by [RenderableManager]
/// Be sure to call the instance's `delete` method when you're done with it.
class_<RenderableManager::Instance>("RenderableManager$Instance");
    /// delete ::method:: Frees an instance obtained via `getInstance`

/// TransformManager ::core class:: Adds transform components to entities.
class_<TransformManager>("TransformManager")
    .function("hasComponent", &TransformManager::hasComponent)

    /// getInstance ::method:: Gets an instance representing the transform component for an entity.
    /// entity ::argument:: an [Entity]
    /// ::retval:: a transform component that can be passed to `setTransform`.
    .function("getInstance", &TransformManager::getInstance)

    .function("create", EMBIND_LAMBDA(void,
            (TransformManager* self, utils::Entity entity), {
        self->create(entity);
    }), allow_raw_pointers())

    .function("destroy", &TransformManager::destroy)
    .function("setParent", &TransformManager::setParent)
    .function("getParent", &TransformManager::getParent)

    .function("getChildren", EMBIND_LAMBDA(EntityVector,
            (TransformManager* self, TransformManager::Instance instance), {
        EntityVector result(self->getChildCount(instance));
        self->getChildren(instance, result.data(), result.size());
        return result;
    }), allow_raw_pointers())

    /// setTransform ::method:: Sets the mat4 value of a transform component.
    /// instance ::argument:: The transform instance of entity, obtained via `getInstance`.
    /// matrix ::argument:: Array of 16 numbers (mat4)
    .function("setTransform", EMBIND_LAMBDA(void,
            (TransformManager* self, TransformManager::Instance instance, flatmat4 m), {
        self->setTransform(instance, m.m); }), allow_raw_pointers())

    .function("getTransform", EMBIND_LAMBDA(flatmat4,
            (TransformManager* self, TransformManager::Instance instance), {
        return flatmat4 { self->getTransform(instance) } ; }), allow_raw_pointers())

    .function("getWorldTransform", EMBIND_LAMBDA(flatmat4,
            (TransformManager* self, TransformManager::Instance instance), {
        return flatmat4 { self->getWorldTransform(instance) } ; }), allow_raw_pointers())

    .function("openLocalTransformTransaction", &TransformManager::openLocalTransformTransaction)
    .function("commitLocalTransformTransaction",
            &TransformManager::commitLocalTransformTransaction);

/// TransformManager$Instance ::class:: Component instance returned by [TransformManager]
/// Be sure to call the instance's `delete` method when you're done with it.
class_<TransformManager::Instance>("TransformManager$Instance");
    /// delete ::method:: Frees an instance obtained via `getInstance`

class_<LightBuilder>("LightManager$Builder")
    .function("_build", EMBIND_LAMBDA(int, (LightBuilder* builder,
            Engine* engine, utils::Entity entity), {
        return (int) builder->build(*engine, entity);
    }), allow_raw_pointers())
    .BUILDER_FUNCTION("castShadows", LightBuilder, (LightBuilder* builder, bool enable), {
        return &builder->castShadows(enable); })
    .BUILDER_FUNCTION("_shadowOptions", LightBuilder, (LightBuilder* builder,
            LightManager::ShadowOptions options), {
        return &builder->shadowOptions(options); })
    .BUILDER_FUNCTION("castLight", LightBuilder, (LightBuilder* builder, bool enable), {
        return &builder->castLight(enable); })
    .BUILDER_FUNCTION("position", LightBuilder, (LightBuilder* builder, filament::math::float3 value), {
        return &builder->position(value); })
    .BUILDER_FUNCTION("direction", LightBuilder, (LightBuilder* builder, filament::math::float3 value), {
        return &builder->direction(value); })
    .BUILDER_FUNCTION("color", LightBuilder, (LightBuilder* builder, filament::math::float3 value), {
        return &builder->color(value); })
    .BUILDER_FUNCTION("intensity", LightBuilder, (LightBuilder* builder, float value), {
        return &builder->intensity(value); })
    .BUILDER_FUNCTION("falloff", LightBuilder, (LightBuilder* builder, float value), {
        return &builder->falloff(value); })
    .BUILDER_FUNCTION("spotLightCone", LightBuilder,
            (LightBuilder* builder, float inner, float outer), {
        return &builder->spotLightCone(inner, outer); })
    .BUILDER_FUNCTION("sunAngularRadius", LightBuilder,
            (LightBuilder* builder, float value), { return &builder->sunAngularRadius(value); })
    .BUILDER_FUNCTION("sunHaloSize", LightBuilder,
            (LightBuilder* builder, float value), { return &builder->sunHaloSize(value); })
    .BUILDER_FUNCTION("sunHaloFalloff", LightBuilder,
            (LightBuilder* builder, float value), { return &builder->sunHaloFalloff(value); })
    .BUILDER_FUNCTION("lightChannel", LightBuilder,
            (LightBuilder* builder, unsigned int channel, bool enable), {
        return &builder->lightChannel(channel, enable); });

class_<LightManager>("LightManager")
    .function("hasComponent", &LightManager::hasComponent)

    /// getInstance ::method:: Gets an instance of the light component for an entity.
    /// entity ::argument:: an [Entity]
    /// ::retval:: a light source component
    .function("getInstance", &LightManager::getInstance)

    .class_function("Builder", (LightBuilder (*)(LightManager::Type)) [] (LightManager::Type lt) {
        return LightBuilder(lt); })

    .function("getType", &LightManager::getType)
    .function("isDirectional", &LightManager::isDirectional)
    .function("isPointLight", &LightManager::isPointLight)
    .function("isSpotLight", &LightManager::isSpotLight)
    .function("setPosition", &LightManager::setPosition)
    .function("getPosition", &LightManager::getPosition)
    .function("setDirection", &LightManager::setDirection)
    .function("getDirection", &LightManager::getDirection)
    .function("setColor", &LightManager::setColor)
    .function("getColor", &LightManager::getColor)

    .function("setIntensity", EMBIND_LAMBDA(void, (LightManager* self,
            LightManager::Instance instance, float intensity), {
        self->setIntensity(instance, intensity);
    }), allow_raw_pointers())

    .function("setIntensityEnergy", EMBIND_LAMBDA(void, (LightManager* self,
            LightManager::Instance instance, float watts, float efficiency), {
        self->setIntensity(instance, watts, efficiency);
    }), allow_raw_pointers())

    .function("getIntensity", &LightManager::getIntensity)
    .function("setFalloff", &LightManager::setFalloff)
    .function("getFalloff", &LightManager::getFalloff)
    .function("_setShadowOptions", &LightManager::setShadowOptions)
    .function("setSpotLightCone", &LightManager::setSpotLightCone)
    .function("setSunAngularRadius", &LightManager::setSunAngularRadius)
    .function("getSunAngularRadius", &LightManager::getSunAngularRadius)
    .function("setSunHaloSize", &LightManager::setSunHaloSize)
    .function("getSunHaloSize", &LightManager::getSunHaloSize)
    .function("setSunHaloFalloff", &LightManager::setSunHaloFalloff)
    .function("getSunHaloFalloff", &LightManager::getSunHaloFalloff)
    .function("setShadowCaster", &LightManager::setShadowCaster)
    .function("isShadowCaster", &LightManager::isShadowCaster)
    .function("setLightChannel", &LightManager::setLightChannel)
    .function("getLightChannel", &LightManager::getLightChannel)
    ;

/// LightManager$Instance ::class:: Component instance returned by [LightManager]
/// Be sure to call the instance's `delete` method when you're done with it.
class_<LightManager::Instance>("LightManager$Instance");
    /// delete ::method:: Frees an instance obtained via `getInstance`

class_<BufferBuilder>("BufferObject$Builder")
   .function("_build", EMBIND_LAMBDA(BufferObject*, (BufferBuilder* builder, Engine* engine), {
       return builder->build(*engine);
   }), allow_raw_pointers())
   .BUILDER_FUNCTION("bindingType", BufferBuilder, (BufferBuilder* builder,
           BufferObject::BindingType bt), {
       return &builder->bindingType(bt); })
   .BUILDER_FUNCTION("size", BufferBuilder, (BufferBuilder* builder, int byteCount), {
       return &builder->size(byteCount); });

/// BufferObject ::core class:: Represents a single GPU buffer.
class_<BufferObject>("BufferObject")
   .class_function("Builder", (BufferBuilder (*)()) [] { return BufferBuilder(); })
   .function("getByteCount", &BufferObject::getByteCount)
   .function("_setBuffer", EMBIND_LAMBDA(void, (BufferObject* self,
           Engine* engine, BufferDescriptor bd, uint32_t byteOffset), {
       self->setBuffer(*engine, std::move(*bd.bd), byteOffset);
   }), allow_raw_pointers());

class_<VertexBuilder>("VertexBuffer$Builder")
    .function("_build", EMBIND_LAMBDA(VertexBuffer*, (VertexBuilder* builder, Engine* engine), {
        return builder->build(*engine);
    }), allow_raw_pointers())
    .BUILDER_FUNCTION("attribute", VertexBuilder, (VertexBuilder* builder,
            VertexAttribute attr,
            uint8_t bufferIndex,
            VertexBuffer::AttributeType attrType,
            uint32_t byteOffset,
            uint8_t byteStride), {
        return &builder->attribute(attr, bufferIndex, attrType, byteOffset, byteStride); })
    .BUILDER_FUNCTION("vertexCount", VertexBuilder, (VertexBuilder* builder, int count), {
        return &builder->vertexCount(count); })
    .BUILDER_FUNCTION("enableBufferObjects", VertexBuilder, (VertexBuilder* builder, bool enable), {
        return &builder->enableBufferObjects(enable); })
    .BUILDER_FUNCTION("normalized", VertexBuilder, (VertexBuilder* builder,
            VertexAttribute attrib), {
        return &builder->normalized(attrib); })
    .BUILDER_FUNCTION("normalizedIf", VertexBuilder, (VertexBuilder* builder,
            VertexAttribute attrib, bool normalized), {
        return &builder->normalized(attrib, normalized); })
    .BUILDER_FUNCTION("bufferCount", VertexBuilder, (VertexBuilder* builder, int count), {
        return &builder->bufferCount(count); });

/// VertexBuffer ::core class:: Bundle of buffers and associated vertex attributes.
class_<VertexBuffer>("VertexBuffer")
    .class_function("Builder", (VertexBuilder (*)()) [] { return VertexBuilder(); })
    .function("setBufferObjectAt", &VertexBuffer::setBufferObjectAt, allow_raw_pointers())
    .function("_setBufferAt", EMBIND_LAMBDA(void, (VertexBuffer* self,
            Engine* engine, uint8_t bufferIndex, BufferDescriptor vbd, uint32_t byteOffset), {
        self->setBufferAt(*engine, bufferIndex, std::move(*vbd.bd), byteOffset);
    }), allow_raw_pointers());

class_<IndexBuilder>("IndexBuffer$Builder")
    .function("_build", EMBIND_LAMBDA(IndexBuffer*, (IndexBuilder* builder, Engine* engine), {
        return builder->build(*engine);
    }), allow_raw_pointers())
    .BUILDER_FUNCTION("indexCount", IndexBuilder, (IndexBuilder* builder, int count), {
        return &builder->indexCount(count); })
    .BUILDER_FUNCTION("bufferType", IndexBuilder, (IndexBuilder* builder,
            IndexBuffer::IndexType indexType), {
        return &builder->bufferType(indexType); });

/// IndexBuffer ::core class:: Array of 16-bit or 32-bit unsigned integers consumed by the GPU.
class_<IndexBuffer>("IndexBuffer")
    .class_function("Builder", (IndexBuilder (*)()) [] { return IndexBuilder(); })
    .function("_setBuffer", EMBIND_LAMBDA(void, (IndexBuffer* self,
            Engine* engine, BufferDescriptor ibd, uint32_t byteOffset), {
        self->setBuffer(*engine, std::move(*ibd.bd), byteOffset);
    }), allow_raw_pointers());

class_<Material>("Material")
    .function("getDefaultInstance",
            select_overload<MaterialInstance*(void)>(&Material::getDefaultInstance),
            allow_raw_pointers())
    .function("createInstance", EMBIND_LAMBDA(MaterialInstance*, (Material* self), {
        return self->createInstance(); }), allow_raw_pointers())
    .function("createNamedInstance", EMBIND_LAMBDA(MaterialInstance*,
            (Material* self, std::string name), {
        return self->createInstance(name.c_str()); }), allow_raw_pointers())
    .function("getName", EMBIND_LAMBDA(std::string, (Material* self), {
        return std::string(self->getName());
    }), allow_raw_pointers());

class_<MaterialInstance>("MaterialInstance")
    .function("getName", EMBIND_LAMBDA(std::string, (MaterialInstance* self), {
        return std::string(self->getName());
    }), allow_raw_pointers())
    .function("setBoolParameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, bool value), {
        self->setParameter(name.c_str(), value); }), allow_raw_pointers())
    .function("setFloatParameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, float value), {
        self->setParameter(name.c_str(), value); }), allow_raw_pointers())
    .function("setFloat2Parameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, filament::math::float2 value), {
        self->setParameter(name.c_str(), value); }), allow_raw_pointers())
    .function("setFloat3Parameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, filament::math::float3 value), {
        self->setParameter(name.c_str(), value); }), allow_raw_pointers())
    .function("setFloat4Parameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, filament::math::float4 value), {
        self->setParameter(name.c_str(), value); }), allow_raw_pointers())
    .function("setMat3Parameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, flatmat3 value), {
        self->setParameter(name.c_str(), value.m); }), allow_raw_pointers())
    .function("setMat4Parameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, flatmat4 value), {
        self->setParameter(name.c_str(), value.m); }), allow_raw_pointers())
    .function("setTextureParameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, Texture* value, TextureSampler sampler), {
        self->setParameter(name.c_str(), value, sampler); }), allow_raw_pointers())
    .function("setColor3Parameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, RgbType type, filament::math::float3 value), {
        self->setParameter(name.c_str(), type, value); }), allow_raw_pointers())
    .function("setColor4Parameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, RgbaType type, filament::math::float4 value), {
        self->setParameter(name.c_str(), type, value); }), allow_raw_pointers())
    .function("setPolygonOffset", &MaterialInstance::setPolygonOffset)
    .function("setMaskThreshold", &MaterialInstance::setMaskThreshold)
    .function("getMaskThreshold", &MaterialInstance::getMaskThreshold)
    .function("setSpecularAntiAliasingVariance", &MaterialInstance::setSpecularAntiAliasingVariance)
    .function("getSpecularAntiAliasingVariance", &MaterialInstance::getSpecularAntiAliasingVariance)
    .function("setSpecularAntiAliasingThreshold", &MaterialInstance::setSpecularAntiAliasingThreshold)
    .function("getSpecularAntiAliasingThreshold", &MaterialInstance::getSpecularAntiAliasingThreshold)
    .function("setDoubleSided", &MaterialInstance::setDoubleSided)
    .function("isDoubleSided", &MaterialInstance::isDoubleSided)
    .function("setTransparencyMode", &MaterialInstance::setTransparencyMode)
    .function("getTransparencyMode", &MaterialInstance::getTransparencyMode)
    .function("setCullingMode", EMBIND_LAMBDA(void,
            (MaterialInstance* self, MaterialInstance::CullingMode mode), {
        self->setCullingMode(mode); }), allow_raw_pointers())
    .function("setCullingModeSeparate", EMBIND_LAMBDA(void,
            (MaterialInstance* self, MaterialInstance::CullingMode color, MaterialInstance::CullingMode shadows), {
        self->setCullingMode(color, shadows); }), allow_raw_pointers())
    .function("getCullingMode", &MaterialInstance::getCullingMode)
    .function("getShadowCullingMode", &MaterialInstance::getShadowCullingMode)
    .function("setColorWrite", &MaterialInstance::setColorWrite)
    .function("isColorWriteEnabled", &MaterialInstance::isColorWriteEnabled)
    .function("setDepthWrite", &MaterialInstance::setDepthWrite)
    .function("isDepthWriteEnabled", &MaterialInstance::isDepthWriteEnabled)
    .function("setStencilWrite", &MaterialInstance::setStencilWrite)
    .function("setDepthCulling", &MaterialInstance::setDepthCulling)
    .function("isDepthCullingEnabled", &MaterialInstance::isDepthCullingEnabled)
    .function("setDepthFunc", &MaterialInstance::setDepthFunc)
    .function("getDepthFunc", &MaterialInstance::getDepthFunc)
    .function("setStencilCompareFunction", &MaterialInstance::setStencilCompareFunction)
    .function("setStencilCompareFunction", EMBIND_LAMBDA(void,
            (MaterialInstance* self, MaterialInstance::StencilCompareFunc func), {
                self->setStencilCompareFunction(func, backend::StencilFace::FRONT_AND_BACK);
            }), allow_raw_pointers())
    .function("setStencilOpStencilFail", &MaterialInstance::setStencilOpStencilFail)
    .function("setStencilOpStencilFail", EMBIND_LAMBDA(void,
            (MaterialInstance* self, MaterialInstance::StencilOperation op), {
                self->setStencilOpStencilFail(op, backend::StencilFace::FRONT_AND_BACK);
            }), allow_raw_pointers())
    .function("setStencilOpDepthFail", &MaterialInstance::setStencilOpDepthFail)
    .function("setStencilOpDepthFail", EMBIND_LAMBDA(void,
            (MaterialInstance* self, MaterialInstance::StencilOperation op), {
                self->setStencilOpDepthFail(op, backend::StencilFace::FRONT_AND_BACK);
            }), allow_raw_pointers())
    .function("setStencilOpDepthStencilPass", &MaterialInstance::setStencilOpDepthStencilPass)
    .function("setStencilOpDepthStencilPass", EMBIND_LAMBDA(void,
            (MaterialInstance* self, MaterialInstance::StencilOperation op), {
                self->setStencilOpDepthStencilPass(op, backend::StencilFace::FRONT_AND_BACK);
            }), allow_raw_pointers())
    .function("setStencilReferenceValue", &MaterialInstance::setStencilReferenceValue)
    .function("setStencilReferenceValue", EMBIND_LAMBDA(void,
            (MaterialInstance* self, uint8_t value), {
                self->setStencilReferenceValue(value, backend::StencilFace::FRONT_AND_BACK);
            }), allow_raw_pointers())
    .function("setStencilReadMask", &MaterialInstance::setStencilReadMask)
    .function("setStencilReadMask", EMBIND_LAMBDA(void,
            (MaterialInstance* self, uint8_t readMask), {
                self->setStencilReadMask(readMask, backend::StencilFace::FRONT_AND_BACK);
            }), allow_raw_pointers())
    .function("setStencilWriteMask", &MaterialInstance::setStencilWriteMask)
    .function("setStencilWriteMask", EMBIND_LAMBDA(void,
            (MaterialInstance* self, uint8_t writeMask), {
                self->setStencilWriteMask(writeMask, backend::StencilFace::FRONT_AND_BACK);
            }), allow_raw_pointers());

class_<TextureSampler>("TextureSampler")
    .constructor<backend::SamplerMinFilter, backend::SamplerMagFilter, backend::SamplerWrapMode>()
    .function("setAnisotropy", &TextureSampler::setAnisotropy)
    .function("setCompareMode", &TextureSampler::setCompareMode);

/// Texture ::core class:: 2D image or cubemap that can be sampled by the GPU, possibly mipmapped.
class_<Texture>("Texture")
    .class_function("Builder", (TexBuilder (*)()) [] { return TexBuilder(); })
    .class_function("isTextureFormatMipmappable", &Texture::isTextureFormatMipmappable)
    .class_function("validatePixelFormatAndType", &Texture::validatePixelFormatAndType)
    .function("generateMipmaps", &Texture::generateMipmaps)
    .function("_setImage", EMBIND_LAMBDA(void, (Texture* self,
            Engine* engine, uint8_t level, PixelBufferDescriptor pbd), {
        self->setImage(*engine, level, std::move(*pbd.pbd));
    }), allow_raw_pointers())
    .function("_setImageCube", EMBIND_LAMBDA(void, (Texture* self,
            Engine* engine, uint8_t level, PixelBufferDescriptor pbd), {
        uint32_t faceSize = pbd.pbd->size / 6;
        Texture::FaceOffsets offsets(faceSize);
        self->setImage(*engine, level, std::move(*pbd.pbd), offsets);
    }), allow_raw_pointers())
    .function("_getWidth", EMBIND_LAMBDA(size_t, (Texture* self,
            Engine* engine, uint8_t level), {
        return self->getWidth(level);
    }), allow_raw_pointers())
    .function("_getHeight", EMBIND_LAMBDA(size_t, (Texture* self,
            Engine* engine, uint8_t level), {
        return self->getHeight(level);
    }), allow_raw_pointers())
    .function("_getDepth", EMBIND_LAMBDA(size_t, (Texture* self,
            Engine* engine, uint8_t level), {
        return self->getDepth(level);
    }), allow_raw_pointers())
    .function("_getLevels", EMBIND_LAMBDA(size_t, (Texture* self,
            Engine* engine), {
        return self->getLevels();
    }), allow_raw_pointers());

class_<TexBuilder>("Texture$Builder")
    .function("_build", EMBIND_LAMBDA(Texture*, (TexBuilder* builder, Engine* engine), {
        return builder->build(*engine);
    }), allow_raw_pointers())
    .BUILDER_FUNCTION("width", TexBuilder, (TexBuilder* builder, uint32_t width), {
        return &builder->width(width); })
    .BUILDER_FUNCTION("height", TexBuilder, (TexBuilder* builder, uint32_t height), {
        return &builder->height(height); })
    .BUILDER_FUNCTION("depth", TexBuilder, (TexBuilder* builder, uint32_t depth), {
        return &builder->depth(depth); })
    .BUILDER_FUNCTION("levels", TexBuilder, (TexBuilder* builder, uint8_t levels), {
        return &builder->levels(levels); })
    .BUILDER_FUNCTION("sampler", TexBuilder, (TexBuilder* builder, Texture::Sampler target), {
        return &builder->sampler(target); })
    .BUILDER_FUNCTION("format", TexBuilder, (TexBuilder* builder, Texture::InternalFormat fmt), {
        return &builder->format(fmt); })
    .BUILDER_FUNCTION("external", TexBuilder, (TexBuilder* builder), {
        return &builder->external(); })

    // This takes a bitfield that can be composed by or'ing constants.
    // - JS clients should use the value member, as in: "Texture$Usage.SAMPLEABLE.value".
    // - TypeScript clients can simply say "TextureUsage.SAMPLEABLE" (note the lack of $)
    .BUILDER_FUNCTION("usage", TexBuilder, (TexBuilder* builder, uint8_t usage), {
        return &builder->usage((Texture::Usage)usage); });

class_<IndirectLight>("IndirectLight")
    .class_function("Builder", (IblBuilder (*)()) [] { return IblBuilder(); })
    .function("setIntensity", &IndirectLight::setIntensity)
    .function("getIntensity", &IndirectLight::getIntensity)
    .function("setRotation", EMBIND_LAMBDA(void, (IndirectLight* self, flatmat3 value), {
        return self->setRotation(value.m);
    }), allow_raw_pointers())
    .function("getRotation", EMBIND_LAMBDA(flatmat3, (IndirectLight* self), {
        return flatmat3 { self->getRotation() };
    }), allow_raw_pointers())
    .function("getReflectionsTexture", EMBIND_LAMBDA(Texture*, (IndirectLight* self), {
        return (Texture*) self->getReflectionsTexture(); // cast away const to appease embind
    }), allow_raw_pointers())
    .function("getIrradianceTexture", EMBIND_LAMBDA(Texture*, (IndirectLight* self), {
        return (Texture*) self->getIrradianceTexture(); // cast away const to appease embind
    }), allow_raw_pointers())
   .class_function("getDirectionEstimate", EMBIND_LAMBDA(filament::math::float3, (val ta), {
        size_t nfloats = ta["length"].as<size_t>();
        std::vector<float> floats(nfloats);
        for (size_t i = 0; i < nfloats; i++) {
            floats[i] = ta[i].as<float>();
        }
        return IndirectLight::getDirectionEstimate((filament::math::float3*) floats.data());
   }), allow_raw_pointers())
   .class_function("getColorEstimate", EMBIND_LAMBDA(filament::math::float4, (val ta,
            filament::math::float3 dir), {
        size_t nfloats = ta["length"].as<size_t>();
        std::vector<float> floats(nfloats);
        for (size_t i = 0; i < nfloats; i++) {
            floats[i] = ta[i].as<float>();
        }
        return IndirectLight::getColorEstimate((filament::math::float3*) floats.data(), dir);
   }), allow_raw_pointers());

class_<IblBuilder>("IndirectLight$Builder")
    .function("_build", EMBIND_LAMBDA(IndirectLight*, (IblBuilder* builder, Engine* engine), {
        return builder->build(*engine);
    }), allow_raw_pointers())
    .BUILDER_FUNCTION("reflections", IblBuilder, (IblBuilder* builder, Texture const* cubemap), {
        return &builder->reflections(cubemap); })
    .BUILDER_FUNCTION("irradianceTex", IblBuilder, (IblBuilder* builder, Texture const* cubemap), {
        return &builder->irradiance(cubemap); })
    .BUILDER_FUNCTION("irradianceSh", IblBuilder, (IblBuilder* builder, uint8_t nbands, val ta), {
        // This is not efficient but consuming a BufferDescriptor would be overkill.
        size_t nfloats = ta["length"].as<size_t>();
        if (nfloats != nbands * nbands * 3) {
            printf("Received %zu floats for spherical harmonics, expected %d.", nfloats, nbands);
            return builder;
        }
        std::vector<float> floats(nfloats);
        for (size_t i = 0; i < nfloats; i++) {
            floats[i] = ta[i].as<float>();
        }
        return &builder->irradiance(nbands, (filament::math::float3 const*) floats.data()); })
    .BUILDER_FUNCTION("intensity", IblBuilder, (IblBuilder* builder, float value), {
        return &builder->intensity(value); })
    .BUILDER_FUNCTION("rotation", IblBuilder, (IblBuilder* builder, flatmat3 value), {
        return &builder->rotation(value.m); });

class_<Skybox>("Skybox")
    .class_function("Builder", (SkyBuilder (*)()) [] { return SkyBuilder(); })
    .function("setColor", &Skybox::setColor)
    .function("getTexture", EMBIND_LAMBDA(Texture*, (Skybox* skybox), {
        return (Texture*) skybox->getTexture(); // cast away const to appease embind
    }), allow_raw_pointers());

class_<SkyBuilder>("Skybox$Builder")
    .function("_build", EMBIND_LAMBDA(Skybox*, (SkyBuilder* builder, Engine* engine), {
        return builder->build(*engine);
    }), allow_raw_pointers())
    .BUILDER_FUNCTION("color", SkyBuilder, (SkyBuilder* builder, filament::math::float4 color), {
        return &builder->color(color); })
    .BUILDER_FUNCTION("environment", SkyBuilder, (SkyBuilder* builder, Texture* cubemap), {
        return &builder->environment(cubemap); })
    .BUILDER_FUNCTION("showSun", SkyBuilder, (SkyBuilder* builder, bool show), {
        return &builder->showSun(show); });

// UTILS TYPES
// -----------

/// Entity ::core class:: Handle to an object consisting of a set of components.
/// To create an entity with no components, use [EntityManager].
/// TODO: It would be better to expose these as JS numbers rather than as JS objects.
/// This would also be more consistent with Filament's Java bindings.
class_<utils::Entity>("Entity")
    .function("getId", &utils::Entity::getId);
    /// delete ::method:: Frees an entity.

/// EntityManager ::core class:: Singleton used for constructing entities in Filament's ECS.
class_<utils::EntityManager>("EntityManager")
    /// get ::static method:: Gets the singleton entity manager instance.
    /// ::retval:: the one and only entity manager
    .class_function("get", (utils::EntityManager* (*)()) []
        { return &utils::EntityManager::get(); }, allow_raw_pointers())

#if FILAMENT_UTILS_TRACK_ENTITIES
    .function("getActiveEntityCount", EMBIND_LAMBDA(size_t, (utils::EntityManager* self), {
        return self->getActiveEntities().size();
    }), allow_raw_pointers())
#endif

    /// create ::method::
    /// ::retval:: an [Entity] without any components
    .function("create", select_overload<utils::Entity()>(&utils::EntityManager::create))
    .function("destroy", select_overload<void(utils::Entity)>(&utils::EntityManager::destroy));

// DRIVER TYPES
// ------------

/// BufferDescriptor ::class:: Low level buffer wrapper.
/// Clients should use the [Buffer] helper function to contruct BufferDescriptor objects.
class_<BufferDescriptor>("driver$BufferDescriptor")
    .constructor<uint32_t>()
    /// getBytes ::method:: Gets a view of the WASM heap referenced by the buffer descriptor.
    /// ::retval:: Uint8Array
    .function("getBytes", &BufferDescriptor::getBytes);

/// PixelBufferDescriptor ::class:: Low level pixel buffer wrapper.
/// Clients should use the [PixelBuffer] helper function to contruct PixelBufferDescriptor objects.
class_<PixelBufferDescriptor>("driver$PixelBufferDescriptor")
    .constructor<uint32_t, backend::PixelDataFormat, backend::PixelDataType>()
    .constructor<uint32_t, backend::CompressedPixelDataType, int, bool>()
    /// getBytes ::method:: Gets a view of the WASM heap referenced by the buffer descriptor.
    /// ::retval:: Uint8Array
    .function("getBytes", &PixelBufferDescriptor::getBytes);

// HELPER TYPES
// ------------

/// Ktx1Bundle ::class:: In-memory representation of a KTX file.
/// Most clients should use one of the `create*FromKtx` utility methods in the JavaScript [Engine]
/// wrapper rather than interacting with `Ktx1Bundle` directly.
class_<Ktx1Bundle>("Ktx1Bundle")
    .constructor(EMBIND_LAMBDA(Ktx1Bundle*, (BufferDescriptor kbd), {
        return new Ktx1Bundle((uint8_t*) kbd.bd->buffer, (uint32_t) kbd.bd->size);
    }))

    /// info ::method:: Obtains properties of the KTX header.
    /// ::retval:: The [KtxInfo] property accessor object.
    .function("getNumMipLevels", &Ktx1Bundle::getNumMipLevels)

    /// getArrayLength ::method:: Obtains length of the texture array.
    /// ::retval:: The number of elements in the texture array
    .function("getArrayLength", &Ktx1Bundle::getArrayLength)

    /// getInternalFormat ::method::
    /// srgb ::argument:: boolean that forces the resulting format to SRGB if possible.
    /// ::retval:: [Texture$InternalFormat]
    /// Returns "undefined" if no valid Filament enumerant exists.
    .function("getInternalFormat",
            EMBIND_LAMBDA(Texture::InternalFormat, (Ktx1Bundle* self, bool srgb), {
        auto result = Ktx1Reader::toTextureFormat(self->info());
        if (srgb) {
            if (result == Texture::InternalFormat::RGB8) {
                result = Texture::InternalFormat::SRGB8;
            }
            if (result == Texture::InternalFormat::RGBA8) {
                result = Texture::InternalFormat::SRGB8_A8;
            }
        }
        return result;
    }), allow_raw_pointers())

    /// getPixelDataFormat ::method::
    /// ::retval:: [PixelDataFormat]
    /// Returns "undefined" if no valid Filament enumerant exists.
    .function("getPixelDataFormat",
            EMBIND_LAMBDA(backend::PixelDataFormat, (Ktx1Bundle* self), {
        return Ktx1Reader::toPixelDataFormat(self->getInfo());
    }), allow_raw_pointers())

    /// getPixelDataType ::method::
    /// ::retval:: [PixelDataType]
    /// Returns "undefined" if no valid Filament enumerant exists.
    .function("getPixelDataType",
            EMBIND_LAMBDA(backend::PixelDataType, (Ktx1Bundle* self), {
        return Ktx1Reader::toPixelDataType(self->getInfo());
    }), allow_raw_pointers())

    /// getCompressedPixelDataType ::method::
    /// ::retval:: [CompressedPixelDataType]
    /// Returns "undefined" if no valid Filament enumerant exists.
    .function("getCompressedPixelDataType",
            EMBIND_LAMBDA(backend::CompressedPixelDataType, (Ktx1Bundle* self), {
        return Ktx1Reader::toCompressedPixelDataType(self->getInfo());
    }), allow_raw_pointers())

    /// isCompressed ::method::
    /// Per spec, compressed textures in KTX always have their glFormat field set to 0.
    /// ::retval:: boolean
    .function("isCompressed", EMBIND_LAMBDA(bool, (Ktx1Bundle* self), {
        return Ktx1Reader::isCompressed(self->getInfo());
    }), allow_raw_pointers())

    .function("isCubemap", &Ktx1Bundle::isCubemap)
    .function("_getBlob", EMBIND_LAMBDA(BufferDescriptor, (Ktx1Bundle* self, KtxBlobIndex index), {
        uint8_t* data;
        uint32_t size;
        self->getBlob(index, &data, &size);
        return BufferDescriptor(data, size);
    }), allow_raw_pointers())
    .function("_getCubeBlob", EMBIND_LAMBDA(BufferDescriptor,
            (Ktx1Bundle* self, uint32_t miplevel), {
        uint8_t* data;
        uint32_t size;
        self->getBlob({miplevel}, &data, &size);
        return BufferDescriptor(data, size * 6);
    }), allow_raw_pointers())

    /// info ::method:: Obtains properties of the KTX header.
    /// ::retval:: The [KtxInfo] property accessor object.
    .function("info", &Ktx1Bundle::info)

    /// getMetadata ::method:: Obtains arbitrary metadata from the KTX file.
    /// key ::argument:: string
    /// ::retval:: string
    .function("getMetadata", EMBIND_LAMBDA(std::string, (Ktx1Bundle* self, std::string key), {
        return std::string(self->getMetadata(key.c_str()));
    }), allow_raw_pointers());

function("ktx1reader$createTexture", EMBIND_LAMBDA(Texture*,
        (Engine* engine, const Ktx1Bundle& ktx, bool srgb), {
    return Ktx1Reader::createTexture(engine, ktx, srgb, nullptr, nullptr);
}), allow_raw_pointers());

class_<Ktx2Reader>("Ktx2Reader")
    .constructor<Engine&, bool>()
    .function("requestFormat", &Ktx2Reader::requestFormat)
    .function("unrequestFormat", &Ktx2Reader::unrequestFormat)
    .function("load", EMBIND_LAMBDA(Texture*, (Ktx2Reader* self, BufferDescriptor bd,
            Ktx2Reader::TransferFunction transfer), {
        return self->load((uint8_t*) bd.bd->buffer, (uint32_t) bd.bd->size, transfer);
    }), allow_raw_pointers());

/// KtxInfo ::class:: Property accessor for KTX1 header.
/// For example, `Ktx1Bundle.info().pixelWidth`. See the
/// [KTX spec](https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/) for the list of
/// properties.
class_<KtxInfo>("KtxInfo")
    .property("endianness", &KtxInfo::endianness)
    .property("glType", &KtxInfo::glType)
    .property("glTypeSize", &KtxInfo::glTypeSize)
    .property("glFormat", &KtxInfo::glFormat)
    .property("glInternalFormat", &KtxInfo::glInternalFormat)
    .property("glBaseInternalFormat", &KtxInfo::glBaseInternalFormat)
    .property("pixelWidth", &KtxInfo::pixelWidth)
    .property("pixelHeight", &KtxInfo::pixelHeight)
    .property("pixelDepth", &KtxInfo::pixelDepth);

class_<MeshReader::MaterialRegistry>("MeshReader$MaterialRegistry")
    .constructor<>()
    .function("size", &MeshReader::MaterialRegistry::numRegistered)
    .function("get", EMBIND_LAMBDA(val, (MeshReader::MaterialRegistry* self, std::string k), {
          const utils::CString name(k.c_str(), k.size());
          auto i = self->getMaterialInstance(name);
          if (i == nullptr) {
              return val::undefined();
          } else {
              return val(i);
          }
    }), allow_raw_pointers())
    .function("set", EMBIND_LAMBDA(void, (MeshReader::MaterialRegistry* self, std::string k, filament::MaterialInstance* v), {
          const utils::CString name(k.c_str(), k.size());
          self->registerMaterialInstance(name, v);
    }), allow_raw_pointers())
    .function("keys", EMBIND_LAMBDA(std::vector<std::string>, (MeshReader::MaterialRegistry* self), {
        std::vector<utils::CString> names(self->numRegistered());
        self->getRegisteredMaterialNames(names.data());
        std::vector<std::string> result(self->numRegistered());
        for (const auto& name : names) {
            result.emplace_back(name.c_str());
        }
        return result;
    }), allow_raw_pointers());

// MeshReader ::class:: Simple parser for filamesh files.
// JavaScript clients are encouraged to use the [loadFilamesh] helper function instead of using
// this class directly.
class_<MeshReader>("MeshReader")
    // loadMeshFromBuffer ::static method:: Parses a filamesh buffer.
    // engine ::argument:: [Engine]
    // buffer ::argument:: [Buffer]
    // materials ::argument:: [MeshReader$MaterialRegistry]
    // ::retval:: the [MeshReader$Mesh] object
    .class_function("loadMeshFromBuffer", EMBIND_LAMBDA(MeshReader::Mesh,
            (Engine* engine, BufferDescriptor buffer, MeshReader::MaterialRegistry& matreg), {
        // This destruction lambda is called for the vertex buffer AND index buffer, so release
        // CPU memory only after both have been uploaded to the GPU.
        struct Bundle { int count; BufferDescriptor buffer; };
        Bundle* bundle = new Bundle({ .count = 0, .buffer = buffer });
        const auto destructor = [](void* buffer, size_t size, void* user) {
            Bundle* bundle = (Bundle*) user;
            if (++bundle->count == 2) {
                delete bundle;
            }
        };
        // Parse the filamesh buffer. This creates the VB, IB, and renderable.
        return MeshReader::loadMeshFromBuffer(
                engine, buffer.bd->buffer,
                destructor, bundle, matreg);
    }), allow_raw_pointers());

// MeshReader$Mesh ::class:: Property accessor for objects created by [MeshReader].
// This exposes three getter methods: `renderable()`, `vertexBuffer()`, and `indexBuffer()`. These
// are of type [Entity], [VertexBuffer], and [IndexBuffer]. JavaScript clients are encouraged to
// use the [loadFilamesh] helper function instead of using this class directly.
class_<MeshReader::Mesh>("MeshReader$Mesh")
    .function("renderable", EMBIND_LAMBDA(utils::Entity, (MeshReader::Mesh mesh), {
        return mesh.renderable;
    }), allow_raw_pointers())
    .function("vertexBuffer", EMBIND_LAMBDA(VertexBuffer*, (MeshReader::Mesh mesh), {
        return mesh.vertexBuffer;
    }), allow_raw_pointers())
    .function("indexBuffer", EMBIND_LAMBDA(IndexBuffer*, (MeshReader::Mesh mesh), {
        return mesh.indexBuffer;
    }), allow_raw_pointers());

// Clients should call [createTextureFromPng] (et al) rather than using decodeImage directly.

function("decodeImage", &decodeImage);

class_<DecodedImage>("DecodedImage")
    .property("width", &DecodedImage::width)
    .property("height", &DecodedImage::height)
    .property("data", &DecodedImage::decoded_data);

class_<SurfaceBuilder>("SurfaceOrientation$Builder")

    .constructor<>()

    .BUILDER_FUNCTION("vertexCount", SurfaceBuilder, (SurfaceBuilder* builder, size_t nverts), {
        return &builder->vertexCount(nverts);
    })

    .BUILDER_FUNCTION("_normals", SurfaceBuilder, (SurfaceBuilder* builder,
            intptr_t data, int stride), {
        return &builder->normals((const filament::math::float3*) data, stride);
    })

    .BUILDER_FUNCTION("_tangents", SurfaceBuilder, (SurfaceBuilder* builder,
            intptr_t data, int stride), {
        return &builder->tangents((const filament::math::float4*) data, stride);
    })

    .BUILDER_FUNCTION("_uvs", SurfaceBuilder, (SurfaceBuilder* builder, intptr_t data, int stride), {
        return &builder->uvs((const filament::math::float2*) data, stride);
    })

    .BUILDER_FUNCTION("_positions", SurfaceBuilder, (SurfaceBuilder* builder,
            intptr_t data, int stride), {
        return &builder->positions((const filament::math::float3*) data, stride);
    })

    .BUILDER_FUNCTION("triangleCount", SurfaceBuilder, (SurfaceBuilder* builder, size_t n), {
        return &builder->triangleCount(n);
    })

    .BUILDER_FUNCTION("_triangles16", SurfaceBuilder, (SurfaceBuilder* builder, intptr_t data), {
        return &builder->triangles((filament::math::ushort3*) data);
    })

    .BUILDER_FUNCTION("_triangles32", SurfaceBuilder, (SurfaceBuilder* builder, intptr_t data), {
        return &builder->triangles((filament::math::uint3*) data);
    })

    .function("_build", EMBIND_LAMBDA(SurfaceOrientation*, (SurfaceBuilder* builder), {
        return builder->build();
    }), allow_raw_pointers());

class_<SurfaceOrientation>("SurfaceOrientation")
    .function("_getQuats", EMBIND_LAMBDA(void, (SurfaceOrientation* self,
            intptr_t out, size_t quatCount, VertexBuffer::AttributeType attrtype), {
        switch (attrtype) {
            case VertexBuffer::AttributeType::FLOAT4: {
                self->getQuats((filament::math::quatf*) out, quatCount);
                break;
            }
            case VertexBuffer::AttributeType::HALF4: {
                self->getQuats((filament::math::quath*) out, quatCount);
                break;
            }
            case VertexBuffer::AttributeType::SHORT4: {
                self->getQuats((filament::math::short4*) out, quatCount);
                break;
            }
            default:
                utils::slog.e << "Unsupported quaternion type." << utils::io::endl;
        }
    }), allow_raw_pointers());

class_<Animator>("gltfio$Animator")
    .function("applyAnimation", &Animator::applyAnimation)
    .function("updateBoneMatrices", &Animator::updateBoneMatrices)
    .function("applyCrossFade", &Animator::applyCrossFade)
    .function("resetBoneMatrices", &Animator::resetBoneMatrices)
    .function("getAnimationCount", &Animator::getAnimationCount)
    .function("getAnimationDuration", &Animator::getAnimationDuration)
    .function("getAnimationName", EMBIND_LAMBDA(std::string, (Animator* self, size_t index), {
        return std::string(self->getAnimationName(index));
    }), allow_raw_pointers());

class_<FilamentAsset>("gltfio$FilamentAsset")
    .function("_getEntities", EMBIND_LAMBDA(EntityVector, (FilamentAsset* self), {
        const utils::Entity* ptr = self->getEntities();
        return EntityVector(ptr, ptr + self->getEntityCount());
    }), allow_raw_pointers())

    .function("_getEntitiesByName", EMBIND_LAMBDA(EntityVector, (FilamentAsset* self, std::string name), {
        EntityVector result(self->getEntitiesByName(name.c_str(), nullptr, 0));
        self->getEntitiesByName(name.c_str(), result.data(), result.size());
        return result;
    }), allow_raw_pointers())

    .function("_getEntitiesByPrefix", EMBIND_LAMBDA(EntityVector, (FilamentAsset* self, std::string prefix), {
        EntityVector result(self->getEntitiesByPrefix(prefix.c_str(), nullptr, 0));
        self->getEntitiesByPrefix(prefix.c_str(), result.data(), result.size());
        return result;
    }), allow_raw_pointers())

    .function("getFirstEntityByName", EMBIND_LAMBDA(utils::Entity, (FilamentAsset* self, std::string name), {
        return self->getFirstEntityByName(name.c_str());
    }), allow_raw_pointers())

    .function("_getLightEntities", EMBIND_LAMBDA(EntityVector, (FilamentAsset* self), {
        const utils::Entity* ptr = self->getLightEntities();
        return EntityVector(ptr, ptr + self->getLightEntityCount());
    }), allow_raw_pointers())

    .function("_getRenderableEntities", EMBIND_LAMBDA(EntityVector, (FilamentAsset* self), {
        const utils::Entity* ptr = self->getRenderableEntities();
        return EntityVector(ptr, ptr + self->getRenderableEntityCount());
    }), allow_raw_pointers())

    .function("_getCameraEntities", EMBIND_LAMBDA(EntityVector, (FilamentAsset* self), {
        const utils::Entity* ptr = self->getCameraEntities();
        return EntityVector(ptr, ptr + self->getCameraEntityCount());
    }), allow_raw_pointers())

    .function("getRoot", &FilamentAsset::getRoot)

    .function("popRenderable", &FilamentAsset::popRenderable)

    .function("getInstance", &FilamentAsset::getInstance, allow_raw_pointers())

    .function("_getAssetInstances", EMBIND_LAMBDA(std::vector<FilamentInstance*>,
            (FilamentAsset* self), {
        FilamentInstance** ptr = self->getAssetInstances();
        return std::vector<FilamentInstance*>(ptr, ptr + self->getAssetInstanceCount());
    }), allow_raw_pointers())

    .function("_getResourceUris", EMBIND_LAMBDA(std::vector<std::string>, (FilamentAsset* self), {
        std::vector<std::string> retval;
        auto uris = self->getResourceUris();
        for (size_t i = 0, len = self->getResourceUriCount(); i < len; ++i) {
            retval.push_back(uris[i]);
        }
        return retval;
    }), allow_raw_pointers())

    .function("getBoundingBox", &FilamentAsset::getBoundingBox)
    .function("getName", EMBIND_LAMBDA(std::string, (FilamentAsset* self, utils::Entity entity), {
        return std::string(self->getName(entity));
    }), allow_raw_pointers())
    .function("getExtras", EMBIND_LAMBDA(std::string, (FilamentAsset* self, utils::Entity entity), {
        return std::string(self->getExtras(entity));
    }), allow_raw_pointers())
    .function("getWireframe", &FilamentAsset::getWireframe)
    .function("getEngine", &FilamentAsset::getEngine, allow_raw_pointers())
    .function("releaseSourceData", &FilamentAsset::releaseSourceData);

class_<FilamentInstance>("gltfio$FilamentInstance")
    .function("getAsset", &FilamentInstance::getAsset, allow_raw_pointers())
    .function("getEntities", EMBIND_LAMBDA(EntityVector, (FilamentInstance* self), {
        const utils::Entity* ptr = self->getEntities();
        return EntityVector(ptr, ptr + self->getEntityCount());
    }), allow_raw_pointers())
    .function("getRoot", &FilamentInstance::getRoot)

    .function("getAnimator", &FilamentInstance::getAnimator, allow_raw_pointers())

    .function("getSkinNames", EMBIND_LAMBDA(std::vector<std::string>, (FilamentInstance* self), {
        std::vector<std::string> names(self->getSkinCount());
        for (size_t i = 0; i < names.size(); ++i) {
            names[i] = self->getSkinNameAt(i);
        }
        return names;
    }), allow_raw_pointers())

    .function("attachSkin", &FilamentInstance::attachSkin)
    .function("detachSkin", &FilamentInstance::detachSkin)

    .function("applyMaterialVariant", &FilamentInstance::applyMaterialVariant)

    .function("getMaterialInstances", EMBIND_LAMBDA(std::vector<MaterialInstance*>,
            (FilamentInstance* self), {
        MaterialInstance* const* ptr = self->getMaterialInstances();
        return std::vector<MaterialInstance*>(ptr, ptr + self->getMaterialInstanceCount());
    }), allow_raw_pointers())

    .function("_getMaterialVariantNames", EMBIND_LAMBDA(std::vector<std::string>, (FilamentInstance* self), {
        std::vector<std::string> retval(self->getMaterialVariantCount());
        for (size_t i = 0, len = retval.size(); i < len; ++i) {
            retval[i] = self->getMaterialVariantName(i);
        }
        return retval;
    }), allow_raw_pointers());

// These little wrappers exist to get around RTTI requirements in embind.

struct UbershaderProvider {
    MaterialProvider* provider;
    void destroyMaterials() { provider->destroyMaterials(); }
};

struct StbProvider { TextureProvider* provider; };
struct Ktx2Provider { TextureProvider* provider; };

class_<UbershaderProvider>("gltfio$UbershaderProvider")
    .constructor(EMBIND_LAMBDA(UbershaderProvider, (Engine* engine), {
        return UbershaderProvider { createUbershaderProvider(engine,
                UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE) };
    }))
    .function("destroyMaterials", &UbershaderProvider::destroyMaterials);

class_<StbProvider>("gltfio$StbProvider")
    .constructor(EMBIND_LAMBDA(StbProvider, (Engine* engine), {
        return StbProvider { createStbProvider(engine) };
    }));

class_<Ktx2Provider>("gltfio$Ktx2Provider")
    .constructor(EMBIND_LAMBDA(Ktx2Provider, (Engine* engine), {
        return Ktx2Provider { createKtx2Provider(engine) };
    }));

class_<AssetLoader>("gltfio$AssetLoader")

    .constructor(EMBIND_LAMBDA(AssetLoader*, (Engine* engine, UbershaderProvider materials), {
        auto names = new utils::NameComponentManager(utils::EntityManager::get());
        return AssetLoader::create({ engine, materials.provider, names });
    }), allow_raw_pointers())

    /// createAsset ::method::
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// ::retval:: an instance of [FilamentAsset]
    .function("_createAsset", EMBIND_LAMBDA(FilamentAsset*,
            (AssetLoader* self, BufferDescriptor buffer), {
        return self->createAsset((const uint8_t*) buffer.bd->buffer, buffer.bd->size);
    }), allow_raw_pointers())

    /// createInstancedAsset ::method::
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// ::retval:: an instance of [FilamentAsset]
    .function("_createInstancedAsset", EMBIND_LAMBDA(FilamentAsset*,
            (AssetLoader* self, BufferDescriptor buffer, int numInstances), {
        // Ignore the returned instances, they can be extracted from the asset.
        std::vector<FilamentInstance*> instances;
        return self->createInstancedAsset((const uint8_t*) buffer.bd->buffer,
                buffer.bd->size, instances.data(), numInstances);
    }), allow_raw_pointers())

    // createInstance ::method::
    // Adds a new instance to an instanced asset.
    .function("createInstance", &AssetLoader::createInstance, allow_raw_pointers())

    // destroyAsset ::method::
    // Destroys the given asset and all of its associated Filament objects. This includes
    // components, material instances, vertex buffers, index buffers, and textures.
    // asset ::argument:: the Filament asset created using AssetLoader
    .function("destroyAsset", &AssetLoader::destroyAsset, allow_raw_pointers());

class_<ResourceLoader>("gltfio$ResourceLoader")
    .constructor(EMBIND_LAMBDA(ResourceLoader*, (Engine* engine, bool normalizeSkinningWeights), {
        return new ResourceLoader({
            .engine = engine,
            .gltfPath = nullptr,
            .normalizeSkinningWeights = normalizeSkinningWeights
        });
    }), allow_raw_pointers())

    .function("addResourceData", EMBIND_LAMBDA(void, (ResourceLoader* self, std::string url,
            BufferDescriptor buffer), {
        self->addResourceData(url.c_str(), std::move(*buffer.bd));
    }), allow_raw_pointers())

    .function("addStbProvider", EMBIND_LAMBDA(void, (ResourceLoader* self, std::string mime,
            StbProvider provider), {
        self->addTextureProvider(mime.c_str(), provider.provider);
    }), allow_raw_pointers())

    .function("addKtx2Provider", EMBIND_LAMBDA(void, (ResourceLoader* self, std::string mime,
            Ktx2Provider provider), {
        self->addTextureProvider(mime.c_str(), provider.provider);
    }), allow_raw_pointers())

    .function("hasResourceData", EMBIND_LAMBDA(bool, (ResourceLoader* self, std::string url), {
        return self->hasResourceData(url.c_str());
    }), allow_raw_pointers())

    .function("loadResources", EMBIND_LAMBDA(bool, (ResourceLoader* self, FilamentAsset* asset), {
        return self->loadResources(asset);
    }), allow_raw_pointers())

    .function("asyncBeginLoad", EMBIND_LAMBDA(bool, (ResourceLoader* self, FilamentAsset* asset), {
        return self->asyncBeginLoad(asset);
    }), allow_raw_pointers())

    .function("asyncGetLoadProgress", &ResourceLoader::asyncGetLoadProgress)
    .function("asyncUpdateLoad", &ResourceLoader::asyncUpdateLoad);

class_<Settings>("Settings");

class_<JsonSerializer>("JsonSerializer")
    .constructor<>()
    .function("writeJson", &JsonSerializer::writeJson);

class_<ViewerGui>("ViewerGui")
    .constructor<Engine*, Scene*, View*, int>()
    .function("renderUserInterface", &ViewerGui::renderUserInterface, allow_raw_pointers())
    .function("getSettings", &ViewerGui::getSettings)
    .function("mouseEvent", &ViewerGui::mouseEvent)
    .function("keyDownEvent", &ViewerGui::keyDownEvent)
    .function("keyUpEvent", &ViewerGui::keyUpEvent)
    .function("keyPressEvent", &ViewerGui::keyPressEvent);

function("fitIntoUnitCube", EMBIND_LAMBDA(flatmat4, (Aabb box, float zoffset), {
    return flatmat4 { fitIntoUnitCube(box, zoffset) };
}));

function("multiplyMatrices", EMBIND_LAMBDA(flatmat4, (flatmat4 a, flatmat4 b), {
    return flatmat4 { a.m * b.m };
}));

} // EMSCRIPTEN_BINDINGS

#pragma clang diagnostic pop
