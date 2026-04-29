// File: basisu_wasm_api.h
#pragma once
#include "basisu_wasm_api_common.h"

BU_WASM_EXPORT("bu_get_version")
uint32_t bu_get_version();

BU_WASM_EXPORT("bu_enable_debug_printf")
void bu_enable_debug_printf(uint32_t flag);

BU_WASM_EXPORT("bu_init")
void     bu_init();

BU_WASM_EXPORT("bu_alloc")
uint64_t bu_alloc(uint64_t size);

BU_WASM_EXPORT("bu_free")
void     bu_free(uint64_t ofs);

BU_WASM_EXPORT("bu_new_comp_params")
uint64_t bu_new_comp_params();

BU_WASM_EXPORT("bu_delete_comp_params")
wasm_bool_t bu_delete_comp_params(uint64_t params_ofs);

BU_WASM_EXPORT("bu_comp_params_get_comp_data_size")
uint64_t bu_comp_params_get_comp_data_size(uint64_t params_ofs);

BU_WASM_EXPORT("bu_comp_params_get_comp_data_ofs")
uint64_t bu_comp_params_get_comp_data_ofs(uint64_t params_ofs);

BU_WASM_EXPORT("bu_comp_params_clear")
wasm_bool_t bu_comp_params_clear(uint64_t params_ofs);

BU_WASM_EXPORT("bu_comp_params_set_image_rgba32")
wasm_bool_t bu_comp_params_set_image_rgba32(
    uint64_t params_ofs,
    uint32_t image_index,
    uint64_t img_data_ofs,
    uint32_t width, uint32_t height,
    uint32_t pitch_in_bytes);

BU_WASM_EXPORT("bu_comp_params_set_image_float_rgba")
wasm_bool_t bu_comp_params_set_image_float_rgba(
    uint64_t params_ofs,
    uint32_t image_index,
    uint64_t img_data_ofs,
    uint32_t width, uint32_t height,
    uint32_t pitch_in_bytes);

BU_WASM_EXPORT("bu_compress_texture")
wasm_bool_t bu_compress_texture(
    uint64_t params_ofs,
    uint32_t desired_basis_tex_format,
    int quality_level, int effort_level,
    uint64_t flags_and_quality,
    float low_level_uastc_rdo_or_dct_quality);

