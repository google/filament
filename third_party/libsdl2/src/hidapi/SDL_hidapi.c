/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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

/* Original hybrid wrapper for Linux by Valve Software. Their original notes:
 *
 * The libusb version doesn't support Bluetooth, but not all Linux
 * distributions allow access to /dev/hidraw*
 *
 * This merges the two, at a small performance cost, until distributions
 * have granted access to /dev/hidraw*
 */
#include "../SDL_internal.h"

#include "SDL_loadso.h"
#include "SDL_hidapi.h"

#ifdef SDL_JOYSTICK_HIDAPI

/* Platform HIDAPI Implementation */

#define hid_device_                     PLATFORM_hid_device_
#define hid_device                      PLATFORM_hid_device
#define hid_device_info                 PLATFORM_hid_device_info
#define hid_init                        PLATFORM_hid_init
#define hid_exit                        PLATFORM_hid_exit
#define hid_enumerate                   PLATFORM_hid_enumerate
#define hid_free_enumeration            PLATFORM_hid_free_enumeration
#define hid_open                        PLATFORM_hid_open
#define hid_open_path                   PLATFORM_hid_open_path
#define hid_write                       PLATFORM_hid_write
#define hid_read_timeout                PLATFORM_hid_read_timeout
#define hid_read                        PLATFORM_hid_read
#define hid_set_nonblocking             PLATFORM_hid_set_nonblocking
#define hid_send_feature_report         PLATFORM_hid_send_feature_report
#define hid_get_feature_report          PLATFORM_hid_get_feature_report
#define hid_close                       PLATFORM_hid_close
#define hid_get_manufacturer_string     PLATFORM_hid_get_manufacturer_string
#define hid_get_product_string          PLATFORM_hid_get_product_string
#define hid_get_serial_number_string    PLATFORM_hid_get_serial_number_string
#define hid_get_indexed_string          PLATFORM_hid_get_indexed_string
#define hid_error                       PLATFORM_hid_error
#define new_hid_device                  PLATFORM_new_hid_device
#define free_hid_device                 PLATFORM_free_hid_device
#define input_report                    PLATFORM_input_report
#define return_data                     PLATFORM_return_data
#define make_path                       PLATFORM_make_path
#define read_thread                     PLATFORM_read_thread

#undef HIDAPI_H__
#if __LINUX__

#include "../../core/linux/SDL_udev.h"
#if SDL_USE_LIBUDEV
static const SDL_UDEV_Symbols *udev_ctx = NULL;

#define udev_device_get_sysattr_value                    udev_ctx->udev_device_get_sysattr_value
#define udev_new                                         udev_ctx->udev_new
#define udev_unref                                       udev_ctx->udev_unref
#define udev_device_new_from_devnum                      udev_ctx->udev_device_new_from_devnum
#define udev_device_get_parent_with_subsystem_devtype    udev_ctx->udev_device_get_parent_with_subsystem_devtype
#define udev_device_unref                                udev_ctx->udev_device_unref
#define udev_enumerate_new                               udev_ctx->udev_enumerate_new
#define udev_enumerate_add_match_subsystem               udev_ctx->udev_enumerate_add_match_subsystem
#define udev_enumerate_scan_devices                      udev_ctx->udev_enumerate_scan_devices
#define udev_enumerate_get_list_entry                    udev_ctx->udev_enumerate_get_list_entry
#define udev_list_entry_get_name                         udev_ctx->udev_list_entry_get_name
#define udev_device_new_from_syspath                     udev_ctx->udev_device_new_from_syspath
#define udev_device_get_devnode                          udev_ctx->udev_device_get_devnode
#define udev_list_entry_get_next                         udev_ctx->udev_list_entry_get_next
#define udev_enumerate_unref                             udev_ctx->udev_enumerate_unref

#include "linux/hid.c"
#define HAVE_PLATFORM_BACKEND 1
#endif /* SDL_USE_LIBUDEV */

#elif __MACOSX__
#include "mac/hid.c"
#define HAVE_PLATFORM_BACKEND 1
#define udev_ctx 1
#elif __WINDOWS__
#include "windows/hid.c"
#define HAVE_PLATFORM_BACKEND 1
#define udev_ctx 1
#endif

#undef hid_device_
#undef hid_device
#undef hid_device_info
#undef hid_init
#undef hid_exit
#undef hid_enumerate
#undef hid_free_enumeration
#undef hid_open
#undef hid_open_path
#undef hid_write
#undef hid_read_timeout
#undef hid_read
#undef hid_set_nonblocking
#undef hid_send_feature_report
#undef hid_get_feature_report
#undef hid_close
#undef hid_get_manufacturer_string
#undef hid_get_product_string
#undef hid_get_serial_number_string
#undef hid_get_indexed_string
#undef hid_error
#undef new_hid_device
#undef free_hid_device
#undef input_report
#undef return_data
#undef make_path
#undef read_thread

#ifdef SDL_JOYSTICK_HIDAPI_STEAMXBOX
#define HAVE_DRIVER_BACKEND 1
#endif

#ifdef HAVE_DRIVER_BACKEND

/* DRIVER HIDAPI Implementation */

