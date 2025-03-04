//=== RecordLayoutBuilder.cpp - Helper class for building record layouts ---==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/RecordLayout.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/CrashRecoveryContext.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MathExtras.h"

using namespace clang;

namespace {

/// BaseSubobjectInfo - Represents a single base subobject in a complete class.
/// For a class hierarchy like
///
/// class A { };
/// class B : A { };
/// class C : A, B { };
///
/// The BaseSubobjectInfo graph for C will have three BaseSubobjectInfo
/// instances, one for B and two for A.
///
/// If a base is virtual, it will only have one BaseSubobjectInfo allocated.
struct BaseSubobjectInfo {
  /// Class - The class for this base info.
  const CXXRecordDecl *Class;

  /// IsVirtual - Whether the BaseInfo represents a virtual base or not.
  bool IsVirtual;

  /// Bases - Information about the base subobjects.
  SmallVector<BaseSubobjectInfo*, 4> Bases;

  /// PrimaryVirtualBaseInfo - Holds the base info for the primary virtual base
  /// of this base info (if one exists).
  BaseSubobjectInfo *PrimaryVirtualBaseInfo;

  // FIXME: Document.
  const BaseSubobjectInfo *Derived;
};

/// \brief Externally provided layout. Typically used when the AST source, such
/// as DWARF, lacks all the information that was available at compile time, such
/// as alignment attributes on fields and pragmas in effect.
struct ExternalLayout {
  ExternalLayout() : Size(0), Align(0) {}

  /// \brief Overall record size in bits.
  uint64_t Size;

  /// \brief Overall record alignment in bits.
  uint64_t Align;

  /// \brief Record field offsets in bits.
  llvm::DenseMap<const FieldDecl *, uint64_t> FieldOffsets;

  /// \brief Direct, non-virtual base offsets.
  llvm::DenseMap<const CXXRecordDecl *, CharUnits> BaseOffsets;

  /// \brief Virtual base offsets.
  llvm::DenseMap<const CXXRecordDecl *, CharUnits> VirtualBaseOffsets;

  /// Get the offset of the given field. The external source must provide
  /// entries for all fields in the record.
  uint64_t getExternalFieldOffset(const FieldDecl *FD) {
    assert(FieldOffsets.count(FD) &&
           "Field does not have an external offset");
    return FieldOffsets[FD];
  }

  bool getExternalNVBaseOffset(const CXXRecordDecl *RD, CharUnits &BaseOffset) {
    auto Known = BaseOffsets.find(RD);
    if (Known == BaseOffsets.end())
      return false;
    BaseOffset = Known->second;
    return true;
  }

  bool getExternalVBaseOffset(const CXXRecordDecl *RD, CharUnits &BaseOffset) {
    auto Known = VirtualBaseOffsets.find(RD);
    if (Known == VirtualBaseOffsets.end())
      return false;
    BaseOffset = Known->second;
    return true;
  }
};

/// EmptySubobjectMap - Keeps track of which empty subobjects exist at different
/// offsets while laying out a C++ class.
class EmptySubobjectMap {
  const ASTContext &Context;
  uint64_t CharWidth;
  
  /// Class - The class whose empty entries we're keeping track of.
  const CXXRecordDecl *Class;

  /// EmptyClassOffsets - A map from offsets to empty record decls.
  typedef llvm::TinyPtrVector<const CXXRecordDecl *> ClassVectorTy;
  typedef llvm::DenseMap<CharUnits, ClassVectorTy> EmptyClassOffsetsMapTy;
  EmptyClassOffsetsMapTy EmptyClassOffsets;
  
  /// MaxEmptyClassOffset - The highest offset known to contain an empty
  /// base subobject.
  CharUnits MaxEmptyClassOffset;
  
  /// ComputeEmptySubobjectSizes - Compute the size of the largest base or
  /// member subobject that is empty.
  void ComputeEmptySubobjectSizes();
  
  void AddSubobjectAtOffset(const CXXRecordDecl *RD, CharUnits Offset);
  
  void UpdateEmptyBaseSubobjects(const BaseSubobjectInfo *Info,
                                 CharUnits Offset, bool PlacingEmptyBase);
  
  void UpdateEmptyFieldSubobjects(const CXXRecordDecl *RD, 
                                  const CXXRecordDecl *Class,
                                  CharUnits Offset);
  void UpdateEmptyFieldSubobjects(const FieldDecl *FD, CharUnits Offset);
  
  /// AnyEmptySubobjectsBeyondOffset - Returns whether there are any empty
  /// subobjects beyond the given offset.
  bool AnyEmptySubobjectsBeyondOffset(CharUnits Offset) const {
    return Offset <= MaxEmptyClassOffset;
  }

  CharUnits 
  getFieldOffset(const ASTRecordLayout &Layout, unsigned FieldNo) const {
    uint64_t FieldOffset = Layout.getFieldOffset(FieldNo);
    assert(FieldOffset % CharWidth == 0 && 
           "Field offset not at char boundary!");

    return Context.toCharUnitsFromBits(FieldOffset);
  }

protected:
  bool CanPlaceSubobjectAtOffset(const CXXRecordDecl *RD,
                                 CharUnits Offset) const;

  bool CanPlaceBaseSubobjectAtOffset(const BaseSubobjectInfo *Info,
                                     CharUnits Offset);

  bool CanPlaceFieldSubobjectAtOffset(const CXXRecordDecl *RD, 
                                      const CXXRecordDecl *Class,
                                      CharUnits Offset) const;
  bool CanPlaceFieldSubobjectAtOffset(const FieldDecl *FD,
                                      CharUnits Offset) const;

public:
  /// This holds the size of the largest empty subobject (either a base
  /// or a member). Will be zero if the record being built doesn't contain
  /// any empty classes.
  CharUnits SizeOfLargestEmptySubobject;

  EmptySubobjectMap(const ASTContext &Context, const CXXRecordDecl *Class)
  : Context(Context), CharWidth(Context.getCharWidth()), Class(Class) {
      ComputeEmptySubobjectSizes();
  }

  /// CanPlaceBaseAtOffset - Return whether the given base class can be placed
  /// at the given offset.
  /// Returns false if placing the record will result in two components
  /// (direct or indirect) of the same type having the same offset.
  bool CanPlaceBaseAtOffset(const BaseSubobjectInfo *Info,
                            CharUnits Offset);

  /// CanPlaceFieldAtOffset - Return whether a field can be placed at the given
  /// offset.
  bool CanPlaceFieldAtOffset(const FieldDecl *FD, CharUnits Offset);
};

void EmptySubobjectMap::ComputeEmptySubobjectSizes() {
  // Check the bases.
  for (const CXXBaseSpecifier &Base : Class->bases()) {
    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();

    CharUnits EmptySize;
    const ASTRecordLayout &Layout = Context.getASTRecordLayout(BaseDecl);
    if (BaseDecl->isEmpty()) {
      // If the class decl is empty, get its size.
      EmptySize = Layout.getSize();
    } else {
      // Otherwise, we get the largest empty subobject for the decl.
      EmptySize = Layout.getSizeOfLargestEmptySubobject();
    }

    if (EmptySize > SizeOfLargestEmptySubobject)
      SizeOfLargestEmptySubobject = EmptySize;
  }

  // Check the fields.
  for (const FieldDecl *FD : Class->fields()) {
    const RecordType *RT =
        Context.getBaseElementType(FD->getType())->getAs<RecordType>();

    // We only care about record types.
    if (!RT)
      continue;

    CharUnits EmptySize;
    const CXXRecordDecl *MemberDecl = RT->getAsCXXRecordDecl();
    const ASTRecordLayout &Layout = Context.getASTRecordLayout(MemberDecl);
    if (MemberDecl->isEmpty()) {
      // If the class decl is empty, get its size.
      EmptySize = Layout.getSize();
    } else {
      // Otherwise, we get the largest empty subobject for the decl.
      EmptySize = Layout.getSizeOfLargestEmptySubobject();
    }

    if (EmptySize > SizeOfLargestEmptySubobject)
      SizeOfLargestEmptySubobject = EmptySize;
  }
}

bool
EmptySubobjectMap::CanPlaceSubobjectAtOffset(const CXXRecordDecl *RD, 
                                             CharUnits Offset) const {
  // We only need to check empty bases.
  if (!RD->isEmpty())
    return true;

  EmptyClassOffsetsMapTy::const_iterator I = EmptyClassOffsets.find(Offset);
  if (I == EmptyClassOffsets.end())
    return true;

  const ClassVectorTy &Classes = I->second;
  if (std::find(Classes.begin(), Classes.end(), RD) == Classes.end())
    return true;

  // There is already an empty class of the same type at this offset.
  return false;
}
  
void EmptySubobjectMap::AddSubobjectAtOffset(const CXXRecordDecl *RD, 
                                             CharUnits Offset) {
  // We only care about empty bases.
  if (!RD->isEmpty())
    return;

  // If we have empty structures inside a union, we can assign both
  // the same offset. Just avoid pushing them twice in the list.
  ClassVectorTy &Classes = EmptyClassOffsets[Offset];
  if (std::find(Classes.begin(), Classes.end(), RD) != Classes.end())
    return;
  
  Classes.push_back(RD);
  
  // Update the empty class offset.
  if (Offset > MaxEmptyClassOffset)
    MaxEmptyClassOffset = Offset;
}

bool
EmptySubobjectMap::CanPlaceBaseSubobjectAtOffset(const BaseSubobjectInfo *Info,
                                                 CharUnits Offset) {
  // We don't have to keep looking past the maximum offset that's known to
  // contain an empty class.
  if (!AnyEmptySubobjectsBeyondOffset(Offset))
    return true;

  if (!CanPlaceSubobjectAtOffset(Info->Class, Offset))
    return false;

  // Traverse all non-virtual bases.
  const ASTRecordLayout &Layout = Context.getASTRecordLayout(Info->Class);
  for (const BaseSubobjectInfo *Base : Info->Bases) {
    if (Base->IsVirtual)
      continue;

    CharUnits BaseOffset = Offset + Layout.getBaseClassOffset(Base->Class);

    if (!CanPlaceBaseSubobjectAtOffset(Base, BaseOffset))
      return false;
  }

  if (Info->PrimaryVirtualBaseInfo) {
    BaseSubobjectInfo *PrimaryVirtualBaseInfo = Info->PrimaryVirtualBaseInfo;

    if (Info == PrimaryVirtualBaseInfo->Derived) {
      if (!CanPlaceBaseSubobjectAtOffset(PrimaryVirtualBaseInfo, Offset))
        return false;
    }
  }
  
  // Traverse all member variables.
  unsigned FieldNo = 0;
  for (CXXRecordDecl::field_iterator I = Info->Class->field_begin(), 
       E = Info->Class->field_end(); I != E; ++I, ++FieldNo) {
    if (I->isBitField())
      continue;

    CharUnits FieldOffset = Offset + getFieldOffset(Layout, FieldNo);
    if (!CanPlaceFieldSubobjectAtOffset(*I, FieldOffset))
      return false;
  }

  return true;
}

void EmptySubobjectMap::UpdateEmptyBaseSubobjects(const BaseSubobjectInfo *Info, 
                                                  CharUnits Offset,
                                                  bool PlacingEmptyBase) {
  if (!PlacingEmptyBase && Offset >= SizeOfLargestEmptySubobject) {
    // We know that the only empty subobjects that can conflict with empty
    // subobject of non-empty bases, are empty bases that can be placed at
    // offset zero. Because of this, we only need to keep track of empty base 
    // subobjects with offsets less than the size of the largest empty
    // subobject for our class.    
    return;
  }

  AddSubobjectAtOffset(Info->Class, Offset);

  // Traverse all non-virtual bases.
  const ASTRecordLayout &Layout = Context.getASTRecordLayout(Info->Class);
  for (const BaseSubobjectInfo *Base : Info->Bases) {
    if (Base->IsVirtual)
      continue;

    CharUnits BaseOffset = Offset + Layout.getBaseClassOffset(Base->Class);
    UpdateEmptyBaseSubobjects(Base, BaseOffset, PlacingEmptyBase);
  }

  if (Info->PrimaryVirtualBaseInfo) {
    BaseSubobjectInfo *PrimaryVirtualBaseInfo = Info->PrimaryVirtualBaseInfo;
    
    if (Info == PrimaryVirtualBaseInfo->Derived)
      UpdateEmptyBaseSubobjects(PrimaryVirtualBaseInfo, Offset,
                                PlacingEmptyBase);
  }

  // Traverse all member variables.
  unsigned FieldNo = 0;
  for (CXXRecordDecl::field_iterator I = Info->Class->field_begin(), 
       E = Info->Class->field_end(); I != E; ++I, ++FieldNo) {
    if (I->isBitField())
      continue;

    CharUnits FieldOffset = Offset + getFieldOffset(Layout, FieldNo);
    UpdateEmptyFieldSubobjects(*I, FieldOffset);
  }
}

bool EmptySubobjectMap::CanPlaceBaseAtOffset(const BaseSubobjectInfo *Info,
                                             CharUnits Offset) {
  // If we know this class doesn't have any empty subobjects we don't need to
  // bother checking.
  if (SizeOfLargestEmptySubobject.isZero())
    return true;

  if (!CanPlaceBaseSubobjectAtOffset(Info, Offset))
    return false;

  // We are able to place the base at this offset. Make sure to update the
  // empty base subobject map.
  UpdateEmptyBaseSubobjects(Info, Offset, Info->Class->isEmpty());
  return true;
}

bool
EmptySubobjectMap::CanPlaceFieldSubobjectAtOffset(const CXXRecordDecl *RD, 
                                                  const CXXRecordDecl *Class,
                                                  CharUnits Offset) const {
  // We don't have to keep looking past the maximum offset that's known to
  // contain an empty class.
  if (!AnyEmptySubobjectsBeyondOffset(Offset))
    return true;

  if (!CanPlaceSubobjectAtOffset(RD, Offset))
    return false;
  
  const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);

  // Traverse all non-virtual bases.
  for (const CXXBaseSpecifier &Base : RD->bases()) {
    if (Base.isVirtual())
      continue;

    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();

    CharUnits BaseOffset = Offset + Layout.getBaseClassOffset(BaseDecl);
    if (!CanPlaceFieldSubobjectAtOffset(BaseDecl, Class, BaseOffset))
      return false;
  }

  if (RD == Class) {
    // This is the most derived class, traverse virtual bases as well.
    for (const CXXBaseSpecifier &Base : RD->vbases()) {
      const CXXRecordDecl *VBaseDecl = Base.getType()->getAsCXXRecordDecl();

      CharUnits VBaseOffset = Offset + Layout.getVBaseClassOffset(VBaseDecl);
      if (!CanPlaceFieldSubobjectAtOffset(VBaseDecl, Class, VBaseOffset))
        return false;
    }
  }
    
  // Traverse all member variables.
  unsigned FieldNo = 0;
  for (CXXRecordDecl::field_iterator I = RD->field_begin(), E = RD->field_end();
       I != E; ++I, ++FieldNo) {
    if (I->isBitField())
      continue;

    CharUnits FieldOffset = Offset + getFieldOffset(Layout, FieldNo);
    
    if (!CanPlaceFieldSubobjectAtOffset(*I, FieldOffset))
      return false;
  }

  return true;
}

bool
EmptySubobjectMap::CanPlaceFieldSubobjectAtOffset(const FieldDecl *FD,
                                                  CharUnits Offset) const {
  // We don't have to keep looking past the maximum offset that's known to
  // contain an empty class.
  if (!AnyEmptySubobjectsBeyondOffset(Offset))
    return true;
  
  QualType T = FD->getType();
  if (const CXXRecordDecl *RD = T->getAsCXXRecordDecl())
    return CanPlaceFieldSubobjectAtOffset(RD, RD, Offset);

  // If we have an array type we need to look at every element.
  if (const ConstantArrayType *AT = Context.getAsConstantArrayType(T)) {
    QualType ElemTy = Context.getBaseElementType(AT);
    const RecordType *RT = ElemTy->getAs<RecordType>();
    if (!RT)
      return true;

    const CXXRecordDecl *RD = RT->getAsCXXRecordDecl();
    const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);

    uint64_t NumElements = Context.getConstantArrayElementCount(AT);
    CharUnits ElementOffset = Offset;
    for (uint64_t I = 0; I != NumElements; ++I) {
      // We don't have to keep looking past the maximum offset that's known to
      // contain an empty class.
      if (!AnyEmptySubobjectsBeyondOffset(ElementOffset))
        return true;
      
      if (!CanPlaceFieldSubobjectAtOffset(RD, RD, ElementOffset))
        return false;

      ElementOffset += Layout.getSize();
    }
  }

  return true;
}

