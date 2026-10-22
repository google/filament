find_path(MetalIRConverter_INCLUDE_DIR metal_irconverter.h
          HINTS /usr/local/include/metal_irconverter
          DOC "Path to metal IR converter headers"
          )

find_library(MetalIRConverter_LIB NAMES metalirconverter
  PATH_SUFFIXES lib
  )

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MetalIRConverter
                                  REQUIRED_VARS MetalIRConverter_LIB MetalIRConverter_INCLUDE_DIR)

message(STATUS "Metal IR Converter Include Dir: ${MetalIRConverter_INCLUDE_DIR}")
message(STATUS "Metal IR Converter Library: ${MetalIRConverter_LIB}")
mark_as_advanced(MetalIRConverter_LIB MetalIRConverter_INCLUDE_DIR)
