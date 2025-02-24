//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef ANGLE_TESTS_ANGLE_TEST_PLATFORM_H_
#define ANGLE_TESTS_ANGLE_TEST_PLATFORM_H_

#include <string>

#include "util/util_gl.h"

// Driver vendors
bool IsAdreno();

// Renderer back-ends
bool IsD3D11();
// Is a D3D9-class renderer.
bool IsD3D9();
bool IsDesktopOpenGL();
bool IsOpenGLES();
bool IsOpenGL();
bool IsNULL();
bool IsVulkan();
bool IsMetal();
bool IsD3D();

// Debug/Release
bool IsDebug();
bool IsRelease();

bool EnsureGLExtensionEnabled(const std::string &extName);
bool IsEGLClientExtensionEnabled(const std::string &extName);
bool IsEGLDeviceExtensionEnabled(EGLDeviceEXT device, const std::string &extName);
bool IsEGLDisplayExtensionEnabled(EGLDisplay display, const std::string &extName);
bool IsGLExtensionEnabled(const std::string &extName);
bool IsGLExtensionRequestable(const std::string &extName);

#endif  // ANGLE_TESTS_ANGLE_TEST_PLATFORM_H_
