//===-- llvm/Bitcode/NaCl/NaClReaderWriter.h - ------------------*- C++ -*-===//
//      NaCl Bitcode reader/writer.
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header defines interfaces to read and write NaCl bitcode wire format
// files.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_BITCODE_NACL_NACLREADERWRITER_H
#define LLVM_BITCODE_NACL_NACLREADERWRITER_H

#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"

#include <string>

namespace llvm {
class LLVMContext;
class Module;
class NaClBitcodeHeader;
class NaClBitstreamWriter;
class StreamingMemoryObject;
class raw_ostream;

/// Defines the data layout used for PNaCl bitcode files. We set the
/// data layout of the module in the bitcode readers rather than in
/// pnacl-llc so that 'opt' will also use the correct data layout if
/// it is run on a pexe.
extern const char *PNaClDataLayout;

/// Allows (function) local symbol tables (unsupported) in PNaCl bitcode
/// files.
extern cl::opt<bool> PNaClAllowLocalSymbolTables;

/// \brief Defines the integer bit size used to model pointers in PNaCl.
static const unsigned PNaClIntPtrTypeBitSize = 32;

/// Diagnostic handler that redirects error diagnostics to the given stream.
DiagnosticHandlerFunction redirectNaClDiagnosticToStream(raw_ostream &Out);

/// Read the header of the specified bitcode buffer and prepare for lazy
/// deserialization of function bodies.  If successful, this takes ownership
/// of 'Buffer' (extending its lifetime).  On error, this returns an error
/// code and deletes Buffer.
///
/// The AcceptSupportedOnly argument is used to decide which PNaCl versions
/// of the PNaCl bitcode to accept. There are three forms:
///    1) Readable and supported.
///    2) Readable and unsupported. Allows testing of code before becoming
///       supported, as well as running experiments on the bitcode format.
///    3) Unreadable.
/// When AcceptSupportedOnly is true, only form 1 is allowed. When
/// AcceptSupportedOnly is false, forms 1 and 2 are allowed.
ErrorOr<Module *>
getNaClLazyBitcodeModule(std::unique_ptr<MemoryBuffer> &&Buffer,
                         LLVMContext &Context,
                         DiagnosticHandlerFunction DiagnosticHandler = nullptr,
                         bool AcceptSupportedOnly = true);

/// Read the header of the specified stream and prepare for lazy
/// deserialization and streaming of function bodies. On error,
/// this returns null, and fills in *ErrMsg with an error description
/// if ErrMsg is non-null.
///
/// See getNaClLazyBitcodeModule for an explanation of argument
/// AcceptSupportedOnly.
/// TODO(kschimpf): Refactor this and getStreamedBitcodeModule to use
/// ErrorOr<Module *> API so that all methods have the same interface.
Module *getNaClStreamedBitcodeModule(
    const std::string &name, StreamingMemoryObject *streamer,
    LLVMContext &Context, DiagnosticHandlerFunction DiagnosticHandler = nullptr,
    std::string *ErrMsg = nullptr, bool AcceptSupportedOnly = true);

/// Read the bitcode file from a buffer, returning the module.
///
/// See getNaClLazyBitcodeModule for an explanation of argument
/// AcceptSupportedOnly.
ErrorOr<Module *>
NaClParseBitcodeFile(MemoryBufferRef Buffer, LLVMContext &Context,
                     DiagnosticHandlerFunction DiagnosticHandler = nullptr,
                     bool AcceptSupportedOnly = true);

/// Read the textual bitcode records in Filename, returning the module.
/// Note: If Filename is "-", stdin will be read.
///
/// TODO(kschimpf) Replace Verbose argument with a DiagnosticHandlerFunction.
ErrorOr<Module *> parseNaClBitcodeText(const std::string &Filename,
                                       LLVMContext &Context,
                                       raw_ostream *Verbose = nullptr);

/// Write the specified module to the specified raw output stream, using
/// PNaCl wire format.  For streams where it matters, the given stream
/// should be in "binary" mode.
///
/// The AcceptSupportedOnly argument is used to decide which PNaCl versions
/// of the PNaCl bitcode to generate. There are two forms:
///    1) Writable and supported.
///    2) Writable and unsupported. Allows testing of code before becoming
///       supported, as well as running experiments on the bitcode format.
/// When AcceptSupportedOnly is true, only form 1 is allowed. When
/// AcceptSupportedOnly is false, forms 1 and 2 are allowed.
void NaClWriteBitcodeToFile(const Module *M, raw_ostream &Out,
                            bool AcceptSupportedOnly = true);

/// isNaClBitcode - Return true if the given bytes are the magic bytes for
/// PNaCl bitcode wire format.
///
inline bool isNaClBitcode(const unsigned char *BufPtr,
                          const unsigned char *BufEnd) {
  return BufPtr + 4 <= BufEnd && BufPtr[0] == 'P' && BufPtr[1] == 'E' &&
         BufPtr[2] == 'X' && BufPtr[3] == 'E';
}

/// NaClWriteHeader - Generate a default header (using the version
/// number defined by kPNaClVersion) and write to the corresponding
/// bitcode stream.
void NaClWriteHeader(NaClBitstreamWriter &Stream, bool AcceptSupportedOnly);

// NaClWriteHeader - Write the contents of the bitcode header to the
// corresponding bitcode stream.
void NaClWriteHeader(const NaClBitcodeHeader &Header,
                     NaClBitstreamWriter &Stream);

/// NaClObjDump - Read PNaCl bitcode file from input, and print a
/// textual representation of its contents. NoRecords and NoAssembly
/// define what should not be included in the dump.
bool NaClObjDump(MemoryBufferRef Input, raw_ostream &output, bool NoRecords,
                 bool NoAssembly);

} // namespace llvm
#endif
