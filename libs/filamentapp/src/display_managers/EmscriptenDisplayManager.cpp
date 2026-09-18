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

#include <filamentapp/EmscriptenDisplayManager.h>

#include <utils/Log.h>
#include <utils/Panic.h>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgl.h>

#include <string>
#include <utility>
#include <vector>

#include <string.h>

// Translates a position in viewport (client) coordinates into the backing-store pixel coordinates
// of the canvas matched by `selector`, and returns 1 if the position lies on the canvas, 0 if it
// does not, and -1 if there is no such canvas.
//
// EmscriptenMouseEvent::targetX is measured from whichever element the listener is registered on,
// which for a window-level listener is the viewport, and canvasX is no longer reported. Scaling by
// the backing store over the CSS size keeps the result correct when the page stretches the canvas.
// The bounding rectangle includes any CSS border and padding, which the canvas is assumed to lack.
EM_JS(int, filamentapp_client_to_canvas,
        (char const* selector, double clientX, double clientY, int* outX, int* outY), {
    const canvas = document.querySelector(UTF8ToString(selector));
    if (!canvas) {
        return -1;
    }
    const rect = canvas.getBoundingClientRect();
    const scaleX = rect.width > 0 ? canvas.width / rect.width : 1;
    const scaleY = rect.height > 0 ? canvas.height / rect.height : 1;
    const x = Math.floor((clientX - rect.left) * scaleX);
    const y = Math.floor((clientY - rect.top) * scaleY);
    HEAP32[outX >> 2] = x;
    HEAP32[outY >> 2] = y;
    return (x >= 0 && y >= 0 && x < canvas.width && y < canvas.height) ? 1 : 0;
});

