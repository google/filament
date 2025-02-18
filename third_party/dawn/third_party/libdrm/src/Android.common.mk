# XXX: Consider moving these to config.h analogous to autoconf.
LOCAL_CFLAGS += \
	-DMAJOR_IN_SYSMACROS=1 \
	-DHAVE_ALLOCA_H=0 \
	-DHAVE_SYS_SELECT_H=0 \
	-DHAVE_SYS_SYSCTL_H=0 \
	-DHAVE_VISIBILITY=1 \
	-fvisibility=hidden \
	-DHAVE_LIBDRM_ATOMIC_PRIMITIVES=1

LOCAL_CFLAGS += \
	-Wno-error \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith \
	-Wno-enum-conversion

# Quiet down the build system and remove any .h files from the sources
LOCAL_SRC_FILES := $(patsubst %.h, , $(LOCAL_SRC_FILES))
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)

LOCAL_PROPRIETARY_MODULE := true
