// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FUCHSIA_EGL_H_
#define FUCHSIA_EGL_H_

#include <inttypes.h>
#include <zircon/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(FUCHSIA_EGL_EXPORT)
#    define FUCHSIA_EGL_EXPORT __attribute__((__visibility__("default")))
#endif

typedef struct fuchsia_egl_window fuchsia_egl_window;

FUCHSIA_EGL_EXPORT
fuchsia_egl_window *fuchsia_egl_window_create(zx_handle_t image_pipe_handle,
                                              int32_t width,
                                              int32_t height);

FUCHSIA_EGL_EXPORT
void fuchsia_egl_window_destroy(fuchsia_egl_window *egl_window);

FUCHSIA_EGL_EXPORT
void fuchsia_egl_window_resize(fuchsia_egl_window *egl_window, int32_t width, int32_t height);

FUCHSIA_EGL_EXPORT
int32_t fuchsia_egl_window_get_width(fuchsia_egl_window *egl_window);

FUCHSIA_EGL_EXPORT
int32_t fuchsia_egl_window_get_height(fuchsia_egl_window *egl_window);

#ifdef __cplusplus
}
#endif

#endif  // FUCHSIA_EGL_H_
