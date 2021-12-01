/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/ashmem.h>
#include <utils/api_level.h>

#include <errno.h>
#include <assert.h>
#include <stdio.h>

#include <utils/Path.h>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#   include <fcntl.h>
#   include <stdlib.h>
#   include <unistd.h>
#endif

#if defined(WIN32)
#include <io.h>
#include <Windows.h>
#include <Winbase.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#endif


#ifdef __ANDROID__
#   include <sys/ioctl.h>
#   include <sys/stat.h>
#   include <memory.h>
#   include <dlfcn.h>
#   include <linux/ashmem.h>
#   include <android/api-level.h>
#   include <android/sharedmem.h>
#endif

namespace utils {

#ifdef __ANDROID__

#if __ANDROID_API__ >= 26

// Starting with API 26 (Oreo) we have ASharedMemory
int ashmem_create_region(const char *name, size_t size) {
    return ASharedMemory_create(name, size);
}

#else

static int __ashmem_open() {
    int ret;
    struct stat st;

    int fd = open("/dev/ashmem", O_RDWR);
    if (fd < 0) {
        return fd;
    }

    ret = fstat(fd, &st);
    if (ret < 0) {
        int save_errno = errno;
        close(fd);
        errno = save_errno;
        return ret;
    }
    if (!S_ISCHR(st.st_mode) || !st.st_rdev) {
        close(fd);
        errno = ENOTTY;
        return -1;
    }

    return fd;
}

int ashmem_create_region(const char *name, size_t size) {
    // Fetch the API level to avoid dlsym() on API 19
    if (api_level() >= 26) {
        // dynamically check if we have "ASharedMemory_create" (should be the case since 26 (Oreo))
        using TASharedMemory_create = int(*)(const char *name, size_t size);
        TASharedMemory_create pfnASharedMemory_create =
                (TASharedMemory_create)dlsym(RTLD_DEFAULT, "ASharedMemory_create");
        if (pfnASharedMemory_create) {
            return pfnASharedMemory_create(name, size);
        }
    }

    int ret, save_errno;
    int fd = __ashmem_open();
    if (fd < 0) {
        return fd;
    }

    if (name) {
        char buf[ASHMEM_NAME_LEN] = {0};

        strlcpy(buf, name, sizeof(buf));
        ret = ioctl(fd, ASHMEM_SET_NAME, buf);
        if (ret < 0) {
            goto error;
        }
    }

    ret = ioctl(fd, ASHMEM_SET_SIZE, size);
    if (ret < 0) {
        goto error;
    }

    return fd;

error:
    save_errno = errno;
    close(fd);
    errno = save_errno;
    return ret;
}

#endif // __ANDROID_API__ >= 26

#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))

int ashmem_create_region(const char*, size_t size) {
    char template_path[512];
    snprintf(template_path, sizeof(template_path), "%s/filament-ashmem-%d-XXXXXXXXX",
            Path::getTemporaryDirectory().c_str(), getpid());
    int fd = mkstemp(template_path);
    if (fd == -1) return -1;
    unlink(template_path);
    if (ftruncate(fd, (off_t)size) == -1) {
        close(fd);
        return -1;
    }
    return fd;
}

#else
int ashmem_create_region(const char*, size_t size) {
    char template_path[512];
    snprintf(template_path, sizeof(template_path), "%s/filament-ashmem-%lu-XXXXXXXXX",
            Path::getTemporaryDirectory().c_str(), GetCurrentProcessId());
    const char* tmpPath = _mktemp(template_path);
    int fd = _open(tmpPath, _O_BINARY);
    if (fd == -1) return -1;
    unlink(template_path);
    if (_chsize(fd, size) == -1) {
        _close(fd);
        return -1;
    }
    return fd;
}
#endif

} // namespace utils
