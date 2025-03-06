//===-- llvm/CodeGen/DIEHash.cpp - Dwarf Hashing Framework ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains support for DWARF4 hashing of DIEs.
//
//===----------------------------------------------------------------------===//

#include "ByteStreamer.h"
#include "DIEHash.h"
#include "DwarfDebug.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/DIE.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/MD5.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "dwarfdebug"

/// \brief Grabs the string in whichever attribute is passed in and returns
/// a reference to it.
static StringRef getDIEStringAttr(const DIE &Die, uint16_t Attr) {
  // Iterate through all the attributes until we find the one we're
  // looking for, if we can't find it return an empty string.
  for (const auto &V : Die.values())
    if (V.getAttribute() == Attr)
      return V.getDIEString().getString();

  return StringRef("");
}

/// \brief Adds the string in \p Str to the hash. This also hashes
/// a trailing NULL with the string.
void DIEHash::addString(StringRef Str) {
  DEBUG(dbgs() << "Adding string " << Str << " to hash.\n");
  Hash.update(Str);
  Hash.update(makeArrayRef((uint8_t)'\0'));
}

// FIXME: The LEB128 routines are copied and only slightly modified out of
// LEB128.h.

/// \brief Adds the unsigned in \p Value to the hash encoded as a ULEB128.
void DIEHash::addULEB128(uint64_t Value) {
  DEBUG(dbgs() << "Adding ULEB128 " << Value << " to hash.\n");
  do {
    uint8_t Byte = Value & 0x7f;
    Value >>= 7;
    if (Value != 0)
      Byte |= 0x80; // Mark this byte to show that more bytes will follow.
    Hash.update(Byte);
  } while (Value != 0);
}

void DIEHash::addSLEB128(int64_t Value) {
  DEBUG(dbgs() << "Adding ULEB128 " << Value << " to hash.\n");
  bool More;
  do {
    uint8_t Byte = Value & 0x7f;
    Value >>= 7;
    More = !((((Value == 0) && ((Byte & 0x40) == 0)) ||
              ((Value == -1) && ((Byte & 0x40) != 0))));
    if (More)
      Byte |= 0x80; // Mark this byte to show that more bytes will follow.
    Hash.update(Byte);
  } while (More);
}

/// \brief Including \p Parent adds the context of Parent to the hash..
void DIEHash::addParentContext(const DIE &Parent) {

  DEBUG(dbgs() << "Adding parent context to hash...\n");

  // [7.27.2] For each surrounding type or namespace beginning with the
  // outermost such construct...
  SmallVector<const DIE *, 1> Parents;
  const DIE *Cur = &Parent;
  while (Cur->getParent()) {
    Parents.push_back(Cur);
    Cur = Cur->getParent();
  }
  assert(Cur->getTag() == dwarf::DW_TAG_compile_unit ||
         Cur->getTag() == dwarf::DW_TAG_type_unit);

  // Reverse iterate over our list to go from the outermost construct to the
  // innermost.
  for (SmallVectorImpl<const DIE *>::reverse_iterator I = Parents.rbegin(),
                                                      E = Parents.rend();
       I != E; ++I) {
    const DIE &Die = **I;

    // ... Append the letter "C" to the sequence...
    addULEB128('C');

    // ... Followed by the DWARF tag of the construct...
    addULEB128(Die.getTag());

    // ... Then the name, taken from the DW_AT_name attribute.
    StringRef Name = getDIEStringAttr(Die, dwarf::DW_AT_name);
    DEBUG(dbgs() << "... adding context: " << Name << "\n");
    if (!Name.empty())
      addString(Name);
  }
}

