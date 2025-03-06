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

#include <google/protobuf/compiler/cpp/file.h>

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <google/protobuf/compiler/scc.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/compiler/cpp/enum.h>
#include <google/protobuf/compiler/cpp/extension.h>
#include <google/protobuf/compiler/cpp/field.h>
#include <google/protobuf/compiler/cpp/helpers.h>
#include <google/protobuf/compiler/cpp/message.h>
#include <google/protobuf/compiler/cpp/service.h>
#include <google/protobuf/descriptor.pb.h>

// Must be last.
#include <google/protobuf/port_def.inc>

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

namespace {

// When we forward-declare things, we want to create a sorted order so our
// output is deterministic and minimizes namespace changes.
template <class T>
std::string GetSortKey(const T& val) {
  return val.full_name();
}

template <>
std::string GetSortKey<FileDescriptor>(const FileDescriptor& val) {
  return val.name();
}

template <class T>
bool CompareSortKeys(const T* a, const T* b) {
  return GetSortKey(*a) < GetSortKey(*b);
}

template <class T>
std::vector<const T*> Sorted(const std::unordered_set<const T*>& vals) {
  std::vector<const T*> sorted(vals.begin(), vals.end());
  std::sort(sorted.begin(), sorted.end(), CompareSortKeys<T>);
  return sorted;
}

// TODO(b/203101078): remove pragmas that suppresses uninitialized warnings when
// clang bug is fixed.
inline void MuteWuninitialized(Formatter& format) {
  format(
      "#if defined(__llvm__)\n"
      "  #pragma clang diagnostic push\n"
      "  #pragma clang diagnostic ignored \"-Wuninitialized\"\n"
      "#endif  // __llvm__\n");
}

inline void UnmuteWuninitialized(Formatter& format) {
  format(
      "#if defined(__llvm__)\n"
      "  #pragma clang diagnostic pop\n"
      "#endif  // __llvm__\n");
}

}  // namespace

FileGenerator::FileGenerator(const FileDescriptor* file, const Options& options)
    : file_(file), options_(options), scc_analyzer_(options) {
  // These variables are the same on a file level
  SetCommonVars(options, &variables_);
  variables_["dllexport_decl"] = options.dllexport_decl;
  variables_["tablename"] = UniqueName("TableStruct", file_, options_);
  variables_["file_level_metadata"] =
      UniqueName("file_level_metadata", file_, options_);
  variables_["desc_table"] = DescriptorTableName(file_, options_);
  variables_["file_level_enum_descriptors"] =
      UniqueName("file_level_enum_descriptors", file_, options_);
  variables_["file_level_service_descriptors"] =
      UniqueName("file_level_service_descriptors", file_, options_);
  variables_["filename"] = file_->name();
  variables_["package_ns"] = Namespace(file_, options);

  std::vector<const Descriptor*> msgs = FlattenMessagesInFile(file);
  for (int i = 0; i < msgs.size(); i++) {
    // Deleted in destructor
    MessageGenerator* msg_gen =
        new MessageGenerator(msgs[i], variables_, i, options, &scc_analyzer_);
    message_generators_.emplace_back(msg_gen);
    msg_gen->AddGenerators(&enum_generators_, &extension_generators_);
  }

  for (int i = 0; i < file->enum_type_count(); i++) {
    enum_generators_.emplace_back(
        new EnumGenerator(file->enum_type(i), variables_, options));
  }

  for (int i = 0; i < file->service_count(); i++) {
    service_generators_.emplace_back(
        new ServiceGenerator(file->service(i), variables_, options));
  }
  if (HasGenericServices(file_, options_)) {
    for (int i = 0; i < service_generators_.size(); i++) {
      service_generators_[i]->index_in_metadata_ = i;
    }
  }
  for (int i = 0; i < file->extension_count(); i++) {
    extension_generators_.emplace_back(
        new ExtensionGenerator(file->extension(i), options, &scc_analyzer_));
  }
  for (int i = 0; i < file->weak_dependency_count(); ++i) {
    weak_deps_.insert(file->weak_dependency(i));
  }
}

FileGenerator::~FileGenerator() = default;

void FileGenerator::GenerateMacroUndefs(io::Printer* printer) {
  Formatter format(printer, variables_);
  // Only do this for protobuf's own types. There are some google3 protos using
  // macros as field names and the generated code compiles after the macro
  // expansion. Undefing these macros actually breaks such code.
  if (file_->name() != "net/proto2/compiler/proto/plugin.proto" &&
      file_->name() != "google/protobuf/compiler/plugin.proto") {
    return;
  }
  std::vector<std::string> names_to_undef;
  std::vector<const FieldDescriptor*> fields;
  ListAllFields(file_, &fields);
  for (int i = 0; i < fields.size(); i++) {
    const std::string& name = fields[i]->name();
    static const char* kMacroNames[] = {"major", "minor"};
    for (int j = 0; j < GOOGLE_ARRAYSIZE(kMacroNames); ++j) {
      if (name == kMacroNames[j]) {
        names_to_undef.push_back(name);
        break;
      }
    }
  }
  for (int i = 0; i < names_to_undef.size(); ++i) {
    format(
        "#ifdef $1$\n"
        "#undef $1$\n"
        "#endif\n",
        names_to_undef[i]);
  }
}