namespace filament::app {

namespace {

// The extensions filament.js enables explicitly (see web/filament-js/extensions.js).
// enableExtensionsByDefault already covers these; naming them keeps the two paths in step if that
// default changes.
constexpr char const* kRequestedExtensions[] = {
    "WEBGL_compressed_texture_astc",
    "WEBGL_compressed_texture_etc",
    "WEBGL_compressed_texture_s3tc",
    "WEBGL_compressed_texture_s3tc_srgb",
};

// The DOM reports buttons as 0/1/2 (left/middle/right); AppEvent inherits SDL's 1/2/3.
int mapMouseButton(unsigned short domButton) {
    switch (domButton) {
        case 0: return 1;
        case 1: return 2;
        case 2: return 3;
        default: return 0;
    }
}

// Whether the browser's default action for the physical key `domCode` scrolls the page.
bool isScrollKey(char const* domCode) {
    constexpr char const* kScrollKeys[] = {
        "ArrowDown", "ArrowLeft", "ArrowRight", "ArrowUp",
        "End", "Home", "PageDown", "PageUp", "Space",
    };
    for (char const* key: kScrollKeys) {
        if (strcmp(domCode, key) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace

EmscriptenDisplayManager::EmscriptenDisplayManager(filament::Engine::Backend backend,
        const char* canvasSelector)
        : mBackend(backend),
          mCanvasSelector(canvasSelector ? canvasSelector : "#canvas") {
    FILAMENT_CHECK_PRECONDITION(!mCanvasSelector.empty()) << "canvasSelector must not be empty";

    // The context must exist before the Engine does. PlatformWebGL::makeCurrent() is a no-op, so
    // the GL driver adopts whichever context is current and inspects it as it is created, well
    // before FilamentApp2 calls createWindow().
    if (mBackend == filament::Engine::Backend::OPENGL ||
            mBackend == filament::Engine::Backend::DEFAULT) {
        FILAMENT_CHECK_POSTCONDITION(createWebGLContext())
                << "Failed to create a WebGL 2.0 context on " << mCanvasSelector.c_str();
    }
}

EmscriptenDisplayManager::~EmscriptenDisplayManager() { terminate(); }

void EmscriptenDisplayManager::terminate() {
    // Reports the exit. Any frame still scheduled sees the loop is over and unregisters itself.
    endFrameLoop();
    unregisterEventCallbacks();
    if (mGLContext) {
        emscripten_webgl_destroy_context(mGLContext);
        mGLContext = 0;
    }
}

WindowHandle EmscriptenDisplayManager::createWindow(const char* title, uint32_t w, uint32_t h,
        bool resizable) {
    setWindowTitle(nullptr, title);

    // Resizing the canvas resizes the drawing buffer of the already-current context.
    emscripten_set_canvas_element_size(mCanvasSelector.c_str(), (int) w, (int) h);

    // `resizable` cannot be honored: the canvas belongs to the page, which may resize it at will.
    // pollEvents() reports the new size whenever that happens.
    mWindowCreated = true;
    getWindowSize(nullptr, &mWindowWidth, &mWindowHeight);

    registerEventCallbacks();

    // A single canvas means a single window. Use `this` as its (opaque, non-null) handle;
    // FilamentApp2 only requires that it is stable and distinguishable from nullptr.
    return (WindowHandle) this;
}

bool EmscriptenDisplayManager::createWebGLContext() {
    EmscriptenWebGLContextAttributes attributes;
    emscripten_webgl_init_context_attributes(&attributes);

    attributes.majorVersion = 2;
    attributes.minorVersion = 0;
    attributes.alpha = false;
    attributes.depth = true;
    attributes.stencil = true;

    // Filament resolves MSAA itself. A multisampled drawing buffer on top of that could not be
    // read back, and readPixels is how screenshots and renderdiff captures are taken.
    attributes.antialias = false;

    // The browser may clear the drawing buffer whenever it composites, which happens between
    // tasks. Without threading, a frame's commands do not all run in one task: Engine::flush()
    // executes whatever has been recorded so far, and the rest runs from the next frame's
    // Engine::execute(). A screenshot's glReadPixels can therefore land in a later task than the
    // draws it is meant to capture, and would read back a cleared buffer.
    attributes.preserveDrawingBuffer = true;

    attributes.premultipliedAlpha = false;
    attributes.enableExtensionsByDefault = true;
    attributes.failIfMajorPerformanceCaveat = false;

    mGLContext = emscripten_webgl_create_context(mCanvasSelector.c_str(), &attributes);
    if (!mGLContext) {
        return false;
    }

    if (emscripten_webgl_make_context_current(mGLContext) != EMSCRIPTEN_RESULT_SUCCESS) {
        emscripten_webgl_destroy_context(mGLContext);
        mGLContext = 0;
        return false;
    }

    for (char const* extension: kRequestedExtensions) {
        // Absent extensions are not an error; filament probes for them at runtime.
        emscripten_webgl_enable_extension(mGLContext, extension);
    }

    return true;
}

void EmscriptenDisplayManager::destroyWindow(WindowHandle window) {
    // Only the input handlers are released. The canvas belongs to the page, and the context must
    // outlive this call: FilamentApp2::shutdown() destroys the window before the Engine, and the
    // exit callback still reads the last frame back. terminate() releases the context instead.
    unregisterEventCallbacks();
    mWindowCreated = false;
}

void* EmscriptenDisplayManager::getNativeWindow(WindowHandle window) const {
    // PlatformWebGL and WebGPUPlatformWasm both interpret the native window as the canvas' CSS
    // selector string.
    return (void*) mCanvasSelector.c_str();
}

void EmscriptenDisplayManager::setWindowTitle(WindowHandle window, const char* title) {
    if (title) {
        emscripten_set_window_title(title);
    }
}

void EmscriptenDisplayManager::getWindowSize(WindowHandle window, uint32_t* w,
        uint32_t* h) const {
    int width = 0;
    int height = 0;
    emscripten_get_canvas_element_size(mCanvasSelector.c_str(), &width, &height);
    *w = (uint32_t) width;
    *h = (uint32_t) height;
}

void EmscriptenDisplayManager::getDrawableSize(WindowHandle window, uint32_t* w,
        uint32_t* h) const {
    // The canvas' backing store *is* the drawable, so devicePixelRatio is deliberately ignored.
    // That keeps rendered output independent of the display, which renderdiff relies on.
    getWindowSize(window, w, h);
}

uint32_t EmscriptenDisplayManager::getMouseState(int* x, int* y) const {
    if (x) {
        *x = mMouseX;
    }
    if (y) {
        *y = mMouseY;
    }
    return mMouseButtons;
}

bool EmscriptenDisplayManager::isWindowFocused(WindowHandle window) const { return true; }

void EmscriptenDisplayManager::pollEvents(std::vector<AppEvent>& events) {
    // The page can resize the canvas' backing store at any time, and no DOM event reports it:
    // the resize event belongs to the window, and watching the element needs a ResizeObserver.
    // Comparing the size once per poll catches every cause for the price of two property reads.
    if (mWindowCreated) {
        uint32_t width = 0;
        uint32_t height = 0;
        getWindowSize((WindowHandle) this, &width, &height);
        if (width != mWindowWidth || height != mWindowHeight) {
            mWindowWidth = width;
            mWindowHeight = height;

            AppEvent resizeEvent = {};
            resizeEvent.windowId = (WindowHandle) this;
            resizeEvent.type = AppEvent::Type::RESIZED;
            resizeEvent.resize.w = width;
            resizeEvent.resize.h = height;
            mPendingEvents.push_back(resizeEvent);
        }
    }

    // Browser events arrive on callbacks between frames rather than from a queue we can drain, so
    // they are accumulated in mPendingEvents and handed over wholesale here.
    if (mPendingEvents.empty()) {
        return;
    }
    events.insert(events.end(), mPendingEvents.begin(), mPendingEvents.end());
    mPendingEvents.clear();
}

double EmscriptenDisplayManager::getTime() const {
    // Within a frame, report the timestamp requestAnimationFrame gave us: it is the presentation
    // time the browser intends, and it does not drift between reads. Both clocks are milliseconds
    // from performance.timeOrigin, so the fallback below is interchangeable with it.
    if (mFrameTimeMs > 0.0) {
        return mFrameTimeMs * 1.0e-3;
    }
    return emscripten_get_now() * 1.0e-3;
}

void EmscriptenDisplayManager::runFrameLoop(FrameFn frame) {
    FILAMENT_CHECK_PRECONDITION(!mFrameLoopActive) << "runFrameLoop() called twice";
    mFrameFn = std::move(frame);
    mFrameLoopActive = true;

    // Returns immediately; the callback fires on every animation frame until it returns EM_FALSE.
    // Nothing blocks here: a browser cannot be, and rAF already paces us to the refresh rate.
    emscripten_request_animation_frame_loop(&onAnimationFrame, this);
}

EM_BOOL EmscriptenDisplayManager::onAnimationFrame(double time, void* userData) {
    auto* self = static_cast<EmscriptenDisplayManager*>(userData);

    // terminate() may have ended the run and reported the exit while this frame was scheduled.
    if (!self->mFrameLoopActive) {
        return EM_FALSE;
    }

    // Latched before the frame runs, so that every getTime() during it sees the same value.
    self->mFrameTimeMs = time;

    if (!self->mFrameFn()) {
        return EM_TRUE;
    }

    // The frame callback has already run FilamentApp2::shutdown().
    self->endFrameLoop();
    return EM_FALSE;
}

void EmscriptenDisplayManager::endFrameLoop() {
    // Idempotent: a run ends once, so the exit is reported once.
    if (!mFrameLoopActive) {
        return;
    }
    mFrameLoopActive = false;
    // No frame is in progress any more, so getTime() goes back to reading the clock.
    mFrameTimeMs = 0.0;
    if (mExitCallback) {
        mExitCallback();
    }
}

void EmscriptenDisplayManager::registerEventCallbacks() {
    if (mCallbacksRegistered) {
        return;
    }

    // A press or a wheel turn only counts on the canvas.
    char const* canvas = mCanvasSelector.c_str();
    emscripten_set_mousedown_callback(canvas, this, EM_TRUE, &onMouseEvent);
    emscripten_set_wheel_callback(canvas, this, EM_TRUE, &onWheelEvent);

    // Moves, releases and keys go to the window instead. A drag that starts on the canvas
    // routinely leaves it and has to keep tracking until the release, wherever that happens, and
    // the canvas only receives key events while it has focus. onMouseEvent() translates every
    // position into canvas coordinates, so the target a listener sits on does not matter.
    emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE,
            &onMouseEvent);
    emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, &onMouseEvent);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, &onKeyEvent);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, &onKeyEvent);

