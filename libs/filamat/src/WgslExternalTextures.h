/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMAT_WGSLEXTERNALTEXTURES_H
#define TNT_FILAMAT_WGSLEXTERNALTEXTURES_H

#include <utils/CString.h>

#include <vector>

#include <stdint.h>

namespace tint::core::ir {
class Module;
} // namespace tint::core::ir

namespace filamat {

/**
 * A WGSL binding point (bind group and binding index) of a texture that must be declared as
 * `texture_external`.
 */
struct WgslBindingPoint {
    uint32_t group;
    uint32_t binding;
};

/**
 * Retypes the given 2D float textures of a Tint IR module to `texture_external`.
 *
 * Filament's SPIR-V has no notion of an external sampler: `SAMPLER_EXTERNAL` is emitted as a
 * regular `sampler2D`, which Tint reads as a `texture_2d<f32>`. This pass restores the external
 * type, in the same way `CodeGenerator::fixupExternalSamplers()` restores `samplerExternalOES` for
 * GLSL ES.
 *
 * The new type is propagated through loads, lets and user function calls. A function that
 * receives an external texture as an argument is cloned with the matching parameter retyped, and
 * the original is removed if it is no longer called. Texture builtins are then rewritten to their
 * `texture_external` equivalents:
 *
 *  - textureSample, textureSampleBias, textureSampleLevel and textureSampleGrad become
 *    textureSampleBaseClampToEdge. External textures have a single mip level, so the bias, level
 *    and gradients have no effect.
 *  - textureLoad and textureDimensions drop their level argument.
 *
 * Any other use of an external texture (e.g. textureGather, or a sample with an offset) is an
 * error.
 *
 * @param module    The module to transform. It is left in an unspecified state on failure.
 * @param bindings  WGSL binding points of the textures to retype. Binding points that don't match
 *                  any texture of the module are ignored.
 * @param error     Receives a description of the failure when the function returns false.
 * @return true on success.
 */
bool retypeExternalTextures(tint::core::ir::Module& module,
        std::vector<WgslBindingPoint> const& bindings, utils::CString* error);

} // namespace filamat

#endif // TNT_FILAMAT_WGSLEXTERNALTEXTURES_H
