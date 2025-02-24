// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FUCHSIA_EGL_BACKEND_H_
#define FUCHSIA_EGL_BACKEND_H_

#include <zircon/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(FUCHSIA_EGL_EXPORT)
#    define FUCHSIA_EGL_EXPORT __attribute__((__visibility__("default")))
#endif

FUCHSIA_EGL_EXPORT
zx_handle_t fuchsia_egl_window_release_image_pipe(fuchsia_egl_window *egl_window);

#ifdef __cplusplus
}
#endif

#endif  // FUCHSIA_EGL_BACKEND_H_
