//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// util_export.h : Defines ANGLE_UTIL_EXPORT, a macro for exporting symbols.

#ifndef UTIL_EXPORT_H_
#define UTIL_EXPORT_H_

// How to get the right import/export annotations on Windows:
// We define "IMPLEMENTATION" to get the dllexport label when compiling the
// util library. Consumers get the dllimport label by default.
#if !defined(ANGLE_UTIL_EXPORT)
#    if defined(_WIN32)
#        if defined(LIBANGLE_UTIL_IMPLEMENTATION)
#            define ANGLE_UTIL_EXPORT __declspec(dllexport)
#        else
#            define ANGLE_UTIL_EXPORT __declspec(dllimport)
#        endif
#    elif defined(__GNUC__)
#        define ANGLE_UTIL_EXPORT __attribute__((visibility("default")))
#    else
#        define ANGLE_UTIL_EXPORT
#    endif
#endif  // !defined(ANGLE_UTIL_EXPORT)

#endif  // UTIL_EXPORT_H_