// Collect all of the attributes for a particular DIE in single structure.
void DIEHash::collectAttributes(const DIE &Die, DIEAttrs &Attrs) {
#define COLLECT_ATTR(NAME)                                                     \
  case dwarf::NAME:                                                            \
    Attrs.NAME = V;                                                            \
    break

  for (const auto &V : Die.values()) {
    DEBUG(dbgs() << "Attribute: "
                 << dwarf::AttributeString(V.getAttribute())
                 << " added.\n");
    switch (V.getAttribute()) {
      COLLECT_ATTR(DW_AT_name);
      COLLECT_ATTR(DW_AT_accessibility);
      COLLECT_ATTR(DW_AT_address_class);
      COLLECT_ATTR(DW_AT_allocated);
      COLLECT_ATTR(DW_AT_artificial);
      COLLECT_ATTR(DW_AT_associated);
      COLLECT_ATTR(DW_AT_binary_scale);
      COLLECT_ATTR(DW_AT_bit_offset);
      COLLECT_ATTR(DW_AT_bit_size);
      COLLECT_ATTR(DW_AT_bit_stride);
      COLLECT_ATTR(DW_AT_byte_size);
      COLLECT_ATTR(DW_AT_byte_stride);
      COLLECT_ATTR(DW_AT_const_expr);
      COLLECT_ATTR(DW_AT_const_value);
      COLLECT_ATTR(DW_AT_containing_type);
      COLLECT_ATTR(DW_AT_count);
      COLLECT_ATTR(DW_AT_data_bit_offset);
      COLLECT_ATTR(DW_AT_data_location);
      COLLECT_ATTR(DW_AT_data_member_location);
      COLLECT_ATTR(DW_AT_decimal_scale);
      COLLECT_ATTR(DW_AT_decimal_sign);
      COLLECT_ATTR(DW_AT_default_value);
      COLLECT_ATTR(DW_AT_digit_count);
      COLLECT_ATTR(DW_AT_discr);
      COLLECT_ATTR(DW_AT_discr_list);
      COLLECT_ATTR(DW_AT_discr_value);
      COLLECT_ATTR(DW_AT_encoding);
      COLLECT_ATTR(DW_AT_enum_class);
      COLLECT_ATTR(DW_AT_endianity);
      COLLECT_ATTR(DW_AT_explicit);
      COLLECT_ATTR(DW_AT_is_optional);
      COLLECT_ATTR(DW_AT_location);
      COLLECT_ATTR(DW_AT_lower_bound);
      COLLECT_ATTR(DW_AT_mutable);
      COLLECT_ATTR(DW_AT_ordering);
      COLLECT_ATTR(DW_AT_picture_string);
      COLLECT_ATTR(DW_AT_prototyped);
      COLLECT_ATTR(DW_AT_small);
      COLLECT_ATTR(DW_AT_segment);
      COLLECT_ATTR(DW_AT_string_length);
      COLLECT_ATTR(DW_AT_threads_scaled);
      COLLECT_ATTR(DW_AT_upper_bound);
      COLLECT_ATTR(DW_AT_use_location);
      COLLECT_ATTR(DW_AT_use_UTF8);
      COLLECT_ATTR(DW_AT_variable_parameter);
      COLLECT_ATTR(DW_AT_virtuality);
      COLLECT_ATTR(DW_AT_visibility);
      COLLECT_ATTR(DW_AT_vtable_elem_location);
      COLLECT_ATTR(DW_AT_type);
    default:
      break;
    }
  }
}

void DIEHash::hashShallowTypeReference(dwarf::Attribute Attribute,
                                       const DIE &Entry, StringRef Name) {
  // append the letter 'N'
  addULEB128('N');

  // the DWARF attribute code (DW_AT_type or DW_AT_friend),
  addULEB128(Attribute);

  // the context of the tag,
  if (const DIE *Parent = Entry.getParent())
    addParentContext(*Parent);

  // the letter 'E',
  addULEB128('E');

  // and the name of the type.
  addString(Name);

  // Currently DW_TAG_friends are not used by Clang, but if they do become so,
  // here's the relevant spec text to implement:
  //
  // For DW_TAG_friend, if the referenced entry is the DW_TAG_subprogram,
  // the context is omitted and the name to be used is the ABI-specific name
  // of the subprogram (e.g., the mangled linker name).
}

void DIEHash::hashRepeatedTypeReference(dwarf::Attribute Attribute,
                                        unsigned DieNumber) {
  // a) If T is in the list of [previously hashed types], use the letter
  // 'R' as the marker
  addULEB128('R');

  addULEB128(Attribute);

  // and use the unsigned LEB128 encoding of [the index of T in the
  // list] as the attribute value;
  addULEB128(DieNumber);
}

