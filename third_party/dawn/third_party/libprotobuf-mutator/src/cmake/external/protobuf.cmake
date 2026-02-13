# Copyright 2016 Google Inc. All rights reserved.
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

# We only need protobuf_generate_cpp from FindProtobuf, and we are going to
# override the rest with ExternalProject version.
include (FindProtobuf)

set(PROTOBUF_TARGET external.protobuf)
set(PROTOBUF_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PROTOBUF_TARGET})

set(PROTOBUF_INCLUDE_DIRS ${PROTOBUF_INSTALL_DIR}/include)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  set(PROTOBUF_LIBRARIES protobufd)
ELSE()
  set(PROTOBUF_LIBRARIES protobuf)
ENDIF()

# TODO: find a better way to link absl.
list(APPEND PROTOBUF_LIBRARIES
  absl_bad_any_cast_impl
  absl_bad_optional_access
  absl_bad_variant_access
  absl_base
  absl_city
  absl_civil_time
  absl_cord
  absl_cord_internal
  absl_cordz_functions
  absl_cordz_handle
  absl_cordz_info
  absl_cordz_sample_token
  absl_crc_cord_state
  absl_crc_cpu_detect
  absl_crc_internal
  absl_crc32c
  absl_debugging_internal
  absl_demangle_internal
  absl_die_if_null
  absl_examine_stack
  absl_exponential_biased
  absl_failure_signal_handler
  absl_flags_commandlineflag
  absl_flags_commandlineflag_internal
  absl_flags_config
  absl_flags_internal
  absl_flags_marshalling
  absl_flags_parse
  absl_flags_private_handle_accessor
  absl_flags_program_name
  absl_flags_reflection
  absl_flags_usage
  absl_flags_usage_internal
  absl_graphcycles_internal
  absl_hash
  absl_hashtablez_sampler
  absl_int128
  absl_kernel_timeout_internal
  absl_leak_check
  absl_log_entry
  absl_log_flags
  absl_log_globals
  absl_log_initialize
  absl_log_internal_check_op
  absl_log_internal_conditions
  absl_log_internal_format
  absl_log_internal_globals
  absl_log_internal_log_sink_set
  absl_log_internal_message
  absl_log_internal_nullguard
  absl_log_internal_proto
  absl_log_severity
  absl_log_sink
  absl_low_level_hash
  absl_malloc_internal
  absl_periodic_sampler
  absl_random_distributions
  absl_random_internal_distribution_test_util
  absl_random_internal_platform
  absl_random_internal_pool_urbg
  absl_random_internal_randen
  absl_random_internal_randen_hwaes
  absl_random_internal_randen_hwaes_impl
  absl_random_internal_randen_slow
  absl_random_internal_seed_material
  absl_random_seed_gen_exception
  absl_random_seed_sequences
  absl_raw_hash_set
  absl_raw_logging_internal
  absl_scoped_set_env
  absl_spinlock_wait
  absl_stacktrace
  absl_status
  absl_statusor
  absl_str_format_internal
  absl_strerror
  absl_string_view
  absl_strings
  absl_strings_internal
  absl_symbolize
  absl_synchronization
  absl_throw_delegate
  absl_time
  absl_time_zone
  utf8_validity
)

foreach(lib ${PROTOBUF_LIBRARIES})
  if (MSVC)
    set(LIB_PATH ${PROTOBUF_INSTALL_DIR}/lib/lib${lib}.lib)
  else()
    set(LIB_PATH ${PROTOBUF_INSTALL_DIR}/lib/lib${lib}.a)
  endif()
  list(APPEND PROTOBUF_BUILD_BYPRODUCTS ${LIB_PATH})

  add_library(${lib} STATIC IMPORTED)
  set_property(TARGET ${lib} PROPERTY IMPORTED_LOCATION
               ${LIB_PATH})
  add_dependencies(${lib} ${PROTOBUF_TARGET})
endforeach(lib)

set(PROTOBUF_PROTOC_EXECUTABLE ${PROTOBUF_INSTALL_DIR}/bin/protoc)
list(APPEND PROTOBUF_BUILD_BYPRODUCTS ${PROTOBUF_PROTOC_EXECUTABLE})

if(${CMAKE_VERSION} VERSION_LESS "3.10.0")
  set(PROTOBUF_PROTOC_TARGET protoc)
else()
  set(PROTOBUF_PROTOC_TARGET protobuf::protoc)
endif()

if(NOT TARGET ${PROTOBUF_PROTOC_TARGET})
  add_executable(${PROTOBUF_PROTOC_TARGET} IMPORTED)
endif()
set_property(TARGET ${PROTOBUF_PROTOC_TARGET} PROPERTY IMPORTED_LOCATION
             ${PROTOBUF_PROTOC_EXECUTABLE})
add_dependencies(${PROTOBUF_PROTOC_TARGET} ${PROTOBUF_TARGET})

include (ExternalProject)
ExternalProject_Add(${PROTOBUF_TARGET}
    PREFIX ${PROTOBUF_TARGET}
    GIT_REPOSITORY https://github.com/google/protobuf.git
    GIT_TAG v27.1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ${CMAKE_COMMAND} ${PROTOBUF_INSTALL_DIR}/src/${PROTOBUF_TARGET}
        -G${CMAKE_GENERATOR}
        -DCMAKE_INSTALL_PREFIX=${PROTOBUF_INSTALL_DIR}
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER_LAUNCHER:FILEPATH=${CMAKE_C_COMPILER_LAUNCHER}
        -DCMAKE_CXX_COMPILER_LAUNCHER:FILEPATH=${CMAKE_CXX_COMPILER_LAUNCHER}
        -DCMAKE_C_FLAGS=${PROTOBUF_CFLAGS}
        -DCMAKE_CXX_FLAGS=${PROTOBUF_CXXFLAGS}
        -DCMAKE_CXX_STANDARD=14
        -Dprotobuf_BUILD_TESTS=OFF
    BUILD_BYPRODUCTS ${PROTOBUF_BUILD_BYPRODUCTS}
)

# Allow to use alphabetically ordered absl lib list.
set(CMAKE_LINK_GROUP_USING_cross_refs_SUPPORTED TRUE)
set(CMAKE_LINK_GROUP_USING_cross_refs
  "LINKER:--start-group"
  "LINKER:--end-group"
)

# cmake 3.7 uses Protobuf_ when 3.5 PROTOBUF_ prefixes.
set(Protobuf_INCLUDE_DIRS ${PROTOBUF_INCLUDE_DIRS})
set(Protobuf_LIBRARIES "$<LINK_GROUP:cross_refs,${PROTOBUF_LIBRARIES}>")
set(Protobuf_PROTOC_EXECUTABLE ${PROTOBUF_PROTOC_EXECUTABLE})