void FileGenerator::GenerateHeader(io::Printer* printer) {
  Formatter format(printer, variables_);

  // port_def.inc must be included after all other includes.
  IncludeFile("net/proto2/public/port_def.inc", printer);
  format("#define $1$$ dllexport_decl$\n", FileDllExport(file_, options_));
  GenerateMacroUndefs(printer);

  // For Any support with lite protos, we need to friend AnyMetadata, so we
  // forward-declare it here.
  format(
      "PROTOBUF_NAMESPACE_OPEN\n"
      "namespace internal {\n"
      "class AnyMetadata;\n"
      "}  // namespace internal\n"
      "PROTOBUF_NAMESPACE_CLOSE\n");

  GenerateGlobalStateFunctionDeclarations(printer);

  GenerateForwardDeclarations(printer);

  {
    NamespaceOpener ns(Namespace(file_, options_), format);

    format("\n");

    GenerateEnumDefinitions(printer);

    format(kThickSeparator);
    format("\n");

    GenerateMessageDefinitions(printer);

    format("\n");
    format(kThickSeparator);
    format("\n");

    GenerateServiceDefinitions(printer);

    GenerateExtensionIdentifiers(printer);

    format("\n");
    format(kThickSeparator);
    format("\n");

    GenerateInlineFunctionDefinitions(printer);

    format(
        "\n"
        "// @@protoc_insertion_point(namespace_scope)\n"
        "\n");
  }

  // We need to specialize some templates in the ::google::protobuf namespace:
  GenerateProto2NamespaceEnumSpecializations(printer);

  format(
      "\n"
      "// @@protoc_insertion_point(global_scope)\n"
      "\n");
  IncludeFile("net/proto2/public/port_undef.inc", printer);
}

void FileGenerator::GenerateProtoHeader(io::Printer* printer,
                                        const std::string& info_path) {
  Formatter format(printer, variables_);
  if (!options_.proto_h) {
    return;
  }

  GenerateTopHeaderGuard(printer, false);

  if (!options_.opensource_runtime) {
    format(
        "#ifdef SWIG\n"
        "#error \"Do not SWIG-wrap protobufs.\"\n"
        "#endif  // SWIG\n"
        "\n");
  }

  if (IsBootstrapProto(options_, file_)) {
    format("// IWYU pragma: private, include \"$1$.proto.h\"\n\n",
           StripProto(file_->name()));
  }

  GenerateLibraryIncludes(printer);

  for (int i = 0; i < file_->public_dependency_count(); i++) {
    const FileDescriptor* dep = file_->public_dependency(i);
    format("#include \"$1$.proto.h\"\n", StripProto(dep->name()));
  }

  format("// @@protoc_insertion_point(includes)\n");

  GenerateMetadataPragma(printer, info_path);

  GenerateHeader(printer);

  GenerateBottomHeaderGuard(printer, false);
}

void FileGenerator::GeneratePBHeader(io::Printer* printer,
                                     const std::string& info_path) {
  Formatter format(printer, variables_);
  GenerateTopHeaderGuard(printer, true);

  if (options_.proto_h) {
    std::string target_basename = StripProto(file_->name());
    if (!options_.opensource_runtime) {
      GetBootstrapBasename(options_, target_basename, &target_basename);
    }
    format("#include \"$1$.proto.h\"  // IWYU pragma: export\n",
           target_basename);
  } else {
    GenerateLibraryIncludes(printer);
  }

  if (options_.transitive_pb_h) {
    GenerateDependencyIncludes(printer);
  }

  // This is unfortunately necessary for some plugins. I don't see why we
  // need two of the same insertion points.
  // TODO(gerbens) remove this.
  format("// @@protoc_insertion_point(includes)\n");

  GenerateMetadataPragma(printer, info_path);

  if (!options_.proto_h) {
    GenerateHeader(printer);
  } else {
    {
      NamespaceOpener ns(Namespace(file_, options_), format);
      format(
          "\n"
          "// @@protoc_insertion_point(namespace_scope)\n");
    }
    format(
        "\n"
        "// @@protoc_insertion_point(global_scope)\n"
        "\n");
  }

  GenerateBottomHeaderGuard(printer, true);
}

void FileGenerator::DoIncludeFile(const std::string& google3_name,
                                  bool do_export, io::Printer* printer) {
  Formatter format(printer, variables_);
  const std::string prefix = "net/proto2/";
  GOOGLE_CHECK(google3_name.find(prefix) == 0) << google3_name;

  if (options_.opensource_runtime) {
    std::string path = google3_name.substr(prefix.size());

    path = StringReplace(path, "internal/", "", false);
    path = StringReplace(path, "proto/", "", false);
    path = StringReplace(path, "public/", "", false);
    if (options_.runtime_include_base.empty()) {
      format("#include <google/protobuf/$1$>", path);
    } else {
      format("#include \"$1$google/protobuf/$2$\"",
             options_.runtime_include_base, path);
    }
  } else {
    std::string path = google3_name;
    // The bootstrapped proto generated code needs to use the
    // third_party/protobuf header paths to avoid circular dependencies.
    if (options_.bootstrap) {
      path = StringReplace(google3_name, "net/proto2/public",
                           "third_party/protobuf", false);
    }
    format("#include \"$1$\"", path);
  }

  if (do_export) {
    format("  // IWYU pragma: export");
  }

  format("\n");
}

std::string FileGenerator::CreateHeaderInclude(const std::string& basename,
                                               const FileDescriptor* file) {
  bool use_system_include = false;
  std::string name = basename;

  if (options_.opensource_runtime) {
    if (IsWellKnownMessage(file)) {
      if (options_.runtime_include_base.empty()) {
        use_system_include = true;
      } else {
        name = options_.runtime_include_base + basename;
      }
    }
  }

  std::string left = "\"";
  std::string right = "\"";
  if (use_system_include) {
    left = "<";
    right = ">";
  }
  return left + name + right;
}

