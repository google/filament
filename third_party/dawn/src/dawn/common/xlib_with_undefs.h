// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_XLIB_WITH_UNDEFS_H_
#define SRC_DAWN_COMMON_XLIB_WITH_UNDEFS_H_

#include "dawn/common/Platform.h"

#if !DAWN_PLATFORM_IS(LINUX)
#error "xlib_with_undefs.h included on non-Linux"
#endif

// This header includes <X11/Xlib.h> but removes all the extra defines that conflict with
// identifiers in internal code. It should never be included in something that is part of the public
// interface.
#include <X11/Xlib.h>

// Xlib-xcb.h technically includes Xlib.h but we separate the includes to make it more clear what
// the problem is if one of these two includes fail.
#include <X11/Xlib-xcb.h>

#undef Success
#undef None
#undef Always
#undef Bool
#undef Status
#undef False
#undef True

using XErrorHandler = int (*)(Display*, XErrorEvent*);

#endif  // SRC_DAWN_COMMON_XLIB_WITH_UNDEFS_H_
