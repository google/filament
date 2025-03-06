//===-- ObjDumper.cpp - Base dumper class -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file implements ObjDumper.
///
//===----------------------------------------------------------------------===//

#include "ObjDumper.h"
#include "Error.h"
#include "StreamWriter.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {

ObjDumper::ObjDumper(StreamWriter& Writer)
  : W(Writer) {
}

ObjDumper::~ObjDumper() {
}

} // namespace llvm