void FileGenerator::GenerateSourceIncludes(io::Printer* printer) {
  Formatter format(printer, variables_);
  std::string target_basename = StripProto(file_->name());
  if (!options_.opensource_runtime) {
    GetBootstrapBasename(options_, target_basename, &target_basename);
  }
  target_basename += options_.proto_h ? ".proto.h" : ".pb.h";
  format(
      "// Generated by the protocol buffer compiler.  DO NOT EDIT!\n"
      "// source: $filename$\n"
      "\n"
      "#include $1$\n"
      "\n"
      "#include <algorithm>\n"  // for swap()
      "\n",
      CreateHeaderInclude(target_basename, file_));

  IncludeFile("net/proto2/io/public/coded_stream.h", printer);
  // TODO(gerbens) This is to include parse_context.h, we need a better way
  IncludeFile("net/proto2/public/extension_set.h", printer);
  IncludeFile("net/proto2/public/wire_format_lite.h", printer);

  // Unknown fields implementation in lite mode uses StringOutputStream
  if (!UseUnknownFieldSet(file_, options_) && !message_generators_.empty()) {
    IncludeFile("net/proto2/io/public/zero_copy_stream_impl_lite.h", printer);
  }

  if (HasDescriptorMethods(file_, options_)) {
    IncludeFile("net/proto2/public/descriptor.h", printer);
    IncludeFile("net/proto2/public/generated_message_reflection.h", printer);
    IncludeFile("net/proto2/public/reflection_ops.h", printer);
    IncludeFile("net/proto2/public/wire_format.h", printer);
  }

  if (HasGeneratedMethods(file_, options_) &&
      options_.tctable_mode != Options::kTCTableNever) {
    IncludeFile("net/proto2/public/generated_message_tctable_impl.h", printer);
  }

  if (options_.proto_h) {
    // Use the smaller .proto.h files.
    for (int i = 0; i < file_->dependency_count(); i++) {
      const FileDescriptor* dep = file_->dependency(i);
      // Do not import weak deps.
      if (!options_.opensource_runtime && IsDepWeak(dep)) continue;
      std::string basename = StripProto(dep->name());
      if (IsBootstrapProto(options_, file_)) {
        GetBootstrapBasename(options_, basename, &basename);
      }
      format("#include \"$1$.proto.h\"\n", basename);
    }
  }
  if (HasCordFields(file_, options_)) {
    format(
        "#include \"third_party/absl/strings/internal/string_constant.h\"\n");
  }

  format("// @@protoc_insertion_point(includes)\n");
  IncludeFile("net/proto2/public/port_def.inc", printer);
}

void FileGenerator::GenerateSourcePrelude(io::Printer* printer) {
  Formatter format(printer, variables_);

  // For MSVC builds, we use #pragma init_seg to move the initialization of our
  // libraries to happen before the user code.
  // This worksaround the fact that MSVC does not do constant initializers when
  // required by the standard.
  format("\nPROTOBUF_PRAGMA_INIT_SEG\n");

  // Generate convenience aliases.
  format(
      "\n"
      "namespace _pb = ::$1$;\n"
      "namespace _pbi = _pb::internal;\n",
      ProtobufNamespace(options_));
  if (HasGeneratedMethods(file_, options_) &&
      options_.tctable_mode != Options::kTCTableNever) {
    format("namespace _fl = _pbi::field_layout;\n");
  }
  format("\n");
}

void FileGenerator::GenerateSourceDefaultInstance(int idx,
                                                  io::Printer* printer) {
  Formatter format(printer, variables_);
  MessageGenerator* generator = message_generators_[idx].get();
  // Generate the split instance first because it's needed in the constexpr
  // constructor.
  if (ShouldSplit(generator->descriptor_, options_)) {
    // Use a union to disable the destructor of the _instance member.
    // We can constant initialize, but the object will still have a non-trivial
    // destructor that we need to elide.
    format(
        "struct $1$ {\n"
        "  PROTOBUF_CONSTEXPR $1$()\n"
        "      : _instance{",
        DefaultInstanceType(generator->descriptor_, options_,
                            /*split=*/true));
    generator->GenerateInitDefaultSplitInstance(printer);
    format(
        "} {}\n"
        "  ~$1$() {}\n"
        "  union {\n"
        "    $2$ _instance;\n"
        "  };\n"
        "};\n",
        DefaultInstanceType(generator->descriptor_, options_, /*split=*/true),
        StrCat(generator->classname_, "::Impl_::Split"));
    // NO_DESTROY is not necessary for correctness. The empty destructor is
    // enough. However, the empty destructor fails to be elided in some
    // configurations (like non-opt or with certain sanitizers). NO_DESTROY is
    // there just to improve performance and binary size in these builds.
    format(
        "PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT "
        "PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 $1$ $2$;\n",
        DefaultInstanceType(generator->descriptor_, options_, /*split=*/true),
        DefaultInstanceName(generator->descriptor_, options_, /*split=*/true));
  }

  generator->GenerateConstexprConstructor(printer);
  format(
      "struct $1$ {\n"
      "  PROTOBUF_CONSTEXPR $1$()\n"
      "      : _instance(::_pbi::ConstantInitialized{}) {}\n"
      "  ~$1$() {}\n"
      "  union {\n"
      "    $2$ _instance;\n"
      "  };\n"
      "};\n",
      DefaultInstanceType(generator->descriptor_, options_),
      generator->classname_);
  format(
      "PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT "
      "PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 $1$ $2$;\n",
      DefaultInstanceType(generator->descriptor_, options_),
      DefaultInstanceName(generator->descriptor_, options_));

  for (int i = 0; i < generator->descriptor_->field_count(); i++) {
    const FieldDescriptor* field = generator->descriptor_->field(i);
    if (IsStringInlined(field, options_)) {
      // Force the initialization of the inlined string in the default instance.
      format(
          "PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 std::true_type "
          "$1$::Impl_::_init_inline_$2$_ = "
          "($3$._instance.$4$.Init(), std::true_type{});\n",
          ClassName(generator->descriptor_), FieldName(field),
          DefaultInstanceName(generator->descriptor_, options_),
          FieldMemberName(field, ShouldSplit(field, options_)));
    }
  }

  if (options_.lite_implicit_weak_fields) {
    format(
        "PROTOBUF_CONSTINIT const void* $1$ =\n"
        "    &$2$;\n",
        DefaultInstancePtr(generator->descriptor_, options_),
        DefaultInstanceName(generator->descriptor_, options_));
  }
}

// A list of things defined in one .pb.cc file that we need to reference from
// another .pb.cc file.
struct FileGenerator::CrossFileReferences {
  // Populated if we are referencing from messages or files.
  std::unordered_set<const Descriptor*> weak_default_instances;

