//===------ RawBufferMethods.h ---- Raw Buffer Methods ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_RAWBUFFERMETHODS_H
#define LLVM_CLANG_SPIRV_RAWBUFFERMETHODS_H

class ASTContext;
class SpirvBuilder;
class SpirvInstruction;

#include "SpirvEmitter.h"

namespace clang {
namespace spirv {

class RawBufferHandler {
public:
  RawBufferHandler(SpirvEmitter &emitter)
      : theEmitter(emitter), astContext(emitter.getASTContext()),
        spvBuilder(emitter.getSpirvBuilder()) {}

  /// \brief Performs (RW)ByteAddressBuffer.Load<T>(byteAddress).
  /// (RW)ByteAddressBuffers are represented as structs with only one member
  /// which is a runtime array in SPIR-V. This method works by loading one or
  /// more uints, and performing necessary casts and composite constructions
  /// to build the 'targetType'. The layout rule for the result will be `Void`
  /// because the value will be built and used internally only. It does not have
  /// to match `buffer`.
  ///
  /// Example:
  /// targetType = uint16_t, byteAddress=0
  ///                 --> Load the first 16-bit uint starting at byte address 0.
  SpirvInstruction *processTemplatedLoadFromBuffer(
      SpirvInstruction *buffer, SpirvInstruction *byteAddress,
      const QualType targetType, SourceRange range = {});

  /// \brief Performs RWByteAddressBuffer.Store<T>(address, value).
  /// RWByteAddressBuffers are represented in SPIR-V as structs with only one
  /// member which is a runtime array of uints. This method works by decomposing
  /// the given |value| to reach numeric/bool types. Then performs necessary
  /// casts to uints and stores them in the underlying runtime array.
  ///
  /// Example:
  /// targetType = uint16_t, address=0
  ///                 --> Store to the first 16-bit uint starting at address 0.
  void processTemplatedStoreToBuffer(SpirvInstruction *value,
                                     SpirvInstruction *buffer,
                                     SpirvInstruction *&byteAddress,
                                     const QualType valueType,
                                     SourceRange range = {});

private:
  class BufferAddress {
  public:
    BufferAddress(SpirvInstruction *&byteAddress, SpirvEmitter &emitter)
        : byteAddress(byteAddress), wordIndex(),
          spvBuilder(emitter.getSpirvBuilder()),
          astContext(emitter.getASTContext()) {}
    SpirvInstruction *getByteAddress();
    SpirvInstruction *getWordIndex(SourceLocation loc, SourceRange range);

    void incrementByteAddress(SpirvInstruction *width, SourceLocation loc,
                              SourceRange range);
    void incrementByteAddress(uint32_t width, SourceLocation loc,
                              SourceRange range);

    void incrementWordIndex(SourceLocation loc, SourceRange range);

  private:
    SpirvInstruction *byteAddress;
    llvm::Optional<SpirvInstruction *> wordIndex;

    SpirvBuilder &spvBuilder;
    ASTContext &astContext;
  };

  SpirvInstruction *processTemplatedLoadFromBuffer(SpirvInstruction *buffer,
                                                   BufferAddress &address,
                                                   const QualType targetType,
                                                   SourceRange range = {});
  void processTemplatedStoreToBuffer(SpirvInstruction *value,
                                     SpirvInstruction *buffer,
                                     BufferAddress &address,
                                     const QualType valueType,
                                     SourceRange range = {});

  SpirvInstruction *load16Bits(SpirvInstruction *buffer, BufferAddress &address,
                               QualType target16BitType,
                               SourceRange range = {});

  SpirvInstruction *load32Bits(SpirvInstruction *buffer, BufferAddress &address,
                               QualType target32BitType,
                               SourceRange range = {});

  SpirvInstruction *load64Bits(SpirvInstruction *buffer, BufferAddress &address,
                               QualType target64BitType,
                               SourceRange range = {});

private:
  void store16Bits(SpirvInstruction *value, SpirvInstruction *buffer,
                   BufferAddress &address, const QualType valueType,
                   SourceRange range = {});

  void store32Bits(SpirvInstruction *value, SpirvInstruction *buffer,
                   BufferAddress &address, const QualType valueType,
                   SourceRange range = {});

  void store64Bits(SpirvInstruction *value, SpirvInstruction *buffer,
                   BufferAddress &address, const QualType valueType,
                   SourceRange range = {});

  /// \brief Serializes the given values into their components until a scalar or
  /// a struct has been reached. Returns the most basic type it reaches.
  QualType serializeToScalarsOrStruct(std::deque<SpirvInstruction *> *values,
                                      QualType valueType, SourceLocation,
                                      SourceRange range = {});

private:
  /// \brief Performs an OpBitCast from |fromType| to |toType| on the given
  /// instruction.
  ///
  /// If the |toType| is a boolean type, it performs a regular type cast.
  ///
  /// If the |fromType| and |toType| are the same, does not thing and returns
  /// the given instruction
  SpirvInstruction *bitCastToNumericalOrBool(SpirvInstruction *instr,
                                             QualType fromType, QualType toType,
                                             SourceLocation loc,
                                             SourceRange range = {});

private:
  SpirvEmitter &theEmitter;
  ASTContext &astContext;
  SpirvBuilder &spvBuilder;
};

} // namespace spirv
} // namespace clang

#endif // LLVM_CLANG_SPIRV_RAWBUFFERMETHODS_H
