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

#include "HtmlDisplayManager.h"

#include <backend/PixelBufferDescriptor.h>
#include <filament/Engine.h>

#include <utils/Log.h>
#include <utils/Mutex.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <chrono>

namespace filament::app {

using namespace utils;

class WebHandler : public CivetHandler {
public:
    bool handleGet(CivetServer* server, struct mg_connection* conn) override {
        const char* html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Filament Web View</title>
    <style>
        body { margin: 0; overflow: hidden; background: #000; }
        canvas { display: block; width: 100vw; height: 100vh; object-fit: contain; }
        img { display: block; width: 100vw; height: 100vh; object-fit: contain; }
    </style>
</head>
<body>
    <img id="view" />
    <script>
        const view = document.getElementById('view');
        const ws = new WebSocket('ws://' + location.host + '/ws');
        ws.binaryType = 'arraybuffer';

        ws.onmessage = (event) => {
            const blob = new Blob([event.data], { type: 'image/png' });
            if (view.src) URL.revokeObjectURL(view.src);
            view.src = URL.createObjectURL(blob);
        };

        window.addEventListener('mousedown', (e) => {
            const buffer = new ArrayBuffer(9);
            const view = new DataView(buffer);
            view.setUint8(0, 4); // MOUSE_BUTTON_DOWN
            view.setUint8(1, e.button + 1);
            view.setInt32(2, e.clientX, true);
            view.setInt32(6, e.clientY, true);
            ws.send(buffer);
        });

        window.addEventListener('mouseup', (e) => {
            const buffer = new ArrayBuffer(9);
            const view = new DataView(buffer);
            view.setUint8(0, 5); // MOUSE_BUTTON_UP
            view.setUint8(1, e.button + 1);
            view.setInt32(2, e.clientX, true);
            view.setInt32(6, e.clientY, true);
            ws.send(buffer);
        });

        window.addEventListener('mousemove', (e) => {
            const buffer = new ArrayBuffer(9);
            const view = new DataView(buffer);
            view.setUint8(0, 6); // MOUSE_MOVE
            view.setInt32(1, e.clientX, true);
            view.setInt32(5, e.clientY, true);
            ws.send(buffer);
        });

        window.addEventListener('wheel', (e) => {
            const buffer = new ArrayBuffer(5);
            const view = new DataView(buffer);
            view.setUint8(0, 3); // MOUSE_WHEEL
            view.setInt32(1, e.deltaY > 0 ? -1 : 1, true);
            ws.send(buffer);
        });

        window.addEventListener('keydown', (e) => {
            if (e.repeat) return;
            // Simple mapping for demo
            let code = 0;
            if (e.key === 'Escape') code = 1;
            if (e.key === 'w') code = 56;
            if (e.key === 'a') code = 37;
            if (e.key === 's') code = 55;
            if (e.key === 'd') code = 40;
            if (e.key === 'q') code = 53;
            if (e.key === 'e') code = 41;

            const buffer = new ArrayBuffer(5);
            const view = new DataView(buffer);
            view.setUint8(0, 1); // KEYDOWN
            view.setUint32(1, code, true);
            ws.send(buffer);
        });

        window.addEventListener('keyup', (e) => {
            // Simple mapping for demo
            let code = 0;
            if (e.key === 'Escape') code = 1;
            if (e.key === 'w') code = 56;
            if (e.key === 'a') code = 37;
            if (e.key === 's') code = 55;
            if (e.key === 'd') code = 40;
            if (e.key === 'q') code = 53;
            if (e.key === 'e') code = 41;

            const buffer = new ArrayBuffer(5);
            const view = new DataView(buffer);
            view.setUint8(0, 2); // KEYUP
            view.setUint32(1, code, true);
            ws.send(buffer);
        });
    </script>
</body>
</html>
            )";
        mg_printf(conn,
                "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zd\r\n\r\n%s",
                strlen(html), html);
        return true;
    }
};

static WebHandler sWebHandler;

HtmlDisplayManager::HtmlDisplayManager()
        : mStartTime(std::chrono::steady_clock::now().time_since_epoch().count()) {}

HtmlDisplayManager::~HtmlDisplayManager() { terminate(); }

bool HtmlDisplayManager::init(const Config& config) {
    mConfig = config;
    const char* options[] = { "listening_ports", "8888", nullptr };

    slog.i << "HtmlDisplayManager initializing on port 8888..." << io::endl;

    try {
        mServer = std::make_unique<CivetServer>(options);
        if (!mServer->getContext()) {
            slog.e << "CivetServer failed to start (context is null)" << io::endl;
            return false;
        }
        mWebSocketHandler = std::make_unique<WebSocketHandler>(this);
        mServer->addWebSocketHandler("/ws", mWebSocketHandler.get());
        mServer->addHandler("/", &sWebHandler);
    } catch (const CivetException& e) {
        slog.e << "CivetServer exception: " << e.what() << io::endl;
        return false;
    }

    slog.i << "HtmlDisplayManager started on port 8888" << io::endl;
    return true;
}

void HtmlDisplayManager::terminate() {
    if (mServer) {
        mServer->close();
        mServer.reset();
    }
}

FilamentApp::Window::Handle HtmlDisplayManager::createWindow(const char* title, uint32_t w,
        uint32_t h, bool resizable, bool headless) {
    LockGuard<Mutex> lock(mMutex);
    FilamentApp::Window::Handle handle =
            (FilamentApp::Window::Handle)(uintptr_t) (mWindows.size() + 1);
    mWindows[handle] = { title, w, h };
    return handle;
}

void HtmlDisplayManager::destroyWindow(FilamentApp::Window::Handle window) {
    LockGuard<Mutex> lock(mMutex);
    mWindows.erase(window);
}

void* HtmlDisplayManager::getNativeWindow(FilamentApp::Window::Handle window) const {
    return nullptr; // Headless
}

void HtmlDisplayManager::setWindowTitle(FilamentApp::Window::Handle window, const char* title) {
    LockGuard<Mutex> lock(mMutex);
    if (mWindows.count(window)) {
        mWindows[window].title = title;
    }
}

void HtmlDisplayManager::getWindowSize(FilamentApp::Window::Handle window, uint32_t* w,
        uint32_t* h) const {
    LockGuard<Mutex> lock(mMutex);
    if (mWindows.count(window)) {
        const auto& info = mWindows.at(window);
        *w = info.width;
        *h = info.height;
    }
}

void HtmlDisplayManager::getDrawableSize(FilamentApp::Window::Handle window, uint32_t* w,
        uint32_t* h) const {
    getWindowSize(window, w, h);
}

void HtmlDisplayManager::pollEvents(std::vector<AppEvent>& events) {
    LockGuard<Mutex> lock(mMutex);
    while (!mEventQueue.empty()) {
        events.push_back(mEventQueue.front());
        mEventQueue.pop();
    }
}

uint32_t HtmlDisplayManager::getMouseState(int* x, int* y) const {
    LockGuard<Mutex> lock(mMutex);
    if (x) *x = mMouseX;
    if (y) *y = mMouseY;
    return mMouseButtons;
}

double HtmlDisplayManager::getTime() const {
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    return (double) (now - (int64_t) mStartTime) / 1e9;
}

struct ReadPixelsUser {
    HtmlDisplayManager* displayManager;
    uint32_t width;
    uint32_t height;
};

void HtmlDisplayManager::onFrameFinished(FilamentApp::Window::Handle window,
        filament::Engine* engine, filament::Renderer* renderer) {
    uint32_t width, height;
    getWindowSize(window, &width, &height);

    // TODO: Throttle readPixels
    ReadPixelsUser* user = new ReadPixelsUser{ this, width, height };
    filament::backend::PixelBufferDescriptor desc(
            malloc(width * height * 4), width * height * 4,
            filament::backend::PixelDataFormat::RGBA, filament::backend::PixelDataType::UBYTE,
            [](void* buffer, size_t size, void* userPtr) {
                ReadPixelsUser* user = (ReadPixelsUser*) userPtr;
                int pngSize = 0;
                unsigned char* pngData = stbi_write_png_to_mem((unsigned char*) buffer,
                        user->width * 4, user->width, user->height, 4, &pngSize);

                if (pngData) {
                    user->displayManager->mMutex.lock();
                    for (auto* conn: user->displayManager->mConnections) {
                        mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_BINARY, (char*) pngData,
                                pngSize);
                    }
                    user->displayManager->mMutex.unlock();
                    free(pngData);
                }
                free(buffer);
                delete user;
            },
            user);

