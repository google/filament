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
#include <cstring>
#include <thread>

namespace filament::app {

namespace {
static constexpr uint8_t EVENT_QUIT = 0;
static constexpr uint8_t EVENT_KEYDOWN = 1;
static constexpr uint8_t EVENT_KEYUP = 2;
static constexpr uint8_t EVENT_MOUSE_WHEEL = 3;
static constexpr uint8_t EVENT_MOUSE_BUTTON_DOWN = 4;
static constexpr uint8_t EVENT_MOUSE_BUTTON_UP = 5;
static constexpr uint8_t EVENT_MOUSE_MOVE = 6;
static constexpr uint8_t EVENT_RESIZE = 7;
} // anonymous namespace

using namespace utils;

// If set to 0, this serves HTML from a resgen resource. Use 1 only during local development, which
// serves files directly from the source code tree.
#define SERVE_FROM_SOURCE_TREE 0

#if SERVE_FROM_SOURCE_TREE
namespace {
utils::CString const BASE_URL("libs/filamentapp/src/display_managers/web");
} // namespace
#else
#include "generated/web_resources/filamentapp_web_resources.h"
#include <unordered_map>
namespace {
struct Asset {
    std::string_view mime;
    std::string_view data;
};
std::unordered_map<std::string_view, Asset> ASSET_MAP;
void initAssetMap() {
    if (!ASSET_MAP.empty()) return;
    ASSET_MAP["/index.html"] = {
        .mime = "text/html",
        .data = { (char const*) FILAMENTAPP_WEB_RESOURCES_INDEX_DATA },
    };
    ASSET_MAP["/style.css"] = {
        .mime = "text/css",
        .data = { (char const*) FILAMENTAPP_WEB_RESOURCES_STYLE_DATA },
    };
    ASSET_MAP["/app.js"] = {
        .mime = "text/javascript",
        .data = { (char const*) FILAMENTAPP_WEB_RESOURCES_APP_DATA },
    };
}
} // namespace
#endif

class WebHandler : public CivetHandler {
public:
    bool handleGet(CivetServer* server, struct mg_connection* conn) override {
        const struct mg_request_info* req_info = mg_get_request_info(conn);
        utils::CString uri(req_info->local_uri);
        if (uri == "/") {
            uri = utils::CString("/index.html");
        }

#if SERVE_FROM_SOURCE_TREE
        if (uri == "/index.html" || uri == "/style.css" || uri == "/app.js") {
            mg_send_file(conn, (BASE_URL + uri).c_str());
            return true;
        }
#else
        initAssetMap();
        auto const& asset_itr = ASSET_MAP.find(uri);
        if (asset_itr != ASSET_MAP.end()) {
            auto const& mime = asset_itr->second.mime;
            auto const& data = asset_itr->second.data;
            mg_printf(conn,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: %.*s\r\n"
                    "Content-Length: %zu\r\n"
                    "Connection: close\r\n\r\n",
                    (int) mime.size(), mime.data(), data.size());
            mg_write(conn, data.data(), data.size());
            return true;
        }
#endif
        return false;
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
        mServer->addHandler("", &sWebHandler);
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

FilamentApp2::Window::Handle HtmlDisplayManager::createWindow(const char* title, uint32_t w,
        uint32_t h, bool resizable, bool headless) {
    LockGuard<Mutex> lock(mMutex);
    FilamentApp2::Window::Handle handle =
            (FilamentApp2::Window::Handle)(uintptr_t) (mWindows.size() + 1);
    mWindows[handle] = { utils::CString(title), w, h };
    return handle;
}

void HtmlDisplayManager::destroyWindow(FilamentApp2::Window::Handle window) {
    LockGuard<Mutex> lock(mMutex);
    mWindows.erase(window);
}

void* HtmlDisplayManager::getNativeWindow(FilamentApp2::Window::Handle window) const {
    return nullptr; // Headless
}

void HtmlDisplayManager::setWindowTitle(FilamentApp2::Window::Handle window, const char* title) {
    LockGuard<Mutex> lock(mMutex);
    if (mWindows.count(window)) {
        mWindows[window].title = utils::CString(title);
    }
}

void HtmlDisplayManager::getWindowSize(FilamentApp2::Window::Handle window, uint32_t* w,
        uint32_t* h) const {
    LockGuard<Mutex> lock(mMutex);
    if (mWindows.count(window)) {
        const auto& info = mWindows.at(window);
        *w = info.width;
        *h = info.height;
    }
}

void HtmlDisplayManager::getDrawableSize(FilamentApp2::Window::Handle window, uint32_t* w,
        uint32_t* h) const {
    getWindowSize(window, w, h);
}

#ifdef __APPLE__
extern "C" void pumpCocoaEvents();
#endif

void HtmlDisplayManager::pollEvents(std::vector<AppEvent>& events) {
#ifdef __APPLE__
    pumpCocoaEvents();
#endif

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

void HtmlDisplayManager::onFrameFinished(FilamentApp2::Window::Handle window,
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
    // Handshake will add to mConnections once magic is verified
}

bool HtmlDisplayManager::WebSocketHandler::handleData(CivetServer* server,
        struct mg_connection* conn, int bits, char* data, size_t data_len) {
    if (data_len < 1) return true;

    int opcode = bits & 0xf;
    if (opcode == MG_WEBSOCKET_OPCODE_TEXT) {
        std::string_view text(data, data_len);
        if (text.find("\"magic\":\"FILAMENT_HEADLESS\"") != std::string_view::npos &&
                text.find("\"type\":\"handshake\"") != std::string_view::npos) {

            utils::CString ack(
                    ("{\"type\":\"handshake_ack\",\"magic\":\"FILAMENT_HEADLESS\",\"event_map\":{"
                     "\"QUIT\":" +
                            std::to_string(EVENT_QUIT) +
                            ","
                            "\"KEYDOWN\":" +
                            std::to_string(EVENT_KEYDOWN) +
                            ","
                            "\"KEYUP\":" +
                            std::to_string(EVENT_KEYUP) +
                            ","
                            "\"MOUSE_WHEEL\":" +
                            std::to_string(EVENT_MOUSE_WHEEL) +
                            ","
                            "\"MOUSE_BUTTON_DOWN\":" +
                            std::to_string(EVENT_MOUSE_BUTTON_DOWN) +
                            ","
                            "\"MOUSE_BUTTON_UP\":" +
                            std::to_string(EVENT_MOUSE_BUTTON_UP) +
                            ","
                            "\"MOUSE_MOVE\":" +
                            std::to_string(EVENT_MOUSE_MOVE) +
                            ","
                            "\"RESIZE\":" +
                            std::to_string(EVENT_RESIZE) + "}}")
                            .c_str());

            mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, ack.data(), ack.size());

            LockGuard<Mutex> lock(mDisplayManager->mMutex);
            if (std::find(mDisplayManager->mConnections.begin(),
                        mDisplayManager->mConnections.end(),
                        conn) == mDisplayManager->mConnections.end()) {
                mDisplayManager->mConnections.push_back(conn);
            }
        }
        return true;
    }

    if (opcode != MG_WEBSOCKET_OPCODE_BINARY) {
        return true;
    }

    {
        LockGuard<Mutex> lock(mDisplayManager->mMutex);
        if (std::find(mDisplayManager->mConnections.begin(), mDisplayManager->mConnections.end(),
                    conn) == mDisplayManager->mConnections.end()) {
            return true; // Connection not handshaked yet
        }
    }

    AppEvent event;
    uint8_t type = (uint8_t) data[0];

    LockGuard<Mutex> lock(mDisplayManager->mMutex);

    switch (type) {
        case EVENT_QUIT:
            event.type = AppEvent::Type::QUIT;
            break;
        case EVENT_KEYDOWN:
        case EVENT_KEYUP:
            if (data_len < 5) return true;
            event.type = (type == EVENT_KEYDOWN) ? AppEvent::Type::KEYDOWN : AppEvent::Type::KEYUP;
            uint32_t code;
            std::memcpy(&code, &data[1], sizeof(code));
            event.key.code = (AppKey) code;
            event.key.modifiers = 0;
            break;
        case EVENT_MOUSE_WHEEL:
            if (data_len < 5) return true;
            event.type = AppEvent::Type::MOUSE_WHEEL;
            std::memcpy(&event.mouseWheel.delta, &data[1], sizeof(event.mouseWheel.delta));
            break;
        case EVENT_MOUSE_BUTTON_DOWN:
        case EVENT_MOUSE_BUTTON_UP:
            if (data_len < 9) return true;
            event.type = (type == EVENT_MOUSE_BUTTON_DOWN) ? AppEvent::Type::MOUSE_BUTTON_DOWN
                                                           : AppEvent::Type::MOUSE_BUTTON_UP;
            event.mouseButton.button = (uint8_t) data[1];
            std::memcpy(&event.mouseButton.x, &data[2], sizeof(event.mouseButton.x));
            std::memcpy(&event.mouseButton.y, &data[6], sizeof(event.mouseButton.y));
            mDisplayManager->mMouseX = event.mouseButton.x;
            mDisplayManager->mMouseY = event.mouseButton.y;
            if (type == EVENT_MOUSE_BUTTON_DOWN) {
                mDisplayManager->mMouseButtons |= (1 << (event.mouseButton.button - 1));
            } else {
                mDisplayManager->mMouseButtons &= ~(1 << (event.mouseButton.button - 1));
            }
            break;
        case EVENT_MOUSE_MOVE:
            if (data_len < 9) return true;
            event.type = AppEvent::Type::MOUSE_MOVE;
            std::memcpy(&event.mouseMove.x, &data[1], sizeof(event.mouseMove.x));
            std::memcpy(&event.mouseMove.y, &data[5], sizeof(event.mouseMove.y));
            mDisplayManager->mMouseX = event.mouseMove.x;
            mDisplayManager->mMouseY = event.mouseMove.y;
            break;
        case EVENT_RESIZE: {
            if (data_len < 9) return true;
            uint32_t width;
            uint32_t height;
            std::memcpy(&width, &data[1], sizeof(width));
            std::memcpy(&height, &data[5], sizeof(height));
            if (!mDisplayManager->mWindows.empty()) {
                auto it = mDisplayManager->mWindows.begin();
                it->second.width = width;
                it->second.height = height;

                event.type = AppEvent::Type::RESIZED;
                event.windowId = it->first;
                event.resize.w = width;
                event.resize.h = height;
                mDisplayManager->mEventQueue.push(event);
            }
            return true;
        }
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

void HtmlDisplayManager::startRendering(std::function<bool()> doFrame) {
    // TODO: use websocket throughput to throttle rendering.
    while (!doFrame()) {
        // Delay rendering for roughly one monitor refresh interval
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

} // namespace filament::app