    mCallbacksRegistered = true;
}

void EmscriptenDisplayManager::unregisterEventCallbacks() {
    if (!mCallbacksRegistered) {
        return;
    }

    char const* canvas = mCanvasSelector.c_str();
    emscripten_set_mousedown_callback(canvas, nullptr, EM_TRUE, nullptr);
    emscripten_set_wheel_callback(canvas, nullptr, EM_TRUE, nullptr);
    emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
    emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, nullptr);

    mCallbacksRegistered = false;
}

EM_BOOL EmscriptenDisplayManager::onMouseEvent(int eventType, EmscriptenMouseEvent const* event,
        void* userData) {
    auto* self = static_cast<EmscriptenDisplayManager*>(userData);

    int x = 0;
    int y = 0;
    int const onCanvas = filamentapp_client_to_canvas(self->mCanvasSelector.c_str(),
            (double) event->clientX, (double) event->clientY, &x, &y);
    if (onCanvas < 0) {
        return EM_FALSE;
    }

    AppEvent appEvent = {};
    appEvent.windowId = (WindowHandle) self;

    switch (eventType) {
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
            // Like SDL, report moves over the canvas, and anywhere at all during a drag.
            if (!onCanvas && self->mMouseButtons == 0) {
                return EM_FALSE;
            }
            appEvent.type = AppEvent::Type::MOUSE_MOVE;
            appEvent.mouseMove.x = (int32_t) x;
            appEvent.mouseMove.y = (int32_t) y;
            break;
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
        case EMSCRIPTEN_EVENT_MOUSEUP: {
            bool const down = eventType == EMSCRIPTEN_EVENT_MOUSEDOWN;
            int const button = mapMouseButton(event->button);
            if (button == 0) {
                return EM_FALSE;
            }
            // The release listener sees every click on the page. Only a release whose press
            // started on the canvas belongs to the app.
            if (!down && (self->mMouseButtons & appMouseButtonMask(button)) == 0) {
                return EM_FALSE;
            }
            appSetMouseButton(self->mMouseButtons, button, down);

            appEvent.type = down ? AppEvent::Type::MOUSE_BUTTON_DOWN
                                 : AppEvent::Type::MOUSE_BUTTON_UP;
            appEvent.mouseButton.button = button;
            appEvent.mouseButton.x = (int32_t) x;
            appEvent.mouseButton.y = (int32_t) y;
            break;
        }
        default:
            return EM_FALSE;
    }

    self->mMouseX = x;
    self->mMouseY = y;
    self->mPendingEvents.push_back(appEvent);

    // Only a press on the canvas is kept from the browser. Window-level moves and releases also
    // serve the rest of the page, such as text selection, so they are left alone.
    return eventType == EMSCRIPTEN_EVENT_MOUSEDOWN ? EM_TRUE : EM_FALSE;
}