bool
EmptySubobjectMap::CanPlaceFieldAtOffset(const FieldDecl *FD, 
                                         CharUnits Offset) {
  if (!CanPlaceFieldSubobjectAtOffset(FD, Offset))
    return false;
  
  // We are able to place the member variable at this offset.
  // Make sure to update the empty base subobject map.
  UpdateEmptyFieldSubobjects(FD, Offset);
  return true;
}

void EmptySubobjectMap::UpdateEmptyFieldSubobjects(const CXXRecordDecl *RD, 
                                                   const CXXRecordDecl *Class,
                                                   CharUnits Offset) {
  // We know that the only empty subobjects that can conflict with empty
  // field subobjects are subobjects of empty bases that can be placed at offset
  // zero. Because of this, we only need to keep track of empty field 
  // subobjects with offsets less than the size of the largest empty
  // subobject for our class.
  if (Offset >= SizeOfLargestEmptySubobject)
    return;

  AddSubobjectAtOffset(RD, Offset);

  const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);

  // Traverse all non-virtual bases.
  for (const CXXBaseSpecifier &Base : RD->bases()) {
    if (Base.isVirtual())
      continue;

    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();

    CharUnits BaseOffset = Offset + Layout.getBaseClassOffset(BaseDecl);
    UpdateEmptyFieldSubobjects(BaseDecl, Class, BaseOffset);
  }

  if (RD == Class) {
    // This is the most derived class, traverse virtual bases as well.
    for (const CXXBaseSpecifier &Base : RD->vbases()) {
      const CXXRecordDecl *VBaseDecl = Base.getType()->getAsCXXRecordDecl();

      CharUnits VBaseOffset = Offset + Layout.getVBaseClassOffset(VBaseDecl);
      UpdateEmptyFieldSubobjects(VBaseDecl, Class, VBaseOffset);
    }
  }
  
  // Traverse all member variables.
  unsigned FieldNo = 0;
  for (CXXRecordDecl::field_iterator I = RD->field_begin(), E = RD->field_end();
       I != E; ++I, ++FieldNo) {
    if (I->isBitField())
      continue;

    CharUnits FieldOffset = Offset + getFieldOffset(Layout, FieldNo);

    UpdateEmptyFieldSubobjects(*I, FieldOffset);
  }
}
  
void EmptySubobjectMap::UpdateEmptyFieldSubobjects(const FieldDecl *FD,
                                                   CharUnits Offset) {
  QualType T = FD->getType();
  if (const CXXRecordDecl *RD = T->getAsCXXRecordDecl()) {
    UpdateEmptyFieldSubobjects(RD, RD, Offset);
    return;
  }

  // If we have an array type we need to update every element.
  if (const ConstantArrayType *AT = Context.getAsConstantArrayType(T)) {
    QualType ElemTy = Context.getBaseElementType(AT);
    const RecordType *RT = ElemTy->getAs<RecordType>();
    if (!RT)
      return;

    const CXXRecordDecl *RD = RT->getAsCXXRecordDecl();
    const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);
    
    uint64_t NumElements = Context.getConstantArrayElementCount(AT);
    CharUnits ElementOffset = Offset;
    
    for (uint64_t I = 0; I != NumElements; ++I) {
      // We know that the only empty subobjects that can conflict with empty
      // field subobjects are subobjects of empty bases that can be placed at 
      // offset zero. Because of this, we only need to keep track of empty field
      // subobjects with offsets less than the size of the largest empty
      // subobject for our class.
      if (ElementOffset >= SizeOfLargestEmptySubobject)
        return;

      UpdateEmptyFieldSubobjects(RD, RD, ElementOffset);
      ElementOffset += Layout.getSize();
    }
  }
}

typedef llvm::SmallPtrSet<const CXXRecordDecl*, 4> ClassSetTy;

class RecordLayoutBuilder {
protected:
  // FIXME: Remove this and make the appropriate fields public.
  friend class clang::ASTContext;

  const ASTContext &Context;

  EmptySubobjectMap *EmptySubobjects;

  /// Size - The current size of the record layout.
  uint64_t Size;

  /// Alignment - The current alignment of the record layout.
  CharUnits Alignment;

  /// \brief The alignment if attribute packed is not used.
  CharUnits UnpackedAlignment;

  SmallVector<uint64_t, 16> FieldOffsets;

  /// \brief Whether the external AST source has provided a layout for this
  /// record.
  unsigned UseExternalLayout : 1;

  /// \brief Whether we need to infer alignment, even when we have an 
  /// externally-provided layout.
  unsigned InferAlignment : 1;
  
  /// Packed - Whether the record is packed or not.
  unsigned Packed : 1;

  unsigned IsUnion : 1;

  unsigned IsMac68kAlign : 1;
  
  unsigned IsMsStruct : 1;

  /// UnfilledBitsInLastUnit - If the last field laid out was a bitfield,
  /// this contains the number of bits in the last unit that can be used for
  /// an adjacent bitfield if necessary.  The unit in question is usually
  /// a byte, but larger units are used if IsMsStruct.
  unsigned char UnfilledBitsInLastUnit;
  /// LastBitfieldTypeSize - If IsMsStruct, represents the size of the type
  /// of the previous field if it was a bitfield.
  unsigned char LastBitfieldTypeSize;

  /// MaxFieldAlignment - The maximum allowed field alignment. This is set by
  /// #pragma pack.
  CharUnits MaxFieldAlignment;

  /// DataSize - The data size of the record being laid out.
  uint64_t DataSize;

  CharUnits NonVirtualSize;
  CharUnits NonVirtualAlignment;

  /// PrimaryBase - the primary base class (if one exists) of the class
  /// we're laying out.
  const CXXRecordDecl *PrimaryBase;

  /// PrimaryBaseIsVirtual - Whether the primary base of the class we're laying
  /// out is virtual.
  bool PrimaryBaseIsVirtual;

  /// HasOwnVFPtr - Whether the class provides its own vtable/vftbl
  /// pointer, as opposed to inheriting one from a primary base class.
  bool HasOwnVFPtr;

  typedef llvm::DenseMap<const CXXRecordDecl *, CharUnits> BaseOffsetsMapTy;

  /// Bases - base classes and their offsets in the record.
  BaseOffsetsMapTy Bases;

  // VBases - virtual base classes and their offsets in the record.
  ASTRecordLayout::VBaseOffsetsMapTy VBases;

  /// IndirectPrimaryBases - Virtual base classes, direct or indirect, that are
  /// primary base classes for some other direct or indirect base class.
  CXXIndirectPrimaryBaseSet IndirectPrimaryBases;

  /// FirstNearlyEmptyVBase - The first nearly empty virtual base class in
  /// inheritance graph order. Used for determining the primary base class.
  const CXXRecordDecl *FirstNearlyEmptyVBase;

  /// VisitedVirtualBases - A set of all the visited virtual bases, used to
  /// avoid visiting virtual bases more than once.
  llvm::SmallPtrSet<const CXXRecordDecl *, 4> VisitedVirtualBases;

  /// Valid if UseExternalLayout is true.
  ExternalLayout External;

  RecordLayoutBuilder(const ASTContext &Context,
                      EmptySubobjectMap *EmptySubobjects)
    : Context(Context), EmptySubobjects(EmptySubobjects), Size(0), 
      Alignment(CharUnits::One()), UnpackedAlignment(CharUnits::One()),
      UseExternalLayout(false), InferAlignment(false),
      Packed(false), IsUnion(false), IsMac68kAlign(false), IsMsStruct(false),
      UnfilledBitsInLastUnit(0), LastBitfieldTypeSize(0),
      MaxFieldAlignment(CharUnits::Zero()), 
      DataSize(0), NonVirtualSize(CharUnits::Zero()), 
      NonVirtualAlignment(CharUnits::One()), 
      PrimaryBase(nullptr), PrimaryBaseIsVirtual(false),
      HasOwnVFPtr(false),
      FirstNearlyEmptyVBase(nullptr) {}

  void Layout(const RecordDecl *D);
  void Layout(const CXXRecordDecl *D);
  void Layout(const ObjCInterfaceDecl *D);

  void LayoutFields(const RecordDecl *D);
  void LayoutField(const FieldDecl *D, bool InsertExtraPadding);
  void LayoutWideBitField(uint64_t FieldSize, uint64_t TypeSize,
                          bool FieldPacked, const FieldDecl *D);
  void LayoutBitField(const FieldDecl *D);

  TargetCXXABI getCXXABI() const {
    return Context.getTargetInfo().getCXXABI();
  }

  /// BaseSubobjectInfoAllocator - Allocator for BaseSubobjectInfo objects.
  llvm::SpecificBumpPtrAllocator<BaseSubobjectInfo> BaseSubobjectInfoAllocator;
  
  typedef llvm::DenseMap<const CXXRecordDecl *, BaseSubobjectInfo *>
    BaseSubobjectInfoMapTy;

  /// VirtualBaseInfo - Map from all the (direct or indirect) virtual bases
  /// of the class we're laying out to their base subobject info.
  BaseSubobjectInfoMapTy VirtualBaseInfo;
  
  /// NonVirtualBaseInfo - Map from all the direct non-virtual bases of the
  /// class we're laying out to their base subobject info.
  BaseSubobjectInfoMapTy NonVirtualBaseInfo;

  /// ComputeBaseSubobjectInfo - Compute the base subobject information for the
  /// bases of the given class.
  void ComputeBaseSubobjectInfo(const CXXRecordDecl *RD);

  /// ComputeBaseSubobjectInfo - Compute the base subobject information for a
  /// single class and all of its base classes.
  BaseSubobjectInfo *ComputeBaseSubobjectInfo(const CXXRecordDecl *RD, 
                                              bool IsVirtual,
                                              BaseSubobjectInfo *Derived);

  /// DeterminePrimaryBase - Determine the primary base of the given class.
  void DeterminePrimaryBase(const CXXRecordDecl *RD);

  void SelectPrimaryVBase(const CXXRecordDecl *RD);

  void EnsureVTablePointerAlignment(CharUnits UnpackedBaseAlign);

  /// LayoutNonVirtualBases - Determines the primary base class (if any) and
  /// lays it out. Will then proceed to lay out all non-virtual base clasess.
  void LayoutNonVirtualBases(const CXXRecordDecl *RD);

  /// LayoutNonVirtualBase - Lays out a single non-virtual base.
  void LayoutNonVirtualBase(const BaseSubobjectInfo *Base);

  void AddPrimaryVirtualBaseOffsets(const BaseSubobjectInfo *Info,
                                    CharUnits Offset);

  /// LayoutVirtualBases - Lays out all the virtual bases.
  void LayoutVirtualBases(const CXXRecordDecl *RD,
                          const CXXRecordDecl *MostDerivedClass);

  /// LayoutVirtualBase - Lays out a single virtual base.
  void LayoutVirtualBase(const BaseSubobjectInfo *Base);

  /// LayoutBase - Will lay out a base and return the offset where it was
  /// placed, in chars.
  CharUnits LayoutBase(const BaseSubobjectInfo *Base);

  /// InitializeLayout - Initialize record layout for the given record decl.
  void InitializeLayout(const Decl *D);

  /// FinishLayout - Finalize record layout. Adjust record size based on the
  /// alignment.
  void FinishLayout(const NamedDecl *D);

  void UpdateAlignment(CharUnits NewAlignment, CharUnits UnpackedNewAlignment);
  void UpdateAlignment(CharUnits NewAlignment) {
    UpdateAlignment(NewAlignment, NewAlignment);
  }

  /// \brief Retrieve the externally-supplied field offset for the given
  /// field.
  ///
  /// \param Field The field whose offset is being queried.
  /// \param ComputedOffset The offset that we've computed for this field.
  uint64_t updateExternalFieldOffset(const FieldDecl *Field, 
                                     uint64_t ComputedOffset);
  
  void CheckFieldPadding(uint64_t Offset, uint64_t UnpaddedOffset,
                          uint64_t UnpackedOffset, unsigned UnpackedAlign,
                          bool isPacked, const FieldDecl *D);

  DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID);

  CharUnits getSize() const { 
    assert(Size % Context.getCharWidth() == 0);
    return Context.toCharUnitsFromBits(Size); 
  }
  uint64_t getSizeInBits() const { return Size; }

  void setSize(CharUnits NewSize) { Size = Context.toBits(NewSize); }
  void setSize(uint64_t NewSize) { Size = NewSize; }

  CharUnits getAligment() const { return Alignment; }

  CharUnits getDataSize() const { 
    assert(DataSize % Context.getCharWidth() == 0);
    return Context.toCharUnitsFromBits(DataSize); 
  }
  uint64_t getDataSizeInBits() const { return DataSize; }

  void setDataSize(CharUnits NewSize) { DataSize = Context.toBits(NewSize); }
  void setDataSize(uint64_t NewSize) { DataSize = NewSize; }

  RecordLayoutBuilder(const RecordLayoutBuilder &) = delete;
  void operator=(const RecordLayoutBuilder &) = delete;
};
} // end anonymous namespace

void
RecordLayoutBuilder::SelectPrimaryVBase(const CXXRecordDecl *RD) {
  for (const auto &I : RD->bases()) {
    assert(!I.getType()->isDependentType() &&
           "Cannot layout class with dependent bases.");

    const CXXRecordDecl *Base = I.getType()->getAsCXXRecordDecl();

    // Check if this is a nearly empty virtual base.
    if (I.isVirtual() && Context.isNearlyEmpty(Base)) {
      // If it's not an indirect primary base, then we've found our primary
      // base.
      if (!IndirectPrimaryBases.count(Base)) {
        PrimaryBase = Base;
        PrimaryBaseIsVirtual = true;
        return;
      }

      // Is this the first nearly empty virtual base?
      if (!FirstNearlyEmptyVBase)
        FirstNearlyEmptyVBase = Base;
    }

    SelectPrimaryVBase(Base);
    if (PrimaryBase)
      return;
  }
}

/// DeterminePrimaryBase - Determine the primary base of the given class.
void RecordLayoutBuilder::DeterminePrimaryBase(const CXXRecordDecl *RD) {
  // If the class isn't dynamic, it won't have a primary base.
  if (!RD->isDynamicClass())
    return;

  // Compute all the primary virtual bases for all of our direct and
  // indirect bases, and record all their primary virtual base classes.
  RD->getIndirectPrimaryBases(IndirectPrimaryBases);

  // If the record has a dynamic base class, attempt to choose a primary base
  // class. It is the first (in direct base class order) non-virtual dynamic
  // base class, if one exists.
  for (const auto &I : RD->bases()) {
    // Ignore virtual bases.
    if (I.isVirtual())
      continue;

    const CXXRecordDecl *Base = I.getType()->getAsCXXRecordDecl();

    if (Base->isDynamicClass()) {
      // We found it.
      PrimaryBase = Base;
      PrimaryBaseIsVirtual = false;
      return;
    }
  }

  // Under the Itanium ABI, if there is no non-virtual primary base class,
  // try to compute the primary virtual base.  The primary virtual base is
  // the first nearly empty virtual base that is not an indirect primary
  // virtual base class, if one exists.
  if (RD->getNumVBases() != 0) {
    SelectPrimaryVBase(RD);
    if (PrimaryBase)
      return;
  }

  // Otherwise, it is the first indirect primary base class, if one exists.
  if (FirstNearlyEmptyVBase) {
    PrimaryBase = FirstNearlyEmptyVBase;
    PrimaryBaseIsVirtual = true;
    return;
  }

  assert(!PrimaryBase && "Should not get here with a primary base!");
}

