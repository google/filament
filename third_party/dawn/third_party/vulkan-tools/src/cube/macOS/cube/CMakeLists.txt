# ~~~
# Copyright (c) 2018-2024 Valve Corporation
# Copyright (c) 2018-2024 LunarG, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~

# VkCube Application Bundle

set(cube_RESOURCES ${PROJECT_BINARY_DIR}/staging-json/MoltenVK_icd.json
    ${CMAKE_CURRENT_LIST_DIR}/Resources/VulkanIcon.icns)

# Have Xcode handle the Storyboard
if(XCODE)
    set(cube_RESOURCES ${cube_RESOURCES} ${CMAKE_CURRENT_LIST_DIR}/Resources/Main.storyboard)
endif()

add_executable(vkcube MACOSX_BUNDLE)

target_sources(vkcube PRIVATE
    main.m
    AppDelegate.m
    AppDelegate.h
    DemoViewController.m
    DemoViewController.h
    ${cube_RESOURCES}
    ../../cube.vert.inc
    ../../cube.frag.inc
)

# Handle the Storyboard ourselves
if(NOT XCODE)
    # Compile the storyboard file with the ibtool.
    add_custom_command(TARGET vkcube POST_BUILD
                       COMMAND ${IBTOOL}
                               --errors
                               --warnings
                               --notices
                               --output-format human-readable-text
                               --compile ${CMAKE_CURRENT_BINARY_DIR}/vkcube.app/Contents/Resources/Main.storyboardc
                                         ${CMAKE_CURRENT_LIST_DIR}/Resources/Main.storyboard
                       COMMENT "Compiling storyboard")
endif()

add_dependencies(vkcube MoltenVK_icd-staging-json)

# Include demo source code dir because the MacOS cube's Objective-C source includes the "original" cube application C source code.
target_include_directories(vkcube PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# We do this so vkcube is linked to an individual library and NOT a framework.
target_link_libraries(vkcube Vulkan::Loader "-framework Cocoa -framework QuartzCore")

# Disable warnings about sprintf
target_compile_options(vkcube PRIVATE -Wno-deprecated-declarations)

set_target_properties(vkcube PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/Info.plist)

# The RESOURCE target property cannot be used in conjunction with the MACOSX_PACKAGE_LOCATION property.  We need fine-grained
# control over the Resource directory, so we have to specify the destination of all the resource files on a per-destination-
# directory basis. If all the files went into the top-level Resource directory, then we could simply set the RESOURCE property to a
# list of all the resource files.
set_source_files_properties(${cube_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_source_files_properties("${PROJECT_BINARY_DIR}/staging-json/MoltenVK_icd.json"
                            PROPERTIES
                            MACOSX_PACKAGE_LOCATION
                            "Resources/vulkan/icd.d")

# Copy the MoltenVK lib into the bundle.
if(XCODE)
    add_custom_command(TARGET vkcube POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy "${MOLTENVK_DIR}/Package/Release/MoltenVK/dynamic/dylib/macOS/libMoltenVK.dylib"
                               ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/vkcube.app/Contents/Frameworks/libMoltenVK.dylib)
else()
    add_custom_command(TARGET vkcube POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy "${MOLTENVK_DIR}/Package/Release/MoltenVK/dynamic/dylib/macOS/libMoltenVK.dylib"
                               ${CMAKE_CURRENT_BINARY_DIR}/vkcube.app/Contents/Frameworks/libMoltenVK.dylib)
endif()
