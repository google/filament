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

#include "libxml/parser.h"
#include "libxml/xmlsave.h"

#include "examples/xml/xml.pb.h"
#include "examples/xml/xml_writer.h"
#include "src/libfuzzer/libfuzzer_macro.h"

namespace {
protobuf_mutator::protobuf::LogSilencer log_silincer;
void ignore(void* ctx, const char* msg, ...) {}

template <class T, class D>
std::unique_ptr<T, D> MakeUnique(T* obj, D del) {
  return {obj, del};
}
}

DEFINE_PROTO_FUZZER(const protobuf_mutator::xml::Input& message) {
  std::string xml = MessageToXml(message.document());
  int options = message.options();

  // Network requests are too slow.
  options |= XML_PARSE_NONET;
  // These flags can cause network or file access and hangs.
  options &= ~(XML_PARSE_NOENT | XML_PARSE_HUGE | XML_PARSE_DTDVALID |
               XML_PARSE_DTDLOAD | XML_PARSE_DTDATTR);

  xmlSetGenericErrorFunc(nullptr, &ignore);

  if (auto doc =
          MakeUnique(xmlReadMemory(xml.c_str(), static_cast<int>(xml.size()),
                                   "", nullptr, options),
                     &xmlFreeDoc)) {
    auto buf = MakeUnique(xmlBufferCreate(), &xmlBufferFree);
    auto ctxt =
        MakeUnique(xmlSaveToBuffer(buf.get(), nullptr, 0), &xmlSaveClose);
    xmlSaveDoc(ctxt.get(), doc.get());
  }
}
