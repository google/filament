LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Import variables LIBDRM_ETNAVIV_FILES, LIBDRM_ETNAVIV_H_FILES
include $(LOCAL_PATH)/Makefile.sources

LOCAL_MODULE := libdrm_etnaviv

LOCAL_SHARED_LIBRARIES := libdrm

LOCAL_SRC_FILES := $(LIBDRM_ETNAVIV_FILES)

include $(LIBDRM_COMMON_MK)
include $(BUILD_SHARED_LIBRARY)
