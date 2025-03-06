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

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <google/protobuf/compiler/objectivec/objectivec_generator.h>
#include <google/protobuf/compiler/objectivec/objectivec_file.h>
#include <google/protobuf/compiler/objectivec/objectivec_helpers.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/strutil.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace objectivec {

namespace {

// Convert a string with "yes"/"no" (case insensitive) to a boolean, returning
// true/false for if the input string was a valid value. If the input string is
// invalid, `result` is unchanged.
bool StringToBool(const std::string& value, bool* result) {
  std::string upper_value(value);
  UpperString(&upper_value);
  if (upper_value == "NO") {
    *result = false;
    return true;
  }
  if (upper_value == "YES") {
    *result = true;
    return true;
  }

  return false;
}

}  // namespace

ObjectiveCGenerator::ObjectiveCGenerator() {}

ObjectiveCGenerator::~ObjectiveCGenerator() {}

bool ObjectiveCGenerator::HasGenerateAll() const {
  return true;
}

bool ObjectiveCGenerator::Generate(const FileDescriptor* file,
                                   const std::string& parameter,
                                   GeneratorContext* context,
                                   std::string* error) const {
  *error = "Unimplemented Generate() method. Call GenerateAll() instead.";
  return false;
}

bool ObjectiveCGenerator::GenerateAll(
    const std::vector<const FileDescriptor*>& files,
    const std::string& parameter, GeneratorContext* context,
    std::string* error) const {
  // -----------------------------------------------------------------
  // Parse generator options. These options are passed to the compiler using the
  // --objc_opt flag. The options are passed as a comma separated list of
  // options along with their values. If the option appears multiple times, only
  // the last value will be considered.
  //
  // e.g. protoc ... --objc_opt=expected_prefixes=file.txt,generate_for_named_framework=MyFramework

  Options validation_options;
  FileGenerator::GenerationOptions generation_options;

  std::vector<std::pair<std::string, std::string> > options;
  ParseGeneratorParameter(parameter, &options);
  for (int i = 0; i < options.size(); i++) {
    if (options[i].first == "expected_prefixes_path") {
      // Path to find a file containing the expected prefixes
      // (objc_class_prefix "PREFIX") for proto packages (package NAME). The
      // generator will then issue warnings/errors if in the proto files being
      // generated the option is not listed/wrong/etc in the file.
      //
      // The format of the file is:
      //   - An entry is a line of "package=prefix".
      //   - Comments start with "#".
      //   - A comment can go on a line after a expected package/prefix pair.
      //     (i.e. - "package=prefix # comment")
      //   - For files that do NOT have a proto package (not recommended), an
      //     entry can be made as "no_package:PATH=prefix", where PATH is the
      //     path for the .proto file.
      //
      // There is no validation that the prefixes are good prefixes, it is
      // assumed that they are when you create the file.
      validation_options.expected_prefixes_path = options[i].second;
    } else if (options[i].first == "expected_prefixes_suppressions") {
      // A semicolon delimited string that lists the paths of .proto files to
      // exclude from the package prefix validations (expected_prefixes_path).
      // This is provided as an "out", to skip some files being checked.
      for (StringPiece split_piece : Split(
               options[i].second, ";", true)) {
        validation_options.expected_prefixes_suppressions.push_back(
            std::string(split_piece));
      }
    } else if (options[i].first == "prefixes_must_be_registered") {
      // If objc prefix file option value must be registered to be used. This
      // option has no meaning if an "expected_prefixes_path" isn't set. The
      // available options are:
      //   "no": They don't have to be registered.
      //   "yes": They must be registered and an error will be raised if a files
      //     tried to use a prefix that isn't registered.
      // Default is "no".
      if (!StringToBool(options[i].second,
                        &validation_options.prefixes_must_be_registered)) {
        *error = "error: Unknown value for prefixes_must_be_registered: " + options[i].second;
        return false;
      }
    } else if (options[i].first == "require_prefixes") {
      // If every file must have an objc prefix file option to be used. The
      // available options are:
      //   "no": Files can be generated without the prefix option.
      //   "yes": Files must have the objc prefix option, and an error will be
      //     raised if a files doesn't have one.
      // Default is "no".
      if (!StringToBool(options[i].second,
                        &validation_options.require_prefixes)) {
        *error = "error: Unknown value for require_prefixes: " + options[i].second;
        return false;
      }
    } else if (options[i].first == "generate_for_named_framework") {
      // The name of the framework that protos are being generated for. This
      // will cause the #import statements to be framework based using this
      // name (i.e. - "#import <NAME/proto.pbobjc.h>).
      //
      // NOTE: If this option is used with
      // named_framework_to_proto_path_mappings_path, then this is effectively
      // the "default" framework name used for everything that wasn't mapped by
      // the mapping file.
      generation_options.generate_for_named_framework = options[i].second;
    } else if (options[i].first == "named_framework_to_proto_path_mappings_path") {
      // Path to find a file containing the list of framework names and proto
      // files. The generator uses this to decide if a proto file
      // referenced should use a framework style import vs. a user level import
      // (#import <FRAMEWORK/file.pbobjc.h> vs #import "dir/file.pbobjc.h").
      //
      // The format of the file is:
      //   - An entry is a line of "frameworkName: file.proto, dir/file2.proto".
      //   - Comments start with "#".
      //   - A comment can go on a line after a expected package/prefix pair.
      //     (i.e. - "frameworkName: file.proto # comment")
      //
      // Any number of files can be listed for a framework, just separate them
      // with commas.
      //
      // There can be multiple lines listing the same frameworkName in case it
      // has a lot of proto files included in it; having multiple lines makes
      // things easier to read. If a proto file is not configured in the
      // mappings file, it will use the default framework name if one was passed
      // with generate_for_named_framework, or the relative path to it's include
      // path otherwise.
      generation_options.named_framework_to_proto_path_mappings_path = options[i].second;
    } else if (options[i].first == "runtime_import_prefix") {
      // Path to use as a prefix on #imports of runtime provided headers in the
      // generated files. When integrating ObjC protos into a build system,
      // this can be used to avoid having to add the runtime directory to the
      // header search path since the generate #import will be more complete.
      generation_options.runtime_import_prefix = StripSuffixString(options[i].second, "/");
    } else if (options[i].first == "package_to_prefix_mappings_path") {
      // Path to use for when loading the objc class prefix mappings to use.
      // The `objc_class_prefix` file option is always honored first if one is present.
      // This option also has precedent over the use_package_as_prefix option.
      //
      // The format of the file is:
      //   - An entry is a line of "package=prefix".
      //   - Comments start with "#".
      //   - A comment can go on a line after a expected package/prefix pair.
      //     (i.e. - "package=prefix # comment")
      //   - For files that do NOT have a proto package (not recommended), an
      //     entry can be made as "no_package:PATH=prefix", where PATH is the
      //     path for the .proto file.
      //
      SetPackageToPrefixMappingsPath(options[i].second);
    } else if (options[i].first == "use_package_as_prefix") {
      // Controls how the symbols should be prefixed to avoid symbols
      // collisions. The objc_class_prefix file option is always honored, this
      // is just what to do if that isn't set. The available options are:
      //   "no": Not prefixed (the existing mode).
      //   "yes": Make a prefix out of the proto package.
      bool value = false;
      if (StringToBool(options[i].second, &value)) {
        SetUseProtoPackageAsDefaultPrefix(value);
      } else {
        *error = "error: Unknown use_package_as_prefix: " + options[i].second;
        return false;
      }
    } else if (options[i].first == "proto_package_prefix_exceptions_path") {
      // Path to find a file containing the list of proto package names that are
      // exceptions when use_package_as_prefix is enabled. This can be used to
      // migrate packages one at a time to use_package_as_prefix since there
      // are likely code updates needed with each one.
      //
      // The format of the file is:
      //   - An entry is a line of "proto.package.name".
      //   - Comments start with "#".
      //   - A comment can go on a line after a expected package/prefix pair.
      //     (i.e. - "some.proto.package # comment")
      SetProtoPackagePrefixExceptionList(options[i].second);
    } else if (options[i].first == "headers_use_forward_declarations") {
      if (!StringToBool(options[i].second,
                        &generation_options.headers_use_forward_declarations)) {
        *error = "error: Unknown value for headers_use_forward_declarations: " + options[i].second;
        return false;
      }
    } else {
      *error = "error: Unknown generator option: " + options[i].first;
      return false;
    }
  }

  // -----------------------------------------------------------------

  // These are not official generation options and could be removed/changed in
  // the future and doing that won't count as a breaking change.
  bool headers_only = getenv("GPB_OBJC_HEADERS_ONLY") != NULL;
  std::unordered_set<std::string> skip_impls;
  if (getenv("GPB_OBJC_SKIP_IMPLS_FILE") != NULL) {
    std::ifstream skip_file(getenv("GPB_OBJC_SKIP_IMPLS_FILE"));
    if (skip_file.is_open()) {
      std::string line;
      while (std::getline(skip_file, line)) {
        skip_impls.insert(line);
      }
    } else {
      *error = "error: Failed to open GPB_OBJC_SKIP_IMPLS_FILE file";
      return false;
    }
  }

  // -----------------------------------------------------------------

  // Validate the objc prefix/package pairings.
  if (!ValidateObjCClassPrefixes(files, validation_options, error)) {
    // *error will have been filled in.
    return false;
  }

  FileGenerator::CommonState state;
  for (int i = 0; i < files.size(); i++) {
    const FileDescriptor* file = files[i];
    FileGenerator file_generator(file, generation_options, state);
    std::string filepath = FilePath(file);

    // Generate header.
    {
      std::unique_ptr<io::ZeroCopyOutputStream> output(
          context->Open(filepath + ".pbobjc.h"));
      io::Printer printer(output.get(), '$');
      file_generator.GenerateHeader(&printer);
    }

    // Generate m file.
    if (!headers_only && skip_impls.count(file->name()) == 0) {
      std::unique_ptr<io::ZeroCopyOutputStream> output(
          context->Open(filepath + ".pbobjc.m"));
      io::Printer printer(output.get(), '$');
      file_generator.GenerateSource(&printer);
    }
  }

  return true;
}

}  // namespace objectivec
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
