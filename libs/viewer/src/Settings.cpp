/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <viewer/Settings.h>

#include "jsonParseUtils.h"
#include "Settings_generated.h"

#include <filament/Engine.h>
#include <filament/Camera.h>
#include <filament/Renderer.h>
#include <filament/Skybox.h>

#include <utils/Log.h>

#include <math/mat3.h>

#include <sstream>
#include <string>

#include <assert.h>
#include <string.h>

using namespace utils;

namespace filament::viewer {

std::string_view to_string(color::ColorSpace const& colorspace) noexcept {
    using namespace color;
    if (colorspace == Rec709-Linear-D65) {
        return "Rec709-Linear-D65";
    }
    if (colorspace == Rec709-sRGB-D65) {
        return "Rec709-sRGB-D65";
    }
    return "unknown";
}

// Skips over an unused token.
int parse(jsmntok_t const* tokens, int i) {
    int end = i + 1;
    while (i < end) {
        switch (tokens[i].type) {
            case JSMN_OBJECT:
                end += tokens[i].size * 2; break;
            case JSMN_ARRAY:
                end += tokens[i].size; break;
            case JSMN_PRIMITIVE:
            case JSMN_STRING:
                break;
            default: return -1;
        }
        i++;
    }
    return i;
}


using CGQL = filament::ColorGrading::QualityLevel;

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, CGQL* out) {
    if (0 == compare(tokens[i], jsonChunk, "LOW")) { *out = CGQL::LOW; }
    else if (0 == compare(tokens[i], jsonChunk, "MEDIUM")) { *out = CGQL::MEDIUM; }
    else if (0 == compare(tokens[i], jsonChunk, "HIGH")) { *out = CGQL::HIGH; }
    else if (0 == compare(tokens[i], jsonChunk, "ULTRA")) { *out = CGQL::ULTRA; }
    else {
        slog.w << "Invalid QualityLevel: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, ToneMapping* out) {
    if (0 == compare(tokens[i], jsonChunk, "LINEAR")) { *out = ToneMapping::LINEAR; }
    else if (0 == compare(tokens[i], jsonChunk, "ACES_LEGACY")) { *out = ToneMapping::ACES_LEGACY; }
    else if (0 == compare(tokens[i], jsonChunk, "ACES")) { *out = ToneMapping::ACES; }
    else if (0 == compare(tokens[i], jsonChunk, "FILMIC")) { *out = ToneMapping::FILMIC; }
    else if (0 == compare(tokens[i], jsonChunk, "AGX")) { *out = ToneMapping::AGX; }
    else if (0 == compare(tokens[i], jsonChunk, "GENERIC")) { *out = ToneMapping::GENERIC; }
    else if (0 == compare(tokens[i], jsonChunk, "PBR_NEUTRAL")) { *out = ToneMapping::PBR_NEUTRAL; }
    else if (0 == compare(tokens[i], jsonChunk, "GT7")) { *out = ToneMapping::GT7; }
    else if (0 == compare(tokens[i], jsonChunk, "DISPLAY_RANGE")) { *out = ToneMapping::DISPLAY_RANGE; }
    else {
        slog.w << "Invalid ToneMapping: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, color::ColorSpace* out) {
    using namespace filament::color;
    if (0 == compare(tokens[i], jsonChunk, "Rec709-Linear-D65")) { *out = Rec709-Linear-D65; }
    else if (0 == compare(tokens[i], jsonChunk, "Rec709-sRGB-D65")) { *out = Rec709-sRGB-D65; }
    else {
        slog.w << "Invalid ColorSpace: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, CameraProjection* out) {
    if (0 == compare(tokens[i], jsonChunk, "PERSPECTIVE")) {
        *out = CameraProjection::PERSPECTIVE;
    } else if (0 == compare(tokens[i], jsonChunk, "ORTHO")) {
        *out = CameraProjection::ORTHO;
    } else {
        slog.w << "Invalid CameraProjection: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}


static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, LightManager::Type* out) {
    if (0 == compare(tokens[i], jsonChunk, "SUN")) {
        *out = LightManager::Type::SUN;
    } else if (0 == compare(tokens[i], jsonChunk, "DIRECTIONAL")) {
        *out = LightManager::Type::DIRECTIONAL;
    } else if (0 == compare(tokens[i], jsonChunk, "POINT")) {
        *out = LightManager::Type::POINT;
    } else if (0 == compare(tokens[i], jsonChunk, "FOCUSED_SPOT")) {
        *out = LightManager::Type::FOCUSED_SPOT;
    } else if (0 == compare(tokens[i], jsonChunk, "SPOT")) {
        *out = LightManager::Type::SPOT;
    } else {
        slog.w << "Invalid Light Type: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, GenericToneMapperSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "contrast") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->contrast);
        } else if (compare(tok, jsonChunk, "midGrayIn") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->midGrayIn);
        } else if (compare(tok, jsonChunk, "midGrayOut") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->midGrayOut);
        } else if (compare(tok, jsonChunk, "hdrMax") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->hdrMax);
        } else {
            slog.w << "Invalid generic tone mapper key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid generic tone mapper value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, AgxToneMapper::AgxLook* out) {
    if (0 == compare(tokens[i], jsonChunk, "NONE")) { *out = AgxToneMapper::AgxLook::NONE; }
    else if (0 == compare(tokens[i], jsonChunk, "PUNCHY")) { *out = AgxToneMapper::AgxLook::PUNCHY; }
    else if (0 == compare(tokens[i], jsonChunk, "GOLDEN")) { *out = AgxToneMapper::AgxLook::GOLDEN; }
    else {
        slog.w << "Invalid AgxLook: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, AgxToneMapperSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "look") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->look);
        } else {
            slog.w << "Invalid AgX tone mapper key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid AgX tone mapper value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, ColorGradingSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "enabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
        } else if (compare(tok, jsonChunk, "colorspace") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->colorspace);
        } else if (compare(tok, jsonChunk, "quality") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->quality);
        } else if (compare(tok, jsonChunk, "toneMapping") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->toneMapping);
        } else if (compare(tok, jsonChunk, "genericToneMapper") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->genericToneMapper);
        } else if (compare(tok, jsonChunk, "agxToneMapper") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->agxToneMapper);
        } else if (compare(tok, jsonChunk, "luminanceScaling") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->luminanceScaling);
        } else if (compare(tok, jsonChunk, "gamutMapping") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->gamutMapping);
        } else if (compare(tok, jsonChunk, "exposure") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->exposure);
        } else if (compare(tok, jsonChunk, "nightAdaptation") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->nightAdaptation);
        } else if (compare(tok, jsonChunk, "temperature") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->temperature);
        } else if (compare(tok, jsonChunk, "tint") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->tint);
        } else if (compare(tok, jsonChunk, "outRed") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->outRed);
        } else if (compare(tok, jsonChunk, "outGreen") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->outGreen);
        } else if (compare(tok, jsonChunk, "outBlue") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->outBlue);
        } else if (compare(tok, jsonChunk, "shadows") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadows);
        } else if (compare(tok, jsonChunk, "midtones") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->midtones);
        } else if (compare(tok, jsonChunk, "highlights") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->highlights);
        } else if (compare(tok, jsonChunk, "ranges") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->ranges);
        } else if (compare(tok, jsonChunk, "contrast") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->contrast);
        } else if (compare(tok, jsonChunk, "vibrance") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->vibrance);
        } else if (compare(tok, jsonChunk, "saturation") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->saturation);
        } else if (compare(tok, jsonChunk, "slope") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->slope);
        } else if (compare(tok, jsonChunk, "offset") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->offset);
        } else if (compare(tok, jsonChunk, "power") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->power);
        } else if (compare(tok, jsonChunk, "gamma") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->gamma);
        } else if (compare(tok, jsonChunk, "midPoint") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->midPoint);
        } else if (compare(tok, jsonChunk, "scale") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->scale);
        } else if (compare(tok, jsonChunk, "linkedCurves") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->linkedCurves);
        } else {
            slog.w << "Invalid color grading key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid color grading value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk,
        DynamicLightingSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (0 == compare(tok, jsonChunk, "zLightNear")) {
            i = parse(tokens, i + 1, jsonChunk, &out->zLightNear);
        } else if (0 == compare(tok, jsonChunk, "zLightFar")) {
            i = parse(tokens, i + 1, jsonChunk, &out->zLightFar);
        } else {
            slog.w << "Invalid dynamic lighting key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid dynamic lighting value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, ViewSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "antiAliasing") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->antiAliasing);
        } else if (compare(tok, jsonChunk, "msaa") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->msaa);
        } else if (compare(tok, jsonChunk, "taa") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->taa);
        } else if (compare(tok, jsonChunk, "dsr") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->dsr);
        } else if (compare(tok, jsonChunk, "colorGrading") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->colorGrading);
        } else if (compare(tok, jsonChunk, "ssao") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->ssao);
        } else if (compare(tok, jsonChunk, "screenSpaceReflections") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->screenSpaceReflections);
        } else if (compare(tok, jsonChunk, "bloom") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->bloom);
        } else if (compare(tok, jsonChunk, "fog") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->fog);
        } else if (compare(tok, jsonChunk, "dof") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->dof);
        } else if (compare(tok, jsonChunk, "vignette") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->vignette);
        } else if (compare(tok, jsonChunk, "dithering") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->dithering);
        } else if (compare(tok, jsonChunk, "renderQuality") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->renderQuality);
        } else if (compare(tok, jsonChunk, "dynamicLighting") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->dynamicLighting);
        } else if (compare(tok, jsonChunk, "shadowType") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadowType);
        } else if (compare(tok, jsonChunk, "guardBand") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->guardBand);
        } else if (compare(tok, jsonChunk, "vsmShadowOptions") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->vsmShadowOptions);
        } else if (compare(tok, jsonChunk, "postProcessingEnabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->postProcessingEnabled);
        } else if (compare(tok, jsonChunk, "stereoscopicOptions") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->stereoscopicOptions);
        } else if (compare(tok, jsonChunk, "blendMode") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->blendMode);
        } else if (compare(tok, jsonChunk, "stencilBufferEnabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->stencilBufferEnabled);
        } else if (compare(tok, jsonChunk, "visibleLayers") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->visibleLayers);
        } else {
            slog.w << "Invalid view setting key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid view setting value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

template <typename T>
static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, MaterialProperty<T>* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        const std::string name = STR(tok, jsonChunk);

        // Find the first unused slot, or the first slot that matches the given name.
        size_t k = 0;
        for (; k < MaterialSettings::MAX_COUNT; ++k) {
            if (out[k].name == name || out[k].name.empty()) {
                break;
            }
        }
        if (k == MaterialSettings::MAX_COUNT) {
            slog.e << "Too many material settings." << io::endl;
            return i;
        }

        out[k].name = name;
        i = parse(tokens, i + 1, jsonChunk, &out[k].value);
        if (i < 0) {
            slog.e << "Invalid materials value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, MaterialSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "scalar") == 0) {
            i = parse(tokens, i + 1, jsonChunk, out->scalar);
        } else if (compare(tok, jsonChunk, "float3") == 0) {
            i = parse(tokens, i + 1, jsonChunk, out->float3);
        } else if (compare(tok, jsonChunk, "float4") == 0) {
            i = parse(tokens, i + 1, jsonChunk, out->float4);
        } else {
            slog.w << "Invalid materials key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid materials value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk,
        LightManager::ShadowOptions::Vsm* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "elvsm") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->elvsm);
        } else if (compare(tok, jsonChunk, "blurWidth") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->blurWidth);
        } else {
            slog.w << "Invalid shadow options VSM key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid shadow options VSM value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk,
        LightManager::ShadowOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int const size = tokens[i++].size;
    math::float3 splitsVector;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "mapSize") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->mapSize);
        } else if (compare(tok, jsonChunk, "shadowCascades") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadowCascades);
        } else if (compare(tok, jsonChunk, "cascadeSplitPositions") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &splitsVector);
            out->cascadeSplitPositions[0] = splitsVector[0];
            out->cascadeSplitPositions[1] = splitsVector[1];
            out->cascadeSplitPositions[2] = splitsVector[2];
        } else if (compare(tok, jsonChunk, "constantBias") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->constantBias);
        } else if (compare(tok, jsonChunk, "normalBias") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->normalBias);
        } else if (compare(tok, jsonChunk, "shadowFar") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadowFar);
        } else if (compare(tok, jsonChunk, "shadowNearHint") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadowNearHint);
        } else if (compare(tok, jsonChunk, "shadowFarHint") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadowFarHint);
        } else if (compare(tok, jsonChunk, "stable") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->stable);
        } else if (compare(tok, jsonChunk, "lispsm") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->lispsm);
        } else if (compare(tok, jsonChunk, "polygonOffsetConstant") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->polygonOffsetConstant);
        } else if (compare(tok, jsonChunk, "polygonOffsetSlope") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->polygonOffsetSlope);
        } else if (compare(tok, jsonChunk, "screenSpaceContactShadows") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->screenSpaceContactShadows);
        } else if (compare(tok, jsonChunk, "stepCount") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->stepCount);
        } else if (compare(tok, jsonChunk, "maxShadowDistance") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->maxShadowDistance);
        } else if (compare(tok, jsonChunk, "vsm") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->vsm);
        } else if (compare(tok, jsonChunk, "shadowBulbRadius") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadowBulbRadius);
        } else if (compare(tok, jsonChunk, "transform") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->transform.xyzw);
        } else {
            slog.w << "Invalid shadow options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid shadow options value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, LightDefinition* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "type") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->type);
        } else if (compare(tok, jsonChunk, "position") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->position);
        } else if (compare(tok, jsonChunk, "direction") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->direction);
        } else if (compare(tok, jsonChunk, "color") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->color);
        } else if (compare(tok, jsonChunk, "intensity") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->intensity);
        } else if (compare(tok, jsonChunk, "falloff") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->falloff);
        } else if (compare(tok, jsonChunk, "spotInner") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->spotInner);
        } else if (compare(tok, jsonChunk, "spotOuter") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->spotOuter);
        } else if (compare(tok, jsonChunk, "spotOuter") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->spotOuter);
        } else if (compare(tok, jsonChunk, "sunHaloSize") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->sunHaloSize);
        } else if (compare(tok, jsonChunk, "sunHaloFalloff") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->sunHaloFalloff);
        } else if (compare(tok, jsonChunk, "sunAngularRadius") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->sunAngularRadius);
        } else if (compare(tok, jsonChunk, "castShadows") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->castShadows);
        } else if (compare(tok, jsonChunk, "shadowOptions") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadowOptions);
        } else {
            slog.w << "Invalid light definition key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid light definition value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk,
        std::vector<LightDefinition>* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
    int size = tokens[i++].size;
    out->resize(size);
    for (int j = 0; j < size; ++j) {
        i = parse(tokens, i, jsonChunk, &(*out)[j]);
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, CameraSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "center") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->center);
        } else if (compare(tok, jsonChunk, "lookAt") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->lookAt);
        } else if (compare(tok, jsonChunk, "up") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->up);
        } else if (compare(tok, jsonChunk, "horizontalFov") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->horizontalFov);
        } else if (compare(tok, jsonChunk, "near") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->near);
        } else if (compare(tok, jsonChunk, "far") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->far);
        } else if (compare(tok, jsonChunk, "focalLength") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->focalLength);
        } else if (compare(tok, jsonChunk, "projection") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->projection);
        } else if (compare(tok, jsonChunk, "enabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
        } else if (compare(tok, jsonChunk, "scaling") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->scaling);
        } else if (compare(tok, jsonChunk, "shift") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shift);
        } else if (compare(tok, jsonChunk, "aperture") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->aperture);
        } else if (compare(tok, jsonChunk, "shutterSpeed") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shutterSpeed);
        } else if (compare(tok, jsonChunk, "sensitivity") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->sensitivity);
        } else if (compare(tok, jsonChunk, "focusDistance") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->focusDistance);
        } else if (compare(tok, jsonChunk, "eyeOcularDistance") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->eyeOcularDistance);
        } else if (compare(tok, jsonChunk, "eyeToeIn") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->eyeToeIn);
        } else {
            slog.w << "Invalid camera options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid camera options value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, AnimationSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "enabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
        } else if (compare(tok, jsonChunk, "time") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->time);
        } else if (compare(tok, jsonChunk, "speed") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->speed);
        } else {
            slog.w << "Invalid animation options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid animation options value: '" << STR(tok, jsonChunk) << "'"
                   << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk,
        Renderer::ClearOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "clearColor") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->clearColor);
        } else if (compare(tok, jsonChunk, "clear") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->clear);
        } else if (compare(tok, jsonChunk, "discard") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->discard);
        } else if (compare(tok, jsonChunk, "clearStencil") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->clearStencil);
        } else {
            slog.w << "Invalid clear options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid clear options value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}


static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk,
        Renderer::FrameRateOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int const size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "headRoomRatio") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->headRoomRatio);
        } else if (compare(tok, jsonChunk, "history") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->history);
        } else if (compare(tok, jsonChunk, "scaleRate") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->scaleRate);
        } else {
            slog.w << "Invalid frame rate options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid frame rate options value: '" << STR(tok, jsonChunk) << "'"
                   << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, RenderSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int const size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "clearOptions") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->clearOptions);
        } else if (compare(tok, jsonChunk, "frameRateOptions") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->frameRateOptions);
        } else {
            slog.w << "Invalid render settings key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid render settings value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}


static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, LightSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int const size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "enableShadows") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enableShadows);
        } else if (compare(tok, jsonChunk, "enableSunlight") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enableSunlight);
        } else if (compare(tok, jsonChunk, "softShadowOptions") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->softShadowOptions);
        } else if (compare(tok, jsonChunk, "sunlight") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->sunlight);
        } else if (compare(tok, jsonChunk, "iblIntensity") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->iblIntensity);
        } else if (compare(tok, jsonChunk, "iblRotation") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->iblRotation);
        } else if (compare(tok, jsonChunk, "lights") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->lights);
        } else {
            slog.w << "Invalid light setting key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid light setting value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, ViewerOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int const size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "groundShadowStrength") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->groundShadowStrength);
        } else if (compare(tok, jsonChunk, "groundPlaneEnabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->groundPlaneEnabled);
        } else if (compare(tok, jsonChunk, "skyboxEnabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->skyboxEnabled);
        } else if (compare(tok, jsonChunk, "backgroundColor") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->backgroundColor);
        } else if (compare(tok, jsonChunk, "autoInstancingEnabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->autoInstancingEnabled);
        } else if (compare(tok, jsonChunk, "autoScaleEnabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->autoScaleEnabled);
        } else {
            slog.w << "Invalid viewer options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid viewer options value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, DebugOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int const size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "skipFrames") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->skipFrames);
        } else {
            slog.w << "Invalid debug options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid debug options value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, Settings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int const size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "view") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->view);
        } else if (compare(tok, jsonChunk, "material") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->material);
        } else if (compare(tok, jsonChunk, "lighting") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->lighting);
        } else if (compare(tok, jsonChunk, "viewer") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->viewer);
        } else if (compare(tok, jsonChunk, "camera") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->camera);
        } else if (compare(tok, jsonChunk, "animation") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->animation);
        } else if (compare(tok, jsonChunk, "render") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->render);
        } else if (compare(tok, jsonChunk, "debug") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->debug);
        } else {
            slog.w << "Invalid group key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid group value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

void applySettings(Engine* engine, const ViewSettings& settings, View* dest) {
    dest->setAntiAliasing(settings.antiAliasing);
    dest->setTemporalAntiAliasingOptions(settings.taa);
    dest->setMultiSampleAntiAliasingOptions(settings.msaa);
    dest->setDynamicResolutionOptions(settings.dsr);
    dest->setAmbientOcclusionOptions(settings.ssao);
    dest->setScreenSpaceReflectionsOptions(settings.screenSpaceReflections);
    dest->setBloomOptions(settings.bloom);
    dest->setFogOptions(settings.fog);
    dest->setDepthOfFieldOptions(settings.dof);
    dest->setVignetteOptions(settings.vignette);
    dest->setDithering(settings.dithering);
    dest->setRenderQuality(settings.renderQuality);
    dest->setDynamicLightingOptions(settings.dynamicLighting.zLightNear,
            settings.dynamicLighting.zLightFar);
    dest->setShadowType(settings.shadowType);
    dest->setVsmShadowOptions(settings.vsmShadowOptions);
    dest->setGuardBandOptions(settings.guardBand);
    dest->setStereoscopicOptions(settings.stereoscopicOptions);
    dest->setPostProcessingEnabled(settings.postProcessingEnabled);
    dest->setBlendMode(static_cast<View::BlendMode>(settings.blendMode));
    dest->setStencilBufferEnabled(settings.stencilBufferEnabled);
    dest->setVisibleLayers(settings.visibleLayers, settings.visibleLayers);
}

template <typename T>
static void apply(MaterialProperty<T> prop, MaterialInstance* dest) {
    if (!prop.name.empty()) {
        dest->setParameter(prop.name.c_str(), prop.value);
    }
}

void applySettings(Engine* engine, const MaterialSettings& settings, MaterialInstance* dest) {
    for (const auto& prop : settings.scalar) { apply(prop, dest); }
    for (const auto& prop : settings.float3) { apply(prop, dest); }
    for (const auto& prop : settings.float4) { apply(prop, dest); }
}

void applySettings(Engine* engine, const LightSettings& settings, IndirectLight* ibl, utils::Entity sunlight,
        const utils::Entity* sceneLights, size_t sceneLightCount, LightManager* lm, Scene* scene, View* view) {
    auto light = lm->getInstance(sunlight);
    if (light) {
        if (settings.enableSunlight) {
            scene->addEntity(sunlight);
        } else {
            scene->remove(sunlight);
        }
        lm->setIntensity(light, settings.sunlight.intensity);
        lm->setSunHaloSize(light, settings.sunlight.sunHaloSize);
        lm->setSunHaloFalloff(light, settings.sunlight.sunHaloFalloff);
        lm->setSunAngularRadius(light, settings.sunlight.sunAngularRadius);
        lm->setDirection(light, normalize(settings.sunlight.direction));
        lm->setColor(light, settings.sunlight.color);
        lm->setShadowCaster(light, settings.sunlight.castShadows && settings.enableShadows);
        lm->setShadowOptions(light, settings.sunlight.shadowOptions);
    }
    if (ibl) {
        ibl->setIntensity(settings.iblIntensity);
        ibl->setRotation(math::mat3f::rotation(settings.iblRotation, math::float3 { 0, 1, 0 }));
    }
    for (size_t i = 0; i < sceneLightCount; i++) {
        auto const li = lm->getInstance(sceneLights[i]);
        if (li) {
            // Note: We don't have individual settings for these "scene lights" (IBL maps etc)
            // in the current structure unless we map them by index or name.
            // For now, we only enforce the global shadow toggle.
            lm->setShadowCaster(li, settings.enableShadows);
            // We removed the global shadowOptions. If they need defaults, they should be set elsewhere or
            // we should keep a "defaultShadowOptions" but for now assuming individual control via automation
            // or existing defaults.
        }
    }
    view->setSoftShadowOptions(settings.softShadowOptions);
}

static LinearColor inverseTonemapSRGB(sRGBColor x) {
    return (x * -0.155f) / (x - 1.019f);
}

void applySettings(Engine* engine, const ViewerOptions& settings, Camera* camera, Skybox* skybox,
        Renderer* renderer) {
    if (renderer) {
        // we have to clear because the side-bar doesn't have a background, we cannot use
        // a skybox on the ui scene, because the ui view is always full screen.
        renderer->setClearOptions({
                .clearColor = { inverseTonemapSRGB(settings.backgroundColor), 1.0f },
                .clear = true
        });
    }
    if (skybox) {
        skybox->setLayerMask(0xff, settings.skyboxEnabled ? 0xff : 0x00);
    }
    engine->setAutomaticInstancingEnabled(settings.autoInstancingEnabled);
}

void applySettings(Engine* engine, const DebugOptions& settings, Renderer* renderer) {
    if (!renderer->getFrameToSkipCount()) {
        renderer->skipNextFrames(settings.skipFrames);
    }
}

void applySettings(Engine* engine, const CameraSettings& settings, Camera* camera,
        double aspectRatio) {
    camera->setExposure(settings.aperture, 1.0f / settings.shutterSpeed, settings.sensitivity);

    camera->setFocusDistance(settings.focusDistance);

    // Eyes are rendered from left-to-right, i.e., eye 0 is rendered to the left side of the
    // window.
    // For testing, we want to render a side-by-side layout so users can view with
    // "cross-eyed" stereo.
    // For cross-eyed stereo, Eye 0 is really the RIGHT eye, while Eye 1 is the LEFT eye.
    double const od = settings.eyeOcularDistance;
    double const toeIn = settings.eyeToeIn;
    size_t const eyeCount = engine->getConfig().stereoscopicEyeCount;
    double3 const up = double3(0.0, 1.0, 0.0);
    mat4 const rightEye = mat4::translation(double3{ od, 0.0, 0.0 }) * mat4::rotation(toeIn, up);
    mat4 const leftEye = mat4::translation(double3{ -od, 0.0, 0.0 }) * mat4::rotation(-toeIn, up);
    mat4 const modelMatrices[2] = { rightEye, leftEye };
    for (size_t i = 0; i < eyeCount; i++) {
        camera->setEyeModelMatrix(i, modelMatrices[i % 2]);
    }

    if (settings.enabled && aspectRatio > 0.0) {
        camera->lookAt(settings.center, settings.lookAt, settings.up);
        if (settings.projection == CameraProjection::PERSPECTIVE) {
            if (settings.focalLength > 0.0f) {
                camera->setLensProjection(settings.focalLength, aspectRatio, settings.near,
                        settings.far);
            } else {
                camera->setProjection(settings.horizontalFov, aspectRatio, settings.near,
                        settings.far, Camera::Fov::VERTICAL);
            }
        } else {
            // ORTHO
            double const left = -settings.scaling.x * aspectRatio;
            double const right = settings.scaling.x * aspectRatio;
            double const bottom = -settings.scaling.y;
            double const top = settings.scaling.y;
            camera->setProjection(Camera::Projection::ORTHO, left + settings.shift.x,
                    right + settings.shift.x, bottom + settings.shift.y, top + settings.shift.y,
                    settings.near, settings.far);
        }
    }
}

constexpr ToneMapper* createToneMapper(const ColorGradingSettings& settings) noexcept {
    switch (settings.toneMapping) {
        case ToneMapping::LINEAR: return new LinearToneMapper;
        case ToneMapping::ACES_LEGACY: return new ACESLegacyToneMapper;
        case ToneMapping::ACES: return new ACESToneMapper;
        case ToneMapping::FILMIC: return new FilmicToneMapper;
        case ToneMapping::AGX: return new AgxToneMapper(settings.agxToneMapper.look);
        case ToneMapping::GENERIC: return new GenericToneMapper(
                settings.genericToneMapper.contrast,
                settings.genericToneMapper.midGrayIn,
                settings.genericToneMapper.midGrayOut,
                settings.genericToneMapper.hdrMax
        );
        case ToneMapping::PBR_NEUTRAL: return new PBRNeutralToneMapper;
        case ToneMapping::GT7: return new GT7ToneMapper;
        case ToneMapping::DISPLAY_RANGE: return new DisplayRangeToneMapper;
    }
}

ColorGrading* createColorGrading(const ColorGradingSettings& settings, Engine* engine) {
    ToneMapper* toneMapper = createToneMapper(settings);
    ColorGrading *colorGrading = ColorGrading::Builder()
            .quality(settings.quality)
            .exposure(settings.exposure)
            .nightAdaptation(settings.nightAdaptation)
            .whiteBalance(settings.temperature, settings.tint)
            .channelMixer(settings.outRed, settings.outGreen, settings.outBlue)
            .shadowsMidtonesHighlights(
                    Color::toLinear(settings.shadows),
                    Color::toLinear(settings.midtones),
                    Color::toLinear(settings.highlights),
                    settings.ranges
            )
            .slopeOffsetPower(settings.slope, settings.offset, settings.power)
            .contrast(settings.contrast)
            .vibrance(settings.vibrance)
            .saturation(settings.saturation)
            .curves(settings.gamma, settings.midPoint, settings.scale)
            .toneMapper(toneMapper)
            .luminanceScaling(settings.luminanceScaling)
            .gamutMapping(settings.gamutMapping)
            .outputColorSpace(settings.colorspace)
            .build(*engine);
    delete toneMapper;
    return colorGrading;
}

static std::ostream& operator<<(std::ostream& out, CGQL in) {
    switch (in) {
        case CGQL::LOW: return out << "\"LOW\"";
        case CGQL::MEDIUM: return out << "\"MEDIUM\"";
        case CGQL::HIGH: return out << "\"HIGH\"";
        case CGQL::ULTRA: return out << "\"ULTRA\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& operator<<(std::ostream& out, CameraProjection in) {
    switch (in) {
        case CameraProjection::ORTHO: return out << "\"ORTHO\"";
        case CameraProjection::PERSPECTIVE: return out << "\"PERSPECTIVE\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& operator<<(std::ostream& out, LightManager::Type in) {
    switch (in) {
        case LightManager::Type::SUN: return out << "\"SUN\"";
        case LightManager::Type::DIRECTIONAL: return out << "\"DIRECTIONAL\"";
        case LightManager::Type::POINT: return out << "\"POINT\"";
        case LightManager::Type::FOCUSED_SPOT: return out << "\"FOCUSED_SPOT\"";
        case LightManager::Type::SPOT: return out << "\"SPOT\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& operator<<(std::ostream& out, ToneMapping in) {
    switch (in) {
        case ToneMapping::LINEAR: return out << "\"LINEAR\"";
        case ToneMapping::ACES_LEGACY: return out << "\"ACES_LEGACY\"";
        case ToneMapping::ACES: return out << "\"ACES\"";
        case ToneMapping::FILMIC: return out << "\"FILMIC\"";
        case ToneMapping::AGX: return out << "\"AGX\"";
        case ToneMapping::GENERIC: return out << "\"GENERIC\"";
        case ToneMapping::PBR_NEUTRAL: return out << "\"PBR_NEUTRAL\"";
        case ToneMapping::GT7: return out << "\"GT7\"";
        case ToneMapping::DISPLAY_RANGE: return out << "\"DISPLAY_RANGE\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& operator<<(std::ostream& out, const GenericToneMapperSettings& in) {
    return out << "{\n"
       << "\"contrast\": " << (in.contrast) << ",\n"
       << "\"midGrayIn\": " << (in.midGrayIn) << ",\n"
       << "\"midGrayOut\": " << (in.midGrayOut) << ",\n"
       << "\"hdrMax\": " << (in.hdrMax) << "\n"
       << "}";
}

static std::ostream& operator<<(std::ostream& out, AgxToneMapper::AgxLook in) {
    switch (in) {
        case AgxToneMapper::AgxLook::NONE: return out << "\"NONE\"";
        case AgxToneMapper::AgxLook::PUNCHY: return out << "\"PUNCHY\"";
        case AgxToneMapper::AgxLook::GOLDEN: return out << "\"GOLDEN\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& operator<<(std::ostream& out, const AgxToneMapperSettings& in) {
    return out << "{\n"
               << "\"look\": " << (in.look) << ",\n"
               << "}";
}

static std::ostream& operator<<(std::ostream& out, const ColorGradingSettings& in) {
    return out << "{\n"
        << "\"enabled\": " << to_string(in.enabled) << ",\n"
        << "\"colorspace\": " << to_string(in.colorspace) << ",\n"
        << "\"quality\": " << (in.quality) << ",\n"
        << "\"toneMapping\": " << (in.toneMapping) << ",\n"
        << "\"genericToneMapper\": " << (in.genericToneMapper) << ",\n"
        << "\"agxToneMapper\": " << (in.agxToneMapper) << ",\n"
        << "\"luminanceScaling\": " << to_string(in.luminanceScaling) << ",\n"
        << "\"gamutMapping\": " << to_string(in.gamutMapping) << ",\n"
        << "\"exposure\": " << (in.exposure) << ",\n"
        << "\"nightAdaptation\": " << (in.nightAdaptation) << ",\n"
        << "\"temperature\": " << (in.temperature) << ",\n"
        << "\"tint\": " << (in.tint) << ",\n"
        << "\"outRed\": " << (in.outRed) << ",\n"
        << "\"outGreen\": " << (in.outGreen) << ",\n"
        << "\"outBlue\": " << (in.outBlue) << ",\n"
        << "\"shadows\": " << (in.shadows) << ",\n"
        << "\"midtones\": " << (in.midtones) << ",\n"
        << "\"highlights\": " << (in.highlights) << ",\n"
        << "\"ranges\": " << (in.ranges) << ",\n"
        << "\"contrast\": " << (in.contrast) << ",\n"
        << "\"vibrance\": " << (in.vibrance) << ",\n"
        << "\"saturation\": " << (in.saturation) << ",\n"
        << "\"slope\": " << (in.slope) << ",\n"
        << "\"offset\": " << (in.offset) << ",\n"
        << "\"power\": " << (in.power) << ",\n"
        << "\"gamma\": " << (in.gamma) << ",\n"
        << "\"midPoint\": " << (in.midPoint) << ",\n"
        << "\"scale\": " << (in.scale) << ",\n"
        << "\"linkedCurves\": " << to_string(in.linkedCurves) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const LightManager::ShadowOptions& in) {
    const float* splits = in.cascadeSplitPositions;
    math::float3 const splitsVector = { splits[0], splits[1], splits[2] };
    return out << "{\n"
        << "\"vsm\": {\n"
        << "\"elvsm\": " << to_string(in.vsm.elvsm) << ",\n"
        << "\"blurWidth\": " << in.vsm.blurWidth << "\n"
        << "},\n"
        << "\"mapSize\": " << in.mapSize << ",\n"
        << "\"shadowCascades\": " << int(in.shadowCascades) << ",\n"
        << "\"cascadeSplitPositions\": " << splitsVector << "\n"
        << "\"constantBias\": " << in.constantBias << ",\n"
        << "\"normalBias\": " << in.normalBias << ",\n"
        << "\"shadowFar\": " << in.shadowFar << ",\n"
        << "\"shadowNearHint\": " << in.shadowNearHint << ",\n"
        << "\"shadowFarHint\": " << in.shadowFarHint << ",\n"
        << "\"stable\": " << to_string(in.stable) << ",\n"
        << "\"lispsm\": " << to_string(in.lispsm) << ",\n"
        << "\"polygonOffsetConstant\": " << in.polygonOffsetConstant << ",\n"
        << "\"polygonOffsetSlope\": " << in.polygonOffsetSlope << ",\n"
        << "\"screenSpaceContactShadows\": " << to_string(in.screenSpaceContactShadows) << ",\n"
        << "\"stepCount\": " << +in.stepCount << ",\n"
        << "\"maxShadowDistance\": " << in.maxShadowDistance << ",\n"
        << "\"shadowBulbRadius\": " << in.shadowBulbRadius << ",\n"
        << "\"transform\": " << in.transform.xyzw << ",\n"
        << "}";
}

template <typename T>
void writeJson(MaterialProperty<T> prop, std::ostream& oss) {
    if (!prop.name.empty()) {
        oss << "\"" << prop.name << "\": " << prop.value << ",\n";
    }
}

static std::ostream& operator<<(std::ostream& out, const MaterialSettings& in) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "\"scalar\": {\n";
    for (const auto& prop : in.scalar) { writeJson(prop, oss); }
    oss << "},\n";
    oss << "\"float3\": {\n";
    for (const auto& prop : in.float3) { writeJson(prop, oss); }
    oss << "},\n";
    oss << "\"float4\": {\n";
    for (const auto& prop : in.float4) { writeJson(prop, oss); }
    oss << "},\n";
    oss << "}";
    std::string result = oss.str();

    const auto replace = [&result](const std::string& s, const std::string& t) {
        std::string::size_type n = 0;
        while ((n = result.find(s, n )) != std::string::npos) {
            result.replace(n, s.size(), t);
            n += t.size();
        }
    };

    // Remove empty objects and trailing commas.
    replace("\"scalar\": {\n},\n", "");
    replace("\"float3\": {\n},\n", "");
    replace("\"float4\": {\n},\n", "");
    replace(",\n}", "\n}");
    replace("{\n}", "{}");

    return out << result;
}

static std::ostream& operator<<(std::ostream& out, const LightDefinition& in) {
    return out << "{\n"
               << "\"type\": " << in.type << ",\n"
               << "\"position\": " << in.position << ",\n"
               << "\"direction\": " << in.direction << ",\n"
               << "\"color\": " << in.color << ",\n"
               << "\"intensity\": " << in.intensity << ",\n"
               << "\"falloff\": " << in.falloff << ",\n"
               << "\"spotInner\": " << in.spotInner << ",\n"
               << "\"spotOuter\": " << in.spotOuter << ",\n"
               << "\"sunHaloSize\": " << in.sunHaloSize << ",\n"
               << "\"sunHaloFalloff\": " << in.sunHaloFalloff << ",\n"
               << "\"sunAngularRadius\": " << in.sunAngularRadius << ",\n"
               << "\"castShadows\": " << to_string(in.castShadows) << ",\n"
               << "\"shadowOptions\": " << in.shadowOptions << "\n"
               << "}";
}

static std::ostream& operator<<(std::ostream& out, const LightSettings& in) {
    out << "{\n"
        << "\"enableShadows\": " << to_string(in.enableShadows) << ",\n"
        << "\"enableSunlight\": " << to_string(in.enableSunlight) << ",\n"
        << "\"softShadowOptions\": " << (in.softShadowOptions) << ",\n"
        << "\"sunlight\": " << (in.sunlight) << ",\n"
        << "\"iblIntensity\": " << (in.iblIntensity) << ",\n"
        << "\"iblRotation\": " << (in.iblRotation) << ",\n"
        << "\"lights\": [\n";
    for (size_t i = 0; i < in.lights.size(); ++i) {
        out << in.lights[i];
        if (i < in.lights.size() - 1) {
            out << ",";
        }
        out << "\n";
    }
    return out << "]\n" << "}";
}

static std::ostream& operator<<(std::ostream& out, const CameraSettings& in) {
    return out << "{\n"
               << "\"center\": " << in.center << ",\n"
               << "\"lookAt\": " << in.lookAt << ",\n"
               << "\"up\": " << in.up << ",\n"
               << "\"horizontalFov\": " << in.horizontalFov << ",\n"
               << "\"near\": " << in.near << ",\n"
               << "\"far\": " << in.far << ",\n"
               << "\"focalLength\": " << in.focalLength << ",\n"
               << "\"aperture\": " << in.aperture << ",\n"
               << "\"shutterSpeed\": " << in.shutterSpeed << ",\n"
               << "\"sensitivity\": " << in.sensitivity << ",\n"
               << "\"focusDistance\": " << in.focusDistance << ",\n"
               << "\"eyeOcularDistance\": " << in.eyeOcularDistance << ",\n"
               << "\"eyeToeIn\": " << in.eyeToeIn << ",\n"
               << "\"projection\": " << in.projection << ",\n"
               << "\"enabled\": " << to_string(in.enabled) << ",\n"
               << "\"scaling\": " << in.scaling << ",\n"
               << "\"shift\": " << in.shift << "\n"
               << "}";
}

static std::ostream& operator<<(std::ostream& out, const ViewerOptions& in) {
    return out << "{\n"
               << "\"groundShadowStrength\": " << (in.groundShadowStrength) << ",\n"
               << "\"groundPlaneEnabled\": " << to_string(in.groundPlaneEnabled) << ",\n"
               << "\"skyboxEnabled\": " << to_string(in.skyboxEnabled) << ",\n"
               << "\"backgroundColor\": " << (in.backgroundColor) << ",\n"
               << "\"autoInstancingEnabled\": " << to_string(in.autoInstancingEnabled) << ",\n"
               << "\"autoScaleEnabled\": " << to_string(in.autoScaleEnabled) << "\n"
               << "}";
}

static std::ostream& operator<<(std::ostream& out, const DebugOptions& in) {
    return out << "{\n"
        << "\"skipFrames\": " << (in.skipFrames) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const DynamicLightingSettings& in) {
    return out << "{\n"
        << "\"zLightNear\": " << (in.zLightNear) << ",\n"
        << "\"zLightFar\": " << (in.zLightFar) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const ViewSettings& in) {
    return out << "{\n"
        << "\"antiAliasing\": " << in.antiAliasing << ",\n"
        << "\"msaa\": " << in.msaa << ",\n"
        << "\"taa\": " << in.taa << ",\n"
        << "\"dsr\": " << in.dsr << ",\n"
        << "\"colorGrading\": " << (in.colorGrading) << ",\n"
        << "\"ssao\": " << (in.ssao) << ",\n"
        << "\"screenSpaceReflections\": " << (in.screenSpaceReflections) << ",\n"
        << "\"bloom\": " << (in.bloom) << ",\n"
        << "\"fog\": " << (in.fog) << ",\n"
        << "\"dof\": " << (in.dof) << ",\n"
        << "\"vignette\": " << (in.vignette) << ",\n"
        << "\"dithering\": " << (in.dithering) << ",\n"
        << "\"renderQuality\": " << (in.renderQuality) << ",\n"
        << "\"dynamicLighting\": " << (in.dynamicLighting) << ",\n"
        << "\"shadowType\": " << (in.shadowType) << ",\n"
        << "\"vsmShadowOptions\": " << (in.vsmShadowOptions) << ",\n"
        << "\"guardBand\": " << (in.guardBand) << ",\n"
        << "\"stereoscopicOptions\": " << (in.stereoscopicOptions) << ",\n"
        << "\"postProcessingEnabled\": " << to_string(in.postProcessingEnabled) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const Settings& in) {
    return out << "{\n"
               << "\"view\": " << (in.view) << ",\n"
               << "\"material\": " << (in.material) << ",\n"
               << "\"lighting\": " << (in.lighting) << ",\n"
               << "\"viewer\": " << (in.viewer) << ",\n"
               << "\"camera\": " << (in.camera) << ",\n"
               << "\"debug\": " << (in.debug) << "}";
}

bool GenericToneMapperSettings::operator==(const GenericToneMapperSettings& rhs) const {
    static_assert(sizeof(GenericToneMapperSettings) == 16, "Please update Settings.cpp");
    return contrast == rhs.contrast &&
           midGrayIn == rhs.midGrayIn &&
           midGrayOut == rhs.midGrayOut &&
           hdrMax == rhs.hdrMax;
}

bool AgxToneMapperSettings::operator==(const AgxToneMapperSettings& rhs) const {
    static_assert(sizeof(AgxToneMapperSettings) == 1, "Please update Settings.cpp");
    return look == rhs.look;
}

bool ColorGradingSettings::operator==(const ColorGradingSettings& rhs) const {
    // If you had to fix the following codeline, then you likely also need to update the
    // implementation of operator==.
    static_assert(sizeof(ColorGradingSettings) == 312, "Please update Settings.cpp");
    return enabled == rhs.enabled &&
            colorspace == rhs.colorspace &&
            quality == rhs.quality &&
            toneMapping == rhs.toneMapping &&
            genericToneMapper == rhs.genericToneMapper &&
            agxToneMapper == rhs.agxToneMapper &&
            luminanceScaling == rhs.luminanceScaling &&
            gamutMapping == rhs.gamutMapping &&
            exposure == rhs.exposure &&
            nightAdaptation == rhs.nightAdaptation &&
            temperature == rhs.temperature &&
            tint == rhs.tint &&
            outRed == rhs.outRed &&
            outGreen == rhs.outGreen &&
            outBlue == rhs.outBlue &&
            shadows == rhs.shadows &&
            midtones == rhs.midtones &&
            highlights == rhs.highlights &&
            ranges == rhs.ranges &&
            contrast == rhs.contrast &&
            vibrance == rhs.vibrance &&
            saturation == rhs.saturation &&
            slope == rhs.slope &&
            offset == rhs.offset &&
            power == rhs.power &&
            gamma == rhs.gamma &&
            midPoint == rhs.midPoint &&
            linkedCurves == rhs.linkedCurves &&
            scale == rhs.scale;
}

class JsonSerializer::Context {
  public:
    const std::string& writeJson(const Settings& in) {
        mStringStream.str("");
        mStringStream.clear();
        mStringStream << in;
        mResultBuffer = mStringStream.str();
        return mResultBuffer;
    }
  private:
    std::ostringstream mStringStream;
    std::string mResultBuffer;
};

JsonSerializer::JsonSerializer() {
    context = new JsonSerializer::Context();
}

JsonSerializer::~JsonSerializer() {
    delete context;
}

const std::string& JsonSerializer::writeJson(const Settings& in) {
    return context->writeJson(in);
}

bool JsonSerializer::readJson(const char* jsonChunk, size_t size, Settings* out) {
    jsmn_parser parser = { 0, 0, 0 };

    int tokenCount = jsmn_parse(&parser, jsonChunk, size, nullptr, 0);
    if (tokenCount <= 0) {
        return false;
    }

    jsmntok_t* tokens = (jsmntok_t*) malloc(sizeof(jsmntok_t) * tokenCount);
    assert(tokens);

    jsmn_init(&parser);
    tokenCount = jsmn_parse(&parser, jsonChunk, size, tokens, tokenCount);

    if (tokenCount <= 0) {
        free(tokens);
        return false;
    }

    int i = parse(tokens, 0, jsonChunk, out);
    free(tokens);
    return i >= 0;
}

} // namespace filament::viewer
