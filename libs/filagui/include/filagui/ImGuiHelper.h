/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef FILAGUI_IMGUIHELPER_H_
#define FILAGUI_IMGUIHELPER_H_

#include <vector>
#include <functional>

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/Entity.h>
#include <utils/Path.h>

struct ImDrawData;
struct ImGuiIO;
struct ImGuiContext;

namespace filagui {

// Translates ImGui's draw commands into Filament primitives, textures, vertex buffers, etc.
// Creates a UI-specific Scene object and populates it with a Renderable. Does not handle
// event processing; clients can simply call ImGui::GetIO() directly and set the mouse state.
class UTILS_PUBLIC ImGuiHelper {
public:
    // Using std::function instead of a vanilla C callback to make it easy for clients to pass in
    // lambdas that have captures.
    using Callback = std::function<void(filament::Engine*, filament::View*)>;

    // The constructor creates its own Scene and places it in the given View.
    ImGuiHelper(filament::Engine* engine, filament::View* view, const utils::Path& fontPath,
            ImGuiContext* imGuiContext = nullptr);
    ~ImGuiHelper();

    // Informs ImGui of the current display size, as well as a scaling factor when scissoring.
    void setDisplaySize(int width, int height, float scaleX = 1.0f,
            float scaleY = 1.0f, bool flipVertical = false);

    // High-level utility method that takes a callback for creating all ImGui windows and widgets.
    // Clients are responsible for rendering the View. This should be called on every frame,
    // regardless of whether the Renderer wants to skip or not.
    void render(float timeStepInSeconds, Callback imguiCommands);

    // Low-level alternative to render() that consumes an ImGui command list and translates it into
    // various Filament calls. This includes updating the vertex buffer, setting up material
    // instances, and rebuilding the Renderable component that encompasses the entire UI. Since this
    // makes Filament calls, it must be called from the main thread.
    void processImGuiCommands(ImDrawData* commands, const ImGuiIO& io);

    // Helper method called after resolving fontPath; public so fonts can be added by caller.
    void createAtlasTexture(filament::Engine* engine);

    // Returns the client-owned view, useful for drawing 2D overlays.
    filament::View* getView() const { return mView; }

  private:
      void createBuffers(int numRequiredBuffers);
      void populateVertexData(size_t bufferIndex, size_t vbSizeInBytes, void* vbData,
                  size_t ibSizeInBytes, void* ibData);
      void createVertexBuffer(size_t bufferIndex, size_t capacity);
      void createIndexBuffer(size_t bufferIndex, size_t capacity);
      void syncThreads();
      filament::Engine* mEngine;
      filament::View* mView; // The view is owned by the client.
      filament::Scene* mScene;
      filament::Material* mMaterial2d = nullptr;
      filament::Material* mMaterialExternal = nullptr;
      filament::Camera* mCamera = nullptr;
      std::vector<filament::VertexBuffer*> mVertexBuffers;
      std::vector<filament::IndexBuffer*> mIndexBuffers;
      std::vector<filament::MaterialInstance*> mMaterial2dInstances;
      std::vector<filament::MaterialInstance*> mMaterialExternalInstances;
      utils::Entity mRenderable;
      utils::Entity mCameraEntity;
      filament::Texture* mTexture = nullptr;
      bool mHasSynced = false;
      ImGuiContext* mImGuiContext;
      filament::TextureSampler mSampler;
      bool mFlipVertical = false;
      utils::Path mSettingsPath;
};

} // namespace filagui

#endif /* FILAGUI_IMGUIHELPER_H_ */
