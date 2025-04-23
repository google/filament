find_path(METAL_IRCONVERTER_INCLUDE_DIR metal_irconverter.h
          HINTS /usr/local/include/metal_irconverter
          DOC "Path to metal IR converter headers"
          )

find_library(METAL_IRCONVERTER_LIB NAMES metalirconverter
  PATH_SUFFIXES lib
  )

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(METAL_IRCONVERTER
                                    REQUIRED_VARS METAL_IRCONVERTER_LIB METAL_IRCONVERTER_INCLUDE_DIR)

message(STATUS "Metal IR Converter Include Dir: ${METAL_IRCONVERTER_INCLUDE_DIR}")
message(STATUS "Metal IR Converter Library: ${METAL_IRCONVERTER_LIB}")
mark_as_advanced(METAL_IRCONVERTER_LIB METAL_IRCONVERTER_INCLUDE_DIR)
