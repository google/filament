/*
 * Copyright (C) 2026 The Android Open Source Project
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
 * GPU tests of the motion vectors (see View::setMotionVectorsOptions()).
 *
 * Like RenderingTest, these tests render, so they need a GPU or a software rasterizer (e.g. Mesa's
 * lavapipe, which the CI uses, or llvmpipe; see test/filament-unit-test/README.md). They use the
 * default backend, unless the FILAMENT_TEST_BACKEND environment variable is set to opengl,
 * vulkan, metal or webgpu.
 *
 * The object motion vectors ("objectMotionVectors") of each render are copied into a RG16F texture
 * with the internal FView::setMotionVectorsReadback() hook, then read back with
 * Renderer::readPixels(). They're compared with the analytic motion of the surface point P seen
 * through each pixel center:
 *
 *      v = s' * (uvRT(C' * P) - uvRT(C' * P'))
 *
 * where C' is the clip-from-world transform of the View's camera at its previous render, P' is the
 * position of P at the previous render, and uvRT() converts clip space to UVs over the viewport,
 * whose y axis points down on Vulkan, Metal and WebGPU. The motion vectors are in render-target
 * UVs, which also cover the padding and the guard band of the color pass: s' is the scale of that
 * padding (the xy of its clipTransform) at the previous render.
 */

#include "DynamicSpecConstKey.h"
#include "LocalProgramCache.h"
#include "MaterialParser.h"

#include "details/Engine.h"
#include "details/Material.h"
#include "details/Scene.h"
#include "details/Texture.h"
#include "details/View.h"

#include <private/filament/Variant.h>

#include <filament/Box.h>
#include <filament/Camera.h>
#include <filament/DebugRegistry.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/InstanceBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialEnums.h>
#include <filament/MaterialInstance.h>
#include <filament/Options.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/RenderTarget.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <filamat/MaterialBuilder.h>
#include <filamat/Package.h>

#include <backend/DriverEnums.h>
#include <backend/PixelBufferDescriptor.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/Panic.h>
#include <utils/tribool.h>

#include <math/half.h>
#include <math/mat4.h>
#include <math/scalar.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

using namespace filament;
using namespace filament::math;
using namespace utils;

using backend::PixelBufferDescriptor;
using backend::PixelDataFormat;
using backend::PixelDataType;

namespace {

// The backend to test, see the top of this file.
Engine::Backend getTestBackend() noexcept {
    if (char const* const name = std::getenv("FILAMENT_TEST_BACKEND")) {
        if (!strcmp(name, "opengl")) return Engine::Backend::OPENGL;
        if (!strcmp(name, "vulkan")) return Engine::Backend::VULKAN;
        if (!strcmp(name, "metal")) return Engine::Backend::METAL;
        if (!strcmp(name, "webgpu")) return Engine::Backend::WEBGPU;
    }
    return Engine::Backend::DEFAULT;
}

mat4f translation(float const x, float const y, float const z = 0.0f) noexcept {
    return mat4f::translation(float3{ x, y, z });
}

mat4f rotationZ(float const degrees) noexcept {
    return mat4f::rotation(degrees * f::DEG_TO_RAD, float3{ 0.0f, 0.0f, 1.0f });
}

mat4f scaling(float const s) noexcept {
    return mat4f::scaling(float3{ s, s, 1.0f });
}

// A 1x1 quad in the XY plane, centered on the origin and facing +Z.
constexpr float3 kQuadVertices[4] = {
        { -0.5f, -0.5f, 0.0f }, { 0.5f, -0.5f, 0.0f },
        { -0.5f, 0.5f, 0.0f }, { 0.5f, 0.5f, 0.0f } };
constexpr uint16_t kQuadIndices[6] = { 0, 1, 2, 2, 1, 3 };

// With kHalfTransparentFragment, the left half of the quad (x < 0 in model space) is transparent,
// so MASKED materials discard it.
constexpr char const* kLocalPositionVertex = R"(
    void materialVertex(inout MaterialVertexInputs material) {
        material.localPos = getPosition();
    }
)";

constexpr char const* kHalfTransparentFragment = R"(
    void material(inout MaterialInputs material) {
        prepareMaterial(material);
        material.baseColor = vec4(1.0, 1.0, 1.0, variable_localPos.x < 0.0 ? 0.0 : 1.0);
    }
)";

// Displaces the vertices by the `offset` parameter, in world space.
constexpr char const* kDisplacementVertex = R"(
    void materialVertex(inout MaterialVertexInputs material) {
        material.worldPosition.xyz += materialParams.offset;
    }
)";

// 8 vertical stripes across the quad (see kStripeCount), alternating dark and light. This is
// content with a lot of high-frequency detail, which is what a history sample misaligned by a
// texel or two smears: both colors are in every 3x3 neighborhood near a stripe edge, so box
// clipping can't remove the stale history there.
constexpr char const* kStripesFragment = R"(
    void material(inout MaterialInputs material) {
        prepareMaterial(material);
        float s = step(0.5, fract((variable_localPos.x + 0.5) * 8.0));
        material.baseColor = vec4(vec3(mix(0.05, 0.9, s)), 1.0);
    }
)";
constexpr int kStripeCount = 8;

class MotionVectorsTest : public testing::Test {
protected:
    static constexpr uint32_t kSize = 64;

    // The motion vectors are compared with the analytic motion at the pixel centers. They're
    // stored as FP16, whose rounding error is at most 2.4e-4 for |v| < 1 (half of the spacing of
    // the FP16 values in [0.5, 1)). (The largest error measured with lavapipe and llvmpipe is
    // 1.04e-4.)
    static constexpr double kTolerance = 1.0e-3;

    // Initial contents of the readback textures, which no render writes.
    static constexpr float kSentinel = 7.0f;

    // A View with its own motion vectors readback.
    struct TestView {
        View* view = nullptr;
        Texture* texture = nullptr;
        RenderTarget* target = nullptr;
        uint2 size{ kSize };                // of the texture, and of the color pass's viewport
        std::vector<half> pixels;           // RG
        bool pixelsReady = false;

        // The state of the last render of this View:
        mat4 clipFromWorld;         // the camera's clip-from-world transform
        mat4 prevClipFromWorld;     // the same at the render before, i.e. C'
        float4 prevClipTransform{ 1.0f, 1.0f, 0.0f, 0.0f }; // of the color pass at the render before
        bool prevValid = false;     // the render before was usable as history
        bool hadPass = false;       // the object motion vectors pass ran

        // The motion vector of the pixel p, with (0, 0) at the bottom-left.
        float2 at(int2 const p) const noexcept {
            // readPixels() returns the rows from top to bottom
            size_t const i = 2 * (size_t(size.y - 1 - p.y) * size.x + size_t(p.x));
            return { float(pixels[i]), float(pixels[i + 1]) };
        }

        bool isAll(float const value) const noexcept {
            return std::all_of(pixels.begin(), pixels.end(), [value](half const h) {
                return float(h) == value;
            });
        }

        bool isAllZero() const noexcept {
            return isAll(0.0f);
        }
    };

    struct MaterialDesc {
        BlendingMode blending = BlendingMode::OPAQUE;
        UserVariantFilterMask variantFilter = 0;
        char const* fragment = nullptr;     // material() code
        char const* vertex = nullptr;       // materialVertex() code
        bool localPosVariable = false;      // declares the custom variable `localPos`
        bool offsetParameter = false;       // declares the float3 parameter `offset`
    };

    Engine* mEngine = nullptr;
    SwapChain* mSwapChain = nullptr;
    Renderer* mRenderer = nullptr;
    Scene* mScene = nullptr;
    Skybox* mSkybox = nullptr;
    Camera* mCamera = nullptr;
    Entity mCameraEntity;
    VertexBuffer* mVertexBuffer = nullptr;
    IndexBuffer* mIndexBuffer = nullptr;
    TestView* mView = nullptr;
    bool mFlipY = false;
    bool mMaterialBuilderInitialized = false;

    std::vector<std::unique_ptr<TestView>> mViews;
    std::vector<Scene*> mScenes;
    std::vector<Entity> mEntities;
    std::vector<InstanceBuffer*> mInstanceBuffers;
    std::vector<Material*> mMaterials;
    std::vector<MaterialInstance*> mMaterialInstances;

    void SetUp() override {
        mEngine = Engine::Builder().backend(getTestBackend()).build();
        if (!mEngine) {
            GTEST_SKIP() << "This backend isn't available";
        }
        mFlipY = mEngine->getBackend() != Engine::Backend::OPENGL;

        mSwapChain = mEngine->createSwapChain(kSize, kSize);
        mRenderer = mEngine->createRenderer();
        mScene = createScene();
        mSkybox = Skybox::Builder().color({ 0.1f, 0.2f, 0.3f, 1.0f }).build(*mEngine);
        mScene->setSkybox(mSkybox);

        mCameraEntity = EntityManager::get().create();
        mCamera = mEngine->createCamera(mCameraEntity);
        mCamera->setProjection(45.0, 1.0, 0.1, 100.0, Camera::Fov::VERTICAL);
        lookFrom(0.0, 0.0, 4.0);

        mVertexBuffer = VertexBuffer::Builder()
                .vertexCount(4)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                .build(*mEngine);
        mVertexBuffer->setBufferAt(*mEngine, 0,
                VertexBuffer::BufferDescriptor(kQuadVertices, sizeof(kQuadVertices)));
        mIndexBuffer = IndexBuffer::Builder()
                .indexCount(6)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*mEngine);
        mIndexBuffer->setBuffer(*mEngine,
                IndexBuffer::BufferDescriptor(kQuadIndices, sizeof(kQuadIndices)));