BaseSubobjectInfo *
RecordLayoutBuilder::ComputeBaseSubobjectInfo(const CXXRecordDecl *RD, 
                                              bool IsVirtual,
                                              BaseSubobjectInfo *Derived) {
  BaseSubobjectInfo *Info;
  
  if (IsVirtual) {
    // Check if we already have info about this virtual base.
    BaseSubobjectInfo *&InfoSlot = VirtualBaseInfo[RD];
    if (InfoSlot) {
      assert(InfoSlot->Class == RD && "Wrong class for virtual base info!");
      return InfoSlot;
    }

    // We don't, create it.
    InfoSlot = new (BaseSubobjectInfoAllocator.Allocate()) BaseSubobjectInfo;
    Info = InfoSlot;
  } else {
    Info = new (BaseSubobjectInfoAllocator.Allocate()) BaseSubobjectInfo;
  }
  
  Info->Class = RD;
  Info->IsVirtual = IsVirtual;
  Info->Derived = nullptr;
  Info->PrimaryVirtualBaseInfo = nullptr;

  const CXXRecordDecl *PrimaryVirtualBase = nullptr;
  BaseSubobjectInfo *PrimaryVirtualBaseInfo = nullptr;

  // Check if this base has a primary virtual base.
  if (RD->getNumVBases()) {
    const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);
    if (Layout.isPrimaryBaseVirtual()) {
      // This base does have a primary virtual base.
      PrimaryVirtualBase = Layout.getPrimaryBase();
      assert(PrimaryVirtualBase && "Didn't have a primary virtual base!");
      
      // Now check if we have base subobject info about this primary base.
      PrimaryVirtualBaseInfo = VirtualBaseInfo.lookup(PrimaryVirtualBase);
      
      if (PrimaryVirtualBaseInfo) {
        if (PrimaryVirtualBaseInfo->Derived) {
          // We did have info about this primary base, and it turns out that it
          // has already been claimed as a primary virtual base for another
          // base.
          PrimaryVirtualBase = nullptr;
        } else {
          // We can claim this base as our primary base.
          Info->PrimaryVirtualBaseInfo = PrimaryVirtualBaseInfo;
          PrimaryVirtualBaseInfo->Derived = Info;
        }
      }
    }
  }

  // Now go through all direct bases.
  for (const auto &I : RD->bases()) {
    bool IsVirtual = I.isVirtual();

    const CXXRecordDecl *BaseDecl = I.getType()->getAsCXXRecordDecl();

    Info->Bases.push_back(ComputeBaseSubobjectInfo(BaseDecl, IsVirtual, Info));
  }
  
  if (PrimaryVirtualBase && !PrimaryVirtualBaseInfo) {
    // Traversing the bases must have created the base info for our primary
    // virtual base.
    PrimaryVirtualBaseInfo = VirtualBaseInfo.lookup(PrimaryVirtualBase);
    assert(PrimaryVirtualBaseInfo &&
           "Did not create a primary virtual base!");
      
    // Claim the primary virtual base as our primary virtual base.
    Info->PrimaryVirtualBaseInfo = PrimaryVirtualBaseInfo;
    PrimaryVirtualBaseInfo->Derived = Info;
  }
  
  return Info;
}

void RecordLayoutBuilder::ComputeBaseSubobjectInfo(const CXXRecordDecl *RD) {
  for (const auto &I : RD->bases()) {
    bool IsVirtual = I.isVirtual();

    const CXXRecordDecl *BaseDecl = I.getType()->getAsCXXRecordDecl();

    // Compute the base subobject info for this base.
    BaseSubobjectInfo *Info = ComputeBaseSubobjectInfo(BaseDecl, IsVirtual,
                                                       nullptr);

    if (IsVirtual) {
      // ComputeBaseInfo has already added this base for us.
      assert(VirtualBaseInfo.count(BaseDecl) &&
             "Did not add virtual base!");
    } else {
      // Add the base info to the map of non-virtual bases.
      assert(!NonVirtualBaseInfo.count(BaseDecl) &&
             "Non-virtual base already exists!");
      NonVirtualBaseInfo.insert(std::make_pair(BaseDecl, Info));
    }
  }
}

void
RecordLayoutBuilder::EnsureVTablePointerAlignment(CharUnits UnpackedBaseAlign) {
  CharUnits BaseAlign = (Packed) ? CharUnits::One() : UnpackedBaseAlign;

  // The maximum field alignment overrides base align.
  if (!MaxFieldAlignment.isZero()) {
    BaseAlign = std::min(BaseAlign, MaxFieldAlignment);
    UnpackedBaseAlign = std::min(UnpackedBaseAlign, MaxFieldAlignment);
  }

  // Round up the current record size to pointer alignment.
  setSize(getSize().RoundUpToAlignment(BaseAlign));
  setDataSize(getSize());

  // Update the alignment.
  UpdateAlignment(BaseAlign, UnpackedBaseAlign);
}

void
RecordLayoutBuilder::LayoutNonVirtualBases(const CXXRecordDecl *RD) {
  // Then, determine the primary base class.
  DeterminePrimaryBase(RD);

  // Compute base subobject info.
  ComputeBaseSubobjectInfo(RD);
  
  // If we have a primary base class, lay it out.
  if (PrimaryBase) {
    if (PrimaryBaseIsVirtual) {
      // If the primary virtual base was a primary virtual base of some other
      // base class we'll have to steal it.
      BaseSubobjectInfo *PrimaryBaseInfo = VirtualBaseInfo.lookup(PrimaryBase);
      PrimaryBaseInfo->Derived = nullptr;

      // We have a virtual primary base, insert it as an indirect primary base.
      IndirectPrimaryBases.insert(PrimaryBase);

      assert(!VisitedVirtualBases.count(PrimaryBase) &&
             "vbase already visited!");
      VisitedVirtualBases.insert(PrimaryBase);

      LayoutVirtualBase(PrimaryBaseInfo);
    } else {
      BaseSubobjectInfo *PrimaryBaseInfo = 
        NonVirtualBaseInfo.lookup(PrimaryBase);
      assert(PrimaryBaseInfo && 
             "Did not find base info for non-virtual primary base!");

      LayoutNonVirtualBase(PrimaryBaseInfo);
    }

  // If this class needs a vtable/vf-table and didn't get one from a
  // primary base, add it in now.
  } else if (RD->isDynamicClass()) {
    assert(DataSize == 0 && "Vtable pointer must be at offset zero!");
    CharUnits PtrWidth = 
      Context.toCharUnitsFromBits(Context.getTargetInfo().getPointerWidth(0));
    CharUnits PtrAlign = 
      Context.toCharUnitsFromBits(Context.getTargetInfo().getPointerAlign(0));
    EnsureVTablePointerAlignment(PtrAlign);
    HasOwnVFPtr = true;
    setSize(getSize() + PtrWidth);
    setDataSize(getSize());
  }

  // Now lay out the non-virtual bases.
  for (const auto &I : RD->bases()) {

    // Ignore virtual bases.
    if (I.isVirtual())
      continue;

    const CXXRecordDecl *BaseDecl = I.getType()->getAsCXXRecordDecl();

    // Skip the primary base, because we've already laid it out.  The
    // !PrimaryBaseIsVirtual check is required because we might have a
    // non-virtual base of the same type as a primary virtual base.
    if (BaseDecl == PrimaryBase && !PrimaryBaseIsVirtual)
      continue;

    // Lay out the base.
    BaseSubobjectInfo *BaseInfo = NonVirtualBaseInfo.lookup(BaseDecl);
    assert(BaseInfo && "Did not find base info for non-virtual base!");

    LayoutNonVirtualBase(BaseInfo);
  }
}

void RecordLayoutBuilder::LayoutNonVirtualBase(const BaseSubobjectInfo *Base) {
  // Layout the base.
  CharUnits Offset = LayoutBase(Base);

  // Add its base class offset.
  assert(!Bases.count(Base->Class) && "base offset already exists!");
  Bases.insert(std::make_pair(Base->Class, Offset));

  AddPrimaryVirtualBaseOffsets(Base, Offset);
}

void
RecordLayoutBuilder::AddPrimaryVirtualBaseOffsets(const BaseSubobjectInfo *Info, 
                                                  CharUnits Offset) {
  // This base isn't interesting, it has no virtual bases.
  if (!Info->Class->getNumVBases())
    return;
  
  // First, check if we have a virtual primary base to add offsets for.
  if (Info->PrimaryVirtualBaseInfo) {
    assert(Info->PrimaryVirtualBaseInfo->IsVirtual && 
           "Primary virtual base is not virtual!");
    if (Info->PrimaryVirtualBaseInfo->Derived == Info) {
      // Add the offset.
      assert(!VBases.count(Info->PrimaryVirtualBaseInfo->Class) && 
             "primary vbase offset already exists!");
      VBases.insert(std::make_pair(Info->PrimaryVirtualBaseInfo->Class,
                                   ASTRecordLayout::VBaseInfo(Offset, false)));

      // Traverse the primary virtual base.
      AddPrimaryVirtualBaseOffsets(Info->PrimaryVirtualBaseInfo, Offset);
    }
  }

  // Now go through all direct non-virtual bases.
  const ASTRecordLayout &Layout = Context.getASTRecordLayout(Info->Class);
  for (const BaseSubobjectInfo *Base : Info->Bases) {
    if (Base->IsVirtual)
      continue;

    CharUnits BaseOffset = Offset + Layout.getBaseClassOffset(Base->Class);
    AddPrimaryVirtualBaseOffsets(Base, BaseOffset);
  }
}

void
RecordLayoutBuilder::LayoutVirtualBases(const CXXRecordDecl *RD,
                                        const CXXRecordDecl *MostDerivedClass) {
  const CXXRecordDecl *PrimaryBase;
  bool PrimaryBaseIsVirtual;

  if (MostDerivedClass == RD) {
    PrimaryBase = this->PrimaryBase;
    PrimaryBaseIsVirtual = this->PrimaryBaseIsVirtual;
  } else {
    const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);
    PrimaryBase = Layout.getPrimaryBase();
    PrimaryBaseIsVirtual = Layout.isPrimaryBaseVirtual();
  }

  for (const CXXBaseSpecifier &Base : RD->bases()) {
    assert(!Base.getType()->isDependentType() &&
           "Cannot layout class with dependent bases.");

    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();

    if (Base.isVirtual()) {
      if (PrimaryBase != BaseDecl || !PrimaryBaseIsVirtual) {
        bool IndirectPrimaryBase = IndirectPrimaryBases.count(BaseDecl);

        // Only lay out the virtual base if it's not an indirect primary base.
        if (!IndirectPrimaryBase) {
          // Only visit virtual bases once.
          if (!VisitedVirtualBases.insert(BaseDecl).second)
            continue;

          const BaseSubobjectInfo *BaseInfo = VirtualBaseInfo.lookup(BaseDecl);
          assert(BaseInfo && "Did not find virtual base info!");
          LayoutVirtualBase(BaseInfo);
        }
      }
    }

    if (!BaseDecl->getNumVBases()) {
      // This base isn't interesting since it doesn't have any virtual bases.
      continue;
    }

    LayoutVirtualBases(BaseDecl, MostDerivedClass);
  }
}

void RecordLayoutBuilder::LayoutVirtualBase(const BaseSubobjectInfo *Base) {
  assert(!Base->Derived && "Trying to lay out a primary virtual base!");
  
  // Layout the base.
  CharUnits Offset = LayoutBase(Base);

  // Add its base class offset.
  assert(!VBases.count(Base->Class) && "vbase offset already exists!");
  VBases.insert(std::make_pair(Base->Class, 
                       ASTRecordLayout::VBaseInfo(Offset, false)));

  AddPrimaryVirtualBaseOffsets(Base, Offset);
}

CharUnits RecordLayoutBuilder::LayoutBase(const BaseSubobjectInfo *Base) {
  const ASTRecordLayout &Layout = Context.getASTRecordLayout(Base->Class);

  
  CharUnits Offset;
  
  // Query the external layout to see if it provides an offset.
  bool HasExternalLayout = false;
  if (UseExternalLayout) {
    llvm::DenseMap<const CXXRecordDecl *, CharUnits>::iterator Known;
    if (Base->IsVirtual)
      HasExternalLayout = External.getExternalNVBaseOffset(Base->Class, Offset);
    else
      HasExternalLayout = External.getExternalVBaseOffset(Base->Class, Offset);
  }
  
  CharUnits UnpackedBaseAlign = Layout.getNonVirtualAlignment();
  CharUnits BaseAlign = (Packed) ? CharUnits::One() : UnpackedBaseAlign;
 
  // If we have an empty base class, try to place it at offset 0.
  if (Base->Class->isEmpty() &&
      (!HasExternalLayout || Offset == CharUnits::Zero()) &&
      EmptySubobjects->CanPlaceBaseAtOffset(Base, CharUnits::Zero())) {
    setSize(std::max(getSize(), Layout.getSize()));
    UpdateAlignment(BaseAlign, UnpackedBaseAlign);

    return CharUnits::Zero();
  }

  // The maximum field alignment overrides base align.
  if (!MaxFieldAlignment.isZero()) {
    BaseAlign = std::min(BaseAlign, MaxFieldAlignment);
    UnpackedBaseAlign = std::min(UnpackedBaseAlign, MaxFieldAlignment);
  }

  if (!HasExternalLayout) {
    // Round up the current record size to the base's alignment boundary.
    Offset = getDataSize().RoundUpToAlignment(BaseAlign);

    // Try to place the base.
    while (!EmptySubobjects->CanPlaceBaseAtOffset(Base, Offset))
      Offset += BaseAlign;
  } else {
    bool Allowed = EmptySubobjects->CanPlaceBaseAtOffset(Base, Offset);
    (void)Allowed;
    assert(Allowed && "Base subobject externally placed at overlapping offset");

    if (InferAlignment && Offset < getDataSize().RoundUpToAlignment(BaseAlign)){
      // The externally-supplied base offset is before the base offset we
      // computed. Assume that the structure is packed.
      Alignment = CharUnits::One();
      InferAlignment = false;
    }
  }
  
  if (!Base->Class->isEmpty()) {
    // Update the data size.
    setDataSize(Offset + Layout.getNonVirtualSize());

    setSize(std::max(getSize(), getDataSize()));
  } else
    setSize(std::max(getSize(), Offset + Layout.getSize()));

  // Remember max struct/class alignment.
  UpdateAlignment(BaseAlign, UnpackedBaseAlign);

  return Offset;
}

void RecordLayoutBuilder::InitializeLayout(const Decl *D) {
  if (const RecordDecl *RD = dyn_cast<RecordDecl>(D)) {
    IsUnion = RD->isUnion();
    IsMsStruct = RD->isMsStruct(Context);
  }

  Packed = D->hasAttr<PackedAttr>();  

  // Honor the default struct packing maximum alignment flag.
  if (unsigned DefaultMaxFieldAlignment = Context.getLangOpts().PackStruct) {
    MaxFieldAlignment = CharUnits::fromQuantity(DefaultMaxFieldAlignment);
  }

  // mac68k alignment supersedes maximum field alignment and attribute aligned,
  // and forces all structures to have 2-byte alignment. The IBM docs on it
  // allude to additional (more complicated) semantics, especially with regard
  // to bit-fields, but gcc appears not to follow that.
  if (D->hasAttr<AlignMac68kAttr>()) {
    IsMac68kAlign = true;
    MaxFieldAlignment = CharUnits::fromQuantity(2);
    Alignment = CharUnits::fromQuantity(2);
  } else {
    if (const MaxFieldAlignmentAttr *MFAA = D->getAttr<MaxFieldAlignmentAttr>())
      MaxFieldAlignment = Context.toCharUnitsFromBits(MFAA->getAlignment());

    if (unsigned MaxAlign = D->getMaxAlignment())
      UpdateAlignment(Context.toCharUnitsFromBits(MaxAlign));
  }
  
  // If there is an external AST source, ask it for the various offsets.
  if (const RecordDecl *RD = dyn_cast<RecordDecl>(D))
    if (ExternalASTSource *Source = Context.getExternalSource()) {
      UseExternalLayout = Source->layoutRecordType(
          RD, External.Size, External.Align, External.FieldOffsets,
          External.BaseOffsets, External.VirtualBaseOffsets);

      // Update based on external alignment.
      if (UseExternalLayout) {
        if (External.Align > 0) {
          Alignment = Context.toCharUnitsFromBits(External.Align);
        } else {
          // The external source didn't have alignment information; infer it.
          InferAlignment = true;
        }
      }
    }
}

void RecordLayoutBuilder::Layout(const RecordDecl *D) {
  InitializeLayout(D);
  LayoutFields(D);

  // Finally, round the size of the total struct up to the alignment of the
  // struct itself.
  FinishLayout(D);
}

void RecordLayoutBuilder::Layout(const CXXRecordDecl *RD) {
  InitializeLayout(RD);

  // Lay out the vtable and the non-virtual bases.
  LayoutNonVirtualBases(RD);

  LayoutFields(RD);

  NonVirtualSize = Context.toCharUnitsFromBits(
        llvm::RoundUpToAlignment(getSizeInBits(), 
                                 Context.getTargetInfo().getCharAlign()));
  NonVirtualAlignment = Alignment;

  // Lay out the virtual bases and add the primary virtual base offsets.
  LayoutVirtualBases(RD, RD);

  // Finally, round the size of the total struct up to the alignment
  // of the struct itself.
  FinishLayout(RD);

#ifndef NDEBUG
  // Check that we have base offsets for all bases.
  for (const CXXBaseSpecifier &Base : RD->bases()) {
    if (Base.isVirtual())
      continue;

    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();

    assert(Bases.count(BaseDecl) && "Did not find base offset!");
  }

  // And all virtual bases.
  for (const CXXBaseSpecifier &Base : RD->vbases()) {
    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();

    assert(VBases.count(BaseDecl) && "Did not find base offset!");
  }
#endif
}

void RecordLayoutBuilder::Layout(const ObjCInterfaceDecl *D) {
  if (ObjCInterfaceDecl *SD = D->getSuperClass()) {
    const ASTRecordLayout &SL = Context.getASTObjCInterfaceLayout(SD);

    UpdateAlignment(SL.getAlignment());

    // We start laying out ivars not at the end of the superclass
    // structure, but at the next byte following the last field.
    setSize(SL.getDataSize());
    setDataSize(getSize());
  }

  InitializeLayout(D);
  // Layout each ivar sequentially.
  for (const ObjCIvarDecl *IVD = D->all_declared_ivar_begin(); IVD;
       IVD = IVD->getNextIvar())
    LayoutField(IVD, false);

  // Finally, round the size of the total struct up to the alignment of the
  // struct itself.
  FinishLayout(D);
}

