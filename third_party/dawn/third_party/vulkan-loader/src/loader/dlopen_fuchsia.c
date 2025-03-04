/*
 *
 * Copyright (c) 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "dlopen_fuchsia.h"

#include <fcntl.h>
#include <fuchsia/vulkan/loader/c/fidl.h>
#include <lib/fdio/io.h>
#include <lib/fdio/directory.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>
#include <zircon/dlfcn.h>
#include <zircon/syscalls.h>

static char g_error[128] = {};

const char *dlerror_fuchsia(void) { return g_error; }

static zx_handle_t vulkan_loader_svc = ZX_HANDLE_INVALID;
void connect_to_vulkan_loader_svc(void) {
    zx_handle_t svc1, svc2;
    if (zx_channel_create(0, &svc1, &svc2) != ZX_OK) return;

    if (fdio_service_connect("/svc/" fuchsia_vulkan_loader_Loader_Name, svc1) != ZX_OK) {
        zx_handle_close(svc2);
        return;
    }

    vulkan_loader_svc = svc2;
}

static once_flag svc_connect_once_flag = ONCE_FLAG_INIT;

void *dlopen_fuchsia(const char *name, int mode, bool driver) {
    // First try to just dlopen() from our own namespace. This will succeed for
    // any layers that are packaged with the application, but will fail for
    // client drivers loaded from the system.
    void *result;
    if (!driver) {
        result = dlopen(name, mode);
        if (result != NULL) return result;
    }

    // If we couldn't find the library in our own namespace, connect to the
    // loader service to request this library.
    call_once(&svc_connect_once_flag, connect_to_vulkan_loader_svc);

    if (vulkan_loader_svc == ZX_HANDLE_INVALID) {
        snprintf(g_error, sizeof(g_error), "libvulkan.so:dlopen_fuchsia: no connection to loader svc\n");
        return NULL;
    }

    zx_handle_t vmo = ZX_HANDLE_INVALID;
    zx_status_t st = fuchsia_vulkan_loader_LoaderGet(vulkan_loader_svc, name, strlen(name), &vmo);
    if (st != ZX_OK) {
        snprintf(g_error, sizeof(g_error), "libvulkan.so:dlopen_fuchsia: Get() failed: %d\n", st);
        return NULL;
    }

    if (vmo == ZX_HANDLE_INVALID) {
        snprintf(g_error, sizeof(g_error), "libvulkan.so:dlopen_fuchsia: Get() returned invalid vmo\n");
        return NULL;
    }

    result = dlopen_vmo(vmo, mode);
    zx_handle_close(vmo);
    if (!result) {
        snprintf(g_error, sizeof(g_error), "%s", dlerror());
    }
    return result;
}
