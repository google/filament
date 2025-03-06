//===-- CodeGen/ObjectFilePCHContainerOperations.h - ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_CODEGEN_OBJECT_FILE_PCH_CONTAINER_OPERATIONS_H
#define LLVM_CLANG_CODEGEN_OBJECT_FILE_PCH_CONTAINER_OPERATIONS_H

#include "clang/Frontend/PCHContainerOperations.h"

namespace clang {

/// A PCHContainerWriter implementation that uses LLVM to
/// wraps Clang modules inside a COFF, ELF, or Mach-O container.
class ObjectFilePCHContainerWriter : public PCHContainerWriter {
  StringRef getFormat() const override { return "obj"; }

  /// Return an ASTConsumer that can be chained with a
  /// PCHGenerator that produces a wrapper file format
  /// that also contains full debug info for the module.
  std::unique_ptr<ASTConsumer> CreatePCHContainerGenerator(
      DiagnosticsEngine &Diags, const HeaderSearchOptions &HSO,
      const PreprocessorOptions &PPO, const TargetOptions &TO,
      const LangOptions &LO, const std::string &MainFileName,
      const std::string &OutputFileName, llvm::raw_pwrite_stream *OS,
      std::shared_ptr<PCHBuffer> Buffer) const override;
};

/// A PCHContainerReader implementation that uses LLVM to
/// wraps Clang modules inside a COFF, ELF, or Mach-O container.
class ObjectFilePCHContainerReader : public PCHContainerReader {
  StringRef getFormat() const override { return "obj"; }

  /// Initialize an llvm::BitstreamReader with the serialized
  /// AST inside the PCH container Buffer.
  void ExtractPCH(llvm::MemoryBufferRef Buffer,
                  llvm::BitstreamReader &StreamFile) const override;
};
}

#endif
