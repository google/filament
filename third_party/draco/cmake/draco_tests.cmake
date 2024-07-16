if(DRACO_CMAKE_DRACO_TESTS_CMAKE)
  return()
endif()
set(DRACO_CMAKE_DRACO_TESTS_CMAKE 1)

# The factory tests are in a separate target to avoid breaking tests that rely
# on file I/O via the factories. The fake reader and writer implementations
# interfere with normal file I/O function.
set(draco_factory_test_sources
    "${draco_src_root}/io/file_reader_factory_test.cc"
    "${draco_src_root}/io/file_writer_factory_test.cc")

list(
  APPEND
    draco_test_sources
    "${draco_src_root}/animation/keyframe_animation_encoding_test.cc"
    "${draco_src_root}/animation/keyframe_animation_test.cc"
    "${draco_src_root}/attributes/point_attribute_test.cc"
    "${draco_src_root}/compression/attributes/point_d_vector_test.cc"
    "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_canonicalized_transform_test.cc"
    "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_transform_test.cc"
    "${draco_src_root}/compression/attributes/sequential_integer_attribute_encoding_test.cc"
    "${draco_src_root}/compression/bit_coders/rans_coding_test.cc"
    "${draco_src_root}/compression/decode_test.cc"
    "${draco_src_root}/compression/encode_test.cc"
    "${draco_src_root}/compression/entropy/shannon_entropy_test.cc"
    "${draco_src_root}/compression/entropy/symbol_coding_test.cc"
    "${draco_src_root}/compression/mesh/mesh_edgebreaker_encoding_test.cc"
    "${draco_src_root}/compression/mesh/mesh_encoder_test.cc"
    "${draco_src_root}/compression/point_cloud/point_cloud_kd_tree_encoding_test.cc"
    "${draco_src_root}/compression/point_cloud/point_cloud_sequential_encoding_test.cc"
    "${draco_src_root}/core/buffer_bit_coding_test.cc"
    "${draco_src_root}/core/draco_test_base.h"
    "${draco_src_root}/core/draco_test_utils.cc"
    "${draco_src_root}/core/draco_test_utils.h"
    "${draco_src_root}/core/math_utils_test.cc"
    "${draco_src_root}/core/quantization_utils_test.cc"
    "${draco_src_root}/core/status_test.cc"
    "${draco_src_root}/core/vector_d_test.cc"
    "${draco_src_root}/io/file_reader_test_common.h"
    "${draco_src_root}/io/file_utils_test.cc"
    "${draco_src_root}/io/stdio_file_reader_test.cc"
    "${draco_src_root}/io/stdio_file_writer_test.cc"
    "${draco_src_root}/io/obj_decoder_test.cc"
    "${draco_src_root}/io/obj_encoder_test.cc"
    "${draco_src_root}/io/ply_decoder_test.cc"
    "${draco_src_root}/io/ply_reader_test.cc"
    "${draco_src_root}/io/point_cloud_io_test.cc"
    "${draco_src_root}/mesh/mesh_are_equivalent_test.cc"
    "${draco_src_root}/mesh/mesh_cleanup_test.cc"
    "${draco_src_root}/mesh/triangle_soup_mesh_builder_test.cc"
    "${draco_src_root}/metadata/metadata_encoder_test.cc"
    "${draco_src_root}/metadata/metadata_test.cc"
    "${draco_src_root}/point_cloud/point_cloud_builder_test.cc"
    "${draco_src_root}/point_cloud/point_cloud_test.cc")

list(APPEND draco_gtest_all
            "${draco_root}/../googletest/googletest/src/gtest-all.cc")
list(APPEND draco_gtest_main
            "${draco_root}/../googletest/googletest/src/gtest_main.cc")

macro(draco_setup_test_targets)
  if(DRACO_TESTS)
    if(NOT (EXISTS ${draco_gtest_all} AND EXISTS ${draco_gtest_main}))
      message(FATAL "googletest must be a sibling directory of ${draco_root}.")
    endif()

    list(APPEND draco_test_defines GTEST_HAS_PTHREAD=0)

    draco_add_library(TEST
                      NAME
                      draco_gtest
                      TYPE
                      STATIC
                      SOURCES
                      ${draco_gtest_all}
                      DEFINES
                      ${draco_defines}
                      ${draco_test_defines}
                      INCLUDES
                      ${draco_test_include_paths})

    draco_add_library(TEST
                      NAME
                      draco_gtest_main
                      TYPE
                      STATIC
                      SOURCES
                      ${draco_gtest_main}
                      DEFINES
                      ${draco_defines}
                      ${draco_test_defines}
                      INCLUDES
                      ${draco_test_include_paths})

    set(DRACO_TEST_DATA_DIR "${draco_root}/testdata")
    set(DRACO_TEST_TEMP_DIR "${draco_build}/draco_test_temp")
    file(MAKE_DIRECTORY "${DRACO_TEST_TEMP_DIR}")

    # Sets DRACO_TEST_DATA_DIR and DRACO_TEST_TEMP_DIR.
    configure_file("${draco_root}/cmake/draco_test_config.h.cmake"
                   "${draco_build}/testing/draco_test_config.h")

    # Create the test targets.
    draco_add_executable(NAME
                         draco_tests
                         SOURCES
                         ${draco_test_sources}
                         DEFINES
                         ${draco_defines}
                         ${draco_test_defines}
                         INCLUDES
                         ${draco_test_include_paths}
                         LIB_DEPS
                         draco_static
                         draco_gtest
                         draco_gtest_main)

    draco_add_executable(NAME
                         draco_factory_tests
                         SOURCES
                         ${draco_factory_test_sources}
                         DEFINES
                         ${draco_defines}
                         ${draco_test_defines}
                         INCLUDES
                         ${draco_test_include_paths}
                         LIB_DEPS
                         draco_static
                         draco_gtest
                         draco_gtest_main)
  endif()
endmacro()
