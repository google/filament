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

option(SANITIZE_ADDRESS "Enable AddressSanitizer for sanitized targets." Off)

set(FLAG_CANDIDATES
    # Clang 3.2+ use this version. The no-omit-frame-pointer option is optional.
    "-g -fsanitize=address -fno-omit-frame-pointer"
    "-g -fsanitize=address"

    # Older deprecated flag for ASan
    "-g -faddress-sanitizer"
)


if (SANITIZE_ADDRESS AND (SANITIZE_THREAD OR SANITIZE_MEMORY))
    message(FATAL_ERROR "AddressSanitizer is not compatible with "
        "ThreadSanitizer or MemorySanitizer.")
endif ()


include(sanitize-helpers)

if (SANITIZE_ADDRESS)
    sanitizer_check_compiler_flags("${FLAG_CANDIDATES}" "AddressSanitizer"
        "ASan")

    find_program(ASan_WRAPPER "asan-wrapper" PATHS ${CMAKE_MODULE_PATH})
	mark_as_advanced(ASan_WRAPPER)
endif ()

function (add_sanitize_address TARGET)
    if (NOT SANITIZE_ADDRESS)
        return()
    endif ()

    sanitizer_add_flags(${TARGET} "AddressSanitizer" "ASan")
endfunction ()
