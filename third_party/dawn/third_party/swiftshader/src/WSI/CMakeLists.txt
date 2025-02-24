# Copyright 2020 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(ROOT_PROJECT_COMPILE_OPTIONS
    ${SWIFTSHADER_COMPILE_OPTIONS}
    ${WARNINGS_AS_ERRORS}
)

set(WSI_SRC_FILES
    HeadlessSurfaceKHR.cpp
    HeadlessSurfaceKHR.hpp
    VkSurfaceKHR.cpp
    VkSurfaceKHR.hpp
    VkSwapchainKHR.cpp
    VkSwapchainKHR.hpp
)

if(WIN32)
    list(APPEND WSI_SRC_FILES
        Win32SurfaceKHR.cpp
        Win32SurfaceKHR.hpp
    )
elseif(LINUX)
    if(SWIFTSHADER_BUILD_WSI_XCB)
        list(APPEND WSI_SRC_FILES
            XcbSurfaceKHR.cpp
            XcbSurfaceKHR.hpp
            libXCB.cpp
            libXCB.hpp
        )
    endif()

    if(SWIFTSHADER_BUILD_WSI_WAYLAND)
        list(APPEND WSI_SRC_FILES
            WaylandSurfaceKHR.cpp
            WaylandSurfaceKHR.hpp
            libWaylandClient.cpp
            libWaylandClient.hpp
        )
    endif()

    if(DIRECTFB)
        list(APPEND WSI_SRC_FILES
            DirectFBSurfaceEXT.cpp
            DirectFBSurfaceEXT.hpp
        )
    endif()

    if(D2D)
        list(APPEND WSI_SRC_FILES
            DisplaySurfaceKHR.cpp
            DisplaySurfaceKHR.hpp
        )
    endif()
elseif(APPLE)
    list(APPEND WSI_SRC_FILES
        MetalSurface.mm
        MetalSurface.hpp
    )
endif()

set(WSI_COMPILE_OPTIONS "")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND WSI_COMPILE_OPTIONS
        "-Wexit-time-destructors"  # declaration requires an exit-time destructor
    )
endif()

add_library(vk_wsi EXCLUDE_FROM_ALL
    ${WSI_SRC_FILES}
)

set_target_properties(vk_wsi PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    FOLDER "SwiftShader VK"
)

target_include_directories(vk_wsi
    PUBLIC
        ".."
        "${SWIFTSHADER_DIR}/include"
)

target_compile_options(vk_wsi
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
        ${WSI_COMPILE_OPTIONS}
)

target_link_options(vk_wsi
    PUBLIC
        ${SWIFTSHADER_LINK_FLAGS}
)

target_link_libraries(vk_wsi
    PUBLIC
        vk_pipeline
)