        mView = &createTestView(mScene);
        if (!downcast(mView->view)->evaluateMotionVectorsActive(*downcast(mEngine))) {
            GTEST_SKIP() << "Motion vectors aren't supported";
        }
    }

    void TearDown() override {
        if (!mEngine) {
            return;
        }
        EntityManager& em = EntityManager::get();
        for (Entity const e : mEntities) {
            mEngine->destroy(e);
            em.destroy(e);
        }
        for (MaterialInstance const* mi : mMaterialInstances) {
            mEngine->destroy(mi);
        }
        for (Material const* material : mMaterials) {
            mEngine->destroy(material);
        }
        for (InstanceBuffer const* ib : mInstanceBuffers) {
            mEngine->destroy(ib);
        }
        for (auto const& tv : mViews) {
            downcast(tv->view)->setMotionVectorsReadback({});
            mEngine->destroy(tv->view);
            mEngine->destroy(tv->target);
            mEngine->destroy(tv->texture);
        }
        mViews.clear();
        for (Scene const* scene : mScenes) {
            mEngine->destroy(scene);
        }
        mEngine->destroy(mSkybox);
        mEngine->destroy(mVertexBuffer);
        mEngine->destroy(mIndexBuffer);
        mEngine->destroyCameraComponent(mCameraEntity);
        em.destroy(mCameraEntity);
        mEngine->destroy(mRenderer);
        mEngine->destroy(mSwapChain);
        Engine::destroy(&mEngine);
        if (mMaterialBuilderInitialized) {
            filamat::MaterialBuilder::shutdown();
        }
    }

    // --------------------------------------------------------------------------------------------
    // Scene setup

    Scene* createScene() {
        return mScenes.emplace_back(mEngine->createScene());
    }

    // Creates a View of `scene` with mCamera, which reads back its object motion vectors into a
    // texture of `size`: the size of the color pass's viewport, which is the View's viewport
    // unless dynamic resolution scales it.
    TestView& createTestView(Scene* const scene, uint2 const size = uint2{ kSize }) {
        TestView& tv = *mViews.emplace_back(std::make_unique<TestView>());
        tv.size = size;
        tv.pixels.resize(2 * size_t(size.x) * size_t(size.y));
        tv.view = mEngine->createView();
        tv.view->setViewport({ 0, 0, kSize, kSize });
        tv.view->setScene(scene);
        tv.view->setCamera(mCamera);
        tv.view->setPostProcessingEnabled(true);
        tv.view->setDithering(View::Dithering::NONE);
        tv.view->setAntiAliasing(View::AntiAliasing::NONE);
        tv.view->setTemporalAntiAliasingOptions({ .enabled = false });
        tv.view->setShadowingEnabled(false);
        tv.view->setMotionVectorsOptions({ .enabled = true });

        tv.texture = Texture::Builder()
                .width(size.x)
                .height(size.y)
                .levels(1)
                .format(Texture::InternalFormat::RG16F)
                .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE |
                       Texture::Usage::BLIT_SRC | Texture::Usage::BLIT_DST |
                       Texture::Usage::UPLOADABLE)
                .build(*mEngine);
        // Only the renders with motion vectors overwrite this.
        size_t const count = tv.pixels.size();
        auto* const sentinel = new half[count];
        std::fill_n(sentinel, count, half{ kSentinel });
        tv.texture->setImage(*mEngine, 0, PixelBufferDescriptor(sentinel, count * sizeof(half),
                PixelDataFormat::RG, PixelDataType::HALF,
                [](void* buffer, size_t, void*) { delete[] static_cast<half*>(buffer); }));
        tv.target = RenderTarget::Builder()
                .texture(RenderTarget::AttachmentPoint::COLOR, tv.texture)
                .build(*mEngine);
        downcast(tv.view)->setMotionVectorsReadback({ .texture = downcast(tv.texture) });
        return tv;
    }

    // Places mCamera at (x, y, z), looking down -Z.
    void lookFrom(double const x, double const y, double const z) {
        mCamera->lookAt({ x, y, z }, { x, y, z - 1.0 }, { 0.0, 1.0, 0.0 });
    }

    Material* buildMaterial(MaterialDesc const& desc) {
        if (!mMaterialBuilderInitialized) {
            filamat::MaterialBuilder::init();
            mMaterialBuilderInitialized = true;
        }
        using filamat::MaterialBuilder;
        bool const mobile = downcast(mEngine)->getShaderModel() == backend::ShaderModel::MOBILE;
        MaterialBuilder builder;
        builder.name("MotionVectorsTestMaterial")
                .targetApi(filamat::targetApiFromBackend(mEngine->getBackend()))
                .platform(mobile ? MaterialBuilder::Platform::MOBILE
                                 : MaterialBuilder::Platform::DESKTOP)
                .optimization(MaterialBuilder::Optimization::NONE)
                .shading(Shading::UNLIT)
                .blending(desc.blending)
                .variantFilter(desc.variantFilter);
        if (desc.fragment) {
            builder.material(desc.fragment);
        }
        if (desc.vertex) {
            builder.materialVertex(desc.vertex);
        }
        if (desc.localPosVariable) {
            builder.variable(MaterialBuilder::Variable::CUSTOM0, "localPos");
        }
        if (desc.offsetParameter) {
            builder.parameter("offset", MaterialBuilder::UniformType::FLOAT3);
        }
        filamat::Package const package = builder.build(mEngine->getJobSystem());
        if (!package.isValid()) {
            return nullptr;
        }
        Material* const material = Material::Builder()
                .package(package.getData(), package.getSize())
                .build(*mEngine);
        if (material) {
            mMaterials.push_back(material);
        }
        return material;
    }

    MaterialInstance* createMaterialInstance(Material const* const material) {
        return mMaterialInstances.emplace_back(material->createInstance());
    }

    InstanceBuffer* createInstanceBuffer(size_t const count, mat4f const* const localTransforms) {
        return mInstanceBuffers.emplace_back(
                InstanceBuffer::Builder(count).localTransforms(localTransforms).build(*mEngine));
    }

    void destroyInstanceBuffer(InstanceBuffer* const ib) {
        mInstanceBuffers.erase(std::find(mInstanceBuffers.begin(), mInstanceBuffers.end(), ib));
        mEngine->destroy(ib);
    }

    // Creates an entity with a transform, which is a child of `parent` if it's not null.
    Entity createTransform(mat4f const& transform, Entity const parent = {}) {
        Entity const e = mEntities.emplace_back(EntityManager::get().create());
        TransformManager& tcm = mEngine->getTransformManager();
        tcm.create(e, parent ? tcm.getInstance(parent) : TransformManager::Instance{}, transform);
        return e;
    }

    // Gives `e` a quad renderable, with the default material if `mi` is null.
    void buildQuad(Entity const e, MaterialInstance const* const mi = nullptr,
            InstanceBuffer* const instances = nullptr, bool const culling = false) {
        RenderableManager::Builder builder(1);
        builder.boundingBox({ { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.1f } })
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                        mVertexBuffer, mIndexBuffer, 0, 6)
                .culling(culling)
                .castShadows(false)
                .receiveShadows(false);
        if (mi) {
            builder.material(0, mi);
        }
        if (instances) {
            // large enough for the instances of these tests
            builder.boundingBox({ { 0.0f, 0.0f, 0.0f }, { 1.5f, 0.5f, 0.1f } })
                    .instances(instances->getInstanceCount(), instances);
        }
        builder.build(*mEngine, e);
    }

    // Creates a quad, and adds it to `scene` if it's not null.
    Entity createQuad(Scene* const scene, mat4f const& transform,
            MaterialInstance const* const mi = nullptr, Entity const parent = {}) {
        Entity const e = createTransform(transform, parent);
        buildQuad(e, mi);
        if (scene) {
            scene->addEntity(e);
        }
        return e;
    }

    void setTransform(Entity const e, mat4f const& transform) {
        TransformManager& tcm = mEngine->getTransformManager();
        tcm.setTransform(tcm.getInstance(e), transform);
    }

    mat4 getWorld(Entity const e) {
        TransformManager const& tcm = mEngine->getTransformManager();
        return tcm.getWorldTransformAccurate(tcm.getInstance(e));
    }

    // The row of `e` in the visible renderables of the last render of `tv`, or -1.
    static int findVisibleRenderable(TestView const& tv, Entity const e) {
        FView const* const view = downcast(tv.view);
        Entity const* const entities = view->getRenderableData().data<FScene::RENDERABLE_ENTITY>();
        for (uint32_t const i : view->getVisibleRenderables()) {
            if (entities[i] == e) {
                return int(i);
            }
        }
        return -1;
    }

    // Whether `e` was visible at the last render of `tv`.
    static bool isVisible(TestView const& tv, Entity const e) {
        return findVisibleRenderable(tv, e) >= 0;
    }

    // Whether `e` was visible and reported as moving at the last render of `tv`. This is what
    // selects the motion vectors variant; a renderable whose motion is the identity would also
    // have zero motion vectors.
    static bool isMoving(TestView const& tv, Entity const e) {
        int const i = findVisibleRenderable(tv, e);
        return i >= 0 && downcast(tv.view)->getRenderableData()
                .data<FScene::VISIBILITY_STATE>()[i].motionMoved;
    }

    // --------------------------------------------------------------------------------------------
    // Rendering

    // Renders one frame with the given Views, in order, and reads back their motion vectors. If
    // `color` isn't null, it also reads back the RGBA8 image of the SwapChain (rows from top to
    // bottom), into which the Views render.
    [[nodiscard]] bool render(std::initializer_list<TestView*> const views,
            std::vector<uint8_t>* const color = nullptr) {
        for (TestView* const tv : views) {
            auto const& previous = downcast(tv->view)->getFrameHistory().getPrevious();
            tv->prevValid = previous.motionVectors.valid;
            tv->prevClipTransform = previous.motionVectors.clipTransform;
            tv->prevClipFromWorld = tv->clipFromWorld;
            tv->clipFromWorld = getClipFromWorld(tv->view->getCamera());
            tv->pixelsReady = false;
        }
        for (int attempts = 0; !mRenderer->beginFrame(mSwapChain); attempts++) {
            if (attempts == 10) {
                ADD_FAILURE() << "beginFrame() keeps failing";
                return false;
            }
            mEngine->flushAndWait();
        }
        for (TestView* const tv : views) {
            mRenderer->render(tv->view);
        }
        auto const setReady = [](void*, size_t, void* user) { *static_cast<bool*>(user) = true; };
        for (TestView* const tv : views) {
            mRenderer->readPixels(tv->target, 0, 0, tv->size.x, tv->size.y, PixelBufferDescriptor(
                    tv->pixels.data(), tv->pixels.size() * sizeof(half),
                    PixelDataFormat::RG, PixelDataType::HALF, setReady, &tv->pixelsReady));
        }
        bool colorReady = false;
        if (color) {
            color->assign(4 * kSize * kSize, 0);
            mRenderer->readPixels(0, 0, kSize, kSize, PixelBufferDescriptor(
                    color->data(), color->size(), PixelDataFormat::RGBA, PixelDataType::UBYTE,
                    setReady, &colorReady));
        }
        mRenderer->endFrame();
        // this is where the readPixels() callbacks are called
        mEngine->flushAndWait();

        bool success = true;
        for (TestView* const tv : views) {
            FView const* const view = downcast(tv->view);
            // Without history, there are no motion vectors. Otherwise, the object motion vectors
            // pass only runs when something moved (it has a fast path).
            tv->hadPass = view->isMotionVectorsActive() && tv->prevValid &&
                    view->hasMovingRenderables();
            if (!tv->pixelsReady) {
                ADD_FAILURE() << "readPixels() didn't complete";
                success = false;
            }
        }
        if (color && !colorReady) {
            ADD_FAILURE() << "readPixels() of the SwapChain didn't complete";
            success = false;
        }
        return success;
    }

    // --------------------------------------------------------------------------------------------
    // Expected values

    static mat4 getClipFromWorld(Camera const& camera) {
        return camera.getProjectionMatrix() * camera.getViewMatrix();
    }

    // The pixel of the color pass's viewport through which the world-space point p is seen at the
    // last render of `tv`, (0, 0) being the bottom-left one.
    static int2 pixelOf(TestView const& tv, double3 const& p) {
        double4 const c = tv.clipFromWorld * double4{ p, 1.0 };
        double2 const uv = c.xy / c.w * 0.5 + 0.5;
        return { int(std::floor(uv.x * tv.size.x)), int(std::floor(uv.y * tv.size.y)) };
    }

    // The pixel through which the point `local` of a quad (in its model space) is seen.
    static int2 pixelOf(TestView const& tv, mat4 const& quadWorld, double2 const local) {
        return pixelOf(tv, (quadWorld * double4{ local, 0.0, 1.0 }).xyz);
    }

    static bool isInside(TestView const& tv, int2 const p) noexcept {
        return p.x >= 0 && p.y >= 0 && p.x < int(tv.size.x) && p.y < int(tv.size.y);
    }

    // Two points of the ray through the center of `pixel` at the last render of `tv`, the second
    // one being farther from the camera.
    static std::pair<double3, double3> getRay(TestView const& tv, int2 const pixel) {
        mat4 const worldFromClip = inverse(tv.clipFromWorld);
        double2 const ndc = (double2(pixel) + 0.5) / double2(tv.size) * 2.0 - 1.0;
        auto const unproject = [&](double const z) {
            double4 const p = worldFromClip * double4{ ndc, z, 1.0 };
            return p.xyz / p.w;
        };
        // The projection's far plane is at infinity (z = 1).
        return { unproject(0.5), unproject(0.75) };
    }

    // The intersection of the ray through the center of `pixel` with the plane of a quad.
    static double3 surfacePoint(TestView const& tv, int2 const pixel, mat4 const& quadWorld) {
        auto const [a, b] = getRay(tv, pixel);
        double3 const origin = quadWorld[3].xyz;
        double3 const normal = cross(quadWorld[0].xyz, quadWorld[1].xyz);
        return a + (b - a) * (dot(origin - a, normal) / dot(b - a, normal));
    }

    // The render-target UV of the homogeneous world-space point p (a direction when p.w is 0),
    // over the viewport.
    double2 uvRT(mat4 const& clipFromWorld, double4 const& p) const {
        double4 const c = clipFromWorld * p;
        double2 uv = c.xy / c.w * 0.5 + 0.5;
        if (mFlipY) {
            uv.y = 1.0 - uv.y;
        }
        return uv;
    }

    // The analytic motion vector of the homogeneous world-space point p, which was at prevP at the
    // previous render of `tv` (see the top of this file).
    double2 expectedMotion(TestView const& tv, double4 const& p, double4 const& prevP) const {
        // in the render-target UVs of the previous render
        return double2(tv.prevClipTransform.xy) *
                (uvRT(tv.prevClipFromWorld, p) - uvRT(tv.prevClipFromWorld, prevP));
    }

    // Expects the motion vector of `pixel` to be `expected`.
    static void expectMotionVector(TestView const& tv, int2 const pixel, double2 const expected) {
        // otherwise, the test wouldn't check much
        EXPECT_GT(std::max(std::abs(expected.x), std::abs(expected.y)), 10.0 * kTolerance);
        float2 const actual = tv.at(pixel);
        EXPECT_NEAR(actual.x, expected.x, kTolerance);
        EXPECT_NEAR(actual.y, expected.y, kTolerance);
    }

    // Expects the motion vector of `pixel` to be the motion of the quad seen through it, whose
    // world transform is `quadWorld`, and whose points P were at prevFromCurr * P at the previous
    // render.
    void expectMotionAt(TestView const& tv, int2 const pixel, mat4 const& quadWorld,
            mat4 const& prevFromCurr) const {
        SCOPED_TRACE(testing::Message() << "pixel (" << pixel.x << ", " << pixel.y << ")");
        ASSERT_TRUE(isInside(tv, pixel));
        double4 const p{ surfacePoint(tv, pixel, quadWorld), 1.0 };
        expectMotionVector(tv, pixel, expectedMotion(tv, p, prevFromCurr * p));
    }

    // Expects the motion vector of the pixel through which the point `local` of a quad is seen to
    // be the quad's motion (see expectMotionAt()).
    void expectMotion(TestView const& tv, mat4 const& quadWorld, mat4 const& prevFromCurr,
            double2 const local) const {
        SCOPED_TRACE(testing::Message() << "local point (" << local.x << ", " << local.y << ")");
        expectMotionAt(tv, pixelOf(tv, quadWorld, local), quadWorld, prevFromCurr);
    }

    // Same as expectMotion(), for a quad whose world transform was prevWorld at the previous
    // render.
    void expectQuadMotion(TestView const& tv, mat4 const& prevWorld, mat4 const& quadWorld,
            std::initializer_list<double2> const locals) const {
        mat4 const prevFromCurr = prevWorld * inverse(quadWorld);
        for (double2 const local : locals) {
            expectMotion(tv, quadWorld, prevFromCurr, local);
        }
    }

    // Expects no motion at the pixels through which the points `locals` of a quad are seen.
    static void expectNoMotion(TestView const& tv, mat4 const& quadWorld,
            std::initializer_list<double2> const locals) {
        for (double2 const local : locals) {
            int2 const pixel = pixelOf(tv, quadWorld, local);
            SCOPED_TRACE(testing::Message() << "local point (" << local.x << ", " << local.y
                    << "), pixel (" << pixel.x << ", " << pixel.y << ")");
            ASSERT_TRUE(isInside(tv, pixel));
            float2 const actual = tv.at(pixel);
            EXPECT_EQ(actual.x, 0.0f);
            EXPECT_EQ(actual.y, 0.0f);
        }
    }

    // Pixels near the corners of the color pass's viewport, which see the sky in these tests.
    static std::array<int2, 4> getSkyPixels(TestView const& tv) {
        int const w = int(tv.size.x);
        int const h = int(tv.size.y);
        return { int2{ 2, 2 }, int2{ w - 3, 2 }, int2{ 2, h - 3 }, int2{ w - 3, h - 3 } };
    }

    // Points of a quad, in its model space, away from its edges.
    static constexpr std::initializer_list<double2> kQuadPoints = {
            { 0.0, 0.0 }, { 0.3, 0.3 }, { -0.3, 0.25 }, { 0.25, -0.3 } };
};

} // anonymous namespace

