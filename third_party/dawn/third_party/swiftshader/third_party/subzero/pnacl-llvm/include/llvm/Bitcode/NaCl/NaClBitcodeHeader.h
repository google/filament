//===-- llvm/Bitcode/NaCl/NaClBitcodeHeader.h - ----------------*- C++ -*-===//
//      NaCl Bitcode header reader.
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header defines interfaces to read and write NaCl bitcode wire format
// file headers.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_BITCODE_NACL_NACLBITCODEHEADER_H
#define LLVM_BITCODE_NACL_NACLBITCODEHEADER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/DataTypes.h"
#include <string>
#include <vector>

namespace llvm {
class MemoryObject;

// Class representing a variable-size metadata field in the bitcode header.
// Also contains the list of known (typed) Tag IDs.
//
// The serialized format has 2 fixed subfields (ID:type and data length) and the
// variable-length data subfield
class NaClBitcodeHeaderField {
  NaClBitcodeHeaderField(const NaClBitcodeHeaderField &) = delete;
  void operator=(const NaClBitcodeHeaderField &) = delete;

public:
  // Defines the ID associated with the value. Valid values are in
  // {0x0, ..., 0xFFF}
  typedef enum {
    kInvalid = 0,             // KUnknownType.
    kPNaClVersion = 1,        // kUint32Type.
    kAlignBitcodeRecords = 2, // kFlagType.
    kTag_MAX = kAlignBitcodeRecords
  } Tag;
  // Defines the type of value.
  typedef enum {
    kBufferType, // Buffer of form uint8_t[len].
    kUInt32Type,
    kFlagType,
    kUnknownType,
    kFieldType_MAX = kUnknownType
  } FieldType;
  // Defines the number of bytes in a (32-bit) word.
  static const int WordSize = 4;

  // Defines the encoding of the fixed fields {i.e. ID:type and data length).
  typedef uint16_t FixedSubfield;

  // Create an invalid header field.
  NaClBitcodeHeaderField();

  // Creates a header field where MyID is a flag.
  NaClBitcodeHeaderField(Tag MyID);

  // Create a header field with an uint32_t value.
  NaClBitcodeHeaderField(Tag MyID, uint32_t value);

  // Create a header field for the given data.
  NaClBitcodeHeaderField(Tag MyID, size_t MyLen, uint8_t *MyData);

  virtual ~NaClBitcodeHeaderField() {
    if (Data)
      delete[] Data;
  }

  /// \brief Number of bytes used to represent header field.
  size_t GetTotalSize() const {
    // Round up to 4 byte alignment
    return (kTagLenSize + Len + (WordSize - 1)) & ~(WordSize - 1);
  }

  /// \brief Write field into Buf[BufLen].
  bool Write(uint8_t *Buf, size_t BufLen) const;

  /// \brief Read field from Buf[BufLen].
  bool Read(const uint8_t *Buf, size_t BufLen);

  /// \brief Returns string describing ID of field.
  static const char *IDName(Tag ID);
  const char *IDName() const { return IDName(ID); }

  /// \brief Returns string describing type of field.
  static const char *TypeName(FieldType FType);
  const char *TypeName() const { return TypeName(FType); }

  /// \brief Returns string describing field.
  std::string Contents() const;

  /// \brief Get the data size from a serialized field to allow allocation.
  static size_t GetDataSizeFromSerialized(const uint8_t *Buf) {
    FixedSubfield Length;
    ReadFixedSubfield(&Length, Buf + sizeof(FixedSubfield));
    return Length;
  }

  /// \brief Return the ID of the field.
  Tag GetID() const { return ID; }

  FieldType GetType() const { return FType; }

  /// \brief Return the length of the data (in bytes).
  size_t GetLen() const { return Len; }

  /// \brief Return the data. Data is array getData()[getLen()].
  const uint8_t *GetData() const { return Data; }

  /// \brief Returns the uint32_t value stored. Requires that
  /// getType() == kUint32Type
  uint32_t GetUInt32Value() const;

private:
  // Convert ID:Type into a fixed subfield
  FixedSubfield EncodeTypedID() const { return (ID << 4) | FType; }
  // Extract out ID and Type from a fixed subfield.
  void DecodeTypedID(FixedSubfield Subfield, Tag &ID, FieldType &FType) {
    FixedSubfield PossibleID = Subfield >> 4;
    ID = (PossibleID > kTag_MAX ? kInvalid : static_cast<Tag>(PossibleID));
    FixedSubfield PossibleFType = Subfield & 0xF;
    FType = (PossibleFType > kFieldType_MAX
                 ? kUnknownType
                 : static_cast<FieldType>(PossibleFType));
  }
  // Combined size of the fixed subfields
  const static size_t kTagLenSize = 2 * sizeof(FixedSubfield);
  static void WriteFixedSubfield(FixedSubfield Value, uint8_t *Buf) {
    Buf[0] = Value & 0xFF;
    Buf[1] = (Value >> 8) & 0xFF;
  }
  static void ReadFixedSubfield(FixedSubfield *Value, const uint8_t *Buf) {
    *Value = Buf[0] | Buf[1] << 8;
  }
  Tag ID;
  FieldType FType;
  size_t Len;
  uint8_t *Data;
};

/// \brief Class holding parsed header fields in PNaCl bitcode file.
class NaClBitcodeHeader {
  NaClBitcodeHeader(const NaClBitcodeHeader &) = delete;
  void operator=(const NaClBitcodeHeader &) = delete;