EM_BOOL EmscriptenDisplayManager::onWheelEvent(int eventType, EmscriptenWheelEvent const* event,
        void* userData) {
    auto* self = static_cast<EmscriptenDisplayManager*>(userData);

    if (event->deltaY == 0.0) {
        return EM_FALSE;
    }

    AppEvent appEvent = {};
    appEvent.windowId = (WindowHandle) self;
    appEvent.type = AppEvent::Type::MOUSE_WHEEL;
    // deltaY is in pixels, lines or pages depending on deltaMode, and is positive when scrolling
    // down; SDL reports discrete clicks, positive when scrolling up. Only the sign is portable.
    appEvent.mouseWheel.delta = event->deltaY > 0.0 ? -1 : 1;

    self->mPendingEvents.push_back(appEvent);
    return EM_TRUE;
}

EM_BOOL EmscriptenDisplayManager::onKeyEvent(int eventType, EmscriptenKeyboardEvent const* event,
        void* userData) {
    auto* self = static_cast<EmscriptenDisplayManager*>(userData);

    bool const down = eventType == EMSCRIPTEN_EVENT_KEYDOWN;

    AppEvent appEvent = {};
    appEvent.windowId = (WindowHandle) self;
    appEvent.type = down ? AppEvent::Type::KEYDOWN : AppEvent::Type::KEYUP;
    appEvent.key.code = mapKey(event->code);
    appEvent.key.modifiers = mapModifiers(event);
    self->mPendingEvents.push_back(appEvent);

    // KeyboardEvent.key holds the printable character for character-producing keys, and a name
    // longer than one character ("Shift", "ArrowUp", ...) for the rest. A chord with Ctrl or Meta
    // is a shortcut rather than typing, so it produces no text. Ctrl together with Alt is let
    // through, because that is how Windows reports AltGr, which does type characters.
    bool const isShortcut = event->metaKey || (event->ctrlKey && !event->altKey);
    if (down && !isShortcut && event->key[0] != '\0' && strlen(event->key) == 1) {
        AppEvent textEvent = {};
        textEvent.windowId = (WindowHandle) self;
        textEvent.type = AppEvent::Type::TEXTINPUT;
        textEvent.text.text[0] = event->key[0];
        textEvent.text.text[1] = '\0';
        self->mPendingEvents.push_back(textEvent);
    }

    // Unmodified navigation keys drive the camera, and left to the browser they would also scroll
    // the page. Everything else, and every chord, falls through so that the browser's own
    // shortcuts (reload, devtools, ...) and Tab focus navigation keep working.
    bool const hasModifier = event->ctrlKey || event->altKey || event->metaKey;
    return (!hasModifier && isScrollKey(event->code)) ? EM_TRUE : EM_FALSE;
}

