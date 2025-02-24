///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxcBindingTable.h                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/DXIL/DxilConstants.h"
#include "llvm/ADT/StringRef.h"

#include <map>
#include <string>

namespace llvm {
class raw_ostream;
class Module;
} // namespace llvm

namespace hlsl {
class DxilModule;
}

namespace hlsl {
struct DxcBindingTable {
  typedef std::pair<std::string, hlsl::DXIL::ResourceClass> Key;
  struct Entry {
    unsigned index = UINT_MAX;
    unsigned space = UINT_MAX;
  };
  std::map<Key, Entry> entries;
};

bool ParseBindingTable(llvm::StringRef fileName, llvm::StringRef content,
                       llvm::raw_ostream &errors, DxcBindingTable *outTable);
void WriteBindingTableToMetadata(llvm::Module &M, const DxcBindingTable &table);
void ApplyBindingTableFromMetadata(hlsl::DxilModule &DM);

} // namespace hlsl
