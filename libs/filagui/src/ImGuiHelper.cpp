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

#include <filagui/ImGuiHelper.h>

#include <vector>
#include <unordered_map>

#include <imgui.h>

#include <filamat/MaterialBuilder.h>
#include <filament/Fence.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <utils/EntityManager.h>

using namespace filament::math;
using namespace filament;
using namespace utils;

namespace filagui {

#include "generated/resources/filagui_resources.h"

ImGuiHelper::ImGuiHelper(Engine* engine, filament::View* view, const Path& fontPath) :
        mEngine(engine), mView(view), mScene(engine->createScene()),
        mImGuiContext(ImGui::CreateContext()) {
    ImGuiIO& io = ImGui::GetIO();

    // Create a simple alpha-blended 2D blitting material.
    mMaterial = Material::Builder()
            .package(FILAGUI_RESOURCES_UIBLIT_DATA, FILAGUI_RESOURCES_UIBLIT_SIZE)
            .build(*engine);

    // If the given font path is invalid, ImGui will silently fall back to proggy, which is a
    // tiny "pixel art" texture that is compiled into the library.
    if (!fontPath.isEmpty()) {
        io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);
        createAtlasTexture(engine);
    }

    view->setPostProcessingEnabled(false);
    view->setBlendMode(View::BlendMode::TRANSLUCENT);
    view->setShadowingEnabled(false);

    // Attach a scene for our one and only Renderable.
    view->setScene(mScene);

    EntityManager& em = utils::EntityManager::get();
    mRenderable = em.create();
    mScene->addEntity(mRenderable);

    ImGui::StyleColorsDark();
}

void ImGuiHelper::createAtlasTexture(Engine* engine) {
    engine->destroy(mTexture);
    ImGuiIO& io = ImGui::GetIO();
    // Create the grayscale texture that ImGui uses for its glyph atlas.
    static unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t size = (size_t) (width * height * 4);
    Texture::PixelBufferDescriptor pb(
            pixels, size,
            Texture::Format::RGBA, Texture::Type::UBYTE);
    mTexture = Texture::Builder()
            .width((uint32_t) width)
            .height((uint32_t) height)
            .levels((uint8_t) 1)
            .format(Texture::InternalFormat::RGBA8)
            .sampler(Texture::Sampler::SAMPLER_2D)
            .build(*engine);
    mTexture->setImage(*engine, 0, std::move(pb));

    TextureSampler sampler(TextureSampler::MinFilter::LINEAR, TextureSampler::MagFilter::LINEAR);
    mMaterial->setDefaultParameter("albedo", mTexture, sampler);
}

ImGuiHelper::~ImGuiHelper() {
    mEngine->destroy(mScene);
    mEngine->destroy(mRenderable);
    for (auto& mi : mMaterialInstances) {
        mEngine->destroy(mi);
    }
    mEngine->destroy(mMaterial);
    mEngine->destroy(mTexture);
    for (auto& vb : mVertexBuffers) {
        mEngine->destroy(vb);
    }
    for (auto& ib : mIndexBuffers) {
        mEngine->destroy(ib);
    }
    ImGui::DestroyContext(mImGuiContext);
    mImGuiContext = nullptr;
}

void ImGuiHelper::setDisplaySize(int width, int height, float scaleX, float scaleY) {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
    io.DisplayFramebufferScale.x = scaleX;
    io.DisplayFramebufferScale.y = scaleY;
}

void ImGuiHelper::render(float timeStepInSeconds, Callback imguiCommands) {
    ImGui::SetCurrentContext(mImGuiContext);
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = timeStepInSeconds;
    // First, let ImGui process events and increment its internal frame count.
    // This call will also update the io.WantCaptureMouse, io.WantCaptureKeyboard flag
    // that tells us whether to dispatch inputs (or not) to the app.
    ImGui::NewFrame();
    // Allow the client app to create widgets.
    imguiCommands(mEngine, mView);
    // Let ImGui build up its draw data.
    ImGui::Render();
    // Finally, translate the draw data into Filament objects.
    processImGuiCommands(ImGui::GetDrawData(), ImGui::GetIO());
}