  // Only if we are referencing from files.
  std::unordered_set<const FileDescriptor*> strong_reflection_files;
  std::unordered_set<const FileDescriptor*> weak_reflection_files;
};

void FileGenerator::GetCrossFileReferencesForField(const FieldDescriptor* field,
                                                   CrossFileReferences* refs) {
  const Descriptor* msg = field->message_type();
  if (msg == nullptr) return;

  if (IsImplicitWeakField(field, options_, &scc_analyzer_) ||
      IsWeak(field, options_)) {
    refs->weak_default_instances.insert(msg);
  }
}

void FileGenerator::GetCrossFileReferencesForFile(const FileDescriptor* file,
                                                  CrossFileReferences* refs) {
  ForEachField(file, [this, refs](const FieldDescriptor* field) {
    GetCrossFileReferencesForField(field, refs);
  });

  if (!HasDescriptorMethods(file, options_)) return;

  for (int i = 0; i < file->dependency_count(); i++) {
    const FileDescriptor* dep = file->dependency(i);
    if (IsDepWeak(dep)) {
      refs->weak_reflection_files.insert(dep);
    } else {
      refs->strong_reflection_files.insert(dep);
    }
  }
}

// Generates references to variables defined in other files.
void FileGenerator::GenerateInternalForwardDeclarations(
    const CrossFileReferences& refs, io::Printer* printer) {
  Formatter format(printer, variables_);

  {
    NamespaceOpener ns(format);
    for (auto instance : Sorted(refs.weak_default_instances)) {
      ns.ChangeTo(Namespace(instance, options_));
      if (options_.lite_implicit_weak_fields) {
        format(
            "PROTOBUF_CONSTINIT __attribute__((weak)) const void* $1$ =\n"
            "    &::_pbi::implicit_weak_message_default_instance;\n",
            DefaultInstancePtr(instance, options_));
      } else {
        format("extern __attribute__((weak)) $1$ $2$;\n",
               DefaultInstanceType(instance, options_),
               DefaultInstanceName(instance, options_));
      }
    }
  }

  for (auto file : Sorted(refs.weak_reflection_files)) {
    format(
        "extern __attribute__((weak)) const ::_pbi::DescriptorTable $1$;\n",
        DescriptorTableName(file, options_));
  }
}

void FileGenerator::GenerateSourceForMessage(int idx, io::Printer* printer) {
  Formatter format(printer, variables_);
  GenerateSourceIncludes(printer);
  GenerateSourcePrelude(printer);

  if (IsAnyMessage(file_, options_)) MuteWuninitialized(format);

  CrossFileReferences refs;
  ForEachField(message_generators_[idx]->descriptor_,
               [this, &refs](const FieldDescriptor* field) {
                 GetCrossFileReferencesForField(field, &refs);
               });
  GenerateInternalForwardDeclarations(refs, printer);

  {  // package namespace
    NamespaceOpener ns(Namespace(file_, options_), format);

    // Define default instances
    GenerateSourceDefaultInstance(idx, printer);

    // Generate classes.
    format("\n");
    message_generators_[idx]->GenerateClassMethods(printer);

    format(
        "\n"
        "// @@protoc_insertion_point(namespace_scope)\n");
  }  // end package namespace

  {
    NamespaceOpener proto_ns(ProtobufNamespace(options_), format);
    message_generators_[idx]->GenerateSourceInProto2Namespace(printer);
  }

  if (IsAnyMessage(file_, options_)) UnmuteWuninitialized(format);

  format(
      "\n"
      "// @@protoc_insertion_point(global_scope)\n");
}

void FileGenerator::GenerateSourceForExtension(int idx, io::Printer* printer) {
  Formatter format(printer, variables_);
  GenerateSourceIncludes(printer);
  GenerateSourcePrelude(printer);
  NamespaceOpener ns(Namespace(file_, options_), format);
  extension_generators_[idx]->GenerateDefinition(printer);
}

void FileGenerator::GenerateGlobalSource(io::Printer* printer) {
  Formatter format(printer, variables_);
  GenerateSourceIncludes(printer);
  GenerateSourcePrelude(printer);

  {
    // Define the code to initialize reflection. This code uses a global
    // constructor to register reflection data with the runtime pre-main.
    if (HasDescriptorMethods(file_, options_)) {
      GenerateReflectionInitializationCode(printer);
    }
  }

  NamespaceOpener ns(Namespace(file_, options_), format);

  // Generate enums.
  for (int i = 0; i < enum_generators_.size(); i++) {
    enum_generators_[i]->GenerateMethods(i, printer);
  }
}

void FileGenerator::GenerateSource(io::Printer* printer) {
  Formatter format(printer, variables_);
  GenerateSourceIncludes(printer);
  GenerateSourcePrelude(printer);
  CrossFileReferences refs;
  GetCrossFileReferencesForFile(file_, &refs);
  GenerateInternalForwardDeclarations(refs, printer);

  if (IsAnyMessage(file_, options_)) MuteWuninitialized(format);

  {
    NamespaceOpener ns(Namespace(file_, options_), format);

    // Define default instances
    for (int i = 0; i < message_generators_.size(); i++) {
      GenerateSourceDefaultInstance(i, printer);
    }
  }

  {
    if (HasDescriptorMethods(file_, options_)) {
      // Define the code to initialize reflection. This code uses a global
      // constructor to register reflection data with the runtime pre-main.
      GenerateReflectionInitializationCode(printer);
    }
  }

  {
    NamespaceOpener ns(Namespace(file_, options_), format);

    // Actually implement the protos

    // Generate enums.
    for (int i = 0; i < enum_generators_.size(); i++) {
      enum_generators_[i]->GenerateMethods(i, printer);
    }

    // Generate classes.
    for (int i = 0; i < message_generators_.size(); i++) {
      format("\n");
      format(kThickSeparator);
      format("\n");
      message_generators_[i]->GenerateClassMethods(printer);
    }

    if (HasGenericServices(file_, options_)) {
      // Generate services.
      for (int i = 0; i < service_generators_.size(); i++) {
        if (i == 0) format("\n");
        format(kThickSeparator);
        format("\n");
        service_generators_[i]->GenerateImplementation(printer);
      }
    }

    // Define extensions.
    for (int i = 0; i < extension_generators_.size(); i++) {
      extension_generators_[i]->GenerateDefinition(printer);
    }

    format(
        "\n"
        "// @@protoc_insertion_point(namespace_scope)\n");
  }

  {
    NamespaceOpener proto_ns(ProtobufNamespace(options_), format);
    for (int i = 0; i < message_generators_.size(); i++) {
      message_generators_[i]->GenerateSourceInProto2Namespace(printer);
    }
  }

  format(
      "\n"
      "// @@protoc_insertion_point(global_scope)\n");

  if (IsAnyMessage(file_, options_)) UnmuteWuninitialized(format);

  IncludeFile("net/proto2/public/port_undef.inc", printer);
}

