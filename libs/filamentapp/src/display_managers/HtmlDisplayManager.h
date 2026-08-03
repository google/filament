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

#ifndef TNT_FILAMENT_FILAMENTAPP_WEB_DISPLAY_MANAGER_H
#define TNT_FILAMENT_FILAMENTAPP_WEB_DISPLAY_MANAGER_H

#include <filamentapp/Config.h>
#include <filamentapp/DisplayManager.h>

#include <utils/CString.h>
#include <utils/Mutex.h>

#include <CivetServer.h>

#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

namespace filament::app {

class HtmlDisplayManager : public DisplayManager {
public:
    HtmlDisplayManager();
    ~HtmlDisplayManager() override;

    bool init(const Config& config) override;
    void terminate() override;

    FilamentApp2::Window::Handle createWindow(const char* title, uint32_t w, uint32_t h,
            bool resizable, bool headless) override;
    void destroyWindow(FilamentApp2::Window::Handle window) override;

    void* getNativeWindow(FilamentApp2::Window::Handle window) const override;

    void setWindowTitle(FilamentApp2::Window::Handle window, const char* title) override;
    void getWindowSize(FilamentApp2::Window::Handle window, uint32_t* w, uint32_t* h) const override;
    void getDrawableSize(FilamentApp2::Window::Handle window, uint32_t* w,
            uint32_t* h) const override;

    void pollEvents(std::vector<AppEvent>& events) override;

    uint32_t getMouseState(int* x, int* y) const override;

    bool isWindowFocused(FilamentApp2::Window::Handle window) const override { return true; }

    double getTime() const override;

    void onFrameFinished(FilamentApp2::Window::Handle window, filament::Engine* engine,
            filament::Renderer* renderer) override;

    void startRendering(std::function<bool()> doFrame) override;

private:
    struct WindowInfo {
        utils::CString title;
        uint32_t width;
        uint32_t height;
    };

    class WebSocketHandler : public CivetWebSocketHandler {
    public:
        WebSocketHandler(HtmlDisplayManager* platform)
                : mDisplayManager(platform) {}
        bool handleConnection(CivetServer* server, const struct mg_connection* conn) override;
        void handleReadyState(CivetServer* server, struct mg_connection* conn) override;
        bool handleData(CivetServer* server, struct mg_connection* conn, int bits, char* data,
                size_t data_len) override;
        void handleClose(CivetServer* server, const struct mg_connection* conn) override;

    private:
        HtmlDisplayManager* mDisplayManager;
    };

    mutable utils::Mutex mMutex;
    std::unique_ptr<CivetServer> mServer;
    std::unique_ptr<WebSocketHandler> mWebSocketHandler;
    std::vector<struct mg_connection*> mConnections;

    std::unordered_map<FilamentApp2::Window::Handle, WindowInfo> mWindows;
    std::queue<AppEvent> mEventQueue;
    uint32_t mMouseButtons = 0;
    int32_t mMouseX = 0;
    int32_t mMouseY = 0;
    Config mConfig;
    uint64_t mStartTime;
};

} // namespace filament::app

#endif // TNT_FILAMENT_FILAMENTAPP_WEB_DISPLAY_MANAGER_H