#define hid_device_                     DRIVER_hid_device_
#define hid_device                      DRIVER_hid_device
#define hid_device_info                 DRIVER_hid_device_info
#define hid_init                        DRIVER_hid_init
#define hid_exit                        DRIVER_hid_exit
#define hid_enumerate                   DRIVER_hid_enumerate
#define hid_free_enumeration            DRIVER_hid_free_enumeration
#define hid_open                        DRIVER_hid_open
#define hid_open_path                   DRIVER_hid_open_path
#define hid_write                       DRIVER_hid_write
#define hid_read_timeout                DRIVER_hid_read_timeout
#define hid_read                        DRIVER_hid_read
#define hid_set_nonblocking             DRIVER_hid_set_nonblocking
#define hid_send_feature_report         DRIVER_hid_send_feature_report
#define hid_get_feature_report          DRIVER_hid_get_feature_report
#define hid_close                       DRIVER_hid_close
#define hid_get_manufacturer_string     DRIVER_hid_get_manufacturer_string
#define hid_get_product_string          DRIVER_hid_get_product_string
#define hid_get_serial_number_string    DRIVER_hid_get_serial_number_string
#define hid_get_indexed_string          DRIVER_hid_get_indexed_string
#define hid_error                       DRIVER_hid_error

#ifdef SDL_JOYSTICK_HIDAPI_STEAMXBOX
#undef HIDAPI_H__
#include "steamxbox/hid.c"
#else
#error Need a driver hid.c for this platform!
#endif

#undef hid_device_
#undef hid_device
#undef hid_device_info
#undef hid_init
#undef hid_exit
#undef hid_enumerate
#undef hid_free_enumeration
#undef hid_open
#undef hid_open_path
#undef hid_write
#undef hid_read_timeout
#undef hid_read
#undef hid_set_nonblocking
#undef hid_send_feature_report
#undef hid_get_feature_report
#undef hid_close
#undef hid_get_manufacturer_string
#undef hid_get_product_string
#undef hid_get_serial_number_string
#undef hid_get_indexed_string
#undef hid_error

#endif /* HAVE_DRIVER_BACKEND */


#ifdef SDL_LIBUSB_DYNAMIC
/* libusb HIDAPI Implementation */

/* Include this now, for our dynamically-loaded libusb context */
#include <libusb.h>

static struct
{
    void* libhandle;

    int (*init)(libusb_context **ctx);
    void (*exit)(libusb_context *ctx);
    ssize_t (*get_device_list)(libusb_context *ctx, libusb_device ***list);
    void (*free_device_list)(libusb_device **list, int unref_devices);
    int (*get_device_descriptor)(libusb_device *dev, struct libusb_device_descriptor *desc);
    int (*get_active_config_descriptor)(libusb_device *dev,    struct libusb_config_descriptor **config);
    int (*get_config_descriptor)(
        libusb_device *dev,
        uint8_t config_index,
        struct libusb_config_descriptor **config
    );
    void (*free_config_descriptor)(struct libusb_config_descriptor *config);
    uint8_t (*get_bus_number)(libusb_device *dev);
    uint8_t (*get_device_address)(libusb_device *dev);
    int (*open)(libusb_device *dev, libusb_device_handle **dev_handle);
    void (*close)(libusb_device_handle *dev_handle);
    int (*claim_interface)(libusb_device_handle *dev_handle, int interface_number);
    int (*release_interface)(libusb_device_handle *dev_handle, int interface_number);
    int (*kernel_driver_active)(libusb_device_handle *dev_handle, int interface_number);
    int (*detach_kernel_driver)(libusb_device_handle *dev_handle, int interface_number);
    int (*attach_kernel_driver)(libusb_device_handle *dev_handle, int interface_number);
    int (*set_interface_alt_setting)(libusb_device_handle *dev, int interface_number, int alternate_setting);
    struct libusb_transfer * (*alloc_transfer)(int iso_packets);
    int (*submit_transfer)(struct libusb_transfer *transfer);
    int (*cancel_transfer)(struct libusb_transfer *transfer);
    void (*free_transfer)(struct libusb_transfer *transfer);
    int (*control_transfer)(
        libusb_device_handle *dev_handle,
        uint8_t request_type,
        uint8_t bRequest,
        uint16_t wValue,
        uint16_t wIndex,
        unsigned char *data,
        uint16_t wLength,
        unsigned int timeout
    );
    int (*interrupt_transfer)(
        libusb_device_handle *dev_handle,
        unsigned char endpoint,
        unsigned char *data,
        int length,
        int *actual_length,
        unsigned int timeout
    );
    int (*handle_events)(libusb_context *ctx);
    int (*handle_events_completed)(libusb_context *ctx, int *completed);
} libusb_ctx;

