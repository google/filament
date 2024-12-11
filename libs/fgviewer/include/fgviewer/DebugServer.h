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

#include <utils/CString.h>

#include <tsl/robin_map.h>
#include <utils/Mutex.h>

class CivetServer;

namespace filament::fgviewer {

using FrameGraphInfoKey = uint32_t;

struct FrameGraphPassInfo {
    utils::CString pass_name;
    // TODO: Add struct detail properties
};

struct FrameGraphInfo {
    utils::CString view_name;
    std::vector<FrameGraphPassInfo> passes;
};

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

    DebugServer(int port);
    ~DebugServer();

    /**
     * Notifies the debugger that a new view has been added.
     */
    void addView(const utils::CString& name, FrameGraphInfo info);

    /**
     * Notifies the debugger that the given view has been deleted.
     */
    void removeView(const utils::CString& name);

    /**
     * Updates the information for a given view.
     */
    void updateView(const utils::CString& name, FrameGraphInfo info);

    bool isReady() const { return mServer; }

private:
    static FrameGraphInfoKey getKeybyString(const utils::CString &input,
                                            uint32_t seed);

    CivetServer* mServer;

    tsl::robin_map<FrameGraphInfoKey, FrameGraphInfo> mViews;
    mutable utils::Mutex mViewsMutex;

    class FileRequestHandler* mFileHandler = nullptr;
    class ApiHandler* mApiHandler = nullptr;

    friend class FileRequestHandler;
    friend class ApiHandler;
};

} // namespace filament::fgviewer

#endif  // FGVIEWER_DEBUGSERVER_H
