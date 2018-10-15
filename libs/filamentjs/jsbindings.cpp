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
 * The purpose of the filamentjs library is to offer a first-class JavaScript interface to the core
 * Filament classes: Engine, Renderer, Texture, etc.
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

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/SwapChain.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <image/KtxBundle.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat4.h>

#include <utils/EntityManager.h>

#include <emscripten.h>
#include <emscripten/bind.h>

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
        BIND(RenderableManager)
        BIND(TransformManager)
        BIND(VertexBuffer)
        BIND(IndexBuffer)
        BIND(Material)
        BIND(MaterialInstance)
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

// We avoid directly exposing driver::BufferDescriptor because embind does not support move
// semantics and void* doesn't make sense to JavaScript anyway. This little wrapper class is exposed
// to JavaScript as "driver$BufferDescriptor", but clients will normally use our "Filament.Buffer"
// helper function, which is implemented in wasmloader.
struct BufferDescriptor {
    // This form is used when JavaScript sends a buffer into WASM.
    BufferDescriptor(val arrdata) {
        auto byteLength = arrdata["byteLength"].as<uint32_t>();
        this->bd = new driver::BufferDescriptor(malloc(byteLength), byteLength,
                [](void* buffer, size_t size, void* user) { free(buffer); });
    }
    // This form is used when WASM needs to return a buffer to a JavaScript.
    BufferDescriptor(uint8_t* data, uint32_t size) {
        this->bd = new driver::BufferDescriptor(data, size);
    }
    ~BufferDescriptor() {
        delete bd;
    }
    val getBytes() {
        unsigned char *byteBuffer = (unsigned char*) bd->buffer;
        size_t bufferLength = bd->size;
        return val(typed_memory_view(bufferLength, byteBuffer));
    };
    driver::BufferDescriptor* bd;
};

} // anonymous namespace

