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

#include <vector>

#include "expat.h"  // NOLINT

#include "examples/xml/xml.pb.h"
#include "examples/xml/xml_writer.h"
#include "src/libfuzzer/libfuzzer_macro.h"

namespace {
protobuf_mutator::protobuf::LogSilencer log_silincer;
std::vector<const char*> kEncodings = {{"UTF-16", "UTF-8", "ISO-8859-1",
                                        "US-ASCII", "UTF-16BE", "UTF-16LE",
                                        "INVALIDENCODING"}};
}

DEFINE_PROTO_FUZZER(const protobuf_mutator::xml::Input& message) {
  std::string xml = MessageToXml(message.document());
  int options = message.options();

  int use_ns = options % 2;
  options /= 2;
  auto enc = kEncodings[options % kEncodings.size()];
  XML_Parser parser =
      use_ns ? XML_ParserCreateNS(enc, '\n') : XML_ParserCreate(enc);
  XML_Parse(parser, xml.data(), xml.size(), true);
  XML_ParserFree(parser);
}
