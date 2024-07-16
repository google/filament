# The MIT License (MIT)
#
# Copyright (c)
#   2013 Matthew Arsenault
#   2015-2016 RWTH Aachen University, Federal Republic of Germany
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# If any of the used compiler is a GNU compiler, add a second option to static
# link against the sanitizers.
option(SANITIZE_LINK_STATIC "Try to link static against sanitizers." Off)




set(FIND_QUIETLY_FLAG "")
if (DEFINED Sanitizers_FIND_QUIETLY)
    set(FIND_QUIETLY_FLAG "QUIET")
endif ()

find_package(ASan ${FIND_QUIETLY_FLAG})
find_package(TSan ${FIND_QUIETLY_FLAG})
find_package(MSan ${FIND_QUIETLY_FLAG})
find_package(UBSan ${FIND_QUIETLY_FLAG})




function(sanitizer_add_blacklist_file FILE)
    if(NOT IS_ABSOLUTE ${FILE})
        set(FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
    endif()
    get_filename_component(FILE "${FILE}" REALPATH)

    sanitizer_check_compiler_flags("-fsanitize-blacklist=${FILE}"
        "SanitizerBlacklist" "SanBlist")
endfunction()

function(add_sanitizers ...)
    # If no sanitizer is enabled, return immediately.
    if (NOT (SANITIZE_ADDRESS OR SANITIZE_MEMORY OR SANITIZE_THREAD OR
        SANITIZE_UNDEFINED))
        return()
    endif ()

    foreach (TARGET ${ARGV})
        # Check if this target will be compiled by exactly one compiler. Other-
        # wise sanitizers can't be used and a warning should be printed once.
        get_target_property(TARGET_TYPE ${TARGET} TYPE)
        if (TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
            message(WARNING "Can't use any sanitizers for target ${TARGET}, "
                    "because it is an interface library and cannot be "
                    "compiled directly.")
            return()
        endif ()
        sanitizer_target_compilers(${TARGET} TARGET_COMPILER)
        list(LENGTH TARGET_COMPILER NUM_COMPILERS)
        if (NUM_COMPILERS GREATER 1)
            message(WARNING "Can't use any sanitizers for target ${TARGET}, "
                    "because it will be compiled by incompatible compilers. "
                    "Target will be compiled without sanitizers.")
            return()

        # If the target is compiled by no or no known compiler, give a warning.
        elseif (NUM_COMPILERS EQUAL 0)
            message(WARNING "Sanitizers for target ${TARGET} may not be"
                    " usable, because it uses no or an unknown compiler. "
                    "This is a false warning for targets using only "
		    "object lib(s) as input.")
        endif ()

        # Add sanitizers for target.
        add_sanitize_address(${TARGET})
        add_sanitize_thread(${TARGET})
        add_sanitize_memory(${TARGET})
        add_sanitize_undefined(${TARGET})
	endforeach ()
endfunction(add_sanitizers)
