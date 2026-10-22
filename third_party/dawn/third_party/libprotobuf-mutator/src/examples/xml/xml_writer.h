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

#ifndef EXAMPLES_XML_XML_WRITER_H_
#define EXAMPLES_XML_XML_WRITER_H_

#include <string>

namespace protobuf_mutator {
namespace xml {

// Converts protobuf into XML.
std::string MessageToXml(const class Document& document);

}  // namespace xml
}  // namespace protobuf_mutator

#endif  // EXAMPLES_XML_XML_WRITER_H_