void RecordLayoutBuilder::LayoutFields(const RecordDecl *D) {
  // Layout each field, for now, just sequentially, respecting alignment.  In
  // the future, this will need to be tweakable by targets.
  bool InsertExtraPadding = D->mayInsertExtraPadding(/*EmitRemark=*/true);
  bool HasFlexibleArrayMember = D->hasFlexibleArrayMember();
  for (auto I = D->field_begin(), End = D->field_end(); I != End; ++I) {
    auto Next(I);
    ++Next;
    LayoutField(*I,
                InsertExtraPadding && (Next != End || !HasFlexibleArrayMember));
  }
}

// Rounds the specified size to have it a multiple of the char size.
static uint64_t
roundUpSizeToCharAlignment(uint64_t Size,
                           const ASTContext &Context) {
  uint64_t CharAlignment = Context.getTargetInfo().getCharAlign();
  return llvm::RoundUpToAlignment(Size, CharAlignment);
}

void RecordLayoutBuilder::LayoutWideBitField(uint64_t FieldSize,
                                             uint64_t TypeSize,
                                             bool FieldPacked,
                                             const FieldDecl *D) {
  assert(Context.getLangOpts().CPlusPlus &&
         "Can only have wide bit-fields in C++!");

  // Itanium C++ ABI 2.4:
  //   If sizeof(T)*8 < n, let T' be the largest integral POD type with
  //   sizeof(T')*8 <= n.

  QualType IntegralPODTypes[] = {
    Context.UnsignedCharTy, Context.UnsignedShortTy, Context.UnsignedIntTy,
    Context.UnsignedLongTy, Context.UnsignedLongLongTy
  };

  QualType Type;
  for (const QualType &QT : IntegralPODTypes) {
    uint64_t Size = Context.getTypeSize(QT);

    if (Size > FieldSize)
      break;

    Type = QT;
  }
  assert(!Type.isNull() && "Did not find a type!");

  CharUnits TypeAlign = Context.getTypeAlignInChars(Type);

  // We're not going to use any of the unfilled bits in the last byte.
  UnfilledBitsInLastUnit = 0;
  LastBitfieldTypeSize = 0;

  uint64_t FieldOffset;
  uint64_t UnpaddedFieldOffset = getDataSizeInBits() - UnfilledBitsInLastUnit;

  if (IsUnion) {
    uint64_t RoundedFieldSize = roundUpSizeToCharAlignment(FieldSize,
                                                           Context);
    setDataSize(std::max(getDataSizeInBits(), RoundedFieldSize));
    FieldOffset = 0;
  } else {
    // The bitfield is allocated starting at the next offset aligned 
    // appropriately for T', with length n bits.
    FieldOffset = llvm::RoundUpToAlignment(getDataSizeInBits(), 
                                           Context.toBits(TypeAlign));

    uint64_t NewSizeInBits = FieldOffset + FieldSize;

    setDataSize(llvm::RoundUpToAlignment(NewSizeInBits, 
                                         Context.getTargetInfo().getCharAlign()));
    UnfilledBitsInLastUnit = getDataSizeInBits() - NewSizeInBits;
  }

  // Place this field at the current location.
  FieldOffsets.push_back(FieldOffset);

  CheckFieldPadding(FieldOffset, UnpaddedFieldOffset, FieldOffset,
                    Context.toBits(TypeAlign), FieldPacked, D);

  // Update the size.
  setSize(std::max(getSizeInBits(), getDataSizeInBits()));

  // Remember max struct/class alignment.
  UpdateAlignment(TypeAlign);
}

void RecordLayoutBuilder::LayoutBitField(const FieldDecl *D) {
  bool FieldPacked = Packed || D->hasAttr<PackedAttr>();
  uint64_t FieldSize = D->getBitWidthValue(Context);
  TypeInfo FieldInfo = Context.getTypeInfo(D->getType());
  uint64_t TypeSize = FieldInfo.Width;
  unsigned FieldAlign = FieldInfo.Align;

  // UnfilledBitsInLastUnit is the difference between the end of the
  // last allocated bitfield (i.e. the first bit offset available for
  // bitfields) and the end of the current data size in bits (i.e. the
  // first bit offset available for non-bitfields).  The current data
  // size in bits is always a multiple of the char size; additionally,
  // for ms_struct records it's also a multiple of the
  // LastBitfieldTypeSize (if set).

  // The struct-layout algorithm is dictated by the platform ABI,
  // which in principle could use almost any rules it likes.  In
  // practice, UNIXy targets tend to inherit the algorithm described
  // in the System V generic ABI.  The basic bitfield layout rule in
  // System V is to place bitfields at the next available bit offset
  // where the entire bitfield would fit in an aligned storage unit of
  // the declared type; it's okay if an earlier or later non-bitfield
  // is allocated in the same storage unit.  However, some targets
  // (those that !useBitFieldTypeAlignment(), e.g. ARM APCS) don't
  // require this storage unit to be aligned, and therefore always put
  // the bitfield at the next available bit offset.

  // ms_struct basically requests a complete replacement of the
  // platform ABI's struct-layout algorithm, with the high-level goal
  // of duplicating MSVC's layout.  For non-bitfields, this follows
  // the standard algorithm.  The basic bitfield layout rule is to
  // allocate an entire unit of the bitfield's declared type
  // (e.g. 'unsigned long'), then parcel it up among successive
  // bitfields whose declared types have the same size, making a new
  // unit as soon as the last can no longer store the whole value.
  // Since it completely replaces the platform ABI's algorithm,
  // settings like !useBitFieldTypeAlignment() do not apply.

  // A zero-width bitfield forces the use of a new storage unit for
  // later bitfields.  In general, this occurs by rounding up the
  // current size of the struct as if the algorithm were about to
  // place a non-bitfield of the field's formal type.  Usually this
  // does not change the alignment of the struct itself, but it does
  // on some targets (those that useZeroLengthBitfieldAlignment(),
  // e.g. ARM).  In ms_struct layout, zero-width bitfields are
  // ignored unless they follow a non-zero-width bitfield.

  // A field alignment restriction (e.g. from #pragma pack) or
  // specification (e.g. from __attribute__((aligned))) changes the
  // formal alignment of the field.  For System V, this alters the
  // required alignment of the notional storage unit that must contain
  // the bitfield.  For ms_struct, this only affects the placement of
  // new storage units.  In both cases, the effect of #pragma pack is
  // ignored on zero-width bitfields.

  // On System V, a packed field (e.g. from #pragma pack or
  // __attribute__((packed))) always uses the next available bit
  // offset.

  // In an ms_struct struct, the alignment of a fundamental type is
  // always equal to its size.  This is necessary in order to mimic
  // the i386 alignment rules on targets which might not fully align
  // all types (e.g. Darwin PPC32, where alignof(long long) == 4).

  // First, some simple bookkeeping to perform for ms_struct structs.
  if (IsMsStruct) {
    // The field alignment for integer types is always the size.
    FieldAlign = TypeSize;

    // If the previous field was not a bitfield, or was a bitfield
    // with a different storage unit size, we're done with that
    // storage unit.
    if (LastBitfieldTypeSize != TypeSize) {
      // Also, ignore zero-length bitfields after non-bitfields.
      if (!LastBitfieldTypeSize && !FieldSize)
        FieldAlign = 1;

      UnfilledBitsInLastUnit = 0;
      LastBitfieldTypeSize = 0;
    }
  }

  // If the field is wider than its declared type, it follows
  // different rules in all cases.
  if (FieldSize > TypeSize) {
    LayoutWideBitField(FieldSize, TypeSize, FieldPacked, D);
    return;
  }

  // Compute the next available bit offset.
  uint64_t FieldOffset =
    IsUnion ? 0 : (getDataSizeInBits() - UnfilledBitsInLastUnit);

  // Handle targets that don't honor bitfield type alignment.
  if (!IsMsStruct && !Context.getTargetInfo().useBitFieldTypeAlignment()) {
    // Some such targets do honor it on zero-width bitfields.
    if (FieldSize == 0 &&
        Context.getTargetInfo().useZeroLengthBitfieldAlignment()) {
      // The alignment to round up to is the max of the field's natural
      // alignment and a target-specific fixed value (sometimes zero).
      unsigned ZeroLengthBitfieldBoundary =
        Context.getTargetInfo().getZeroLengthBitfieldBoundary();
      FieldAlign = std::max(FieldAlign, ZeroLengthBitfieldBoundary);

    // If that doesn't apply, just ignore the field alignment.
    } else {
      FieldAlign = 1;
    }
  }

  // Remember the alignment we would have used if the field were not packed.
  unsigned UnpackedFieldAlign = FieldAlign;

  // Ignore the field alignment if the field is packed unless it has zero-size.
  if (!IsMsStruct && FieldPacked && FieldSize != 0)
    FieldAlign = 1;

  // But, if there's an 'aligned' attribute on the field, honor that.
  if (unsigned ExplicitFieldAlign = D->getMaxAlignment()) {
    FieldAlign = std::max(FieldAlign, ExplicitFieldAlign);
    UnpackedFieldAlign = std::max(UnpackedFieldAlign, ExplicitFieldAlign);
  }

  // But, if there's a #pragma pack in play, that takes precedent over
  // even the 'aligned' attribute, for non-zero-width bitfields.
  if (!MaxFieldAlignment.isZero() && FieldSize) {
    unsigned MaxFieldAlignmentInBits = Context.toBits(MaxFieldAlignment);
    FieldAlign = std::min(FieldAlign, MaxFieldAlignmentInBits);
    UnpackedFieldAlign = std::min(UnpackedFieldAlign, MaxFieldAlignmentInBits);
  }

  // For purposes of diagnostics, we're going to simultaneously
  // compute the field offsets that we would have used if we weren't
  // adding any alignment padding or if the field weren't packed.
  uint64_t UnpaddedFieldOffset = FieldOffset;
  uint64_t UnpackedFieldOffset = FieldOffset;

  // Check if we need to add padding to fit the bitfield within an
  // allocation unit with the right size and alignment.  The rules are
  // somewhat different here for ms_struct structs.
  if (IsMsStruct) {
    // If it's not a zero-width bitfield, and we can fit the bitfield
    // into the active storage unit (and we haven't already decided to
    // start a new storage unit), just do so, regardless of any other
    // other consideration.  Otherwise, round up to the right alignment.
    if (FieldSize == 0 || FieldSize > UnfilledBitsInLastUnit) {
      FieldOffset = llvm::RoundUpToAlignment(FieldOffset, FieldAlign);
      UnpackedFieldOffset = llvm::RoundUpToAlignment(UnpackedFieldOffset,
                                                     UnpackedFieldAlign);
      UnfilledBitsInLastUnit = 0;
    }

  } else {
    // #pragma pack, with any value, suppresses the insertion of padding.
    bool AllowPadding = MaxFieldAlignment.isZero();

    // Compute the real offset.
    if (FieldSize == 0 || 
        (AllowPadding &&
         (FieldOffset & (FieldAlign-1)) + FieldSize > TypeSize)) {
      FieldOffset = llvm::RoundUpToAlignment(FieldOffset, FieldAlign);
    }

    // Repeat the computation for diagnostic purposes.
    if (FieldSize == 0 ||
        (AllowPadding &&
         (UnpackedFieldOffset & (UnpackedFieldAlign-1)) + FieldSize > TypeSize))
      UnpackedFieldOffset = llvm::RoundUpToAlignment(UnpackedFieldOffset,
                                                     UnpackedFieldAlign);
  }

  // If we're using external layout, give the external layout a chance
  // to override this information.
  if (UseExternalLayout)
    FieldOffset = updateExternalFieldOffset(D, FieldOffset);

  // Okay, place the bitfield at the calculated offset.
  FieldOffsets.push_back(FieldOffset);

  // Bookkeeping:

  // Anonymous members don't affect the overall record alignment,
  // except on targets where they do.
  if (!IsMsStruct &&
      !Context.getTargetInfo().useZeroLengthBitfieldAlignment() &&
      !D->getIdentifier())
    FieldAlign = UnpackedFieldAlign = 1;

  // Diagnose differences in layout due to padding or packing.
  if (!UseExternalLayout)
    CheckFieldPadding(FieldOffset, UnpaddedFieldOffset, UnpackedFieldOffset,
                      UnpackedFieldAlign, FieldPacked, D);

  // Update DataSize to include the last byte containing (part of) the bitfield.

  // For unions, this is just a max operation, as usual.
  if (IsUnion) {
    uint64_t RoundedFieldSize = roundUpSizeToCharAlignment(FieldSize,
                                                           Context);
    setDataSize(std::max(getDataSizeInBits(), RoundedFieldSize));
  // For non-zero-width bitfields in ms_struct structs, allocate a new
  // storage unit if necessary.
  } else if (IsMsStruct && FieldSize) {
    // We should have cleared UnfilledBitsInLastUnit in every case
    // where we changed storage units.
    if (!UnfilledBitsInLastUnit) {
      setDataSize(FieldOffset + TypeSize);
      UnfilledBitsInLastUnit = TypeSize;
    }
    UnfilledBitsInLastUnit -= FieldSize;
    LastBitfieldTypeSize = TypeSize;

  // Otherwise, bump the data size up to include the bitfield,
  // including padding up to char alignment, and then remember how
  // bits we didn't use.
  } else {
    uint64_t NewSizeInBits = FieldOffset + FieldSize;
    uint64_t CharAlignment = Context.getTargetInfo().getCharAlign();
    setDataSize(llvm::RoundUpToAlignment(NewSizeInBits, CharAlignment));
    UnfilledBitsInLastUnit = getDataSizeInBits() - NewSizeInBits;

    // The only time we can get here for an ms_struct is if this is a
    // zero-width bitfield, which doesn't count as anything for the
    // purposes of unfilled bits.
    LastBitfieldTypeSize = 0;
  }

  // Update the size.
  setSize(std::max(getSizeInBits(), getDataSizeInBits()));

  // Remember max struct/class alignment.
  UpdateAlignment(Context.toCharUnitsFromBits(FieldAlign), 
                  Context.toCharUnitsFromBits(UnpackedFieldAlign));
}

