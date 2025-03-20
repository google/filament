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

#include "WebGPUSurfaceConfiguration.h"
#include <backend/platforms/WebGPUPlatform.h>
#include "WebGPUDriver.h"
#include "WebGPUConstants.h"


#include <webgpu/webgpu_cpp.h>

#include <utils/CString.h>
#include <utils/ostream.h>
#include <iostream>

namespace filament::backend {
WebGPUSurfaceConfiguration::WebGPUSurfaceConfiguration(wgpu::Device device, wgpu::Surface surface, uint32_t width, uint32_t height, wgpu::TextureFormat format)
:mDevice(device), mSurface(surface), mWidth(width), mHeight(height), mFormat(format) {
    ConfigureSwapChain();
}

WebGPUSurfaceConfiguration::~WebGPUSurfaceConfiguration() {
}

void WebGPUSurfaceConfiguration::ConfigureSwapChain() {
FWGPU_LOGW << "Called ConfigureSwapChain" << utils::io::endl;
}

}// namespace filament::backend