// Case 1: a static scene viewed by a moving camera has no object motion, and the motion vectors
// pass is skipped (fast path) whenever nothing moved.
TEST_F(MotionVectorsTest, FastPath) {
    TestView& tv = *mView;
    FView const* const view = downcast(tv.view);
    Entity const quad = createQuad(mScene, translation(-0.2f, 0.1f));

    // The first render has no history.
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(view->isMotionVectorsActive());
    EXPECT_FALSE(tv.prevValid);
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(tv.isAllZero());
    EXPECT_TRUE(view->getFrameHistory().getPrevious().motionVectors.valid);

    // Only the camera moves.
    lookFrom(0.2, -0.1, 3.5);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.prevValid);
    EXPECT_FALSE(view->hasMovingRenderables());
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(tv.isAllZero());

    // The quad moves.
    mat4 const prev = getWorld(quad);
    setTransform(quad, translation(0.1f, -0.05f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(view->hasMovingRenderables());
    EXPECT_TRUE(tv.hadPass);
    expectQuadMotion(tv, prev, getWorld(quad), kQuadPoints);
    EXPECT_EQ(tv.at({ 1, 1 }), float2{ 0.0f });     // background

    // Nothing moves: the fast path's zeros replace the motion vectors of the previous render.
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.prevValid);
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(tv.isAllZero());
}