void ImGuiHelper::processImGuiCommands(ImDrawData* commands, const ImGuiIO& io) {
    ImGui::SetCurrentContext(mImGuiContext);

    mHasSynced = false;
    auto& rcm = mEngine->getRenderableManager();

    // Avoid rendering when minimized and scale coordinates for retina displays.
    int fbwidth = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fbheight = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fbwidth == 0 || fbheight == 0)
        return;
    commands->ScaleClipRects(io.DisplayFramebufferScale);

    // Ensure that we have enough vertex buffers and index buffers.
    createBuffers(commands->CmdListsCount);

    // Count how many primitives we'll need, then create a Renderable builder.
    size_t nPrims = 0;
    std::unordered_map<uint64_t, filament::MaterialInstance*> scissorRects;
    for (int cmdListIndex = 0; cmdListIndex < commands->CmdListsCount; cmdListIndex++) {
        const ImDrawList* cmds = commands->CmdLists[cmdListIndex];
        nPrims += cmds->CmdBuffer.size();
    }
    auto rbuilder = RenderableManager::Builder(nPrims);
    rbuilder.boundingBox({{ 0, 0, 0 }, { 10000, 10000, 10000 }}).culling(false);

    // Ensure that we have a material instance for each primitive.
    size_t previousSize = mMaterialInstances.size();
    if (nPrims > mMaterialInstances.size()) {
        mMaterialInstances.resize(nPrims);
        for (size_t i = previousSize; i < mMaterialInstances.size(); i++) {
            mMaterialInstances[i] = mMaterial->createInstance();
        }
    }

    // Recreate the Renderable component and point it to the vertex buffers.
    rcm.destroy(mRenderable);
    int bufferIndex = 0;
    int primIndex = 0;
    for (int cmdListIndex = 0; cmdListIndex < commands->CmdListsCount; cmdListIndex++) {
        const ImDrawList* cmds = commands->CmdLists[cmdListIndex];
        size_t indexOffset = 0;
        populateVertexData(bufferIndex,
                cmds->VtxBuffer.Size * sizeof(ImDrawVert), cmds->VtxBuffer.Data,
                cmds->IdxBuffer.Size * sizeof(ImDrawIdx), cmds->IdxBuffer.Data);
        for (const auto& pcmd : cmds->CmdBuffer) {
            if (pcmd.UserCallback) {
                pcmd.UserCallback(cmds, &pcmd);
            } else {
                MaterialInstance* materialInstance = mMaterialInstances[primIndex];
                materialInstance->setScissor( pcmd.ClipRect.x, fbheight - pcmd.ClipRect.w,
                        (uint16_t) (pcmd.ClipRect.z - pcmd.ClipRect.x),
                        (uint16_t) (pcmd.ClipRect.w - pcmd.ClipRect.y));
                if (pcmd.TextureId) {
                    TextureSampler sampler(TextureSampler::MinFilter::LINEAR, TextureSampler::MagFilter::LINEAR);
                    materialInstance->setParameter("albedo", (Texture const*)pcmd.TextureId, sampler);
                }
                rbuilder
                        .geometry(primIndex, RenderableManager::PrimitiveType::TRIANGLES,
                                mVertexBuffers[bufferIndex], mIndexBuffers[bufferIndex],
                                indexOffset, pcmd.ElemCount)
                        .blendOrder(primIndex, primIndex)
                        .material(primIndex, materialInstance);
                primIndex++;
            }
            indexOffset += pcmd.ElemCount;
        }
        bufferIndex++;
    }
    if (commands->CmdListsCount > 0) {
        rbuilder.build(*mEngine, mRenderable);
    }
}

