//===- subzero/src/IceAssembler.h - Integrated assembler --------*- C++ -*-===//
// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
//
// Modified by the Subzero authors.
//
//===----------------------------------------------------------------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the Assembler base class.
///
/// Instructions are assembled by architecture-specific assemblers that derive
/// from this base class. This base class manages buffers and fixups for
/// emitting code, etc.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEASSEMBLER_H
#define SUBZERO_SRC_ICEASSEMBLER_H

#include "IceDefs.h"
#include "IceFixups.h"
#include "IceStringPool.h"
#include "IceUtils.h"

#include "llvm/Support/Allocator.h"

namespace Ice {

class Assembler;

/// A Label can be in one of three states:
///  - Unused.
///  - Linked, unplaced and tracking the position of branches to the label.
///  - Bound, placed and tracking its position.
class Label {
  Label(const Label &) = delete;
  Label &operator=(const Label &) = delete;

public:
  Label() = default;
  virtual ~Label() = default;

  virtual void finalCheck() const {
    // Assert if label is being destroyed with unresolved branches pending.
    assert(!isLinked());
  }

  /// Returns the encoded position stored in the label.
  intptr_t getEncodedPosition() const { return Position; }

  /// Returns the position for bound labels (branches that come after this are
  /// considered backward branches). Cannot be used for unused or linked labels.
  intptr_t getPosition() const {
    assert(isBound());
    return -Position - kWordSize;
  }

  /// Returns the position of an earlier branch instruction that was linked to
  /// this label (branches that use this are considered forward branches). The
  /// linked instructions form a linked list, of sorts, using the instruction's
  /// displacement field for the location of the next instruction that is also
  /// linked to this label.
  intptr_t getLinkPosition() const {
    assert(isLinked());
    return Position - kWordSize;
  }

  void setPosition(intptr_t NewValue) { Position = NewValue; }

  bool isBound() const { return Position < 0; }
  bool isLinked() const { return Position > 0; }

  virtual bool isUnused() const { return Position == 0; }

  void bindTo(intptr_t position) {
    assert(!isBound());
    Position = -position - kWordSize;
    assert(isBound());
  }

  void linkTo(const Assembler &Asm, intptr_t position);

protected:
  intptr_t Position = 0;

  // TODO(jvoung): why are labels offset by this?
  static constexpr uint32_t kWordSize = sizeof(uint32_t);
};

/// Assembler buffers are used to emit binary code. They grow on demand.
class AssemblerBuffer {
  AssemblerBuffer(const AssemblerBuffer &) = delete;
  AssemblerBuffer &operator=(const AssemblerBuffer &) = delete;

public:
  AssemblerBuffer(Assembler &);
  ~AssemblerBuffer();

  /// \name Basic support for emitting, loading, and storing.
  /// @{
  // These use memcpy instead of assignment to avoid undefined behaviour of
  // assigning to unaligned addresses. Since the size of the copy is known the
  // compiler can inline the memcpy with simple moves.
  template <typename T> void emit(T Value) {
    assert(hasEnsuredCapacity());
    memcpy(reinterpret_cast<void *>(Cursor), &Value, sizeof(T));
    Cursor += sizeof(T);
  }

  template <typename T> T load(intptr_t Position) const {
    assert(Position >= 0 &&
           Position <= (size() - static_cast<intptr_t>(sizeof(T))));
    T Value;
    memcpy(&Value, reinterpret_cast<void *>(Contents + Position), sizeof(T));
    return Value;
  }

  template <typename T> void store(intptr_t Position, T Value) {
    assert(Position >= 0 &&
           Position <= (size() - static_cast<intptr_t>(sizeof(T))));
    memcpy(reinterpret_cast<void *>(Contents + Position), &Value, sizeof(T));
  }
  /// @{

  /// Emit a fixup at the current location.
  void emitFixup(AssemblerFixup *Fixup) { Fixup->set_position(size()); }

  /// Get the size of the emitted code.
  intptr_t size() const { return Cursor - Contents; }
  uintptr_t contents() const { return Contents; }