void DIEHash::hashDIEEntry(dwarf::Attribute Attribute, dwarf::Tag Tag,
                           const DIE &Entry) {
  assert(Tag != dwarf::DW_TAG_friend && "No current LLVM clients emit friend "
                                        "tags. Add support here when there's "
                                        "a use case");
  // Step 5
  // If the tag in Step 3 is one of [the below tags]
  if ((Tag == dwarf::DW_TAG_pointer_type ||
       Tag == dwarf::DW_TAG_reference_type ||
       Tag == dwarf::DW_TAG_rvalue_reference_type ||
       Tag == dwarf::DW_TAG_ptr_to_member_type) &&
      // and the referenced type (via the [below attributes])
      // FIXME: This seems overly restrictive, and causes hash mismatches
      // there's a decl/def difference in the containing type of a
      // ptr_to_member_type, but it's what DWARF says, for some reason.
      Attribute == dwarf::DW_AT_type) {
    // ... has a DW_AT_name attribute,
    StringRef Name = getDIEStringAttr(Entry, dwarf::DW_AT_name);
    if (!Name.empty()) {
      hashShallowTypeReference(Attribute, Entry, Name);
      return;
    }
  }

  unsigned &DieNumber = Numbering[&Entry];
  if (DieNumber) {
    hashRepeatedTypeReference(Attribute, DieNumber);
    return;
  }

  // otherwise, b) use the letter 'T' as the marker, ...
  addULEB128('T');

  addULEB128(Attribute);

  // ... process the type T recursively by performing Steps 2 through 7, and
  // use the result as the attribute value.
  DieNumber = Numbering.size();
  computeHash(Entry);
}

// Hash all of the values in a block like set of values. This assumes that
// all of the data is going to be added as integers.
void DIEHash::hashBlockData(const DIE::const_value_range &Values) {
  for (const auto &V : Values)
    Hash.update((uint64_t)V.getDIEInteger().getValue());
}

// Hash the contents of a loclistptr class.
void DIEHash::hashLocList(const DIELocList &LocList) {
  HashingByteStreamer Streamer(*this);
  DwarfDebug &DD = *AP->getDwarfDebug();
  const DebugLocStream &Locs = DD.getDebugLocs();
  for (const auto &Entry : Locs.getEntries(Locs.getList(LocList.getValue())))
    DD.emitDebugLocEntry(Streamer, Entry);
}

// Hash an individual attribute \param Attr based on the type of attribute and
// the form.
void DIEHash::hashAttribute(DIEValue Value, dwarf::Tag Tag) {
  dwarf::Attribute Attribute = Value.getAttribute();

  // Other attribute values use the letter 'A' as the marker, and the value
  // consists of the form code (encoded as an unsigned LEB128 value) followed by
  // the encoding of the value according to the form code. To ensure
  // reproducibility of the signature, the set of forms used in the signature
  // computation is limited to the following: DW_FORM_sdata, DW_FORM_flag,
  // DW_FORM_string, and DW_FORM_block.

  switch (Value.getType()) {
  case DIEValue::isNone:
    llvm_unreachable("Expected valid DIEValue");

    // 7.27 Step 3
    // ... An attribute that refers to another type entry T is processed as
    // follows:
  case DIEValue::isEntry:
    hashDIEEntry(Attribute, Tag, Value.getDIEEntry().getEntry());
    break;
  case DIEValue::isInteger: {
    addULEB128('A');
    addULEB128(Attribute);
    switch (Value.getForm()) {
    case dwarf::DW_FORM_data1:
    case dwarf::DW_FORM_data2:
    case dwarf::DW_FORM_data4:
    case dwarf::DW_FORM_data8:
    case dwarf::DW_FORM_udata:
    case dwarf::DW_FORM_sdata:
      addULEB128(dwarf::DW_FORM_sdata);
      addSLEB128((int64_t)Value.getDIEInteger().getValue());
      break;
    // DW_FORM_flag_present is just flag with a value of one. We still give it a
    // value so just use the value.
    case dwarf::DW_FORM_flag_present:
    case dwarf::DW_FORM_flag:
      addULEB128(dwarf::DW_FORM_flag);
      addULEB128((int64_t)Value.getDIEInteger().getValue());
      break;
    default:
      llvm_unreachable("Unknown integer form!");
    }
    break;
  }
  case DIEValue::isString:
    addULEB128('A');
    addULEB128(Attribute);
    addULEB128(dwarf::DW_FORM_string);
    addString(Value.getDIEString().getString());
    break;
  case DIEValue::isBlock:
  case DIEValue::isLoc:
  case DIEValue::isLocList:
    addULEB128('A');
    addULEB128(Attribute);
    addULEB128(dwarf::DW_FORM_block);
    if (Value.getType() == DIEValue::isBlock) {
      addULEB128(Value.getDIEBlock().ComputeSize(AP));
      hashBlockData(Value.getDIEBlock().values());
    } else if (Value.getType() == DIEValue::isLoc) {
      addULEB128(Value.getDIELoc().ComputeSize(AP));
      hashBlockData(Value.getDIELoc().values());
    } else {
      // We could add the block length, but that would take
      // a bit of work and not add a lot of uniqueness
      // to the hash in some way we could test.
      hashLocList(Value.getDIELocList());
    }
    break;
    // FIXME: It's uncertain whether or not we should handle this at the moment.
  case DIEValue::isExpr:
  case DIEValue::isLabel:
  case DIEValue::isDelta:
  case DIEValue::isTypeSignature:
    llvm_unreachable("Add support for additional value types.");
  }
}