void FileGenerator::GenerateReflectionInitializationCode(io::Printer* printer) {
  Formatter format(printer, variables_);

  if (!message_generators_.empty()) {
    format("static ::_pb::Metadata $file_level_metadata$[$1$];\n",
           message_generators_.size());
  }
  if (!enum_generators_.empty()) {
    format(
        "static const ::_pb::EnumDescriptor* "
        "$file_level_enum_descriptors$[$1$];\n",
        enum_generators_.size());
  } else {
    format(
        "static "
        "constexpr ::_pb::EnumDescriptor const** "
        "$file_level_enum_descriptors$ = nullptr;\n");
  }
  if (HasGenericServices(file_, options_) && file_->service_count() > 0) {
    format(
        "static "
        "const ::_pb::ServiceDescriptor* "
        "$file_level_service_descriptors$[$1$];\n",
        file_->service_count());
  } else {
    format(
        "static "
        "constexpr ::_pb::ServiceDescriptor const** "
        "$file_level_service_descriptors$ = nullptr;\n");
  }

  if (!message_generators_.empty()) {
    format(
        "\n"
        "const $uint32$ $tablename$::offsets[] "
        "PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {\n");
    format.Indent();
    std::vector<std::pair<size_t, size_t> > pairs;
    pairs.reserve(message_generators_.size());
    for (int i = 0; i < message_generators_.size(); i++) {
      pairs.push_back(message_generators_[i]->GenerateOffsets(printer));
    }
    format.Outdent();
    format(
        "};\n"
        "static const ::_pbi::MigrationSchema schemas[] "
        "PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {\n");
    format.Indent();
    {
      int offset = 0;
      for (int i = 0; i < message_generators_.size(); i++) {
        message_generators_[i]->GenerateSchema(printer, offset,
                                               pairs[i].second);
        offset += pairs[i].first;
      }
    }
    format.Outdent();
    format(
        "};\n"
        "\nstatic const ::_pb::Message* const file_default_instances[] = {\n");
    format.Indent();
    for (int i = 0; i < message_generators_.size(); i++) {
      const Descriptor* descriptor = message_generators_[i]->descriptor_;
      format("&$1$::_$2$_default_instance_._instance,\n",
             Namespace(descriptor, options_),  // 1
             ClassName(descriptor));           // 2
    }
    format.Outdent();
    format(
        "};\n"
        "\n");
  } else {
    // we still need these symbols to exist
    format(
        // MSVC doesn't like empty arrays, so we add a dummy.
        "const $uint32$ $tablename$::offsets[1] = {};\n"
        "static constexpr ::_pbi::MigrationSchema* schemas = nullptr;\n"
        "static constexpr ::_pb::Message* const* "
        "file_default_instances = nullptr;\n"
        "\n");
  }

  // ---------------------------------------------------------------

  // Embed the descriptor.  We simply serialize the entire
  // FileDescriptorProto/ and embed it as a string literal, which is parsed and
  // built into real descriptors at initialization time.
  const std::string protodef_name =
      UniqueName("descriptor_table_protodef", file_, options_);
  format("const char $1$[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =\n",
         protodef_name);
  format.Indent();
  FileDescriptorProto file_proto;
  file_->CopyTo(&file_proto);
  std::string file_data;
  file_proto.SerializeToString(&file_data);

  {
    if (file_data.size() > 65535) {
      // Workaround for MSVC: "Error C1091: compiler limit: string exceeds
      // 65535 bytes in length". Declare a static array of chars rather than
      // use a string literal. Only write 25 bytes per line.
      static const int kBytesPerLine = 25;
      format("{ ");
      for (int i = 0; i < file_data.size();) {
        for (int j = 0; j < kBytesPerLine && i < file_data.size(); ++i, ++j) {
          format("'$1$', ", CEscape(file_data.substr(i, 1)));
        }
        format("\n");
      }
      format("'\\0' }");  // null-terminate
    } else {
      // Only write 40 bytes per line.
      static const int kBytesPerLine = 40;
      for (int i = 0; i < file_data.size(); i += kBytesPerLine) {
        format(
            "\"$1$\"\n",
            EscapeTrigraphs(CEscape(file_data.substr(i, kBytesPerLine))));
      }
    }
    format(";\n");
  }
  format.Outdent();

  CrossFileReferences refs;
  GetCrossFileReferencesForFile(file_, &refs);
  int num_deps =
      refs.strong_reflection_files.size() + refs.weak_reflection_files.size();

  // Build array of DescriptorTable deps.
  if (num_deps > 0) {
    format(
        "static const ::_pbi::DescriptorTable* const "
        "$desc_table$_deps[$1$] = {\n",
        num_deps);

    for (auto dep : Sorted(refs.strong_reflection_files)) {
      format("  &::$1$,\n", DescriptorTableName(dep, options_));
    }
    for (auto dep : Sorted(refs.weak_reflection_files)) {
      format("  &::$1$,\n", DescriptorTableName(dep, options_));
    }

    format("};\n");
  }

  // The DescriptorTable itself.
  // Should be "bool eager = NeedsEagerDescriptorAssignment(file_, options_);"
  // however this might cause a tsan failure in superroot b/148382879,
  // so disable for now.
  bool eager = false;
  format(
      "static ::_pbi::once_flag $desc_table$_once;\n"
      "const ::_pbi::DescriptorTable $desc_table$ = {\n"
      "    false, $1$, $2$, $3$,\n"
      "    \"$filename$\",\n"
      "    &$desc_table$_once, $4$, $5$, $6$,\n"
      "    schemas, file_default_instances, $tablename$::offsets,\n"
      "    $7$, $file_level_enum_descriptors$,\n"
      "    $file_level_service_descriptors$,\n"
      "};\n"
      // This function exists to be marked as weak.
      // It can significantly speed up compilation by breaking up LLVM's SCC in
      // the .pb.cc translation units. Large translation units see a reduction
      // of more than 35% of walltime for optimized builds.
      // Without the weak attribute all the messages in the file, including all
      // the vtables and everything they use become part of the same SCC through
      // a cycle like:
      // GetMetadata -> descriptor table -> default instances ->
      //   vtables -> GetMetadata
      // By adding a weak function here we break the connection from the
      // individual vtables back into the descriptor table.
      "PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* "
      "$desc_table$_getter() {\n"
      "  return &$desc_table$;\n"
      "}\n"
      "\n",
      eager ? "true" : "false", file_data.size(), protodef_name,
      num_deps == 0 ? "nullptr" : variables_["desc_table"] + "_deps", num_deps,
      message_generators_.size(),
      message_generators_.empty() ? "nullptr"
                                  : variables_["file_level_metadata"]);

  // For descriptor.proto we want to avoid doing any dynamic initialization,
  // because in some situations that would otherwise pull in a lot of
  // unnecessary code that can't be stripped by --gc-sections. Descriptor
  // initialization will still be performed lazily when it's needed.
  if (file_->name() != "net/proto2/proto/descriptor.proto") {
    format(
        "// Force running AddDescriptors() at dynamic initialization time.\n"
        "PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 "
        "static ::_pbi::AddDescriptorsRunner $1$(&$desc_table$);\n",
        UniqueName("dynamic_init_dummy", file_, options_));
  }
}

