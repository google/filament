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

set(ASTC_ENCODER_SRC_FILES
    Source/astc_block_sizes2.cpp
    Source/astc_codec_internals.h
    Source/astc_color_unquantize.cpp
    Source/astc_decompress_symbolic.cpp
    Source/astc_image_load_store.cpp
    Source/astc_integer_sequence.cpp
    Source/astc_mathlib.cpp
    Source/astc_mathlib.h
    Source/astc_mathlib_softfloat.cpp
    Source/astc_partition_tables.cpp
    Source/astc_quantization.cpp
    Source/astc_symbolic_physical.cpp
    Source/astc_weight_quant_xfer_tables.cpp
)

add_library(astc-encoder STATIC EXCLUDE_FROM_ALL
    ${ASTC_ENCODER_SRC_FILES}
)

set_target_properties(astc-encoder PROPERTIES
    POSITION_INDEPENDENT_CODE 1
)

target_include_directories(astc-encoder
    PUBLIC
        "Source"
)

target_compile_definitions(astc-encoder
    PUBLIC
        # TODO: Remove SWIFTSHADER from the name
        "SWIFTSHADER_ENABLE_ASTC"
)

target_compile_options(astc-encoder
    PRIVATE
        ${ROOT_PROJECT_COMPILE_OPTIONS}
)
