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

namespace filament {
namespace viewer {

static const char* to_string(bool b) { return b ? "true" : "false"; }

// Compares a JSON string token against a C string.
int compare(jsmntok_t tok, const char* jsonChunk, const char* str) {
    size_t slen = strlen(str);
    size_t tlen = tok.end - tok.start;
    return (slen == tlen) ? strncmp(jsonChunk + tok.start, str, slen) : 128;
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

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, uint8_t* val) {
    CHECK_TOKTYPE(tokens[i], JSMN_PRIMITIVE);
    *val = strtol(jsonChunk + tokens[i].start, nullptr, 10);
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, uint16_t* val) {
    CHECK_TOKTYPE(tokens[i], JSMN_PRIMITIVE);
    *val = strtol(jsonChunk + tokens[i].start, nullptr, 10);
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, uint32_t* val) {
    CHECK_TOKTYPE(tokens[i], JSMN_PRIMITIVE);
    *val = strtol(jsonChunk + tokens[i].start, nullptr, 10);
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, int* val) {
    CHECK_TOKTYPE(tokens[i], JSMN_PRIMITIVE);
    *val = strtol(jsonChunk + tokens[i].start, nullptr, 10);
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, float* val) {
    CHECK_TOKTYPE(tokens[i], JSMN_PRIMITIVE);
    *val = strtod(jsonChunk + tokens[i].start, nullptr);
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, float* vals, int size) {
    CHECK_TOKTYPE(tokens[i], JSMN_ARRAY);
    if (tokens[i].size != size) {
        slog.w << "Expected " << size << " floats, got " << tokens[i].size << io::endl;
        return i + 1 + tokens[i].size;
    }
    ++i;
    for (int j = 0; j < size; ++j) {
        i = parse(tokens, i, jsonChunk, &vals[j]);
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, bool* val) {
    CHECK_TOKTYPE(tokens[i], JSMN_PRIMITIVE);
    if (0 == compare(tokens[i], jsonChunk, "true")) {
        *val = true;
        return i + 1;
    }
    if (0 == compare(tokens[i], jsonChunk, "false")) {
        *val = false;
        return i + 1;
    }
    return -1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, math::float3* val) {
    float values[3];
    i = parse(tokens, i, jsonChunk, values, 3);
    *val = {values[0], values[1], values[2]};
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, math::float4* val) {
    float values[4];
    i = parse(tokens, i, jsonChunk, values, 4);
    *val = {values[0], values[1], values[2], values[3]};
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, AntiAliasing* out) {
    if (0 == compare(tokens[i], jsonChunk, "NONE")) { *out = AntiAliasing::NONE; }
    else if (0 == compare(tokens[i], jsonChunk, "FXAA")) { *out = AntiAliasing::FXAA; }
    else {
        slog.w << "Invalid AntiAliasing: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

using BlendMode = BloomOptions::BlendMode;

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, BlendMode* out) {
    if (0 == compare(tokens[i], jsonChunk, "ADD")) { *out = BlendMode::ADD; }
    else if (0 == compare(tokens[i], jsonChunk, "INTERPOLATE")) { *out = BlendMode::INTERPOLATE; }
    else {
        slog.w << "Invalid BlendMode: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

using VQL = filament::View::QualityLevel;

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, VQL* out) {
    if (0 == compare(tokens[i], jsonChunk, "LOW")) { *out = VQL::LOW; }
    else if (0 == compare(tokens[i], jsonChunk, "MEDIUM")) { *out = VQL::MEDIUM; }
    else if (0 == compare(tokens[i], jsonChunk, "HIGH")) { *out = VQL::HIGH; }
    else if (0 == compare(tokens[i], jsonChunk, "ULTRA")) { *out = VQL::ULTRA; }
    else {
        slog.w << "Invalid QualityLevel: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
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
    else if (0 == compare(tokens[i], jsonChunk, "RESERVED")) { *out = ToneMapping::RESERVED; }
    else if (0 == compare(tokens[i], jsonChunk, "REINHARD")) { *out = ToneMapping::REINHARD; }
    else if (0 == compare(tokens[i], jsonChunk, "DISPLAY_RANGE")) { *out = ToneMapping::DISPLAY_RANGE; }
    else {
        slog.w << "Invalid ToneMapping: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, Dithering* out) {
    if (0 == compare(tokens[i], jsonChunk, "NONE")) { *out = Dithering::NONE; }
    else if (0 == compare(tokens[i], jsonChunk, "TEMPORAL")) { *out = Dithering::TEMPORAL; }
    else {
        slog.w << "Invalid Dithering: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, ShadowType* out) {
    if (0 == compare(tokens[i], jsonChunk, "PCF")) { *out = ShadowType::PCF; }
    else if (0 == compare(tokens[i], jsonChunk, "VSM")) { *out = ShadowType::VSM; }
    else {
        slog.w << "Invalid ShadowType: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk,
        VsmShadowOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (0 == compare(tok, jsonChunk, "anisotropy")) {
            i = parse(tokens, i + 1, jsonChunk, &out->anisotropy);
        } else if (0 == compare(tok, jsonChunk, "mipmapping")) {
            i = parse(tokens, i + 1, jsonChunk, &out->mipmapping);
        } else if (0 == compare(tok, jsonChunk, "exponent")) {
            i = parse(tokens, i + 1, jsonChunk, &out->exponent);
        } else if (0 == compare(tok, jsonChunk, "minVarianceScale")) {
            i = parse(tokens, i + 1, jsonChunk, &out->minVarianceScale);
        } else if (0 == compare(tok, jsonChunk, "lightBleedReduction")) {
            i = parse(tokens, i + 1, jsonChunk, &out->lightBleedReduction);
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

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk,
        TemporalAntiAliasingOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "filterWidth") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->filterWidth);
        } else if (compare(tok, jsonChunk, "feedback") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->feedback);
        } else if (compare(tok, jsonChunk, "enabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
        } else {
            slog.w << "Invalid taa key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid taa value: '" << STR(tok, jsonChunk) << "'" << io::endl;
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
        } else if (compare(tok, jsonChunk, "quality") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->quality);
        } else if (compare(tok, jsonChunk, "toneMapping") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->toneMapping);
        } else if (compare(tok, jsonChunk, "luminanceScaling") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->luminanceScaling);
        } else if (compare(tok, jsonChunk, "exposure") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->exposure);
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
        AmbientOcclusionOptions::Ssct* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "enabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
        } else if (compare(tok, jsonChunk, "lightConeRad") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->lightConeRad);
        } else if (compare(tok, jsonChunk, "shadowDistance") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadowDistance);
        } else if (compare(tok, jsonChunk, "contactDistanceMax") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->contactDistanceMax);
        } else if (compare(tok, jsonChunk, "intensity") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->intensity);
        } else if (compare(tok, jsonChunk, "lightDirection") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->lightDirection);
        } else if (compare(tok, jsonChunk, "depthBias") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->depthBias);
        } else if (compare(tok, jsonChunk, "depthSlopeBias") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->depthSlopeBias);
        } else if (compare(tok, jsonChunk, "sampleCount") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->sampleCount);
        } else if (compare(tok, jsonChunk, "rayCount") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->rayCount);
        } else {
            slog.w << "Invalid SSCT key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid SSCT value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk,
        AmbientOcclusionOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "radius") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->radius);
        } else if (compare(tok, jsonChunk, "power") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->power);
        } else if (compare(tok, jsonChunk, "bias") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->bias);
        } else if (compare(tok, jsonChunk, "resolution") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->resolution);
        } else if (compare(tok, jsonChunk, "intensity") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->intensity);
        } else if (compare(tok, jsonChunk, "bilateralThreshold") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->bilateralThreshold);
        } else if (compare(tok, jsonChunk, "quality") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->quality);
        } else if (compare(tok, jsonChunk, "lowPassFilter") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->lowPassFilter);
        } else if (compare(tok, jsonChunk, "upsampling") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->upsampling);
        } else if (compare(tok, jsonChunk, "enabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
        } else if (compare(tok, jsonChunk, "minHorizonAngleRad") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->minHorizonAngleRad);
        } else if (compare(tok, jsonChunk, "ssct") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->ssct);
        } else {
            slog.w << "Invalid AO key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid AO value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, BloomOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "strength") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->strength);
        } else if (compare(tok, jsonChunk, "resolution") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->resolution);
        } else if (compare(tok, jsonChunk, "anamorphism") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->anamorphism);
        } else if (compare(tok, jsonChunk, "levels") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->levels);
        } else if (compare(tok, jsonChunk, "blendMode") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->blendMode);
        } else if (compare(tok, jsonChunk, "threshold") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->threshold);
        } else if (compare(tok, jsonChunk, "enabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
        } else if (compare(tok, jsonChunk, "highlight") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->highlight);
        } else if (compare(tok, jsonChunk, "lensFlare") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->lensFlare);
        } else if (compare(tok, jsonChunk, "starburst") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->starburst);
        } else if (compare(tok, jsonChunk, "chromaticAberration") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->chromaticAberration);
        } else if (compare(tok, jsonChunk, "ghostCount") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->ghostCount);
        } else if (compare(tok, jsonChunk, "ghostSpacing") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->ghostSpacing);
        } else if (compare(tok, jsonChunk, "ghostThreshold") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->ghostThreshold);
        } else if (compare(tok, jsonChunk, "haloThickness") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->haloThickness);
        } else if (compare(tok, jsonChunk, "haloRadius") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->haloRadius);
        } else if (compare(tok, jsonChunk, "haloThreshold") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->haloThreshold);
        } else {
            slog.w << "Invalid bloom options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid bloom options value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, FogOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (0 == compare(tok, jsonChunk, "distance")) {
            i = parse(tokens, i + 1, jsonChunk, &out->distance);
        } else if (0 == compare(tok, jsonChunk, "maximumOpacity")) {
            i = parse(tokens, i + 1, jsonChunk, &out->maximumOpacity);
        } else if (0 == compare(tok, jsonChunk, "height")) {
            i = parse(tokens, i + 1, jsonChunk, &out->height);
        } else if (0 == compare(tok, jsonChunk, "heightFalloff")) {
            i = parse(tokens, i + 1, jsonChunk, &out->heightFalloff);
        } else if (0 == compare(tok, jsonChunk, "color")) {
            i = parse(tokens, i + 1, jsonChunk, &out->color);
        } else if (0 == compare(tok, jsonChunk, "density")) {
            i = parse(tokens, i + 1, jsonChunk, &out->density);
        } else if (0 == compare(tok, jsonChunk, "inScatteringStart")) {
            i = parse(tokens, i + 1, jsonChunk, &out->inScatteringStart);
        } else if (0 == compare(tok, jsonChunk, "inScatteringSize")) {
            i = parse(tokens, i + 1, jsonChunk, &out->inScatteringSize);
        } else if (0 == compare(tok, jsonChunk, "fogColorFromIbl")) {
            i = parse(tokens, i + 1, jsonChunk, &out->fogColorFromIbl);
        } else if (0 == compare(tok, jsonChunk, "enabled")) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
        } else {
            slog.w << "Invalid fog options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid fog options value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, DepthOfFieldOptions::Filter* out) {
    if (0 == compare(tokens[i], jsonChunk, "NONE")) { *out = DepthOfFieldOptions::Filter::NONE; }
    else if (0 == compare(tokens[i], jsonChunk, "MEDIAN")) { *out = DepthOfFieldOptions::Filter::MEDIAN; }
    else {
        slog.w << "Invalid DepthOfFieldOptions::Filter: '" << STR(tokens[i], jsonChunk) << "'" << io::endl;
    }
    return i + 1;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, DepthOfFieldOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (0 == compare(tok, jsonChunk, "cocScale")) {
            i = parse(tokens, i + 1, jsonChunk, &out->cocScale);
        } else if (0 == compare(tok, jsonChunk, "maxApertureDiameter")) {
            i = parse(tokens, i + 1, jsonChunk, &out->maxApertureDiameter);
        } else if (0 == compare(tok, jsonChunk, "enabled")) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
        } else if (0 == compare(tok, jsonChunk, "filter")) {
            i = parse(tokens, i + 1, jsonChunk, &out->filter);
        } else if (0 == compare(tok, jsonChunk, "nativeResolution")) {
            i = parse(tokens, i + 1, jsonChunk, &out->nativeResolution);
        } else if (0 == compare(tok, jsonChunk, "foregroundRingCount")) {
            i = parse(tokens, i + 1, jsonChunk, &out->foregroundRingCount);
        } else if (0 == compare(tok, jsonChunk, "backgroundRingCount")) {
            i = parse(tokens, i + 1, jsonChunk, &out->backgroundRingCount);
        } else if (0 == compare(tok, jsonChunk, "fastGatherRingCount")) {
            i = parse(tokens, i + 1, jsonChunk, &out->fastGatherRingCount);
        } else if (0 == compare(tok, jsonChunk, "maxForegroundCOC")) {
            i = parse(tokens, i + 1, jsonChunk, &out->maxForegroundCOC);
        } else if (0 == compare(tok, jsonChunk, "maxBackgroundCOC")) {
            i = parse(tokens, i + 1, jsonChunk, &out->maxBackgroundCOC);
        } else {
            slog.w << "Invalid dof options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid dof options value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, VignetteOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (0 == compare(tok, jsonChunk, "midPoint")) {
            i = parse(tokens, i + 1, jsonChunk, &out->midPoint);
        } else if (0 == compare(tok, jsonChunk, "roundness")) {
            i = parse(tokens, i + 1, jsonChunk, &out->roundness);
        } else if (0 == compare(tok, jsonChunk, "feather")) {
            i = parse(tokens, i + 1, jsonChunk, &out->feather);
        } else if (0 == compare(tok, jsonChunk, "color")) {
            i = parse(tokens, i + 1, jsonChunk, &out->color);
        } else if (0 == compare(tok, jsonChunk, "enabled")) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
        } else {
            slog.w << "Invalid vignette options key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid vignette options value: '" << STR(tok, jsonChunk) << "'" << io::endl;
            return i;
        }
    }
    return i;
}

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, RenderQuality* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (0 == compare(tok, jsonChunk, "hdrColorBuffer")) {
            i = parse(tokens, i + 1, jsonChunk, &out->hdrColorBuffer);
        } else {
            slog.w << "Invalid render quality key: '" << STR(tok, jsonChunk) << "'" << io::endl;
            i = parse(tokens, i + 1);
        }
        if (i < 0) {
            slog.e << "Invalid render quality value: '" << STR(tok, jsonChunk) << "'" << io::endl;
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
        if (compare(tok, jsonChunk, "sampleCount") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->sampleCount);
        } else if (compare(tok, jsonChunk, "antiAliasing") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->antiAliasing);
        } else if (compare(tok, jsonChunk, "taa") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->taa);
        } else if (compare(tok, jsonChunk, "colorGrading") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->colorGrading);
        } else if (compare(tok, jsonChunk, "ssao") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->ssao);
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
        } else if (compare(tok, jsonChunk, "vsmShadowOptions") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->vsmShadowOptions);
        } else if (compare(tok, jsonChunk, "postProcessingEnabled") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->postProcessingEnabled);
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
        if (compare(tok, jsonChunk, "msaaSamples") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->msaaSamples);
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
    int size = tokens[i++].size;
    math::float3 splitsVector;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "mapSize") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->mapSize);
        } else if (compare(tok, jsonChunk, "screenSpaceContactShadows") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->screenSpaceContactShadows);
        } else if (compare(tok, jsonChunk, "shadowCascades") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadowCascades);
        } else if (compare(tok, jsonChunk, "cascadeSplitPositions") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &splitsVector);
            out->cascadeSplitPositions[0] = splitsVector[0];
            out->cascadeSplitPositions[1] = splitsVector[1];
            out->cascadeSplitPositions[2] = splitsVector[2];
        } else if (compare(tok, jsonChunk, "vsm") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->vsm);
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

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, LightSettings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "enableShadows") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enableShadows);
        } else if (compare(tok, jsonChunk, "enableSunlight") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->enableSunlight);
        } else if (compare(tok, jsonChunk, "shadowOptions") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->shadowOptions);
        } else if (compare(tok, jsonChunk, "sunlightIntensity") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->sunlightIntensity);
        } else if (compare(tok, jsonChunk, "sunlightDirection") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->sunlightDirection);
        } else if (compare(tok, jsonChunk, "sunlightColor") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->sunlightColor);
        } else if (compare(tok, jsonChunk, "iblIntensity") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->iblIntensity);
        } else if (compare(tok, jsonChunk, "iblRotation") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->iblRotation);
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
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "cameraAperture") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->cameraAperture);
        } else if (compare(tok, jsonChunk, "cameraSpeed") == 0) {
             i = parse(tokens, i + 1, jsonChunk, &out->cameraSpeed);
        } else if (compare(tok, jsonChunk, "cameraISO") == 0) {
             i = parse(tokens, i + 1, jsonChunk, &out->cameraISO);
        } else if (compare(tok, jsonChunk, "groundShadowStrength") == 0) {
             i = parse(tokens, i + 1, jsonChunk, &out->groundShadowStrength);
        } else if (compare(tok, jsonChunk, "groundPlaneEnabled") == 0) {
             i = parse(tokens, i + 1, jsonChunk, &out->groundPlaneEnabled);
        } else if (compare(tok, jsonChunk, "skyboxEnabled") == 0) {
             i = parse(tokens, i + 1, jsonChunk, &out->skyboxEnabled);
        } else if (compare(tok, jsonChunk, "backgroundColor") == 0) {
             i = parse(tokens, i + 1, jsonChunk, &out->backgroundColor);
        } else if (compare(tok, jsonChunk, "cameraFocalLength") == 0) {
             i = parse(tokens, i + 1, jsonChunk, &out->cameraFocalLength);
        } else if (compare(tok, jsonChunk, "cameraFocusDistance") == 0) {
             i = parse(tokens, i + 1, jsonChunk, &out->cameraFocusDistance);
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

int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, Settings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
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

void applySettings(const ViewSettings& settings, View* dest) {
    dest->setSampleCount(settings.sampleCount);
    dest->setAntiAliasing(settings.antiAliasing);
    dest->setTemporalAntiAliasingOptions(settings.taa);
    dest->setAmbientOcclusionOptions(settings.ssao);
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
    dest->setPostProcessingEnabled(settings.postProcessingEnabled);
}

template <typename T>
static void apply(MaterialProperty<T> prop, MaterialInstance* dest) {
    if (!prop.name.empty()) {
        dest->setParameter(prop.name.c_str(), prop.value);
    }
}

void applySettings(const MaterialSettings& settings, MaterialInstance* dest) {
    for (auto prop : settings.scalar) { apply(prop, dest); }
    for (auto prop : settings.float3) { apply(prop, dest); }
    for (auto prop : settings.float4) { apply(prop, dest); }
}

void applySettings(const LightSettings& settings, IndirectLight* ibl, utils::Entity sunlight,
        utils::Entity* sceneLights, size_t sceneLightCount, LightManager* lm, Scene* scene) {
    auto light = lm->getInstance(sunlight);
    if (light) {
        if (settings.enableSunlight) {
            scene->addEntity(sunlight);
        } else {
            scene->remove(sunlight);
        }
        lm->setIntensity(light, settings.sunlightIntensity);
        lm->setDirection(light, normalize(settings.sunlightDirection));
        lm->setColor(light, settings.sunlightColor);
        lm->setShadowCaster(light, settings.enableShadows);
        lm->setShadowOptions(light, settings.shadowOptions);
    }
    if (ibl) {
        ibl->setIntensity(settings.iblIntensity);
        ibl->setRotation(math::mat3f::rotation(settings.iblRotation, math::float3 { 0, 1, 0 }));
    }
    for (size_t i = 0; i < sceneLightCount; i++) {
        auto light = lm->getInstance(sceneLights[i]);
        if (lm->isSpotLight(light)) {
            lm->setShadowCaster(light, settings.enableShadows);
        }
        lm->setShadowOptions(light, settings.shadowOptions);
    }
}

static LinearColor inverseTonemapSRGB(sRGBColor x) {
    return (x * -0.155) / (x - 1.019);
}

void applySettings(const ViewerOptions& settings, Camera* camera, Skybox* skybox,
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
    if (camera) {
        camera->setExposure(
                settings.cameraAperture,
                1.0f / settings.cameraSpeed,
                settings.cameraISO);

        camera->setFocusDistance(settings.cameraFocusDistance);
    }
}

ColorGrading* createColorGrading(const ColorGradingSettings& settings, Engine* engine) {
    return ColorGrading::Builder()
        .quality(settings.quality)
        .exposure(settings.exposure)
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
        .toneMapping(settings.toneMapping)
        .luminanceScaling(settings.luminanceScaling)
        .build(*engine);
}

static std::ostream& operator<<(std::ostream& out, AntiAliasing in) {
    switch (in) {
        case AntiAliasing::NONE: return out << "\"NONE\"";
        case AntiAliasing::FXAA: return out << "\"FXAA\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& operator<<(std::ostream& out, Dithering in) {
    switch (in) {
        case Dithering::NONE: return out << "\"NONE\"";
        case Dithering::TEMPORAL: return out << "\"TEMPORAL\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& operator<<(std::ostream& out, ShadowType in) {
    switch (in) {
        case ShadowType::PCF: return out << "\"PCF\"";
        case ShadowType::VSM: return out << "\"VSM\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& operator<<(std::ostream& out, BlendMode in) {
    switch (in) {
        case BlendMode::ADD: return out << "\"ADD\"";
        case BlendMode::INTERPOLATE: return out << "\"INTERPOLATE\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& operator<<(std::ostream& out, VQL in) {
    switch (in) {
        case VQL::LOW: return out << "\"LOW\"";
        case VQL::MEDIUM: return out << "\"MEDIUM\"";
        case VQL::HIGH: return out << "\"HIGH\"";
        case VQL::ULTRA: return out << "\"ULTRA\"";
    }
    return out << "\"INVALID\"";
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

static std::ostream& operator<<(std::ostream& out, DepthOfFieldOptions::Filter in) {
    switch (in) {
        case DepthOfFieldOptions::Filter::NONE: return out << "\"NONE\"";
        case DepthOfFieldOptions::Filter::MEDIAN: return out << "\"MEDIAN\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& operator<<(std::ostream& out, ToneMapping in) {
    switch (in) {
        case ToneMapping::LINEAR: return out << "\"LINEAR\"";
        case ToneMapping::ACES_LEGACY: return out << "\"ACES_LEGACY\"";
        case ToneMapping::ACES: return out << "\"ACES\"";
        case ToneMapping::FILMIC: return out << "\"FILMIC\"";
        case ToneMapping::RESERVED: return out << "\"RESERVED\"";
        case ToneMapping::REINHARD: return out << "\"REINHARD\"";
        case ToneMapping::DISPLAY_RANGE: return out << "\"DISPLAY_RANGE\"";
    }
    return out << "\"INVALID\"";
}

static std::ostream& writeJson(std::ostream& oss, const float* v, int count) {
    oss << "[";
    for (int i = 0; i < count; i++) {
        oss << v[i];
        if (i < count - 1) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss;
}

static std::ostream& operator<<(std::ostream& out, math::float3 v) {
    return writeJson(out, &v.x, 3);
}

static std::ostream& operator<<(std::ostream& out, math::float4 v) {
    return writeJson(out, &v.x, 4);
}

static std::ostream& operator<<(std::ostream& out, const TemporalAntiAliasingOptions& in) {
    return out << "{\n"
        << "\"filterWidth\": " << (in.filterWidth) << ",\n"
        << "\"feedback\": " << (in.feedback) << ",\n"
        << "\"enabled\": " << to_string(in.enabled) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const ColorGradingSettings& in) {
    return out << "{\n"
        << "\"enabled\": " << to_string(in.enabled) << ",\n"
        << "\"quality\": " << (in.quality) << ",\n"
        << "\"toneMapping\": " << (in.toneMapping) << ",\n"
        << "\"luminanceScaling\": " << to_string(in.luminanceScaling) << ",\n"
        << "\"exposure\": " << (in.exposure) << ",\n"
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

static std::ostream& operator<<(std::ostream& out, const AmbientOcclusionOptions::Ssct& in) {
    return out << "{\n"
        << "\"enabled\": " << to_string(in.enabled) << ",\n"
        << "\"lightConeRad\": " << (in.lightConeRad) << ",\n"
        << "\"shadowDistance\": " << (in.shadowDistance) << ",\n"
        << "\"contactDistanceMax\": " << (in.contactDistanceMax) << ",\n"
        << "\"intensity\": " << (in.intensity) << ",\n"
        << "\"lightDirection\": " << (in.lightDirection) << ",\n"
        << "\"depthBias\": " << (in.depthBias) << ",\n"
        << "\"depthSlopeBias\": " << (in.depthSlopeBias) << ",\n"
        << "\"sampleCount\": " << int(in.sampleCount) << ",\n"
        << "\"rayCount\": " << int(in.rayCount) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const AmbientOcclusionOptions& in) {
    return out << "{\n"
        << "\"radius\": " << (in.radius) << ",\n"
        << "\"power\": " << (in.power) << ",\n"
        << "\"bias\": " << (in.bias) << ",\n"
        << "\"resolution\": " << (in.resolution) << ",\n"
        << "\"intensity\": " << (in.intensity) << ",\n"
        << "\"bilateralThreshold\": " << (in.bilateralThreshold) << ",\n"
        << "\"quality\": " << (in.quality) << ",\n"
        << "\"lowPassFilter\": " << (in.lowPassFilter) << ",\n"
        << "\"upsampling\": " << (in.upsampling) << ",\n"
        << "\"enabled\": " << to_string(in.enabled) << ",\n"
        << "\"minHorizonAngleRad\": " << (in.minHorizonAngleRad) << ",\n"
        << "\"ssct\": " << (in.ssct) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const BloomOptions& in) {
    return out << "{\n"
        << "\"strength\": " << (in.strength) << ",\n"
        << "\"resolution\": " << (in.resolution) << ",\n"
        << "\"anamorphism\": " << (in.anamorphism) << ",\n"
        << "\"levels\": " << int(in.levels) << ",\n"
        << "\"blendMode\": " << (in.blendMode) << ",\n"
        << "\"threshold\": " << to_string(in.threshold) << ",\n"
        << "\"enabled\": " << to_string(in.enabled) << ",\n"
        << "\"highlight\": " << (in.highlight) << ",\n"
        << "\"lensFlare\": " << to_string(in.lensFlare) << ",\n"
        << "\"starburst\": " << to_string(in.starburst) << ",\n"
        << "\"chromaticAberration\": " << (in.chromaticAberration) << ",\n"
        << "\"ghostCount\": " << int(in.ghostCount) << ",\n"
        << "\"ghostSpacing\": " << (in.ghostSpacing) << ",\n"
        << "\"ghostThreshold\": " << (in.ghostThreshold) << ",\n"
        << "\"haloThickness\": " << (in.haloThickness) << ",\n"
        << "\"haloRadius\": " << (in.haloRadius) << ",\n"
        << "\"haloThreshold\": " << (in.haloThreshold) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const FogOptions& in) {
    return out << "{\n"
        << "\"distance\": " << (in.distance) << ",\n"
        << "\"maximumOpacity\": " << (in.maximumOpacity) << ",\n"
        << "\"height\": " << (in.height) << ",\n"
        << "\"heightFalloff\": " << (in.heightFalloff) << ",\n"
        << "\"color\": " << (in.color) << ",\n"
        << "\"density\": " << (in.density) << ",\n"
        << "\"inScatteringStart\": " << (in.inScatteringStart) << ",\n"
        << "\"inScatteringSize\": " << (in.inScatteringSize) << ",\n"
        << "\"fogColorFromIbl\": " << to_string(in.fogColorFromIbl) << ",\n"
        << "\"enabled\": " << to_string(in.enabled) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const LightManager::ShadowOptions& in) {
    const float* splits = in.cascadeSplitPositions;
    math::float3 splitsVector = { splits[0], splits[1], splits[2] };
    return out << "{\n"
        << "\"vsm\": {\n"
        << "\"msaaSamples\": " << int(in.vsm.msaaSamples) << ",\n"
        << "\"blurWidth\": " << in.vsm.blurWidth << "\n"
        << "},\n"
        << "\"mapSize\": " << in.mapSize << ",\n"
        << "\"screenSpaceContactShadows\": " << to_string(in.screenSpaceContactShadows) << ",\n"
        << "\"shadowCascades\": " << int(in.shadowCascades) << ",\n"
        << "\"cascadeSplitPositions\": " << (splitsVector) << "\n"
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
    for (auto prop : in.scalar) { writeJson(prop, oss); }
    oss << "},\n";
    oss << "\"float3\": {\n";
    for (auto prop : in.float3) { writeJson(prop, oss); }
    oss << "},\n";
    oss << "\"float4\": {\n";
    for (auto prop : in.float4) { writeJson(prop, oss); }
    oss << "},\n";
    oss << "}";
    std::string result = oss.str();

    const auto replace = [&result](std::string s, std::string t) {
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

static std::ostream& operator<<(std::ostream& out, const LightSettings& in) {
    return out << "{\n"
        << "\"enableShadows\": " << to_string(in.enableShadows) << ",\n"
        << "\"enableSunlight\": " << to_string(in.enableSunlight) << ",\n"
        << "\"shadowOptions\": " << (in.shadowOptions) << ",\n"
        << "\"sunlightIntensity\": " << (in.sunlightIntensity) << ",\n"
        << "\"sunlightDirection\": " << (in.sunlightDirection) << ",\n"
        << "\"sunlightColor\": " << (in.sunlightColor) << ",\n"
        << "\"iblIntensity\": " << (in.iblIntensity) << ",\n"
        << "\"iblRotation\": " << (in.iblRotation) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const ViewerOptions& in) {
    return out << "{\n"
        << "\"cameraAperture\": " << (in.cameraAperture) << ",\n"
        << "\"cameraSpeed\": " << (in.cameraSpeed) << ",\n"
        << "\"cameraISO\": " << (in.cameraISO) << ",\n"
        << "\"groundShadowStrength\": " << (in.groundShadowStrength) << ",\n"
        << "\"groundPlaneEnabled\": " << to_string(in.groundPlaneEnabled) << ",\n"
        << "\"skyboxEnabled\": " << to_string(in.skyboxEnabled) << ",\n"
        << "\"backgroundColor\": " << (in.backgroundColor) << ",\n"
        << "\"cameraFocalLength\": " << (in.cameraFocalLength) << ",\n"
        << "\"cameraFocusDistance\": " << (in.cameraFocusDistance) << ",\n"
        << "\"autoScaleEnabled\": " << to_string(in.autoScaleEnabled) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const DepthOfFieldOptions& in) {
    return out << "{\n"
        << "\"cocScale\": " << (in.cocScale) << ",\n"
        << "\"maxApertureDiameter\": " << (in.maxApertureDiameter) << ",\n"
        << "\"enabled\": " << to_string(in.enabled) << ",\n"
        << "\"filter\": " << (in.filter) << ",\n"
        << "\"nativeResolution\": " << to_string(in.nativeResolution) << ",\n"
        << "\"foregroundRingCount\": " << int(in.foregroundRingCount) << ",\n"
        << "\"backgroundRingCount\": " << int(in.backgroundRingCount) << ",\n"
        << "\"fastGatherRingCount\": " << int(in.fastGatherRingCount) << ",\n"
        << "\"maxForegroundCOC\": " << (in.maxForegroundCOC) << ",\n"
        << "\"maxBackgroundCOC\": " << (in.maxBackgroundCOC) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const VignetteOptions& in) {
    return out << "{\n"
        << "\"midPoint\": " << (in.midPoint) << ",\n"
        << "\"roundness\": " << (in.roundness) << ",\n"
        << "\"feather\": " << (in.feather) << ",\n"
        << "\"color\": " << (in.color) << ",\n"
        << "\"enabled\": " << to_string(in.enabled) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const RenderQuality& in) {
    return out << "{\n"
        << "\"hdrColorBuffer\": " << (in.hdrColorBuffer) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const DynamicLightingSettings& in) {
    return out << "{\n"
        << "\"zLightNear\": " << (in.zLightNear) << ",\n"
        << "\"zLightFar\": " << (in.zLightFar) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const VsmShadowOptions& in) {
    return out << "{\n"
        << "\"anisotropy\": " << int(in.anisotropy) << ",\n"
        << "\"mipmapping\": " << to_string(in.mipmapping) << ",\n"
        << "\"exponent\": " << in.exponent << ",\n"
        << "\"minVarianceScale\": " << in.minVarianceScale << ",\n"
        << "\"lightBleedReduction\": " << in.lightBleedReduction << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const ViewSettings& in) {
    return out << "{\n"
        << "\"sampleCount\": " << int(in.sampleCount) << ",\n"
        << "\"antiAliasing\": " << in.antiAliasing << ",\n"
        << "\"taa\": " << in.taa << ",\n"
        << "\"colorGrading\": " << (in.colorGrading) << ",\n"
        << "\"ssao\": " << (in.ssao) << ",\n"
        << "\"bloom\": " << (in.bloom) << ",\n"
        << "\"fog\": " << (in.fog) << ",\n"
        << "\"dof\": " << (in.dof) << ",\n"
        << "\"vignette\": " << (in.vignette) << ",\n"
        << "\"dithering\": " << (in.dithering) << ",\n"
        << "\"renderQuality\": " << (in.renderQuality) << ",\n"
        << "\"dynamicLighting\": " << (in.dynamicLighting) << ",\n"
        << "\"shadowType\": " << (in.shadowType) << ",\n"
        << "\"vsmShadowOptions\": " << (in.vsmShadowOptions) << ",\n"
        << "\"postProcessingEnabled\": " << to_string(in.postProcessingEnabled) << "\n"
        << "}";
}

static std::ostream& operator<<(std::ostream& out, const Settings& in) {
    return out << "{\n"
        << "\"view\": " << (in.view) << ",\n"
        << "\"material\": " << (in.material) << ",\n"
        << "\"lighting\": " << (in.lighting) << ",\n"
        << "\"viewer\": " << (in.viewer)
        << "}";
}

bool ColorGradingSettings::operator==(const ColorGradingSettings &rhs) const {
    // If you had to fix the following codeline, then you likely also need to update the
    // implementation of operator==.
    static_assert(sizeof(ColorGradingSettings) == 204, "Please update Settings.cpp");
    return enabled == rhs.enabled &&
            quality == rhs.quality &&
            toneMapping == rhs.toneMapping &&
            luminanceScaling == rhs.luminanceScaling &&
            exposure == rhs.exposure &&
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

} // namespace viewer
} // namespace filament