// Go through the attributes from \param Attrs in the order specified in 7.27.4
// and hash them.
void DIEHash::hashAttributes(const DIEAttrs &Attrs, dwarf::Tag Tag) {
#define ADD_ATTR(ATTR)                                                         \
  {                                                                            \
    if (ATTR)                                                                  \
      hashAttribute(ATTR, Tag);                                                \
  }

  ADD_ATTR(Attrs.DW_AT_name);
  ADD_ATTR(Attrs.DW_AT_accessibility);
  ADD_ATTR(Attrs.DW_AT_address_class);
  ADD_ATTR(Attrs.DW_AT_allocated);
  ADD_ATTR(Attrs.DW_AT_artificial);
  ADD_ATTR(Attrs.DW_AT_associated);
  ADD_ATTR(Attrs.DW_AT_binary_scale);
  ADD_ATTR(Attrs.DW_AT_bit_offset);
  ADD_ATTR(Attrs.DW_AT_bit_size);
  ADD_ATTR(Attrs.DW_AT_bit_stride);
  ADD_ATTR(Attrs.DW_AT_byte_size);
  ADD_ATTR(Attrs.DW_AT_byte_stride);
  ADD_ATTR(Attrs.DW_AT_const_expr);
  ADD_ATTR(Attrs.DW_AT_const_value);
  ADD_ATTR(Attrs.DW_AT_containing_type);
  ADD_ATTR(Attrs.DW_AT_count);
  ADD_ATTR(Attrs.DW_AT_data_bit_offset);
  ADD_ATTR(Attrs.DW_AT_data_location);
  ADD_ATTR(Attrs.DW_AT_data_member_location);
  ADD_ATTR(Attrs.DW_AT_decimal_scale);
  ADD_ATTR(Attrs.DW_AT_decimal_sign);
  ADD_ATTR(Attrs.DW_AT_default_value);
  ADD_ATTR(Attrs.DW_AT_digit_count);
  ADD_ATTR(Attrs.DW_AT_discr);
  ADD_ATTR(Attrs.DW_AT_discr_list);
  ADD_ATTR(Attrs.DW_AT_discr_value);
  ADD_ATTR(Attrs.DW_AT_encoding);
  ADD_ATTR(Attrs.DW_AT_enum_class);
  ADD_ATTR(Attrs.DW_AT_endianity);
  ADD_ATTR(Attrs.DW_AT_explicit);
  ADD_ATTR(Attrs.DW_AT_is_optional);
  ADD_ATTR(Attrs.DW_AT_location);
  ADD_ATTR(Attrs.DW_AT_lower_bound);
  ADD_ATTR(Attrs.DW_AT_mutable);
  ADD_ATTR(Attrs.DW_AT_ordering);
  ADD_ATTR(Attrs.DW_AT_picture_string);
  ADD_ATTR(Attrs.DW_AT_prototyped);
  ADD_ATTR(Attrs.DW_AT_small);
  ADD_ATTR(Attrs.DW_AT_segment);
  ADD_ATTR(Attrs.DW_AT_string_length);
  ADD_ATTR(Attrs.DW_AT_threads_scaled);
  ADD_ATTR(Attrs.DW_AT_upper_bound);
  ADD_ATTR(Attrs.DW_AT_use_location);
  ADD_ATTR(Attrs.DW_AT_use_UTF8);
  ADD_ATTR(Attrs.DW_AT_variable_parameter);
  ADD_ATTR(Attrs.DW_AT_virtuality);
  ADD_ATTR(Attrs.DW_AT_visibility);
  ADD_ATTR(Attrs.DW_AT_vtable_elem_location);
  ADD_ATTR(Attrs.DW_AT_type);

  // FIXME: Add the extended attributes.
}

