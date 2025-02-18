#
# Copyright © 2011-2012 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#

LIBDRM_ANDROID_MAJOR_VERSION := $(word 1, $(subst ., , $(PLATFORM_VERSION)))
ifneq ($(filter 2 4, $(LIBDRM_ANDROID_MAJOR_VERSION)),)
$(error "Android 4.4 and earlier not supported")
endif

LIBDRM_COMMON_MK := $(call my-dir)/Android.common.mk

LOCAL_PATH := $(call my-dir)
LIBDRM_TOP := $(LOCAL_PATH)

include $(CLEAR_VARS)

# Import variables LIBDRM_{,H,INCLUDE_H,INCLUDE_ANDROID_H,INCLUDE_VMWGFX_H}_FILES
include $(LOCAL_PATH)/Makefile.sources

#static library for the device (recovery)
include $(CLEAR_VARS)
LOCAL_MODULE := libdrm

LOCAL_SRC_FILES := $(LIBDRM_FILES)
LOCAL_EXPORT_C_INCLUDE_DIRS := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/include/drm \
	$(LOCAL_PATH)/android

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include/drm

include $(LIBDRM_COMMON_MK)
include $(BUILD_STATIC_LIBRARY)

# Shared library for the device
include $(CLEAR_VARS)
LOCAL_MODULE := libdrm

LOCAL_SRC_FILES := $(LIBDRM_FILES)
LOCAL_EXPORT_C_INCLUDE_DIRS := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/include/drm \
	$(LOCAL_PATH)/android

LOCAL_SHARED_LIBRARIES := \
	libcutils

LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/include/drm

include $(LIBDRM_COMMON_MK)
include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