void RecordLayoutBuilder::LayoutField(const FieldDecl *D,
                                      bool InsertExtraPadding) {
  if (D->isBitField()) {
    LayoutBitField(D);
    return;
  }

  uint64_t UnpaddedFieldOffset = getDataSizeInBits() - UnfilledBitsInLastUnit;

  // Reset the unfilled bits.
  UnfilledBitsInLastUnit = 0;
  LastBitfieldTypeSize = 0;

  bool FieldPacked = Packed || D->hasAttr<PackedAttr>();
  CharUnits FieldOffset = 
    IsUnion ? CharUnits::Zero() : getDataSize();
  CharUnits FieldSize;
  CharUnits FieldAlign;

  if (D->getType()->isIncompleteArrayType()) {
    // This is a flexible array member; we can't directly
    // query getTypeInfo about these, so we figure it out here.
    // Flexible array members don't have any size, but they
    // have to be aligned appropriately for their element type.
    FieldSize = CharUnits::Zero();
    const ArrayType* ATy = Context.getAsArrayType(D->getType());
    FieldAlign = Context.getTypeAlignInChars(ATy->getElementType());
  } else if (const ReferenceType *RT = D->getType()->getAs<ReferenceType>()) {
    unsigned AS = RT->getPointeeType().getAddressSpace();
    FieldSize = 
      Context.toCharUnitsFromBits(Context.getTargetInfo().getPointerWidth(AS));
    FieldAlign = 
      Context.toCharUnitsFromBits(Context.getTargetInfo().getPointerAlign(AS));
  } else {
    std::pair<CharUnits, CharUnits> FieldInfo = 
      Context.getTypeInfoInChars(D->getType());
    FieldSize = FieldInfo.first;
    FieldAlign = FieldInfo.second;

    if (IsMsStruct) {
      // If MS bitfield layout is required, figure out what type is being
      // laid out and align the field to the width of that type.
      
      // Resolve all typedefs down to their base type and round up the field
      // alignment if necessary.
      QualType T = Context.getBaseElementType(D->getType());
      if (const BuiltinType *BTy = T->getAs<BuiltinType>()) {
        CharUnits TypeSize = Context.getTypeSizeInChars(BTy);
        if (TypeSize > FieldAlign)
          FieldAlign = TypeSize;
      }
    }
  }

  // The align if the field is not packed. This is to check if the attribute
  // was unnecessary (-Wpacked).
  CharUnits UnpackedFieldAlign = FieldAlign;
  CharUnits UnpackedFieldOffset = FieldOffset;

  if (FieldPacked)
    FieldAlign = CharUnits::One();
  CharUnits MaxAlignmentInChars = 
    Context.toCharUnitsFromBits(D->getMaxAlignment());
  FieldAlign = std::max(FieldAlign, MaxAlignmentInChars);
  UnpackedFieldAlign = std::max(UnpackedFieldAlign, MaxAlignmentInChars);

  // The maximum field alignment overrides the aligned attribute.
  if (!MaxFieldAlignment.isZero()) {
    FieldAlign = std::min(FieldAlign, MaxFieldAlignment);
    UnpackedFieldAlign = std::min(UnpackedFieldAlign, MaxFieldAlignment);
  }

  // Round up the current record size to the field's alignment boundary.
  FieldOffset = FieldOffset.RoundUpToAlignment(FieldAlign);
  UnpackedFieldOffset = 
    UnpackedFieldOffset.RoundUpToAlignment(UnpackedFieldAlign);

  if (UseExternalLayout) {
    FieldOffset = Context.toCharUnitsFromBits(
                    updateExternalFieldOffset(D, Context.toBits(FieldOffset)));
    
    if (!IsUnion && EmptySubobjects) {
      // Record the fact that we're placing a field at this offset.
      bool Allowed = EmptySubobjects->CanPlaceFieldAtOffset(D, FieldOffset);
      (void)Allowed;
      assert(Allowed && "Externally-placed field cannot be placed here");      
    }
  } else {
    if (!IsUnion && EmptySubobjects) {
      // Check if we can place the field at this offset.
      while (!EmptySubobjects->CanPlaceFieldAtOffset(D, FieldOffset)) {
        // We couldn't place the field at the offset. Try again at a new offset.
        FieldOffset += FieldAlign;
      }
    }
  }
  
  // Place this field at the current location.
  FieldOffsets.push_back(Context.toBits(FieldOffset));

  if (!UseExternalLayout)
    CheckFieldPadding(Context.toBits(FieldOffset), UnpaddedFieldOffset, 
                      Context.toBits(UnpackedFieldOffset),
                      Context.toBits(UnpackedFieldAlign), FieldPacked, D);

  if (InsertExtraPadding) {
    CharUnits ASanAlignment = CharUnits::fromQuantity(8);
    CharUnits ExtraSizeForAsan = ASanAlignment;
    if (FieldSize % ASanAlignment)
      ExtraSizeForAsan +=
          ASanAlignment - CharUnits::fromQuantity(FieldSize % ASanAlignment);
    FieldSize += ExtraSizeForAsan;
  }

  // Reserve space for this field.
  uint64_t FieldSizeInBits = Context.toBits(FieldSize);
  if (IsUnion)
    setDataSize(std::max(getDataSizeInBits(), FieldSizeInBits));
  else
    setDataSize(FieldOffset + FieldSize);

  // Update the size.
  setSize(std::max(getSizeInBits(), getDataSizeInBits()));

  // Remember max struct/class alignment.
  UpdateAlignment(FieldAlign, UnpackedFieldAlign);
}

void RecordLayoutBuilder::FinishLayout(const NamedDecl *D) {
  // In C++, records cannot be of size 0.
  if (Context.getLangOpts().CPlusPlus && getSizeInBits() == 0) {
    if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(D)) {
      // Compatibility with gcc requires a class (pod or non-pod)
      // which is not empty but of size 0; such as having fields of
      // array of zero-length, remains of Size 0
      if (RD->isEmpty())
        setSize(CharUnits::One());
    }
    else
      setSize(CharUnits::One());
  }

  // Finally, round the size of the record up to the alignment of the
  // record itself.
  uint64_t UnpaddedSize = getSizeInBits() - UnfilledBitsInLastUnit;
  uint64_t UnpackedSizeInBits =
  llvm::RoundUpToAlignment(getSizeInBits(),
                           Context.toBits(UnpackedAlignment));
  CharUnits UnpackedSize = Context.toCharUnitsFromBits(UnpackedSizeInBits);
  uint64_t RoundedSize
    = llvm::RoundUpToAlignment(getSizeInBits(), Context.toBits(Alignment));

  if (UseExternalLayout) {
    // If we're inferring alignment, and the external size is smaller than
    // our size after we've rounded up to alignment, conservatively set the
    // alignment to 1.
    if (InferAlignment && External.Size < RoundedSize) {
      Alignment = CharUnits::One();
      InferAlignment = false;
    }
    setSize(External.Size);
    return;
  }

  // Set the size to the final size.
  setSize(RoundedSize);

  unsigned CharBitNum = Context.getTargetInfo().getCharWidth();
  if (const RecordDecl *RD = dyn_cast<RecordDecl>(D)) {
    // Warn if padding was introduced to the struct/class/union.
    if (getSizeInBits() > UnpaddedSize) {
      unsigned PadSize = getSizeInBits() - UnpaddedSize;
      bool InBits = true;
      if (PadSize % CharBitNum == 0) {
        PadSize = PadSize / CharBitNum;
        InBits = false;
      }
      Diag(RD->getLocation(), diag::warn_padded_struct_size)
          << Context.getTypeDeclType(RD)
          << PadSize
          << (InBits ? 1 : 0) /*(byte|bit)*/ << (PadSize > 1); // plural or not
    }

    // Warn if we packed it unnecessarily. If the alignment is 1 byte don't
    // bother since there won't be alignment issues.
    if (Packed && UnpackedAlignment > CharUnits::One() && 
        getSize() == UnpackedSize)
      Diag(D->getLocation(), diag::warn_unnecessary_packed)
          << Context.getTypeDeclType(RD);
  }
}

void RecordLayoutBuilder::UpdateAlignment(CharUnits NewAlignment,
                                          CharUnits UnpackedNewAlignment) {
  // The alignment is not modified when using 'mac68k' alignment or when
  // we have an externally-supplied layout that also provides overall alignment.
  if (IsMac68kAlign || (UseExternalLayout && !InferAlignment))
    return;

  if (NewAlignment > Alignment) {
    assert(llvm::isPowerOf2_64(NewAlignment.getQuantity()) &&
           "Alignment not a power of 2");
    Alignment = NewAlignment;
  }

  if (UnpackedNewAlignment > UnpackedAlignment) {
    assert(llvm::isPowerOf2_64(UnpackedNewAlignment.getQuantity()) &&
           "Alignment not a power of 2");
    UnpackedAlignment = UnpackedNewAlignment;
  }
}

uint64_t
RecordLayoutBuilder::updateExternalFieldOffset(const FieldDecl *Field, 
                                               uint64_t ComputedOffset) {
  uint64_t ExternalFieldOffset = External.getExternalFieldOffset(Field);

  if (InferAlignment && ExternalFieldOffset < ComputedOffset) {
    // The externally-supplied field offset is before the field offset we
    // computed. Assume that the structure is packed.
    Alignment = CharUnits::One();
    InferAlignment = false;
  }
  
  // Use the externally-supplied field offset.
  return ExternalFieldOffset;
}

/// \brief Get diagnostic %select index for tag kind for
/// field padding diagnostic message.
/// WARNING: Indexes apply to particular diagnostics only!
///
/// \returns diagnostic %select index.
static unsigned getPaddingDiagFromTagKind(TagTypeKind Tag) {
  switch (Tag) {
  case TTK_Struct: return 0;
  case TTK_Interface: return 1;
  case TTK_Class: return 2;
  default: llvm_unreachable("Invalid tag kind for field padding diagnostic!");
  }
}

void RecordLayoutBuilder::CheckFieldPadding(uint64_t Offset,
                                            uint64_t UnpaddedOffset,
                                            uint64_t UnpackedOffset,
                                            unsigned UnpackedAlign,
                                            bool isPacked,
                                            const FieldDecl *D) {
  // We let objc ivars without warning, objc interfaces generally are not used
  // for padding tricks.
  if (isa<ObjCIvarDecl>(D))
    return;

  // Don't warn about structs created without a SourceLocation.  This can
  // be done by clients of the AST, such as codegen.
  if (D->getLocation().isInvalid())
    return;
  
  unsigned CharBitNum = Context.getTargetInfo().getCharWidth();

  // Warn if padding was introduced to the struct/class.
  if (!IsUnion && Offset > UnpaddedOffset) {
    unsigned PadSize = Offset - UnpaddedOffset;
    bool InBits = true;
    if (PadSize % CharBitNum == 0) {
      PadSize = PadSize / CharBitNum;
      InBits = false;
    }
    if (D->getIdentifier())
      Diag(D->getLocation(), diag::warn_padded_struct_field)
          << getPaddingDiagFromTagKind(D->getParent()->getTagKind())
          << Context.getTypeDeclType(D->getParent())
          << PadSize
          << (InBits ? 1 : 0) /*(byte|bit)*/ << (PadSize > 1) // plural or not
          << D->getIdentifier();
    else
      Diag(D->getLocation(), diag::warn_padded_struct_anon_field)
          << getPaddingDiagFromTagKind(D->getParent()->getTagKind())
          << Context.getTypeDeclType(D->getParent())
          << PadSize
          << (InBits ? 1 : 0) /*(byte|bit)*/ << (PadSize > 1); // plural or not
  }

  // Warn if we packed it unnecessarily. If the alignment is 1 byte don't
  // bother since there won't be alignment issues.
  if (isPacked && UnpackedAlign > CharBitNum && Offset == UnpackedOffset)
    Diag(D->getLocation(), diag::warn_unnecessary_packed)
        << D->getIdentifier();
}

static const CXXMethodDecl *computeKeyFunction(ASTContext &Context,
                                               const CXXRecordDecl *RD) {
  // If a class isn't polymorphic it doesn't have a key function.
  if (!RD->isPolymorphic())
    return nullptr;

  // A class that is not externally visible doesn't have a key function. (Or
  // at least, there's no point to assigning a key function to such a class;
  // this doesn't affect the ABI.)
  if (!RD->isExternallyVisible())
    return nullptr;

  // Template instantiations don't have key functions per Itanium C++ ABI 5.2.6.
  // Same behavior as GCC.
  TemplateSpecializationKind TSK = RD->getTemplateSpecializationKind();
  if (TSK == TSK_ImplicitInstantiation ||
      TSK == TSK_ExplicitInstantiationDeclaration ||
      TSK == TSK_ExplicitInstantiationDefinition)
    return nullptr;

  bool allowInlineFunctions =
    Context.getTargetInfo().getCXXABI().canKeyFunctionBeInline();

  for (const CXXMethodDecl *MD : RD->methods()) {
    if (!MD->isVirtual())
      continue;

    if (MD->isPure())
      continue;

    // Ignore implicit member functions, they are always marked as inline, but
    // they don't have a body until they're defined.
    if (MD->isImplicit())
      continue;

    if (MD->isInlineSpecified())
      continue;

    if (MD->hasInlineBody())
      continue;

    // Ignore inline deleted or defaulted functions.
    if (!MD->isUserProvided())
      continue;

    // In certain ABIs, ignore functions with out-of-line inline definitions.
    if (!allowInlineFunctions) {
      const FunctionDecl *Def;
      if (MD->hasBody(Def) && Def->isInlineSpecified())
        continue;
    }

    // If the key function is dllimport but the class isn't, then the class has
    // no key function. The DLL that exports the key function won't export the
    // vtable in this case.
    if (MD->hasAttr<DLLImportAttr>() && !RD->hasAttr<DLLImportAttr>())
      return nullptr;

    // We found it.
    return MD;
  }

  return nullptr;
}

DiagnosticBuilder
RecordLayoutBuilder::Diag(SourceLocation Loc, unsigned DiagID) {
  return Context.getDiagnostics().Report(Loc, DiagID);
}

/// Does the target C++ ABI require us to skip over the tail-padding
/// of the given class (considering it as a base class) when allocating
/// objects?
static bool mustSkipTailPadding(TargetCXXABI ABI, const CXXRecordDecl *RD) {
  switch (ABI.getTailPaddingUseRules()) {
  case TargetCXXABI::AlwaysUseTailPadding:
    return false;

  case TargetCXXABI::UseTailPaddingUnlessPOD03:
    // FIXME: To the extent that this is meant to cover the Itanium ABI
    // rules, we should implement the restrictions about over-sized
    // bitfields:
    //
    // http://mentorembedded.github.com/cxx-abi/abi.html#POD :
    //   In general, a type is considered a POD for the purposes of
    //   layout if it is a POD type (in the sense of ISO C++
    //   [basic.types]). However, a POD-struct or POD-union (in the
    //   sense of ISO C++ [class]) with a bitfield member whose
    //   declared width is wider than the declared type of the
    //   bitfield is not a POD for the purpose of layout.  Similarly,
    //   an array type is not a POD for the purpose of layout if the
    //   element type of the array is not a POD for the purpose of
    //   layout.
    //
    //   Where references to the ISO C++ are made in this paragraph,
    //   the Technical Corrigendum 1 version of the standard is
    //   intended.
    return RD->isPOD();

  case TargetCXXABI::UseTailPaddingUnlessPOD11:
    // This is equivalent to RD->getTypeForDecl().isCXX11PODType(),
    // but with a lot of abstraction penalty stripped off.  This does
    // assume that these properties are set correctly even in C++98
    // mode; fortunately, that is true because we want to assign
    // consistently semantics to the type-traits intrinsics (or at
    // least as many of them as possible).
    return RD->isTrivial() && RD->isStandardLayout();
  }

  llvm_unreachable("bad tail-padding use kind");
}

static bool isMsLayout(const RecordDecl* D) {
  return D->getASTContext().getTargetInfo().getCXXABI().isMicrosoft();
}

// This section contains an implementation of struct layout that is, up to the
// included tests, compatible with cl.exe (2013).  The layout produced is
// significantly different than those produced by the Itanium ABI.  Here we note
// the most important differences.
//
// * The alignment of bitfields in unions is ignored when computing the
//   alignment of the union.
// * The existence of zero-width bitfield that occurs after anything other than
//   a non-zero length bitfield is ignored.
// * There is no explicit primary base for the purposes of layout.  All bases
//   with vfptrs are laid out first, followed by all bases without vfptrs.
// * The Itanium equivalent vtable pointers are split into a vfptr (virtual
//   function pointer) and a vbptr (virtual base pointer).  They can each be
//   shared with a, non-virtual bases. These bases need not be the same.  vfptrs
//   always occur at offset 0.  vbptrs can occur at an arbitrary offset and are
//   placed after the lexiographically last non-virtual base.  This placement
//   is always before fields but can be in the middle of the non-virtual bases
//   due to the two-pass layout scheme for non-virtual-bases.
// * Virtual bases sometimes require a 'vtordisp' field that is laid out before
//   the virtual base and is used in conjunction with virtual overrides during
//   construction and destruction.  This is always a 4 byte value and is used as
//   an alternative to constructor vtables.
// * vtordisps are allocated in a block of memory with size and alignment equal
//   to the alignment of the completed structure (before applying __declspec(
//   align())).  The vtordisp always occur at the end of the allocation block,
//   immediately prior to the virtual base.
// * vfptrs are injected after all bases and fields have been laid out.  In
//   order to guarantee proper alignment of all fields, the vfptr injection
//   pushes all bases and fields back by the alignment imposed by those bases
//   and fields.  This can potentially add a significant amount of padding.
//   vfptrs are always injected at offset 0.
// * vbptrs are injected after all bases and fields have been laid out.  In
//   order to guarantee proper alignment of all fields, the vfptr injection
//   pushes all bases and fields back by the alignment imposed by those bases
//   and fields.  This can potentially add a significant amount of padding.
//   vbptrs are injected immediately after the last non-virtual base as
//   lexiographically ordered in the code.  If this site isn't pointer aligned
//   the vbptr is placed at the next properly aligned location.  Enough padding
//   is added to guarantee a fit.
// * The last zero sized non-virtual base can be placed at the end of the
//   struct (potentially aliasing another object), or may alias with the first
//   field, even if they are of the same type.
// * The last zero size virtual base may be placed at the end of the struct
//   potentially aliasing another object.
// * The ABI attempts to avoid aliasing of zero sized bases by adding padding
//   between bases or vbases with specific properties.  The criteria for
//   additional padding between two bases is that the first base is zero sized
//   or ends with a zero sized subobject and the second base is zero sized or
//   trails with a zero sized base or field (sharing of vfptrs can reorder the
//   layout of the so the leading base is not always the first one declared).
//   This rule does take into account fields that are not records, so padding
//   will occur even if the last field is, e.g. an int. The padding added for
//   bases is 1 byte.  The padding added between vbases depends on the alignment
//   of the object but is at least 4 bytes (in both 32 and 64 bit modes).
// * There is no concept of non-virtual alignment, non-virtual alignment and
//   alignment are always identical.
// * There is a distinction between alignment and required alignment.
//   __declspec(align) changes the required alignment of a struct.  This
//   alignment is _always_ obeyed, even in the presence of #pragma pack. A
//   record inherits required alignment from all of its fields and bases.
// * __declspec(align) on bitfields has the effect of changing the bitfield's
//   alignment instead of its required alignment.  This is the only known way
//   to make the alignment of a struct bigger than 8.  Interestingly enough
//   this alignment is also immune to the effects of #pragma pack and can be
//   used to create structures with large alignment under #pragma pack.
//   However, because it does not impact required alignment, such a structure,
//   when used as a field or base, will not be aligned if #pragma pack is
//   still active at the time of use.
//
// Known incompatibilities:
// * all: #pragma pack between fields in a record
// * 2010 and back: If the last field in a record is a bitfield, every object
//   laid out after the record will have extra padding inserted before it.  The
//   extra padding will have size equal to the size of the storage class of the
//   bitfield.  0 sized bitfields don't exhibit this behavior and the extra
//   padding can be avoided by adding a 0 sized bitfield after the non-zero-
//   sized bitfield.
// * 2012 and back: In 64-bit mode, if the alignment of a record is 16 or
//   greater due to __declspec(align()) then a second layout phase occurs after
//   The locations of the vf and vb pointers are known.  This layout phase
//   suffers from the "last field is a bitfield" bug in 2010 and results in
//   _every_ field getting padding put in front of it, potentially including the
//   vfptr, leaving the vfprt at a non-zero location which results in a fault if
//   anything tries to read the vftbl.  The second layout phase also treats
//   bitfields as separate entities and gives them each storage rather than
//   packing them.  Additionally, because this phase appears to perform a
//   (an unstable) sort on the members before laying them out and because merged
//   bitfields have the same address, the bitfields end up in whatever order
//   the sort left them in, a behavior we could never hope to replicate.

