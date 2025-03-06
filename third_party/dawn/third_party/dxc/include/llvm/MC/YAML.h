
#ifndef LLVM_MC_YAML_H
#define LLVM_MC_YAML_H

#include "llvm/Support/YAMLTraits.h"

namespace llvm {
namespace yaml {
/// \brief Specialized YAMLIO scalar type for representing a binary blob.
///
/// A typical use case would be to represent the content of a section in a
/// binary file.
/// This class has custom YAMLIO traits for convenient reading and writing.
/// It renders as a string of hex digits in a YAML file.
/// For example, it might render as `DEADBEEFCAFEBABE` (YAML does not
/// require the quotation marks, so for simplicity when outputting they are
/// omitted).
/// When reading, any string whose content is an even number of hex digits
/// will be accepted.
/// For example, all of the following are acceptable:
/// `DEADBEEF`, `"DeADbEeF"`, `"\x44EADBEEF"` (Note: '\x44' == 'D')
///
/// A significant advantage of using this class is that it never allocates
/// temporary strings or buffers for any of its functionality.
///
/// Example:
///
/// The YAML mapping:
/// \code
/// Foo: DEADBEEFCAFEBABE
/// \endcode
///
/// Could be modeled in YAMLIO by the struct:
/// \code
/// struct FooHolder {
///   BinaryRef Foo;
/// };
/// namespace llvm {
/// namespace yaml {
/// template <>
/// struct MappingTraits<FooHolder> {
///   static void mapping(IO &IO, FooHolder &FH) {
///     IO.mapRequired("Foo", FH.Foo);
///   }
/// };
/// } // end namespace yaml
/// } // end namespace llvm
/// \endcode
class BinaryRef {
  friend bool operator==(const BinaryRef &LHS, const BinaryRef &RHS);
  /// \brief Either raw binary data, or a string of hex bytes (must always
  /// be an even number of characters).
  ArrayRef<uint8_t> Data;
  /// \brief Discriminator between the two states of the `Data` member.
  bool DataIsHexString;

public:
  BinaryRef(ArrayRef<uint8_t> Data) : Data(Data), DataIsHexString(false) {}
  BinaryRef(StringRef Data)
      : Data(reinterpret_cast<const uint8_t *>(Data.data()), Data.size()),
        DataIsHexString(true) {}
  BinaryRef() : DataIsHexString(true) {}
  /// \brief The number of bytes that are represented by this BinaryRef.
  /// This is the number of bytes that writeAsBinary() will write.
  ArrayRef<uint8_t>::size_type binary_size() const {
    if (DataIsHexString)
      return Data.size() / 2;
    return Data.size();
  }
  /// \brief Write the contents (regardless of whether it is binary or a
  /// hex string) as binary to the given raw_ostream.
  void writeAsBinary(raw_ostream &OS) const;
  /// \brief Write the contents (regardless of whether it is binary or a
  /// hex string) as hex to the given raw_ostream.
  ///
  /// For example, a possible output could be `DEADBEEFCAFEBABE`.
  void writeAsHex(raw_ostream &OS) const;
};

inline bool operator==(const BinaryRef &LHS, const BinaryRef &RHS) {
  // Special case for default constructed BinaryRef.
  if (LHS.Data.empty() && RHS.Data.empty())
    return true;

  return LHS.DataIsHexString == RHS.DataIsHexString && LHS.Data == RHS.Data;
}

template <> struct ScalarTraits<BinaryRef> {
  static void output(const BinaryRef &, void *, llvm::raw_ostream &);
  static StringRef input(StringRef, void *, BinaryRef &);
  static bool mustQuote(StringRef S) { return needsQuotes(S); }
};
}
}
#endif
