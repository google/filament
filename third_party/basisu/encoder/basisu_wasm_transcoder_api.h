// File: basisu_wasm_transcoder_api.h - Transcoding API support for WASM WASI modules and Python native support.
#pragma once
#include "basisu_wasm_api_common.h"

// High-level functions

BU_WASM_EXPORT("bt_get_version")
uint32_t bt_get_version();

BU_WASM_EXPORT("bt_enable_debug_printf")
void bt_enable_debug_printf(uint32_t flag);

BU_WASM_EXPORT("bt_init")
void bt_init();

BU_WASM_EXPORT("bt_alloc")
uint64_t bt_alloc(uint64_t size);

BU_WASM_EXPORT("bt_free")
void bt_free(uint64_t ofs);

// basis_tex_format helpers

BU_WASM_EXPORT("bt_basis_tex_format_is_xuastc_ldr")
wasm_bool_t bt_basis_tex_format_is_xuastc_ldr(uint32_t basis_tex_fmt_u32);

BU_WASM_EXPORT("bt_basis_tex_format_is_astc_ldr")
wasm_bool_t bt_basis_tex_format_is_astc_ldr(uint32_t basis_tex_fmt_u32);

BU_WASM_EXPORT("bt_basis_tex_format_get_block_width")
uint32_t bt_basis_tex_format_get_block_width(uint32_t basis_tex_fmt_u32);

BU_WASM_EXPORT("bt_basis_tex_format_get_block_height")
uint32_t bt_basis_tex_format_get_block_height(uint32_t basis_tex_fmt_u32);

BU_WASM_EXPORT("bt_basis_tex_format_is_hdr")
wasm_bool_t bt_basis_tex_format_is_hdr(uint32_t basis_tex_format_u32);

BU_WASM_EXPORT("bt_basis_tex_format_is_ldr")
wasm_bool_t bt_basis_tex_format_is_ldr(uint32_t basis_tex_format_u32);

// transcoder_texture_format helpers

BU_WASM_EXPORT("bt_basis_get_bytes_per_block_or_pixel")
uint32_t bt_basis_get_bytes_per_block_or_pixel(uint32_t transcoder_texture_format_u32);

BU_WASM_EXPORT("bt_basis_transcoder_format_has_alpha")
wasm_bool_t bt_basis_transcoder_format_has_alpha(uint32_t transcoder_texture_format_u32);

BU_WASM_EXPORT("bt_basis_transcoder_format_is_hdr")
wasm_bool_t bt_basis_transcoder_format_is_hdr(uint32_t transcoder_texture_format_u32);

BU_WASM_EXPORT("bt_basis_transcoder_format_is_ldr")
wasm_bool_t bt_basis_transcoder_format_is_ldr(uint32_t transcoder_texture_format_u32);

BU_WASM_EXPORT("bt_basis_transcoder_texture_format_is_astc")
wasm_bool_t bt_basis_transcoder_texture_format_is_astc(uint32_t transcoder_texture_format_u32);

BU_WASM_EXPORT("bt_basis_transcoder_format_is_uncompressed")
wasm_bool_t bt_basis_transcoder_format_is_uncompressed(uint32_t transcoder_texture_format_u32);

BU_WASM_EXPORT("bt_basis_get_uncompressed_bytes_per_pixel")
uint32_t bt_basis_get_uncompressed_bytes_per_pixel(uint32_t transcoder_texture_format_u32);

BU_WASM_EXPORT("bt_basis_get_block_width")
uint32_t bt_basis_get_block_width(uint32_t transcoder_texture_format_u32);

BU_WASM_EXPORT("bt_basis_get_block_height")
uint32_t bt_basis_get_block_height(uint32_t transcoder_texture_format_u32);

BU_WASM_EXPORT("bt_basis_get_transcoder_texture_format_from_basis_tex_format")
uint32_t bt_basis_get_transcoder_texture_format_from_basis_tex_format(uint32_t basis_tex_format_u32);

BU_WASM_EXPORT("bt_basis_is_format_supported")
wasm_bool_t bt_basis_is_format_supported(uint32_t transcoder_texture_format_u32, uint32_t basis_tex_format_u32);

BU_WASM_EXPORT("bt_basis_compute_transcoded_image_size_in_bytes")
uint32_t bt_basis_compute_transcoded_image_size_in_bytes(uint32_t transcoder_texture_format_u32, uint32_t orig_width, uint32_t orig_height);

// Transcoding
BU_WASM_EXPORT("bt_ktx2_open")
uint64_t bt_ktx2_open(uint64_t data_mem_ofs, uint32_t data_len);

