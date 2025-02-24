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

#include "examples/xml/xml_writer.h"

#include <algorithm>
#include <sstream>

#include "examples/xml/xml.pb.h"

namespace protobuf_mutator {
namespace xml {

namespace {

class XmlWriter {
 public:
  XmlWriter();
  std::string ToXml(const Document& doc);

 private:
  void ToXml(const std::string& name, const std::string& value);
  void ToXml(const Element& element);
  void ToXml(const Content& content);
  void ToXml(const Misk& misk);
  void ToXml(const DoctypeDecl& doctype);

  std::ostringstream out_;
};

XmlWriter::XmlWriter() {}

void XmlWriter::ToXml(const std::string& name, const std::string& value) {
  char quote = (name.size() % 2) ? '"' : '\'';
  out_ << " " << name << "=" << quote << value << quote;
}

void XmlWriter::ToXml(const Misk& misk) {
  if (misk.has_pi()) {
    out_ << "<?" << misk.pi().target() << misk.pi().data() << "?>";
  }

  if (misk.has_comment()) {
    out_ << "<!--" << misk.comment() << "-->";
  }
}

void XmlWriter::ToXml(const DoctypeDecl& doctype) {
  out_ << "<!DOCTYPE " << doctype.name();
  if (doctype.has_external_id()) out_ << " " << doctype.external_id();
  if (doctype.has_int_subset()) out_ << " [" << doctype.int_subset() << "]";
  for (int i = 0; i < doctype.misk_size(); ++i) ToXml(doctype.misk(i));
  out_ << ">";
}

void XmlWriter::ToXml(const Content& content) {
  if (content.has_char_data()) out_ << content.char_data();
  if (content.has_element()) ToXml(content.element());
  if (content.has_reference()) {
    out_ << (content.reference().entry() ? '&' : '%')
         << content.reference().name() << ';';
  }
  if (content.has_cdsect()) out_ << "<![CDATA[" << content.cdsect() << "]]>";

  if (content.has_misk()) ToXml(content.misk());
}

void XmlWriter::ToXml(const Element& element) {
  std::string tag;
  std::string name;
  tag += element.tag().name();
  out_ << "<" << tag;

  for (int i = 0; i < element.tag().attribute_size(); ++i) {
    ToXml(element.tag().attribute(i).name(),
          element.tag().attribute(i).value());
  }

  if (element.content_size() == 0) {
    out_ << "/>";
  } else {
    out_ << ">";
    for (int i = 0; i < element.content_size(); ++i) ToXml(element.content(i));
    out_ << "</" << tag << ">";
  }
}

std::string XmlWriter::ToXml(const Document& doc) {
  out_.str("");

  if (doc.has_version() || doc.has_encoding() || doc.has_standalone()) {
    out_ << "<?xml";
    if (doc.has_version())
      ToXml("version", (doc.version().size() == 7) ? "1.0" : doc.version());
    if (doc.has_encoding()) ToXml("encoding", doc.encoding());
    if (doc.has_standalone())
      ToXml("encoding", doc.standalone() ? "yes" : "no");
    out_ << "?>";
  }

  for (int i = 0; i < doc.misk1_size(); ++i) ToXml(doc.misk1(i));
  if (doc.has_doctype()) ToXml(doc.doctype());
  ToXml(doc.element());
  for (int i = 0; i < doc.misk2_size(); ++i) ToXml(doc.misk2(i));

  return out_.str();
}

}  // namespace

std::string MessageToXml(const Document& document) {
  XmlWriter writer;
  return writer.ToXml(document);
}

}  // namespace xml
}  // namespace protobuf_mutator