EMSCRIPTEN_BINDINGS(array_types) {

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

// CONSTANTS and ENUMS
// -------------------

enum_<VertexAttribute>("VertexAttribute")
        .value("POSITION", POSITION)
        .value("TANGENTS", TANGENTS)
        .value("COLOR", COLOR)
        .value("UV0", UV0)
        .value("UV1", UV1)
        .value("BONE_INDICES", BONE_INDICES)
        .value("BONE_WEIGHTS", BONE_WEIGHTS);

 enum_<VertexBuffer::AttributeType>("VertexBuffer$AttributeType")
        .value("BYTE", VertexBuffer::AttributeType::BYTE)
        .value("BYTE2", VertexBuffer::AttributeType::BYTE2)
        .value("BYTE3", VertexBuffer::AttributeType::BYTE3)
        .value("BYTE4", VertexBuffer::AttributeType::BYTE4)
        .value("UBYTE", VertexBuffer::AttributeType::UBYTE)
        .value("UBYTE2", VertexBuffer::AttributeType::UBYTE2)
        .value("UBYTE3", VertexBuffer::AttributeType::UBYTE3)
        .value("UBYTE4", VertexBuffer::AttributeType::UBYTE4)
        .value("SHORT", VertexBuffer::AttributeType::SHORT)
        .value("SHORT2", VertexBuffer::AttributeType::SHORT2)
        .value("SHORT3", VertexBuffer::AttributeType::SHORT3)
        .value("SHORT4", VertexBuffer::AttributeType::SHORT4)
        .value("USHORT", VertexBuffer::AttributeType::USHORT)
        .value("USHORT2", VertexBuffer::AttributeType::USHORT2)
        .value("USHORT3", VertexBuffer::AttributeType::USHORT3)
        .value("USHORT4", VertexBuffer::AttributeType::USHORT4)
        .value("INT", VertexBuffer::AttributeType::INT)
        .value("UINT", VertexBuffer::AttributeType::UINT)
        .value("FLOAT", VertexBuffer::AttributeType::FLOAT)
        .value("FLOAT2", VertexBuffer::AttributeType::FLOAT2)
        .value("FLOAT3", VertexBuffer::AttributeType::FLOAT3)
        .value("FLOAT4", VertexBuffer::AttributeType::FLOAT4)
        .value("HALF", VertexBuffer::AttributeType::HALF)
        .value("HALF2", VertexBuffer::AttributeType::HALF2)
        .value("HALF3", VertexBuffer::AttributeType::HALF3)
        .value("HALF4", VertexBuffer::AttributeType::HALF4);

 enum_<IndexBuffer::IndexType>("IndexBuffer$IndexType")
        .value("USHORT", IndexBuffer::IndexType::USHORT)
        .value("UINT", IndexBuffer::IndexType::UINT);

 enum_<RenderableManager::PrimitiveType>("RenderableManager$PrimitiveType")
        .value("POINTS", RenderableManager::PrimitiveType::POINTS)
        .value("LINES", RenderableManager::PrimitiveType::LINES)
        .value("TRIANGLES", RenderableManager::PrimitiveType::TRIANGLES)
        .value("NONE", RenderableManager::PrimitiveType::NONE);

 enum_<View::DepthPrepass>("View$DepthPrepass")
        .value("DEFAULT", View::DepthPrepass::DEFAULT)
        .value("DISABLED", View::DepthPrepass::DISABLED)
        .value("ENABLED", View::DepthPrepass::ENABLED);

 enum_<Camera::Projection>("Camera$Projection")
        .value("PERSPECTIVE", Camera::Projection::PERSPECTIVE)
        .value("ORTHO", Camera::Projection::ORTHO);

// CORE FILAMENT CLASSES
// ---------------------

class_<Engine>("Engine")
    .class_function("_create", (Engine* (*)()) [] { return Engine::create(); },
            allow_raw_pointers())
    .class_function("destroy", (void (*)(Engine*)) []
            (Engine* engine) { Engine::destroy(&engine); }, allow_raw_pointers())
    .function("execute", &Engine::execute)
    .function("getTransformManager", EMBIND_LAMBDA(TransformManager*, (Engine* engine), {
        return &engine->getTransformManager();
    }), allow_raw_pointers())
    .function("createSwapChain", (SwapChain* (*)(Engine*)) []
            (Engine* engine) { return engine->createSwapChain(nullptr); },
            allow_raw_pointers())
    .function("destroySwapChain", (void (*)(Engine*, SwapChain*)) []
            (Engine* engine, SwapChain* swapChain) { engine->destroy(swapChain); },
            allow_raw_pointers())
    .function("createRenderer", &Engine::createRenderer, allow_raw_pointers())
    .function("destroyRenderer", (void (*)(Engine*, Renderer*)) []
            (Engine* engine, Renderer* renderer) { engine->destroy(renderer); },
            allow_raw_pointers())
    .function("createView", &Engine::createView, allow_raw_pointers())
    .function("destroyView", (void (*)(Engine*, View*)) []
            (Engine* engine, View* view) { engine->destroy(view); },
            allow_raw_pointers())
    .function("createScene", &Engine::createScene, allow_raw_pointers())
    .function("destroyScene", (void (*)(Engine*, Scene*)) []
            (Engine* engine, Scene* scene) { engine->destroy(scene); },
            allow_raw_pointers())
    .function("createCamera", select_overload<Camera*(void)>(&Engine::createCamera),
            allow_raw_pointers())
    .function("destroyCamera", (void (*)(Engine*, Camera*)) []
            (Engine* engine, Camera* camera) { engine->destroy(camera); },
            allow_raw_pointers())
    .function("createMaterial", EMBIND_LAMBDA(Material*, (Engine* engine, BufferDescriptor mbd), {
        // TODO: consider adding a BufferDescriptor API to Material::Builder, then possibly removing
        // this convenient helper method.
        return Material::Builder().package(mbd.bd->buffer, mbd.bd->size).build(*engine);
    }), allow_raw_pointers())
    .function("destroyMaterial", (void (*)(Engine*, Material*)) []
            (Engine* engine, Material* mat) { engine->destroy(mat); },
            allow_raw_pointers())
    .function("destroyEntity", (void (*)(Engine*, utils::Entity)) []
            (Engine* engine, utils::Entity entity) { engine->destroy(entity); },
            allow_raw_pointers())
    .function("destroyMaterialInstance", (void (*)(Engine*, MaterialInstance*)) []
            (Engine* engine, MaterialInstance* mi) { engine->destroy(mi); },
            allow_raw_pointers())
    .function("destroyVertexBuffer", (void (*)(Engine*, VertexBuffer*)) []
            (Engine* engine, VertexBuffer* vb) { engine->destroy(vb); },
            allow_raw_pointers())
    .function("destroyIndexBuffer", (void (*)(Engine*, IndexBuffer*)) []
            (Engine* engine, IndexBuffer* ib) { engine->destroy(ib); },
            allow_raw_pointers());

class_<SwapChain>("SwapChain");

class_<Renderer>("Renderer")
    .function("render", &Renderer::render, allow_raw_pointers())
    .function("beginFrame", &Renderer::beginFrame, allow_raw_pointers())
    .function("endFrame", &Renderer::endFrame, allow_raw_pointers());

class_<View>("View")
    .function("setScene", &View::setScene, allow_raw_pointers())
    .function("setCamera", &View::setCamera, allow_raw_pointers())
    .function("getViewport", &View::getViewport)
    .function("setViewport", &View::setViewport)
    .function("setClearColor", &View::setClearColor)
    .function("setDepthPrepass", &View::setDepthPrepass)
    .function("setPostProcessingEnabled", &View::setPostProcessingEnabled);

class_<Scene>("Scene")
    .function("addEntity", &Scene::addEntity);

class_<Camera>("Camera")
    .function("setProjection", EMBIND_LAMBDA(void, (Camera* self, Camera::Projection projection,
            double left, double right, double bottom, double top, double near, double far), {
        self->setProjection(projection, left, right, bottom, top, near, far);
    }), allow_raw_pointers());

class_<RenderBuilder>("RenderableManager$Builder")
    .function("build", EMBIND_LAMBDA(void, (RenderBuilder* builder,
            Engine* engine, utils::Entity entity), {
        builder->build(*engine, entity);
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
        return &builder->material(index, mi); });

class_<RenderableManager>("RenderableManager")
    .class_function("Builder", (RenderBuilder (*)(int)) [] (int n) { return RenderBuilder(n); });

class_<TransformManager>("TransformManager")
    .function("getInstance", &TransformManager::getInstance)
    .function("setTransform", EMBIND_LAMBDA(void,
            (TransformManager* self, TransformManager::Instance instance, flatmat4 m), {
        self->setTransform(instance, m.m); }), allow_raw_pointers());

class_<TransformManager::Instance>("TransformManager$Instance");

class_<VertexBuilder>("VertexBuffer$Builder")
    .function("build", EMBIND_LAMBDA(VertexBuffer*, (VertexBuilder* builder, Engine* engine), {
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

class_<VertexBuffer>("VertexBuffer")
    .class_function("Builder", (VertexBuilder (*)()) [] { return VertexBuilder(); })
    .function("setBufferAt", EMBIND_LAMBDA(void, (VertexBuffer* self,
            Engine* engine, uint8_t bufferIndex, BufferDescriptor vbd), {
        self->setBufferAt(*engine, bufferIndex, std::move(*vbd.bd));
    }), allow_raw_pointers());

class_<IndexBuilder>("IndexBuffer$Builder")
    .function("build", EMBIND_LAMBDA(IndexBuffer*, (IndexBuilder* builder, Engine* engine), {
        return builder->build(*engine);
    }), allow_raw_pointers())
    .BUILDER_FUNCTION("indexCount", IndexBuilder, (IndexBuilder* builder, int count), {
        return &builder->indexCount(count); })
    .BUILDER_FUNCTION("bufferType", IndexBuilder, (IndexBuilder* builder,
            IndexBuffer::IndexType indexType), {
        return &builder->bufferType(indexType); });

class_<IndexBuffer>("IndexBuffer")
    .class_function("Builder", (IndexBuilder (*)()) [] { return IndexBuilder(); })
    .function("setBuffer", EMBIND_LAMBDA(void, (IndexBuffer* self,
            Engine* engine, BufferDescriptor ibd), {
        self->setBuffer(*engine, std::move(*ibd.bd));
    }), allow_raw_pointers());

class_<Material>("Material")
    .function("getDefaultInstance",
            select_overload<MaterialInstance*(void)>(&Material::getDefaultInstance),
            allow_raw_pointers())
    .function("getInstance", &Material::createInstance, allow_raw_pointers());

class_<MaterialInstance>("MaterialInstance");

// UTILS TYPES
// -----------

class_<utils::Entity>("Entity");

class_<utils::EntityManager>("EntityManager")
    .class_function("get", (utils::EntityManager* (*)()) []
        { return &utils::EntityManager::get(); }, allow_raw_pointers())
    .function("create", select_overload<utils::Entity()>(&utils::EntityManager::create))
    .function("destroy", select_overload<void(utils::Entity)>(&utils::EntityManager::destroy));

// DRIVER TYPES
// ------------

class_<BufferDescriptor>("driver$BufferDescriptor")
    .constructor<emscripten::val>()
    .function("getBytes", &BufferDescriptor::getBytes);

// IMAGE TYPES
// ------------

class_<KtxBundle>("KtxBundle")
    .constructor(EMBIND_LAMBDA(KtxBundle*, (BufferDescriptor kbd), {
        return new KtxBundle((uint8_t*) kbd.bd->buffer, (uint32_t) kbd.bd->size);
    }))
    .function("getNumMipLevels", &KtxBundle::getNumMipLevels)
    .function("getArrayLength", &KtxBundle::getArrayLength)
    .function("isCubemap", &KtxBundle::isCubemap)
    .function("getBlob", EMBIND_LAMBDA(BufferDescriptor, (KtxBundle* self, KtxBlobIndex index), {
        uint8_t* data;
        uint32_t size;
        self->getBlob(index, &data, &size);
        return BufferDescriptor(data, size);
    }), allow_raw_pointers())
    .function("info", &KtxBundle::info);

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

} // EMSCRIPTEN_BINDINGS
