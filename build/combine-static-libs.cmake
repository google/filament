# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Portable equivalent of build/linux/combine-static-libs.sh, used when the host has no shell (i.e.
# cross-compiling to Android or Linux from Windows).
#
# Usage:
#   cmake -P combine-static-libs.cmake -- <path-to-ar> <output-archive> <archives>...
#
# Mach-O universal binaries are not handled; a Windows host never produces them.

math(EXPR LAST_ARG "${CMAKE_ARGC} - 1")
set(ARGS)
set(SEEN_SEPARATOR FALSE)
foreach(i RANGE 1 ${LAST_ARG})
    if (SEEN_SEPARATOR)
        list(APPEND ARGS "${CMAKE_ARGV${i}}")
    elseif ("${CMAKE_ARGV${i}}" STREQUAL "--")
        set(SEEN_SEPARATOR TRUE)
    endif()
endforeach()

list(LENGTH ARGS ARG_COUNT)
if (ARG_COUNT LESS 3)
    message(FATAL_ERROR
        "Usage: cmake -P combine-static-libs.cmake -- <ar> <output> <archives>...")
endif()

list(POP_FRONT ARGS AR_TOOL)
list(POP_FRONT ARGS OUTPUT_PATH)
set(ARCHIVES ${ARGS})

get_filename_component(OUTPUT_DIR "${OUTPUT_PATH}" DIRECTORY)
get_filename_component(OUTPUT_NAME "${OUTPUT_PATH}" NAME_WE)
set(TEMP_DIR "${OUTPUT_DIR}/_${OUTPUT_NAME}")

file(REMOVE_RECURSE "${TEMP_DIR}")
file(MAKE_DIRECTORY "${TEMP_DIR}")

set(ALL_OBJECTS)
foreach(ARCHIVE ${ARCHIVES})
    # Each archive is extracted into its own directory, and its objects are then prefixed, because
    # different archives routinely contain object files with the same name.
    get_filename_component(ARCHIVE_NAME "${ARCHIVE}" NAME_WE)
    string(SHA256 ARCHIVE_HASH "${ARCHIVE}")
    string(SUBSTRING "${ARCHIVE_HASH}" 0 8 ARCHIVE_HASH)
    set(ARCHIVE_PREFIX "${ARCHIVE_NAME}_${ARCHIVE_HASH}")
    set(ARCHIVE_DIR "${TEMP_DIR}/${ARCHIVE_PREFIX}")
    file(MAKE_DIRECTORY "${ARCHIVE_DIR}")

    execute_process(
        COMMAND "${AR_TOOL}" -x "${ARCHIVE}"
        WORKING_DIRECTORY "${ARCHIVE_DIR}"
        RESULT_VARIABLE EXTRACT_RESULT
        ERROR_VARIABLE EXTRACT_ERROR
    )
    if (NOT EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract ${ARCHIVE}: ${EXTRACT_ERROR}")
    endif()

    file(GLOB OBJECTS "${ARCHIVE_DIR}/*.o")
    foreach(OBJECT ${OBJECTS})
        get_filename_component(OBJECT_NAME "${OBJECT}" NAME)
        set(RENAMED "${ARCHIVE_DIR}/${ARCHIVE_PREFIX}_${OBJECT_NAME}")
        file(RENAME "${OBJECT}" "${RENAMED}")
        list(APPEND ALL_OBJECTS "${RENAMED}")
    endforeach()
endforeach()

if (NOT ALL_OBJECTS)
    message(FATAL_ERROR "No object files found in: ${ARCHIVES}")
endif()

# The object list routinely exceeds the Windows command line limit, so drive ar with a response
# file instead.
set(RESPONSE_FILE "${TEMP_DIR}/objects.rsp")
set(QUOTED_OBJECTS)
foreach(OBJECT ${ALL_OBJECTS})
    list(APPEND QUOTED_OBJECTS "\"${OBJECT}\"")
endforeach()
string(REPLACE ";" "\n" RESPONSE_CONTENT "${QUOTED_OBJECTS}")
file(WRITE "${RESPONSE_FILE}" "${RESPONSE_CONTENT}\n")

file(REMOVE "${OUTPUT_PATH}")
execute_process(
    COMMAND "${AR_TOOL}" -qc "${OUTPUT_PATH}" "@${RESPONSE_FILE}"
    RESULT_VARIABLE ARCHIVE_RESULT
    ERROR_VARIABLE ARCHIVE_ERROR
)
if (NOT ARCHIVE_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to create ${OUTPUT_PATH}: ${ARCHIVE_ERROR}")
endif()

file(REMOVE_RECURSE "${TEMP_DIR}")