#define libusb_init                            libusb_ctx.init
#define libusb_exit                            libusb_ctx.exit
#define libusb_get_device_list                 libusb_ctx.get_device_list
#define libusb_free_device_list                libusb_ctx.free_device_list
#define libusb_get_device_descriptor           libusb_ctx.get_device_descriptor
#define libusb_get_active_config_descriptor    libusb_ctx.get_active_config_descriptor
#define libusb_get_config_descriptor           libusb_ctx.get_config_descriptor
#define libusb_free_config_descriptor          libusb_ctx.free_config_descriptor
#define libusb_get_bus_number                  libusb_ctx.get_bus_number
#define libusb_get_device_address              libusb_ctx.get_device_address
#define libusb_open                            libusb_ctx.open
#define libusb_close                           libusb_ctx.close
#define libusb_claim_interface                 libusb_ctx.claim_interface
#define libusb_release_interface               libusb_ctx.release_interface
#define libusb_kernel_driver_active            libusb_ctx.kernel_driver_active
#define libusb_detach_kernel_driver            libusb_ctx.detach_kernel_driver
#define libusb_attach_kernel_driver            libusb_ctx.attach_kernel_driver
#define libusb_set_interface_alt_setting       libusb_ctx.set_interface_alt_setting
#define libusb_alloc_transfer                  libusb_ctx.alloc_transfer
#define libusb_submit_transfer                 libusb_ctx.submit_transfer
#define libusb_cancel_transfer                 libusb_ctx.cancel_transfer
#define libusb_free_transfer                   libusb_ctx.free_transfer
#define libusb_control_transfer                libusb_ctx.control_transfer
#define libusb_interrupt_transfer              libusb_ctx.interrupt_transfer
#define libusb_handle_events                   libusb_ctx.handle_events
#define libusb_handle_events_completed         libusb_ctx.handle_events_completed

#define hid_device_                     LIBUSB_hid_device_
#define hid_device                      LIBUSB_hid_device
#define hid_device_info                 LIBUSB_hid_device_info
#define hid_init                        LIBUSB_hid_init
#define hid_exit                        LIBUSB_hid_exit
#define hid_enumerate                   LIBUSB_hid_enumerate
#define hid_free_enumeration            LIBUSB_hid_free_enumeration
#define hid_open                        LIBUSB_hid_open
#define hid_open_path                   LIBUSB_hid_open_path
#define hid_write                       LIBUSB_hid_write
#define hid_read_timeout                LIBUSB_hid_read_timeout
#define hid_read                        LIBUSB_hid_read
#define hid_set_nonblocking             LIBUSB_hid_set_nonblocking
#define hid_send_feature_report         LIBUSB_hid_send_feature_report
#define hid_get_feature_report          LIBUSB_hid_get_feature_report
#define hid_close                       LIBUSB_hid_close
#define hid_get_manufacturer_string     LIBUSB_hid_get_manufacturer_string
#define hid_get_product_string          LIBUSB_hid_get_product_string
#define hid_get_serial_number_string    LIBUSB_hid_get_serial_number_string
#define hid_get_indexed_string          LIBUSB_hid_get_indexed_string
#define hid_error                       LIBUSB_hid_error
#define new_hid_device                  LIBUSB_new_hid_device
#define free_hid_device                 LIBUSB_free_hid_device
#define input_report                    LIBUSB_input_report
#define return_data                     LIBUSB_return_data
#define make_path                       LIBUSB_make_path
#define read_thread                     LIBUSB_read_thread

#ifndef __FreeBSD__
/* this is awkwardly inlined, so we need to re-implement it here
 * so we can override the libusb_control_transfer call */
static int
SDL_libusb_get_string_descriptor(libusb_device_handle *dev,
                                 uint8_t descriptor_index, uint16_t lang_id,
                                 unsigned char *data, int length)
{
    return libusb_control_transfer(dev,
                                   LIBUSB_ENDPOINT_IN | 0x0, /* Endpoint 0 IN */
                                   LIBUSB_REQUEST_GET_DESCRIPTOR,
                                   (LIBUSB_DT_STRING << 8) | descriptor_index,
                                   lang_id,
                                   data,
                                   (uint16_t) length,
                                   1000);
}
#define libusb_get_string_descriptor SDL_libusb_get_string_descriptor
#endif /* __FreeBSD__ */

#undef HIDAPI_H__
#include "libusb/hid.c"

#undef hid_device_
#undef hid_device
#undef hid_device_info
#undef hid_init
#undef hid_exit
#undef hid_enumerate
#undef hid_free_enumeration
#undef hid_open
#undef hid_open_path
#undef hid_write
#undef hid_read_timeout
#undef hid_read
#undef hid_set_nonblocking
#undef hid_send_feature_report
#undef hid_get_feature_report
#undef hid_close
#undef hid_get_manufacturer_string
#undef hid_get_product_string
#undef hid_get_serial_number_string
#undef hid_get_indexed_string
#undef hid_error
#undef new_hid_device
#undef free_hid_device
#undef input_report
#undef return_data
#undef make_path
#undef read_thread

#endif /* SDL_LIBUSB_DYNAMIC */

/* Shared HIDAPI Implementation */

#undef HIDAPI_H__
#include "hidapi/hidapi.h"

