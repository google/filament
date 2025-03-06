// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <google/protobuf/compiler/java/doc_comment.h>

#include <vector>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/descriptor.pb.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace java {

std::string EscapeJavadoc(const std::string& input) {
  std::string result;
  result.reserve(input.size() * 2);

  char prev = '*';

  for (std::string::size_type i = 0; i < input.size(); i++) {
    char c = input[i];
    switch (c) {
      case '*':
        // Avoid "/*".
        if (prev == '/') {
          result.append("&#42;");
        } else {
          result.push_back(c);
        }
        break;
      case '/':
        // Avoid "*/".
        if (prev == '*') {
          result.append("&#47;");
        } else {
          result.push_back(c);
        }
        break;
      case '@':
        // '@' starts javadoc tags including the @deprecated tag, which will
        // cause a compile-time error if inserted before a declaration that
        // does not have a corresponding @Deprecated annotation.
        result.append("&#64;");
        break;
      case '<':
        // Avoid interpretation as HTML.
        result.append("&lt;");
        break;
      case '>':
        // Avoid interpretation as HTML.
        result.append("&gt;");
        break;
      case '&':
        // Avoid interpretation as HTML.
        result.append("&amp;");
        break;
      case '\\':
        // Java interprets Unicode escape sequences anywhere!
        result.append("&#92;");
        break;
      default:
        result.push_back(c);
        break;
    }

    prev = c;
  }

  return result;
}

static void WriteDocCommentBodyForLocation(io::Printer* printer,
                                           const SourceLocation& location) {
  std::string comments = location.leading_comments.empty()
                             ? location.trailing_comments
                             : location.leading_comments;
  if (!comments.empty()) {
    // TODO(kenton):  Ideally we should parse the comment text as Markdown and
    //   write it back as HTML, but this requires a Markdown parser.  For now
    //   we just use <pre> to get fixed-width text formatting.

    // If the comment itself contains block comment start or end markers,
    // HTML-escape them so that they don't accidentally close the doc comment.
    comments = EscapeJavadoc(comments);

    std::vector<std::string> lines = Split(comments, "\n");
    while (!lines.empty() && lines.back().empty()) {
      lines.pop_back();
    }

    printer->Print(" * <pre>\n");
    for (int i = 0; i < lines.size(); i++) {
      // Most lines should start with a space.  Watch out for lines that start
      // with a /, since putting that right after the leading asterisk will
      // close the comment.
      if (!lines[i].empty() && lines[i][0] == '/') {
        printer->Print(" * $line$\n", "line", lines[i]);
      } else {
        printer->Print(" *$line$\n", "line", lines[i]);
      }
    }
    printer->Print(
        " * </pre>\n"
        " *\n");
  }
}

template <typename DescriptorType>
static void WriteDocCommentBody(io::Printer* printer,
                                const DescriptorType* descriptor) {
  SourceLocation location;
  if (descriptor->GetSourceLocation(&location)) {
    WriteDocCommentBodyForLocation(printer, location);
  }
}

static std::string FirstLineOf(const std::string& value) {
  std::string result = value;

  std::string::size_type pos = result.find_first_of('\n');
  if (pos != std::string::npos) {
    result.erase(pos);
  }

  // If line ends in an opening brace, make it "{ ... }" so it looks nice.
  if (!result.empty() && result[result.size() - 1] == '{') {
    result.append(" ... }");
  }

  return result;
}

void WriteMessageDocComment(io::Printer* printer, const Descriptor* message) {
  printer->Print("/**\n");
  WriteDocCommentBody(printer, message);
  printer->Print(
      " * Protobuf type {@code $fullname$}\n"
      " */\n",
      "fullname", EscapeJavadoc(message->full_name()));
}

void WriteFieldDocComment(io::Printer* printer, const FieldDescriptor* field) {
  // We start the comment with the main body based on the comments from the
  // .proto file (if present). We then continue with the field declaration,
  // e.g.:
  //   optional string foo = 5;
  // And then we end with the javadoc tags if applicable.
  // If the field is a group, the debug string might end with {.
  printer->Print("/**\n");
  WriteDocCommentBody(printer, field);
  printer->Print(" * <code>$def$</code>\n", "def",
                 EscapeJavadoc(FirstLineOf(field->DebugString())));
  printer->Print(" */\n");
}

