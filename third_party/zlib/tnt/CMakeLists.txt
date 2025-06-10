# ----------------------------------------------------------------------------
#  CMake file for zlib. See root CMakeLists.txt
#
# ----------------------------------------------------------------------------

project(zlib C)

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckTypeSize)

#
# Check for fseeko
#
check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
  add_definitions(-DNO_FSEEKO)
endif()

#
# Check for unistd.h
#
if(NOT MSVC)
  check_include_file(unistd.h Z_HAVE_UNISTD_H)
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

#
# Check to see if we have large file support
#
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
  add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              zlib)
set(SRC_DIR             ${OUR_DIR})
set(PUBLIC_HDR_DIR      ${OUR_DIR})
set(PRIVATE_HDR_DIR     ${OUR_DIR})

configure_file(	${PUBLIC_HDR_DIR}/zconf.h.cmakein
                ${PUBLIC_HDR_DIR}/zconf.h @ONLY)

include_directories("${OUR_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")

set(ZLIB_PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/zconf.h
    ${PUBLIC_HDR_DIR}/zlib.h
)
set(ZLIB_PRIVATE_HDRS
    ${PRIVATE_HDR_DIR}/crc32.h
    ${PRIVATE_HDR_DIR}/deflate.h
    ${PRIVATE_HDR_DIR}/gzguts.h
    ${PRIVATE_HDR_DIR}/inffast.h
    ${PRIVATE_HDR_DIR}/inffixed.h
    ${PRIVATE_HDR_DIR}/inflate.h
    ${PRIVATE_HDR_DIR}/inftrees.h
    ${PRIVATE_HDR_DIR}/trees.h
    ${PRIVATE_HDR_DIR}/zutil.h
)
set(ZLIB_SRCS
    ${SRC_DIR}/adler32.c
    ${SRC_DIR}/compress.c
    ${SRC_DIR}/crc32.c
    ${SRC_DIR}/deflate.c
    ${SRC_DIR}/gzclose.c
    ${SRC_DIR}/gzlib.c
    ${SRC_DIR}/gzread.c
    ${SRC_DIR}/gzwrite.c
    ${SRC_DIR}/inflate.c
    ${SRC_DIR}/infback.c
    ${SRC_DIR}/inftrees.c
    ${SRC_DIR}/inffast.c
    ${SRC_DIR}/trees.c
    ${SRC_DIR}/uncompr.c
    ${SRC_DIR}/zutil.c
)

add_library(zlib STATIC ${OPENCV_3RDPARTY_EXCLUDE_FROM_ALL} ${ZLIB_SRCS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})
set_target_properties(zlib PROPERTIES DEFINE_SYMBOL ZLIB_DLL)

# evaluate whether this is even necessary, if so migrate to target_compile_options
#ocv_warnings_disable(CMAKE_C_FLAGS -Wshorten-64-to-32 -Wattributes -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wshift-negative-value
#    -Wundef  # _LFS64_LARGEFILE is not defined
#    /wd4267  # MSVS 2015 (x64) + zlib 1.2.11
#    -Wimplicit-fallthrough
#    /wd4244  # MSVS + zlib 1.2.12: warning C4244: '=': conversion from 'ush' to 'uchf', possible loss of data
#)

set_target_properties(zlib PROPERTIES
    OUTPUT_NAME zlib
    COMPILE_PDB_NAME zlib
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
)

if(ENABLE_SOLUTION_FOLDERS)
  set_target_properties(zlib PROPERTIES FOLDER "third_party")
endif()

# evaluate if we even need this, if so migrate to install()
if(NOT BUILD_SHARED_LIBS)
#  ocv_install_target(zlib EXPORT OpenCVModules ARCHIVE DESTINATION ${OPENCV_3P_LIB_INSTALL_PATH} COMPONENT dev)
endif()

# what does this even do?
#ocv_install_3rdparty_licenses(zlib LICENSE)
