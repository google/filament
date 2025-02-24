//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// shader_cache_file_hooking:
//    Hooks file API functions to intercept Metal shader cache access and return as if the file
//    doesn't exist. This is to avoid slow cache access happened after the cache becomes huge over
//    time.
//

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <regex>

namespace
{
constexpr char kMetalCacheExpr[] = R"(.*com\.apple\.metal.*(libraries|functions).*\.(maps|data))";
}

int HookedOpen(const char *path, int flags, mode_t mode)
{
    std::regex expr(kMetalCacheExpr);

    if (!std::regex_match(path, expr))
    {
        return open(path, flags, mode);
    }

#if !defined(NDEBUG)
    static int messageRepeatCount = 5;
    if (messageRepeatCount > 0)
    {
        messageRepeatCount--;
        std::cerr << "open(\"" << path << "\", \"" << flags
                  << "\") is skipped. This message will repeat " << messageRepeatCount
                  << " more times." << std::endl;
    }
#endif

    if (flags & O_CREAT)
    {
        errno = EACCES;
        return -1;
    }

    // Treat as if the cache doesn't exist
    errno = ENOENT;
    return -1;
}

FILE *HookedFopen(const char *filename, const char *mode)
{
    std::regex expr(kMetalCacheExpr);
    if (!std::regex_match(filename, expr))
    {
        return fopen(filename, mode);
    }

#if !defined(NDEBUG)
    static int messageRepeatCount = 5;
    if (messageRepeatCount > 0)
    {
        messageRepeatCount--;
        std::cerr << "fopen(\"" << filename << "\", \"" << mode
                  << "\") is skipped. This message will repeat " << messageRepeatCount
                  << " more times." << std::endl;
    }
#endif

    if (strstr(mode, "r"))
    {
        // Treat as if the cache doesn't exist
        errno = ENOENT;
        return nullptr;
    }

    errno = EACCES;
    return nullptr;
}

// See https://opensource.apple.com/source/dyld/dyld-210.2.3/include/mach-o/dyld-interposing.h
#define DYLD_INTERPOSE(_replacment, _replacee)                                  \
    __attribute__((used)) static struct                                         \
    {                                                                           \
        const void *replacment;                                                 \
        const void *replacee;                                                   \
    } _interpose_##_replacee __attribute__((section("__DATA,__interpose"))) = { \
        (const void *)(unsigned long)&_replacment, (const void *)(unsigned long)&_replacee};

DYLD_INTERPOSE(HookedOpen, open)
DYLD_INTERPOSE(HookedFopen, fopen)
