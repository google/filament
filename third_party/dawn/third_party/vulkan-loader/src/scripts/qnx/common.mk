ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION = "Vulkan ICD Loader"
endef

ICD_ROOT=$(CURDIR)/../../../../..

EXTRA_INCVPATH+=$(ICD_ROOT)/scripts/gn
EXTRA_INCVPATH+=$(ICD_ROOT)/external/Vulkan-Headers/include

EXTRA_SRCVPATH+=$(ICD_ROOT)/loader
EXTRA_SRCVPATH+=$(ICD_ROOT)/loader/generated

SO_VERSION=1
NAME=vulkan

# Make the library

SRCS = cJSON.c debug_utils.c dev_ext_trampoline.c loader.c \
	phys_dev_ext.c trampoline.c unknown_ext_chain.c wsi.c \
	extension_manual.c unknown_function_handling.c settings.c \
	log.c allocation.c loader_environment.c gpa_helper.c \
	terminator.c

LDFLAGS += -Wl,--unresolved-symbols=report-all -Wl,--no-undefined -Wl,-fPIC

include $(MKFILES_ROOT)/qtargets.mk

CCFLAGS += -DVK_USE_PLATFORM_SCREEN_QNX=1 -DVK_ENABLE_BETA_EXTENSIONS
CCFLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers
CCFLAGS += -fno-strict-aliasing -Wno-stringop-truncation
CCFLAGS += -Wno-stringop-overflow -fvisibility=hidden
CCFLAGS += -Wpointer-arith -fPIC

CXXFLAGS += $(CCFLAGS)

# cJSON requires math library for pow() function
LIBS += m

INSTALLDIR=usr/lib
