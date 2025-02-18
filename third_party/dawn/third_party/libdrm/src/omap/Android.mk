LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libdrm_omap
LOCAL_VENDOR_MODULE := true

LOCAL_SRC_FILES := omap_drm.c

LOCAL_SHARED_LIBRARIES := libdrm

include $(LIBDRM_COMMON_MK)

include $(BUILD_SHARED_LIBRARY)
