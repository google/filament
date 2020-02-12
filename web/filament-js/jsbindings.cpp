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

#include <filament/Camera.h>
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

#include <gltfio/Animator.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/Image.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/ResourceLoader.h>

#include <image/KtxBundle.h>
#include <image/KtxUtility.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat4.h>

#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>
#include <utils/Log.h>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace filament;
using namespace filamesh;
using namespace geometry;
using namespace gltfio;
using namespace image;

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
// that have non-public destructors.
#define BIND(T) template<> void raw_destructor<T>(T* ptr) {}
namespace emscripten {
    namespace internal {
        BIND(Animator)
        BIND(AssetLoader)
        BIND(Camera)
        BIND(Engine)
        BIND(FilamentAsset)
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
        BIND(utils::Entity)
        BIND(utils::EntityManager)
        BIND(VertexBuffer)
        BIND(View)
    }
}
#undef BIND

namespace {

// For convenience, declare terse private aliases to nested types. This lets us avoid extremely
// verbose binding declarations.
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

// CORE FILAMENT CLASSES
// ---------------------

/// Engine ::core class:: Central manager and resource owner.
class_<Engine>("Engine")
    .class_function("_create", (Engine* (*)()) [] { return Engine::create(); },
            allow_raw_pointers())
    /// destroy ::static method:: Destroys an engine instance and cleans up resources.
    /// engine ::argument:: the instance to destroy
    .class_function("destroy", (void (*)(Engine*)) []
            (Engine* engine) { Engine::destroy(&engine); }, allow_raw_pointers())
    .function("execute", &Engine::execute)

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
    /// ::retval:: an instance of [Camera]
    .function("createCamera", select_overload<Camera*(void)>(&Engine::createCamera),
            allow_raw_pointers())
    /// destroyCamera ::method::
    /// camera ::argument:: an instance of [Camera]
    .function("destroyCamera", (void (*)(Engine*, Camera*)) []
            (Engine* engine, Camera* camera) { engine->destroy(camera); },
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
            allow_raw_pointers());

/// SwapChain ::core class:: Represents the platform's native rendering surface.
/// See also the [Engine] methods `createSwapChain` and `destroySwapChain`.
class_<SwapChain>("SwapChain");

/// Renderer ::core class:: Represents the platform's native window.
/// See also the [Engine] methods `createRenderer` and `destroyRenderer`.
class_<Renderer>("Renderer")
    .function("renderView", &Renderer::render, allow_raw_pointers())
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
    .function("beginFrame", &Renderer::beginFrame, allow_raw_pointers())
    .function("endFrame", &Renderer::endFrame, allow_raw_pointers());

/// View ::core class:: Encompasses all the state needed for rendering a Scene.
/// A view is associated with a particular [Scene], [Camera], and viewport.
/// See also the [Engine] methods `createView` and `destroyView`.
class_<View>("View")
    .function("setScene", &View::setScene, allow_raw_pointers())
    .function("setCamera", &View::setCamera, allow_raw_pointers())
    .function("getViewport", &View::getViewport)
    .function("setViewport", &View::setViewport)
    .function("setClearColor", &View::setClearColor)
    .function("setPostProcessingEnabled", &View::setPostProcessingEnabled)
    .function("setAntiAliasing", &View::setAntiAliasing)
    .function("getAntiAliasing", &View::getAntiAliasing)
    .function("setSampleCount", &View::setSampleCount)
    .function("setRenderTarget", EMBIND_LAMBDA(void, (View* self, RenderTarget* renderTarget), {
        self->setRenderTarget(renderTarget);
    }), allow_raw_pointers());

/// Scene ::core class:: Flat container of renderables and lights.
/// See also the [Engine] methods `createScene` and `destroyScene`.
class_<Scene>("Scene")
    .function("addEntity", &Scene::addEntity)

    .function("addEntities", EMBIND_LAMBDA(void,
            (Scene* self, std::vector<utils::Entity> entities), {
        self->addEntities(entities.data(), entities.size());
    }), allow_raw_pointers())

    .function("hasEntity", &Scene::hasEntity)
    .function("remove", &Scene::remove)
    .function("setSkybox", &Scene::setSkybox, allow_raw_pointers())
    .function("setIndirectLight", &Scene::setIndirectLight, allow_raw_pointers())
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

    .function("getProjectionMatrix", EMBIND_LAMBDA(flatmat4, (Camera* self), {
        return flatmat4 { filament::math::mat4f(self->getProjectionMatrix()) };
    }), allow_raw_pointers())

