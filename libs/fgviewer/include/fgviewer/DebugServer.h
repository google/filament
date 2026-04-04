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
#include <string_view>
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
    enum class PixelDataFormat : uint8_t { R, RG, RGB, RGBA, L, LA };
    struct Format {
        static constexpr PixelDataFormat R = PixelDataFormat::R;
        static constexpr PixelDataFormat RG = PixelDataFormat::RG;
        static constexpr PixelDataFormat RGB = PixelDataFormat::RGB;
        static constexpr PixelDataFormat RGBA = PixelDataFormat::RGBA;
        static constexpr PixelDataFormat L = PixelDataFormat::L;
        static constexpr PixelDataFormat LA = PixelDataFormat::LA;
    };

    struct FormatInfo {
        bool isDepth = false;
        bool isD16 = false;
        bool isD24 = false;
        bool isD32F = false;
        bool isFloat = false;
        bool isR8 = false;
        uint8_t samples = 1;
    };

    using PixelBuffer = std::vector<uint8_t>;
    using ReadbackRequest = std::function<void(ViewHandle, uint32_t, utils::CString const&,
            std::function<void(PixelBuffer, uint32_t, uint32_t, PixelDataFormat, FormatInfo)>)>;

    static std::string_view const kSuccessHeader;
    static std::string_view const kErrorHeader;

    explicit DebugServer(int port, ReadbackRequest request = {});
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
    void update(ViewHandle h, FrameGraphInfo&& info);

    /**
     * Ticks the server to trigger monitoring requests.
     */
    void tick();

    /**
     * Set a resource to be monitored.
     */
    void setResourceMonitor(ViewHandle viewId, uint32_t resourceId,
            utils::CString const& resourceName, bool enabled);

    /**
     * Clear all resource monitors.
     */
    void clearResourceMonitors();

    struct ActiveMonitor {
        ViewHandle viewId;
        uint32_t resourceId;
        utils::CString name;
    };

    /**
     * Get currently active resource monitors.
     */
    std::vector<ActiveMonitor> getMonitoredResources() const;

    /**
     * Get the latest rendered image for a monitored resource.
     */
    std::vector<uint8_t> getImage(ViewHandle viewId, uint32_t resourceId) const;

    bool isReady() const { return mServer; }

private:
    CivetServer* mServer;

    std::unordered_map<ViewHandle, FrameGraphInfo> mViews;
    uint32_t mViewCounter = 0;
    mutable utils::Mutex mViewsMutex;

    struct ResourceMonitor {
        ViewHandle viewId;
        uint32_t resourceId;
        bool enabled = false;
        bool inProgress = false;
        size_t readbackTick = 0; // Marks the next tick where an actual readback will be issued.
                                 // This relaxes the amount of read back requests per-frame.
        utils::CString name;
        std::vector<uint8_t> lastImage;
    };
    std::unordered_map<uint64_t, ResourceMonitor> mMonitoredResources;
    mutable utils::Mutex mMonitoredResourcesMutex;

    ReadbackRequest mReadbackRequest;

    size_t mTickCount = 0;

    class FileRequestHandler* mFileHandler = nullptr;
    class ApiHandler* mApiHandler = nullptr;

    friend class FileRequestHandler;
    friend class ApiHandler;
};

} // namespace filament::fgviewer

#endif  // FGVIEWER_DEBUGSERVER_H
