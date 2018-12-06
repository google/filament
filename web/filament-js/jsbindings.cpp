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
#include <filament/IndexBuffer.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/SwapChain.h>
#include <filament/Texture.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <image/KtxBundle.h>
#include <image/KtxUtility.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat4.h>

#include <utils/EntityManager.h>

#include <emscripten.h>
#include <emscripten/bind.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include <stb_image.h>

using namespace emscripten;
using namespace filament;
using namespace image;

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
        BIND(Engine)
        BIND(SwapChain)
        BIND(Renderer)
        BIND(View)
        BIND(Scene)
        BIND(Camera)
        BIND(LightManager)
        BIND(RenderableManager)
        BIND(TransformManager)
        BIND(VertexBuffer)
        BIND(IndexBuffer)
        BIND(IndirectLight)
        BIND(Material)
        BIND(MaterialInstance)
        BIND(Skybox)
        BIND(Texture)
        BIND(utils::Entity)
        BIND(utils::EntityManager)
    }
}
#undef BIND

namespace {

// For convenience, declare terse private aliases to nested types. This lets us avoid extremely
// verbose binding declarations.
using RenderBuilder = RenderableManager::Builder;
using VertexBuilder = VertexBuffer::Builder;
using IndexBuilder = IndexBuffer::Builder;
using MatBuilder = Material::Builder;
using TexBuilder = Texture::Builder;
using LightBuilder = LightManager::Builder;
using IblBuilder = IndirectLight::Builder;
using SkyBuilder = Skybox::Builder;

// We avoid directly exposing driver::BufferDescriptor because embind does not support move
// semantics and void* doesn't make sense to JavaScript anyway. This little wrapper class is exposed
// to JavaScript as "driver$BufferDescriptor", but clients will normally use our "Filament.Buffer"
// helper function (implemented in utilities.js)
struct BufferDescriptor {
    BufferDescriptor() {}
    // This form is used when JavaScript sends a buffer into WASM.
    BufferDescriptor(val arrdata) {
        auto byteLength = arrdata["byteLength"].as<uint32_t>();
        this->bd.reset(new driver::BufferDescriptor(malloc(byteLength), byteLength,
                [](void* buffer, size_t size, void* user) { free(buffer); }));
    }
    // This form is used when WASM needs to return a buffer to JavaScript.
    BufferDescriptor(uint8_t* data, uint32_t size) {
        this->bd.reset(new driver::BufferDescriptor(data, size));
    }
    val getBytes() {
        unsigned char *byteBuffer = (unsigned char*) bd->buffer;
        size_t bufferLength = bd->size;
        return val(typed_memory_view(bufferLength, byteBuffer));
    }
    // In order to match its JavaScript counterpart, the Buffer wrapper needs to use reference
    // counting, and the easiest way to achieve that is with shared_ptr.
    std::shared_ptr<driver::BufferDescriptor> bd;
};

// Exposed to JavaScript as "driver$PixelBufferDescriptor", but clients will normally use the
// PixelBuffer or CompressedPixelBuffer helper functions (implemented in utilities.js)
struct PixelBufferDescriptor {
    PixelBufferDescriptor(val arrdata, driver::PixelDataFormat fmt, driver::PixelDataType dtype) {
        auto byteLength = arrdata["byteLength"].as<uint32_t>();
        this->pbd.reset(new driver::PixelBufferDescriptor(malloc(byteLength), byteLength,
                fmt, dtype, [](void* buffer, size_t size, void* user) { free(buffer); }));
    }
    // Note that embind allows overloading based on number of arguments, but not on types.
    // It's fine to have two constructors but they can't both have the same number of arguments.
    PixelBufferDescriptor(val arrdata, driver::CompressedPixelDataType cdtype, int imageSize,
            bool compressed) {
        auto byteLength = arrdata["byteLength"].as<uint32_t>();
        assert(compressed == true);
        // For compressed cubemaps, the image size should be one-sixth the size of the entire blob.
        assert(imageSize == byteLength || imageSize == byteLength / 6);
        this->pbd.reset(new driver::PixelBufferDescriptor(malloc(byteLength), byteLength,
                cdtype, imageSize, [](void* buffer, size_t size, void* user) { free(buffer); }));
    }
    val getBytes() {
        unsigned char *byteBuffer = (unsigned char*) pbd->buffer;
        size_t bufferLength = pbd->size;
        return val(typed_memory_view(bufferLength, byteBuffer));
    };
    // In order to match its JavaScript counterpart, the Buffer wrapper needs to use reference
    // counting, and the easiest way to achieve that is with shared_ptr.
    std::shared_ptr<driver::PixelBufferDescriptor> pbd;
};

// Small structure whose sole purpose is to return decoded image data to JavaScript.
struct DecodedPng {
    int width;
    int height;
    int encoded_ncomp;
    int decoded_ncomp;
    BufferDescriptor decoded_data;
};

// JavaScript clients should call [createTextureFromPng] rather than calling this directly.
DecodedPng decodePng(BufferDescriptor encoded_data, int requested_ncomp) {
    DecodedPng result;
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

value_array<math::float2>("float2")
    .element(&math::float2::x)
    .element(&math::float2::y);

value_array<math::float3>("float3")
    .element(&math::float3::x)
    .element(&math::float3::y)
    .element(&math::float3::z);

value_array<math::float4>("float4")
    .element(&math::float4::x)
    .element(&math::float4::y)
    .element(&math::float4::z)
    .element(&math::float4::w);

value_array<Box>("Box")
    .element(&Box::center)
    .element(&Box::halfExtent);

value_array<Viewport>("Viewport")
    .element(&Viewport::left)
    .element(&Viewport::bottom)
    .element(&Viewport::width)
    .element(&Viewport::height);

value_array<KtxBlobIndex>("KtxBlobIndex")
    .element(&KtxBlobIndex::mipLevel)
    .element(&KtxBlobIndex::arrayIndex)
    .element(&KtxBlobIndex::cubeFace);

// In JavaScript, a flat contiguous representation is best for matrices (see gl-matrix) so we
// need to define a small wrapper here.

struct flatmat4 {
    math::mat4f m;
    float& operator[](int i) { return m[i / 4][i % 4]; }
};

value_array<flatmat4>("mat4")
    .element(index< 0>()).element(index< 1>()).element(index< 2>()).element(index< 3>())
    .element(index< 4>()).element(index< 5>()).element(index< 6>()).element(index< 7>())
    .element(index< 8>()).element(index< 9>()).element(index<10>()).element(index<11>())
    .element(index<12>()).element(index<13>()).element(index<14>()).element(index<15>());

struct flatmat3 {
    math::mat3f m;
    float& operator[](int i) { return m[i / 3][i % 3]; }
};

value_array<flatmat3>("mat3")
    .element(index<0>()).element(index<1>()).element(index<2>())
    .element(index<3>()).element(index<4>()).element(index<5>())
    .element(index<6>()).element(index<7>()).element(index<8>());

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
    .function("setDepthPrepass", &View::setDepthPrepass)
    .function("setPostProcessingEnabled", &View::setPostProcessingEnabled)
    .function("setAntiAliasing", &View::setAntiAliasing)
    .function("getAntiAliasing", &View::getAntiAliasing);

/// Scene ::core class:: Flat container of renderables and lights.
/// See also the [Engine] methods `createScene` and `destroyScene`.
class_<Scene>("Scene")
    .function("addEntity", &Scene::addEntity)
    .function("remove", &Scene::remove)
    .function("setSkybox", &Scene::setSkybox, allow_raw_pointers())
    .function("setIndirectLight", &Scene::setIndirectLight, allow_raw_pointers())
    .function("getRenderableCount", &Scene::getRenderableCount)
    .function("getLightCount", &Scene::getLightCount);

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
    .function("setExposure", &Camera::setExposure)
    .function("lookAt", &Camera::lookAt);

class_<RenderBuilder>("RenderableManager$Builder")
    .function("_build", EMBIND_LAMBDA(int, (RenderBuilder* builder,
            Engine* engine, utils::Entity entity), {
        return (int) builder->build(*engine, entity);
    }), allow_raw_pointers())
    .BUILDER_FUNCTION("boundingBox", RenderBuilder, (RenderBuilder* builder, Box box), {
        return &builder->boundingBox(box); })
    .BUILDER_FUNCTION("culling", RenderBuilder, (RenderBuilder* builder, bool enable), {
        return &builder->culling(enable); })
    .BUILDER_FUNCTION("receiveShadows", RenderBuilder, (RenderBuilder* builder, bool enable), {
        return &builder->receiveShadows(enable); })
    .BUILDER_FUNCTION("castShadows", RenderBuilder, (RenderBuilder* builder, bool enable), {
        return &builder->castShadows(enable); })
    .BUILDER_FUNCTION("geometry", RenderBuilder, (RenderBuilder* builder,
            size_t index,
            RenderableManager::PrimitiveType type,
            VertexBuffer* vertices,
            IndexBuffer* indices), {
        return &builder->geometry(index, type, vertices, indices); })
    .BUILDER_FUNCTION("material", RenderBuilder, (RenderBuilder* builder,
            size_t index, MaterialInstance* mi), {
        return &builder->material(index, mi); })
    .BUILDER_FUNCTION("blendOrder", RenderBuilder,
            (RenderBuilder* builder, size_t index, uint16_t order), {
        return &builder->blendOrder(index, order); });

/// RenderableManager ::core class:: Allows access to properties of drawable objects.
class_<RenderableManager>("RenderableManager")
    .class_function("Builder", (RenderBuilder (*)(int)) [] (int n) { return RenderBuilder(n); })

    /// getInstance ::method:: Gets an instance of the renderable component for an entity.
    /// entity ::argument:: an [Entity]
    /// ::retval:: a renderable component
    .function("getInstance", &RenderableManager::getInstance)

    .function("setAxisAlignedBoundingBox", &RenderableManager::setAxisAlignedBoundingBox)
    .function("setLayerMask", &RenderableManager::setLayerMask)
    .function("setPriority", &RenderableManager::setPriority)
    .function("setCastShadows", &RenderableManager::setCastShadows)
    .function("setReceiveShadows", &RenderableManager::setReceiveShadows)
    .function("isShadowCaster", &RenderableManager::isShadowCaster)
    .function("isShadowReceiver", &RenderableManager::isShadowReceiver)
    .function("getAxisAlignedBoundingBox", &RenderableManager::getAxisAlignedBoundingBox)
    .function("getPrimitiveCount", &RenderableManager::getPrimitiveCount)
    .function("setMaterialInstanceAt", &RenderableManager::setMaterialInstanceAt,
            allow_raw_pointers())
    .function("getMaterialInstanceAt", &RenderableManager::getMaterialInstanceAt,
            allow_raw_pointers())
    .function("setBlendOrderAt", &RenderableManager::setBlendOrderAt)

    // TODO: provide bindings for AttributeBitset
    .function("getEnabledAttributesAt", &RenderableManager::getEnabledAttributesAt)

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
    }), allow_raw_pointers());

