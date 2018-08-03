LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -std=c++11 -Wall -Wextra
LOCAL_MODULE := spirv-cross
LOCAL_SRC_FILES := ../spirv_cfg.cpp ../spirv_cross.cpp ../spirv_cross_util.cpp ../spirv_glsl.cpp ../spirv_hlsl.cpp ../spirv_msl.cpp ../spirv_cpp.cpp
LOCAL_CPP_FEATURES := exceptions
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -D__STDC_LIMIT_MACROS

include $(BUILD_STATIC_LIBRARY)
