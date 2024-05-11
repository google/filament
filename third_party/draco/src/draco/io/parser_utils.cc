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
#include "draco/io/parser_utils.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>

namespace draco {
namespace parser {

void SkipCharacters(DecoderBuffer *buffer, const char *skip_chars) {
  if (skip_chars == nullptr) {
    return;
  }
  const int num_skip_chars = static_cast<int>(strlen(skip_chars));
  char c;
  while (buffer->Peek(&c)) {
    // Check all characters in the pattern.
    bool skip = false;
    for (int i = 0; i < num_skip_chars; ++i) {
      if (c == skip_chars[i]) {
        skip = true;
        break;
      }
    }
    if (!skip) {
      return;
    }
    buffer->Advance(1);
  }
}

void SkipWhitespace(DecoderBuffer *buffer) {
  bool end_reached = false;
  while (PeekWhitespace(buffer, &end_reached) && !end_reached) {
    // Skip the whitespace character
    buffer->Advance(1);
  }
}

bool PeekWhitespace(DecoderBuffer *buffer, bool *end_reached) {
  uint8_t c;
  if (!buffer->Peek(&c)) {
    *end_reached = true;
    return false;  // eof reached.
  }
  if (!isspace(c)) {
    return false;  // Non-whitespace character reached.
  }
  return true;
}

void SkipLine(DecoderBuffer *buffer) { ParseLine(buffer, nullptr); }

bool ParseFloat(DecoderBuffer *buffer, float *value) {
  // Read optional sign.
  char ch;
  if (!buffer->Peek(&ch)) {
    return false;
  }
  int sign = GetSignValue(ch);
  if (sign != 0) {
    buffer->Advance(1);
  } else {
    sign = 1;
  }

  // Parse integer component.
  bool have_digits = false;
  double v = 0.0;
  while (buffer->Peek(&ch) && ch >= '0' && ch <= '9') {
    v *= 10.0;
    v += (ch - '0');
    buffer->Advance(1);
    have_digits = true;
  }
  if (ch == '.') {
    // Parse fractional component.
    buffer->Advance(1);
    double fraction = 1.0;
    while (buffer->Peek(&ch) && ch >= '0' && ch <= '9') {
      fraction *= 0.1;
      v += (ch - '0') * fraction;
      buffer->Advance(1);
      have_digits = true;
    }
  }

  if (!have_digits) {
    // Check for special constants (inf, nan, ...).
    std::string text;
    if (!ParseString(buffer, &text)) {
      return false;
    }
    if (text == "inf" || text == "Inf") {
      v = std::numeric_limits<double>::infinity();
    } else if (text == "nan" || text == "NaN") {
      v = nan("");
    } else {
      // Invalid string.
      return false;
    }
  } else {
    // Handle exponent if present.
    if (ch == 'e' || ch == 'E') {
      buffer->Advance(1);  // Skip 'e' marker.

      // Parse integer exponent.
      int32_t exponent = 0;
      if (!ParseSignedInt(buffer, &exponent)) {
        return false;
      }

      // Apply exponent scaling to value.
      v *= pow(static_cast<double>(10.0), exponent);
    }
  }

  *value = (sign < 0) ? static_cast<float>(-v) : static_cast<float>(v);
  return true;
}

bool ParseSignedInt(DecoderBuffer *buffer, int32_t *value) {
  // Parse any explicit sign and set the appropriate largest magnitude
  // value that can be represented without overflow.
  char ch;
  if (!buffer->Peek(&ch)) {
    return false;
  }
  const int sign = GetSignValue(ch);
  if (sign != 0) {
    buffer->Advance(1);
  }

  // Attempt to parse integer body.
  uint32_t v;
  if (!ParseUnsignedInt(buffer, &v)) {
    return false;
  }
  *value = (sign < 0) ? -v : v;
  return true;
}

bool ParseUnsignedInt(DecoderBuffer *buffer, uint32_t *value) {
  // Parse the number until we run out of digits.
  uint32_t v = 0;
  char ch;
  bool have_digits = false;
  while (buffer->Peek(&ch) && ch >= '0' && ch <= '9') {
    v *= 10;
    v += (ch - '0');
    buffer->Advance(1);
    have_digits = true;
  }
  if (!have_digits) {
    return false;
  }
  *value = v;
  return true;
}

int GetSignValue(char c) {
  if (c == '-') {
    return -1;
  }
  if (c == '+') {
    return 1;
  }
  return 0;
}

bool ParseString(DecoderBuffer *buffer, std::string *out_string) {
  out_string->clear();
  SkipWhitespace(buffer);
  bool end_reached = false;
  while (!PeekWhitespace(buffer, &end_reached) && !end_reached) {
    char c;
    if (!buffer->Decode(&c)) {
      return false;
    }
    *out_string += c;
  }
  return true;
}

void ParseLine(DecoderBuffer *buffer, std::string *out_string) {
  if (out_string) {
    out_string->clear();
  }
  char c;
  bool delim_reached = false;
  while (buffer->Peek(&c)) {
    // Check if |c| is a delimeter. We want to parse all delimeters until we
    // reach a non-delimeter symbol. (E.g. we want to ignore '\r\n' at the end
    // of the line).
    const bool is_delim = (c == '\r' || c == '\n');

    // If |c| is a delimeter or it is a non-delimeter symbol before any
    // delimeter was found, we advance the buffer to the next character.
    if (is_delim || !delim_reached) {
      buffer->Advance(1);
    }

    if (is_delim) {
      // Mark that we found a delimeter symbol.
      delim_reached = true;
      continue;
    }
    if (delim_reached) {
      // We reached a non-delimeter symbol after a delimeter was already found.
      // Stop the parsing.
      return;
    }
    // Otherwise we put the non-delimeter symbol into the output string.
    if (out_string) {
      out_string->push_back(c);
    }
  }
}

DecoderBuffer ParseLineIntoDecoderBuffer(DecoderBuffer *buffer) {
  const char *const head = buffer->data_head();
  char c;
  while (buffer->Peek(&c)) {
    // Skip the character.
    buffer->Advance(1);
    if (c == '\n') {
      break;  // End of the line reached.
    }
    if (c == '\r') {
      continue;  // Ignore extra line ending characters.
    }
  }
  DecoderBuffer out_buffer;
  out_buffer.Init(head, buffer->data_head() - head);
  return out_buffer;
}

std::string ToLower(const std::string &str) {
  std::string out;
  std::transform(str.begin(), str.end(), std::back_inserter(out), tolower);
  return out;
}

}  // namespace parser
}  // namespace draco