  // The set of parsed header fields. The header takes ownership of
  // all fields in this vector.
  std::vector<NaClBitcodeHeaderField *> Fields;
  // The number of bytes in the PNaCl header.
  size_t HeaderSize;
  // String defining why it is unsupported (if unsupported).
  std::string UnsupportedMessage;
  // Flag defining if header is supported.
  bool IsSupportedFlag;
  // Flag defining if the corresponding bitcode file is readable.
  bool IsReadableFlag;
  // Defines the PNaCl version defined by the header file.
  uint32_t PNaClVersion;
  // Byte align bitcode records when nonzero.
  bool AlignBitcodeRecords = false;

public:
  static const int WordSize = NaClBitcodeHeaderField::WordSize;

  NaClBitcodeHeader();
  ~NaClBitcodeHeader();

  /// \brief Installs the fields of the header, defining if the header
  /// is readable and supported. Sets UnsupportedMessage on failure.
  void InstallFields();

  /// \brief Adds a field to the list of fields in a header. Takes ownership
  /// of fields added.
  void push_back(NaClBitcodeHeaderField *Field) { Fields.push_back(Field); }

  /// \brief Read the PNaCl bitcode header, The format of the header is:
  ///
  ///    1) 'PEXE' - The four character sequence defining the magic number.
  ///    2) uint_16 num_fields - The number of NaClBitcodeHeaderField's.
  ///    3) uint_16 num_bytes - The number of bytes to hold fields in
  ///                           the header.
  ///    4) NaClBitcodeHeaderField f1 - The first bitcode header field.
  ///    ...
  ///    2 + num_fields) NaClBitcodeHeaderField fn - The last bitcode header
  /// field.
  ///
  /// Returns false if able to read (all of) the bitcode header.
  bool Read(const unsigned char *BufPtr, const unsigned char *BufEnd);

  // \brief Read the PNaCl bitcode header, recording the fields found
  // in the header. Returns false if able to read (all of) the bitcode header.
  bool Read(MemoryObject *Bytes);

  // \brief Returns the number of bytes read to consume the header.
  size_t getHeaderSize() { return HeaderSize; }

  /// \brief Returns string describing why the header describes
  /// an unsupported PNaCl Bitcode file.
  const std::string &Unsupported() const { return UnsupportedMessage; }

  /// \brief Returns true if supported. That is, it can be run in the
  /// browser.
  bool IsSupported() const { return IsSupportedFlag; }

  /// \brief Returns true if the bitcode file should be readable. Note
  /// that just because it is readable, it doesn't necessarily mean that
  /// it is supported.
  bool IsReadable() const { return IsReadableFlag; }

  /// \brief Returns number of fields defined.
  size_t NumberFields() const { return Fields.size(); }

  /// \brief Returns a pointer to the field with the given ID
  /// (0 if no such field).
  NaClBitcodeHeaderField *GetTaggedField(NaClBitcodeHeaderField::Tag ID) const;

  /// \brief Returns a pointer to the Nth field in the header
  /// (0 if no such field).
  NaClBitcodeHeaderField *GetField(size_t index) const;

  /// \brief Returns the PNaClVersion, as defined by the header.
  uint32_t GetPNaClVersion() const { return PNaClVersion; }

  /// \brief Returns if one should byte align bitcode records.
  bool getAlignBitcodeRecords() const { return AlignBitcodeRecords; }

private:
  // Reads and verifies the first 8 bytes of the header, consisting
  // of the magic number 'PEXE', and the value defining the number
  // of fields and number of bytes used to hold fields.
  // Returns false if successful, sets UnsupportedMessage otherwise.
  bool ReadPrefix(const unsigned char *BufPtr, const unsigned char *BufEnd,
                  unsigned &NumFields, unsigned &NumBytes);

  // Reads and verifies the fields in the header.
  // Returns false if successful, sets UnsupportedMessage otherwise.
  bool ReadFields(const unsigned char *BufPtr, const unsigned char *BufEnd,
                  unsigned NumFields, unsigned NumBytes);

  // Sets the Unsupported error message and returns true.
  bool UnsupportedError(StringRef Message) {
    UnsupportedMessage = Message.str();
    return true;
  }
};

} // namespace llvm

#endif