  /// To emit an instruction to the assembler buffer, the EnsureCapacity helper
  /// must be used to guarantee that the underlying data area is big enough to
  /// hold the emitted instruction. Usage:
  ///
  ///     AssemblerBuffer buffer;
  ///     AssemblerBuffer::EnsureCapacity ensured(&buffer);
  ///     ... emit bytes for single instruction ...
  class EnsureCapacity {
    EnsureCapacity(const EnsureCapacity &) = delete;
    EnsureCapacity &operator=(const EnsureCapacity &) = delete;

  public:
    explicit EnsureCapacity(AssemblerBuffer *Buffer) : Buffer(Buffer) {
      if (Buffer->cursor() >= Buffer->limit())
        Buffer->extendCapacity();
      if (BuildDefs::asserts())
        validate(Buffer);
    }
    ~EnsureCapacity();

  private:
    AssemblerBuffer *Buffer;
    intptr_t Gap = 0;

    void validate(AssemblerBuffer *Buffer);
    intptr_t computeGap() { return Buffer->capacity() - Buffer->size(); }
  };

  bool HasEnsuredCapacity;
  bool hasEnsuredCapacity() const {
    if (BuildDefs::asserts())
      return HasEnsuredCapacity;
    // Disable the actual check in non-debug mode.
    return true;
  }

  /// Returns the position in the instruction stream.
  intptr_t getPosition() const { return Cursor - Contents; }

  /// Create and track a fixup in the current function.
  AssemblerFixup *createFixup(FixupKind Kind, const Constant *Value);

  /// Create and track a textual fixup in the current function.
  AssemblerTextFixup *createTextFixup(const std::string &Text,
                                      size_t BytesUsed);

  /// Mark that an attempt was made to emit, but failed. Hence, in order to
  /// continue, one must emit a text fixup.
  void setNeedsTextFixup() { TextFixupNeeded = true; }
  void resetNeedsTextFixup() { TextFixupNeeded = false; }

  /// Returns true if last emit failed and needs a text fixup.
  bool needsTextFixup() const { return TextFixupNeeded; }

  /// Installs a created fixup, after it has been allocated.
  void installFixup(AssemblerFixup *F);

  const FixupRefList &fixups() const { return Fixups; }

  void setSize(intptr_t NewSize) {
    assert(NewSize <= size());
    Cursor = Contents + NewSize;
  }

private:
  /// The limit is set to kMinimumGap bytes before the end of the data area.
  /// This leaves enough space for the longest possible instruction and allows
  /// for a single, fast space check per instruction.
  static constexpr intptr_t kMinimumGap = 32;

  uintptr_t Contents;
  uintptr_t Cursor;
  uintptr_t Limit;
  // The member variable is named Assemblr to avoid hiding the class Assembler.
  Assembler &Assemblr;
  /// List of pool-allocated fixups relative to the current function.
  FixupRefList Fixups;
  // True if a textual fixup is needed, because the assembler was unable to
  // emit the last request.
  bool TextFixupNeeded;

  uintptr_t cursor() const { return Cursor; }
  uintptr_t limit() const { return Limit; }
  intptr_t capacity() const {
    assert(Limit >= Contents);
    return (Limit - Contents) + kMinimumGap;
  }

  /// Compute the limit based on the data area and the capacity. See description
  /// of kMinimumGap for the reasoning behind the value.
  static uintptr_t computeLimit(uintptr_t Data, intptr_t Capacity) {
    return Data + Capacity - kMinimumGap;
  }

  void extendCapacity();
};

class Assembler {
  Assembler() = delete;
  Assembler(const Assembler &) = delete;
  Assembler &operator=(const Assembler &) = delete;

public:
  enum AssemblerKind {
    Asm_ARM32,
    Asm_MIPS32,
    Asm_X8632,
    Asm_X8664,
  };

  virtual ~Assembler() = default;

  /// Allocate a chunk of bytes using the per-Assembler allocator.
  uintptr_t allocateBytes(size_t bytes) {
    // For now, alignment is not related to NaCl bundle alignment, since the
    // buffer's GetPosition is relative to the base. So NaCl bundle alignment
    // checks can be relative to that base. Later, the buffer will be copied
    // out to a ".text" section (or an in memory-buffer that can be mprotect'ed
    // with executable permission), and that second buffer should be aligned
    // for NaCl.
    const size_t Alignment = 16;
    return reinterpret_cast<uintptr_t>(Allocator.Allocate(bytes, Alignment));
  }

