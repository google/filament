# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
file(TO_NATIVE_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" DOS_STYLE_OUTPUT_DIRECTORY)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" DOS_STYLE_SOURCE_DIR)

if (CMAKE_C_COMPILER_ARCHITECTURE_ID MATCHES "x64" )
  set ( DOTNET_PLATFORM_TARGET "x64" )
elseif (CMAKE_GENERATOR STREQUAL "Visual Studio 12" )
  set ( DOTNET_PLATFORM_TARGET "x86" )
else ()
  set ( DOTNET_PLATFORM_TARGET "AnyCPU" )
endif ()

configure_file(dndxc.csproj.txt dndxc.csproj)

include_external_msproject(
    dndxc ${CMAKE_CURRENT_BINARY_DIR}/dndxc.csproj
    TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)

set_target_properties(dndxc
   PROPERTIES
   FOLDER "Clang executables")
