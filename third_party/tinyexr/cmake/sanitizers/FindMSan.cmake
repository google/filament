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

option(SANITIZE_MEMORY "Enable MemorySanitizer for sanitized targets." Off)

set(FLAG_CANDIDATES
    "-g -fsanitize=memory"
)


include(sanitize-helpers)

if (SANITIZE_MEMORY)
    if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        message(WARNING "MemorySanitizer disabled for target ${TARGET} because "
            "MemorySanitizer is supported for Linux systems only.")
        set(SANITIZE_MEMORY Off CACHE BOOL
            "Enable MemorySanitizer for sanitized targets." FORCE)
    elseif (NOT ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        message(WARNING "MemorySanitizer disabled for target ${TARGET} because "
            "MemorySanitizer is supported for 64bit systems only.")
        set(SANITIZE_MEMORY Off CACHE BOOL
            "Enable MemorySanitizer for sanitized targets." FORCE)
    else ()
        sanitizer_check_compiler_flags("${FLAG_CANDIDATES}" "MemorySanitizer"
            "MSan")
    endif ()
endif ()

function (add_sanitize_memory TARGET)
    if (NOT SANITIZE_MEMORY)
        return()
    endif ()

    sanitizer_add_flags(${TARGET} "MemorySanitizer" "MSan")
endfunction ()