namespace {
struct MicrosoftRecordLayoutBuilder {
  struct ElementInfo {
    CharUnits Size;
    CharUnits Alignment;
  };
  typedef llvm::DenseMap<const CXXRecordDecl *, CharUnits> BaseOffsetsMapTy;
  MicrosoftRecordLayoutBuilder(const ASTContext &Context) : Context(Context) {}
private:
  MicrosoftRecordLayoutBuilder(const MicrosoftRecordLayoutBuilder &) = delete;
  void operator=(const MicrosoftRecordLayoutBuilder &) = delete;
public:
  void layout(const RecordDecl *RD);
  void cxxLayout(const CXXRecordDecl *RD);
  /// \brief Initializes size and alignment and honors some flags.
  void initializeLayout(const RecordDecl *RD);
  /// \brief Initialized C++ layout, compute alignment and virtual alignment and
  /// existence of vfptrs and vbptrs.  Alignment is needed before the vfptr is
  /// laid out.
  void initializeCXXLayout(const CXXRecordDecl *RD);
  void layoutNonVirtualBases(const CXXRecordDecl *RD);
  void layoutNonVirtualBase(const CXXRecordDecl *BaseDecl,
                            const ASTRecordLayout &BaseLayout,
                            const ASTRecordLayout *&PreviousBaseLayout);
  void injectVFPtr(const CXXRecordDecl *RD);
  void injectVBPtr(const CXXRecordDecl *RD);
  /// \brief Lays out the fields of the record.  Also rounds size up to
  /// alignment.
  void layoutFields(const RecordDecl *RD);
  void layoutField(const FieldDecl *FD);
  void layoutBitField(const FieldDecl *FD);
  /// \brief Lays out a single zero-width bit-field in the record and handles
  /// special cases associated with zero-width bit-fields.
  void layoutZeroWidthBitField(const FieldDecl *FD);
  void layoutVirtualBases(const CXXRecordDecl *RD);
  void finalizeLayout(const RecordDecl *RD);
  /// \brief Gets the size and alignment of a base taking pragma pack and
  /// __declspec(align) into account.
  ElementInfo getAdjustedElementInfo(const ASTRecordLayout &Layout);
  /// \brief Gets the size and alignment of a field taking pragma  pack and
  /// __declspec(align) into account.  It also updates RequiredAlignment as a
  /// side effect because it is most convenient to do so here.
  ElementInfo getAdjustedElementInfo(const FieldDecl *FD);
  /// \brief Places a field at an offset in CharUnits.
  void placeFieldAtOffset(CharUnits FieldOffset) {
    FieldOffsets.push_back(Context.toBits(FieldOffset));
  }
  /// \brief Places a bitfield at a bit offset.
  void placeFieldAtBitOffset(uint64_t FieldOffset) {
    FieldOffsets.push_back(FieldOffset);
  }
  /// \brief Compute the set of virtual bases for which vtordisps are required.
  void computeVtorDispSet(
      llvm::SmallPtrSetImpl<const CXXRecordDecl *> &HasVtorDispSet,
      const CXXRecordDecl *RD) const;
  const ASTContext &Context;
  /// \brief The size of the record being laid out.
  CharUnits Size;
  /// \brief The non-virtual size of the record layout.
  CharUnits NonVirtualSize;
  /// \brief The data size of the record layout.
  CharUnits DataSize;
  /// \brief The current alignment of the record layout.
  CharUnits Alignment;
  /// \brief The maximum allowed field alignment. This is set by #pragma pack.
  CharUnits MaxFieldAlignment;
  /// \brief The alignment that this record must obey.  This is imposed by
  /// __declspec(align()) on the record itself or one of its fields or bases.
  CharUnits RequiredAlignment;
  /// \brief The size of the allocation of the currently active bitfield.
  /// This value isn't meaningful unless LastFieldIsNonZeroWidthBitfield
  /// is true.
  CharUnits CurrentBitfieldSize;
  /// \brief Offset to the virtual base table pointer (if one exists).
  CharUnits VBPtrOffset;
  /// \brief Minimum record size possible.
  CharUnits MinEmptyStructSize;
  /// \brief The size and alignment info of a pointer.
  ElementInfo PointerInfo;
  /// \brief The primary base class (if one exists).
  const CXXRecordDecl *PrimaryBase;
  /// \brief The class we share our vb-pointer with.
  const CXXRecordDecl *SharedVBPtrBase;
  /// \brief The collection of field offsets.
  SmallVector<uint64_t, 16> FieldOffsets;
  /// \brief Base classes and their offsets in the record.
  BaseOffsetsMapTy Bases;
  /// \brief virtual base classes and their offsets in the record.
  ASTRecordLayout::VBaseOffsetsMapTy VBases;
  /// \brief The number of remaining bits in our last bitfield allocation.
  /// This value isn't meaningful unless LastFieldIsNonZeroWidthBitfield is
  /// true.
  unsigned RemainingBitsInField;
  bool IsUnion : 1;
  /// \brief True if the last field laid out was a bitfield and was not 0
  /// width.
  bool LastFieldIsNonZeroWidthBitfield : 1;
  /// \brief True if the class has its own vftable pointer.
  bool HasOwnVFPtr : 1;
  /// \brief True if the class has a vbtable pointer.
  bool HasVBPtr : 1;
  /// \brief True if the last sub-object within the type is zero sized or the
  /// object itself is zero sized.  This *does not* count members that are not
  /// records.  Only used for MS-ABI.
  bool EndsWithZeroSizedObject : 1;
  /// \brief True if this class is zero sized or first base is zero sized or
  /// has this property.  Only used for MS-ABI.
  bool LeadsWithZeroSizedBase : 1;

  /// \brief True if the external AST source provided a layout for this record.
  bool UseExternalLayout : 1;

  /// \brief The layout provided by the external AST source. Only active if
  /// UseExternalLayout is true.
  ExternalLayout External;
};
} // namespace

MicrosoftRecordLayoutBuilder::ElementInfo
MicrosoftRecordLayoutBuilder::getAdjustedElementInfo(
    const ASTRecordLayout &Layout) {
  ElementInfo Info;
  Info.Alignment = Layout.getAlignment();
  // Respect pragma pack.
  if (!MaxFieldAlignment.isZero())
    Info.Alignment = std::min(Info.Alignment, MaxFieldAlignment);
  // Track zero-sized subobjects here where it's already available.
  EndsWithZeroSizedObject = Layout.hasZeroSizedSubObject();
  // Respect required alignment, this is necessary because we may have adjusted
  // the alignment in the case of pragam pack.  Note that the required alignment
  // doesn't actually apply to the struct alignment at this point.
  Alignment = std::max(Alignment, Info.Alignment);
  RequiredAlignment = std::max(RequiredAlignment, Layout.getRequiredAlignment());
  Info.Alignment = std::max(Info.Alignment, Layout.getRequiredAlignment());
  Info.Size = Layout.getNonVirtualSize();
  return Info;
}

MicrosoftRecordLayoutBuilder::ElementInfo
MicrosoftRecordLayoutBuilder::getAdjustedElementInfo(
    const FieldDecl *FD) {
  // Get the alignment of the field type's natural alignment, ignore any
  // alignment attributes.
  ElementInfo Info;
  std::tie(Info.Size, Info.Alignment) =
      Context.getTypeInfoInChars(FD->getType()->getUnqualifiedDesugaredType());
  // Respect align attributes on the field.
  CharUnits FieldRequiredAlignment =
      Context.toCharUnitsFromBits(FD->getMaxAlignment());
  // Respect align attributes on the type.
  if (Context.isAlignmentRequired(FD->getType()))
    FieldRequiredAlignment = std::max(
        Context.getTypeAlignInChars(FD->getType()), FieldRequiredAlignment);
  // Respect attributes applied to subobjects of the field.
  if (FD->isBitField())
    // For some reason __declspec align impacts alignment rather than required
    // alignment when it is applied to bitfields.
    Info.Alignment = std::max(Info.Alignment, FieldRequiredAlignment);
  else {
    if (auto RT =
            FD->getType()->getBaseElementTypeUnsafe()->getAs<RecordType>()) {
      auto const &Layout = Context.getASTRecordLayout(RT->getDecl());
      EndsWithZeroSizedObject = Layout.hasZeroSizedSubObject();
      FieldRequiredAlignment = std::max(FieldRequiredAlignment,
                                        Layout.getRequiredAlignment());
    }
    // Capture required alignment as a side-effect.
    RequiredAlignment = std::max(RequiredAlignment, FieldRequiredAlignment);
  }
  // Respect pragma pack, attribute pack and declspec align
  if (!MaxFieldAlignment.isZero())
    Info.Alignment = std::min(Info.Alignment, MaxFieldAlignment);
  if (FD->hasAttr<PackedAttr>())
    Info.Alignment = CharUnits::One();
  Info.Alignment = std::max(Info.Alignment, FieldRequiredAlignment);
  return Info;
}

void MicrosoftRecordLayoutBuilder::layout(const RecordDecl *RD) {
  // For C record layout, zero-sized records always have size 4.
  MinEmptyStructSize = CharUnits::fromQuantity(4);
  initializeLayout(RD);
  layoutFields(RD);
  DataSize = Size = Size.RoundUpToAlignment(Alignment);
  RequiredAlignment = std::max(
      RequiredAlignment, Context.toCharUnitsFromBits(RD->getMaxAlignment()));
  finalizeLayout(RD);
}

void MicrosoftRecordLayoutBuilder::cxxLayout(const CXXRecordDecl *RD) {
  // HLSL Change Begins
  if (Context.getLangOpts().HLSL) {
    MinEmptyStructSize = CharUnits::fromQuantity(0);
  }
  else { // HLSL Change Ends
    // The C++ standard says that empty structs have size 1.
    MinEmptyStructSize = CharUnits::One();
  } // HLSL Change
  initializeLayout(RD);
  initializeCXXLayout(RD);
  layoutNonVirtualBases(RD);
  layoutFields(RD);
  injectVBPtr(RD);
  injectVFPtr(RD);
  if (HasOwnVFPtr || (HasVBPtr && !SharedVBPtrBase))
    Alignment = std::max(Alignment, PointerInfo.Alignment);
  auto RoundingAlignment = Alignment;
  if (!MaxFieldAlignment.isZero())
    RoundingAlignment = std::min(RoundingAlignment, MaxFieldAlignment);
  NonVirtualSize = Size = Size.RoundUpToAlignment(RoundingAlignment);
  RequiredAlignment = std::max(
      RequiredAlignment, Context.toCharUnitsFromBits(RD->getMaxAlignment()));
  layoutVirtualBases(RD);
  finalizeLayout(RD);
}

void MicrosoftRecordLayoutBuilder::initializeLayout(const RecordDecl *RD) {
  IsUnion = RD->isUnion();
  Size = CharUnits::Zero();
  Alignment = CharUnits::One();
  // In 64-bit mode we always perform an alignment step after laying out vbases.
  // In 32-bit mode we do not.  The check to see if we need to perform alignment
  // checks the RequiredAlignment field and performs alignment if it isn't 0.
  RequiredAlignment = Context.getTargetInfo().getTriple().isArch64Bit()
                          ? CharUnits::One()
                          : CharUnits::Zero();
  // Compute the maximum field alignment.
  MaxFieldAlignment = CharUnits::Zero();
  // Honor the default struct packing maximum alignment flag.
  if (unsigned DefaultMaxFieldAlignment = Context.getLangOpts().PackStruct)
      MaxFieldAlignment = CharUnits::fromQuantity(DefaultMaxFieldAlignment);
  // Honor the packing attribute.  The MS-ABI ignores pragma pack if its larger
  // than the pointer size.
  if (const MaxFieldAlignmentAttr *MFAA = RD->getAttr<MaxFieldAlignmentAttr>()){
    unsigned PackedAlignment = MFAA->getAlignment();
    if (PackedAlignment <= Context.getTargetInfo().getPointerWidth(0))
      MaxFieldAlignment = Context.toCharUnitsFromBits(PackedAlignment);
  }
  // Packed attribute forces max field alignment to be 1.
  if (RD->hasAttr<PackedAttr>())
    MaxFieldAlignment = CharUnits::One();

  // Try to respect the external layout if present.
  UseExternalLayout = false;
  if (ExternalASTSource *Source = Context.getExternalSource())
    UseExternalLayout = Source->layoutRecordType(
        RD, External.Size, External.Align, External.FieldOffsets,
        External.BaseOffsets, External.VirtualBaseOffsets);
}

void
MicrosoftRecordLayoutBuilder::initializeCXXLayout(const CXXRecordDecl *RD) {
  EndsWithZeroSizedObject = false;
  LeadsWithZeroSizedBase = false;
  HasOwnVFPtr = false;
  HasVBPtr = false;
  PrimaryBase = nullptr;
  SharedVBPtrBase = nullptr;
  // Calculate pointer size and alignment.  These are used for vfptr and vbprt
  // injection.
  PointerInfo.Size =
      Context.toCharUnitsFromBits(Context.getTargetInfo().getPointerWidth(0));
  PointerInfo.Alignment =
      Context.toCharUnitsFromBits(Context.getTargetInfo().getPointerAlign(0));
  // Respect pragma pack.
  if (!MaxFieldAlignment.isZero())
    PointerInfo.Alignment = std::min(PointerInfo.Alignment, MaxFieldAlignment);
}

void
MicrosoftRecordLayoutBuilder::layoutNonVirtualBases(const CXXRecordDecl *RD) {
  // The MS-ABI lays out all bases that contain leading vfptrs before it lays
  // out any bases that do not contain vfptrs.  We implement this as two passes
  // over the bases.  This approach guarantees that the primary base is laid out
  // first.  We use these passes to calculate some additional aggregated
  // information about the bases, such as reqruied alignment and the presence of
  // zero sized members.
  const ASTRecordLayout *PreviousBaseLayout = nullptr;
  // Iterate through the bases and lay out the non-virtual ones.
  for (const CXXBaseSpecifier &Base : RD->bases()) {
    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();
    const ASTRecordLayout &BaseLayout = Context.getASTRecordLayout(BaseDecl);
    // Mark and skip virtual bases.
    if (Base.isVirtual()) {
      HasVBPtr = true;
      continue;
    }
    // Check fo a base to share a VBPtr with.
    if (!SharedVBPtrBase && BaseLayout.hasVBPtr()) {
      SharedVBPtrBase = BaseDecl;
      HasVBPtr = true;
    }
    // Only lay out bases with extendable VFPtrs on the first pass.
    if (!BaseLayout.hasExtendableVFPtr())
      continue;
    // If we don't have a primary base, this one qualifies.
    if (!PrimaryBase) {
      PrimaryBase = BaseDecl;
      LeadsWithZeroSizedBase = BaseLayout.leadsWithZeroSizedBase();
    }
    // Lay out the base.
    layoutNonVirtualBase(BaseDecl, BaseLayout, PreviousBaseLayout);
  }
  // Figure out if we need a fresh VFPtr for this class.
  if (!PrimaryBase && RD->isDynamicClass())
    for (CXXRecordDecl::method_iterator i = RD->method_begin(),
                                        e = RD->method_end();
         !HasOwnVFPtr && i != e; ++i)
      HasOwnVFPtr = i->isVirtual() && i->size_overridden_methods() == 0;
  // If we don't have a primary base then we have a leading object that could
  // itself lead with a zero-sized object, something we track.
  bool CheckLeadingLayout = !PrimaryBase;
  // Iterate through the bases and lay out the non-virtual ones.
  for (const CXXBaseSpecifier &Base : RD->bases()) {
    if (Base.isVirtual())
      continue;
    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();
    const ASTRecordLayout &BaseLayout = Context.getASTRecordLayout(BaseDecl);
    // Only lay out bases without extendable VFPtrs on the second pass.
    if (BaseLayout.hasExtendableVFPtr()) {
      VBPtrOffset = Bases[BaseDecl] + BaseLayout.getNonVirtualSize();
      continue;
    }
    // If this is the first layout, check to see if it leads with a zero sized
    // object.  If it does, so do we.
    if (CheckLeadingLayout) {
      CheckLeadingLayout = false;
      LeadsWithZeroSizedBase = BaseLayout.leadsWithZeroSizedBase();
    }
    // Lay out the base.
    layoutNonVirtualBase(BaseDecl, BaseLayout, PreviousBaseLayout);
    VBPtrOffset = Bases[BaseDecl] + BaseLayout.getNonVirtualSize();
  }
  // Set our VBPtroffset if we know it at this point.
  if (!HasVBPtr)
    VBPtrOffset = CharUnits::fromQuantity(-1);
  else if (SharedVBPtrBase) {
    const ASTRecordLayout &Layout = Context.getASTRecordLayout(SharedVBPtrBase);
    VBPtrOffset = Bases[SharedVBPtrBase] + Layout.getVBPtrOffset();
  }
}