struct hidapi_backend {
    int  (*hid_write)(hid_device* device, const unsigned char* data, size_t length);
    int  (*hid_read_timeout)(hid_device* device, unsigned char* data, size_t length, int milliseconds);
    int  (*hid_read)(hid_device* device, unsigned char* data, size_t length);
    int  (*hid_set_nonblocking)(hid_device* device, int nonblock);
    int  (*hid_send_feature_report)(hid_device* device, const unsigned char* data, size_t length);
    int  (*hid_get_feature_report)(hid_device* device, unsigned char* data, size_t length);
    void (*hid_close)(hid_device* device);
    int  (*hid_get_manufacturer_string)(hid_device* device, wchar_t* string, size_t maxlen);
    int  (*hid_get_product_string)(hid_device* device, wchar_t* string, size_t maxlen);
    int  (*hid_get_serial_number_string)(hid_device* device, wchar_t* string, size_t maxlen);
    int  (*hid_get_indexed_string)(hid_device* device, int string_index, wchar_t* string, size_t maxlen);
    const wchar_t* (*hid_error)(hid_device* device);
};

#if HAVE_PLATFORM_BACKEND
static const struct hidapi_backend PLATFORM_Backend = {
    (void*)PLATFORM_hid_write,
    (void*)PLATFORM_hid_read_timeout,
    (void*)PLATFORM_hid_read,
    (void*)PLATFORM_hid_set_nonblocking,
    (void*)PLATFORM_hid_send_feature_report,
    (void*)PLATFORM_hid_get_feature_report,
    (void*)PLATFORM_hid_close,
    (void*)PLATFORM_hid_get_manufacturer_string,
    (void*)PLATFORM_hid_get_product_string,
    (void*)PLATFORM_hid_get_serial_number_string,
    (void*)PLATFORM_hid_get_indexed_string,
    (void*)PLATFORM_hid_error
};
#endif /* HAVE_PLATFORM_BACKEND */

#if HAVE_DRIVER_BACKEND
static const struct hidapi_backend DRIVER_Backend = {
    (void*)DRIVER_hid_write,
    (void*)DRIVER_hid_read_timeout,
    (void*)DRIVER_hid_read,
    (void*)DRIVER_hid_set_nonblocking,
    (void*)DRIVER_hid_send_feature_report,
    (void*)DRIVER_hid_get_feature_report,
    (void*)DRIVER_hid_close,
    (void*)DRIVER_hid_get_manufacturer_string,
    (void*)DRIVER_hid_get_product_string,
    (void*)DRIVER_hid_get_serial_number_string,
    (void*)DRIVER_hid_get_indexed_string,
    (void*)DRIVER_hid_error
};
#endif /* HAVE_DRIVER_BACKEND */

#ifdef SDL_LIBUSB_DYNAMIC
static const struct hidapi_backend LIBUSB_Backend = {
    (void*)LIBUSB_hid_write,
    (void*)LIBUSB_hid_read_timeout,
    (void*)LIBUSB_hid_read,
    (void*)LIBUSB_hid_set_nonblocking,
    (void*)LIBUSB_hid_send_feature_report,
    (void*)LIBUSB_hid_get_feature_report,
    (void*)LIBUSB_hid_close,
    (void*)LIBUSB_hid_get_manufacturer_string,
    (void*)LIBUSB_hid_get_product_string,
    (void*)LIBUSB_hid_get_serial_number_string,
    (void*)LIBUSB_hid_get_indexed_string,
    (void*)LIBUSB_hid_error
};
#endif /* SDL_LIBUSB_DYNAMIC */

typedef struct _HIDDeviceWrapper HIDDeviceWrapper;
struct _HIDDeviceWrapper
{
    hid_device *device; /* must be first field */
    const struct hidapi_backend *backend;
};

#if HAVE_PLATFORM_BACKEND || HAVE_DRIVER_BACKEND || defined(SDL_LIBUSB_DYNAMIC)

static HIDDeviceWrapper *
CreateHIDDeviceWrapper(hid_device *device, const struct hidapi_backend *backend)
{
    HIDDeviceWrapper *ret = (HIDDeviceWrapper *)SDL_malloc(sizeof(*ret));
    ret->device = device;
    ret->backend = backend;
    return ret;
}

static hid_device *
WrapHIDDevice(HIDDeviceWrapper *wrapper)
{
    return (hid_device *)wrapper;
}

#endif /* HAVE_PLATFORM_BACKEND || HAVE_DRIVER_BACKEND || SDL_LIBUSB_DYNAMIC */

static HIDDeviceWrapper *
UnwrapHIDDevice(hid_device *device)
{
    return (HIDDeviceWrapper *)device;
}

static void
DeleteHIDDeviceWrapper(HIDDeviceWrapper *device)
{
    SDL_free(device);
}

#define COPY_IF_EXISTS(var) \
    if (pSrc->var != NULL) { \
        pDst->var = SDL_strdup(pSrc->var); \
    } else { \
        pDst->var = NULL; \
    }
#define WCOPY_IF_EXISTS(var) \
    if (pSrc->var != NULL) { \
        pDst->var = SDL_wcsdup(pSrc->var); \
    } else { \
        pDst->var = NULL; \
    }

