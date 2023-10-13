# Copyright (C) 2020 The Khronos Group Inc.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
#    Neither the name of The Khronos Group Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

add_library(OSDependent STATIC ossource.cpp ../osinclude.h)
set_property(TARGET OSDependent PROPERTY FOLDER glslang)
set_property(TARGET OSDependent PROPERTY POSITION_INDEPENDENT_CODE ON)

# Link pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(OSDependent Threads::Threads)

if(ENABLE_GLSLANG_INSTALL AND NOT BUILD_SHARED_LIBS)
    install(TARGETS OSDependent EXPORT glslang-targets)

    # Backward compatibility
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/OSDependentTargets.cmake" "
        message(WARNING \"Using `OSDependentTargets.cmake` is deprecated: use `find_package(glslang)` to find glslang CMake targets.\")

        if (NOT TARGET glslang::OSDependent)
            include(\"${CMAKE_INSTALL_FULL_LIBDIR}/cmake/${PROJECT_NAME}/glslang-targets.cmake\")
        endif()

        add_library(OSDependent ALIAS glslang::OSDependent)
    ")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/OSDependentTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)
endif()