void WriteDeprecatedJavadoc(io::Printer* printer, const FieldDescriptor* field,
                            const FieldAccessorType type) {
  if (!field->options().deprecated()) {
    return;
  }

  // Lite codegen does not annotate set & clear methods with @Deprecated.
  if (field->file()->options().optimize_for() == FileOptions::LITE_RUNTIME &&
      (type == SETTER || type == CLEARER)) {
    return;
  }

  std::string startLine = "0";
  SourceLocation location;
  if (field->GetSourceLocation(&location)) {
    startLine = std::to_string(location.start_line);
  }

  printer->Print(" * @deprecated $name$ is deprecated.\n", "name",
                 field->full_name());
  printer->Print(" *     See $file$;l=$line$\n", "file", field->file()->name(),
                 "line", startLine);
}

void WriteFieldAccessorDocComment(io::Printer* printer,
                                  const FieldDescriptor* field,
                                  const FieldAccessorType type,
                                  const bool builder) {
  printer->Print("/**\n");
  WriteDocCommentBody(printer, field);
  printer->Print(" * <code>$def$</code>\n", "def",
                 EscapeJavadoc(FirstLineOf(field->DebugString())));
  WriteDeprecatedJavadoc(printer, field, type);
  switch (type) {
    case HAZZER:
      printer->Print(" * @return Whether the $name$ field is set.\n", "name",
                     field->camelcase_name());
      break;
    case GETTER:
      printer->Print(" * @return The $name$.\n", "name",
                     field->camelcase_name());
      break;
    case SETTER:
      printer->Print(" * @param value The $name$ to set.\n", "name",
                     field->camelcase_name());
      break;
    case CLEARER:
      // Print nothing
      break;
    // Repeated
    case LIST_COUNT:
      printer->Print(" * @return The count of $name$.\n", "name",
                     field->camelcase_name());
      break;
    case LIST_GETTER:
      printer->Print(" * @return A list containing the $name$.\n", "name",
                     field->camelcase_name());
      break;
    case LIST_INDEXED_GETTER:
      printer->Print(" * @param index The index of the element to return.\n");
      printer->Print(" * @return The $name$ at the given index.\n", "name",
                     field->camelcase_name());
      break;
    case LIST_INDEXED_SETTER:
      printer->Print(" * @param index The index to set the value at.\n");
      printer->Print(" * @param value The $name$ to set.\n", "name",
                     field->camelcase_name());
      break;
    case LIST_ADDER:
      printer->Print(" * @param value The $name$ to add.\n", "name",
                     field->camelcase_name());
      break;
    case LIST_MULTI_ADDER:
      printer->Print(" * @param values The $name$ to add.\n", "name",
                     field->camelcase_name());
      break;
  }
  if (builder) {
    printer->Print(" * @return This builder for chaining.\n");
  }
  printer->Print(" */\n");
}

void WriteFieldEnumValueAccessorDocComment(io::Printer* printer,
                                           const FieldDescriptor* field,
                                           const FieldAccessorType type,
                                           const bool builder) {
  printer->Print("/**\n");
  WriteDocCommentBody(printer, field);
  printer->Print(" * <code>$def$</code>\n", "def",
                 EscapeJavadoc(FirstLineOf(field->DebugString())));
  WriteDeprecatedJavadoc(printer, field, type);
  switch (type) {
    case HAZZER:
      // Should never happen
      break;
    case GETTER:
      printer->Print(
          " * @return The enum numeric value on the wire for $name$.\n", "name",
          field->camelcase_name());
      break;
    case SETTER:
      printer->Print(
          " * @param value The enum numeric value on the wire for $name$ to "
          "set.\n",
          "name", field->camelcase_name());
      break;
    case CLEARER:
      // Print nothing
      break;
    // Repeated
    case LIST_COUNT:
      // Should never happen
      break;
    case LIST_GETTER:
      printer->Print(
          " * @return A list containing the enum numeric values on the wire "
          "for $name$.\n",
          "name", field->camelcase_name());
      break;
    case LIST_INDEXED_GETTER:
      printer->Print(" * @param index The index of the value to return.\n");
      printer->Print(
          " * @return The enum numeric value on the wire of $name$ at the "
          "given index.\n",
          "name", field->camelcase_name());
      break;
    case LIST_INDEXED_SETTER:
      printer->Print(" * @param index The index to set the value at.\n");
      printer->Print(
          " * @param value The enum numeric value on the wire for $name$ to "
          "set.\n",
          "name", field->camelcase_name());
      break;
    case LIST_ADDER:
      printer->Print(
          " * @param value The enum numeric value on the wire for $name$ to "
          "add.\n",
          "name", field->camelcase_name());
      break;
    case LIST_MULTI_ADDER:
      printer->Print(
          " * @param values The enum numeric values on the wire for $name$ to "
          "add.\n",
          "name", field->camelcase_name());
      break;
  }
  if (builder) {
    printer->Print(" * @return This builder for chaining.\n");
  }
  printer->Print(" */\n");
}