class FileGenerator::ForwardDeclarations {
 public:
  void AddMessage(const Descriptor* d) { classes_[ClassName(d)] = d; }
  void AddEnum(const EnumDescriptor* d) { enums_[ClassName(d)] = d; }
  void AddSplit(const Descriptor* d) { splits_[ClassName(d)] = d; }

  void Print(const Formatter& format, const Options& options) const {
    for (const auto& p : enums_) {
      const std::string& enumname = p.first;
      const EnumDescriptor* enum_desc = p.second;
      format(
          "enum ${1$$2$$}$ : int;\n"
          "bool $2$_IsValid(int value);\n",
          enum_desc, enumname);
    }
    for (const auto& p : classes_) {
      const std::string& classname = p.first;
      const Descriptor* class_desc = p.second;
      format(
          "class ${1$$2$$}$;\n"
          "struct $3$;\n"
          "$dllexport_decl $extern $3$ $4$;\n",
          class_desc, classname, DefaultInstanceType(class_desc, options),
          DefaultInstanceName(class_desc, options));
    }
    for (const auto& p : splits_) {
      const Descriptor* class_desc = p.second;
      format(
          "struct $1$;\n"
          "$dllexport_decl $extern $1$ $2$;\n",
          DefaultInstanceType(class_desc, options, /*split=*/true),
          DefaultInstanceName(class_desc, options, /*split=*/true));
    }
  }

  void PrintTopLevelDecl(const Formatter& format,
                         const Options& options) const {
    for (const auto& pair : classes_) {
      format(
          "template<> $dllexport_decl $"
          "$1$* Arena::CreateMaybeMessage<$1$>(Arena*);\n",
          QualifiedClassName(pair.second, options));
    }
  }

 private:
  std::map<std::string, const Descriptor*> classes_;
  std::map<std::string, const EnumDescriptor*> enums_;
  std::map<std::string, const Descriptor*> splits_;
};

static void PublicImportDFS(const FileDescriptor* fd,
                            std::unordered_set<const FileDescriptor*>* fd_set) {
  for (int i = 0; i < fd->public_dependency_count(); i++) {
    const FileDescriptor* dep = fd->public_dependency(i);
    if (fd_set->insert(dep).second) PublicImportDFS(dep, fd_set);
  }
}

void FileGenerator::GenerateForwardDeclarations(io::Printer* printer) {
  Formatter format(printer, variables_);
  std::vector<const Descriptor*> classes;
  std::vector<const EnumDescriptor*> enums;

  FlattenMessagesInFile(file_, &classes);  // All messages need forward decls.

  if (options_.proto_h) {  // proto.h needs extra forward declarations.
    // All classes / enums referred to as field members
    std::vector<const FieldDescriptor*> fields;
    ListAllFields(file_, &fields);
    for (int i = 0; i < fields.size(); i++) {
      classes.push_back(fields[i]->containing_type());
      classes.push_back(fields[i]->message_type());
      enums.push_back(fields[i]->enum_type());
    }
    ListAllTypesForServices(file_, &classes);
  }

  // Calculate the set of files whose definitions we get through include.
  // No need to forward declare types that are defined in these.
  std::unordered_set<const FileDescriptor*> public_set;
  PublicImportDFS(file_, &public_set);

  std::map<std::string, ForwardDeclarations> decls;
  for (int i = 0; i < classes.size(); i++) {
    const Descriptor* d = classes[i];
    if (d && !public_set.count(d->file()))
      decls[Namespace(d, options_)].AddMessage(d);
  }
  for (int i = 0; i < enums.size(); i++) {
    const EnumDescriptor* d = enums[i];
    if (d && !public_set.count(d->file()))
      decls[Namespace(d, options_)].AddEnum(d);
  }
  for (const auto& mg : message_generators_) {
    const Descriptor* d = mg->descriptor_;
    if ((d != nullptr) && (public_set.count(d->file()) == 0u) &&
        ShouldSplit(mg->descriptor_, options_))
      decls[Namespace(d, options_)].AddSplit(d);
  }

  {
    NamespaceOpener ns(format);
    for (const auto& pair : decls) {
      ns.ChangeTo(pair.first);
      pair.second.Print(format, options_);
    }
  }
  format("PROTOBUF_NAMESPACE_OPEN\n");
  for (const auto& pair : decls) {
    pair.second.PrintTopLevelDecl(format, options_);
  }
  format("PROTOBUF_NAMESPACE_CLOSE\n");
}

