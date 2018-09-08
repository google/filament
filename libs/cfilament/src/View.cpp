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

#include <filament/View.h>

#include "API.h"

using namespace filament;

void Filament_View_SetName(View *view, const char *name) {
    view->setName(name);
}

const char *Filament_View_GetName(View *view) {
    return view->getName();
}

void Filament_View_SetScene(View *view, Scene *scene) {
    view->setScene(scene);
}

void Filament_View_SetCamera(View *view, Camera *camera) {
    view->setCamera(camera);
}

void Filament_View_SetViewport(View *view, int left, int bottom,
        uint32_t width, uint32_t height) {
    view->setViewport({left, bottom, width, height});
}

void Filament_View_SetClearColor(View *view, LinearColorA color) {
    view->setClearColor(color);
}

void Filament_View_GetClearColor(View *view, LinearColorA *colorOut) {
    *colorOut = view->getClearColor();
}

void Filament_View_SetClearTargets(View *view, FBool color, FBool depth,
        FBool stencil) {
    view->setClearTargets(color, depth, stencil);
}

void Filament_View_SetVisibleLayers(View *view, uint8_t select, uint8_t value) {
    view->setVisibleLayers(select, value);
}

void Filament_View_SetShadowsEnabled(View *view, FBool enabled) {
    view->setShadowsEnabled(enabled);
}

void Filament_View_SetSampleCount(View *view, uint8_t count) {
    view->setSampleCount(count);
}

uint8_t Filament_View_GetSampleCount(View *view) {
    return view->getSampleCount();
}

void Filament_View_SetAntiAliasing(View *view, View::AntiAliasing type) {
    view->setAntiAliasing(type);
}

View::AntiAliasing Filament_View_GetAntiAliasing(View *view) {
    return view->getAntiAliasing();
}

void Filament_View_SetDynamicResolutionOptions(View *view, FDynamicResolutionOptions optionsIn) {
    View::DynamicResolutionOptions options;
    options.enabled = optionsIn.enabled;
    options.homogeneousScaling = optionsIn.homogeneousScaling;
    options.targetFrameTimeMilli = optionsIn.targetFrameTimeMilli;
    options.headRoomRatio = optionsIn.headRoomRatio;
    options.scaleRate = optionsIn.scaleRate;
    options.minScale = math::float2{optionsIn.minScale};
    options.maxScale = math::float2{optionsIn.maxScale};
    options.history = optionsIn.history;
    return view->setDynamicResolutionOptions(options);
}

void Filament_View_GetDynamicResolutionOptions(View *view, FDynamicResolutionOptions *optionsOut) {
    auto options = view->getDynamicResolutionOptions();
    optionsOut->enabled = options.enabled;
    optionsOut->homogeneousScaling = options.homogeneousScaling;
    optionsOut->targetFrameTimeMilli = options.targetFrameTimeMilli;
    optionsOut->headRoomRatio = options.headRoomRatio;
    optionsOut->scaleRate = options.scaleRate;
    optionsOut->minScale = options.minScale.x;
    optionsOut->maxScale = options.maxScale.x;
    optionsOut->history = options.history;
}

void Filament_View_SetDynamicLightingOptions(View *view, float zLightNear, float zLightFar) {
    view->setDynamicLightingOptions(zLightNear, zLightFar);
}

void Filament_View_SetDepthPrepass(View *view, View::DepthPrepass value) {
    view->setDepthPrepass(value);
}
