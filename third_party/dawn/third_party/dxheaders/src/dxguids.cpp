// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// This file's sole purpose is to initialize the GUIDs declared using the DEFINE_GUID macro.
#define INITGUID

#ifndef _WIN32
#include <wsl/winadapter.h>
#endif

#include <directx/dxcore.h>
#include <directx/d3d12.h>