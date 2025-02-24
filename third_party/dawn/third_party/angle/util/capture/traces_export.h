//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// restricted_traces_export: Export definitions for restricted traces.

#ifndef ANGLE_TRACES_EXPORT_H_
#define ANGLE_TRACES_EXPORT_H_

// for KHRONOS_APIENTRY
#include <KHR/khrplatform.h>

// See util/util_export.h for details on import/export labels.
#if !defined(ANGLE_TRACE_EXPORT)
#    if defined(_WIN32)
#        if defined(ANGLE_TRACE_IMPLEMENTATION)
#            define ANGLE_TRACE_EXPORT __declspec(dllexport)
#        else
#            define ANGLE_TRACE_EXPORT __declspec(dllimport)
#        endif
#    elif defined(__GNUC__)
#        define ANGLE_TRACE_EXPORT __attribute__((visibility("default")))
#    else
#        define ANGLE_TRACE_EXPORT
#    endif
#endif  // !defined(ANGLE_TRACE_EXPORT)

#if !defined(ANGLE_TRACE_LOADER_EXPORT)
#    if defined(_WIN32)
#        if defined(ANGLE_TRACE_LOADER_IMPLEMENTATION)
#            define ANGLE_TRACE_LOADER_EXPORT __declspec(dllexport)
#        else
#            define ANGLE_TRACE_LOADER_EXPORT __declspec(dllimport)
#        endif
#    elif defined(__GNUC__)
#        define ANGLE_TRACE_LOADER_EXPORT __attribute__((visibility("default")))
#    else
#        define ANGLE_TRACE_LOADER_EXPORT
#    endif
#endif  // !defined(ANGLE_TRACE_LOADER_EXPORT)

#if !defined(ANGLE_REPLAY_EXPORT)
#    if defined(_WIN32)
#        if defined(ANGLE_REPLAY_IMPLEMENTATION)
#            define ANGLE_REPLAY_EXPORT __declspec(dllexport)
#        else
#            define ANGLE_REPLAY_EXPORT __declspec(dllimport)
#        endif
#    elif defined(__GNUC__)
#        define ANGLE_REPLAY_EXPORT __attribute__((visibility("default")))
#    else
#        define ANGLE_REPLAY_EXPORT
#    endif
#endif  // !defined(ANGLE_REPLAY_EXPORT)

typedef void (*GenericProc)(void);
typedef GenericProc(KHRONOS_APIENTRY *LoadProc)(const char *);

#if defined(__cplusplus)
extern "C" {
#endif  // defined(__cplusplus)
ANGLE_TRACE_LOADER_EXPORT void LoadTraceEGL(LoadProc loadProc);
ANGLE_TRACE_LOADER_EXPORT void LoadTraceGLES(LoadProc loadProc);
#if defined(__cplusplus)
}  // extern "C"
#endif  // defined(__cplusplus)

#endif  // ANGLE_TRACES_EXPORT_H_
