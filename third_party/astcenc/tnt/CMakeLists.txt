cmake_minimum_required(VERSION 3.19)
project(astcenc)

set(TARGET astcenc)

set(SRCDIR "${CMAKE_CURRENT_SOURCE_DIR}/../Source")
set(HDRDIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

set(SOURCES
    ${SRCDIR}/astc_averages_and_directions.cpp
    ${SRCDIR}/astc_block_sizes2.cpp
    ${SRCDIR}/astc_color_quantize.cpp
    ${SRCDIR}/astc_color_unquantize.cpp
    ${SRCDIR}/astc_compress_symbolic.cpp
    ${SRCDIR}/astc_decompress_symbolic.cpp
    ${SRCDIR}/astc_encoding_choice_error.cpp
    ${SRCDIR}/astc_find_best_partitioning.cpp
    ${SRCDIR}/astc_ideal_endpoints_and_weights.cpp
    ${SRCDIR}/astc_image_load_store.cpp
    ${SRCDIR}/astc_integer_sequence.cpp
    ${SRCDIR}/astc_ktx_dds.cpp
    ${SRCDIR}/astc_kmeans_partitioning.cpp
    ${SRCDIR}/astc_partition_tables.cpp
    ${SRCDIR}/astc_percentile_tables.cpp
    ${SRCDIR}/astc_pick_best_endpoint_format.cpp
    ${SRCDIR}/astc_quantization.cpp
    ${SRCDIR}/astc_toplevel.cpp
    ${SRCDIR}/astc_symbolic_physical.cpp
    ${SRCDIR}/astc_weight_align.cpp
    ${SRCDIR}/astc_weight_quant_xfer_tables.cpp
    ${SRCDIR}/astc_compute_variance.cpp
    ${SRCDIR}/mathlib.cpp
    ${SRCDIR}/softfloat.cpp
    ${SRCDIR}/astc_codec_internals.h
    ${SRCDIR}/mathlib.h
    ${SRCDIR}/softfloat.h
)

if (NOT MSVC)
    add_definitions(-Wno-tautological-compare)
endif()

add_library(${TARGET} STATIC ${SOURCES})

target_link_libraries(${TARGET} LINK_PUBLIC)

target_include_directories (${TARGET} PUBLIC ${HDRDIR})
