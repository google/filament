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

#include <utils/Log.h>

#include "jsonParseUtils.h"

#include <assert.h>

#include <sstream>
#include <string>

using namespace utils;

namespace filament {
namespace viewer {

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
    *val = 0 == compare(tokens[i], jsonChunk, "true");
    return i + 1;
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
    else if (0 == compare(tokens[i], jsonChunk, "UCHIMURA")) { *out = ToneMapping::UCHIMURA; }
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
        } else if (compare(tok, jsonChunk, "sampleCount") == 0){
            i = parse(tokens, i + 1, jsonChunk, &out->sampleCount);
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
        } else if (compare(tok, jsonChunk, "quality") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->quality);
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

static int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, DepthOfFieldOptions* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (0 == compare(tok, jsonChunk, "focusDistance")) {
            i = parse(tokens, i + 1, jsonChunk, &out->focusDistance);
        } else if (0 == compare(tok, jsonChunk, "cocScale")) {
            i = parse(tokens, i + 1, jsonChunk, &out->cocScale);
        } else if (0 == compare(tok, jsonChunk, "maxApertureDiameter")) {
            i = parse(tokens, i + 1, jsonChunk, &out->maxApertureDiameter);
        } else if (0 == compare(tok, jsonChunk, "enabled")) {
            i = parse(tokens, i + 1, jsonChunk, &out->enabled);
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

int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, Settings* out) {
    CHECK_TOKTYPE(tokens[i], JSMN_OBJECT);
    int size = tokens[i++].size;
    for (int j = 0; j < size; ++j) {
        const jsmntok_t tok = tokens[i];
        CHECK_KEY(tok);
        if (compare(tok, jsonChunk, "view") == 0) {
            i = parse(tokens, i + 1, jsonChunk, &out->view);
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

bool readJson(const char* jsonChunk, size_t size, Settings* out) {
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
    dest->setPostProcessingEnabled(settings.postProcessingEnabled);
}

ColorGrading* createColorGrading(const ColorGradingSettings& settings, Engine* engine) {
    return ColorGrading::Builder()
        .quality(settings.quality)
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
        .build(*engine);
}

static std::string writeJson(bool v) { return v ? "true" : "false"; }
static std::string writeJson(float v) { return std::to_string(v); }
static std::string writeJson(int v) {  return std::to_string(v); }
static std::string writeJson(uint32_t v) { return std::to_string(v); }
static std::string writeJson(uint8_t v) {  return std::to_string((int)v); }

static const char* writeJson(AntiAliasing in) {
    switch (in) {
        case AntiAliasing::NONE: return "\"NONE\"";
        case AntiAliasing::FXAA: return "\"FXAA\"";
    }
    return "\"INVALID\"";
}

static const char* writeJson(Dithering in) {
    switch (in) {
        case Dithering::NONE: return "\"NONE\"";
        case Dithering::TEMPORAL: return "\"TEMPORAL\"";
    }
    return "\"INVALID\"";
}

static const char* writeJson(ShadowType in) {
    switch (in) {
        case ShadowType::PCF: return "\"PCF\"";
        case ShadowType::VSM: return "\"VSM\"";
    }
    return "\"INVALID\"";
}

static const char* writeJson(BlendMode in) {
    switch (in) {
        case BlendMode::ADD: return "\"ADD\"";
        case BlendMode::INTERPOLATE: return "\"INTERPOLATE\"";
    }
    return "\"INVALID\"";
}

static const char* writeJson(VQL in) {
    switch (in) {
        case VQL::LOW: return "\"LOW\"";
        case VQL::MEDIUM: return "\"MEDIUM\"";
        case VQL::HIGH: return "\"HIGH\"";
        case VQL::ULTRA: return "\"ULTRA\"";
    }
    return "\"INVALID\"";
}

static const char* writeJson(CGQL in) {
    switch (in) {
        case CGQL::LOW: return "\"LOW\"";
        case CGQL::MEDIUM: return "\"MEDIUM\"";
        case CGQL::HIGH: return "\"HIGH\"";
        case CGQL::ULTRA: return "\"ULTRA\"";
    }
    return "\"INVALID\"";
}

static const char* writeJson(ToneMapping in) {
    switch (in) {
        case ToneMapping::LINEAR: return "\"LINEAR\"";
        case ToneMapping::ACES_LEGACY: return "\"ACES_LEGACY\"";
        case ToneMapping::ACES: return "\"ACES\"";
        case ToneMapping::FILMIC: return "\"FILMIC\"";
        case ToneMapping::UCHIMURA: return "\"UCHIMURA\"";
        case ToneMapping::REINHARD: return "\"REINHARD\"";
        case ToneMapping::DISPLAY_RANGE: return "\"DISPLAY_RANGE\"";
    }
    return "\"INVALID\"";
}

static std::string writeJson(const float* v, int count) {
    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < count; i++) {
        oss << v[i];
        if (i < count - 1) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}

static std::string writeJson(math::float3 v) {
    return writeJson(&v.x, 3);
}

static std::string writeJson(math::float4 v) {
    return writeJson(&v.x, 4);
}

std::string writeJson(const Settings& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"view\": "
        << writeJson(in.view)
        << "}";
    return oss.str();
}

std::string writeJson(const TemporalAntiAliasingOptions& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"filterWidth\": " << writeJson(in.filterWidth) << ",\n"
        << "\"feedback\": " << writeJson(in.feedback) << ",\n"
        << "\"enabled\": " << writeJson(in.enabled) << "\n"
        << "}";
    return oss.str();
}

std::string writeJson(const ColorGradingSettings& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"enabled\": " << writeJson(in.enabled) << ",\n"
        << "\"quality\": " << writeJson(in.quality) << ",\n"
        << "\"toneMapping\": " << writeJson(in.toneMapping) << ",\n"
        << "\"temperature\": " << writeJson(in.temperature) << ",\n"
        << "\"tint\": " << writeJson(in.tint) << ",\n"
        << "\"outRed\": " << writeJson(in.outRed) << ",\n"
        << "\"outGreen\": " << writeJson(in.outGreen) << ",\n"
        << "\"outBlue\": " << writeJson(in.outBlue) << ",\n"
        << "\"shadows\": " << writeJson(in.shadows) << ",\n"
        << "\"midtones\": " << writeJson(in.midtones) << ",\n"
        << "\"highlights\": " << writeJson(in.highlights) << ",\n"
        << "\"ranges\": " << writeJson(in.ranges) << ",\n"
        << "\"contrast\": " << writeJson(in.contrast) << ",\n"
        << "\"vibrance\": " << writeJson(in.vibrance) << ",\n"
        << "\"saturation\": " << writeJson(in.saturation) << ",\n"
        << "\"slope\": " << writeJson(in.slope) << ",\n"
        << "\"offset\": " << writeJson(in.offset) << ",\n"
        << "\"power\": " << writeJson(in.power) << ",\n"
        << "\"gamma\": " << writeJson(in.gamma) << ",\n"
        << "\"midPoint\": " << writeJson(in.midPoint) << ",\n"
        << "\"scale\": " << writeJson(in.scale) << ",\n"
        << "\"linkedCurves\": " << writeJson(in.linkedCurves) << "\n"
        << "}";
    return oss.str();
}

std::string writeJson(const AmbientOcclusionOptions::Ssct& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"enabled\": " << writeJson(in.enabled) << ",\n"
        << "\"lightConeRad\": " << writeJson(in.lightConeRad) << ",\n"
        << "\"shadowDistance\": " << writeJson(in.shadowDistance) << ",\n"
        << "\"contactDistanceMax\": " << writeJson(in.contactDistanceMax) << ",\n"
        << "\"intensity\": " << writeJson(in.intensity) << ",\n"
        << "\"lightDirection\": " << writeJson(in.lightDirection) << ",\n"
        << "\"depthBias\": " << writeJson(in.depthBias) << ",\n"
        << "\"depthSlopeBias\": " << writeJson(in.depthSlopeBias) << ",\n"
        << "\"sampleCount\": " << writeJson(in.sampleCount) << "\n"
        << "}";
    return oss.str();
}

std::string writeJson(const AmbientOcclusionOptions& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"radius\": " << writeJson(in.radius) << ",\n"
        << "\"power\": " << writeJson(in.power) << ",\n"
        << "\"bias\": " << writeJson(in.bias) << ",\n"
        << "\"resolution\": " << writeJson(in.resolution) << ",\n"
        << "\"intensity\": " << writeJson(in.intensity) << ",\n"
        << "\"quality\": " << writeJson(in.quality) << ",\n"
        << "\"upsampling\": " << writeJson(in.upsampling) << ",\n"
        << "\"enabled\": " << writeJson(in.enabled) << ",\n"
        << "\"minHorizonAngleRad\": " << writeJson(in.minHorizonAngleRad) << ",\n"
        << "\"ssct\": " << writeJson(in.ssct) << "\n"
        << "}";
    return oss.str();
}

std::string writeJson(const BloomOptions& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"strength\": " << writeJson(in.strength) << ",\n"
        << "\"resolution\": " << writeJson(in.resolution) << ",\n"
        << "\"anamorphism\": " << writeJson(in.anamorphism) << ",\n"
        << "\"levels\": " << writeJson(in.levels) << ",\n"
        << "\"blendMode\": " << writeJson(in.blendMode) << ",\n"
        << "\"threshold\": " << writeJson(in.threshold) << ",\n"
        << "\"enabled\": " << writeJson(in.enabled) << ",\n"
        << "\"highlight\": " << writeJson(in.highlight) << "\n"
        << "}";
    return oss.str();
}

std::string writeJson(const FogOptions& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"distance\": " << writeJson(in.distance) << ",\n"
        << "\"maximumOpacity\": " << writeJson(in.maximumOpacity) << ",\n"
        << "\"height\": " << writeJson(in.height) << ",\n"
        << "\"heightFalloff\": " << writeJson(in.heightFalloff) << ",\n"
        << "\"color\": " << writeJson(in.color) << ",\n"
        << "\"density\": " << writeJson(in.density) << ",\n"
        << "\"inScatteringStart\": " << writeJson(in.inScatteringStart) << ",\n"
        << "\"inScatteringSize\": " << writeJson(in.inScatteringSize) << ",\n"
        << "\"fogColorFromIbl\": " << writeJson(in.fogColorFromIbl) << ",\n"
        << "\"enabled\": " << writeJson(in.enabled) << "\n"
        << "}";
    return oss.str();
}

std::string writeJson(const DepthOfFieldOptions& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"focusDistance\": " << writeJson(in.focusDistance) << ",\n"
        << "\"cocScale\": " << writeJson(in.cocScale) << ",\n"
        << "\"maxApertureDiameter\": " << writeJson(in.maxApertureDiameter) << ",\n"
        << "\"enabled\": " << writeJson(in.enabled) << "\n"
        << "}";
    return oss.str();
}

std::string writeJson(const VignetteOptions& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"midPoint\": " << writeJson(in.midPoint) << ",\n"
        << "\"roundness\": " << writeJson(in.roundness) << ",\n"
        << "\"feather\": " << writeJson(in.feather) << ",\n"
        << "\"color\": " << writeJson(in.color) << ",\n"
        << "\"enabled\": " << writeJson(in.enabled) << "\n"
        << "}";
    return oss.str();
}

std::string writeJson(const RenderQuality& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"hdrColorBuffer\": " << writeJson(in.hdrColorBuffer) << "\n"
        << "}";
    return oss.str();
}

std::string writeJson(const DynamicLightingSettings& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"zLightNear\": " << writeJson(in.zLightNear) << ",\n"
        << "\"zLightFar\": " << writeJson(in.zLightFar) << "\n"
        << "}";
    return oss.str();
}

std::string writeJson(const ViewSettings& in) {
    std::ostringstream oss;
    oss << "{\n"
        << "\"sampleCount\": " << writeJson(in.sampleCount) << ",\n"
        << "\"antiAliasing\": " << writeJson(in.antiAliasing) << ",\n"
        << "\"taa\": " << writeJson(in.taa) << ",\n"
        << "\"colorGrading\": " << writeJson(in.colorGrading) << ",\n"
        << "\"ssao\": " << writeJson(in.ssao) << ",\n"
        << "\"bloom\": " << writeJson(in.bloom) << ",\n"
        << "\"fog\": " << writeJson(in.fog) << ",\n"
        << "\"dof\": " << writeJson(in.dof) << ",\n"
        << "\"vignette\": " << writeJson(in.vignette) << ",\n"
        << "\"dithering\": " << writeJson(in.dithering) << ",\n"
        << "\"renderQuality\": " << writeJson(in.renderQuality) << ",\n"
        << "\"dynamicLighting\": " << writeJson(in.dynamicLighting) << ",\n"
        << "\"shadowType\": " << writeJson(in.shadowType) << ",\n"
        << "\"postProcessingEnabled\": " << writeJson(in.postProcessingEnabled) << "\n"
        << "}";
    return oss.str();
}


} // namespace viewer
} // namespace filament
