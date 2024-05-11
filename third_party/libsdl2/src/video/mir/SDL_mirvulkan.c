/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/*
 * @author Mark Callow, www.edgewise-consulting.com. Based on Jacob Lifshay's
 * SDL_x11vulkan.c.
 */

#include "../../SDL_internal.h"

#if SDL_VIDEO_VULKAN && SDL_VIDEO_DRIVER_MIR

#include "SDL_mirvideo.h"
#include "SDL_mirwindow.h"
#include "SDL_assert.h"

#include "SDL_loadso.h"
#include "SDL_mirvulkan.h"
#include "SDL_syswm.h"

int MIR_Vulkan_LoadLibrary(_THIS, const char *path)
{
    VkExtensionProperties *extensions = NULL;
    Uint32 extensionCount = 0;
    SDL_bool hasSurfaceExtension = SDL_FALSE;
    SDL_bool hasMIRSurfaceExtension = SDL_FALSE;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = NULL;
    if(_this->vulkan_config.loader_handle)
        return SDL_SetError("Vulkan already loaded");

    /* Load the Vulkan loader library */
    if(!path)
        path = SDL_getenv("SDL_VULKAN_LIBRARY");
    if(!path)
        path = "libvulkan.so.1";
    _this->vulkan_config.loader_handle = SDL_LoadObject(path);
    if(!_this->vulkan_config.loader_handle)
        return -1;
    SDL_strlcpy(_this->vulkan_config.loader_path, path,
                SDL_arraysize(_this->vulkan_config.loader_path));
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_LoadFunction(
        _this->vulkan_config.loader_handle, "vkGetInstanceProcAddr");
    if(!vkGetInstanceProcAddr)
        goto fail;
    _this->vulkan_config.vkGetInstanceProcAddr = (void *)vkGetInstanceProcAddr;
    _this->vulkan_config.vkEnumerateInstanceExtensionProperties =
        (void *)((PFN_vkGetInstanceProcAddr)_this->vulkan_config.vkGetInstanceProcAddr)(
            VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties");
    if(!_this->vulkan_config.vkEnumerateInstanceExtensionProperties)
        goto fail;
    extensions = SDL_Vulkan_CreateInstanceExtensionsList(
        (PFN_vkEnumerateInstanceExtensionProperties)
            _this->vulkan_config.vkEnumerateInstanceExtensionProperties,
        &extensionCount);
    if(!extensions)
        goto fail;
    for(Uint32 i = 0; i < extensionCount; i++)
    {
        if(SDL_strcmp(VK_KHR_SURFACE_EXTENSION_NAME, extensions[i].extensionName) == 0)
            hasSurfaceExtension = SDL_TRUE;
        else if(SDL_strcmp(VK_KHR_MIR_SURFACE_EXTENSION_NAME, extensions[i].extensionName) == 0)
            hasMIRSurfaceExtension = SDL_TRUE;
    }
    SDL_free(extensions);
    if(!hasSurfaceExtension)
    {
        SDL_SetError("Installed Vulkan doesn't implement the "
                     VK_KHR_SURFACE_EXTENSION_NAME " extension");
        goto fail;
    }
    else if(!hasMIRSurfaceExtension)
    {
        SDL_SetError("Installed Vulkan doesn't implement the "
                     VK_KHR_MIR_SURFACE_EXTENSION_NAME "extension");
        goto fail;
    }
    return 0;

fail:
    SDL_UnloadObject(_this->vulkan_config.loader_handle);
    _this->vulkan_config.loader_handle = NULL;
    return -1;
}

void MIR_Vulkan_UnloadLibrary(_THIS)
{
    if(_this->vulkan_config.loader_handle)
    {
        SDL_UnloadObject(_this->vulkan_config.loader_handle);
        _this->vulkan_config.loader_handle = NULL;
    }
}

SDL_bool MIR_Vulkan_GetInstanceExtensions(_THIS,
                                          SDL_Window *window,
                                          unsigned *count,
                                          const char **names)
{
    static const char *const extensionsForMir[] = {
        VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_MIR_SURFACE_EXTENSION_NAME
    };
    if(!_this->vulkan_config.loader_handle)
    {
        SDL_SetError("Vulkan is not loaded");
        return SDL_FALSE;
    }
    return SDL_Vulkan_GetInstanceExtensions_Helper(
            count, names, SDL_arraysize(extensionsForMir),
            extensionsForMir);
}

SDL_bool MIR_Vulkan_CreateSurface(_THIS,
                                  SDL_Window *window,
                                  VkInstance instance,
                                  VkSurfaceKHR *surface)
{
    MIR_Window *windowData = (MIR_Window *)window->driverdata;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        (PFN_vkGetInstanceProcAddr)_this->vulkan_config.vkGetInstanceProcAddr;
    PFN_vkCreateMirSurfaceKHR vkCreateMirSurfaceKHR =
        (PFN_vkCreateMirSurfaceKHR)vkGetInstanceProcAddr(
                                            (VkInstance)instance,
                                            "vkCreateMirSurfaceKHR");
    VkMirSurfaceCreateInfoKHR createInfo;
    VkResult result;

    if(!_this->vulkan_config.loader_handle)
    {
        SDL_SetError("Vulkan is not loaded");
        return SDL_FALSE;
    }

    if(!vkCreateMirSurfaceKHR)
    {
        SDL_SetError(VK_KHR_MIR_SURFACE_EXTENSION_NAME
                     " extension is not enabled in the Vulkan instance.");
        return SDL_FALSE;
    }
    SDL_zero(createInfo);
    createInfo.sType = VK_STRUCTURE_TYPE_MIR_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.connection = windowData->mir_data->connection;
    createInfo.mirSurface = windowData->window;
    result = vkCreateMirSurfaceKHR(instance, &createInfo,
                                       NULL, surface);
    if(result != VK_SUCCESS)
    {
        SDL_SetError("vkCreateMirSurfaceKHR failed: %s",
                     SDL_Vulkan_GetResultString(result));
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

#endif

/* vim: set ts=4 sw=4 expandtab: */
