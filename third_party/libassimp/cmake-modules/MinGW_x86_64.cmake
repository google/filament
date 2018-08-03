# this one sets internal to crosscompile (in theory)
SET(CMAKE_SYSTEM_NAME Windows)

# the minimalistic settings
SET(CMAKE_C_COMPILER "/usr/bin/x86_64-w64-mingw32-gcc")
SET(CMAKE_CXX_COMPILER "/usr/bin/x86_64-w64-mingw32-g++")
SET(CMAKE_RC_COMPILER "/usr/bin/x86_64-w64-mingw32-windres")

# where is the target (so called staging) environment
SET(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# search for programs in the build host directories (default BOTH)
#SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