BU_WASM_EXPORT("bt_ktx2_close")
void bt_ktx2_close(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_width")
uint32_t bt_ktx2_get_width(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_height")
uint32_t bt_ktx2_get_height(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_levels")
uint32_t bt_ktx2_get_levels(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_faces")
uint32_t bt_ktx2_get_faces(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_layers")
uint32_t bt_ktx2_get_layers(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_basis_tex_format")
uint32_t bt_ktx2_get_basis_tex_format(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_is_etc1s")
wasm_bool_t bt_ktx2_is_etc1s(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_is_uastc_ldr_4x4")
wasm_bool_t bt_ktx2_is_uastc_ldr_4x4(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_is_hdr")
wasm_bool_t bt_ktx2_is_hdr(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_is_hdr_4x4")
wasm_bool_t bt_ktx2_is_hdr_4x4(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_is_hdr_6x6")
wasm_bool_t bt_ktx2_is_hdr_6x6(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_is_ldr")
wasm_bool_t bt_ktx2_is_ldr(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_is_astc_ldr")
wasm_bool_t bt_ktx2_is_astc_ldr(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_is_xuastc_ldr")
wasm_bool_t bt_ktx2_is_xuastc_ldr(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_block_width")
uint32_t bt_ktx2_get_block_width(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_block_height")
uint32_t bt_ktx2_get_block_height(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_has_alpha")
wasm_bool_t bt_ktx2_has_alpha(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_dfd_color_model")
uint32_t bt_ktx2_get_dfd_color_model(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_dfd_color_primaries")
uint32_t bt_ktx2_get_dfd_color_primaries(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_dfd_transfer_func")
uint32_t bt_ktx2_get_dfd_transfer_func(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_is_srgb")
wasm_bool_t bt_ktx2_is_srgb(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_dfd_flags")
uint32_t bt_ktx2_get_dfd_flags(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_dfd_total_samples")
uint32_t bt_ktx2_get_dfd_total_samples(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_dfd_channel_id0")
uint32_t bt_ktx2_get_dfd_channel_id0(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_dfd_channel_id1")
uint32_t bt_ktx2_get_dfd_channel_id1(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_is_video")
wasm_bool_t bt_ktx2_is_video(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_ldr_hdr_upconversion_nit_multiplier")
float bt_ktx2_get_ldr_hdr_upconversion_nit_multiplier(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_get_level_orig_width")
uint32_t bt_ktx2_get_level_orig_width(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index);

BU_WASM_EXPORT("bt_ktx2_get_level_orig_height")
uint32_t bt_ktx2_get_level_orig_height(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index);

BU_WASM_EXPORT("bt_ktx2_get_level_actual_width")
uint32_t bt_ktx2_get_level_actual_width(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index);

BU_WASM_EXPORT("bt_ktx2_get_level_actual_height")
uint32_t bt_ktx2_get_level_actual_height(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index);

BU_WASM_EXPORT("bt_ktx2_get_level_num_blocks_x")
uint32_t bt_ktx2_get_level_num_blocks_x(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index);

BU_WASM_EXPORT("bt_ktx2_get_level_num_blocks_y")
uint32_t bt_ktx2_get_level_num_blocks_y(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index);

BU_WASM_EXPORT("bt_ktx2_get_level_total_blocks")
uint32_t bt_ktx2_get_level_total_blocks(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index);

BU_WASM_EXPORT("bt_ktx2_get_level_alpha_flag")
wasm_bool_t bt_ktx2_get_level_alpha_flag(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index);

BU_WASM_EXPORT("bt_ktx2_get_level_iframe_flag")
wasm_bool_t bt_ktx2_get_level_iframe_flag(uint64_t handle, uint32_t level_index, uint32_t layer_index, uint32_t face_index);

BU_WASM_EXPORT("bt_ktx2_start_transcoding")
wasm_bool_t bt_ktx2_start_transcoding(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_create_transcode_state")
uint64_t bt_ktx2_create_transcode_state();

BU_WASM_EXPORT("bt_ktx2_destroy_transcode_state")
void bt_ktx2_destroy_transcode_state(uint64_t handle);

BU_WASM_EXPORT("bt_ktx2_transcode_image_level")
wasm_bool_t bt_ktx2_transcode_image_level(
	uint64_t ktx2_handle, // handle to KTX2 file, see bt_ktx2_open()
	uint32_t level_index, uint32_t layer_index, uint32_t face_index, // KTX2 level/layer/face to transcode
	uint64_t output_block_mem_ofs, // allocate using bt_alloc()
	uint32_t output_blocks_buf_size_in_blocks_or_pixels,
	uint32_t transcoder_texture_format_u32, // target format, TF_ETC1_RGB etc.
	uint32_t decode_flags, // DECODE_FLAGS_
	uint32_t output_row_pitch_in_blocks_or_pixels, // can be 0
	uint32_t output_rows_in_pixels, // can be 0
	int channel0, int channel1, // both default to -1
	uint64_t state_handle); // thread local state: can be 0, or bt_ktx2_create_transcode_state()

