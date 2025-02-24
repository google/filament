//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SoftLinking.h: Macros for soft-linking Frameworks and Functions.

#ifndef SOFT_LINKING_APPLE_H_
#define SOFT_LINKING_APPLE_H_

#include "common/platform.h"

#if defined(ANGLE_PLATFORM_APPLE)

#    include "common/debug.h"

#    import <dispatch/dispatch.h>
#    import <dlfcn.h>
#    import <objc/runtime.h>

#    define RELEASE_ASSERT(expression, message)                                               \
        (expression                                                                           \
             ? static_cast<void>(0)                                                           \
             : (FATAL() << "\t! Assert failed in " << __FUNCTION__ << " (" << __FILE__ << ":" \
                        << __LINE__ << "): " << #expression << "\n\t! Message: " << message))

#    ifdef __cplusplus
#        define EXTERN_C_BEGIN extern "C" {
#        define EXTERN_C_END }
#    else
#        define EXTERN_C_BEGIN
#        define EXTERN_C_END
#    endif

#    define SOFT_LINK_FRAMEWORK_HEADER(framework) extern void *framework##Library();

#    define SOFT_LINK_FRAMEWORK_SOURCE(framework)                                               \
        void *framework##Library()                                                              \
        {                                                                                       \
            static dispatch_once_t once   = 0;                                                  \
            static void *frameworkLibrary = NULL;                                               \
            dispatch_once(&once, ^{                                                             \
              frameworkLibrary = dlopen(                                                        \
                  "/System/Library/Frameworks/" #framework ".framework/" #framework, RTLD_NOW); \
              RELEASE_ASSERT(frameworkLibrary, "Unable to load " #framework ".framework");      \
            });                                                                                 \
            return frameworkLibrary;                                                            \
        }

#    define SOFT_LINK_FUNCTION_HEADER(framework, functionName, resultType, parameterDeclarations, \
                                      parameterNames)                                             \
        EXTERN_C_BEGIN                                                                            \
        resultType functionName parameterDeclarations;                                            \
        EXTERN_C_END                                                                              \
        extern resultType init##framework##functionName parameterDeclarations;                    \
        extern resultType(*softLink##framework##functionName) parameterDeclarations;              \
        inline __attribute__((__always_inline__)) resultType functionName parameterDeclarations   \
        {                                                                                         \
            return softLink##framework##functionName parameterNames;                              \
        }

#    define SOFT_LINK_FUNCTION_SOURCE(framework, functionName, resultType, parameterDeclarations,  \
                                      parameterNames)                                              \
        resultType(*softLink##framework##functionName) parameterDeclarations =                     \
            init##framework##functionName;                                                         \
        resultType init##framework##functionName parameterDeclarations                             \
        {                                                                                          \
            static dispatch_once_t once;                                                           \
            dispatch_once(&once, ^{                                                                \
              softLink##framework##functionName =                                                  \
                  (resultType(*) parameterDeclarations)dlsym(framework##Library(), #functionName); \
            });                                                                                    \
            return softLink##framework##functionName parameterNames;                               \
        }

#    define SOFT_LINK_CLASS_HEADER(className)    \
        @class className;                        \
        extern Class (*get##className##Class)(); \
        className *alloc##className##Instance(); \
        inline className *alloc##className##Instance() { return [get##className##Class() alloc]; }

#    define SOFT_LINK_CLASS(framework, className)                                       \
        @class className;                                                               \
        static Class init##className();                                                 \
        Class (*get##className##Class)() = init##className;                             \
        static Class class##className;                                                  \
                                                                                        \
        static Class className##Function() { return class##className; }                 \
                                                                                        \
        static Class init##className()                                                  \
        {                                                                               \
            static dispatch_once_t once;                                                \
            dispatch_once(&once, ^{                                                     \
              framework##Library();                                                     \
              class##className = objc_getClass(#className);                             \
              RELEASE_ASSERT(class##className, "objc_getClass failed for " #className); \
              get##className##Class = className##Function;                              \
            });                                                                         \
            return class##className;                                                    \
        }                                                                               \
        _Pragma("clang diagnostic push")                                                \
            _Pragma("clang diagnostic ignored \"-Wunused-function\"") static className  \
                *alloc##className##Instance()                                           \
        {                                                                               \
            return [get##className##Class() alloc];                                     \
        }                                                                               \
        _Pragma("clang diagnostic pop")

#endif  // defined(ANGLE_PLATFORM_APPLE)

#endif  // SOFT_LINKING_APPLE_H_
