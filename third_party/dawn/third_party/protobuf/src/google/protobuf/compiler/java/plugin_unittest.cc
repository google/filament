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

#include <memory>
#include <string>

#include <google/protobuf/testing/file.h>
#include <google/protobuf/testing/file.h>
#include <google/protobuf/compiler/java/generator.h>
#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/testing/googletest.h>
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace java {
namespace {

class TestGenerator : public CodeGenerator {
 public:
  TestGenerator() {}
  ~TestGenerator() override {}

  bool Generate(const FileDescriptor* file, const std::string& parameter,
                GeneratorContext* context, std::string* error) const override {
    std::string filename = "Test.java";
    TryInsert(filename, "outer_class_scope", context);
    TryInsert(filename, "class_scope:foo.Bar", context);
    TryInsert(filename, "class_scope:foo.Bar.Baz", context);
    TryInsert(filename, "builder_scope:foo.Bar", context);
    TryInsert(filename, "builder_scope:foo.Bar.Baz", context);
    TryInsert(filename, "enum_scope:foo.Qux", context);
    return true;
  }

  void TryInsert(const std::string& filename,
                 const std::string& insertion_point,
                 GeneratorContext* context) const {
    std::unique_ptr<io::ZeroCopyOutputStream> output(
        context->OpenForInsert(filename, insertion_point));
    io::Printer printer(output.get(), '$');
    printer.Print("// inserted $name$\n", "name", insertion_point);
  }
};

// This test verifies that all the expected insertion points exist.  It does
// not verify that they are correctly-placed; that would require actually
// compiling the output which is a bit more than I care to do for this test.
TEST(JavaPluginTest, PluginTest) {
  GOOGLE_CHECK_OK(File::SetContents(TestTempDir() + "/test.proto",
                             "syntax = \"proto2\";\n"
                             "package foo;\n"
                             "option java_package = \"\";\n"
                             "option java_outer_classname = \"Test\";\n"
                             "message Bar {\n"
                             "  message Baz {}\n"
                             "}\n"
                             "enum Qux { BLAH = 1; }\n",
                             true));

  CommandLineInterface cli;
  cli.SetInputsAreProtoPathRelative(true);

  JavaGenerator java_generator;
  TestGenerator test_generator;
  cli.RegisterGenerator("--java_out", &java_generator, "");
  cli.RegisterGenerator("--test_out", &test_generator, "");

  std::string proto_path = "-I" + TestTempDir();
  std::string java_out = "--java_out=" + TestTempDir();
  std::string test_out = "--test_out=" + TestTempDir();

  const char* argv[] = {"protoc", proto_path.c_str(), java_out.c_str(),
                        test_out.c_str(), "test.proto"};

  EXPECT_EQ(0, cli.Run(5, argv));

  // Loop over the lines of the generated code and verify that we find what we
  // expect

  std::string output;
  GOOGLE_CHECK_OK(File::GetContents(TestTempDir() + "/Test.java", &output,
                             true));
  std::vector<std::string> lines = Split(output, "\n");
  bool found_generated_annotation = false;
  bool found_do_not_edit = false;
  for (const auto& line : lines) {
    if (line.find(" DO NOT EDIT!") != std::string::npos) {
      found_do_not_edit = true;
    }
    if (line.find("@com.google.protobuf.Generated") != std::string::npos) {
      found_generated_annotation = true;
    }
  }
  EXPECT_TRUE(found_do_not_edit);
  (void)found_generated_annotation;
}

}  // namespace
}  // namespace java
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
