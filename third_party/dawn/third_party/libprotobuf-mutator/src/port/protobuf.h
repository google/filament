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
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/wire_format.h"

// clang-format off
#include "google/protobuf/port_def.inc"  // MUST be last header included
// clang-format on
namespace google {
namespace protobuf {
#if PROTOBUF_VERSION < 4025000

template <typename T>
const T* DownCastMessage(const Message* message) {
  return static_cast<const T*>(message);
}

template <typename T>
T* DownCastMessage(Message* message) {
  const Message* message_const = message;
  return const_cast<T*>(DownCastMessage<T>(message_const));
}

#elif PROTOBUF_VERSION < 5029000

template <typename T>
const T* DownCastMessage(const Message* message) {
  return DownCastToGenerated<T>(message);
}

template <typename T>
T* DownCastMessage(Message* message) {
  return DownCastToGenerated<T>(message);
}

#endif  // PROTOBUF_VERSION
}  // namespace protobuf
}  // namespace google
#include "google/protobuf/port_undef.inc"


namespace protobuf_mutator {

namespace protobuf = google::protobuf;

}  // namespace protobuf_mutator

#endif  // PORT_PROTOBUF_H_
