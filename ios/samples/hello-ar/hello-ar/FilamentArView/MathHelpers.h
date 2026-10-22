/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef MathHelpers_h
#define MathHelpers_h

#define FILAMENT_MAT4F_FROM_SIMD(m) (filament::math::mat4f \
        (m.columns[0][0], m.columns[0][1], m.columns[0][2], m.columns[0][3], \
         m.columns[1][0], m.columns[1][1], m.columns[1][2], m.columns[1][3], \
         m.columns[2][0], m.columns[2][1], m.columns[2][2], m.columns[2][3], \
         m.columns[3][0], m.columns[3][1], m.columns[3][2], m.columns[3][3]))

#define FILAMENT_MAT4_FROM_SIMD(m) (filament::math::mat4 \
        (m.columns[0][0], m.columns[0][1], m.columns[0][2], m.columns[0][3], \
         m.columns[1][0], m.columns[1][1], m.columns[1][2], m.columns[1][3], \
         m.columns[2][0], m.columns[2][1], m.columns[2][2], m.columns[2][3], \
         m.columns[3][0], m.columns[3][1], m.columns[3][2], m.columns[3][3]))

#define SIMD_FLOAT4X4_FROM_FILAMENT(m) (simd_matrix( \
        simd_make_float4(m[0][0], m[0][1], m[0][2], m[0][3]), \
        simd_make_float4(m[1][0], m[1][1], m[1][2], m[1][3]), \
        simd_make_float4(m[2][0], m[2][1], m[2][2], m[2][3]), \
        simd_make_float4(m[3][0], m[3][1], m[3][2], m[3][3])))

#endif /* MathHelpers_h */