void FileGenerator::GenerateTopHeaderGuard(io::Printer* printer, bool pb_h) {
  Formatter format(printer, variables_);
  // Generate top of header.
  format(
      "// Generated by the protocol buffer compiler.  DO NOT EDIT!\n"
      "// source: $filename$\n"
      "\n"
      "#ifndef $1$\n"
      "#define $1$\n"
      "\n"
      "#include <limits>\n"
      "#include <string>\n",
      IncludeGuard(file_, pb_h, options_));
  if (!options_.opensource_runtime && !enum_generators_.empty()) {
    // Add header to provide std::is_integral for safe Enum_Name() function.
    format("#include <type_traits>\n");
  }
  format("\n");
}

void FileGenerator::GenerateBottomHeaderGuard(io::Printer* printer, bool pb_h) {
  Formatter format(printer, variables_);
  format("#endif  // $GOOGLE_PROTOBUF$_INCLUDED_$1$\n",
         IncludeGuard(file_, pb_h, options_));
}

void FileGenerator::GenerateLibraryIncludes(io::Printer* printer) {
  Formatter format(printer, variables_);
  if (UsingImplicitWeakFields(file_, options_)) {
    IncludeFile("net/proto2/public/implicit_weak_message.h", printer);
  }
  if (HasWeakFields(file_, options_)) {
    GOOGLE_CHECK(!options_.opensource_runtime);
    IncludeFile("net/proto2/public/weak_field_map.h", printer);
  }
  if (HasLazyFields(file_, options_, &scc_analyzer_)) {
    GOOGLE_CHECK(!options_.opensource_runtime);
    IncludeFile("net/proto2/public/lazy_field.h", printer);
  }
  if (ShouldVerify(file_, options_, &scc_analyzer_)) {
    IncludeFile("net/proto2/public/wire_format_verify.h", printer);
  }

  if (options_.opensource_runtime) {
    // Verify the protobuf library header version is compatible with the protoc
    // version before going any further.
    IncludeFile("net/proto2/public/port_def.inc", printer);
    format(
        "#if PROTOBUF_VERSION < $1$\n"
        "#error This file was generated by a newer version of protoc which is\n"
        "#error incompatible with your Protocol Buffer headers. Please update\n"
        "#error your headers.\n"
        "#endif\n"
        "#if $2$ < PROTOBUF_MIN_PROTOC_VERSION\n"
        "#error This file was generated by an older version of protoc which "
        "is\n"
        "#error incompatible with your Protocol Buffer headers. Please\n"
        "#error regenerate this file with a newer version of protoc.\n"
        "#endif\n"
        "\n",
        PROTOBUF_MIN_HEADER_VERSION_FOR_PROTOC,  // 1
        PROTOBUF_VERSION);                       // 2
    IncludeFile("net/proto2/public/port_undef.inc", printer);
  }

  // OK, it's now safe to #include other files.
  IncludeFile("net/proto2/io/public/coded_stream.h", printer);
  IncludeFile("net/proto2/public/arena.h", printer);
  IncludeFile("net/proto2/public/arenastring.h", printer);
  if ((options_.force_inline_string || options_.profile_driven_inline_string) &&
      !options_.opensource_runtime) {
    IncludeFile("net/proto2/public/inlined_string_field.h", printer);
  }
  if (HasSimpleBaseClasses(file_, options_)) {
    IncludeFile("net/proto2/public/generated_message_bases.h", printer);
  }
  if (HasGeneratedMethods(file_, options_) &&
      options_.tctable_mode != Options::kTCTableNever) {
    IncludeFile("net/proto2/public/generated_message_tctable_decl.h", printer);
  }
  IncludeFile("net/proto2/public/generated_message_util.h", printer);
  IncludeFile("net/proto2/public/metadata_lite.h", printer);

  if (HasDescriptorMethods(file_, options_)) {
    IncludeFile("net/proto2/public/generated_message_reflection.h", printer);
  }

  if (!message_generators_.empty()) {
    if (HasDescriptorMethods(file_, options_)) {
      IncludeFile("net/proto2/public/message.h", printer);
    } else {
      IncludeFile("net/proto2/public/message_lite.h", printer);
    }
  }
  if (options_.opensource_runtime) {
    // Open-source relies on unconditional includes of these.
    IncludeFileAndExport("net/proto2/public/repeated_field.h", printer);
    IncludeFileAndExport("net/proto2/public/extension_set.h", printer);
  } else {
    // Google3 includes these files only when they are necessary.
    if (HasExtensionsOrExtendableMessage(file_)) {
      IncludeFileAndExport("net/proto2/public/extension_set.h", printer);
    }
    if (HasRepeatedFields(file_)) {
      IncludeFileAndExport("net/proto2/public/repeated_field.h", printer);
    }
    if (HasStringPieceFields(file_, options_)) {
      IncludeFile("net/proto2/public/string_piece_field_support.h", printer);
    }
    if (HasCordFields(file_, options_)) {
      format("#include \"third_party/absl/strings/cord.h\"\n");
    }
  }
  if (HasMapFields(file_)) {
    IncludeFileAndExport("net/proto2/public/map.h", printer);
    if (HasDescriptorMethods(file_, options_)) {
      IncludeFile("net/proto2/public/map_entry.h", printer);
      IncludeFile("net/proto2/public/map_field_inl.h", printer);
    } else {
      IncludeFile("net/proto2/public/map_entry_lite.h", printer);
      IncludeFile("net/proto2/public/map_field_lite.h", printer);
    }
  }

  if (HasEnumDefinitions(file_)) {
    if (HasDescriptorMethods(file_, options_)) {
      IncludeFile("net/proto2/public/generated_enum_reflection.h", printer);
    } else {
      IncludeFile("net/proto2/public/generated_enum_util.h", printer);
    }
  }

  if (HasGenericServices(file_, options_)) {
    IncludeFile("net/proto2/public/service.h", printer);
  }

  if (UseUnknownFieldSet(file_, options_) && !message_generators_.empty()) {
    IncludeFile("net/proto2/public/unknown_field_set.h", printer);
  }
}

