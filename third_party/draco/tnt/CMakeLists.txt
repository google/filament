cmake_minimum_required(VERSION 3.19)

project(draco C CXX)

if (ANDROID)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID")
endif()

if (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(draco_root "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(draco_src_root "${draco_root}/src/draco")
set(draco_build_dir "${CMAKE_CURRENT_BINARY_DIR}")

include(CMakePackageConfigHelpers)
include("${draco_root}/cmake/compiler_flags.cmake")
include("${draco_root}/cmake/draco_features.cmake")
include("${draco_root}/cmake/draco_tests.cmake")
include("${draco_root}/cmake/sanitizers.cmake")
include("${draco_root}/cmake/util.cmake")

draco_enable_feature(FEATURE "DRACO_MESH_COMPRESSION_SUPPORTED")
draco_enable_feature(FEATURE "DRACO_NORMAL_ENCODING_SUPPORTED")
draco_enable_feature(FEATURE "DRACO_STANDARD_EDGEBREAKER_SUPPORTED")

# Draco source file listing variables.
set(draco_attributes_sources
    "${draco_src_root}/attributes/attribute_octahedron_transform.cc"
    "${draco_src_root}/attributes/attribute_octahedron_transform.h"
    "${draco_src_root}/attributes/attribute_quantization_transform.cc"
    "${draco_src_root}/attributes/attribute_quantization_transform.h"
    "${draco_src_root}/attributes/attribute_transform.cc"
    "${draco_src_root}/attributes/attribute_transform.h"
    "${draco_src_root}/attributes/attribute_transform_data.h"
    "${draco_src_root}/attributes/attribute_transform_type.h"
    "${draco_src_root}/attributes/geometry_attribute.cc"
    "${draco_src_root}/attributes/geometry_attribute.h"
    "${draco_src_root}/attributes/geometry_indices.h"
    "${draco_src_root}/attributes/point_attribute.cc"
    "${draco_src_root}/attributes/point_attribute.h")

set(
  draco_compression_attributes_dec_sources
  "${draco_src_root}/compression/attributes/attributes_decoder.cc"
  "${draco_src_root}/compression/attributes/attributes_decoder.h"
  "${draco_src_root}/compression/attributes/kd_tree_attributes_decoder.cc"
  "${draco_src_root}/compression/attributes/kd_tree_attributes_decoder.h"
  "${draco_src_root}/compression/attributes/kd_tree_attributes_shared.h"
  "${draco_src_root}/compression/attributes/mesh_attribute_indices_encoding_data.h"
  "${draco_src_root}/compression/attributes/normal_compression_utils.h"
  "${draco_src_root}/compression/attributes/point_d_vector.h"
  "${draco_src_root}/compression/attributes/sequential_attribute_decoder.cc"
  "${draco_src_root}/compression/attributes/sequential_attribute_decoder.h"
  "${draco_src_root}/compression/attributes/sequential_attribute_decoders_controller.cc"
  "${draco_src_root}/compression/attributes/sequential_attribute_decoders_controller.h"
  "${draco_src_root}/compression/attributes/sequential_integer_attribute_decoder.cc"
  "${draco_src_root}/compression/attributes/sequential_integer_attribute_decoder.h"
  "${draco_src_root}/compression/attributes/sequential_normal_attribute_decoder.cc"
  "${draco_src_root}/compression/attributes/sequential_normal_attribute_decoder.h"
  "${draco_src_root}/compression/attributes/sequential_quantization_attribute_decoder.cc"
  "${draco_src_root}/compression/attributes/sequential_quantization_attribute_decoder.h"
  )

set(
  draco_compression_attributes_enc_sources
  "${draco_src_root}/compression/attributes/attributes_encoder.cc"
  "${draco_src_root}/compression/attributes/attributes_encoder.h"
  "${draco_src_root}/compression/attributes/kd_tree_attributes_encoder.cc"
  "${draco_src_root}/compression/attributes/kd_tree_attributes_encoder.h"
  "${draco_src_root}/compression/attributes/linear_sequencer.h"
  "${draco_src_root}/compression/attributes/points_sequencer.h"
  "${draco_src_root}/compression/attributes/sequential_attribute_encoder.cc"
  "${draco_src_root}/compression/attributes/sequential_attribute_encoder.h"
  "${draco_src_root}/compression/attributes/sequential_attribute_encoders_controller.cc"
  "${draco_src_root}/compression/attributes/sequential_attribute_encoders_controller.h"
  "${draco_src_root}/compression/attributes/sequential_integer_attribute_encoder.cc"
  "${draco_src_root}/compression/attributes/sequential_integer_attribute_encoder.h"
  "${draco_src_root}/compression/attributes/sequential_normal_attribute_encoder.cc"
  "${draco_src_root}/compression/attributes/sequential_normal_attribute_encoder.h"
  "${draco_src_root}/compression/attributes/sequential_quantization_attribute_encoder.cc"
  "${draco_src_root}/compression/attributes/sequential_quantization_attribute_encoder.h"
  )

set(
  draco_compression_attributes_pred_schemes_dec_sources
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_constrained_multi_parallelogram_decoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_constrained_multi_parallelogram_shared.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_data.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_decoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_geometric_normal_decoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_geometric_normal_predictor_area.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_geometric_normal_predictor_base.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_multi_parallelogram_decoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_parallelogram_encoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_parallelogram_shared.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_tex_coords_decoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_tex_coords_portable_decoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_tex_coords_portable_predictor.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_decoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_decoder_factory.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_decoder_interface.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_decoding_transform.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_delta_decoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_factory.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_interface.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_canonicalized_decoding_transform.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_canonicalized_transform_base.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_decoding_transform.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_transform_base.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_wrap_decoding_transform.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_wrap_transform_base.h"
  )

set(
  draco_compression_attributes_pred_schemes_enc_sources
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_constrained_multi_parallelogram_encoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_constrained_multi_parallelogram_shared.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_data.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_encoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_geometric_normal_encoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_geometric_normal_predictor_area.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_geometric_normal_predictor_base.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_multi_parallelogram_encoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_parallelogram_encoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_parallelogram_shared.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_tex_coords_encoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_tex_coords_portable_encoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/mesh_prediction_scheme_tex_coords_portable_predictor.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_delta_encoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_encoder.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_encoder_factory.cc"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_encoder_factory.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_encoder_interface.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_encoding_transform.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_factory.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_interface.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_canonicalized_encoding_transform.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_canonicalized_transform_base.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_encoding_transform.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_transform_base.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_wrap_encoding_transform.h"
  "${draco_src_root}/compression/attributes/prediction_schemes/prediction_scheme_wrap_transform_base.h"
  )

set(draco_compression_bit_coders_sources
    "${draco_src_root}/compression/bit_coders/adaptive_rans_bit_coding_shared.h"
    "${draco_src_root}/compression/bit_coders/adaptive_rans_bit_decoder.cc"
    "${draco_src_root}/compression/bit_coders/adaptive_rans_bit_decoder.h"
    "${draco_src_root}/compression/bit_coders/adaptive_rans_bit_encoder.cc"
    "${draco_src_root}/compression/bit_coders/adaptive_rans_bit_encoder.h"
    "${draco_src_root}/compression/bit_coders/direct_bit_decoder.cc"
    "${draco_src_root}/compression/bit_coders/direct_bit_decoder.h"
    "${draco_src_root}/compression/bit_coders/direct_bit_encoder.cc"
    "${draco_src_root}/compression/bit_coders/direct_bit_encoder.h"
    "${draco_src_root}/compression/bit_coders/folded_integer_bit_decoder.h"
    "${draco_src_root}/compression/bit_coders/folded_integer_bit_encoder.h"
    "${draco_src_root}/compression/bit_coders/rans_bit_decoder.cc"
    "${draco_src_root}/compression/bit_coders/rans_bit_decoder.h"
    "${draco_src_root}/compression/bit_coders/rans_bit_encoder.cc"
    "${draco_src_root}/compression/bit_coders/rans_bit_encoder.h"
    "${draco_src_root}/compression/bit_coders/symbol_bit_decoder.cc"
    "${draco_src_root}/compression/bit_coders/symbol_bit_decoder.h"
    "${draco_src_root}/compression/bit_coders/symbol_bit_encoder.cc"
    "${draco_src_root}/compression/bit_coders/symbol_bit_encoder.h")

set(draco_enc_config_sources
    "${draco_src_root}/compression/config/compression_shared.h"
    "${draco_src_root}/compression/config/draco_options.h"
    "${draco_src_root}/compression/config/encoder_options.h"
    "${draco_src_root}/compression/config/encoding_features.h")

set(draco_dec_config_sources
    "${draco_src_root}/compression/config/compression_shared.h"
    "${draco_src_root}/compression/config/decoder_options.h"
    "${draco_src_root}/compression/config/draco_options.h")

set(draco_compression_decode_sources "${draco_src_root}/compression/decode.cc"
                                     "${draco_src_root}/compression/decode.h")

set(draco_compression_encode_sources
    "${draco_src_root}/compression/encode.cc"
    "${draco_src_root}/compression/encode.h"
    "${draco_src_root}/compression/encode_base.h"
    "${draco_src_root}/compression/expert_encode.cc"
    "${draco_src_root}/compression/expert_encode.h")

set(
  draco_compression_mesh_traverser_sources
  "${draco_src_root}/compression/mesh/traverser/depth_first_traverser.h"
  "${draco_src_root}/compression/mesh/traverser/max_prediction_degree_traverser.h"
  "${draco_src_root}/compression/mesh/traverser/mesh_attribute_indices_encoding_observer.h"
  "${draco_src_root}/compression/mesh/traverser/mesh_traversal_sequencer.h"
  "${draco_src_root}/compression/mesh/traverser/traverser_base.h")

set(
  draco_compression_mesh_dec_sources
  "${draco_src_root}/compression/mesh/mesh_decoder.cc"
  "${draco_src_root}/compression/mesh/mesh_decoder.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_decoder.cc"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_decoder.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_decoder_impl.cc"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_decoder_impl.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_decoder_impl_interface.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_shared.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_traversal_decoder.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_traversal_predictive_decoder.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_traversal_valence_decoder.h"
  "${draco_src_root}/compression/mesh/mesh_sequential_decoder.cc"
  "${draco_src_root}/compression/mesh/mesh_sequential_decoder.h")

set(
  draco_compression_mesh_enc_sources
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_encoder.cc"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_encoder.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_encoder_impl.cc"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_encoder_impl.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_encoder_impl_interface.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_shared.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_traversal_encoder.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_traversal_predictive_encoder.h"
  "${draco_src_root}/compression/mesh/mesh_edgebreaker_traversal_valence_encoder.h"
  "${draco_src_root}/compression/mesh/mesh_encoder.cc"
  "${draco_src_root}/compression/mesh/mesh_encoder.h"
  "${draco_src_root}/compression/mesh/mesh_sequential_encoder.cc"
  "${draco_src_root}/compression/mesh/mesh_sequential_encoder.h")

set(
  draco_compression_point_cloud_dec_sources
  "${draco_src_root}/compression/point_cloud/point_cloud_decoder.cc"
  "${draco_src_root}/compression/point_cloud/point_cloud_decoder.h"
  "${draco_src_root}/compression/point_cloud/point_cloud_kd_tree_decoder.cc"
  "${draco_src_root}/compression/point_cloud/point_cloud_kd_tree_decoder.h"
  "${draco_src_root}/compression/point_cloud/point_cloud_sequential_decoder.cc"
  "${draco_src_root}/compression/point_cloud/point_cloud_sequential_decoder.h")

set(
  draco_compression_point_cloud_enc_sources
  "${draco_src_root}/compression/point_cloud/point_cloud_encoder.cc"
  "${draco_src_root}/compression/point_cloud/point_cloud_encoder.h"
  "${draco_src_root}/compression/point_cloud/point_cloud_kd_tree_encoder.cc"
  "${draco_src_root}/compression/point_cloud/point_cloud_kd_tree_encoder.h"
  "${draco_src_root}/compression/point_cloud/point_cloud_sequential_encoder.cc"
  "${draco_src_root}/compression/point_cloud/point_cloud_sequential_encoder.h")

set(draco_compression_entropy_sources
    "${draco_src_root}/compression/entropy/ans.h"
    "${draco_src_root}/compression/entropy/rans_symbol_coding.h"
    "${draco_src_root}/compression/entropy/rans_symbol_decoder.h"
    "${draco_src_root}/compression/entropy/rans_symbol_encoder.h"
    "${draco_src_root}/compression/entropy/shannon_entropy.cc"
    "${draco_src_root}/compression/entropy/shannon_entropy.h"
    "${draco_src_root}/compression/entropy/symbol_decoding.cc"
    "${draco_src_root}/compression/entropy/symbol_decoding.h"
    "${draco_src_root}/compression/entropy/symbol_encoding.cc"
    "${draco_src_root}/compression/entropy/symbol_encoding.h")

set(draco_core_sources
    "${draco_src_root}/core/bit_utils.cc"
    "${draco_src_root}/core/bit_utils.h"
    "${draco_src_root}/core/bounding_box.cc"
    "${draco_src_root}/core/bounding_box.h"
    "${draco_src_root}/core/cycle_timer.cc"
    "${draco_src_root}/core/cycle_timer.h"
    "${draco_src_root}/core/data_buffer.cc"
    "${draco_src_root}/core/data_buffer.h"
    "${draco_src_root}/core/decoder_buffer.cc"
    "${draco_src_root}/core/decoder_buffer.h"
    "${draco_src_root}/core/divide.cc"
    "${draco_src_root}/core/divide.h"
    "${draco_src_root}/core/draco_index_type.h"
    "${draco_src_root}/core/draco_index_type_vector.h"
    "${draco_src_root}/core/draco_types.cc"
    "${draco_src_root}/core/draco_types.h"
    "${draco_src_root}/core/encoder_buffer.cc"
    "${draco_src_root}/core/encoder_buffer.h"
    "${draco_src_root}/core/hash_utils.cc"
    "${draco_src_root}/core/hash_utils.h"
    "${draco_src_root}/core/macros.h"
    "${draco_src_root}/core/math_utils.h"
    "${draco_src_root}/core/options.cc"
    "${draco_src_root}/core/options.h"
    "${draco_src_root}/core/quantization_utils.cc"
    "${draco_src_root}/core/quantization_utils.h"
    "${draco_src_root}/core/status.h"
    "${draco_src_root}/core/status_or.h"
    "${draco_src_root}/core/varint_decoding.h"
    "${draco_src_root}/core/varint_encoding.h"
    "${draco_src_root}/core/vector_d.h")

set(draco_io_sources
    "${draco_src_root}/io/file_reader_factory.cc"
    "${draco_src_root}/io/file_reader_factory.h"
    "${draco_src_root}/io/file_reader_interface.h"
    "${draco_src_root}/io/file_utils.cc"
    "${draco_src_root}/io/file_utils.h"
    "${draco_src_root}/io/file_writer_factory.cc"
    "${draco_src_root}/io/file_writer_factory.h"
    "${draco_src_root}/io/file_writer_interface.h"
    "${draco_src_root}/io/mesh_io.cc"
    "${draco_src_root}/io/mesh_io.h"
    "${draco_src_root}/io/obj_decoder.cc"
    "${draco_src_root}/io/obj_decoder.h"
    "${draco_src_root}/io/obj_encoder.cc"
    "${draco_src_root}/io/obj_encoder.h"
    "${draco_src_root}/io/parser_utils.cc"
    "${draco_src_root}/io/parser_utils.h"
    "${draco_src_root}/io/ply_decoder.cc"
    "${draco_src_root}/io/ply_decoder.h"
    "${draco_src_root}/io/ply_encoder.cc"
    "${draco_src_root}/io/ply_encoder.h"
    "${draco_src_root}/io/ply_property_reader.h"
    "${draco_src_root}/io/ply_property_writer.h"
    "${draco_src_root}/io/ply_reader.cc"
    "${draco_src_root}/io/ply_reader.h"
    "${draco_src_root}/io/point_cloud_io.cc"
    "${draco_src_root}/io/point_cloud_io.h"
    "${draco_src_root}/io/stdio_file_reader.cc"
    "${draco_src_root}/io/stdio_file_reader.h"
    "${draco_src_root}/io/stdio_file_writer.cc"
    "${draco_src_root}/io/stdio_file_writer.h")

set(draco_mesh_sources
    "${draco_src_root}/mesh/corner_table.cc"
    "${draco_src_root}/mesh/corner_table.h"
    "${draco_src_root}/mesh/corner_table_iterators.h"
    "${draco_src_root}/mesh/mesh.cc"
    "${draco_src_root}/mesh/mesh.h"
    "${draco_src_root}/mesh/mesh_are_equivalent.cc"
    "${draco_src_root}/mesh/mesh_are_equivalent.h"
    "${draco_src_root}/mesh/mesh_attribute_corner_table.cc"
    "${draco_src_root}/mesh/mesh_attribute_corner_table.h"
    "${draco_src_root}/mesh/mesh_cleanup.cc"
    "${draco_src_root}/mesh/mesh_cleanup.h"
    "${draco_src_root}/mesh/mesh_misc_functions.cc"
    "${draco_src_root}/mesh/mesh_misc_functions.h"
    "${draco_src_root}/mesh/mesh_stripifier.cc"
    "${draco_src_root}/mesh/mesh_stripifier.h"
    "${draco_src_root}/mesh/triangle_soup_mesh_builder.cc"
    "${draco_src_root}/mesh/triangle_soup_mesh_builder.h"
    "${draco_src_root}/mesh/valence_cache.h")

set(draco_point_cloud_sources
    "${draco_src_root}/point_cloud/point_cloud.cc"
    "${draco_src_root}/point_cloud/point_cloud.h"
    "${draco_src_root}/point_cloud/point_cloud_builder.cc"
    "${draco_src_root}/point_cloud/point_cloud_builder.h")

set(
  draco_points_common_sources
  "${draco_src_root}/compression/point_cloud/algorithms/point_cloud_compression_method.h"
  "${draco_src_root}/compression/point_cloud/algorithms/point_cloud_types.h"
  "${draco_src_root}/compression/point_cloud/algorithms/quantize_points_3.h"
  "${draco_src_root}/compression/point_cloud/algorithms/queuing_policy.h")

set(
  draco_points_dec_sources
  "${draco_src_root}/compression/point_cloud/algorithms/dynamic_integer_points_kd_tree_decoder.cc"
  "${draco_src_root}/compression/point_cloud/algorithms/dynamic_integer_points_kd_tree_decoder.h"
  "${draco_src_root}/compression/point_cloud/algorithms/float_points_tree_decoder.cc"
  "${draco_src_root}/compression/point_cloud/algorithms/float_points_tree_decoder.h"
  )

set(
  draco_points_enc_sources
  "${draco_src_root}/compression/point_cloud/algorithms/dynamic_integer_points_kd_tree_encoder.cc"
  "${draco_src_root}/compression/point_cloud/algorithms/dynamic_integer_points_kd_tree_encoder.h"
  "${draco_src_root}/compression/point_cloud/algorithms/float_points_tree_encoder.cc"
  "${draco_src_root}/compression/point_cloud/algorithms/float_points_tree_encoder.h"
  )

set(draco_metadata_sources "${draco_src_root}/metadata/geometry_metadata.cc"
                           "${draco_src_root}/metadata/geometry_metadata.h"
                           "${draco_src_root}/metadata/metadata.cc"
                           "${draco_src_root}/metadata/metadata.h")

set(draco_metadata_enc_sources "${draco_src_root}/metadata/metadata_encoder.cc"
                               "${draco_src_root}/metadata/metadata_encoder.h")

set(draco_metadata_dec_sources "${draco_src_root}/metadata/metadata_decoder.cc"
                               "${draco_src_root}/metadata/metadata_decoder.h")

set(draco_animation_sources "${draco_src_root}/animation/keyframe_animation.cc"
                            "${draco_src_root}/animation/keyframe_animation.h")

set(draco_animation_enc_sources
    "${draco_src_root}/animation/keyframe_animation_encoder.cc"
    "${draco_src_root}/animation/keyframe_animation_encoder.h")

set(draco_animation_dec_sources
    "${draco_src_root}/animation/keyframe_animation_decoder.cc"
    "${draco_src_root}/animation/keyframe_animation_decoder.h")

set(draco_js_dec_sources
    "${draco_src_root}/javascript/emscripten/decoder_webidl_wrapper.cc"
    "${draco_src_root}/javascript/emscripten/draco_decoder_glue_wrapper.cc")

set(draco_js_enc_sources
    "${draco_src_root}/javascript/emscripten/draco_encoder_glue_wrapper.cc"
    "${draco_src_root}/javascript/emscripten/encoder_webidl_wrapper.cc")

set(
  draco_animation_js_dec_sources
  "${draco_src_root}/javascript/emscripten/animation_decoder_webidl_wrapper.cc"
  "${draco_src_root}/javascript/emscripten/draco_animation_decoder_glue_wrapper.cc"
  )

set(
  draco_animation_js_enc_sources
  "${draco_src_root}/javascript/emscripten/animation_encoder_webidl_wrapper.cc"
  "${draco_src_root}/javascript/emscripten/draco_animation_encoder_glue_wrapper.cc"
  )

set(draco_unity_plug_sources "${draco_src_root}/unity/draco_unity_plugin.cc"
                             "${draco_src_root}/unity/draco_unity_plugin.h")

set(draco_maya_plug_sources "${draco_src_root}/maya/draco_maya_plugin.cc"
                            "${draco_src_root}/maya/draco_maya_plugin.h")

include_directories("${draco_root}/src" "${draco_build_dir}")

# Standard Draco libs, encoder and decoder. Object collections that mirror the
# Draco directory structure.
if(BUILD_SHARED_LIBS)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()
add_library(draco_attributes OBJECT ${draco_attributes_sources})
add_library(draco_compression_attributes_dec
            OBJECT ${draco_compression_attributes_dec_sources})
add_library(draco_compression_attributes_enc
            OBJECT ${draco_compression_attributes_enc_sources})
add_library(draco_compression_attributes_pred_schemes_dec
            OBJECT ${draco_compression_attributes_pred_schemes_dec_sources})
add_library(draco_compression_attributes_pred_schemes_enc
            OBJECT ${draco_compression_attributes_pred_schemes_enc_sources})
add_library(draco_compression_bit_coders
            OBJECT ${draco_compression_bit_coders_sources})
add_library(draco_enc_config OBJECT ${draco_enc_config_sources})
add_library(draco_dec_config OBJECT ${draco_dec_config_sources})
add_library(draco_compression_decode OBJECT
                                      ${draco_compression_decode_sources})
add_library(draco_compression_encode OBJECT
                                      ${draco_compression_encode_sources})
add_library(draco_compression_entropy OBJECT
                                      ${draco_compression_entropy_sources})
add_library(draco_compression_mesh_traverser
            OBJECT ${draco_compression_mesh_traverser_sources})
add_library(draco_compression_mesh_dec OBJECT
                                        ${draco_compression_mesh_dec_sources})
add_library(draco_compression_mesh_enc OBJECT
                                        ${draco_compression_mesh_enc_sources})
add_library(draco_compression_point_cloud_dec
            OBJECT ${draco_compression_point_cloud_dec_sources})
add_library(draco_compression_point_cloud_enc
            OBJECT ${draco_compression_point_cloud_enc_sources})
add_library(draco_core OBJECT ${draco_core_sources})
add_library(draco_io OBJECT ${draco_io_sources})
add_library(draco_mesh OBJECT ${draco_mesh_sources})
add_library(draco_metadata_dec OBJECT ${draco_metadata_dec_sources})
add_library(draco_metadata_enc OBJECT ${draco_metadata_enc_sources})
add_library(draco_metadata OBJECT ${draco_metadata_sources})
add_library(draco_animation_dec OBJECT ${draco_animation_dec_sources})
add_library(draco_animation_enc OBJECT ${draco_animation_enc_sources})
add_library(draco_animation OBJECT ${draco_animation_sources})
add_library(draco_point_cloud OBJECT ${draco_point_cloud_sources})
add_library(draco_points_dec OBJECT ${draco_points_common_sources}
                              ${draco_points_dec_sources})
add_library(draco_points_enc OBJECT ${draco_points_common_sources}
                              ${draco_points_enc_sources})

# Library targets that consume the object collections.
add_library(dracodec
            dummy.c     # needed for Xcode
            $<TARGET_OBJECTS:draco_attributes>
            $<TARGET_OBJECTS:draco_compression_attributes_dec>
            $<TARGET_OBJECTS:draco_compression_decode>
            $<TARGET_OBJECTS:draco_compression_entropy>
            $<TARGET_OBJECTS:draco_compression_mesh_dec>
            $<TARGET_OBJECTS:draco_compression_point_cloud_dec>
            $<TARGET_OBJECTS:draco_compression_bit_coders>
            $<TARGET_OBJECTS:draco_core>
            $<TARGET_OBJECTS:draco_dec_config>
            $<TARGET_OBJECTS:draco_io>
            $<TARGET_OBJECTS:draco_mesh>
            $<TARGET_OBJECTS:draco_metadata>
            $<TARGET_OBJECTS:draco_metadata_dec>
            $<TARGET_OBJECTS:draco_animation>
            $<TARGET_OBJECTS:draco_animation_dec>
            $<TARGET_OBJECTS:draco_point_cloud>
            $<TARGET_OBJECTS:draco_points_dec>)

list(APPEND draco_header_only_targets
      draco_compression_attributes_pred_schemes_dec
      draco_compression_mesh_traverser draco_dec_config draco_enc_config)

# For now, enable deduplication for both encoder and decoder.
# TODO(ostava): Support for disabling attribute deduplication for the C++
# decoder is planned in future releases.
draco_enable_feature(FEATURE DRACO_ATTRIBUTE_INDICES_DEDUPLICATION_SUPPORTED)
draco_enable_feature(FEATURE DRACO_ATTRIBUTE_VALUES_DEDUPLICATION_SUPPORTED)

if(BUILD_SHARED_LIBS)
  set_target_properties(dracodec PROPERTIES SOVERSION 1)
endif()

extract_version_string("${draco_src_root}/core/draco_version.h" draco_version)

if(NOT MSVC)
  # Generate draco.pc.
  set(prefix "${CMAKE_INSTALL_PREFIX}")
  set(pkgconfig_file "${draco_build_dir}/draco.pc")
  file(WRITE "${pkgconfig_file}" "# draco pkg-config.\n")
  file(APPEND "${pkgconfig_file}" "prefix=${prefix}\n")
  file(APPEND "${pkgconfig_file}" "exec_prefix=${prefix}/bin\n")
  file(APPEND "${pkgconfig_file}" "libdir=${prefix}/lib\n")
  file(APPEND "${pkgconfig_file}" "includedir=${prefix}/include\n\n")
  file(APPEND "${pkgconfig_file}" "Name: Draco\n")
  file(APPEND "${pkgconfig_file}" "Description: 3D Compression.\n")
  file(APPEND "${pkgconfig_file}" "Version: ${draco_version}\n")
  file(APPEND "${pkgconfig_file}" "Requires:\n")
  file(APPEND "${pkgconfig_file}" "Conflicts:\n")
  file(APPEND "${pkgconfig_file}" "Libs: -L${prefix}/lib -ldraco\n")
  file(APPEND "${pkgconfig_file}" "Cflags: -I${prefix}/include -std=c++11\n")
endif()

# Generate a CMake config file.
set(draco_include_install_dir "include/draco")
set(draco_lib_install_dir "lib")
set(draco_script_install_dir
    "${CMAKE_INSTALL_PREFIX}/${draco_lib_install_dir}/draco/cmake")
set(draco_config_in_file "${draco_root}/cmake/DracoConfig.cmake")
set(draco_config_out_file "${draco_build_dir}/DracoConfig.cmake")

configure_package_config_file("${draco_config_in_file}"
                              "${draco_config_out_file}"
                              NO_CHECK_REQUIRED_COMPONENTS_MACRO
                              INSTALL_DESTINATION
                              "${draco_lib_install_dir}/draco/cmake" PATH_VARS
                              draco_include_install_dir draco_lib_install_dir)
write_basic_package_version_file("${draco_build_dir}/DracoConfigVersion.cmake"
                                  VERSION ${draco_version}
                                  COMPATIBILITY SameMajorVersion)

# Read in the config file and rewrite it with a version string included.
file(STRINGS "${draco_config_out_file}" draco_config_strings)
file(WRITE "${draco_config_out_file}"
      "set(draco_VERSION \"${draco_version}\")\n")
foreach(config_string ${draco_config_strings})
  file(APPEND "${draco_config_out_file}" "${config_string}\n")
endforeach()

# Add install rules for CMake helper scripts.
set(draco_cmake_helper_scripts "${draco_root}/cmake/FindDraco.cmake"
                                "${draco_build_dir}/DracoConfig.cmake"
                                "${draco_build_dir}/DracoConfigVersion.cmake")

# Some generators complain about unknown link language for header only
# targets. Silence the harmless warnings/errors with some dummy source files.
foreach(target ${draco_header_only_targets})
  add_dummy_source_file_to_target("${target}" "cc")
  set_target_properties(${target} PROPERTIES FOLDER ThirdParty/draco)
endforeach()

# Generate $draco_build_dir/draco/draco_features.h.
draco_generate_features_h()

target_include_directories(dracodec
    PUBLIC "${draco_build_dir}"
    PUBLIC "${draco_root}/src")

install(TARGETS dracodec ARCHIVE DESTINATION lib/${DIST_DIR})

# Add Draco Targets to the ThirdParty/draco folder
set_target_properties(draco_attributes                               PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_attributes_dec               PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_attributes_enc               PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_attributes_pred_schemes_dec  PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_attributes_pred_schemes_enc  PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_bit_coders                   PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_enc_config                               PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_dec_config                               PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_decode                       PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_encode                       PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_entropy                      PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_mesh_traverser               PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_mesh_dec                     PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_mesh_enc                     PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_point_cloud_dec              PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_compression_point_cloud_enc              PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_core                                     PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_io                                       PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_mesh                                     PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_metadata_dec                             PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_metadata_enc                             PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_metadata                                 PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_animation_dec                            PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_animation_enc                            PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_animation                                PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_point_cloud                              PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_points_dec                               PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(draco_points_enc                               PROPERTIES FOLDER ThirdParty/draco)
set_target_properties(dracodec                                       PROPERTIES FOLDER ThirdParty/draco)
