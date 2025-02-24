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

function(bundle_libraries output_target)
  function(get_dependencies input_target)
    get_target_property(alias ${input_target} ALIASED_TARGET)
    if(TARGET ${alias})
      set(input_target ${alias})
    endif()
    if(${input_target} IN_LIST all_dependencies)
      return()
    endif()
    list(APPEND all_dependencies ${input_target})

    get_target_property(link_libraries ${input_target} LINK_LIBRARIES)
    foreach(dependency IN LISTS link_libraries)
      if(TARGET ${dependency})
        get_dependencies(${dependency})
      endif()
    endforeach()

    get_target_property(link_libraries ${input_target} INTERFACE_LINK_LIBRARIES)
    foreach(dependency IN LISTS link_libraries)
      if(TARGET ${dependency})
        get_dependencies(${dependency})
      endif()
    endforeach()

    set(all_dependencies ${all_dependencies} PARENT_SCOPE)
  endfunction()

  foreach(input_target IN LISTS ARGN)
    get_dependencies(${input_target})
  endforeach()

  foreach(dependency IN LISTS all_dependencies)
    get_target_property(type ${dependency} TYPE)
    if(${type} STREQUAL "STATIC_LIBRARY")
      list(APPEND all_objects $<TARGET_OBJECTS:${dependency}>)
    elseif(${type} STREQUAL "OBJECT_LIBRARY")
      list(APPEND all_objects $<TARGET_OBJECTS:${dependency}>)
    endif()
  endforeach()

  add_library(${output_target} STATIC ${all_objects})

  add_dependencies(${output_target} ${ARGN})

endfunction()
