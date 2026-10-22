// Copyright 2016 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef DRACO_IO_PARSER_UTILS_H_
#define DRACO_IO_PARSER_UTILS_H_

#include "draco/core/decoder_buffer.h"

namespace draco {
namespace parser {

// Skips to first character not included in |skip_chars|.
void SkipCharacters(DecoderBuffer *buffer, const char *skip_chars);

// Skips any whitespace until a regular character is reached.
void SkipWhitespace(DecoderBuffer *buffer);

// Returns true if the next character is a whitespace.
// |end_reached| is set to true when the end of the stream is reached.
bool PeekWhitespace(DecoderBuffer *buffer, bool *end_reached);

// Skips all characters until the end of the line.
void SkipLine(DecoderBuffer *buffer);

// Parses signed floating point number or returns false on error.
bool ParseFloat(DecoderBuffer *buffer, float *value);

// Parses a signed integer (can be preceded by '-' or '+' characters.
bool ParseSignedInt(DecoderBuffer *buffer, int32_t *value);

// Parses an unsigned integer. It cannot be preceded by '-' or '+'
// characters.
bool ParseUnsignedInt(DecoderBuffer *buffer, uint32_t *value);

// Returns -1 if c == '-'.
// Returns +1 if c == '+'.
// Returns 0 otherwise.
int GetSignValue(char c);

// Parses a string until a whitespace or end of file is reached.
bool ParseString(DecoderBuffer *buffer, std::string *out_string);

// Parses the entire line into the buffer (excluding the new line characters).
void ParseLine(DecoderBuffer *buffer, std::string *out_string);

// Parses line and stores into a new decoder buffer.
DecoderBuffer ParseLineIntoDecoderBuffer(DecoderBuffer *buffer);

// Returns a string with all characters converted to lower case.
std::string ToLower(const std::string &str);

}  // namespace parser
}  // namespace draco

#endif  // DRACO_IO_PARSER_UTILS_H_