    .function("getCullingProjectionMatrix", EMBIND_LAMBDA(flatmat4, (Camera* self), {
        return flatmat4 { filament::math::mat4f(self->getCullingProjectionMatrix()) };
    }), allow_raw_pointers())

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
        return flatmat4 { self->getModelMatrix() };
    }), allow_raw_pointers())

    .function("getViewMatrix", EMBIND_LAMBDA(flatmat4, (Camera* self), {
        return flatmat4 { self->getViewMatrix() };
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

    .class_function("inverseProjection",  (flatmat4 (*)(flatmat4)) [] (flatmat4 m) {
        return flatmat4 { filament::math::mat4f(Camera::inverseProjection(m.m)) };
    }, allow_raw_pointers());

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

    .BUILDER_FUNCTION("culling", RenderableBuilder, (RenderableBuilder* builder, bool enable), {
        return &builder->culling(enable); })

    .BUILDER_FUNCTION("castShadows", RenderableBuilder, (RenderableBuilder* builder, bool enable), {
        return &builder->castShadows(enable); })

    .BUILDER_FUNCTION("receiveShadows", RenderableBuilder, (RenderableBuilder* builder, bool enable), {
        return &builder->receiveShadows(enable); })

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
    })

    .function("destroy", &RenderableManager::destroy)
    .function("setAxisAlignedBoundingBox", &RenderableManager::setAxisAlignedBoundingBox)
    .function("setLayerMask", &RenderableManager::setLayerMask)
    .function("setPriority", &RenderableManager::setPriority)
    .function("setCastShadows", &RenderableManager::setCastShadows)
    .function("setReceiveShadows", &RenderableManager::setReceiveShadows)
    .function("isShadowCaster", &RenderableManager::isShadowCaster)
    .function("isShadowReceiver", &RenderableManager::isShadowReceiver)

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

    // NOTE: this cannot take a float4 due to a binding issue.
    .function("setMorphWeights", EMBIND_LAMBDA(void, (RenderableManager* self,
            RenderableManager::Instance instance, float x, float y, float z, float w), {
        self->setMorphWeights(instance, {x, y, z, w});
    }), allow_raw_pointers())

    .function("getAxisAlignedBoundingBox", &RenderableManager::getAxisAlignedBoundingBox)
    .function("getPrimitiveCount", &RenderableManager::getPrimitiveCount)
    .function("setMaterialInstanceAt", &RenderableManager::setMaterialInstanceAt,
            allow_raw_pointers())
    .function("getMaterialInstanceAt", &RenderableManager::getMaterialInstanceAt,
            allow_raw_pointers())

    .function("setGeometryAt", EMBIND_LAMBDA(void, (RenderableManager* self,
            RenderableManager::Instance instance, size_t primitiveIndex,
            RenderableManager::PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices,
            size_t offset, size_t count), {
        self->setGeometryAt(instance, primitiveIndex, type, vertices, indices, offset, count);
    }), allow_raw_pointers())

    .function("setGeometryRangeAt", EMBIND_LAMBDA(void, (RenderableManager* self,
            RenderableManager::Instance instance, size_t primitiveIndex,
            RenderableManager::PrimitiveType type, size_t offset, size_t count), {
        self->setGeometryAt(instance, primitiveIndex, type, offset, count);
    }), allow_raw_pointers())

    .function("setBlendOrderAt", &RenderableManager::setBlendOrderAt)

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

    .function("getChidren", EMBIND_LAMBDA(std::vector<utils::Entity>,
            (TransformManager* self, TransformManager::Instance instance), {
        std::vector<utils::Entity> result(self->getChildCount(instance));
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
        return flatmat4 { self->getTransform(instance) } ; }), allow_raw_pointers())

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
            (LightBuilder* builder, float value), { return &builder->sunHaloFalloff(value); });

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
    .function("setSpotLightCone", &LightManager::setSpotLightCone)
    .function("setSunAngularRadius", &LightManager::setSunAngularRadius)
    .function("getSunAngularRadius", &LightManager::getSunAngularRadius)
    .function("setSunHaloSize", &LightManager::setSunHaloSize)
    .function("getSunHaloSize", &LightManager::getSunHaloSize)
    .function("setSunHaloFalloff", &LightManager::setSunHaloFalloff)
    .function("getSunHaloFalloff", &LightManager::getSunHaloFalloff)
    .function("setShadowCaster", &LightManager::setShadowCaster)
    .function("isShadowCaster", &LightManager::isShadowCaster)
    ;

