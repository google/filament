///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcshadersourceinfo.h                                                     //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Utility helpers for dealing with DXIL part related to shader sources      //
// and options.                                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DxilContainer/DxilContainer.h"
#include "llvm/ADT/StringRef.h"
#include <stdint.h>
#include <vector>

namespace clang {
class CodeGenOptions;
class SourceManager;
} // namespace clang

namespace hlsl {

// TODO: Move this type to its own library.
struct SourceInfoReader {
  using Buffer = std::vector<uint8_t>;
  Buffer m_UncompressedSources;

  struct Source {
    llvm::StringRef Name;
    llvm::StringRef Content;
  };

  struct ArgPair {
    std::string Name;
    std::string Value;
  };

  std::vector<Source> m_Sources;
  std::vector<ArgPair> m_ArgPairs;

  Source GetSource(unsigned i) const { return m_Sources[i]; }
  unsigned GetSourcesCount() const { return m_Sources.size(); }

  const ArgPair &GetArgPair(unsigned i) const { return m_ArgPairs[i]; }
  unsigned GetArgPairCount() const { return m_ArgPairs.size(); }

  // Note: The memory for SourceInfo must outlive this structure.
  bool Init(const hlsl::DxilSourceInfo *SourceInfo, unsigned sourceInfoSize);
};

// Herper for writing the shader source part.
struct SourceInfoWriter {
  using Buffer = std::vector<uint8_t>;
  Buffer m_Buffer;

  const hlsl::DxilSourceInfo *GetPart() const;
  void Write(llvm::StringRef targetProfile, llvm::StringRef entryPoint,
             clang::CodeGenOptions &cgOpts, clang::SourceManager &srcMgr);
};

} // namespace hlsl
