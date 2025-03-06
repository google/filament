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

#include <getopt.h>

#include <fstream>
#include <iostream>

#include "examples/xml/xml.pb.h"
#include "examples/xml/xml_writer.h"
#include "port/protobuf.h"
#include "src/text_format.h"

using protobuf_mutator::xml::Input;

namespace {

struct option const kLongOptions[] = {{"reverse", no_argument, NULL, 'r'},
                                      {"verbose", no_argument, NULL, 'v'},
                                      {"help", no_argument, NULL, 'h'},
                                      {NULL, 0, NULL, 0}};

void PrintUsage() {
  std::cerr << "Usage: xml_converter [OPTION]... [INFILE [OUTFILE]]\n"
            << "Converts between proto used by fuzzer and XML.\n\n"
            << "\t-h, --help\tPrint this help\n"
            << "\t-r, --reverse\tConverts from XML to proto\n"
            << "\t-v, --verbose\tPrint input\n";
}

struct Options {
  bool reverse = false;
  bool verbose = false;
  std::string in_file;
  std::string out_file;
};

bool ParseOptions(int argc, char** argv, Options* options) {
  int c = 0;
  while ((c = getopt_long(argc, argv, "hrv", kLongOptions, nullptr)) != -1) {
    switch (c) {
      case 'v':
        options->verbose = true;
        break;
      case 'r':
        options->reverse = true;
        break;
      case 'h':
      default:
        return false;
    }
  }

  int i = optind;
  if (i < argc) options->in_file = argv[i++];
  if (i < argc) options->out_file = argv[i++];
  if (i != argc) return false;

  return true;
}

}  // namespace

int main(int argc, char** argv) {
  Options options;
  if (!ParseOptions(argc, argv, &options)) {
    PrintUsage();
    return 1;
  }

  std::istream* cin = &std::cin;
  std::ostream* cout = &std::cout;

  std::ifstream in_file_stream;
  if (!options.in_file.empty()) {
    in_file_stream.open(options.in_file);
    cin = &in_file_stream;
  }

  std::ofstream out_file_stream;
  if (!options.out_file.empty()) {
    out_file_stream.open(options.out_file);
    cout = &out_file_stream;
  }

  std::string input;
  std::vector<char> buff(1 << 20);
  while (auto size = cin->readsome(buff.data(), buff.size())) {
    input += std::string(buff.data(), size);
  }
  std::string output;

  int ret = 0;

  if (options.reverse) {
    Input message;
    message.mutable_document()->mutable_element()->add_content()->set_char_data(
        input);
    output = protobuf_mutator::SaveMessageAsText(message);
  } else {
    Input message;
    bool is_proto = protobuf_mutator::ParseTextMessage(input.data(), &message);
    output = MessageToXml(message.document());
    if (!is_proto) {
      ret = 2;
      if (options.verbose) std::cerr << "Input is not proto\n";
    }
  }

  if (options.verbose) {
    std::cerr << input << "\n\n";
    std::cerr.flush();
  }
  *cout << output;

  return ret;
}
