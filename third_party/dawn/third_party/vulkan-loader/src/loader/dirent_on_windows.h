#pragma once

/*

    Declaration of POSIX directory browsing functions and types for Win32.

    Author:  Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
    History: Created March 1997. Updated June 2003.
    Rights:  See end of file.

*/

#include <vulkan/vulkan.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct DIR DIR;

struct dirent {
    char *d_name;
};

// pass in VkAllocationCallbacks to allow allocation callback usage
DIR *opendir(const VkAllocationCallbacks *pAllocator, const char *);
int closedir(const VkAllocationCallbacks *pAllocator, DIR *);
struct dirent *readdir(DIR *);
void rewinddir(DIR *);

/*

    Copyright Kevlin Henney, 1997, 2003. All rights reserved.
    Copyright (c) 2015-2021 The Khronos Group Inc.
    Copyright (c) 2015-2021 Valve Corporation
    Copyright (c) 2015-2021 LunarG, Inc.

    Permission to use, copy, modify, and distribute this software and its
    documentation for any purpose is hereby granted without fee, provided
    that this copyright and permissions notice appear in all copies and
    derivatives.

    This software is supplied "as is" without express or implied warranty.

    But that said, if there are any problems please get in touch.

*/

#if defined(__cplusplus)
}
#endif