/// LightManager$Instance ::class:: Component instance returned by [LightManager]
/// Be sure to call the instance's `delete` method when you're done with it.
class_<LightManager::Instance>("LightManager$Instance");
    /// delete ::method:: Frees an instance obtained via `getInstance`

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
    .function("_setBufferAt", EMBIND_LAMBDA(void, (VertexBuffer* self,
            Engine* engine, uint8_t bufferIndex, BufferDescriptor vbd), {
        self->setBufferAt(*engine, bufferIndex, std::move(*vbd.bd));
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
            Engine* engine, BufferDescriptor ibd), {
        self->setBuffer(*engine, std::move(*ibd.bd));
    }), allow_raw_pointers());

class_<Material>("Material")
    .function("getDefaultInstance",
            select_overload<MaterialInstance*(void)>(&Material::getDefaultInstance),
            allow_raw_pointers())
    .function("createInstance", &Material::createInstance, allow_raw_pointers())
    .function("getName", EMBIND_LAMBDA(std::string, (Material* self), {
        return std::string(self->getName());
    }), allow_raw_pointers());

class_<MaterialInstance>("MaterialInstance")
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
    .function("setDoubleSided", &MaterialInstance::setDoubleSided)
    .function("setCullingMode", &MaterialInstance::setCullingMode)
    .function("setColorWrite", &MaterialInstance::setColorWrite)
    .function("setDepthWrite", &MaterialInstance::setDepthWrite)
    .function("setDepthCulling", &MaterialInstance::setDepthCulling);

class_<TextureSampler>("TextureSampler")
    .constructor<backend::SamplerMinFilter, backend::SamplerMagFilter, backend::SamplerWrapMode>();

/// Texture ::core class:: 2D image or cubemap that can be sampled by the GPU, possibly mipmapped.
class_<Texture>("Texture")
    .class_function("Builder", (TexBuilder (*)()) [] { return TexBuilder(); })
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
    .BUILDER_FUNCTION("usage", TexBuilder, (TexBuilder* builder, Texture::Usage usage), {
        return &builder->usage(usage); });

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
    .class_function("Builder", (SkyBuilder (*)()) [] { return SkyBuilder(); });

class_<SkyBuilder>("Skybox$Builder")
    .function("_build", EMBIND_LAMBDA(Skybox*, (SkyBuilder* builder, Engine* engine), {
        return builder->build(*engine);
    }), allow_raw_pointers())
    .BUILDER_FUNCTION("environment", SkyBuilder, (SkyBuilder* builder, Texture* cubemap), {
        return &builder->environment(cubemap); })
    .BUILDER_FUNCTION("showSun", SkyBuilder, (SkyBuilder* builder, bool show), {
        return &builder->showSun(show); });

// UTILS TYPES
// -----------

/// Entity ::core class:: Handle to an object consisting of a set of components.
/// To create an entity with no components, use [EntityManager].
class_<utils::Entity>("Entity")
    .function("getId", &utils::Entity::getId);

/// EntityManager ::core class:: Singleton used for constructing entities in Filament's ECS.
class_<utils::EntityManager>("EntityManager")
    /// get ::static method:: Gets the singleton entity manager instance.
    /// ::retval:: the one and only entity manager
    .class_function("get", (utils::EntityManager* (*)()) []
        { return &utils::EntityManager::get(); }, allow_raw_pointers())
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

