# Copyright 2024 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#[==[.rst:
.. cmake:command:: dawn_add_library

  Create a library.

  .. code-block:: cmake

  dawn_add_library(<name>
    [FORCE_STATIC|FORCE_SHARED|FORCE_OBJECT]
    [HEADER_ONLY]
    [ENABLE_EMSCRIPTEN]
    [UTILITY_TARGET           <target>]
    [HEADERS                  <header>...]
    [PRIVATE_HEADERS          <header>...]
    [SOURCES                  <source>...]
    [DEPENDS                  <library>...]
    [PRIVATE_DEPENDS          <library>...])

  * ``FORCE_STATIC`` or ``FORCE_SHARED`` or ``FORCE_OBJECT``: Forces a
    static (respectively, shared and object) library to be created.
    If none is provided, ``BUILD_SHARED_LIBS`` will control the library type.
  * ``HEADER_ONLY``: The library only contains headers (or templates) and contains
    no compilation steps. Mutually exclusive with ``FORCE_STATIC``.
  * ``ENABLE_EMSCRIPTEN``: Enables the library target when building with
    Emscripten. By default, targets are not built with Emscripten.
  * ``UTILITY_TARGET``: If specified, all libraries and executables made by the
    Dawn library API will privately link to this target. This may be used to
    provide things such as project-wide compilation flags or similar.
  * ``HEADERS``: A list of header files.
  * ``PRIVATE_HEADERS``: A list of private header files.
  * ``SOURCES``: A list of source files which require compilation.
  * ``DEPENDS``: A list of libraries that this library must link against,
    equivalent to PUBLIC deps in target_link_libraries.
  * ``PRIVATE_DEPENDS``: A list of libraries that this library must link against,
    equivalent to PRIVATE deps in target_link_libraries.
#]==]
function(dawn_add_library name)
  set(kwargs)
  cmake_parse_arguments(PARSE_ARGV 1 arg
    "FORCE_STATIC;FORCE_SHARED;FORCE_OBJECT;HEADER_ONLY;ENABLE_EMSCRIPTEN"
    "UTILITY_TARGET"
    "HEADERS;PRIVATE_HEADERS;SOURCES;DEPENDS;PRIVATE_DEPENDS")

  if (arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
      "Unparsed arguments for dawn_add_library: "
      "${arg_UNPARSED_ARGUMENTS}")
  endif ()

  # Skip targets that shouldn't be built with Emscripten.
  if (EMSCRIPTEN AND NOT arg_ENABLE_EMSCRIPTEN)
    return()
  endif ()

  if (arg_HEADER_ONLY AND arg_FORCE_STATIC)
    message(FATAL_ERROR
      "The ${name} library cannot be header only yet forced static.")
  endif ()

  if (NOT (arg_SOURCES OR arg_HEADERS))
    message(FATAL_ERROR
      "The ${name} library needs at least one of sources or headers.")
  endif ()

  if (arg_FORCE_SHARED AND arg_FORCE_STATIC)
    message(FATAL_ERROR
      "The ${name} library cannot be both shared and static.")
  elseif (arg_FORCE_SHARED AND arg_FORCE_OBJECT)
    message(FATAL_ERROR
      "The ${name} library cannot be both shared and object.")
  elseif (arg_FORCE_STATIC AND arg_FORCE_OBJECT)
    message(FATAL_ERROR
      "The ${name} library cannot be both static and object.")
  endif ()

  if (NOT arg_SOURCES AND NOT arg_HEADER_ONLY)
    message(AUTHOR_WARNING
      "The ${name} library has no source files. Did you mean to "
      "pass the `HEADER_ONLY` flag?")
  endif ()

  set(library_type)
  if (arg_FORCE_STATIC)
    set(library_type STATIC)
  elseif (arg_FORCE_OBJECT)
    set(library_type OBJECT)
  elseif (arg_FORCE_SHARED)
    set(library_type SHARED)
  elseif (BUILD_SHARED_LIBS)
    set(library_type SHARED)
  else ()
    set(library_type STATIC)
  endif ()

  if (arg_HEADER_ONLY)
    add_library("${name}" INTERFACE)
    target_link_libraries("${name}"
      INTERFACE
        ${arg_DEPENDS})
    target_sources("${name}"
      PUBLIC
        ${arg_HEADERS})
  else ()
    add_library("${name}" ${library_type})
    if (arg_HEADERS)
      target_sources("${name}"
        PUBLIC
          ${arg_HEADERS})
    endif ()
    target_sources("${name}"
      PRIVATE
        ${arg_PRIVATE_HEADERS}
        ${arg_SOURCES})
    target_link_libraries("${name}"
      PUBLIC
        ${arg_DEPENDS}
      PRIVATE
        ${arg_PRIVATE_DEPENDS}
        ${arg_UTILITY_TARGET}
      )
    common_compile_options("${name}")
  endif ()
  add_library("dawn::${name}" ALIAS "${name}")
endfunction()

#[==[.rst:
.. cmake:command:: dawn_install_target

  Install a target and associated header files.

  .. code-block:: cmake

  dawn_install_target(<name>
    [HEADERS                  <header>...]
  )

  * ``HEADERS``: A list of header files to install.
#]==]
function(dawn_install_target name)
  cmake_parse_arguments(PARSE_ARGV 1 arg
    ""
    ""
    "HEADERS")
  if (arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
      "Unparsed arguments for dawn_install_target: "
      "${arg_UNPARSED_ARGUMENTS}")
  endif ()
  install(TARGETS "${name}"
    EXPORT DawnTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  # When building in debug mode with MSVC, install PDB files together with binaries
  if (MSVC)
    get_target_property(target_type "${name}" TYPE)
    if ((target_type STREQUAL "STATIC_LIBRARY") OR (target_type STREQUAL "SHARED_LIBRARY") OR (target_type STREQUAL "EXECUTABLE"))
      install(FILES $<TARGET_PDB_FILE:${name}> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
    endif()
  endif (MSVC)
  foreach(header IN LISTS arg_HEADERS)
    # Starting from CMake 3.20 there is the cmake_path command that could simplify this code.
    # Compute the install subdirectory for the header by stripping out the path to
    # the 'include' (or) 'gen/include' directory...
    string(FIND "${header}" "${DAWN_INCLUDE_DIR}" found)
    if (found EQUAL 0)
      string(LENGTH "${DAWN_INCLUDE_DIR}/" deduction)
    endif()
    string(FIND "${header}" "${DAWN_BUILD_GEN_DIR}/include/" found)
    if (found EQUAL 0)
      string(LENGTH "${DAWN_BUILD_GEN_DIR}/include/" deduction)
    endif()
    string(SUBSTRING "${header}" "${deduction}" -1 subdir)

    # ... then remove everything after the last /
    string(FIND "${subdir}" "/" found REVERSE)
    string(SUBSTRING "${subdir}" 0 ${found} subdir)
    install(FILES "${header}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${subdir}")
  endforeach()
endfunction()
