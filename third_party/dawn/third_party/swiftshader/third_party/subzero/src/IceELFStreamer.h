//===- subzero/src/IceELFStreamer.h - Low level ELF writing -----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Interface for serializing bits for common ELF types (words, extended
/// words, etc.), based on the ELF class.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEELFSTREAMER_H
#define SUBZERO_SRC_ICEELFSTREAMER_H

#include "IceDefs.h"

namespace Ice {

/// Low level writer that can that can handle ELFCLASS32/64. Little endian only
/// for now.
class ELFStreamer {
  ELFStreamer(const ELFStreamer &) = delete;
  ELFStreamer &operator=(const ELFStreamer &) = delete;

public:
  ELFStreamer() = default;
  virtual ~ELFStreamer() = default;

  virtual void write8(uint8_t Value) = 0;
  virtual uint64_t tell() const = 0;
  virtual void seek(uint64_t Off) = 0;

  virtual void writeBytes(llvm::StringRef Bytes) {
    for (char c : Bytes) {
      write8(c);
    }
  }

  void writeLE16(uint16_t Value) {
    write8(uint8_t(Value));
    write8(uint8_t(Value >> 8));
  }

  void writeLE32(uint32_t Value) {
    writeLE16(uint16_t(Value));
    writeLE16(uint16_t(Value >> 16));
  }

  void writeLE64(uint64_t Value) {
    writeLE32(uint32_t(Value));
    writeLE32(uint32_t(Value >> 32));
  }

  template <bool IsELF64, typename T> void writeAddrOrOffset(T Value) {
    if (IsELF64)
      writeLE64(Value);
    else
      writeLE32(Value);
  }

  template <bool IsELF64, typename T> void writeELFWord(T Value) {
    writeLE32(Value);
  }

  template <bool IsELF64, typename T> void writeELFXword(T Value) {
    if (IsELF64)
      writeLE64(Value);
    else
      writeLE32(Value);
  }

  void writeZeroPadding(SizeT N) {
    static const char Zeros[16] = {0};

    for (SizeT i = 0, e = N / 16; i != e; ++i)
      writeBytes(llvm::StringRef(Zeros, 16));

    writeBytes(llvm::StringRef(Zeros, N % 16));
  }
};

/// Implementation of ELFStreamer writing to a file.
class ELFFileStreamer : public ELFStreamer {
  ELFFileStreamer() = delete;
  ELFFileStreamer(const ELFFileStreamer &) = delete;
  ELFFileStreamer &operator=(const ELFFileStreamer &) = delete;

public:
  explicit ELFFileStreamer(Fdstream &Out) : Out(Out) {}

  void write8(uint8_t Value) override { Out << char(Value); }

  void writeBytes(llvm::StringRef Bytes) override { Out << Bytes; }

  uint64_t tell() const override { return Out.tell(); }

  void seek(uint64_t Off) override { Out.seek(Off); }

private:
  Fdstream &Out;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEELFSTREAMER_H