/// KtxBundle ::class:: In-memory representation of a KTX file.
/// Most clients should use one of the `create*FromKtx` utility methods in the JavaScript [Engine]
/// wrapper rather than interacting with `KtxBundle` directly.
class_<KtxBundle>("KtxBundle")
    .constructor(EMBIND_LAMBDA(KtxBundle*, (BufferDescriptor kbd), {
        return new KtxBundle((uint8_t*) kbd.bd->buffer, (uint32_t) kbd.bd->size);
    }))

    /// info ::method:: Obtains properties of the KTX header.
    /// ::retval:: The [KtxInfo] property accessor object.
    .function("getNumMipLevels", &KtxBundle::getNumMipLevels)

    /// getArrayLength ::method:: Obtains length of the texture array.
    /// ::retval:: The number of elements in the texture array
    .function("getArrayLength", &KtxBundle::getArrayLength)

    /// getInternalFormat ::method::
    /// srgb ::argument:: boolean that forces the resulting format to SRGB if possible.
    /// ::retval:: [Texture$InternalFormat]
    /// Returns "undefined" if no valid Filament enumerant exists.
    .function("getInternalFormat",
            EMBIND_LAMBDA(Texture::InternalFormat, (KtxBundle* self, bool srgb), {
        auto result = ktx::toTextureFormat(self->info());
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
            EMBIND_LAMBDA(backend::PixelDataFormat, (KtxBundle* self), {
        return ktx::toPixelDataFormat(self->getInfo());
    }), allow_raw_pointers())

    /// getPixelDataType ::method::
    /// ::retval:: [PixelDataType]
    /// Returns "undefined" if no valid Filament enumerant exists.
    .function("getPixelDataType",
            EMBIND_LAMBDA(backend::PixelDataType, (KtxBundle* self), {
        return ktx::toPixelDataType(self->getInfo());
    }), allow_raw_pointers())

    /// getCompressedPixelDataType ::method::
    /// ::retval:: [CompressedPixelDataType]
    /// Returns "undefined" if no valid Filament enumerant exists.
    .function("getCompressedPixelDataType",
            EMBIND_LAMBDA(backend::CompressedPixelDataType, (KtxBundle* self), {
        return ktx::toCompressedPixelDataType(self->getInfo());
    }), allow_raw_pointers())

    /// isCompressed ::method::
    /// Per spec, compressed textures in KTX always have their glFormat field set to 0.
    /// ::retval:: boolean
    .function("isCompressed", EMBIND_LAMBDA(bool, (KtxBundle* self), {
        return ktx::isCompressed(self->getInfo());
    }), allow_raw_pointers())

    .function("isCubemap", &KtxBundle::isCubemap)
    .function("_getBlob", EMBIND_LAMBDA(BufferDescriptor, (KtxBundle* self, KtxBlobIndex index), {
        uint8_t* data;
        uint32_t size;
        self->getBlob(index, &data, &size);
        return BufferDescriptor(data, size);
    }), allow_raw_pointers())
    .function("_getCubeBlob", EMBIND_LAMBDA(BufferDescriptor,
            (KtxBundle* self, uint32_t miplevel), {
        uint8_t* data;
        uint32_t size;
        self->getBlob({miplevel}, &data, &size);
        return BufferDescriptor(data, size * 6);
    }), allow_raw_pointers())

    /// info ::method:: Obtains properties of the KTX header.
    /// ::retval:: The [KtxInfo] property accessor object.
    .function("info", &KtxBundle::info)

    /// getMetadata ::method:: Obtains arbitrary metadata from the KTX file.
    /// key ::argument:: string
    /// ::retval:: string
    .function("getMetadata", EMBIND_LAMBDA(std::string, (KtxBundle* self, std::string key), {
        return std::string(self->getMetadata(key.c_str()));
    }), allow_raw_pointers());

function("ktx$createTexture", EMBIND_LAMBDA(Texture*,
        (Engine* engine, const KtxBundle& ktx, bool srgb), {
    return ktx::createTexture(engine, ktx, srgb, nullptr, nullptr);
}), allow_raw_pointers());

/// KtxInfo ::class:: Property accessor for KTX header.
/// For example, `ktxbundle.info().pixelWidth`. See the
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

register_vector<std::string>("RegistryKeys");
register_vector<utils::Entity>("EntityVector");

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

    .BUILDER_FUNCTION("normals", SurfaceBuilder, (SurfaceBuilder* builder,
            intptr_t data, int stride), {
        return &builder->normals((const filament::math::float3*) data, stride);
    })

    .BUILDER_FUNCTION("tangents", SurfaceBuilder, (SurfaceBuilder* builder,
            intptr_t data, int stride), {
        return &builder->tangents((const filament::math::float4*) data, stride);
    })

    .BUILDER_FUNCTION("uvs", SurfaceBuilder, (SurfaceBuilder* builder, intptr_t data, int stride), {
        return &builder->uvs((const filament::math::float2*) data, stride);
    })

    .BUILDER_FUNCTION("positions", SurfaceBuilder, (SurfaceBuilder* builder,
            intptr_t data, int stride), {
        return &builder->positions((const filament::math::float3*) data, stride);
    })

    .BUILDER_FUNCTION("triangleCount", SurfaceBuilder, (SurfaceBuilder* builder, size_t n), {
        return &builder->triangleCount(n);
    })

    .BUILDER_FUNCTION("triangles16", SurfaceBuilder, (SurfaceBuilder* builder, intptr_t data), {
        return &builder->triangles((filament::math::ushort3*) data);
    })

    .BUILDER_FUNCTION("triangles32", SurfaceBuilder, (SurfaceBuilder* builder, intptr_t data), {
        return &builder->triangles((filament::math::uint3*) data);
    })

    .function("_build", EMBIND_LAMBDA(SurfaceOrientation*, (SurfaceBuilder* builder), {
        return builder->build();
    }), allow_raw_pointers());

class_<SurfaceOrientation>("SurfaceOrientation")
    .function("getQuats", EMBIND_LAMBDA(void, (SurfaceOrientation* self,
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
    .function("getAnimationCount", &Animator::getAnimationCount)
    .function("getAnimationDuration", &Animator::getAnimationDuration)
    .function("getAnimationName", EMBIND_LAMBDA(std::string, (Animator* self, size_t index), {
        return std::string(self->getAnimationName(index));
    }), allow_raw_pointers());

class_<FilamentAsset>("gltfio$FilamentAsset")
    .function("getEntities", EMBIND_LAMBDA(std::vector<utils::Entity>, (FilamentAsset* self), {
        const utils::Entity* ptr = self->getEntities();
        return std::vector<utils::Entity>(ptr, ptr + self->getEntityCount());
    }), allow_raw_pointers())

    .function("getRoot", &FilamentAsset::getRoot)

    .function("popRenderable", &FilamentAsset::popRenderable)

    .function("getMaterialInstances", EMBIND_LAMBDA(std::vector<const MaterialInstance*>,
            (FilamentAsset* self), {
        const filament::MaterialInstance* const* ptr = self->getMaterialInstances();
        return std::vector<const MaterialInstance*>(ptr, ptr + self->getMaterialInstanceCount());
    }), allow_raw_pointers())

    .function("getResourceUris", EMBIND_LAMBDA(std::vector<std::string>, (FilamentAsset* self), {
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
    .function("getAnimator", &FilamentAsset::getAnimator, allow_raw_pointers())
    .function("getWireframe", &FilamentAsset::getWireframe)
    .function("getEngine", &FilamentAsset::getEngine, allow_raw_pointers())
    .function("releaseSourceData", &FilamentAsset::releaseSourceData);

// This little wrapper exists to get around RTTI requirements in embind.
struct UbershaderLoader {
    MaterialProvider* provider;
    void destroyMaterials() { provider->destroyMaterials(); }
};

class_<UbershaderLoader>("gltfio$UbershaderLoader")
    .constructor(EMBIND_LAMBDA(UbershaderLoader, (Engine* engine), {
        return UbershaderLoader { createUbershaderLoader(engine) };
    }))
    .function("destroyMaterials", &UbershaderLoader::destroyMaterials);

class_<AssetLoader>("gltfio$AssetLoader")

    .constructor(EMBIND_LAMBDA(AssetLoader*, (Engine* engine, UbershaderLoader materials), {
        auto names = new utils::NameComponentManager(utils::EntityManager::get());
        return AssetLoader::create({ engine, materials.provider, names });
    }), allow_raw_pointers())

    /// createAssetFromJson ::static method::
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// ::retval:: an instance of [FilamentAsset]
    .function("_createAssetFromJson", EMBIND_LAMBDA(FilamentAsset*,
            (AssetLoader* self, BufferDescriptor buffer), {
        return self->createAssetFromJson((const uint8_t*) buffer.bd->buffer, buffer.bd->size);
    }), allow_raw_pointers())

    /// createAssetFroBinary ::static method::
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// ::retval:: an instance of [FilamentAsset]
    .function("_createAssetFromBinary", EMBIND_LAMBDA(FilamentAsset*,
            (AssetLoader* self, BufferDescriptor buffer), {
        return self->createAssetFromBinary((const uint8_t*) buffer.bd->buffer, buffer.bd->size);
    }), allow_raw_pointers());

class_<ResourceLoader>("gltfio$ResourceLoader")
    .constructor(EMBIND_LAMBDA(ResourceLoader*, (Engine* engine), {
        return new ResourceLoader({
            .engine = engine,
            .gltfPath = nullptr,
            .normalizeSkinningWeights = true,
            .recomputeBoundingBoxes = true
        });
    }), allow_raw_pointers())

    .function("addResourceData", EMBIND_LAMBDA(void, (ResourceLoader* self, std::string url,
            BufferDescriptor buffer), {
        self->addResourceData(url.c_str(), std::move(*buffer.bd));
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

} // EMSCRIPTEN_BINDINGS
