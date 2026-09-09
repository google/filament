// Copyright 2017 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PORT_PROTOBUF_H_
#define PORT_PROTOBUF_H_

#include <string>

#include "google/protobuf/any.pb.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/message.h"
#include "google/protobuf/stubs/common.h"  // for GOOGLE_PROTOBUF_VERSION
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/wire_format.h"

namespace google {
namespace protobuf {
#if GOOGLE_PROTOBUF_VERSION < 4025000

template <typename T>
const T* DownCastMessage(const Message* message) {
  return static_cast<const T*>(message);
}

template <typename T>
T* DownCastMessage(Message* message) {
  const Message* message_const = message;
  return const_cast<T*>(DownCastMessage<T>(message_const));
}

#elif GOOGLE_PROTOBUF_VERSION < 5029000

template <typename T>
const T* DownCastMessage(const Message* message) {
  return DownCastToGenerated<T>(message);
}

template <typename T>
T* DownCastMessage(Message* message) {
  return DownCastToGenerated<T>(message);
}

#endif  // GOOGLE_PROTOBUF_VERSION
}  // namespace protobuf
}  // namespace google

namespace protobuf_mutator {

namespace protobuf = google::protobuf;

inline bool RequiresUtf8Validation(
    const google::protobuf::FieldDescriptor& descriptor) {
  // commit d85c9944c55fb38f4eae149979a0f680ea125ecb of >= v3(!).22.0
#if GOOGLE_PROTOBUF_VERSION >= 4022000
  return descriptor.requires_utf8_validation();
#else
  return descriptor.type() == google::protobuf::FieldDescriptor::TYPE_STRING &&
         descriptor.file()->syntax() ==
             google::protobuf::FileDescriptor::SYNTAX_PROTO3;
#endif
}

inline bool HasPresence(const google::protobuf::FieldDescriptor& descriptor) {
  // commit bb30225f06c36399757dc698b409d5f79738e8d1 of >=3.12.0
#if GOOGLE_PROTOBUF_VERSION >= 3012000
  return descriptor.has_presence();
#else
  // NOTE: This mimics Protobuf 3.21.12 ("3021012")
  return !descriptor.is_repeated() &&
         (descriptor.cpp_type() ==
              google::protobuf::FieldDescriptor::CppType::CPPTYPE_MESSAGE ||
          descriptor.containing_oneof() ||
          descriptor.file()->syntax() ==
              google::protobuf::FileDescriptor::SYNTAX_PROTO2);
#endif
}

inline void PrepareTextParser(google::protobuf::TextFormat::Parser& parser) {
  // commit d8c2501b43c1b56e3efa74048a18f8ce06ba07fe of >=3.8.0 for .SetRecursionLimit
  // commit 176f7db11d8242b36a3ea6abb1cc436fca5bf75d of >=3.8.0 for .AllowUnknownField
#if GOOGLE_PROTOBUF_VERSION >= 3008000
  parser.SetRecursionLimit(100);
  parser.AllowUnknownField(true);
#endif
}

constexpr bool TextParserCanSetRecursionLimit() {
  // commit d8c2501b43c1b56e3efa74048a18f8ce06ba07fe of >=3.8.0
  return GOOGLE_PROTOBUF_VERSION >= 3008000;
}

constexpr bool TextParserCanAllowUnknownField() {
  // commit 176f7db11d8242b36a3ea6abb1cc436fca5bf75d of >=3.8.0
  return GOOGLE_PROTOBUF_VERSION >= 3008000;
}

}  // namespace protobuf_mutator

#endif  // PORT_PROTOBUF_H_