// Case 2: a moving TransformManager entity, viewed by a moving camera.
TEST_F(MotionVectorsTest, MovingEntityAndCamera) {
    TestView& tv = *mView;
    Entity const quad = createQuad(mScene, translation(-0.1f, 0.1f));
    ASSERT_TRUE(render({ &tv }));

    // The camera dollies in and pans, while the quad moves (including in depth) and rotates.
    lookFrom(0.1, 0.0, 3.5);
    mat4 const prev = getWorld(quad);
    setTransform(quad, translation(0.15f, -0.05f, 0.25f) * rotationZ(15.0f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    expectQuadMotion(tv, prev, getWorld(quad), kQuadPoints);
    EXPECT_EQ(tv.at({ 1, 1 }), float2{ 0.0f });     // background

    // The camera moves the other way, and so does the quad.
    lookFrom(-0.1, 0.1, 4.0);
    mat4 const prev2 = getWorld(quad);
    setTransform(quad, translation(-0.1f, 0.05f, -0.2f) * rotationZ(-10.0f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    expectQuadMotion(tv, prev2, getWorld(quad), kQuadPoints);
}

// Case 3: a static occluder hides the motion of the object behind it.
TEST_F(MotionVectorsTest, OccluderHidesMotion) {
    TestView& tv = *mView;
    Entity const quad = createQuad(mScene, mat4f{});
    // Covers x <= 0, in front of the quad.
    createQuad(mScene, translation(-0.5f, 0.0f, 1.0f) * mat4f::scaling(float3{ 1.0f, 2.0f, 1.0f }));
    ASSERT_TRUE(render({ &tv }));

    mat4 const prev = getWorld(quad);
    setTransform(quad, translation(0.1f, 0.05f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    mat4 const world = getWorld(quad);
    expectNoMotion(tv, world, { { -0.25, 0.0 }, { -0.4, 0.3 } });
    expectQuadMotion(tv, prev, world, { { 0.25, 0.0 }, { 0.3, 0.3 }, { 0.2, -0.3 } });
}

// Case 3 (continued): with MSAA enabled, tilted moving quads and static occluders still pass
// depth testing at pixel centers (whereas reusing an MSAA-resolved depth buffer that picks sample
// 0's sub-pixel depth would fail DepthFunc::GE on one of the two opposite tilt orientations).
TEST_F(MotionVectorsTest, OccluderHidesMotionWithMsaa) {
    TestView& tv = *mView;
    tv.view->setMultiSampleAntiAliasingOptions({ .enabled = true, .sampleCount = 4 });

    mat4f const tiltPos =
            mat4f::rotation(35.0f * f::DEG_TO_RAD, normalize(float3{ 1.0f, 1.0f, 0.0f }));
    mat4f const tiltNeg =
            mat4f::rotation(-35.0f * f::DEG_TO_RAD, normalize(float3{ 1.0f, 1.0f, 0.0f }));
    Entity const quadPos = createQuad(mScene, translation(-0.6f, 0.0f) * tiltPos * scaling(0.8f));
    Entity const quadNeg = createQuad(mScene, translation(0.6f, 0.0f) * tiltNeg * scaling(0.8f));
    // Static occluder in front of the left half of quadPos.
    createQuad(mScene, translation(-0.9f, 0.0f, 1.0f) * mat4f::scaling(float3{ 1.0f, 2.0f, 1.0f }));
    ASSERT_TRUE(render({ &tv }));

    mat4 const prevPos = getWorld(quadPos);
    mat4 const prevNeg = getWorld(quadNeg);
    setTransform(quadPos, translation(-0.5f, 0.05f) * tiltPos * scaling(0.8f));
    setTransform(quadNeg, translation(0.7f, -0.05f) * tiltNeg * scaling(0.8f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);

    mat4 const worldPos = getWorld(quadPos);
    expectNoMotion(tv, worldPos, { { -0.25, 0.0 }, { -0.3, 0.25 } });
    expectQuadMotion(tv, prevPos, worldPos, { { 0.25, 0.0 }, { 0.3, 0.25 }, { 0.25, -0.25 } });
    expectQuadMotion(tv, prevNeg, getWorld(quadNeg), kQuadPoints);
}

// Case 3 (continued): when a channel clears the depth buffer (View::setChannelDepthClearEnabled()),
// the motion vectors pass also clears its depth buffer before that channel, preserves channel
// order, and overwrites earlier channels' motion vectors when a static renderable in the later
// channel covers them.
TEST_F(MotionVectorsTest, ChannelDepthClear) {
    TestView& tv = *mView;
    tv.view->setChannelDepthClearEnabled(1, true);
    // A guard band makes xvp != svp, exercising scissor restoration after the custom depth clear.
    tv.view->setGuardBandOptions({ .enabled = true });
    RenderableManager& rcm = mEngine->getRenderableManager();

    // Channel 0 (at z = 0.5, closer to the camera): a moving quad on the left, a static quad on
    // the right.
    Entity const ch0Moving = createQuad(mScene, translation(-0.6f, 0.0f, 0.5f));
    Entity const ch0Static = createQuad(mScene, translation(0.6f, 0.0f, 0.5f));
    for (Entity const e : { ch0Moving, ch0Static }) {
        rcm.setChannel(rcm.getInstance(e), 0);
    }

    // Channel 1 (at z = -0.5, farther from the camera than channel 0, but drawn after channel 1's
    // depth clear): a static quad over the bottom half of ch0Moving, a moving quad over the bottom
    // half of ch0Static, and a nearer static occluder (at z = 0.0) in channel 1 hiding the right
    // half of ch1Moving.
    Entity const ch1Static = createQuad(mScene, translation(-0.6f, -0.35f, -0.5f) * scaling(0.6f));
    Entity const ch1Moving = createQuad(mScene, translation(0.6f, -0.35f, -0.5f) * scaling(0.6f));
    Entity const ch1Occluder = createQuad(mScene, translation(0.8f, -0.35f, 0.0f) * scaling(0.4f));
    for (Entity const e : { ch1Static, ch1Moving, ch1Occluder }) {
        rcm.setChannel(rcm.getInstance(e), 1);
    }

    ASSERT_TRUE(render({ &tv }));

    mat4 const prevCh0Moving = getWorld(ch0Moving);
    mat4 const prevCh1Moving = getWorld(ch1Moving);
    setTransform(ch0Moving, translation(-0.5f, 0.05f, 0.5f));
    setTransform(ch1Moving, translation(0.55f, -0.3f, -0.5f) * scaling(0.6f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);

    // Top half of ch0Moving isn't covered by ch1Static: reports ch0Moving's motion.
    expectQuadMotion(tv, prevCh0Moving, getWorld(ch0Moving), { { 0.0, 0.3 }, { -0.25, 0.35 } });
    // ch1Static overwrites ch0Moving's motion back to zero despite being at z = -0.5 < 0.5.
    expectNoMotion(tv, getWorld(ch1Static), kQuadPoints);
    // Top half of ch0Static has no motion.
    expectNoMotion(tv, getWorld(ch0Static), { { 0.0, 0.3 }, { 0.25, 0.35 } });
    // Left half of ch1Moving draws over ch0Static (despite z = -0.5 < 0.5) and reports motion.
    expectQuadMotion(tv, prevCh1Moving, getWorld(ch1Moving), { { -0.3, 0.0 }, { -0.25, -0.25 } });
    // Right half of ch1Moving is occluded within channel 1 by ch1Occluder (at z = 0.0 > -0.5).
    expectNoMotion(tv, getWorld(ch1Occluder), { { 0.0, 0.0 }, { -0.2, 0.0 } });
}

// Case 4: a View rendered every other frame reports the motion since its own previous render.
TEST_F(MotionVectorsTest, MultiView) {
    TestView& a = *mView;
    TestView& b = createTestView(mScene);
    Entity const quad = createQuad(mScene, mat4f{});

    std::vector<mat4> worlds;   // the quad's world transform at each frame
    for (int frame = 0; frame < 6; frame++) {
        SCOPED_TRACE(testing::Message() << "frame " << frame);
        lookFrom(0.05 * frame, 0.0, 4.0);
        setTransform(quad, translation(0.08f * float(frame) - 0.2f, 0.04f * float(frame)) *
                rotationZ(4.0f * float(frame)));
        worlds.push_back(getWorld(quad));

        bool const renderB = frame % 2 == 0;
        ASSERT_TRUE(renderB ? render({ &a, &b }) : render({ &a }));
        if (frame >= 1) {
            SCOPED_TRACE("View A");
            EXPECT_TRUE(a.hadPass);
            expectQuadMotion(a, worlds[frame - 1], worlds[frame], kQuadPoints);
        }
        if (renderB) {
            SCOPED_TRACE("View B");
            if (frame == 0) {
                EXPECT_FALSE(b.hadPass);
                EXPECT_TRUE(b.isAllZero());
            } else {
                EXPECT_TRUE(b.hadPass);
                expectQuadMotion(b, worlds[frame - 2], worlds[frame], kQuadPoints);
            }
        }
    }
}

// Case 5: InstanceBuffer instances move with their local transform, and with their root.
TEST_F(MotionVectorsTest, InstanceBuffer) {
    TestView& tv = *mView;
    mat4f locals[3] = {
            translation(-1.0f, 0.0f) * scaling(0.6f),
            translation(0.0f, 0.0f) * scaling(0.6f),
            translation(1.0f, 0.0f) * scaling(0.6f) };
    InstanceBuffer* const ib = createInstanceBuffer(3, locals);
    Entity const root = createTransform(mat4f{});
    buildQuad(root, nullptr, ib);
    mScene->addEntity(root);
    auto const instanceWorld = [&](size_t const i) { return getWorld(root) * mat4{ locals[i] }; };

    ASSERT_TRUE(render({ &tv }));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(tv.hadPass);

    // Only the local transform of instance 1 changes.
    mat4 prev[3] = { instanceWorld(0), instanceWorld(1), instanceWorld(2) };
    locals[1] = translation(0.1f, 0.15f) * rotationZ(20.0f) * scaling(0.6f);
    ib->setLocalTransforms(locals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    expectNoMotion(tv, instanceWorld(0), kQuadPoints);
    expectQuadMotion(tv, prev[1], instanceWorld(1), kQuadPoints);
    expectNoMotion(tv, instanceWorld(2), kQuadPoints);

    // Only the root moves, so the InstanceBuffer doesn't change: all the instances move with it.
    for (size_t i = 0; i < 3; i++) {
        prev[i] = instanceWorld(i);
    }
    setTransform(root, translation(0.1f, -0.2f) * rotationZ(-10.0f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    for (size_t i = 0; i < 3; i++) {
        SCOPED_TRACE(testing::Message() << "instance " << i);
        expectQuadMotion(tv, prev[i], instanceWorld(i), kQuadPoints);
    }

    // Both the root and the local transform of instance 2 change.
    for (size_t i = 0; i < 3; i++) {
        prev[i] = instanceWorld(i);
    }
    setTransform(root, translation(0.05f, -0.1f) * rotationZ(-5.0f));
    locals[2] = translation(1.0f, 0.2f) * scaling(0.5f);
    ib->setLocalTransforms(locals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    for (size_t i = 0; i < 3; i++) {
        SCOPED_TRACE(testing::Message() << "instance " << i);
        expectQuadMotion(tv, prev[i], instanceWorld(i), kQuadPoints);
    }

    // Rewriting the same local transforms (including -0.0f vs +0.0f) isn't a motion.
    ib->setLocalTransforms(locals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(tv.isAllZero());

    mat4f negZeroLocals[3] = { locals[0], locals[1], locals[2] };
    negZeroLocals[0][2][0] = -0.0f;
    ib->setLocalTransforms(negZeroLocals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(tv.isAllZero());

    // Even if another instance moves, the instance with -0.0f vs +0.0f isn't treated as moved.
    mat4 const prev1 = instanceWorld(1);
    negZeroLocals[1] = translation(0.25f, 0.0f) * scaling(0.6f);
    locals[1] = negZeroLocals[1];
    ib->setLocalTransforms(negZeroLocals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    expectNoMotion(tv, instanceWorld(0), kQuadPoints);
    expectQuadMotion(tv, prev1, instanceWorld(1), kQuadPoints);
    expectNoMotion(tv, instanceWorld(2), kQuadPoints);
}

// Case 5 (continued): local transforms that change and are restored before the next render
// change the InstanceBuffer's revision, but the instances didn't move.
TEST_F(MotionVectorsTest, InstanceBufferChangedAndRestored) {
    TestView& tv = *mView;
    mat4f locals[3] = {
            translation(-1.0f, 0.0f) * scaling(0.6f),
            translation(0.0f, 0.0f) * scaling(0.6f),
            translation(1.0f, 0.0f) * scaling(0.6f) };
    InstanceBuffer* const ib = createInstanceBuffer(3, locals);
    Entity const root = createTransform(mat4f{});
    buildQuad(root, nullptr, ib);
    mScene->addEntity(root);
    auto const instanceWorld = [&](size_t const i) { return getWorld(root) * mat4{ locals[i] }; };
    ASSERT_TRUE(render({ &tv }));
    ASSERT_TRUE(render({ &tv }));

    mat4f changed[3] = { locals[0], locals[1], locals[2] };
    changed[1] = translation(0.0f, 0.3f) * scaling(0.6f);
    ib->setLocalTransforms(changed, 3);
    ib->setLocalTransforms(locals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.prevValid);
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(isVisible(tv, root));
    EXPECT_FALSE(isMoving(tv, root));
    EXPECT_TRUE(tv.isAllZero());

    // The next change is reported from the restored local transform.
    mat4 const prev = instanceWorld(1);
    locals[1] = translation(0.1f, 0.15f) * scaling(0.6f);
    ib->setLocalTransforms(locals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    EXPECT_TRUE(isMoving(tv, root));
    expectNoMotion(tv, instanceWorld(0), kQuadPoints);
    expectQuadMotion(tv, prev, instanceWorld(1), kQuadPoints);
    expectNoMotion(tv, instanceWorld(2), kQuadPoints);
}

// Case 6: a new InstanceBuffer (which could reuse the memory of the old one) has no motion.
TEST_F(MotionVectorsTest, InstanceBufferRecreated) {
    TestView& tv = *mView;
    mat4f locals[3] = {
            translation(-1.0f, 0.0f) * scaling(0.6f),
            translation(0.0f, 0.0f) * scaling(0.6f),
            translation(1.0f, 0.0f) * scaling(0.6f) };
    InstanceBuffer* const ib = createInstanceBuffer(3, locals);
    Entity const root = createTransform(mat4f{});
    buildQuad(root, nullptr, ib);
    mScene->addEntity(root);
    Entity const mover = createQuad(mScene, translation(0.0f, 1.0f) * scaling(0.5f));
    ASSERT_TRUE(render({ &tv }));
    ASSERT_TRUE(render({ &tv }));

    // The first InstanceBuffer changes once, so that its revision isn't the initial one.
    mat4 const prev1 = getWorld(root) * mat4{ locals[1] };
    locals[1] = translation(0.0f, 0.1f) * scaling(0.6f);
    ib->setLocalTransforms(locals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    EXPECT_TRUE(isMoving(tv, root));
    expectQuadMotion(tv, prev1, getWorld(root) * mat4{ locals[1] }, kQuadPoints);

    // The renderable is rebuilt with a new InstanceBuffer with other local transforms, while
    // another object moves, so that the motion vectors pass runs.
    mEngine->getRenderableManager().destroy(root);
    destroyInstanceBuffer(ib);
    for (mat4f& local : locals) {
        local = translation(0.0f, -0.2f) * local;
    }
    InstanceBuffer* const ib2 = createInstanceBuffer(3, locals);
    buildQuad(root, nullptr, ib2);
    mat4 const prevMover = getWorld(mover);
    setTransform(mover, translation(0.1f, 1.0f) * scaling(0.5f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    EXPECT_FALSE(isMoving(tv, root));
    expectQuadMotion(tv, prevMover, getWorld(mover), { { 0.0, 0.0 }, { 0.3, 0.3 } });
    for (size_t i = 0; i < 3; i++) {
        SCOPED_TRACE(testing::Message() << "instance " << i);
        expectNoMotion(tv, getWorld(root) * mat4{ locals[i] }, kQuadPoints);
    }

    // The changes of the new InstanceBuffer are reported.
    mat4 const prev = getWorld(root) * mat4{ locals[1] };
    locals[1] = translation(0.1f, -0.1f) * scaling(0.6f);
    ib2->setLocalTransforms(locals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    expectQuadMotion(tv, prev, getWorld(root) * mat4{ locals[1] }, kQuadPoints);
    expectNoMotion(tv, getWorld(root) * mat4{ locals[0] }, kQuadPoints);
}

// Case 6 (continued): an instanced renderable that moves while it's culled has no stale motion
// once visible.
TEST_F(MotionVectorsTest, InstanceBufferCulledThenVisible) {
    TestView& tv = *mView;
    mat4f locals[3] = {
            translation(-1.0f, 0.0f) * scaling(0.6f),
            translation(0.0f, 0.0f) * scaling(0.6f),
            translation(1.0f, 0.0f) * scaling(0.6f) };
    InstanceBuffer* const ib = createInstanceBuffer(3, locals);
    Entity const root = createTransform(mat4f{});
    buildQuad(root, nullptr, ib, true);
    mScene->addEntity(root);
    Entity const mover = createQuad(mScene, translation(0.0f, 1.0f) * scaling(0.5f));
    auto const instanceWorld = [&](size_t const i) { return getWorld(root) * mat4{ locals[i] }; };
    ASSERT_TRUE(render({ &tv }));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(isVisible(tv, root));

    // The camera looks away, while the root and an instance move.
    lookFrom(100.0, 0.0, 4.0);
    setTransform(root, translation(0.1f, 0.0f));
    locals[1] = translation(0.0f, 0.2f) * scaling(0.6f);
    ib->setLocalTransforms(locals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(isVisible(tv, root));

    // The camera looks back, and only the other object moves.
    lookFrom(0.0, 0.0, 4.0);
    mat4 const prevMover = getWorld(mover);
    setTransform(mover, translation(0.1f, 1.0f) * scaling(0.5f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(isVisible(tv, root));
    EXPECT_FALSE(isMoving(tv, root));
    EXPECT_TRUE(tv.hadPass);
    expectQuadMotion(tv, prevMover, getWorld(mover), { { 0.0, 0.0 }, { 0.3, 0.3 } });
    for (size_t i = 0; i < 3; i++) {
        SCOPED_TRACE(testing::Message() << "instance " << i);
        expectNoMotion(tv, instanceWorld(i), kQuadPoints);
    }

    // The root moves: from where it was at the previous render, not before it was culled.
    mat4 prev[3] = { instanceWorld(0), instanceWorld(1), instanceWorld(2) };
    setTransform(root, translation(0.2f, -0.1f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    for (size_t i = 0; i < 3; i++) {
        SCOPED_TRACE(testing::Message() << "instance " << i);
        expectQuadMotion(tv, prev[i], instanceWorld(i), kQuadPoints);
    }
}

// Case 6 (continued): the instances of a renderable that wasn't visible at the previous render
// move from where they were then, like its root. Hiding it with a layer mask takes the same path
// as culling, but the camera doesn't move.
TEST_F(MotionVectorsTest, InstanceBufferEntersView) {
    TestView& tv = *mView;
    mat4f locals[3] = {
            translation(-1.0f, 0.0f) * scaling(0.6f),
            translation(0.0f, 0.0f) * scaling(0.6f),
            translation(1.0f, 0.0f) * scaling(0.6f) };
    InstanceBuffer* const ib = createInstanceBuffer(3, locals);
    Entity const root = createTransform(mat4f{});
    buildQuad(root, nullptr, ib);
    mScene->addEntity(root);
    auto const instanceWorld = [&](size_t const i) { return getWorld(root) * mat4{ locals[i] }; };
    RenderableManager& rcm = mEngine->getRenderableManager();
    RenderableManager::Instance const ri = rcm.getInstance(root);
    ASSERT_TRUE(render({ &tv }));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(isVisible(tv, root));

    // The renderable is hidden while an instance moves.
    rcm.setLayerMask(ri, 0xFF, 0x00);
    locals[1] = translation(0.0f, 0.2f) * scaling(0.6f);
    ib->setLocalTransforms(locals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(isVisible(tv, root));

    // It's visible again, while the root and the same instance move.
    mat4 const prev[3] = { instanceWorld(0), instanceWorld(1), instanceWorld(2) };
    rcm.setLayerMask(ri, 0xFF, 0x01);
    setTransform(root, translation(0.1f, 0.0f));
    locals[1] = translation(0.1f, 0.1f) * rotationZ(10.0f) * scaling(0.6f);
    ib->setLocalTransforms(locals, 3);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(isVisible(tv, root));
    EXPECT_TRUE(tv.hadPass);
    for (size_t i = 0; i < 3; i++) {
        SCOPED_TRACE(testing::Message() << "instance " << i);
        expectQuadMotion(tv, prev[i], instanceWorld(i), kQuadPoints);
    }
}

// Case 7: the motion vectors only account for the transforms, not for materialVertex().
TEST_F(MotionVectorsTest, MaterialVertexDisplacement) {
    TestView& tv = *mView;
    Material const* const material = buildMaterial({
            .vertex = kDisplacementVertex, .offsetParameter = true });
    ASSERT_NE(material, nullptr);
    FMaterial const* const fmaterial = downcast(material);
    EXPECT_TRUE(fmaterial->hasMotionVectorsVariant());
    EXPECT_TRUE(fmaterial->getDefinition().hasCustomDepthShader);
    // The package contains the custom MOT vertex shader, but omits the MOT fragment shader and
    // shares the default material's one.
    MaterialParser const& parser = fmaterial->getDefinition().getMaterialParser();
    backend::ShaderModel const sm = downcast(mEngine)->getShaderModel();
    Variant const mot{ Variant::SPECIAL_MOT_VARIANT };
    EXPECT_TRUE(parser.hasShader(sm, mot, backend::ShaderStage::VERTEX));
    EXPECT_FALSE(parser.hasShader(sm, mot, backend::ShaderStage::FRAGMENT));
    MaterialInstance* const mi = createMaterialInstance(material);
    mi->setParameter("offset", float3{ 0.0f, 0.5f, 0.0f });
    Entity const quad = createQuad(mScene, translation(-0.2f, -0.5f), mi);
    ASSERT_TRUE(render({ &tv }));

    // Both the transform and the displacement change: only the former is reported, at the
    // displaced position. The rotation makes the motion depend on that position.
    mat4 const prev = getWorld(quad);
    setTransform(quad, translation(-0.1f, -0.5f) * rotationZ(-10.0f));
    mi->setParameter("offset", float3{ 0.0f, 0.75f, 0.0f });
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    mat4 const world = getWorld(quad);
    for (double2 const local : kQuadPoints) {
        expectMotion(tv, mat4::translation(double3{ 0.0, 0.75, 0.0 }) * world,
                prev * inverse(world), local);
    }

    // Only the displacement changes: nothing moved.
    mi->setParameter("offset", float3{ 0.0f, 0.25f, 0.0f });
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(tv.isAllZero());
}

// Case 8: filtering out the VSM variant doesn't remove the motion vectors variant,
// although MOT is encoded as MNT | PCK (and MNT aliases S2D).
TEST_F(MotionVectorsTest, MaskedWithVsmFilter) {
    TestView& tv = *mView;
    Material const* const material = buildMaterial({
            .blending = BlendingMode::MASKED,
            .variantFilter = UserVariantFilterMask(UserVariantFilterBit::VSM),
            .fragment = kHalfTransparentFragment,
            .vertex = kLocalPositionVertex,
            .localPosVariable = true });
    ASSERT_NE(material, nullptr);
    EXPECT_TRUE(downcast(material)->hasMotionVectorsVariant());
    Entity const quad = createQuad(mScene, mat4f{}, createMaterialInstance(material));
    ASSERT_TRUE(render({ &tv }));

    mat4 const prev = getWorld(quad);
    setTransform(quad, translation(0.1f, 0.05f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    mat4 const world = getWorld(quad);
    // the transparent half is discarded
    expectNoMotion(tv, world, { { -0.25, 0.0 }, { -0.3, 0.3 } });
    expectQuadMotion(tv, prev, world, { { 0.25, 0.0 }, { 0.3, 0.3 }, { 0.2, -0.3 } });
}

// Case 9: materials without the motion vectors variant are drawn depth-only.
TEST_F(MotionVectorsTest, MotionVectorsFilteredMaterials) {
    TestView& tv = *mView;
    constexpr auto filter = UserVariantFilterMask(UserVariantFilterBit::MOTION_VECTORS);
    Material const* const masked = buildMaterial({
            .blending = BlendingMode::MASKED,
            .variantFilter = filter,
            .fragment = kHalfTransparentFragment,
            .vertex = kLocalPositionVertex,
            .localPosVariable = true });
    Material const* const custom = buildMaterial({
            .variantFilter = filter, .vertex = kDisplacementVertex, .offsetParameter = true });
    ASSERT_NE(masked, nullptr);
    ASSERT_NE(custom, nullptr);
    EXPECT_FALSE(downcast(masked)->hasMotionVectorsVariant());
    EXPECT_FALSE(downcast(custom)->hasMotionVectorsVariant());
    MaterialInstance* const customInstance = createMaterialInstance(custom);
    customInstance->setParameter("offset", float3{ 0.0f });

    // A quad with the default material, behind the MASKED one.
    Entity const back = createQuad(mScene, translation(-0.6f, 0.0f));
    Entity const front = createQuad(mScene, translation(-0.6f, 0.0f, 0.5f),
            createMaterialInstance(masked));
    Entity const other = createQuad(mScene, translation(0.8f, 0.0f) * scaling(0.6f),
            customInstance);
    ASSERT_TRUE(render({ &tv }));

    // They all move.
    mat4 const prevBack = getWorld(back);
    setTransform(back, translation(-0.5f, 0.05f));
    setTransform(front, translation(-0.5f, 0.05f, 0.5f));
    setTransform(other, translation(0.9f, 0.05f) * scaling(0.6f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);

    // The opaque half of the MASKED quad hides the quad behind it, and has no motion vectors...
    mat4 const frontWorld = getWorld(front);
    expectNoMotion(tv, frontWorld, { { 0.25, 0.0 }, { 0.3, 0.3 } });
    // ...but its transparent half shows the quad behind it.
    mat4 const backWorld = getWorld(back);
    for (double2 const local : { double2{ -0.25, 0.0 }, double2{ -0.3, 0.3 } }) {
        expectMotionAt(tv, pixelOf(tv, frontWorld, local), backWorld,
                prevBack * inverse(backWorld));
    }
    // The quad with a custom vertex shader has no motion vectors either.
    expectNoMotion(tv, getWorld(other), kQuadPoints);
}

// Case 9 (continued): warming up a material that draws the motion vectors with the default
// material's programs prepares those, even though its package doesn't have them.
TEST_F(MotionVectorsTest, CompileSharedMotionVectorsPrograms) {
    FEngine& engine = *downcast(mEngine);
    if (!engine.getDriverApi().isParallelShaderCompileSupported()) {
        GTEST_SKIP() << "Engine::compile() only prepares programs with parallel compilation";
    }
    // Even with a custom varying and a non-empty vertex shader that doesn't modify position,
    // the material omits MOT shaders from its package and shares the default material's programs.
    Material const* const material = buildMaterial({
            .vertex = kLocalPositionVertex, .localPosVariable = true });
    ASSERT_NE(material, nullptr);
    FMaterial const* const fmaterial = downcast(material);
    EXPECT_TRUE(fmaterial->hasMotionVectorsVariant());

    FMaterial const* const defaultMaterial = engine.getDefaultMaterial();
    ASSERT_NE(defaultMaterial, nullptr);
    LocalProgramCache::Programs const programs = defaultMaterial->getPrograms().getPrograms();
    auto const isPrepared = [&programs](Variant const variant) {
        return bool(programs[LocalProgramCache::mapCacheEntryKey(
                variant, DynamicSpecConstKey{}, programs.size())]);
    };
    Variant const variants[] = {
            Variant{ Variant::SPECIAL_MOT_VARIANT },
            Variant{ Variant::SPECIAL_MOT_VARIANT | Variant::SKN } };
    for (Variant const variant : variants) {
        SCOPED_TRACE(testing::Message() << "variant " << int(variant.key));
        EXPECT_TRUE(fmaterial->isSharedVariant(variant));
        EXPECT_FALSE(fmaterial->getDefinition().isValidProgram(variant, DynamicSpecConstKey{},
                engine.getShaderModel(), engine.isStereoSupported()));
        // the default material doesn't compile them at init
        EXPECT_FALSE(isPrepared(variant));
    }

    mEngine->compile(backend::CompilerPriorityQueue::HIGH, material, mView->view,
            /* shadowReceiver= */ tribool(false), /* skinning= */ tribool{});
    for (Variant const variant : variants) {
        SCOPED_TRACE(testing::Message() << "variant " << int(variant.key));
        EXPECT_TRUE(isPrepared(variant));
    }
}

// Case 10: an object has no motion at its first render in a Scene, and its first move after that
// is reported.
TEST_F(MotionVectorsTest, FirstMoveAfterAddEntityAndSetScene) {
    TestView& tv = *mView;
    Entity const a = createQuad(mScene, translation(-0.8f, 0.0f) * scaling(0.6f));
    ASSERT_TRUE(render({ &tv }));
    ASSERT_TRUE(render({ &tv }));

    // b is added and moved before its first render, while a moves (so that the pass runs).
    Entity const b = createQuad(nullptr, translation(0.6f, 0.5f) * scaling(0.6f));
    mScene->addEntity(b);
    setTransform(b, translation(0.8f, 0.4f) * scaling(0.6f));
    mat4 const prevA = getWorld(a);
    setTransform(a, translation(-0.7f, 0.0f) * scaling(0.6f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    EXPECT_TRUE(isMoving(tv, a));
    EXPECT_TRUE(isVisible(tv, b));
    EXPECT_FALSE(isMoving(tv, b));
    expectQuadMotion(tv, prevA, getWorld(a), kQuadPoints);
    expectNoMotion(tv, getWorld(b), kQuadPoints);

    // b's first move.
    mat4 const prevB = getWorld(b);
    setTransform(b, translation(0.7f, 0.3f) * rotationZ(10.0f) * scaling(0.6f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    EXPECT_FALSE(isMoving(tv, a));
    EXPECT_TRUE(isMoving(tv, b));
    expectQuadMotion(tv, prevB, getWorld(b), kQuadPoints);
    expectNoMotion(tv, getWorld(a), kQuadPoints);

    // A new Scene: the history can't be used.
    Scene* const scene2 = createScene();
    scene2->addEntity(a);
    scene2->addEntity(b);
    tv.view->setScene(scene2);
    setTransform(b, translation(0.6f, 0.3f) * scaling(0.6f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(tv.prevValid);
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(isVisible(tv, b));
    EXPECT_FALSE(isMoving(tv, b));
    EXPECT_TRUE(tv.isAllZero());

    // b's first move in the new Scene.
    mat4 const prevB2 = getWorld(b);
    setTransform(b, translation(0.5f, 0.2f) * scaling(0.6f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    EXPECT_FALSE(isMoving(tv, a));
    EXPECT_TRUE(isMoving(tv, b));
    expectQuadMotion(tv, prevB2, getWorld(b), kQuadPoints);
    expectNoMotion(tv, getWorld(a), kQuadPoints);
}

// Case 11: like gltfio's Animator, updating the transforms of all the nodes only moves the ones
// whose transform changed.
TEST_F(MotionVectorsTest, TransformTransaction) {
    TestView& tv = *mView;
    TransformManager& tcm = mEngine->getTransformManager();
    Entity const root = createTransform(mat4f{});
    Entity const nodes[3] = {
            createQuad(mScene, translation(-1.0f, 0.0f) * scaling(0.6f), nullptr, root),
            createQuad(mScene, translation(0.0f, 0.0f) * scaling(0.6f), nullptr, root),
            createQuad(mScene, translation(1.0f, 0.0f) * scaling(0.6f), nullptr, root) };
    ASSERT_TRUE(render({ &tv }));
    ASSERT_TRUE(render({ &tv }));

    // Every transform is rewritten with its own value: nothing moved.
    for (Entity const e : { root, nodes[0], nodes[1], nodes[2] }) {
        auto const ti = tcm.getInstance(e);
        mat4f const local = tcm.getTransform(ti);
        tcm.setTransform(ti, local);
    }
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.prevValid);
    EXPECT_FALSE(tv.hadPass);
    for (Entity const node : nodes) {
        EXPECT_TRUE(isVisible(tv, node));
        EXPECT_FALSE(isMoving(tv, node));
    }
    EXPECT_TRUE(tv.isAllZero());

    // Only node 1 changes.
    mat4 const prev1 = getWorld(nodes[1]);
    for (Entity const node : nodes) {
        auto const ti = tcm.getInstance(node);
        mat4f local = tcm.getTransform(ti);
        if (node == nodes[1]) {
            local = translation(0.1f, 0.2f) * rotationZ(15.0f) * scaling(0.6f);
        }
        tcm.setTransform(ti, local);
    }
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    EXPECT_FALSE(isMoving(tv, nodes[0]));
    EXPECT_TRUE(isMoving(tv, nodes[1]));
    EXPECT_FALSE(isMoving(tv, nodes[2]));
    expectNoMotion(tv, getWorld(nodes[0]), kQuadPoints);
    expectQuadMotion(tv, prev1, getWorld(nodes[1]), kQuadPoints);
    expectNoMotion(tv, getWorld(nodes[2]), kQuadPoints);

    // The parent moves: all its children move with it.
    mat4 const prev[3] = { getWorld(nodes[0]), getWorld(nodes[1]), getWorld(nodes[2]) };
    tcm.setTransform(tcm.getInstance(root), translation(0.1f, -0.1f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    for (size_t i = 0; i < 3; i++) {
        SCOPED_TRACE(testing::Message() << "node " << i);
        EXPECT_TRUE(isMoving(tv, nodes[i]));
        expectQuadMotion(tv, prev[i], getWorld(nodes[i]), kQuadPoints);
    }
}

// Case 11 (continued): transforms that change and are restored before the next render are
// reported as changed by the TransformManager, but they didn't move.
TEST_F(MotionVectorsTest, TransformChangedAndRestored) {
    TestView& tv = *mView;
    Entity const root = createTransform(mat4f{});
    Entity const child = createQuad(mScene, translation(-0.6f, 0.0f) * scaling(0.6f), nullptr,
            root);
    Entity const quad = createQuad(mScene, translation(0.6f, 0.0f) * scaling(0.6f));
    ASSERT_TRUE(render({ &tv }));
    ASSERT_TRUE(render({ &tv }));

    // The world transform of a childless entity is updated immediately...
    setTransform(quad, translation(0.6f, 0.5f) * scaling(0.6f));
    setTransform(quad, translation(0.6f, 0.0f) * scaling(0.6f));
    // ...while the children of an entity are updated at the next render.
    setTransform(root, translation(0.0f, 0.5f));
    setTransform(root, mat4f{});
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.prevValid);
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(isVisible(tv, quad));
    EXPECT_FALSE(isMoving(tv, quad));
    EXPECT_TRUE(isVisible(tv, child));
    EXPECT_FALSE(isMoving(tv, child));
    EXPECT_TRUE(tv.isAllZero());
}

// A renderable whose Transform component is destroyed falls back to the null instance, i.e. the
// identity transform. That's a motion like any other: reported once, from where it was.
TEST_F(MotionVectorsTest, TransformComponentDestroyed) {
    TestView& tv = *mView;
    Entity const quad = createQuad(mScene, translation(0.3f, -0.2f) * scaling(0.5f));
    ASSERT_TRUE(render({ &tv }));
    ASSERT_TRUE(render({ &tv }));

    mat4 const prev = getWorld(quad);
    mEngine->getTransformManager().destroy(quad);
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    EXPECT_TRUE(isVisible(tv, quad));
    EXPECT_TRUE(isMoving(tv, quad));
    expectQuadMotion(tv, prev, mat4{}, kQuadPoints);

    // and only once
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(isVisible(tv, quad));
    EXPECT_FALSE(isMoving(tv, quad));
    EXPECT_TRUE(tv.isAllZero());
}

// Case 12: a previous position behind the camera has no projection, which is reported as a
// finite, invalid (|v| >= 1) motion vector.
TEST_F(MotionVectorsTest, PreviousPositionBehindCamera) {
    TestView& tv = *mView;
    // behind the camera (at z = 4)
    Entity const quad = createQuad(mScene, translation(0.0f, 0.0f, 5.0f) * scaling(0.2f));
    ASSERT_TRUE(render({ &tv }));

    setTransform(quad, translation(0.0f, 0.0f, 3.0f) * scaling(0.2f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    size_t invalid = 0;
    for (size_t i = 0; i < tv.pixels.size(); i += 2) {
        float const x = tv.pixels[i];
        float const y = tv.pixels[i + 1];
        ASSERT_TRUE(std::isfinite(x) && std::isfinite(y));
        if (x != 0.0f || y != 0.0f) {
            EXPECT_GE(std::max(std::abs(x), std::abs(y)), 1.0f);
            invalid++;
        }
    }
    EXPECT_GT(invalid, 0u);
    for (double2 const local : kQuadPoints) {
        float2 const v = tv.at(pixelOf(tv, getWorld(quad), local));
        EXPECT_GE(std::max(std::abs(v.x), std::abs(v.y)), 1.0f);
    }
}

// Case 14: with TAA, the renders without history use the zero texture for the object motion.
TEST_F(MotionVectorsTest, TaaWithoutHistory) {
    TestView& tv = *mView;
    tv.view->setTemporalAntiAliasingOptions({ .enabled = true });
    Entity const quad = createQuad(mScene, mat4f{});

    // The first render has no history, even though the quad moved.
    setTransform(quad, translation(0.05f, 0.0f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(tv.prevValid);
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(tv.isAllZero());

    // The camera is jittered, but the motion of a translation parallel to the screen is the same
    // on the whole quad.
    mat4 const prev = getWorld(quad);
    setTransform(quad, translation(0.15f, 0.05f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    expectQuadMotion(tv, prev, getWorld(quad), kQuadPoints);

    // There is no history after clearFrameHistory().
    tv.view->clearFrameHistory(*mEngine);
    setTransform(quad, translation(0.25f, 0.05f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(tv.prevValid);
    EXPECT_FALSE(tv.hadPass);
    EXPECT_TRUE(tv.isAllZero());

    mat4 const prev2 = getWorld(quad);
    setTransform(quad, translation(0.15f, -0.05f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    expectQuadMotion(tv, prev2, getWorld(quad), kQuadPoints);
}

// Case 14 (continued): with motionAdaptiveHistory and depthDisocclusion enabled, a moving object
// resets motionHoldRemaining, skips depthDisocclusion on its own pixels (even when moving in Z),
// and still triggers depthDisocclusion on static background pixels it uncovers.
TEST_F(MotionVectorsTest, TaaMotionAdaptiveAndDepthDisocclusion) {
    TestView& tv = *mView;
    FView const* const view = downcast(tv.view);
    tv.view->setTemporalAntiAliasingOptions({
            .enabled = true,
            .motionAdaptiveHistory = true,
            .motionHoldFrames = 6.0f,
            .depthDisocclusion = true });
    ASSERT_TRUE(mEngine->getDebugRegistry().setProperty("d.taa.visualize_disocclusion", true));

    createQuad(mScene, translation(0.0f, 0.0f, -0.5f) * scaling(2.0f));
    Entity const front = createQuad(mScene, translation(-0.3f, 0.0f, 0.0f) * scaling(0.8f));
    std::vector<uint8_t> image;
    auto const colorAt = [&image](int2 const p) {
        size_t const i = 4 * (size_t(kSize - 1 - p.y) * kSize + size_t(p.x));
        return int3{ image[i], image[i + 1], image[i + 2] };
    };

    // First render seeds history (motionHoldRemaining starts at motionHoldFrames); second render
    // is static so motionHoldRemaining decays by one frame and disocclusion is zero everywhere.
    ASSERT_TRUE(render({ &tv }, &image));
    EXPECT_FLOAT_EQ(view->getFrameHistory().getPrevious().taa.motionHoldRemaining, 6.0f);
    ASSERT_TRUE(render({ &tv }, &image));
    EXPECT_FLOAT_EQ(view->getFrameHistory().getPrevious().taa.motionHoldRemaining, 5.0f);
    for (double2 const local : kQuadPoints) {
        EXPECT_EQ(colorAt(pixelOf(tv, getWorld(front), local)), (int3{ 0, 0, 0 }));
    }

    // The foreground quad translates right and moves toward the camera in Z, uncovering the static
    // background quad where front's center (-0.3, 0, 0) used to sit.
    int2 const uncoveredPixel = pixelOf(tv, double3{ -0.45, 0.0, 0.0 });
    mat4 const prevFront = getWorld(front);
    setTransform(front, translation(0.3f, 0.0f, 0.5f) * scaling(0.8f));
    ASSERT_TRUE(render({ &tv }, &image));
    EXPECT_TRUE(tv.hadPass);
    expectQuadMotion(tv, prevFront, getWorld(front), kQuadPoints);
    // Object motion resets motionHoldRemaining to motionHoldFrames even under a static camera.
    EXPECT_FLOAT_EQ(view->getFrameHistory().getPrevious().taa.motionHoldRemaining, 6.0f);
    // Moving object pixels have hasObjectMotion == true, so depthDisocclusion is skipped despite
    // the Z-axis motion toward the camera.
    for (double2 const local : kQuadPoints) {
        SCOPED_TRACE(testing::Message() << "front local (" << local.x << ", " << local.y << ")");
        EXPECT_EQ(colorAt(pixelOf(tv, getWorld(front), local)), (int3{ 0, 0, 0 }));
    }
    // Newly uncovered static background pixels have v == 0 and fire depthDisocclusion.
    EXPECT_GT(colorAt(uncoveredPixel).r, 200);

    // Once the object stops moving, motionHoldRemaining decays again.
    ASSERT_TRUE(render({ &tv }, &image));
    EXPECT_FALSE(tv.hadPass);
    EXPECT_FLOAT_EQ(view->getFrameHistory().getPrevious().taa.motionHoldRemaining, 5.0f);
}

// Case 14 (continued): TAA reprojects the history of a moving object with its motion vectors, so
// the object's own detail survives the motion. Without them, camera reprojection samples the
// history where the object was, a texel or two off every frame, and smears its detail.
//
// A striped quad translates at a constant 2 texels per frame under TAA, from a converged history.
// Its interior is compared with the same frame rendered without TAA: with motion vectors the
// difference is the small, bounded one of TAA's own reconstruction filter; without them it's the
// smear. The leading and trailing stripes are excluded, so that the comparison measures the
// reprojection of the object's own history rather than the dis/occlusion at its silhouette.
TEST_F(MotionVectorsTest, TaaReprojectsMovingObject) {
    TestView& tv = *mView;
    Material* const material = buildMaterial({
            .fragment = kStripesFragment, .vertex = kLocalPositionVertex,
            .localPosVariable = true });
    ASSERT_NE(material, nullptr);

    // A 2.4 x 1 quad: 46 x 19 pixels at this distance, i.e. 5.8-pixel stripes. It starts on the
    // left and translates right by 2 pixels per frame for 8 frames, staying well inside the view.
    constexpr int kMotionFrames = 8;
    constexpr double kTexelsPerFrame = 2.0;
    constexpr float kQuadWidth = 2.4f;
    mat4f const quadScale = mat4f::scaling(float3{ kQuadWidth, 1.0f, 1.0f });
    Entity const quad = createQuad(mScene, translation(-0.45f, 0.0f) * quadScale,
            createMaterialInstance(material));

    // world units per pixel at the quad's depth (z = 0, camera at z = 4, 45 degrees vertical fov)
    double const unitsPerPixel = 2.0 * 4.0 * std::tan(22.5 * f::DEG_TO_RAD) / double(kSize);
    auto const quadAt = [&](int const frame) {
        return translation(-0.45f + float(kTexelsPerFrame * frame * unitsPerPixel), 0.0f) *
                quadScale;
    };

    std::vector<uint8_t> image;
    auto const lumaAt = [&image](int2 const p) {
        size_t const i = 4 * (size_t(kSize - 1 - p.y) * kSize + size_t(p.x));
        return 0.2126 * image[i] + 0.7152 * image[i + 1] + 0.0722 * image[i + 2];
    };

    // Renders the sequence from a fresh history and returns the luma of the interior of the quad
    // at its last position: one stripe excluded at each end, and the rows near the top and bottom
    // edges. clearFrameHistory() doesn't reset the jitter phase, so each sequence is a multiple
    // of the 16-frame Halton cycle long and the three are comparable pixel for pixel.
    auto const renderSequence = [&](bool const taa, bool const motionVectors) {
        tv.view->setTemporalAntiAliasingOptions({ .enabled = taa });
        tv.view->setMotionVectorsOptions({ .enabled = motionVectors });
        tv.view->clearFrameHistory(*mEngine);
        setTransform(quad, quadAt(0));
        for (int i = 0; i < 32 - kMotionFrames; i++) {   // converge the history on the static quad
            EXPECT_TRUE(render({ &tv }, &image));
        }
        for (int frame = 1; frame <= kMotionFrames; frame++) {
            setTransform(quad, quadAt(frame));
            EXPECT_TRUE(render({ &tv }, &image));
            if (taa && motionVectors) {
                EXPECT_TRUE(tv.hadPass);
                EXPECT_TRUE(isMoving(tv, quad));
            }
        }
        std::vector<double> interior;
        mat4 const world = getWorld(quad);
        double const stripe = 1.0 / kStripeCount;
        int2 const lo = pixelOf(tv, world, double2{ -0.5 + stripe, -0.3 });
        int2 const hi = pixelOf(tv, world, double2{ 0.5 - stripe, 0.3 });
        for (int y = lo.y; y <= hi.y; y++) {
            for (int x = lo.x + 1; x < hi.x; x++) {
                interior.push_back(lumaAt({ x, y }));
            }
        }
        return interior;
    };

    std::vector<double> const reference = renderSequence(false, false);
    std::vector<double> const withMotionVectors = renderSequence(true, true);
    std::vector<double> const withoutMotionVectors = renderSequence(true, false);
    ASSERT_GT(reference.size(), 200u);
    ASSERT_EQ(withMotionVectors.size(), reference.size());
    ASSERT_EQ(withoutMotionVectors.size(), reference.size());

    // mean absolute luma difference with the TAA-less render, in 8-bit units
    auto const meanAbsError = [](std::vector<double> const& a, std::vector<double> const& b) {
        double sum = 0.0;
        for (size_t i = 0; i < a.size(); i++) {
            sum += std::abs(a[i] - b[i]);
        }
        return sum / double(a.size());
    };
    double const errorWith = meanAbsError(withMotionVectors, reference);
    double const errorWithout = meanAbsError(withoutMotionVectors, reference);

    // With motion vectors, what remains is TAA's reconstruction filter softening the 5 stripe
    // edges of the window: 19.5 measured with both lavapipe and llvmpipe. Without them, the
    // smear: 75.0 with both. The bounds leave a 2x margin on each side.
    EXPECT_LT(errorWith, 40.0) << "without motion vectors: " << errorWithout;
    EXPECT_GT(errorWithout, 2.0 * errorWith) << "with motion vectors: " << errorWith;
}

// Identical renderables are drawn with automatic instancing, each with its own motion.
TEST_F(MotionVectorsTest, AutomaticInstancing) {
    TestView& tv = *mView;
    mEngine->setAutomaticInstancingEnabled(true);
    Entity const quads[3] = {
            createQuad(mScene, translation(-1.0f, 0.0f) * scaling(0.6f)),
            createQuad(mScene, translation(0.0f, 0.0f) * scaling(0.6f)),
            createQuad(mScene, translation(1.0f, 0.0f) * scaling(0.6f)) };
    ASSERT_TRUE(render({ &tv }));

    mat4 const prev[3] = { getWorld(quads[0]), getWorld(quads[1]), getWorld(quads[2]) };
    setTransform(quads[0], translation(-1.0f, 0.1f) * scaling(0.6f));
    setTransform(quads[2], translation(0.9f, 0.0f) * rotationZ(10.0f) * scaling(0.6f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    expectQuadMotion(tv, prev[0], getWorld(quads[0]), kQuadPoints);
    expectNoMotion(tv, getWorld(quads[1]), kQuadPoints);
    expectQuadMotion(tv, prev[2], getWorld(quads[2]), kQuadPoints);
}

// ------------------------------------------------------------------------------------------------
// Readback hook and debug view

// The readback texture is only written while motion vectors are active.
TEST_F(MotionVectorsTest, ReadbackHook) {
    TestView& tv = *mView;
    FView* const view = downcast(tv.view);
    Entity const quad = createQuad(mScene, mat4f{});

    tv.view->setMotionVectorsOptions({ .enabled = false });
    ASSERT_TRUE(render({ &tv }));
    EXPECT_FALSE(view->isMotionVectorsActive());
    EXPECT_TRUE(tv.isAll(kSentinel));

    tv.view->setMotionVectorsOptions({ .enabled = true });
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(view->isMotionVectorsActive());
    EXPECT_TRUE(tv.isAllZero());

    mat4 const prev = getWorld(quad);
    setTransform(quad, translation(0.1f, 0.0f));
    ASSERT_TRUE(render({ &tv }));
    EXPECT_TRUE(tv.hadPass);
    expectQuadMotion(tv, prev, getWorld(quad), kQuadPoints);

    FTexture const* const texture = downcast(tv.texture);
#if GTEST_HAS_EXCEPTIONS
    Texture* const rgba = Texture::Builder()
            .width(kSize)
            .height(kSize)
            .format(Texture::InternalFormat::RGBA16F)
            .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::BLIT_DST)
            .build(*mEngine);
    EXPECT_THROW(view->setMotionVectorsReadback({ .texture = downcast(rgba) }),
            PreconditionPanic);

    Texture* const noBlit = Texture::Builder()
            .width(kSize)
            .height(kSize)
            .format(Texture::InternalFormat::RG16F)
            .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE)
            .build(*mEngine);
    EXPECT_THROW(view->setMotionVectorsReadback({ .texture = downcast(noBlit) }),
            PreconditionPanic);

    // the failed calls didn't change the setting
    EXPECT_EQ(view->getMotionVectorsReadback().texture, texture);
    mEngine->destroy(rgba);
    mEngine->destroy(noBlit);
#endif
    EXPECT_EQ(view->getMotionVectorsReadback().texture, texture);
}

// The motion vectors debug view (see FRenderer::FRenderer()) draws the motion vectors over a gray
// image of the View, with the same colors on all backends.
TEST_F(MotionVectorsTest, DebugView) {
    TestView& tv = *mView;
    DebugRegistry& debugRegistry = mEngine->getDebugRegistry();
    Entity const quad = createQuad(mScene, translation(-0.2f, 0.1f));
    // behind the camera (at z = 4)
    Entity const behind = createQuad(mScene, translation(0.0f, 0.0f, 5.0f) * scaling(0.2f));
    std::vector<uint8_t> image;     // of the SwapChain, into which tv renders

    // The color of the pixel p of `image`, (0, 0) being the bottom-left one.
    auto const colorAt = [&image](int2 const p) {
        // readPixels() returns the rows from top to bottom
        size_t const i = 4 * (size_t(kSize - 1 - p.y) * kSize + size_t(p.x));
        return int3{ image[i], image[i + 1], image[i + 2] };
    };
    // Whether c is gray, up to the rounding of its channels.
    auto const isGray = [](int3 const c) {
        return std::max({ c.r, c.g, c.b }) - std::min({ c.r, c.g, c.b }) <= 1;
    };
    // Calls check() with the colors of the pixels through which the points kQuadPoints of the
    // quad `e` are seen.
    auto const checkQuad = [&](Entity const e, auto const& check) {
        mat4 const world = getWorld(e);
        for (double2 const local : kQuadPoints) {
            int2 const pixel = pixelOf(tv, world, local);
            SCOPED_TRACE(testing::Message() << "pixel (" << pixel.x << ", " << pixel.y << ")");
            ASSERT_TRUE(isInside(tv, pixel));
            check(colorAt(pixel));
        }
    };
    auto const expectGraySky = [&]() {
        for (int2 const pixel : getSkyPixels(tv)) {
            SCOPED_TRACE(testing::Message() << "sky pixel (" << pixel.x << ", " << pixel.y << ")");
            EXPECT_TRUE(isGray(colorAt(pixel)));
        }
    };
    auto const expectAllGray = [&]() {
        size_t gray = 0;
        for (int y = 0; y < int(kSize); y++) {
            for (int x = 0; x < int(kSize); x++) {
                gray += isGray(colorAt({ x, y })) ? 1 : 0;
            }
        }
        EXPECT_EQ(gray, size_t(kSize) * kSize);
    };
    auto const expectCheckerboard = [&](Entity const e) {
        mat4 const world = getWorld(e);
        for (double2 const local : kQuadPoints) {
            int2 const pixel = pixelOf(tv, world, local);
            SCOPED_TRACE(testing::Message() << "pixel (" << pixel.x << ", " << pixel.y << ")");
            ASSERT_TRUE(isInside(tv, pixel));
            int const expected = (pixel.x / 8 + pixel.y / 8) % 2 == 1 ? 255 : 0;
            int3 const c = colorAt(pixel);
            EXPECT_EQ(c.r, expected);
            EXPECT_EQ(c.g, expected);
            EXPECT_EQ(c.b, expected);
        }
    };

    // Without the debug view, the sky isn't gray.
    ASSERT_TRUE(render({ &tv }, &image));
    for (int2 const pixel : getSkyPixels(tv)) {
        EXPECT_FALSE(isGray(colorAt(pixel)));
    }

    // Without history, the debug view shows no motion.
    ASSERT_TRUE(debugRegistry.setProperty("d.renderer.display_motion_vectors", true));
    tv.view->clearFrameHistory(*mEngine);
    lookFrom(0.0, -0.1, 4.0);
    ASSERT_TRUE(render({ &tv }, &image));
    EXPECT_FALSE(tv.prevValid);
    expectAllGray();

    // The camera pans up: the quad moves down, which is violet (blue, and half as much red). The
    // sky doesn't move, since the camera only translated, so it's gray.
    lookFrom(0.0, 0.1, 4.0);
    ASSERT_TRUE(render({ &tv }, &image));
    EXPECT_TRUE(tv.prevValid);
    checkQuad(quad, [](int3 const c) {
        EXPECT_GT(c.b - c.r, 16);
        EXPECT_GT(c.r - c.g, 16);
    });
    expectGraySky();

    // The camera pans right: the quad moves left, which is cyan (green and blue).
    lookFrom(0.1, 0.1, 4.0);
    ASSERT_TRUE(render({ &tv }, &image));
    checkQuad(quad, [](int3 const c) {
        EXPECT_NEAR(c.g, c.b, 1);
        EXPECT_GT(c.g - c.r, 16);
    });
    expectGraySky();

    // The object motion vectors of this static scene are zero: the whole image is gray.
    ASSERT_TRUE(debugRegistry.setProperty("d.renderer.display_motion_vectors_object_only", true));
    lookFrom(0.0, 0.0, 4.0);
    ASSERT_TRUE(render({ &tv }, &image));
    EXPECT_FALSE(downcast(tv.view)->hasMovingRenderables());
    expectAllGray();

    // The quad moves right, which is red.
    setTransform(quad, translation(-0.1f, 0.1f));
    ASSERT_TRUE(render({ &tv }, &image));
    EXPECT_TRUE(downcast(tv.view)->hasMovingRenderables());
    int3 const objectOnlyColor = colorAt(pixelOf(tv, getWorld(quad), { 0.0, 0.0 }));
    checkQuad(quad, [](int3 const c) {
        EXPECT_NEAR(c.g, c.b, 1);
        EXPECT_GT(c.r - c.g, 16);
    });
    expectGraySky();

    // When the color pass's padding changes between frames (e.g. toggling the guard band),
    // objectMotionScale converts the previous frame's render-target UV motion to the current
    // frame's, so the same object motion produces the same debug color in object-only mode.
    setTransform(quad, translation(-0.2f, 0.1f));
    tv.view->setGuardBandOptions({ .enabled = true });
    ASSERT_TRUE(render({ &tv }, &image));
    tv.view->setGuardBandOptions({ .enabled = false });
    setTransform(quad, translation(-0.1f, 0.1f));
    ASSERT_TRUE(render({ &tv }, &image));
    int3 const scaledObjectOnlyColor = colorAt(pixelOf(tv, getWorld(quad), { 0.0, 0.0 }));
    EXPECT_NEAR(scaledObjectOnlyColor.r, objectOnlyColor.r, 1);
    EXPECT_NEAR(scaledObjectOnlyColor.g, objectOnlyColor.g, 1);
    EXPECT_NEAR(scaledObjectOnlyColor.b, objectOnlyColor.b, 1);

    // A quad that was behind the camera has no valid previous position, which is drawn as a black
    // and white checkerboard of 8x8 pixel squares, from the bottom-left corner on all backends.
    ASSERT_TRUE(debugRegistry.setProperty("d.renderer.display_motion_vectors_object_only", false));
    setTransform(behind, translation(0.0f, 0.0f, 3.0f) * scaling(0.2f));
    ASSERT_TRUE(render({ &tv }, &image));
    expectCheckerboard(behind);
    expectGraySky();

    // Same when the quad doesn't move, but was behind the camera at the previous render.
    lookFrom(0.0, 0.0, 2.9);
    ASSERT_TRUE(render({ &tv }, &image));
    lookFrom(0.0, 0.0, 4.0);
    ASSERT_TRUE(render({ &tv }, &image));
    EXPECT_FALSE(downcast(tv.view)->hasMovingRenderables());
    expectCheckerboard(behind);
    expectGraySky();

    // Without motion vectors, the debug view has no effect.
    tv.view->setMotionVectorsOptions({ .enabled = false });
    ASSERT_TRUE(render({ &tv }, &image));
    std::vector<uint8_t> const debugImage = image;
    ASSERT_TRUE(debugRegistry.setProperty("d.renderer.display_motion_vectors", false));
    ASSERT_TRUE(render({ &tv }, &image));
    EXPECT_TRUE(image == debugImage);
}