void FileGenerator::GenerateMetadataPragma(io::Printer* printer,
                                           const std::string& info_path) {
  Formatter format(printer, variables_);
  if (!info_path.empty() && !options_.annotation_pragma_name.empty() &&
      !options_.annotation_guard_name.empty()) {
    format.Set("guard", options_.annotation_guard_name);
    format.Set("pragma", options_.annotation_pragma_name);
    format.Set("info_path", info_path);
    format(
        "#ifdef $guard$\n"
        "#pragma $pragma$ \"$info_path$\"\n"
        "#endif  // $guard$\n");
  }
}

void FileGenerator::GenerateDependencyIncludes(io::Printer* printer) {
  Formatter format(printer, variables_);
  for (int i = 0; i < file_->dependency_count(); i++) {
    std::string basename = StripProto(file_->dependency(i)->name());

    // Do not import weak deps.
    if (IsDepWeak(file_->dependency(i))) continue;

    if (IsBootstrapProto(options_, file_)) {
      GetBootstrapBasename(options_, basename, &basename);
    }

    format("#include $1$\n",
           CreateHeaderInclude(basename + ".pb.h", file_->dependency(i)));
  }
}

void FileGenerator::GenerateGlobalStateFunctionDeclarations(
    io::Printer* printer) {
  Formatter format(printer, variables_);
  // Forward-declare the DescriptorTable because this is referenced by .pb.cc
  // files depending on this file.
  //
  // The TableStruct is also outputted in weak_message_field.cc, because the
  // weak fields must refer to table struct but cannot include the header.
  // Also it annotates extra weak attributes.
  // TODO(gerbens) make sure this situation is handled better.
  format(
      "\n"
      "// Internal implementation detail -- do not use these members.\n"
      "struct $dllexport_decl $$tablename$ {\n"
      "  static const $uint32$ offsets[];\n"
      "};\n");
  if (HasDescriptorMethods(file_, options_)) {
    format(
        "$dllexport_decl $extern const ::$proto_ns$::internal::DescriptorTable "
        "$desc_table$;\n");
  }
}

void FileGenerator::GenerateMessageDefinitions(io::Printer* printer) {
  Formatter format(printer, variables_);
  // Generate class definitions.
  for (int i = 0; i < message_generators_.size(); i++) {
    if (i > 0) {
      format("\n");
      format(kThinSeparator);
      format("\n");
    }
    message_generators_[i]->GenerateClassDefinition(printer);
  }
}

void FileGenerator::GenerateEnumDefinitions(io::Printer* printer) {
  // Generate enum definitions.
  for (int i = 0; i < enum_generators_.size(); i++) {
    enum_generators_[i]->GenerateDefinition(printer);
  }
}

void FileGenerator::GenerateServiceDefinitions(io::Printer* printer) {
  Formatter format(printer, variables_);
  if (HasGenericServices(file_, options_)) {
    // Generate service definitions.
    for (int i = 0; i < service_generators_.size(); i++) {
      if (i > 0) {
        format("\n");
        format(kThinSeparator);
        format("\n");
      }
      service_generators_[i]->GenerateDeclarations(printer);
    }

    format("\n");
    format(kThickSeparator);
    format("\n");
  }
}

void FileGenerator::GenerateExtensionIdentifiers(io::Printer* printer) {
  // Declare extension identifiers. These are in global scope and so only
  // the global scope extensions.
  for (auto& extension_generator : extension_generators_) {
    if (extension_generator->IsScoped()) continue;
    extension_generator->GenerateDeclaration(printer);
  }
}

void FileGenerator::GenerateInlineFunctionDefinitions(io::Printer* printer) {
  Formatter format(printer, variables_);
  // TODO(gerbens) remove pragmas when gcc is no longer used. Current version
  // of gcc fires a bogus error when compiled with strict-aliasing.
  format(
      "#ifdef __GNUC__\n"
      "  #pragma GCC diagnostic push\n"
      "  #pragma GCC diagnostic ignored \"-Wstrict-aliasing\"\n"
      "#endif  // __GNUC__\n");
  // Generate class inline methods.
  for (int i = 0; i < message_generators_.size(); i++) {
    if (i > 0) {
      format(kThinSeparator);
      format("\n");
    }
    message_generators_[i]->GenerateInlineMethods(printer);
  }
  format(
      "#ifdef __GNUC__\n"
      "  #pragma GCC diagnostic pop\n"
      "#endif  // __GNUC__\n");

  for (int i = 0; i < message_generators_.size(); i++) {
    if (i > 0) {
      format(kThinSeparator);
      format("\n");
    }
  }
}

void FileGenerator::GenerateProto2NamespaceEnumSpecializations(
    io::Printer* printer) {
  Formatter format(printer, variables_);
  // Emit GetEnumDescriptor specializations into google::protobuf namespace:
  if (HasEnumDefinitions(file_)) {
    format("\n");
    {
      NamespaceOpener proto_ns(ProtobufNamespace(options_), format);
      format("\n");
      for (int i = 0; i < enum_generators_.size(); i++) {
        enum_generators_[i]->GenerateGetEnumDescriptorSpecializations(printer);
      }
      format("\n");
    }
  }
}

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
