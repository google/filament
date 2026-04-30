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

#ifndef TNT_FILAMENT_FGVIEWERMANAGER_H
#define TNT_FILAMENT_FGVIEWERMANAGER_H

#include "details/Engine.h"
#include "PostProcessManager.h"

#include <backend/Handle.h>

#include <fgviewer/DebugServer.h>

#include <utils/CString.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace filament {

class FEngine;
class FView;
class FrameGraph;

class FgviewerManager {

public:
    FgviewerManager(FEngine& engine, utils::CString const& serverPort);

    // Encapsulates a texture readback request
    struct Request {
        using Callback = fgviewer::DebugServer::ReabackFinishedCallback;
        fgviewer::ViewHandle viewId;
        uint32_t id;
        utils::CString name;
        Callback callback;
    };

    // FgViewer offsers the feature of viewing intermediate rendered textures (render targets).
    // Each texture client wants to view is a request to readback the corresponding texture in the
    // framegraph.  This method adds readback passes to the framegraph for each readback request.
    void addReadbacksToFramegraph(FrameGraph& fg,
            backend::Handle<backend::HwRenderTarget> viewTarget, uint32_t viewId);

    // Updates FgViewer's framegraph state with the latest.
    void framegraphUpdated(FrameGraph& fg, FView const& view);

    // This signals to fgviewer that the framegraph has been executed.
    void framegraphExecuted();

    // Returns whether the server has been initialized (e.g. it could have failed because another
    // process owns the port).
    bool isReady() const;

    fgviewer::ViewHandle createView(utils::CString name);
    void destroyView(fgviewer::ViewHandle h);

private:
    void requestTextureReadback(fgviewer::ViewHandle viewId, uint32_t id,
            utils::CString const& name, Request::Callback&& callback);

    std::unique_ptr<fgviewer::DebugServer> mServer;
    utils::Mutex mReadbackRequestsMutex;
    std::vector<Request> mReadbackRequests;

    // Needed for adding a resolve pass for multi-sampled rendertargets.
    PostProcessManager& mPostProcessManager;
};

} // namespace filament

#endif // TNT_FILAMENT_FGVIEWERMANAGER_H
