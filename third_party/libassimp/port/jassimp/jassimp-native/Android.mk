LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := jassimp
LOCAL_SRC_FILES :=  src/jassimp.cpp

LOCAL_CFLAGS += -DJNI_LOG

#LOCAL_STATIC_LIBRARIES := assimp_static
LOCAL_SHARED_LIBRARIES := assimp
LOCAL_LDLIBS    := -llog

include $(BUILD_SHARED_LIBRARY)