/// RenderableManager$Instance ::class:: Component instance returned by [RenderableManager]
/// Be sure to call the instance's `delete` method when you're done with it.
class_<RenderableManager::Instance>("RenderableManager$Instance");
    /// delete ::method:: Frees an instance obtained via `getInstance`

/// TransformManager ::core class:: Adds transform components to entities.
class_<TransformManager>("TransformManager")
    /// getInstance ::method:: Gets an instance representing the transform component for an entity.
    /// entity ::argument:: an [Entity]
    /// ::retval:: a transform component that can be passed to `setTransform`.
    .function("getInstance", &TransformManager::getInstance)
    /// setTransform ::method:: Sets the mat4 value of a transform component.
    /// instance ::argument:: The transform instance of entity, obtained via `getInstance`.
    /// matrix ::argument:: Array of 16 numbers (mat4)
    .function("setTransform", EMBIND_LAMBDA(void,
            (TransformManager* self, TransformManager::Instance instance, flatmat4 m), {
        self->setTransform(instance, m.m); }), allow_raw_pointers());

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
    .BUILDER_FUNCTION("position", LightBuilder, (LightBuilder* builder, math::float3 value), {
        return &builder->position(value); })
    .BUILDER_FUNCTION("direction", LightBuilder, (LightBuilder* builder, math::float3 value), {
        return &builder->direction(value); })
    .BUILDER_FUNCTION("color", LightBuilder, (LightBuilder* builder, math::float3 value), {
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
    .class_function("Builder", (LightBuilder (*)(LightManager::Type)) [] (LightManager::Type lt) {
        return LightBuilder(lt); });

class_<VertexBuilder>("VertexBuffer$Builder")
    .function("_build", EMBIND_LAMBDA(VertexBuffer*, (VertexBuilder* builder, Engine* engine), {
        return builder->build(*engine);
    }), allow_raw_pointers())
    .BUILDER_FUNCTION("attribute", VertexBuilder, (VertexBuilder* builder,
            VertexAttribute attr,
            uint8_t bufferIndex,
            VertexBuffer::AttributeType attrType,
            uint8_t byteOffset,
            uint8_t byteStride), {
        return &builder->attribute(attr, bufferIndex, attrType, byteOffset, byteStride); })
    .BUILDER_FUNCTION("vertexCount", VertexBuilder, (VertexBuilder* builder, int count), {
        return &builder->vertexCount(count); })
    .BUILDER_FUNCTION("normalized", VertexBuilder, (VertexBuilder* builder,
            VertexAttribute attrib), {
        return &builder->normalized(attrib); })
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
    .function("createInstance", &Material::createInstance, allow_raw_pointers());

class_<MaterialInstance>("MaterialInstance")
    .function("setFloatParameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, float value), {
        self->setParameter(name.c_str(), value); }), allow_raw_pointers())
    .function("setFloat2Parameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, math::float2 value), {
        self->setParameter(name.c_str(), value); }), allow_raw_pointers())
    .function("setFloat3Parameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, math::float3 value), {
        self->setParameter(name.c_str(), value); }), allow_raw_pointers())
    .function("setFloat4Parameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, math::float4 value), {
        self->setParameter(name.c_str(), value); }), allow_raw_pointers())
    .function("setTextureParameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, Texture* value, TextureSampler sampler), {
        self->setParameter(name.c_str(), value, sampler); }), allow_raw_pointers())
    .function("setColorParameter", EMBIND_LAMBDA(void,
            (MaterialInstance* self, std::string name, RgbType type, math::float3 value), {
        self->setParameter(name.c_str(), type, value); }), allow_raw_pointers())
    .function("setPolygonOffset", &MaterialInstance::setPolygonOffset);