void MicrosoftRecordLayoutBuilder::layoutNonVirtualBase(
    const CXXRecordDecl *BaseDecl,
    const ASTRecordLayout &BaseLayout,
    const ASTRecordLayout *&PreviousBaseLayout) {
  // Insert padding between two bases if the left first one is zero sized or
  // contains a zero sized subobject and the right is zero sized or one leads
  // with a zero sized base.
  if (PreviousBaseLayout && PreviousBaseLayout->hasZeroSizedSubObject() &&
      BaseLayout.leadsWithZeroSizedBase())
    Size++;
  ElementInfo Info = getAdjustedElementInfo(BaseLayout);
  CharUnits BaseOffset;

  // Respect the external AST source base offset, if present.
  bool FoundBase = false;
  if (UseExternalLayout) {
    FoundBase = External.getExternalNVBaseOffset(BaseDecl, BaseOffset);
    if (FoundBase)
      assert(BaseOffset >= Size && "base offset already allocated");
  }

  if (!FoundBase)
    BaseOffset = Size.RoundUpToAlignment(Info.Alignment);
  Bases.insert(std::make_pair(BaseDecl, BaseOffset));
  Size = BaseOffset + BaseLayout.getNonVirtualSize();
  PreviousBaseLayout = &BaseLayout;
}

void MicrosoftRecordLayoutBuilder::layoutFields(const RecordDecl *RD) {
  LastFieldIsNonZeroWidthBitfield = false;
  for (const FieldDecl *Field : RD->fields())
    layoutField(Field);
}

void MicrosoftRecordLayoutBuilder::layoutField(const FieldDecl *FD) {
  if (FD->isBitField()) {
    layoutBitField(FD);
    return;
  }
  LastFieldIsNonZeroWidthBitfield = false;
  ElementInfo Info = getAdjustedElementInfo(FD);
  Alignment = std::max(Alignment, Info.Alignment);
  if (IsUnion) {
    placeFieldAtOffset(CharUnits::Zero());
    Size = std::max(Size, Info.Size);
  } else {
    CharUnits FieldOffset;
    if (UseExternalLayout) {
      FieldOffset =
          Context.toCharUnitsFromBits(External.getExternalFieldOffset(FD));
      assert(FieldOffset >= Size && "field offset already allocated");
    } else {
      FieldOffset = Size.RoundUpToAlignment(Info.Alignment);
    }
    placeFieldAtOffset(FieldOffset);
    Size = FieldOffset + Info.Size;
  }
}

void MicrosoftRecordLayoutBuilder::layoutBitField(const FieldDecl *FD) {
  unsigned Width = FD->getBitWidthValue(Context);
  if (Width == 0) {
    layoutZeroWidthBitField(FD);
    return;
  }
  ElementInfo Info = getAdjustedElementInfo(FD);
  // Clamp the bitfield to a containable size for the sake of being able
  // to lay them out.  Sema will throw an error.
  if (Width > Context.toBits(Info.Size))
    Width = Context.toBits(Info.Size);
  // Check to see if this bitfield fits into an existing allocation.  Note:
  // MSVC refuses to pack bitfields of formal types with different sizes
  // into the same allocation.
  if (!IsUnion && LastFieldIsNonZeroWidthBitfield &&
      CurrentBitfieldSize == Info.Size && Width <= RemainingBitsInField) {
    placeFieldAtBitOffset(Context.toBits(Size) - RemainingBitsInField);
    RemainingBitsInField -= Width;
    return;
  }
  LastFieldIsNonZeroWidthBitfield = true;
  CurrentBitfieldSize = Info.Size;
  if (IsUnion) {
    placeFieldAtOffset(CharUnits::Zero());
    Size = std::max(Size, Info.Size);
    // TODO: Add a Sema warning that MS ignores bitfield alignment in unions.
  } else {
    // Allocate a new block of memory and place the bitfield in it.
    CharUnits FieldOffset = Size.RoundUpToAlignment(Info.Alignment);
    placeFieldAtOffset(FieldOffset);
    Size = FieldOffset + Info.Size;
    Alignment = std::max(Alignment, Info.Alignment);
    RemainingBitsInField = Context.toBits(Info.Size) - Width;
  }
}

void
MicrosoftRecordLayoutBuilder::layoutZeroWidthBitField(const FieldDecl *FD) {
  // Zero-width bitfields are ignored unless they follow a non-zero-width
  // bitfield.
  if (!LastFieldIsNonZeroWidthBitfield) {
    placeFieldAtOffset(IsUnion ? CharUnits::Zero() : Size);
    // TODO: Add a Sema warning that MS ignores alignment for zero
    // sized bitfields that occur after zero-size bitfields or non-bitfields.
    return;
  }
  LastFieldIsNonZeroWidthBitfield = false;
  ElementInfo Info = getAdjustedElementInfo(FD);
  if (IsUnion) {
    placeFieldAtOffset(CharUnits::Zero());
    Size = std::max(Size, Info.Size);
    // TODO: Add a Sema warning that MS ignores bitfield alignment in unions.
  } else {
    // Round up the current record size to the field's alignment boundary.
    CharUnits FieldOffset = Size.RoundUpToAlignment(Info.Alignment);
    placeFieldAtOffset(FieldOffset);
    Size = FieldOffset;
    Alignment = std::max(Alignment, Info.Alignment);
  }
}

void MicrosoftRecordLayoutBuilder::injectVBPtr(const CXXRecordDecl *RD) {
  if (!HasVBPtr || SharedVBPtrBase)
    return;
  // Inject the VBPointer at the injection site.
  CharUnits InjectionSite = VBPtrOffset;
  // But before we do, make sure it's properly aligned.
  VBPtrOffset = VBPtrOffset.RoundUpToAlignment(PointerInfo.Alignment);
  // Shift everything after the vbptr down, unless we're using an external
  // layout.
  if (UseExternalLayout)
    return;
  // Determine where the first field should be laid out after the vbptr.
  CharUnits FieldStart = VBPtrOffset + PointerInfo.Size;
  // Make sure that the amount we push the fields back by is a multiple of the
  // alignment.
  CharUnits Offset = (FieldStart - InjectionSite).RoundUpToAlignment(
      std::max(RequiredAlignment, Alignment));
  Size += Offset;
  for (uint64_t &FieldOffset : FieldOffsets)
    FieldOffset += Context.toBits(Offset);
  for (BaseOffsetsMapTy::value_type &Base : Bases)
    if (Base.second >= InjectionSite)
      Base.second += Offset;
}

void MicrosoftRecordLayoutBuilder::injectVFPtr(const CXXRecordDecl *RD) {
  if (!HasOwnVFPtr)
    return;
  // Make sure that the amount we push the struct back by is a multiple of the
  // alignment.
  CharUnits Offset = PointerInfo.Size.RoundUpToAlignment(
      std::max(RequiredAlignment, Alignment));
  // Increase the size of the object and push back all fields, the vbptr and all
  // bases by the offset amount.
  Size += Offset;
  for (uint64_t &FieldOffset : FieldOffsets)
    FieldOffset += Context.toBits(Offset);
  if (HasVBPtr)
    VBPtrOffset += Offset;
  for (BaseOffsetsMapTy::value_type &Base : Bases)
    Base.second += Offset;
}

void MicrosoftRecordLayoutBuilder::layoutVirtualBases(const CXXRecordDecl *RD) {
  if (!HasVBPtr)
    return;
  // Vtordisps are always 4 bytes (even in 64-bit mode)
  CharUnits VtorDispSize = CharUnits::fromQuantity(4);
  CharUnits VtorDispAlignment = VtorDispSize;
  // vtordisps respect pragma pack.
  if (!MaxFieldAlignment.isZero())
    VtorDispAlignment = std::min(VtorDispAlignment, MaxFieldAlignment);
  // The alignment of the vtordisp is at least the required alignment of the
  // entire record.  This requirement may be present to support vtordisp
  // injection.
  for (const CXXBaseSpecifier &VBase : RD->vbases()) {
    const CXXRecordDecl *BaseDecl = VBase.getType()->getAsCXXRecordDecl();
    const ASTRecordLayout &BaseLayout = Context.getASTRecordLayout(BaseDecl);
    RequiredAlignment =
        std::max(RequiredAlignment, BaseLayout.getRequiredAlignment());
  }
  VtorDispAlignment = std::max(VtorDispAlignment, RequiredAlignment);
  // Compute the vtordisp set.
  llvm::SmallPtrSet<const CXXRecordDecl *, 2> HasVtorDispSet;
  computeVtorDispSet(HasVtorDispSet, RD);
  // Iterate through the virtual bases and lay them out.
  const ASTRecordLayout *PreviousBaseLayout = nullptr;
  for (const CXXBaseSpecifier &VBase : RD->vbases()) {
    const CXXRecordDecl *BaseDecl = VBase.getType()->getAsCXXRecordDecl();
    const ASTRecordLayout &BaseLayout = Context.getASTRecordLayout(BaseDecl);
    bool HasVtordisp = HasVtorDispSet.count(BaseDecl) > 0;
    // Insert padding between two bases if the left first one is zero sized or
    // contains a zero sized subobject and the right is zero sized or one leads
    // with a zero sized base.  The padding between virtual bases is 4
    // bytes (in both 32 and 64 bits modes) and always involves rounding up to
    // the required alignment, we don't know why.
    if ((PreviousBaseLayout && PreviousBaseLayout->hasZeroSizedSubObject() &&
        BaseLayout.leadsWithZeroSizedBase()) || HasVtordisp) {
      Size = Size.RoundUpToAlignment(VtorDispAlignment) + VtorDispSize;
      Alignment = std::max(VtorDispAlignment, Alignment);
    }
    // Insert the virtual base.
    ElementInfo Info = getAdjustedElementInfo(BaseLayout);
    CharUnits BaseOffset;

    // Respect the external AST source base offset, if present.
    bool FoundBase = false;
    if (UseExternalLayout) {
      FoundBase = External.getExternalVBaseOffset(BaseDecl, BaseOffset);
      if (FoundBase)
        assert(BaseOffset >= Size && "base offset already allocated");
    }
    if (!FoundBase)
      BaseOffset = Size.RoundUpToAlignment(Info.Alignment);

    VBases.insert(std::make_pair(BaseDecl,
        ASTRecordLayout::VBaseInfo(BaseOffset, HasVtordisp)));
    Size = BaseOffset + BaseLayout.getNonVirtualSize();
    PreviousBaseLayout = &BaseLayout;
  }
}

void MicrosoftRecordLayoutBuilder::finalizeLayout(const RecordDecl *RD) {
  // Respect required alignment.  Note that in 32-bit mode Required alignment
  // may be 0 and cause size not to be updated.
  DataSize = Size;
  if (!RequiredAlignment.isZero()) {
    Alignment = std::max(Alignment, RequiredAlignment);
    auto RoundingAlignment = Alignment;
    if (!MaxFieldAlignment.isZero())
      RoundingAlignment = std::min(RoundingAlignment, MaxFieldAlignment);
    RoundingAlignment = std::max(RoundingAlignment, RequiredAlignment);
    Size = Size.RoundUpToAlignment(RoundingAlignment);
  }
  if (Size.isZero()) {
    EndsWithZeroSizedObject = true;
    LeadsWithZeroSizedBase = true;
    if (!Context.getLangOpts().HLSL) { // HLSL Change - allow empty structs to be zero sized
    // Zero-sized structures have size equal to their alignment if a
    // __declspec(align) came into play.
    if (RequiredAlignment >= MinEmptyStructSize)
      Size = Alignment;
    else
      Size = MinEmptyStructSize;
    } // HLSL Change
  }

  if (UseExternalLayout) {
    Size = Context.toCharUnitsFromBits(External.Size);
    if (External.Align)
      Alignment = Context.toCharUnitsFromBits(External.Align);
  }
}

// Recursively walks the non-virtual bases of a class and determines if any of
// them are in the bases with overridden methods set.
static bool
RequiresVtordisp(const llvm::SmallPtrSetImpl<const CXXRecordDecl *> &
                     BasesWithOverriddenMethods,
                 const CXXRecordDecl *RD) {
  if (BasesWithOverriddenMethods.count(RD))
    return true;
  // If any of a virtual bases non-virtual bases (recursively) requires a
  // vtordisp than so does this virtual base.
  for (const CXXBaseSpecifier &Base : RD->bases())
    if (!Base.isVirtual() &&
        RequiresVtordisp(BasesWithOverriddenMethods,
                         Base.getType()->getAsCXXRecordDecl()))
      return true;
  return false;
}

void MicrosoftRecordLayoutBuilder::computeVtorDispSet(
    llvm::SmallPtrSetImpl<const CXXRecordDecl *> &HasVtordispSet,
    const CXXRecordDecl *RD) const {
  // /vd2 or #pragma vtordisp(2): Always use vtordisps for virtual bases with
  // vftables.
  if (RD->getMSVtorDispMode() == MSVtorDispAttr::ForVFTable) {
    for (const CXXBaseSpecifier &Base : RD->vbases()) {
      const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();
      const ASTRecordLayout &Layout = Context.getASTRecordLayout(BaseDecl);
      if (Layout.hasExtendableVFPtr())
        HasVtordispSet.insert(BaseDecl);
    }
    return;
  }

  // If any of our bases need a vtordisp for this type, so do we.  Check our
  // direct bases for vtordisp requirements.
  for (const CXXBaseSpecifier &Base : RD->bases()) {
    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();
    const ASTRecordLayout &Layout = Context.getASTRecordLayout(BaseDecl);
    for (const auto &bi : Layout.getVBaseOffsetsMap())
      if (bi.second.hasVtorDisp())
        HasVtordispSet.insert(bi.first);
  }
  // We don't introduce any additional vtordisps if either:
  // * A user declared constructor or destructor aren't declared.
  // * #pragma vtordisp(0) or the /vd0 flag are in use.
  if ((!RD->hasUserDeclaredConstructor() && !RD->hasUserDeclaredDestructor()) ||
      RD->getMSVtorDispMode() == MSVtorDispAttr::Never)
    return;
  // /vd1 or #pragma vtordisp(1): Try to guess based on whether we think it's
  // possible for a partially constructed object with virtual base overrides to
  // escape a non-trivial constructor.
  assert(RD->getMSVtorDispMode() == MSVtorDispAttr::ForVBaseOverride);
  // Compute a set of base classes which define methods we override.  A virtual
  // base in this set will require a vtordisp.  A virtual base that transitively
  // contains one of these bases as a non-virtual base will also require a
  // vtordisp.
  llvm::SmallPtrSet<const CXXMethodDecl *, 8> Work;
  llvm::SmallPtrSet<const CXXRecordDecl *, 2> BasesWithOverriddenMethods;
  // Seed the working set with our non-destructor, non-pure virtual methods.
  for (const CXXMethodDecl *MD : RD->methods())
    if (MD->isVirtual() && !isa<CXXDestructorDecl>(MD) && !MD->isPure())
      Work.insert(MD);
  while (!Work.empty()) {
    const CXXMethodDecl *MD = *Work.begin();
    CXXMethodDecl::method_iterator i = MD->begin_overridden_methods(),
                                   e = MD->end_overridden_methods();
    // If a virtual method has no-overrides it lives in its parent's vtable.
    if (i == e)
      BasesWithOverriddenMethods.insert(MD->getParent());
    else
      Work.insert(i, e);
    // We've finished processing this element, remove it from the working set.
    Work.erase(MD);
  }
  // For each of our virtual bases, check if it is in the set of overridden
  // bases or if it transitively contains a non-virtual base that is.
  for (const CXXBaseSpecifier &Base : RD->vbases()) {
    const CXXRecordDecl *BaseDecl = Base.getType()->getAsCXXRecordDecl();
    if (!HasVtordispSet.count(BaseDecl) &&
        RequiresVtordisp(BasesWithOverriddenMethods, BaseDecl))
      HasVtordispSet.insert(BaseDecl);
  }
}

