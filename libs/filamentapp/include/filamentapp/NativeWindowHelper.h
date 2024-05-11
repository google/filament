/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMENT_NATIVE_WINDOW_HELPER_H
#define TNT_FILAMENT_NATIVE_WINDOW_HELPER_H

struct SDL_Window;

extern "C" void* getNativeWindow(SDL_Window* sdlWindow);

#if defined(__APPLE__)
// Add a backing CAMetalLayer to the NSView and return the layer.
extern "C" void* setUpMetalLayer(void* nativeWindow);
// Setup the window the way Filament expects (color space, etc.).
extern "C" void prepareNativeWindow(SDL_Window* sdlWindow);
// Resize the backing CAMetalLayer's drawable to match the new view's size. Returns the layer.
extern "C" void* resizeMetalLayer(void* nativeView);
#endif

#endif // TNT_FILAMENT_NATIVE_WINDOW_HELPER_H