#ifdef SDL_LIBUSB_DYNAMIC
static void
LIBUSB_CopyHIDDeviceInfo(struct LIBUSB_hid_device_info *pSrc,
                         struct hid_device_info *pDst)
{
    COPY_IF_EXISTS(path)
    pDst->vendor_id = pSrc->vendor_id;
    pDst->product_id = pSrc->product_id;
    WCOPY_IF_EXISTS(serial_number)
    pDst->release_number = pSrc->release_number;
    WCOPY_IF_EXISTS(manufacturer_string)
    WCOPY_IF_EXISTS(product_string)
    pDst->usage_page = pSrc->usage_page;
    pDst->usage = pSrc->usage;
    pDst->interface_number = pSrc->interface_number;
    pDst->interface_class = pSrc->interface_class;
    pDst->interface_subclass = pSrc->interface_subclass;
    pDst->interface_protocol = pSrc->interface_protocol;
    pDst->next = NULL;
}
#endif /* SDL_LIBUSB_DYNAMIC */

#if HAVE_DRIVER_BACKEND
static void
DRIVER_CopyHIDDeviceInfo(struct DRIVER_hid_device_info *pSrc,
                           struct hid_device_info *pDst)
{
    COPY_IF_EXISTS(path)
    pDst->vendor_id = pSrc->vendor_id;
    pDst->product_id = pSrc->product_id;
    WCOPY_IF_EXISTS(serial_number)
    pDst->release_number = pSrc->release_number;
    WCOPY_IF_EXISTS(manufacturer_string)
    WCOPY_IF_EXISTS(product_string)
    pDst->usage_page = pSrc->usage_page;
    pDst->usage = pSrc->usage;
    pDst->interface_number = pSrc->interface_number;
    pDst->interface_class = pSrc->interface_class;
    pDst->interface_subclass = pSrc->interface_subclass;
    pDst->interface_protocol = pSrc->interface_protocol;
    pDst->next = NULL;
}
#endif /* HAVE_DRIVER_BACKEND */

#if HAVE_PLATFORM_BACKEND
static void
PLATFORM_CopyHIDDeviceInfo(struct PLATFORM_hid_device_info *pSrc,
                           struct hid_device_info *pDst)
{
    COPY_IF_EXISTS(path)
    pDst->vendor_id = pSrc->vendor_id;
    pDst->product_id = pSrc->product_id;
    WCOPY_IF_EXISTS(serial_number)
    pDst->release_number = pSrc->release_number;
    WCOPY_IF_EXISTS(manufacturer_string)
    WCOPY_IF_EXISTS(product_string)
    pDst->usage_page = pSrc->usage_page;
    pDst->usage = pSrc->usage;
    pDst->interface_number = pSrc->interface_number;
    pDst->interface_class = pSrc->interface_class;
    pDst->interface_subclass = pSrc->interface_subclass;
    pDst->interface_protocol = pSrc->interface_protocol;
    pDst->next = NULL;
}
#endif /* HAVE_PLATFORM_BACKEND */

#undef COPY_IF_EXISTS
#undef WCOPY_IF_EXISTS

static SDL_bool SDL_hidapi_wasinit = SDL_FALSE;

