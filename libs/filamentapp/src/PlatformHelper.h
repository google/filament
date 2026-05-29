/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENTAPP_PLATFORM_HELPER_H
#define TNT_FILAMENTAPP_PLATFORM_HELPER_H

#include <filamentapp/Config.h>

#include <filament/Engine.h>

namespace filament::backend {
class Platform;
} // namespace filament::backend

namespace filament::app {

filament::Engine::Backend resolveBackend(filament::Engine::Backend);

/**
 * Creates a Vulkan platform instance.
 * @param gpuHintCstr A string hint to choose the GPU.
 * @return A pointer to the created Vulkan platform.
 */
filament::backend::Platform* createVulkanPlatform(char const* gpuHintCstr);

/**
 * Destroys a Vulkan platform instance.
 * @param platform The platform to destroy.
 */
void destroyVulkanPlatform(filament::backend::Platform* platform);

/**
 * Creates a WebGPU platform instance.
 * @param forcedBackend The WebGPU backend to force.
 * @return A pointer to the created WebGPU platform.
 */
filament::backend::Platform* createWebGPUPlatform(Config::WebGPUBackend forcedBackend);

/**
 * Destroys a WebGPU platform instance.
 * @param platform The platform to destroy.
 */
void destroyWebGPUPlatform(filament::backend::Platform* platform);

} // namespace filament::app

#endif // TNT_FILAMENTAPP_PLATFORM_HELPER_H