uint16_t EmscriptenDisplayManager::mapModifiers(EmscriptenKeyboardEvent const* event) {
    uint16_t modifiers = AppKeyModifier::NONE;
    // The DOM does not distinguish left from right for the modifier *state* of an event, only for
    // the key that produced it, so both sides are reported as pressed.
    if (event->shiftKey) {
        modifiers |= AppKeyModifier::SHIFT;
    }
    if (event->ctrlKey) {
        modifiers |= AppKeyModifier::CTRL;
    }
    if (event->altKey) {
        modifiers |= AppKeyModifier::ALT;
    }
    if (event->metaKey) {
        modifiers |= AppKeyModifier::SUPER;
    }
    return modifiers;
}

AppKey EmscriptenDisplayManager::mapKey(char const* domCode) {
    if (!domCode || domCode[0] == '\0') {
        return AppKey::UNKNOWN;
    }

    // KeyboardEvent.code names a physical key, so this is the direct analogue of SDL's scancodes.
    struct Entry {
        char const* code;
        AppKey key;
    };
    static constexpr Entry kNamedKeys[] = {
        { "AltLeft", AppKey::LEFT_ALT },
        { "AltRight", AppKey::RIGHT_ALT },
        { "ArrowDown", AppKey::DOWN },
        { "ArrowLeft", AppKey::LEFT },
        { "ArrowRight", AppKey::RIGHT },
        { "ArrowUp", AppKey::UP },
        { "Backquote", AppKey::GRAVE },
        { "Backslash", AppKey::BACKSLASH },
        { "Backspace", AppKey::BACKSPACE },
        { "BracketLeft", AppKey::LEFT_BRACKET },
        { "BracketRight", AppKey::RIGHT_BRACKET },
        { "CapsLock", AppKey::CAPS_LOCK },
        { "Comma", AppKey::COMMA },
        { "ContextMenu", AppKey::MENU },
        { "ControlLeft", AppKey::LEFT_CTRL },
        { "ControlRight", AppKey::RIGHT_CTRL },
        { "Delete", AppKey::DEL },
        { "End", AppKey::END },
        { "Enter", AppKey::ENTER },
        { "Equal", AppKey::EQUAL },
        { "Escape", AppKey::ESCAPE },
        { "Home", AppKey::HOME },
        { "Insert", AppKey::INSERT },
        { "MetaLeft", AppKey::LEFT_SUPER },
        { "MetaRight", AppKey::RIGHT_SUPER },
        { "Minus", AppKey::MINUS },
        { "NumLock", AppKey::NUM_LOCK },
        { "NumpadAdd", AppKey::KEYPAD_ADD },
        { "NumpadDecimal", AppKey::KEYPAD_DECIMAL },
        { "NumpadDivide", AppKey::KEYPAD_DIVIDE },
        { "NumpadEnter", AppKey::KEYPAD_ENTER },
        { "NumpadEqual", AppKey::KEYPAD_EQUAL },
        { "NumpadMultiply", AppKey::KEYPAD_MULTIPLY },
        { "NumpadSubtract", AppKey::KEYPAD_SUBTRACT },
        { "PageDown", AppKey::PAGE_DOWN },
        { "PageUp", AppKey::PAGE_UP },
        { "Pause", AppKey::PAUSE },
        { "Period", AppKey::PERIOD },
        { "PrintScreen", AppKey::PRINT_SCREEN },
        { "Quote", AppKey::APOSTROPHE },
        { "ScrollLock", AppKey::SCROLL_LOCK },
        { "Semicolon", AppKey::SEMICOLON },
        { "ShiftLeft", AppKey::LEFT_SHIFT },
        { "ShiftRight", AppKey::RIGHT_SHIFT },
        { "Slash", AppKey::SLASH },
        { "Space", AppKey::SPACE },
        { "Tab", AppKey::TAB },
    };

    for (Entry const& entry: kNamedKeys) {
        if (strcmp(domCode, entry.code) == 0) {
            return entry.key;
        }
    }

    size_t const length = strlen(domCode);

    // "KeyA".."KeyZ", "Digit0".."Digit9", "Numpad0".."Numpad9" and "F1".."F24" are contiguous in
    // AppKey, so they can be computed rather than tabulated.
    if (length == 4 && strncmp(domCode, "Key", 3) == 0 &&
            domCode[3] >= 'A' && domCode[3] <= 'Z') {
        return (AppKey) ((uint32_t) AppKey::A + (uint32_t) (domCode[3] - 'A'));
    }
    if (length == 6 && strncmp(domCode, "Digit", 5) == 0 &&
            domCode[5] >= '0' && domCode[5] <= '9') {
        return (AppKey) ((uint32_t) AppKey::_0 + (uint32_t) (domCode[5] - '0'));
    }
    if (length == 7 && strncmp(domCode, "Numpad", 6) == 0 &&
            domCode[6] >= '0' && domCode[6] <= '9') {
        return (AppKey) ((uint32_t) AppKey::KEYPAD_0 + (uint32_t) (domCode[6] - '0'));
    }
    if (domCode[0] == 'F' && length >= 2 && length <= 3) {
        int number = 0;
        for (size_t i = 1; i < length; i++) {
            if (domCode[i] < '0' || domCode[i] > '9') {
                return AppKey::UNKNOWN;
            }
            number = number * 10 + (domCode[i] - '0');
        }
        if (number >= 1 && number <= 24) {
            return (AppKey) ((uint32_t) AppKey::F1 + (uint32_t) (number - 1));
        }
    }

    return AppKey::UNKNOWN;
}

} // namespace filament::app