// Add all of the attributes for \param Die to the hash.
void DIEHash::addAttributes(const DIE &Die) {
  DIEAttrs Attrs = {};
  collectAttributes(Die, Attrs);
  hashAttributes(Attrs, Die.getTag());
}

void DIEHash::hashNestedType(const DIE &Die, StringRef Name) {
  // 7.27 Step 7
  // ... append the letter 'S',
  addULEB128('S');

  // the tag of C,
  addULEB128(Die.getTag());

  // and the name.
  addString(Name);
}

// Compute the hash of a DIE. This is based on the type signature computation
// given in section 7.27 of the DWARF4 standard. It is the md5 hash of a
// flattened description of the DIE.
void DIEHash::computeHash(const DIE &Die) {
  // Append the letter 'D', followed by the DWARF tag of the DIE.
  addULEB128('D');
  addULEB128(Die.getTag());

  // Add each of the attributes of the DIE.
  addAttributes(Die);

  // Then hash each of the children of the DIE.
  for (auto &C : Die.children()) {
    // 7.27 Step 7
    // If C is a nested type entry or a member function entry, ...
    if (isType(C.getTag()) || C.getTag() == dwarf::DW_TAG_subprogram) {
      StringRef Name = getDIEStringAttr(C, dwarf::DW_AT_name);
      // ... and has a DW_AT_name attribute
      if (!Name.empty()) {
        hashNestedType(C, Name);
        continue;
      }
    }
    computeHash(C);
  }

  // Following the last (or if there are no children), append a zero byte.
  Hash.update(makeArrayRef((uint8_t)'\0'));
}

/// This is based on the type signature computation given in section 7.27 of the
/// DWARF4 standard. It is the md5 hash of a flattened description of the DIE
/// with the exception that we are hashing only the context and the name of the
/// type.
uint64_t DIEHash::computeDIEODRSignature(const DIE &Die) {

  // Add the contexts to the hash. We won't be computing the ODR hash for
  // function local types so it's safe to use the generic context hashing
  // algorithm here.
  // FIXME: If we figure out how to account for linkage in some way we could
  // actually do this with a slight modification to the parent hash algorithm.
  if (const DIE *Parent = Die.getParent())
    addParentContext(*Parent);

  // Add the current DIE information.

  // Add the DWARF tag of the DIE.
  addULEB128(Die.getTag());

  // Add the name of the type to the hash.
  addString(getDIEStringAttr(Die, dwarf::DW_AT_name));

  // Now get the result.
  MD5::MD5Result Result;
  Hash.final(Result);

  // ... take the least significant 8 bytes and return those. Our MD5
  // implementation always returns its results in little endian, swap bytes
  // appropriately.
  return support::endian::read64le(Result + 8);
}

/// This is based on the type signature computation given in section 7.27 of the
/// DWARF4 standard. It is an md5 hash of the flattened description of the DIE
/// with the inclusion of the full CU and all top level CU entities.
// TODO: Initialize the type chain at 0 instead of 1 for CU signatures.
uint64_t DIEHash::computeCUSignature(const DIE &Die) {
  Numbering.clear();
  Numbering[&Die] = 1;

  // Hash the DIE.
  computeHash(Die);

  // Now return the result.
  MD5::MD5Result Result;
  Hash.final(Result);

  // ... take the least significant 8 bytes and return those. Our MD5
  // implementation always returns its results in little endian, swap bytes
  // appropriately.
  return support::endian::read64le(Result + 8);
}

/// This is based on the type signature computation given in section 7.27 of the
/// DWARF4 standard. It is an md5 hash of the flattened description of the DIE
/// with the inclusion of additional forms not specifically called out in the
/// standard.
uint64_t DIEHash::computeTypeSignature(const DIE &Die) {
  Numbering.clear();
  Numbering[&Die] = 1;

  if (const DIE *Parent = Die.getParent())
    addParentContext(*Parent);

  // Hash the DIE.
  computeHash(Die);

  // Now return the result.
  MD5::MD5Result Result;
  Hash.final(Result);

  // ... take the least significant 8 bytes and return those. Our MD5
  // implementation always returns its results in little endian, swap bytes
  // appropriately.
  return support::endian::read64le(Result + 8);
}
