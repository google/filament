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

#ifndef VIEWER_JSON_PARSE_UTILS_H
#define VIEWER_JSON_PARSE_UTILS_H

#include <jsmn.h>

namespace filament {
namespace viewer {

#define CHECK_TOKTYPE(tok_, type_) if ((tok_).type != (type_)) { return -1; }
#define CHECK_KEY(tok_) if ((tok_).type != JSMN_STRING || (tok_).size == 0) { return -1; }
#define STR(tok, jsonChunk) std::string(jsonChunk + tok.start, tok.end - tok.start)

struct Settings;

int parse(jsmntok_t const* tokens, int i, const char* jsonChunk, Settings* out);
int parse(jsmntok_t const* tokens, int i);
int compare(jsmntok_t tok, const char* jsonChunk, const char* str);

} // namespace viewer
} // namespace filament

#endif // VIEWER_JSON_PARSE_UTILS_H