void WriteFieldStringBytesAccessorDocComment(io::Printer* printer,
                                             const FieldDescriptor* field,
                                             const FieldAccessorType type,
                                             const bool builder) {
  printer->Print("/**\n");
  WriteDocCommentBody(printer, field);
  printer->Print(" * <code>$def$</code>\n", "def",
                 EscapeJavadoc(FirstLineOf(field->DebugString())));
  WriteDeprecatedJavadoc(printer, field, type);
  switch (type) {
    case HAZZER:
      // Should never happen
      break;
    case GETTER:
      printer->Print(" * @return The bytes for $name$.\n", "name",
                     field->camelcase_name());
      break;
    case SETTER:
      printer->Print(" * @param value The bytes for $name$ to set.\n", "name",
                     field->camelcase_name());
      break;
    case CLEARER:
      // Print nothing
      break;
    // Repeated
    case LIST_COUNT:
      // Should never happen
      break;
    case LIST_GETTER:
      printer->Print(" * @return A list containing the bytes for $name$.\n",
                     "name", field->camelcase_name());
      break;
    case LIST_INDEXED_GETTER:
      printer->Print(" * @param index The index of the value to return.\n");
      printer->Print(" * @return The bytes of the $name$ at the given index.\n",
                     "name", field->camelcase_name());
      break;
    case LIST_INDEXED_SETTER:
      printer->Print(" * @param index The index to set the value at.\n");
      printer->Print(" * @param value The bytes of the $name$ to set.\n",
                     "name", field->camelcase_name());
      break;
    case LIST_ADDER:
      printer->Print(" * @param value The bytes of the $name$ to add.\n",
                     "name", field->camelcase_name());
      break;
    case LIST_MULTI_ADDER:
      printer->Print(" * @param values The bytes of the $name$ to add.\n",
                     "name", field->camelcase_name());
      break;
  }
  if (builder) {
    printer->Print(" * @return This builder for chaining.\n");
  }
  printer->Print(" */\n");
}

// Enum

void WriteEnumDocComment(io::Printer* printer, const EnumDescriptor* enum_) {
  printer->Print("/**\n");
  WriteDocCommentBody(printer, enum_);
  printer->Print(
      " * Protobuf enum {@code $fullname$}\n"
      " */\n",
      "fullname", EscapeJavadoc(enum_->full_name()));
}

void WriteEnumValueDocComment(io::Printer* printer,
                              const EnumValueDescriptor* value) {
  printer->Print("/**\n");
  WriteDocCommentBody(printer, value);
  printer->Print(
      " * <code>$def$</code>\n"
      " */\n",
      "def", EscapeJavadoc(FirstLineOf(value->DebugString())));
}

void WriteServiceDocComment(io::Printer* printer,
                            const ServiceDescriptor* service) {
  printer->Print("/**\n");
  WriteDocCommentBody(printer, service);
  printer->Print(
      " * Protobuf service {@code $fullname$}\n"
      " */\n",
      "fullname", EscapeJavadoc(service->full_name()));
}

void WriteMethodDocComment(io::Printer* printer,
                           const MethodDescriptor* method) {
  printer->Print("/**\n");
  WriteDocCommentBody(printer, method);
  printer->Print(
      " * <code>$def$</code>\n"
      " */\n",
      "def", EscapeJavadoc(FirstLineOf(method->DebugString())));
}

}  // namespace java
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