int HID_API_EXPORT HID_API_CALL hid_init(void)
{
    int attempts = 0, success = 0;

    if (SDL_hidapi_wasinit == SDL_TRUE) {
        return 0;
    }

#ifdef SDL_LIBUSB_DYNAMIC
    ++attempts;
    libusb_ctx.libhandle = SDL_LoadObject(SDL_LIBUSB_DYNAMIC);
    if (libusb_ctx.libhandle != NULL) {
        SDL_bool loaded = SDL_TRUE;
        #define LOAD_LIBUSB_SYMBOL(func) \
            if (!(libusb_ctx.func = SDL_LoadFunction(libusb_ctx.libhandle, "libusb_" #func))) {loaded = SDL_FALSE;}
        LOAD_LIBUSB_SYMBOL(init)
        LOAD_LIBUSB_SYMBOL(exit)
        LOAD_LIBUSB_SYMBOL(get_device_list)
        LOAD_LIBUSB_SYMBOL(free_device_list)
        LOAD_LIBUSB_SYMBOL(get_device_descriptor)
        LOAD_LIBUSB_SYMBOL(get_active_config_descriptor)
        LOAD_LIBUSB_SYMBOL(get_config_descriptor)
        LOAD_LIBUSB_SYMBOL(free_config_descriptor)
        LOAD_LIBUSB_SYMBOL(get_bus_number)
        LOAD_LIBUSB_SYMBOL(get_device_address)
        LOAD_LIBUSB_SYMBOL(open)
        LOAD_LIBUSB_SYMBOL(close)
        LOAD_LIBUSB_SYMBOL(claim_interface)
        LOAD_LIBUSB_SYMBOL(release_interface)
        LOAD_LIBUSB_SYMBOL(kernel_driver_active)
        LOAD_LIBUSB_SYMBOL(detach_kernel_driver)
        LOAD_LIBUSB_SYMBOL(attach_kernel_driver)
        LOAD_LIBUSB_SYMBOL(set_interface_alt_setting)
        LOAD_LIBUSB_SYMBOL(alloc_transfer)
        LOAD_LIBUSB_SYMBOL(submit_transfer)
        LOAD_LIBUSB_SYMBOL(cancel_transfer)
        LOAD_LIBUSB_SYMBOL(free_transfer)
        LOAD_LIBUSB_SYMBOL(control_transfer)
        LOAD_LIBUSB_SYMBOL(interrupt_transfer)
        LOAD_LIBUSB_SYMBOL(handle_events)
        LOAD_LIBUSB_SYMBOL(handle_events_completed)
        #undef LOAD_LIBUSB_SYMBOL

        if (!loaded) {
            SDL_UnloadObject(libusb_ctx.libhandle);
            libusb_ctx.libhandle = NULL;
            /* SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, SDL_LIBUSB_DYNAMIC " found but could not load function"); */
        } else if (LIBUSB_hid_init() < 0) {
            SDL_UnloadObject(libusb_ctx.libhandle);
            libusb_ctx.libhandle = NULL;
        } else {
            ++success;
        }
    }
#endif /* SDL_LIBUSB_DYNAMIC */

#if HAVE_PLATFORM_BACKEND
    ++attempts;
#if __LINUX__
    udev_ctx = SDL_UDEV_GetUdevSyms();
#endif /* __LINUX __ */
    if (udev_ctx && PLATFORM_hid_init() == 0) {
        ++success;
    }
#endif /* HAVE_PLATFORM_BACKEND */

    if (attempts > 0 && success == 0) {
        return -1;
    }

    SDL_hidapi_wasinit = SDL_TRUE;
    return 0;
}

int HID_API_EXPORT HID_API_CALL hid_exit(void)
{
    int result = 0;

    if (SDL_hidapi_wasinit == SDL_FALSE) {
        return 0;
    }
    SDL_hidapi_wasinit = SDL_FALSE;

#if HAVE_PLATFORM_BACKEND
    if (udev_ctx) {
        result |= PLATFORM_hid_exit();
    }
#endif /* HAVE_PLATFORM_BACKEND */

#ifdef SDL_LIBUSB_DYNAMIC
    if (libusb_ctx.libhandle) {
        result |= LIBUSB_hid_exit();
        SDL_UnloadObject(libusb_ctx.libhandle);
        libusb_ctx.libhandle = NULL;
    }
#endif /* SDL_LIBUSB_DYNAMIC */

    return result;
}

struct hid_device_info HID_API_EXPORT * HID_API_CALL hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
#if HAVE_PLATFORM_BACKEND || HAVE_DRIVER_BACKEND || defined(SDL_LIBUSB_DYNAMIC)
#ifdef SDL_LIBUSB_DYNAMIC
    struct LIBUSB_hid_device_info *usb_devs = NULL;
    struct LIBUSB_hid_device_info *usb_dev;
#endif
#if HAVE_DRIVER_BACKEND
    struct DRIVER_hid_device_info* driver_devs = NULL;
    struct DRIVER_hid_device_info* driver_dev;
#endif
#if HAVE_PLATFORM_BACKEND
    struct PLATFORM_hid_device_info *raw_devs = NULL;
    struct PLATFORM_hid_device_info *raw_dev;
#endif
    struct hid_device_info *devs = NULL, *last = NULL, *new_dev;

    if (hid_init() != 0) {
        return NULL;
    }

#ifdef SDL_LIBUSB_DYNAMIC
    if (libusb_ctx.libhandle) {
        usb_devs = LIBUSB_hid_enumerate(vendor_id, product_id);
  #ifdef DEBUG_HIDAPI
        SDL_Log("libusb devices found:");
  #endif
        for (usb_dev = usb_devs; usb_dev; usb_dev = usb_dev->next) {
            new_dev = (struct hid_device_info*) SDL_malloc(sizeof(struct hid_device_info));
            if (!new_dev) {
                LIBUSB_hid_free_enumeration(usb_devs);
                hid_free_enumeration(devs);
                SDL_OutOfMemory();
                return NULL;
            }
            LIBUSB_CopyHIDDeviceInfo(usb_dev, new_dev);
  #ifdef DEBUG_HIDAPI
            SDL_Log(" - %ls %ls 0x%.4hx 0x%.4hx",
                    usb_dev->manufacturer_string, usb_dev->product_string,
                    usb_dev->vendor_id, usb_dev->product_id);
  #endif

            if (last != NULL) {
                last->next = new_dev;
            } else {
                devs = new_dev;
            }
            last = new_dev;
        }
    }
#endif /* SDL_LIBUSB_DYNAMIC */

#ifdef HAVE_DRIVER_BACKEND
    driver_devs = DRIVER_hid_enumerate(vendor_id, product_id);
    for (driver_dev = driver_devs; driver_dev; driver_dev = driver_dev->next) {
        new_dev = (struct hid_device_info*) SDL_malloc(sizeof(struct hid_device_info));
        DRIVER_CopyHIDDeviceInfo(driver_dev, new_dev);

        if (last != NULL) {
            last->next = new_dev;
        } else {
            devs = new_dev;
        }
        last = new_dev;
    }
#endif /* HAVE_DRIVER_BACKEND */

#if HAVE_PLATFORM_BACKEND
    if (udev_ctx) {
        raw_devs = PLATFORM_hid_enumerate(vendor_id, product_id);
#ifdef DEBUG_HIDAPI
        SDL_Log("hidraw devices found:");
#endif
        for (raw_dev = raw_devs; raw_dev; raw_dev = raw_dev->next) {
            SDL_bool bFound = SDL_FALSE;
#ifdef DEBUG_HIDAPI
            SDL_Log(" - %ls %ls 0x%.4hx 0x%.4hx",
                    raw_dev->manufacturer_string, raw_dev->product_string,
                    raw_dev->vendor_id, raw_dev->product_id);
#endif
#ifdef SDL_LIBUSB_DYNAMIC
            for (usb_dev = usb_devs; usb_dev; usb_dev = usb_dev->next) {
                if (raw_dev->vendor_id == usb_dev->vendor_id &&
                    raw_dev->product_id == usb_dev->product_id &&
                    (raw_dev->interface_number < 0 || raw_dev->interface_number == usb_dev->interface_number)) {
                    bFound = SDL_TRUE;
                    break;
                }
            }
#endif
#ifdef HAVE_DRIVER_BACKEND
            for (driver_dev = driver_devs; driver_dev; driver_dev = driver_dev->next) {
                if (raw_dev->vendor_id == driver_dev->vendor_id &&
                    raw_dev->product_id == driver_dev->product_id &&
                    (raw_dev->interface_number < 0 || raw_dev->interface_number == driver_dev->interface_number)) {
                    bFound = SDL_TRUE;
                    break;
                }
            }
#endif
            if (!bFound) {
                new_dev = (struct hid_device_info*) SDL_malloc(sizeof(struct hid_device_info));
                if (!new_dev) {
#ifdef SDL_LIBUSB_DYNAMIC
                    if (libusb_ctx.libhandle) {
                        LIBUSB_hid_free_enumeration(usb_devs);
                    }
#endif
                    PLATFORM_hid_free_enumeration(raw_devs);
                    hid_free_enumeration(devs);
                    SDL_OutOfMemory();
                    return NULL;
                }
                PLATFORM_CopyHIDDeviceInfo(raw_dev, new_dev);
                new_dev->next = NULL;

                if (last != NULL) {
                    last->next = new_dev;
                } else {
                    devs = new_dev;
                }
                last = new_dev;
            }
        }
        PLATFORM_hid_free_enumeration(raw_devs);
    }
#endif /* HAVE_PLATFORM_BACKEND */

#ifdef SDL_LIBUSB_DYNAMIC
    if (libusb_ctx.libhandle) {
        LIBUSB_hid_free_enumeration(usb_devs);
    }
#endif
    return devs;

#else
    return NULL;
#endif /* HAVE_PLATFORM_BACKEND || HAVE_DRIVER_BACKEND || SDL_LIBUSB_DYNAMIC */
}

void  HID_API_EXPORT HID_API_CALL hid_free_enumeration(struct hid_device_info *devs)
{
    while (devs) {
        struct hid_device_info *next = devs->next;
        SDL_free(devs->path);
        SDL_free(devs->serial_number);
        SDL_free(devs->manufacturer_string);
        SDL_free(devs->product_string);
        SDL_free(devs);
        devs = next;
    }
}

HID_API_EXPORT hid_device * HID_API_CALL hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number)
{
#if HAVE_PLATFORM_BACKEND || HAVE_DRIVER_BACKEND || defined(SDL_LIBUSB_DYNAMIC)
    hid_device *pDevice = NULL;

    if (hid_init() != 0) {
        return NULL;
    }

#if HAVE_PLATFORM_BACKEND
    if (udev_ctx &&
        (pDevice = (hid_device*) PLATFORM_hid_open(vendor_id, product_id, serial_number)) != NULL) {

        HIDDeviceWrapper *wrapper = CreateHIDDeviceWrapper(pDevice, &PLATFORM_Backend);
        return WrapHIDDevice(wrapper);
    }
#endif /* HAVE_PLATFORM_BACKEND */

#if HAVE_DRIVER_BACKEND
    if ((pDevice = (hid_device*) DRIVER_hid_open(vendor_id, product_id, serial_number)) != NULL) {

        HIDDeviceWrapper *wrapper = CreateHIDDeviceWrapper(pDevice, &DRIVER_Backend);
        return WrapHIDDevice(wrapper);
    }
#endif /* HAVE_DRIVER_BACKEND */

#ifdef SDL_LIBUSB_DYNAMIC
    if (libusb_ctx.libhandle &&
        (pDevice = (hid_device*) LIBUSB_hid_open(vendor_id, product_id, serial_number)) != NULL) {

        HIDDeviceWrapper *wrapper = CreateHIDDeviceWrapper(pDevice, &LIBUSB_Backend);
        return WrapHIDDevice(wrapper);
    }
#endif /* SDL_LIBUSB_DYNAMIC */

#endif /* HAVE_PLATFORM_BACKEND || HAVE_DRIVER_BACKEND || SDL_LIBUSB_DYNAMIC */

    return NULL;
}

HID_API_EXPORT hid_device * HID_API_CALL hid_open_path(const char *path, int bExclusive /* = false */)
{
#if HAVE_PLATFORM_BACKEND || HAVE_DRIVER_BACKEND || defined(SDL_LIBUSB_DYNAMIC)
    hid_device *pDevice = NULL;

    if (hid_init() != 0) {
        return NULL;
    }

#if HAVE_PLATFORM_BACKEND
    if (udev_ctx &&
        (pDevice = (hid_device*) PLATFORM_hid_open_path(path, bExclusive)) != NULL) {

        HIDDeviceWrapper *wrapper = CreateHIDDeviceWrapper(pDevice, &PLATFORM_Backend);
        return WrapHIDDevice(wrapper);
    }
#endif /* HAVE_PLATFORM_BACKEND */

#if HAVE_DRIVER_BACKEND
    if ((pDevice = (hid_device*) DRIVER_hid_open_path(path, bExclusive)) != NULL) {

        HIDDeviceWrapper *wrapper = CreateHIDDeviceWrapper(pDevice, &DRIVER_Backend);
        return WrapHIDDevice(wrapper);
    }
#endif /* HAVE_DRIVER_BACKEND */

#ifdef SDL_LIBUSB_DYNAMIC
    if (libusb_ctx.libhandle &&
        (pDevice = (hid_device*) LIBUSB_hid_open_path(path, bExclusive)) != NULL) {

        HIDDeviceWrapper *wrapper = CreateHIDDeviceWrapper(pDevice, &LIBUSB_Backend);
        return WrapHIDDevice(wrapper);
    }
#endif /* SDL_LIBUSB_DYNAMIC */

#endif /* HAVE_PLATFORM_BACKEND || HAVE_DRIVER_BACKEND || SDL_LIBUSB_DYNAMIC */

    return NULL;
}

int  HID_API_EXPORT HID_API_CALL hid_write(hid_device *device, const unsigned char *data, size_t length)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_write(wrapper->device, data, length);
}

