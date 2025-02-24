//===- subzero/src/IceGlobalInits.cpp - Global declarations ---------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the notion of function declarations, global variable
/// declarations, and the corresponding variable initializers in Subzero.
///
//===----------------------------------------------------------------------===//

#include "IceGlobalInits.h"

#include "IceDefs.h"
#include "IceGlobalContext.h"
#include "IceTypes.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"

namespace {
char hexdigit(unsigned X) { return X < 10 ? '0' + X : 'A' + X - 10; }

void dumpLinkage(Ice::Ostream &Stream,
                 llvm::GlobalValue::LinkageTypes Linkage) {
  if (!Ice::BuildDefs::dump())
    return;
  switch (Linkage) {
  case llvm::GlobalValue::ExternalLinkage:
    Stream << "external";
    return;
  case llvm::GlobalValue::InternalLinkage:
    Stream << "internal";
    return;
  default:
    break;
  }
  std::string Buffer;
  llvm::raw_string_ostream StrBuf(Buffer);
  StrBuf << "Unknown linkage value: " << Linkage;
  llvm::report_fatal_error(StrBuf.str());
}

void dumpCallingConv(Ice::Ostream &, llvm::CallingConv::ID CallingConv) {
  if (!Ice::BuildDefs::dump())
    return;
  if (CallingConv == llvm::CallingConv::C)
    return;
  std::string Buffer;
  llvm::raw_string_ostream StrBuf(Buffer);
  StrBuf << "Unknown calling convention: " << CallingConv;
  llvm::report_fatal_error(StrBuf.str());
}

} // end of anonymous namespace

namespace Ice {

bool FunctionDeclaration::validateTypeSignature() const {
  for (SizeT i = 0; i < Signature.getNumArgs(); ++i) {
    if (!isCallParameterType(Signature.getArgType(i)))
      return false;
  }
  return isCallReturnType(Signature.getReturnType());
}

std::string
FunctionDeclaration::getTypeSignatureError(const GlobalContext *Ctx) {
  std::string Buffer;
  llvm::raw_string_ostream StrBuf(Buffer);
  StrBuf << "Invalid type signature for";
  StrBuf << " " << getName() << ": " << getSignature();
  return StrBuf.str();
}

void FunctionDeclaration::dumpType(Ostream &Stream) const {
  if (!Ice::BuildDefs::dump())
    return;
  Stream << Signature;
}

void FunctionDeclaration::dump(Ostream &Stream) const {
  if (!Ice::BuildDefs::dump())
    return;
  if (IsProto)
    Stream << "declare ";
  ::dumpLinkage(Stream, Linkage);
  ::dumpCallingConv(Stream, CallingConv);
  Stream << Signature.getReturnType() << " @" << Name << "(";
  bool IsFirst = true;
  for (Type ArgTy : Signature.getArgList()) {
    if (IsFirst)
      IsFirst = false;
    else
      Stream << ", ";
    Stream << ArgTy;
  }
  Stream << ")";
}

void VariableDeclaration::dumpType(Ostream &Stream) const {
  if (!Ice::BuildDefs::dump())
    return;
  if (Initializers.size() == 1) {
    Initializers.front()->dumpType(Stream);
  } else {
    Stream << "<{ ";
    bool IsFirst = true;
    for (const auto *Init : Initializers) {
      if (IsFirst) {
        IsFirst = false;
      } else {
        Stream << ", ";
      }
      Init->dumpType(Stream);
    }
    Stream << " }>";
  }
}

void VariableDeclaration::dump(Ostream &Stream) const {
  if (!Ice::BuildDefs::dump())
    return;
  Stream << "@" << Name << " = ";
  ::dumpLinkage(Stream, Linkage);
  Stream << " " << (IsConstant ? "constant" : "global") << " ";

  // Add initializer.
  if (Initializers.size() == 1) {
    Initializers.front()->dump(Stream);
  } else {
    dumpType(Stream);
    Stream << " <{ ";
    bool IsFirst = true;
    for (const auto *Init : Initializers) {
      if (IsFirst) {
        IsFirst = false;
      } else {
        Stream << ", ";
      }
      Init->dump(Stream);
    }
    Stream << " }>";
  }

  // Add alignment.
  if (Alignment > 0)
    Stream << ", align " << Alignment;
  Stream << "\n";
}

void VariableDeclaration::Initializer::dumpType(Ostream &Stream) const {
  if (!Ice::BuildDefs::dump())
    return;
  Stream << "[" << getNumBytes() << " x " << Ice::IceType_i8 << "]";
}

void VariableDeclaration::DataInitializer::dump(Ostream &Stream) const {
  if (!Ice::BuildDefs::dump())
    return;
  dumpType(Stream);
  Stream << " c\"";
  // Code taken from PrintEscapedString() in AsmWriter.cpp. Keep the strings in
  // the same format as the .ll file for practical diffing.
  for (SizeT i = 0; i < ContentsSize; ++i) {
    uint8_t C = Contents[i];
    if (isprint(C) && C != '\\' && C != '"')
      Stream << C;
    else
      Stream << '\\' << hexdigit(C >> 4) << hexdigit(C & 0x0F);
  }
  Stream << "\"";
}

void VariableDeclaration::ZeroInitializer::dump(Ostream &Stream) const {
  if (!Ice::BuildDefs::dump())
    return;
  dumpType(Stream);
  Stream << " zeroinitializer";
}

void VariableDeclaration::RelocInitializer::dumpType(Ostream &Stream) const {
  if (!Ice::BuildDefs::dump())
    return;
  Stream << Ice::IceType_i32;
}

void VariableDeclaration::RelocInitializer::dump(Ostream &Stream) const {
  if (!Ice::BuildDefs::dump())
    return;
  const RelocOffsetT Offset = getOffset();
  if (Offset != 0) {
    dumpType(Stream);
    Stream << " add (";
  }
  dumpType(Stream);
  Stream << " ptrtoint (";
  Declaration->dumpType(Stream);
  Stream << "* @" << Declaration->getName() << " to ";
  dumpType(Stream);
  Stream << ")";
  if (Offset != 0) {
    Stream << ", ";
    dumpType(Stream);
    Stream << " " << Offset << ")";
  }
}

} // end of namespace Ice