class_<TextureSampler>("TextureSampler")
    .constructor<driver::SamplerMinFilter, driver::SamplerMagFilter, driver::SamplerWrapMode>();

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
        return &builder->usage(usage); })
    .BUILDER_FUNCTION("rgbm", TexBuilder, (TexBuilder* builder, bool rgbm), {
        return &builder->rgbm(rgbm); });

class_<IndirectLight>("IndirectLight")
    .class_function("Builder", (IblBuilder (*)()) [] { return IblBuilder(); })
    .function("setIntensity", &IndirectLight::setIntensity)
    .function("getIntensity", &IndirectLight::getIntensity)
    .function("setRotation", EMBIND_LAMBDA(void, (IndirectLight* self, flatmat3 value), {
        return self->setRotation(value.m);
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
        return &builder->irradiance(nbands, (math::float3 const*) floats.data()); })
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
class_<utils::Entity>("Entity");

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
    .constructor<emscripten::val>()
    /// getBytes ::method:: Gets a view of the WASM heap referenced by the buffer descriptor.
    /// ::retval:: Uint8Array
    .function("getBytes", &BufferDescriptor::getBytes);

/// PixelBufferDescriptor ::class:: Low level pixel buffer wrapper.
/// Clients should use the [PixelBuffer] helper function to contruct PixelBufferDescriptor objects.
class_<PixelBufferDescriptor>("driver$PixelBufferDescriptor")
    .constructor<emscripten::val, driver::PixelDataFormat, driver::PixelDataType>()
    .constructor<emscripten::val, driver::CompressedPixelDataType, int, bool>()
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
        auto result = KtxUtility::toTextureFormat(self->info());
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
    /// rgbm ::argument:: boolean that configures the alpha channel into an HDR scale.
    /// ::retval:: [PixelDataFormat]
    /// Returns "undefined" if no valid Filament enumerant exists.
    .function("getPixelDataFormat",
            EMBIND_LAMBDA(driver::PixelDataFormat, (KtxBundle* self, bool rgbm), {
        return KtxUtility::toPixelDataFormat(self->getInfo(), rgbm);
    }), allow_raw_pointers())

    /// getPixelDataType ::method::
    /// ::retval:: [PixelDataType]
    /// Returns "undefined" if no valid Filament enumerant exists.
    .function("getPixelDataType",
            EMBIND_LAMBDA(driver::PixelDataType, (KtxBundle* self), {
        return KtxUtility::toPixelDataType(self->getInfo());
    }), allow_raw_pointers())

    /// getCompressedPixelDataType ::method::
    /// ::retval:: [CompressedPixelDataType]
    /// Returns "undefined" if no valid Filament enumerant exists.
    .function("getCompressedPixelDataType",
            EMBIND_LAMBDA(driver::CompressedPixelDataType, (KtxBundle* self), {
        return KtxUtility::toCompressedPixelDataType(self->getInfo());
    }), allow_raw_pointers())

    /// isCompressed ::method::
    /// Per spec, compressed textures in KTX always have their glFormat field set to 0.
    /// ::retval:: boolean
    .function("isCompressed", EMBIND_LAMBDA(bool, (KtxBundle* self), {
        return KtxUtility::isCompressed(self->getInfo());
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

function("KtxUtility$createTexture", EMBIND_LAMBDA(Texture*,
        (Engine* engine, const KtxBundle& ktx, bool srgb, bool rgbm), {
    return KtxUtility::createTexture(engine, ktx, srgb, rgbm, nullptr, nullptr);
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

class_<MeshReader::MaterialRegistry>("MeshReader$MaterialRegistry")
    .constructor<>()
    .function("size", &MeshReader::MaterialRegistry::size)
    .function("get", internal::MapAccess<MeshReader::MaterialRegistry>::get)
    .function("set", internal::MapAccess<MeshReader::MaterialRegistry>::set)
    .function("keys", EMBIND_LAMBDA(std::vector<std::string>, (MeshReader::MaterialRegistry* self), {
        std::vector<std::string> result;
        for (const auto& pair : *self) {
            result.emplace_back(pair.first);
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
            (Engine* engine, BufferDescriptor buffer, const MeshReader::MaterialRegistry& matreg), {
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

// Clients should call [createTextureFromPng] rather than using decodePng and DecodedPng directly.

function("decodePng", &decodePng);
class_<DecodedPng>("DecodedPng")
    .property("width", &DecodedPng::width)
    .property("height", &DecodedPng::height)
    .property("data", &DecodedPng::decoded_data);

} // EMSCRIPTEN_BINDINGS
