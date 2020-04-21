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

set(
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

macro(draco_setup_test_targets)
  if(ENABLE_TESTS)
    # Googletest defaults.
    set(GTEST_SOURCE_DIR
        "${draco_root}/../googletest"
        CACHE STRING "Path to googletest source directory")

    set(gtest_all "${GTEST_SOURCE_DIR}/googletest/src/gtest-all.cc")
    set(gtest_main "${GTEST_SOURCE_DIR}/googletest/src/gtest_main.cc")
    set(draco_gtest_sources "${gtest_all}" "${gtest_main}")

    # Confirm Googletest is where expected.
    if(EXISTS "${gtest_all}" AND EXISTS "${gtest_main}")
      set(DRACO_TEST_DATA_DIR "${draco_root}/testdata")
      if(NOT DRACO_TEST_TEMP_DIR)
        set(DRACO_TEST_TEMP_DIR "${draco_build_dir}/draco_test_temp")
        file(MAKE_DIRECTORY "${DRACO_TEST_TEMP_DIR}")
      endif()

      # Sets DRACO_TEST_DATA_DIR and DRACO_TEST_TEMP_DIR.
      configure_file("${draco_root}/cmake/draco_test_config.h.cmake"
                     "${draco_build_dir}/testing/draco_test_config.h")

      # Create Googletest target and update configuration.
      add_library(draco_gtest STATIC ${draco_gtest_sources})
      target_compile_definitions(draco_gtest PUBLIC GTEST_HAS_PTHREAD=0)
      include_directories("${GTEST_SOURCE_DIR}/googlemock/include"
                          "${GTEST_SOURCE_DIR}/googletest/include"
                          "${GTEST_SOURCE_DIR}/googletest")

      # Create the test targets.
      add_executable(draco_tests ${draco_test_sources})
      target_link_libraries(draco_tests draco draco_gtest)
      add_executable(draco_factory_tests ${draco_factory_test_sources})
      target_link_libraries(draco_factory_tests draco draco_gtest)
    else()
      set(ENABLE_TESTS OFF)
      message("Tests disabled: Google test sources not found in "
              "${GTEST_SOURCE_DIR}.")
    endif()
  endif()
endmacro()
