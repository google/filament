///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxReflection.h                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides the needed headers and defines for D3D reflection.               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _WIN32
#include "dxc/WinAdapter.h"
// need to disable this as it is voilated by this header
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
// Need to instruct non-windows compilers on what an interface is
#define interface struct
#include "d3d12shader.h"
#undef interface
#pragma GCC diagnostic pop
#else
#include <d3d12shader.h>
#endif
