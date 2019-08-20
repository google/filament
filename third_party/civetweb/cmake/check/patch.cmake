message(STATUS "Patching check ${VERSION} ${SOURCE_DIR}")

# Patch checks for MinGW
# https://sourceforge.net/p/check/patches/53/
set(CMAKE_LISTS_LOCATION "${SOURCE_DIR}/CMakeLists.txt")
file(READ ${CMAKE_LISTS_LOCATION} CMAKE_LISTS)
string(REGEX REPLACE
  "(check_type_size\\((clock|clockid|timer)_t [A-Z_]+\\)[\r\n]+[^\r\n]+[\r\n]+[^\r\n]+[\r\n]+endif\\(NOT HAVE[A-Z_]+\\))"
  "set(CMAKE_EXTRA_INCLUDE_FILES time.h)\n\\1\nunset(CMAKE_EXTRA_INCLUDE_FILES)"
  CMAKE_LISTS "${CMAKE_LISTS}")
file(WRITE ${CMAKE_LISTS_LOCATION} "${CMAKE_LISTS}")
message(STATUS "Patched ${CMAKE_LISTS_LOCATION}")