    renderer->readPixels(0, 0, width, height, std::move(desc));
}

bool HtmlDisplayManager::WebSocketHandler::handleConnection(CivetServer* server,
        const struct mg_connection* conn) {
    return true;
}

void HtmlDisplayManager::WebSocketHandler::handleReadyState(CivetServer* server,
        struct mg_connection* conn) {
    LockGuard<Mutex> lock(mDisplayManager->mMutex);
    mDisplayManager->mConnections.push_back(conn);
}

bool HtmlDisplayManager::WebSocketHandler::handleData(CivetServer* server,
        struct mg_connection* conn, int bits, char* data, size_t data_len) {
    if (data_len < 1) return true;

    AppEvent event;
    uint8_t type = (uint8_t) data[0];

    LockGuard<Mutex> lock(mDisplayManager->mMutex);

    switch (type) {
        case 0:
            event.type = AppEvent::Type::QUIT;
            break;
        case 1:
        case 2:
            if (data_len < 5) return true;
            event.type = (type == 1) ? AppEvent::Type::KEYDOWN : AppEvent::Type::KEYUP;
            event.key.code = (AppKey) * (uint32_t*) &data[1];
            event.key.modifiers = 0;
            break;
        case 3:
            if (data_len < 5) return true;
            event.type = AppEvent::Type::MOUSE_WHEEL;
            event.mouseWheel.delta = *(int32_t*) &data[1];
            break;
        case 4:
        case 5:
            if (data_len < 9) return true;
            event.type = (type == 4) ? AppEvent::Type::MOUSE_BUTTON_DOWN
                                     : AppEvent::Type::MOUSE_BUTTON_UP;
            event.mouseButton.button = (uint8_t) data[1];
            event.mouseButton.x = *(int32_t*) &data[2];
            event.mouseButton.y = *(int32_t*) &data[6];
            mDisplayManager->mMouseX = event.mouseButton.x;
            mDisplayManager->mMouseY = event.mouseButton.y;
            if (type == 4) {
                mDisplayManager->mMouseButtons |= (1 << (event.mouseButton.button - 1));
            } else {
                mDisplayManager->mMouseButtons &= ~(1 << (event.mouseButton.button - 1));
            }
            break;
        case 6:
            if (data_len < 9) return true;
            event.type = AppEvent::Type::MOUSE_MOVE;
            event.mouseMove.x = *(int32_t*) &data[1];
            event.mouseMove.y = *(int32_t*) &data[5];
            mDisplayManager->mMouseX = event.mouseMove.x;
            mDisplayManager->mMouseY = event.mouseMove.y;
            break;
        default:
            return true;
    }

    mDisplayManager->mEventQueue.push(event);
    return true;
}

void HtmlDisplayManager::WebSocketHandler::handleClose(CivetServer* server,
        const struct mg_connection* conn) {
    LockGuard<Mutex> lock(mDisplayManager->mMutex);
    for (auto it = mDisplayManager->mConnections.begin(); it != mDisplayManager->mConnections.end();
            ++it) {
        if (*it == conn) {
            mDisplayManager->mConnections.erase(it);
            break;
        }
    }
}

} // namespace filament::app
