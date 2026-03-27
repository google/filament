// Copyright 2018 The Draco Authors.
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
#include "draco/io/gltf_utils.h"

#include <ostream>
#include <string>

#ifdef DRACO_TRANSCODER_SUPPORTED
namespace draco {

std::ostream &operator<<(std::ostream &os, const GltfValue &value) {
  if (value.type_ == GltfValue::INT) {
    os << value.value_int_;
  } else {
    os << value.value_double_;
  }
  return os;
}

Indent::Indent() : indent_space_count_(2) {}

void Indent::Increase() { indent_ += std::string(indent_space_count_, ' '); }

void Indent::Decrease() { indent_.erase(0, indent_space_count_); }

std::ostream &operator<<(std::ostream &os, const Indent &indent) {
  return os << indent.indent_;
}

std::ostream &operator<<(std::ostream &os,
                         const JsonWriter::IndentWrapper &indent) {
  if (indent.writer.mode_ == JsonWriter::READABLE) {
    os << indent.writer.indent_writer_;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const JsonWriter::Separator &separator) {
  if (separator.writer.mode_ == JsonWriter::READABLE) {
    os << " ";
  }
  return os;
}

void JsonWriter::Reset() {
  last_type_ = START;
  o_.clear();
  o_.str("");
}

void JsonWriter::BeginObject() { BeginObject(""); }

void JsonWriter::BeginObject(const std::string &name) {
  FinishPreviousLine(BEGIN);
  o_ << indent_;
  if (!name.empty()) {
    o_ << "\"" << name << "\":" << separator_;
  }
  o_ << "{";
  indent_writer_.Increase();
}

void JsonWriter::EndObject() {
  FinishPreviousLine(END);
  indent_writer_.Decrease();
  o_ << indent_ << "}";
}

void JsonWriter::BeginArray() {
  FinishPreviousLine(BEGIN);
  o_ << indent_ << "[";
  indent_writer_.Increase();
}

void JsonWriter::BeginArray(const std::string &name) {
  FinishPreviousLine(BEGIN);
  o_ << indent_ << "\"" << name << "\":" << separator_ << "[";
  indent_writer_.Increase();
}

void JsonWriter::EndArray() {
  FinishPreviousLine(END);
  indent_writer_.Decrease();
  o_ << indent_ << "]";
}

void JsonWriter::FinishPreviousLine(OutputType curr_type) {
  if (last_type_ != START) {
    if ((last_type_ == VALUE && curr_type == VALUE) ||
        (last_type_ == VALUE && curr_type == BEGIN) ||
        (last_type_ == END && curr_type == BEGIN) ||
        (last_type_ == END && curr_type == VALUE)) {
      o_ << ",";
    }
    if (mode_ == READABLE) {
      o_ << std::endl;
    }
  }
  last_type_ = curr_type;
}

std::string JsonWriter::MoveData() {
  const std::string str = o_.str();
  o_.str("");
  return str;
}

std::string JsonWriter::EscapeCharacter(const std::string &str,
                                        const char character) {
  size_t start = 0;
  if ((start = str.find(character, start)) != std::string::npos) {
    std::string s = str;
    std::string escaped_character = "\\";
    escaped_character += character;
    do {
      s.replace(start, 1, escaped_character);
      start += escaped_character.length();
    } while ((start = s.find(character, start)) != std::string::npos);
    return s;
  }
  return str;
}

std::string JsonWriter::EscapeJsonSpecialCharacters(const std::string &str) {
  std::string s = str;
  const char backspace = '\b';
  const char form_feed = '\f';
  const char newline = '\n';
  const char carriage_return = '\r';
  const char tab = '\t';
  const char double_quote = '\"';
  const char backslash = '\\';

  // Backslash must come first.
  s = EscapeCharacter(s, backslash);
  s = EscapeCharacter(s, backspace);
  s = EscapeCharacter(s, form_feed);
  s = EscapeCharacter(s, newline);
  s = EscapeCharacter(s, carriage_return);
  s = EscapeCharacter(s, tab);
  s = EscapeCharacter(s, double_quote);
  return s;
}

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
