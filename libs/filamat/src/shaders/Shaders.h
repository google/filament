/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMANT_SHADERS_H
#define TNT_FILAMANT_SHADERS_H

namespace filament {
namespace shaders {

extern const char brdf_fs[];
extern const char common_getters_fs[];
extern const char common_graphics_fs[];
extern const char common_lighting_fs[];
extern const char common_material_fs[];
extern const char common_material_vs[];
extern const char common_math_fs[];
extern const char common_types_fs[];
extern const char conversion_functions_fs[];
extern const char depth_main_fs[];
extern const char depth_main_vs[];
extern const char dithering_fs[];
extern const char fxaa_fs[];
extern const char getters_fs[];
extern const char getters_vs[];
extern const char light_directional_fs[];
extern const char light_indirect_fs[];
extern const char light_punctual_fs[];
extern const char main_fs[];
extern const char main_vs[];
extern const char post_process_fs[];
extern const char post_process_vs[];
extern const char shading_lit_fs[];
extern const char shading_model_cloth_fs[];
extern const char shading_model_standard_fs[];
extern const char shading_model_subsurface_fs[];
extern const char shading_parameters_fs[];
extern const char shading_unlit_fs[];
extern const char shadowing_fs[];
extern const char shadowing_vs[];
extern const char tone_mapping_fs[];
extern const char variables_fs[];
extern const char variables_vs[];

}
}

#endif //TNT_FILAMAT_SHADERS_H
