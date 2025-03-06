//========================================================================
// GLFW 3.5 macOS - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2021 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#define GLFW_COCOA_LIBRARY_TIMER_STATE  _GLFWtimerNS   ns;

// Cocoa-specific global timer data
//
typedef struct _GLFWtimerNS
{
    uint64_t        frequency;
} _GLFWtimerNS;