int HID_API_EXPORT HID_API_CALL hid_read_timeout(hid_device *device, unsigned char *data, size_t length, int milliseconds)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_read_timeout(wrapper->device, data, length, milliseconds);
}

int  HID_API_EXPORT HID_API_CALL hid_read(hid_device *device, unsigned char *data, size_t length)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_read(wrapper->device, data, length);
}

int  HID_API_EXPORT HID_API_CALL hid_set_nonblocking(hid_device *device, int nonblock)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_set_nonblocking(wrapper->device, nonblock);
}

int HID_API_EXPORT HID_API_CALL hid_send_feature_report(hid_device *device, const unsigned char *data, size_t length)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_send_feature_report(wrapper->device, data, length);
}

int HID_API_EXPORT HID_API_CALL hid_get_feature_report(hid_device *device, unsigned char *data, size_t length)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_get_feature_report(wrapper->device, data, length);
}

void HID_API_EXPORT HID_API_CALL hid_close(hid_device *device)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    wrapper->backend->hid_close(wrapper->device);
    DeleteHIDDeviceWrapper(wrapper);
}

int HID_API_EXPORT_CALL hid_get_manufacturer_string(hid_device *device, wchar_t *string, size_t maxlen)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_get_manufacturer_string(wrapper->device, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_product_string(hid_device *device, wchar_t *string, size_t maxlen)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_get_product_string(wrapper->device, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_serial_number_string(hid_device *device, wchar_t *string, size_t maxlen)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_get_serial_number_string(wrapper->device, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_indexed_string(hid_device *device, int string_index, wchar_t *string, size_t maxlen)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_get_indexed_string(wrapper->device, string_index, string, maxlen);
}

HID_API_EXPORT const wchar_t* HID_API_CALL hid_error(hid_device *device)
{
    HIDDeviceWrapper *wrapper = UnwrapHIDDevice(device);
    return wrapper->backend->hid_error(wrapper->device);
}

#ifdef HAVE_ENABLE_GAMECUBE_ADAPTORS
/* This is needed to enable input for Nyko and EVORETRO GameCube adaptors */
void SDL_EnableGameCubeAdaptors(void)
{
#ifdef SDL_LIBUSB_DYNAMIC
    libusb_context *usb_context = NULL;
    libusb_device **devs = NULL;
    libusb_device_handle *handle = NULL;
    struct libusb_device_descriptor desc;
    ssize_t i, num_devs;
    int kernel_detached = 0;

    if (libusb_ctx.libhandle == NULL) {
        return;
    }

    if (libusb_init(&usb_context) == 0) {
        num_devs = libusb_get_device_list(usb_context, &devs);
        for (i = 0; i < num_devs; ++i) {
            if (libusb_get_device_descriptor(devs[i], &desc) != 0) {
                continue;
            }

            if (desc.idVendor != 0x057e || desc.idProduct != 0x0337) {
                continue;
            }

            if (libusb_open(devs[i], &handle) != 0) {
                continue;
            }

            if (libusb_kernel_driver_active(handle, 0)) {
                if (libusb_detach_kernel_driver(handle, 0) == 0) {
                    kernel_detached = 1;
                }
            }

            if (libusb_claim_interface(handle, 0) == 0) {
                libusb_control_transfer(handle, 0x21, 11, 0x0001, 0, NULL, 0, 1000);
                libusb_release_interface(handle, 0);
            }

            if (kernel_detached) {
                libusb_attach_kernel_driver(handle, 0);
            }

            libusb_close(handle);
        }

        libusb_free_device_list(devs, 1);

        libusb_exit(usb_context);
    }
#endif /* SDL_LIBUSB_DYNAMIC */
}
#endif /* HAVE_ENABLE_GAMECUBE_ADAPTORS */

#endif /* SDL_JOYSTICK_HIDAPI */

/* vi: set sts=4 ts=4 sw=4 expandtab: */
