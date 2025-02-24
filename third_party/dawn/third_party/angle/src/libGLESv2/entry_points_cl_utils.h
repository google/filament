//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_cl_utils.h: These helpers are used in CL entry point routines.

#ifndef LIBGLESV2_ENTRY_POINTS_CL_UTILS_H_
#define LIBGLESV2_ENTRY_POINTS_CL_UTILS_H_

#include "libANGLE/CLBitField.h"
#include "libANGLE/Debug.h"

#include "common/PackedCLEnums_autogen.h"

#include <cinttypes>
#include <cstdio>

#if defined(ANGLE_ENABLE_DEBUG_TRACE)
#    define CL_EVENT(entryPoint, ...)                    \
        std::printf("CL " #entryPoint ": " __VA_ARGS__); \
        std::printf("\n")
#else
#    define CL_EVENT(entryPoint, ...) (void(0))
#endif

namespace cl
{

// Handling packed enums
template <typename PackedT, typename FromT>
typename std::enable_if_t<std::is_enum<PackedT>::value, PackedT> PackParam(FromT from)
{
    return FromCLenum<PackedT>(from);
}

// Handling bit fields
template <typename PackedT, typename FromT>
typename std::enable_if_t<std::is_same<PackedT, BitField>::value, PackedT> PackParam(FromT from)
{
    return PackedT(from);
}

void InitBackEnds(bool isIcd);

}  // namespace cl

#endif  // LIBGLESV2_ENTRY_POINTS_CL_UTILS_H_
