/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef FGVIEWER_DEBUGSERVER_H
#define FGVIEWER_DEBUGSERVER_H

#include <fgviewer/FrameGraphInfo.h>

#include <utils/CString.h>
#include <utils/Mutex.h>

#include <unordered_map>
#include <vector>

class CivetServer;

namespace filament::fgviewer {

using ViewHandle = uint32_t;



/**
 * Server-side frame graph debugger.
 *
 * This class manages an HTTP server. It receives frame graph packages from the Filament C++ engine or
 * from a standalone tool such as fginfo.
 */
class DebugServer {
public:
    static std::string_view const kSuccessHeader;
    static std::string_view const kErrorHeader;

    explicit DebugServer(int port);
    ~DebugServer();

    /**
     * Notifies the debugger that a new view has been added.
     */
    ViewHandle createView(utils::CString name);

    /**
     * Notifies the debugger that the given view has been deleted.
     */
    void destroyView(ViewHandle h);

    /**
     * Updates the information for a given view.
     */
    void update(ViewHandle h, FrameGraphInfo info);

    bool isReady() const { return mServer; }

private:
    CivetServer* mServer;

    std::unordered_map<ViewHandle, FrameGraphInfo> mViews;
    uint32_t mViewCounter = 0;
    mutable utils::Mutex mViewsMutex;

    class FileRequestHandler* mFileHandler = nullptr;
    class ApiHandler* mApiHandler = nullptr;

    friend class FileRequestHandler;
    friend class ApiHandler;
};

} // namespace filament::fgviewer

#endif  // FGVIEWER_DEBUGSERVER_H
