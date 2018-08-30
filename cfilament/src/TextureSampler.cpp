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

#include <filament/TextureSampler.h>

#include "API.h"

using namespace filament;

FTextureSampler Filament_TextureSampler_Create(FSamplerParams *params) {
  TextureSampler sampler;
  sampler.setMinFilter(params->filterMin);
  sampler.setMagFilter(params->filterMag);
  sampler.setWrapModeR(params->wrapR);
  sampler.setWrapModeS(params->wrapS);
  sampler.setWrapModeT(params->wrapT);
  sampler.setAnisotropy(params->anisotropy);
  sampler.setCompareMode(params->compareMode, params->compareFunc);
  return sampler.getSamplerParams().u;
}

void Filament_TextureSampler_GetParams(FTextureSampler sampler, FSamplerParams *paramsOut) {
  static_assert(sizeof(TextureSampler) == sizeof(FTextureSampler), "TextureSampler not convertible to uint32_t");

  // Internally the sampler is a 32-bit int, but it's inaccessible to us
  TextureSampler decomposedSampler;
  *reinterpret_cast<uint32_t*>(&decomposedSampler) = sampler;

  paramsOut->filterMin = decomposedSampler.getMinFilter();
  paramsOut->filterMag = decomposedSampler.getMagFilter();
  paramsOut->wrapR = decomposedSampler.getWrapModeR();
  paramsOut->wrapS = decomposedSampler.getWrapModeS();
  paramsOut->wrapT = decomposedSampler.getWrapModeT();
  paramsOut->anisotropy = decomposedSampler.getAnisotropy();
  paramsOut->compareMode = decomposedSampler.getCompareMode();
  paramsOut->compareFunc = decomposedSampler.getCompareFunc();
}