  /// Allocate data of type T using the per-Assembler allocator.
  template <typename T> T *allocate() { return Allocator.Allocate<T>(); }

  /// Align the tail end of the function to the required target alignment.
  virtual void alignFunction() = 0;
  /// Align the tail end of the basic block to the required target alignment.
  void alignCfgNode() {
    const SizeT Align = 1 << getBundleAlignLog2Bytes();
    padWithNop(Utils::OffsetToAlignment(Buffer.getPosition(), Align));
  }

  /// Add nop padding of a particular width to the current bundle.
  virtual void padWithNop(intptr_t Padding) = 0;

  virtual SizeT getBundleAlignLog2Bytes() const = 0;

  virtual const char *getAlignDirective() const = 0;
  virtual llvm::ArrayRef<uint8_t> getNonExecBundlePadding() const = 0;

  /// Get the label for a CfgNode.
  virtual Label *getCfgNodeLabel(SizeT NodeNumber) = 0;
  /// Mark the current text location as the start of a CFG node.
  virtual void bindCfgNodeLabel(const CfgNode *Node) = 0;

  virtual bool fixupIsPCRel(FixupKind Kind) const = 0;

  /// Return a view of all the bytes of code for the current function.
  llvm::StringRef getBufferView() const;

  /// Return the value of the given type in the corresponding buffer.
  template <typename T> T load(intptr_t Position) const {
    return Buffer.load<T>(Position);
  }

  template <typename T> void store(intptr_t Position, T Value) {
    Buffer.store(Position, Value);
  }

  /// Emit a fixup at the current location.
  void emitFixup(AssemblerFixup *Fixup) { Buffer.emitFixup(Fixup); }

  const FixupRefList &fixups() const { return Buffer.fixups(); }

  AssemblerFixup *createFixup(FixupKind Kind, const Constant *Value) {
    return Buffer.createFixup(Kind, Value);
  }

  AssemblerTextFixup *createTextFixup(const std::string &Text,
                                      size_t BytesUsed) {
    return Buffer.createTextFixup(Text, BytesUsed);
  }

  void bindRelocOffset(RelocOffset *Offset);

  void setNeedsTextFixup() { Buffer.setNeedsTextFixup(); }
  void resetNeedsTextFixup() { Buffer.resetNeedsTextFixup(); }

  bool needsTextFixup() const { return Buffer.needsTextFixup(); }

  void emitIASBytes(GlobalContext *Ctx) const;
  bool getInternal() const { return IsInternal; }
  void setInternal(bool Internal) { IsInternal = Internal; }
  GlobalString getFunctionName() const { return FunctionName; }
  void setFunctionName(GlobalString NewName) { FunctionName = NewName; }
  intptr_t getBufferSize() const { return Buffer.size(); }
  /// Roll back to a (smaller) size.
  void setBufferSize(intptr_t NewSize) { Buffer.setSize(NewSize); }
  void setPreliminary(bool Value) { Preliminary = Value; }
  bool getPreliminary() const { return Preliminary; }

  AssemblerKind getKind() const { return Kind; }

protected:
  explicit Assembler(AssemblerKind Kind)
      : Kind(Kind), Allocator(), Buffer(*this) {}

private:
  const AssemblerKind Kind;

  using AssemblerAllocator =
      llvm::BumpPtrAllocatorImpl<llvm::MallocAllocator, /*SlabSize=*/32 * 1024>;
  AssemblerAllocator Allocator;

  /// FunctionName and IsInternal are transferred from the original Cfg object,
  /// since the Cfg object may be deleted by the time the assembler buffer is
  /// emitted.
  GlobalString FunctionName;
  bool IsInternal = false;
  /// Preliminary indicates whether a preliminary pass is being made for
  /// calculating bundle padding (Preliminary=true), versus the final pass where
  /// all changes to label bindings, label links, and relocation fixups are
  /// fully committed (Preliminary=false).
  bool Preliminary = false;

  /// Installs a created fixup, after it has been allocated.
  void installFixup(AssemblerFixup *F) { Buffer.installFixup(F); }

protected:
  // Buffer's constructor uses the Allocator, so it needs to appear after it.
  // TODO(jpp): dependencies on construction order are a nice way of shooting
  // yourself in the foot. Fix this.
  AssemblerBuffer Buffer;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEASSEMBLER_H_
