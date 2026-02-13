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

#include <imagediff/ImageDiff.h>

#include <utils/Log.h>

#include <algorithm>
#include <cmath>
#include <cstdlib> // strtof, strtol
#include <cstring>
#include <vector>

// JSMN inclusion
#define JSMN_STATIC
#include <jsmn.h>

namespace imagediff {

using namespace image;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper Functions
// ------------------------------------------------------------------------------------------------

namespace {

bool checkThresholds(float maxAbsDiff, float valRef, float valCand, float mask) {
    float const diff = std::abs(valRef - valCand) * mask;
    return diff <= maxAbsDiff;
}

bool checkPixelRecursive(ImageDiffConfig const& config, float const* pxRef, float const* pxCand,
        float maskVal, float* outMaxDiffs) {
    if (config.mode == ImageDiffConfig::Mode::LEAF) {
        bool passed = true;
        // Check channels using enum
        Channel const channels[] = { Channel::R, Channel::G, Channel::B, Channel::A };
        for (int i = 0; i < 4; ++i) {
            if (config.channelMask & (uint8_t) channels[i]) {
                float const valRef = pxRef[i];
                float const valCand = pxCand[i];

                float const diffUnmasked = std::abs(valRef - valCand);
                if (outMaxDiffs) {
                    outMaxDiffs[i] = std::max(outMaxDiffs[i], diffUnmasked);
                }

                if (!checkThresholds(config.maxAbsDiff, valRef, valCand, maskVal)) {
                    passed = false;
                }
            }
        }
        return passed;
    } else if (config.mode == ImageDiffConfig::Mode::AND) {
        for (auto const& child: config.children) {
            if (!checkPixelRecursive(child, pxRef, pxCand, maskVal, outMaxDiffs)) {
                return false;
            }
        }
        return true;
    } else if (config.mode == ImageDiffConfig::Mode::OR) {
        for (auto const& child: config.children) {
            if (checkPixelRecursive(child, pxRef, pxCand, maskVal, outMaxDiffs)) {
                return true;
            }
        }
        return false;
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// JSON Parsing Helpers
// ------------------------------------------------------------------------------------------------

utils::CString getTokenString(char const* json, jsmntok_t const& tok) {
    if (tok.type == JSMN_STRING || tok.type == JSMN_PRIMITIVE) {
        return utils::CString(json + tok.start, tok.end - tok.start);
    }
    return utils::CString("");
}

int parseJsonRecursive(char const* json, std::vector<jsmntok_t> const& tokens, int idx,
        ImageDiffConfig* outConfig, int depth) {
    if (idx < 0 || idx >= tokens.size()) {
        return idx;
    }

    if (depth > 32) {
        return -1; // Abort on excessive depth
    }

    auto const& tok = tokens[idx];
    if (tok.type != JSMN_OBJECT) {
        return idx + 1; // Expected object
    }

    int const count = tok.size; // number of key-value pairs
    int current = idx + 1;

    for (int i = 0; i < count; ++i) {
        if (current < 0 || current >= tokens.size()) {
            return -1;
        }
        utils::CString key = getTokenString(json, tokens[current]);
        current++; // move to value

        if (key == "mode") {
            utils::CString val = getTokenString(json, tokens[current]);
            if (val == "AND") {
                outConfig->mode = ImageDiffConfig::Mode::AND;
            } else if (val == "OR") {
                outConfig->mode = ImageDiffConfig::Mode::OR;
            } else {
                outConfig->mode = ImageDiffConfig::Mode::LEAF;
            }
            current++;
        } else if (key == "swizzle") {
            utils::CString val = getTokenString(json, tokens[current]);
            if (val == "BGRA") {
                outConfig->swizzle = ImageDiffConfig::Swizzle::BGRA;
            } else {
                outConfig->swizzle = ImageDiffConfig::Swizzle::RGBA;
            }
            current++;
        } else if (key == "channelMask") {
            utils::CString val = getTokenString(json, tokens[current]);
            outConfig->channelMask = (uint8_t) strtol(val.c_str(), nullptr, 10);
            current++;
        } else if (key == "maxFailingPixelsFraction") {
            utils::CString val = getTokenString(json, tokens[current]);
            outConfig->maxFailingPixelsFraction = strtof(val.c_str(), nullptr);
            current++;
        } else if (key == "maxAbsDiff") {
            utils::CString val = getTokenString(json, tokens[current]);
            outConfig->maxAbsDiff = strtof(val.c_str(), nullptr);
            current++;
        } else if (key == "children") {
            if (tokens[current].type == JSMN_ARRAY) {
                int const childCount = tokens[current].size;
                current++;
                for (int check = 0; check < childCount; ++check) {
                    ImageDiffConfig child;
                    current = parseJsonRecursive(json, tokens, current, &child, depth + 1);
                    if (current < 0) return -1;
                    outConfig->children.push_back(child);
                }
            } else {
                current++;
            }
        } else {
            current++;
        }
    }
    return current;
}
} // namespace

// ------------------------------------------------------------------------------------------------
// Core Compare
// ------------------------------------------------------------------------------------------------

ImageDiffResult compare(LinearImage const& reference, LinearImage const& candidate,
        ImageDiffConfig const& config, LinearImage const* mask, bool generateDiffImage) {
    ImageDiffResult result;

    if (reference.getWidth() != candidate.getWidth() ||
            reference.getHeight() != candidate.getHeight()) {
        result.status = ImageDiffResult::Status::SIZE_MISMATCH;
        return result;
    }

    uint32_t const width = reference.getWidth();
    uint32_t const height = reference.getHeight();
    uint32_t const channels = reference.getChannels();

    if (generateDiffImage) {
        result.diffImage = LinearImage(width, height, channels);
        if (mask) {
            result.maskImage = LinearImage(width, height, 1);
        }
    }

    float const* dataRef = reference.getPixelRef();
    float const* dataCand = candidate.getPixelRef();
    float const* dataMask = mask ? mask->getPixelRef() : nullptr;
    float* dataDiff = generateDiffImage ? result.diffImage.getPixelRef() : nullptr;
    float* dataMaskOut = (generateDiffImage && mask) ? result.maskImage.getPixelRef() : nullptr;

    // We assume data is packed float, row-major.
    size_t const totalPixels = width * height;

    for (size_t i = 0; i < totalPixels; ++i) {
        // Extract pixel
        // LinearImage channels can vary. we assume at least RGB(A).
        float pxRef[4] = { 0, 0, 0, 1 };
        float pxCand[4] = { 0, 0, 0, 1 };

        for (uint32_t c = 0; c < channels && c < 4; ++c) {
            pxRef[c] = dataRef[i * channels + c];
            pxCand[c] = dataCand[i * channels + c];
        }

        uint32_t const maskChannels = mask ? mask->getChannels() : 0;
        float const maskVal = dataMask ? dataMask[i * maskChannels + 0] : 1.0f;

        bool const pass = checkPixelRecursive(config, pxRef, pxCand, maskVal, result.maxDiffFound);

        if (!pass) {
            result.failingPixelCount++;
        } else if (maskVal < 1.0f) {
            // Check if it would have failed without the mask
            bool const passUnmasked = checkPixelRecursive(config, pxRef, pxCand, 1.0f, nullptr);
            if (!passUnmasked) {
                result.maskedIgnoredPixelCount++;
            }
        }

        if (dataDiff) {
            for (uint32_t c = 0; c < channels && c < 4; ++c) {
                float const d = std::abs(pxRef[c] - pxCand[c]); // Unmasked
                dataDiff[i * channels + c] = d;
            }
        }
        if (dataMaskOut) {
            dataMaskOut[i] = maskVal;
        }
    }

    float const failFrac = (float) result.failingPixelCount / (float) totalPixels;
    bool const passed = failFrac <= config.maxFailingPixelsFraction;
    if (!passed) {
        result.status = ImageDiffResult::Status::PIXEL_DIFFERENCE;
    }

    return result;
}


bool parseConfig(char const* json, size_t size, ImageDiffConfig* outConfig) {
    jsmn_parser parser;
    jsmn_init(&parser);

    int const count = jsmn_parse(&parser, json, size, nullptr, 0);
    if (count < 0) {
        return false;
    }

    std::vector<jsmntok_t> tokens(count);
    jsmn_init(&parser);
    jsmn_parse(&parser, json, size, tokens.data(), count);

    if (count < 1 || tokens[0].type != JSMN_OBJECT) {
        return false;
    }

    return parseJsonRecursive(json, tokens, 0, outConfig, 0) >= 0;
}

// ------------------------------------------------------------------------------------------------
// JSON Serialization
// ------------------------------------------------------------------------------------------------

utils::CString serializeResult(ImageDiffResult const& result) {
    utils::CString s = "{";
    s += "\"status\": ";
    s += std::to_string((int) result.status).c_str();
    s += ", ";
    s += "\"passed\": ";
    s += (result.status == ImageDiffResult::Status::PASSED ? "true" : "false");
    s += ", ";
    s += "\"failingPixelCount\": ";
    s += std::to_string(result.failingPixelCount).c_str();
    s += ", ";
    s += "\"maxDiffFound\": [";
    s += std::to_string(result.maxDiffFound[0]).c_str();
    s += ", ";
    s += std::to_string(result.maxDiffFound[1]).c_str();
    s += ", ";
    s += std::to_string(result.maxDiffFound[2]).c_str();
    s += ", ";
    s += std::to_string(result.maxDiffFound[3]).c_str();
    s += "]";
    s += "}";
    return s;
}

// ------------------------------------------------------------------------------------------------
// 8-bit Support
// ------------------------------------------------------------------------------------------------

ImageDiffResult compare(Bitmap const& reference, Bitmap const& candidate,
        ImageDiffConfig const& config, Bitmap const* mask, bool generateDiffImage) {
    ImageDiffResult result;

    if (reference.width != candidate.width || reference.height != candidate.height) {
        result.status = ImageDiffResult::Status::SIZE_MISMATCH;
        return result;
    }

    uint32_t const width = reference.width;
    uint32_t const height = reference.height;

    if (generateDiffImage) {
        result.diffImage = LinearImage(width, height, 4);
        if (mask) {
            result.maskImage = LinearImage(width, height, 1);
        }
    }
    float* dataDiff = generateDiffImage ? result.diffImage.getPixelRef() : nullptr;
    float* dataMaskOut = (generateDiffImage && mask) ? result.maskImage.getPixelRef() : nullptr;

    uint8_t const* refBytes = (uint8_t const*) reference.data;
    uint8_t const* candBytes = (uint8_t const*) candidate.data;
    uint8_t const* maskBytes = mask ? (uint8_t const*) mask->data : nullptr;

    bool const isBGRA = (config.swizzle == ImageDiffConfig::Swizzle::BGRA);

    for (uint32_t y = 0; y < height; ++y) {
        uint8_t const* rowRef = refBytes + y * reference.stride;
        uint8_t const* rowCand = candBytes + y * candidate.stride;
        uint8_t const* rowMask = mask ? maskBytes + y * mask->stride : nullptr;

        for (uint32_t x = 0; x < width; ++x) {
            float pxRef[4], pxCand[4];

            auto unpackPixel = [isBGRA](uint8_t const* src, float* out) {
                if (!isBGRA) {
                    out[0] = (float) src[0] / 255.0f;
                    out[1] = (float) src[1] / 255.0f;
                    out[2] = (float) src[2] / 255.0f;
                    out[3] = (float) src[3] / 255.0f;
                } else {
                    out[0] = (float) src[2] / 255.0f;
                    out[1] = (float) src[1] / 255.0f;
                    out[2] = (float) src[0] / 255.0f;
                    out[3] = (float) src[3] / 255.0f;
                }
            };

            unpackPixel(rowRef + x * 4, pxRef);
            unpackPixel(rowCand + x * 4, pxCand);

            float maskVal = 1.0f;
            if (rowMask) {
                maskVal = (float) rowMask[x] / 255.0f;
            }

            bool const pass =
                    checkPixelRecursive(config, pxRef, pxCand, maskVal, result.maxDiffFound);

            if (!pass) {
                result.failingPixelCount++;
            } else if (maskVal < 1.0f) {
                // Check if it would have failed without the mask
                bool const passUnmasked = checkPixelRecursive(config, pxRef, pxCand, 1.0f, nullptr);
                if (!passUnmasked) {
                    result.maskedIgnoredPixelCount++;
                }
            }

            if (dataDiff) {
                size_t const idx = (y * width + x) * 4;
                for (int c = 0; c < 4; ++c) {
                    dataDiff[idx + c] = std::abs(pxRef[c] - pxCand[c]);
                }
            }
            if (dataMaskOut) {
                dataMaskOut[y * width + x] = maskVal;
            }
        }
    }

    float const failFrac = (float) result.failingPixelCount / (float) (width * height);
    bool const passed = failFrac <= config.maxFailingPixelsFraction;
    if (!passed) {
        result.status = ImageDiffResult::Status::PIXEL_DIFFERENCE;
    }

    return result;
}

} // namespace imagediff
