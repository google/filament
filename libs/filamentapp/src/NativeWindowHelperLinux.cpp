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

#include <filamentapp/NativeWindowHelper.h>

#include <utils/Panic.h>

#include <SDL_syswm.h>

void* getNativeWindow(SDL_Window* sdlWindow) {
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    FILAMENT_CHECK_POSTCONDITION(SDL_GetWindowWMInfo(sdlWindow, &wmi))
            << "SDL version unsupported!";
    if (wmi.subsystem == SDL_SYSWM_X11) {
#if defined(FILAMENT_SUPPORTS_X11)
        Window win = (Window) wmi.info.x11.window;
        return (void *) win;
#endif
    }
    else if (wmi.subsystem == SDL_SYSWM_WAYLAND) {
#if defined(FILAMENT_SUPPORTS_WAYLAND)
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(sdlWindow, &width, &height);

        // Static is used here to allocate the struct pointer for the lifetime of the program.
        // Without static the valid struct quickyly goes out of scope, and ends with seemingly
        // random segfaults.
        static struct {
            struct wl_display *display;
            struct wl_surface *surface;
            uint32_t width;
            uint32_t height;
        } wayland {
            wmi.info.wl.display,
            wmi.info.wl.surface,
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        return (void *) &wayland;
#endif
    }
    return nullptr;
}