void ImGuiHelper::createVertexBuffer(size_t bufferIndex, size_t capacity) {
    syncThreads();
    mEngine->destroy(mVertexBuffers[bufferIndex]);
    mVertexBuffers[bufferIndex] = VertexBuffer::Builder()
            .vertexCount(capacity)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0,
                    sizeof(ImDrawVert))
            .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2,
                    sizeof(filament::math::float2), sizeof(ImDrawVert))
            .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4,
                    2 * sizeof(filament::math::float2), sizeof(ImDrawVert))
            .normalized(VertexAttribute::COLOR)
            .build(*mEngine);
}

void ImGuiHelper::createIndexBuffer(size_t bufferIndex, size_t capacity) {
    syncThreads();
    mEngine->destroy(mIndexBuffers[bufferIndex]);
    mIndexBuffers[bufferIndex] = IndexBuffer::Builder()
            .indexCount(capacity)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(*mEngine);
}

void ImGuiHelper::createBuffers(int numRequiredBuffers) {
    if (numRequiredBuffers > mVertexBuffers.size()) {
        size_t previousSize = mVertexBuffers.size();
        mVertexBuffers.resize(numRequiredBuffers, nullptr);
        for (size_t i = previousSize; i < mVertexBuffers.size(); i++) {
            // Pick a reasonable starting capacity; it will grow if needed.
            createVertexBuffer(i, 1000);
        }
    }
    if (numRequiredBuffers > mIndexBuffers.size()) {
        size_t previousSize = mIndexBuffers.size();
        mIndexBuffers.resize(numRequiredBuffers, nullptr);
        for (size_t i = previousSize; i < mIndexBuffers.size(); i++) {
            // Pick a reasonable starting capacity; it will grow if needed.
            createIndexBuffer(i, 5000);
        }
    }
}

void ImGuiHelper::populateVertexData(size_t bufferIndex, size_t vbSizeInBytes, void* vbImguiData,
        size_t ibSizeInBytes, void* ibImguiData)
{
    // Create a new vertex buffer if the size isn't large enough, then copy the ImGui data into
    // a staging area since Filament's render thread might consume the data at any time.
    size_t requiredVertCount = vbSizeInBytes / sizeof(ImDrawVert);
    size_t capacityVertCount = mVertexBuffers[bufferIndex]->getVertexCount();
    if (requiredVertCount > capacityVertCount) {
        createVertexBuffer(bufferIndex, requiredVertCount);
    }
    size_t nVbBytes = requiredVertCount * sizeof(ImDrawVert);
    void* vbFilamentData = malloc(nVbBytes);
    memcpy(vbFilamentData, vbImguiData, nVbBytes);
    mVertexBuffers[bufferIndex]->setBufferAt(*mEngine, 0,
            VertexBuffer::BufferDescriptor(vbFilamentData, nVbBytes,
                [](void* buffer, size_t size, void* user) {
                    free(buffer);
                }, /* user = */ nullptr));

    // Create a new index buffer if the size isn't large enough, then copy the ImGui data into
    // a staging area since Filament's render thread might consume the data at any time.
    size_t requiredIndexCount = ibSizeInBytes / 2;
    size_t capacityIndexCount = mIndexBuffers[bufferIndex]->getIndexCount();
    if (requiredIndexCount > capacityIndexCount) {
        createIndexBuffer(bufferIndex, requiredIndexCount);
    }
    size_t nIbBytes = requiredIndexCount * 2;
    void* ibFilamentData = malloc(nIbBytes);
    memcpy(ibFilamentData, ibImguiData, nIbBytes);
    mIndexBuffers[bufferIndex]->setBuffer(*mEngine,
            IndexBuffer::BufferDescriptor(ibFilamentData, nIbBytes,
                [](void* buffer, size_t size, void* user) {
                    free(buffer);
                }, /* user = */ nullptr));
}

void ImGuiHelper::syncThreads() {
#if UTILS_HAS_THREADING
    if (!mHasSynced) {
        // This is called only when ImGui needs to grow a vertex buffer, which occurs a few times
        // after launching and rarely (if ever) after that.
        Fence::waitAndDestroy(mEngine->createFence());
        mHasSynced = true;
    }
#endif
}

} // namespace filagui
