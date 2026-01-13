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

#include <functional>
#include <unordered_map>
#include <vector>
#include <set>

class CivetServer;

namespace filament::fgviewer {

using ViewHandle = uint32_t;
class WebSocketHandler;

/**
 * Server-side frame graph debugger.
 *
 * This class manages an HTTP server. It receives frame graph packages from the Filament C++ engine or
 * from a standalone tool such as fginfo.
 */
class DebugServer {
public:
    using PixelBuffer = std::vector<uint8_t>;
    using PixelDataFormat = uint32_t;
    struct Format {
        static constexpr PixelDataFormat RGB = 0;
        static constexpr PixelDataFormat RGBA = 1;
    };
    using ReadbackRequest = std::function<void(utils::CString,
            std::function<void(PixelBuffer, uint32_t, uint32_t, PixelDataFormat)>)>;

    static std::string_view const kSuccessHeader;
    static std::string_view const kErrorHeader;

    explicit DebugServer(int port, ReadbackRequest&& readbackRequester);
    ~DebugServer();

    /**
     * Notifies the debugger that a new view has been added.
     */
    ViewHandle createView(utils::CString name);

    /**
     * Notifies the debugger that the given view has been deleted.
     */
    void destroyView(ViewHandle h);
    void update(ViewHandle h, FrameGraphInfo info);

    void broadcast(const char* data, size_t len);
    void tick();

    bool isReady() const { return mServer; }

private:
    void startMonitoring(const utils::CString& resourceName);
    void stopMonitoring(const utils::CString& resourceName);

    CivetServer* mServer;

    ReadbackRequest mReadbackRequester;

    std::unordered_map<ViewHandle, FrameGraphInfo> mViews;
    uint32_t mViewCounter = 0;
    mutable utils::Mutex mViewsMutex;

    std::set<utils::CString> mMonitoredResources;
    mutable utils::Mutex mMonitoredResourcesMutex;
    uint32_t mFrameCount = 0;
    uint32_t mCurrentMonitoredResourceIndex = 0;

    class FileRequestHandler* mFileHandler = nullptr;
    class ApiHandler* mApiHandler = nullptr;
    class WebSocketHandler* mWebSocketHandler = nullptr;

    friend class FileRequestHandler;
    friend class ApiHandler;
    friend class WebSocketHandler;
};

} // namespace filament::fgviewer

#endif  // FGVIEWER_DEBUGSERVER_H