/// \brief Get or compute information about the layout of the specified record
/// (struct/union/class), which indicates its size and field position
/// information.
const ASTRecordLayout *
ASTContext::BuildMicrosoftASTRecordLayout(const RecordDecl *D) const {
  MicrosoftRecordLayoutBuilder Builder(*this);
  if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(D)) {
    Builder.cxxLayout(RD);
    return new (*this) ASTRecordLayout(
        *this, Builder.Size, Builder.Alignment, Builder.RequiredAlignment,
        Builder.HasOwnVFPtr,
        Builder.HasOwnVFPtr || Builder.PrimaryBase,
        Builder.VBPtrOffset, Builder.NonVirtualSize, Builder.FieldOffsets.data(),
        Builder.FieldOffsets.size(), Builder.NonVirtualSize,
        Builder.Alignment, CharUnits::Zero(), Builder.PrimaryBase,
        false, Builder.SharedVBPtrBase,
        Builder.EndsWithZeroSizedObject, Builder.LeadsWithZeroSizedBase,
        Builder.Bases, Builder.VBases);
  } else {
    Builder.layout(D);
    return new (*this) ASTRecordLayout(
        *this, Builder.Size, Builder.Alignment, Builder.RequiredAlignment,
        Builder.Size, Builder.FieldOffsets.data(), Builder.FieldOffsets.size());
  }
}

/// getASTRecordLayout - Get or compute information about the layout of the
/// specified record (struct/union/class), which indicates its size and field
/// position information.
const ASTRecordLayout &
ASTContext::getASTRecordLayout(const RecordDecl *D) const {
  // These asserts test different things.  A record has a definition
  // as soon as we begin to parse the definition.  That definition is
  // not a complete definition (which is what isDefinition() tests)
  // until we *finish* parsing the definition.

  if (D->hasExternalLexicalStorage() && !D->getDefinition())
    getExternalSource()->CompleteType(const_cast<RecordDecl*>(D));
    
  D = D->getDefinition();
  assert(D && "Cannot get layout of forward declarations!");
  assert(!D->isInvalidDecl() && "Cannot get layout of invalid decl!");
  assert(D->isCompleteDefinition() && "Cannot layout type before complete!");

  // Look up this layout, if already laid out, return what we have.
  // Note that we can't save a reference to the entry because this function
  // is recursive.
  const ASTRecordLayout *Entry = ASTRecordLayouts[D];
  if (Entry) return *Entry;

  const ASTRecordLayout *NewEntry = nullptr;

  if (isMsLayout(D)) {
    NewEntry = BuildMicrosoftASTRecordLayout(D);
  } else if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(D)) {
    EmptySubobjectMap EmptySubobjects(*this, RD);
    RecordLayoutBuilder Builder(*this, &EmptySubobjects);
    Builder.Layout(RD);

    // In certain situations, we are allowed to lay out objects in the
    // tail-padding of base classes.  This is ABI-dependent.
    // FIXME: this should be stored in the record layout.
    bool skipTailPadding =
      mustSkipTailPadding(getTargetInfo().getCXXABI(), cast<CXXRecordDecl>(D));

    // FIXME: This should be done in FinalizeLayout.
    CharUnits DataSize =
      skipTailPadding ? Builder.getSize() : Builder.getDataSize();
    CharUnits NonVirtualSize = 
      skipTailPadding ? DataSize : Builder.NonVirtualSize;
    NewEntry =
      new (*this) ASTRecordLayout(*this, Builder.getSize(), 
                                  Builder.Alignment,
                                  /*RequiredAlignment : used by MS-ABI)*/
                                  Builder.Alignment,
                                  Builder.HasOwnVFPtr,
                                  RD->isDynamicClass(),
                                  CharUnits::fromQuantity(-1),
                                  DataSize, 
                                  Builder.FieldOffsets.data(),
                                  Builder.FieldOffsets.size(),
                                  NonVirtualSize,
                                  Builder.NonVirtualAlignment,
                                  EmptySubobjects.SizeOfLargestEmptySubobject,
                                  Builder.PrimaryBase,
                                  Builder.PrimaryBaseIsVirtual,
                                  nullptr, false, false,
                                  Builder.Bases, Builder.VBases);
  } else {
    RecordLayoutBuilder Builder(*this, /*EmptySubobjects=*/nullptr);
    Builder.Layout(D);

    NewEntry =
      new (*this) ASTRecordLayout(*this, Builder.getSize(), 
                                  Builder.Alignment,
                                  /*RequiredAlignment : used by MS-ABI)*/
                                  Builder.Alignment,
                                  Builder.getSize(),
                                  Builder.FieldOffsets.data(),
                                  Builder.FieldOffsets.size());
  }

  ASTRecordLayouts[D] = NewEntry;

  if (getLangOpts().DumpRecordLayouts) {
    llvm::outs() << "\n*** Dumping AST Record Layout\n";
    DumpRecordLayout(D, llvm::outs(), getLangOpts().DumpRecordLayoutsSimple);
  }

  return *NewEntry;
}

const CXXMethodDecl *ASTContext::getCurrentKeyFunction(const CXXRecordDecl *RD) {
  if (!getTargetInfo().getCXXABI().hasKeyFunctions())
    return nullptr;

  assert(RD->getDefinition() && "Cannot get key function for forward decl!");
  RD = cast<CXXRecordDecl>(RD->getDefinition());

  // Beware:
  //  1) computing the key function might trigger deserialization, which might
  //     invalidate iterators into KeyFunctions
  //  2) 'get' on the LazyDeclPtr might also trigger deserialization and
  //     invalidate the LazyDeclPtr within the map itself
  LazyDeclPtr Entry = KeyFunctions[RD];
  const Decl *Result =
      Entry ? Entry.get(getExternalSource()) : computeKeyFunction(*this, RD);

  // Store it back if it changed.
  if (Entry.isOffset() || Entry.isValid() != bool(Result))
    KeyFunctions[RD] = const_cast<Decl*>(Result);

  return cast_or_null<CXXMethodDecl>(Result);
}

void ASTContext::setNonKeyFunction(const CXXMethodDecl *Method) {
  assert(Method == Method->getFirstDecl() &&
         "not working with method declaration from class definition");

  // Look up the cache entry.  Since we're working with the first
  // declaration, its parent must be the class definition, which is
  // the correct key for the KeyFunctions hash.
  const auto &Map = KeyFunctions;
  auto I = Map.find(Method->getParent());

  // If it's not cached, there's nothing to do.
  if (I == Map.end()) return;

  // If it is cached, check whether it's the target method, and if so,
  // remove it from the cache. Note, the call to 'get' might invalidate
  // the iterator and the LazyDeclPtr object within the map.
  LazyDeclPtr Ptr = I->second;
  if (Ptr.get(getExternalSource()) == Method) {
    // FIXME: remember that we did this for module / chained PCH state?
    KeyFunctions.erase(Method->getParent());
  }
}

static uint64_t getFieldOffset(const ASTContext &C, const FieldDecl *FD) {
  const ASTRecordLayout &Layout = C.getASTRecordLayout(FD->getParent());
  return Layout.getFieldOffset(FD->getFieldIndex());
}

uint64_t ASTContext::getFieldOffset(const ValueDecl *VD) const {
  uint64_t OffsetInBits;
  if (const FieldDecl *FD = dyn_cast<FieldDecl>(VD)) {
    OffsetInBits = ::getFieldOffset(*this, FD);
  } else {
    const IndirectFieldDecl *IFD = cast<IndirectFieldDecl>(VD);

    OffsetInBits = 0;
    for (const NamedDecl *ND : IFD->chain())
      OffsetInBits += ::getFieldOffset(*this, cast<FieldDecl>(ND));
  }

  return OffsetInBits;
}

/// getObjCLayout - Get or compute information about the layout of the
/// given interface.
///
/// \param Impl - If given, also include the layout of the interface's
/// implementation. This may differ by including synthesized ivars.
const ASTRecordLayout &
ASTContext::getObjCLayout(const ObjCInterfaceDecl *D,
                          const ObjCImplementationDecl *Impl) const {
  // Retrieve the definition
  if (D->hasExternalLexicalStorage() && !D->getDefinition())
    getExternalSource()->CompleteType(const_cast<ObjCInterfaceDecl*>(D));
  D = D->getDefinition();
  assert(D && D->isThisDeclarationADefinition() && "Invalid interface decl!");

  // Look up this layout, if already laid out, return what we have.
  const ObjCContainerDecl *Key =
    Impl ? (const ObjCContainerDecl*) Impl : (const ObjCContainerDecl*) D;
  if (const ASTRecordLayout *Entry = ObjCLayouts[Key])
    return *Entry;

  // Add in synthesized ivar count if laying out an implementation.
  if (Impl) {
    unsigned SynthCount = CountNonClassIvars(D);
    // If there aren't any sythesized ivars then reuse the interface
    // entry. Note we can't cache this because we simply free all
    // entries later; however we shouldn't look up implementations
    // frequently.
    if (SynthCount == 0)
      return getObjCLayout(D, nullptr);
  }

  RecordLayoutBuilder Builder(*this, /*EmptySubobjects=*/nullptr);
  Builder.Layout(D);

  const ASTRecordLayout *NewEntry =
    new (*this) ASTRecordLayout(*this, Builder.getSize(), 
                                Builder.Alignment,
                                /*RequiredAlignment : used by MS-ABI)*/
                                Builder.Alignment,
                                Builder.getDataSize(),
                                Builder.FieldOffsets.data(),
                                Builder.FieldOffsets.size());

  ObjCLayouts[Key] = NewEntry;

  return *NewEntry;
}

static void PrintOffset(raw_ostream &OS,
                        CharUnits Offset, unsigned IndentLevel) {
  OS << llvm::format("%4" PRId64 " | ", (int64_t)Offset.getQuantity());
  OS.indent(IndentLevel * 2);
}

static void PrintIndentNoOffset(raw_ostream &OS, unsigned IndentLevel) {
  OS << "     | ";
  OS.indent(IndentLevel * 2);
}

static void DumpCXXRecordLayout(raw_ostream &OS,
                                const CXXRecordDecl *RD, const ASTContext &C,
                                CharUnits Offset,
                                unsigned IndentLevel,
                                const char* Description,
                                bool IncludeVirtualBases) {
  const ASTRecordLayout &Layout = C.getASTRecordLayout(RD);

  PrintOffset(OS, Offset, IndentLevel);
  OS << C.getTypeDeclType(const_cast<CXXRecordDecl *>(RD)).getAsString();
  if (Description)
    OS << ' ' << Description;
  if (RD->isEmpty())
    OS << " (empty)";
  OS << '\n';

  IndentLevel++;

  const CXXRecordDecl *PrimaryBase = Layout.getPrimaryBase();
  bool HasOwnVFPtr = Layout.hasOwnVFPtr();
  bool HasOwnVBPtr = Layout.hasOwnVBPtr();

  // Vtable pointer.
  if (RD->isDynamicClass() && !PrimaryBase && !isMsLayout(RD)) {
    PrintOffset(OS, Offset, IndentLevel);
    OS << '(' << *RD << " vtable pointer)\n";
  } else if (HasOwnVFPtr) {
    PrintOffset(OS, Offset, IndentLevel);
    // vfptr (for Microsoft C++ ABI)
    OS << '(' << *RD << " vftable pointer)\n";
  }

  // Collect nvbases.
  SmallVector<const CXXRecordDecl *, 4> Bases;
  for (const CXXBaseSpecifier &Base : RD->bases()) {
    assert(!Base.getType()->isDependentType() &&
           "Cannot layout class with dependent bases.");
    if (!Base.isVirtual())
      Bases.push_back(Base.getType()->getAsCXXRecordDecl());
  }

  // Sort nvbases by offset.
  std::stable_sort(Bases.begin(), Bases.end(),
                   [&](const CXXRecordDecl *L, const CXXRecordDecl *R) {
    return Layout.getBaseClassOffset(L) < Layout.getBaseClassOffset(R);
  });

  // Dump (non-virtual) bases
  for (const CXXRecordDecl *Base : Bases) {
    CharUnits BaseOffset = Offset + Layout.getBaseClassOffset(Base);
    DumpCXXRecordLayout(OS, Base, C, BaseOffset, IndentLevel,
                        Base == PrimaryBase ? "(primary base)" : "(base)",
                        /*IncludeVirtualBases=*/false);
  }

  // vbptr (for Microsoft C++ ABI)
  if (HasOwnVBPtr) {
    PrintOffset(OS, Offset + Layout.getVBPtrOffset(), IndentLevel);
    OS << '(' << *RD << " vbtable pointer)\n";
  }

  // Dump fields.
  uint64_t FieldNo = 0;
  for (CXXRecordDecl::field_iterator I = RD->field_begin(),
         E = RD->field_end(); I != E; ++I, ++FieldNo) {
    const FieldDecl &Field = **I;
    CharUnits FieldOffset = Offset + 
      C.toCharUnitsFromBits(Layout.getFieldOffset(FieldNo));

    if (const CXXRecordDecl *D = Field.getType()->getAsCXXRecordDecl()) {
      DumpCXXRecordLayout(OS, D, C, FieldOffset, IndentLevel,
                          Field.getName().data(),
                          /*IncludeVirtualBases=*/true);
      continue;
    }

    PrintOffset(OS, FieldOffset, IndentLevel);
    OS << Field.getType().getAsString() << ' ' << Field << '\n';
  }

  if (!IncludeVirtualBases)
    return;

  // Dump virtual bases.
  const ASTRecordLayout::VBaseOffsetsMapTy &vtordisps = 
    Layout.getVBaseOffsetsMap();
  for (const CXXBaseSpecifier &Base : RD->vbases()) {
    assert(Base.isVirtual() && "Found non-virtual class!");
    const CXXRecordDecl *VBase = Base.getType()->getAsCXXRecordDecl();

    CharUnits VBaseOffset = Offset + Layout.getVBaseClassOffset(VBase);

    if (vtordisps.find(VBase)->second.hasVtorDisp()) {
      PrintOffset(OS, VBaseOffset - CharUnits::fromQuantity(4), IndentLevel);
      OS << "(vtordisp for vbase " << *VBase << ")\n";
    }

    DumpCXXRecordLayout(OS, VBase, C, VBaseOffset, IndentLevel,
                        VBase == PrimaryBase ?
                        "(primary virtual base)" : "(virtual base)",
                        /*IncludeVirtualBases=*/false);
  }

  PrintIndentNoOffset(OS, IndentLevel - 1);
  OS << "[sizeof=" << Layout.getSize().getQuantity();
  if (!isMsLayout(RD))
    OS << ", dsize=" << Layout.getDataSize().getQuantity();
  OS << ", align=" << Layout.getAlignment().getQuantity() << '\n';

  PrintIndentNoOffset(OS, IndentLevel - 1);
  OS << " nvsize=" << Layout.getNonVirtualSize().getQuantity();
  OS << ", nvalign=" << Layout.getNonVirtualAlignment().getQuantity() << "]\n";
}

void ASTContext::DumpRecordLayout(const RecordDecl *RD,
                                  raw_ostream &OS,
                                  bool Simple) const {
  const ASTRecordLayout &Info = getASTRecordLayout(RD);

  if (const CXXRecordDecl *CXXRD = dyn_cast<CXXRecordDecl>(RD))
    if (!Simple)
      return DumpCXXRecordLayout(OS, CXXRD, *this, CharUnits(), 0, nullptr,
                                 /*IncludeVirtualBases=*/true);

  OS << "Type: " << getTypeDeclType(RD).getAsString() << "\n";
  if (!Simple) {
    OS << "Record: ";
    RD->dump();
  }
  OS << "\nLayout: ";
  OS << "<ASTRecordLayout\n";
  OS << "  Size:" << toBits(Info.getSize()) << "\n";
  if (!isMsLayout(RD))
    OS << "  DataSize:" << toBits(Info.getDataSize()) << "\n";
  OS << "  Alignment:" << toBits(Info.getAlignment()) << "\n";
  OS << "  FieldOffsets: [";
  for (unsigned i = 0, e = Info.getFieldCount(); i != e; ++i) {
    if (i) OS << ", ";
    OS << Info.getFieldOffset(i);
  }
  OS << "]>\n";
}
