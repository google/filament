/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMAT_MATERIAL_CHUNK_TYPES_H
#define TNT_FILAMAT_MATERIAL_CHUNK_TYPES_H

#include <stdint.h>

#include <utils/compiler.h>

namespace filamat {

// Pack an eight character string into a 64 bit integer.
constexpr inline uint64_t charTo64bitNum(const char str[9]) noexcept {
    return
        (  (static_cast<uint64_t >(str[0]) << 56))
        | ((static_cast<uint64_t >(str[1]) << 48) & 0x00FF000000000000U)
        | ((static_cast<uint64_t >(str[2]) << 40) & 0x0000FF0000000000U)
        | ((static_cast<uint64_t >(str[3]) << 32) & 0x000000FF00000000U)
        | ((static_cast<uint64_t >(str[4]) << 24) & 0x00000000FF000000U)
        | ((static_cast<uint64_t >(str[5]) << 16) & 0x0000000000FF0000U)
        | ((static_cast<uint64_t >(str[6]) <<  8) & 0x000000000000FF00U)
        | ( static_cast<uint64_t >(str[7])        & 0x00000000000000FFU);
}

enum UTILS_PUBLIC ChunkType : uint64_t {
    Unknown  = charTo64bitNum("UNKNOWN "),
    MaterialUib = charTo64bitNum("MAT_UIB "),
    MaterialSib = charTo64bitNum("MAT_SIB "),
    MaterialSubpass = charTo64bitNum("MAT_SUB "),
    MaterialGlsl = charTo64bitNum("MAT_GLSL"),
    MaterialEssl1 = charTo64bitNum("MAT_ESS1"),
    MaterialSpirv = charTo64bitNum("MAT_SPIR"),
    MaterialMetal = charTo64bitNum("MAT_METL"),
    MaterialMetalLibrary = charTo64bitNum("MAT_MLIB"),
    MaterialShaderModels = charTo64bitNum("MAT_SMDL"),
    MaterialBindingUniformInfo = charTo64bitNum("MAT_UFRM"),
    MaterialAttributeInfo = charTo64bitNum("MAT_ATTR"),
    MaterialDescriptorBindingsInfo = charTo64bitNum("MAT_DBDI"),
    MaterialDescriptorSetLayoutInfo = charTo64bitNum("MAT_DSLI"),
    MaterialProperties = charTo64bitNum("MAT_PROP"),
    MaterialConstants = charTo64bitNum("MAT_CONS"),
    MaterialPushConstants = charTo64bitNum("MAT_PCON"),

    MaterialName = charTo64bitNum("MAT_NAME"),
    MaterialVersion = charTo64bitNum("MAT_VERS"),
    MaterialCacheId = charTo64bitNum("MAT_UUID"),
    MaterialFeatureLevel = charTo64bitNum("MAT_FEAT"),
    MaterialShading = charTo64bitNum("MAT_SHAD"),
    MaterialBlendingMode = charTo64bitNum("MAT_BLEN"),
    MaterialBlendFunction = charTo64bitNum("MAT_BLFN"),
    MaterialTransparencyMode = charTo64bitNum("MAT_TRMD"),
    MaterialMaskThreshold = charTo64bitNum("MAT_THRS"),
    MaterialShadowMultiplier = charTo64bitNum("MAT_SHML"),
    MaterialSpecularAntiAliasing = charTo64bitNum("MAT_SPAA"),
    MaterialSpecularAntiAliasingVariance = charTo64bitNum("MAT_SVAR"),
    MaterialSpecularAntiAliasingThreshold = charTo64bitNum("MAT_STHR"),
    MaterialClearCoatIorChange = charTo64bitNum("MAT_CIOR"),
    MaterialDomain = charTo64bitNum("MAT_DOMN"),
    MaterialVariantFilterMask = charTo64bitNum("MAT_VFLT"),
    MaterialRefraction = charTo64bitNum("MAT_REFM"),
    MaterialRefractionType = charTo64bitNum("MAT_REFT"),
    MaterialReflectionMode = charTo64bitNum("MAT_REFL"),

    MaterialRequiredAttributes = charTo64bitNum("MAT_REQA"),
    MaterialDoubleSidedSet = charTo64bitNum("MAT_DOSS"),
    MaterialDoubleSided = charTo64bitNum("MAT_DOSI"),

    MaterialColorWrite = charTo64bitNum("MAT_CWRIT"),
    MaterialDepthWriteSet = charTo64bitNum("MAT_DEWS"),
    MaterialDepthWrite = charTo64bitNum("MAT_DWRIT"),
    MaterialDepthTest = charTo64bitNum("MAT_DTEST"),
    MaterialInstanced = charTo64bitNum("MAT_INSTA"),
    MaterialCullingMode = charTo64bitNum("MAT_CUMO"),
    MaterialAlphaToCoverageSet = charTo64bitNum("MAT_A2CS"),
    MaterialAlphaToCoverage = charTo64bitNum("MAT_A2CO"),

    MaterialHasCustomDepthShader =charTo64bitNum("MAT_CSDP"),

    MaterialVertexDomain = charTo64bitNum("MAT_VEDO"),
    MaterialInterpolation = charTo64bitNum("MAT_INTR"),

    DictionaryText = charTo64bitNum("DIC_TEXT"),
    DictionarySpirv = charTo64bitNum("DIC_SPIR"),
    DictionaryMetalLibrary = charTo64bitNum("DIC_MLIB"),
};

} // namespace filamat

#endif // TNT_FILAMAT_MATERIAL_CHUNK_TYPES_H
