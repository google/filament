//===--- VTableBuilder.h - C++ vtable layout builder --------------*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code dealing with generation of the layout of virtual tables.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_VTABLEBUILDER_H
#define LLVM_CLANG_AST_VTABLEBUILDER_H

#include "clang/AST/BaseSubobject.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/GlobalDecl.h"
#include "clang/AST/RecordLayout.h"
#include "clang/Basic/ABI.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include <memory>
#include <utility>

namespace clang {
  class CXXRecordDecl;

/// \brief Represents a single component in a vtable.
class VTableComponent {
public:
  enum Kind {
    CK_VCallOffset,
    CK_VBaseOffset,
    CK_OffsetToTop,
    CK_RTTI,
    CK_FunctionPointer,

    /// \brief A pointer to the complete destructor.
    CK_CompleteDtorPointer,

    /// \brief A pointer to the deleting destructor.
    CK_DeletingDtorPointer,

    /// \brief An entry that is never used.
    ///
    /// In some cases, a vtable function pointer will end up never being
    /// called. Such vtable function pointers are represented as a
    /// CK_UnusedFunctionPointer.
    CK_UnusedFunctionPointer
  };

  VTableComponent() { }

  static VTableComponent MakeVCallOffset(CharUnits Offset) {
    return VTableComponent(CK_VCallOffset, Offset);
  }

  static VTableComponent MakeVBaseOffset(CharUnits Offset) {
    return VTableComponent(CK_VBaseOffset, Offset);
  }

  static VTableComponent MakeOffsetToTop(CharUnits Offset) {
    return VTableComponent(CK_OffsetToTop, Offset);
  }

  static VTableComponent MakeRTTI(const CXXRecordDecl *RD) {
    return VTableComponent(CK_RTTI, reinterpret_cast<uintptr_t>(RD));
  }

  static VTableComponent MakeFunction(const CXXMethodDecl *MD) {
    assert(!isa<CXXDestructorDecl>(MD) &&
           "Don't use MakeFunction with destructors!");

    return VTableComponent(CK_FunctionPointer,
                           reinterpret_cast<uintptr_t>(MD));
  }

  static VTableComponent MakeCompleteDtor(const CXXDestructorDecl *DD) {
    return VTableComponent(CK_CompleteDtorPointer,
                           reinterpret_cast<uintptr_t>(DD));
  }

  static VTableComponent MakeDeletingDtor(const CXXDestructorDecl *DD) {
    return VTableComponent(CK_DeletingDtorPointer,
                           reinterpret_cast<uintptr_t>(DD));
  }

  static VTableComponent MakeUnusedFunction(const CXXMethodDecl *MD) {
    assert(!isa<CXXDestructorDecl>(MD) &&
           "Don't use MakeUnusedFunction with destructors!");
    return VTableComponent(CK_UnusedFunctionPointer,
                           reinterpret_cast<uintptr_t>(MD));
  }

  static VTableComponent getFromOpaqueInteger(uint64_t I) {
    return VTableComponent(I);
  }

  /// \brief Get the kind of this vtable component.
  Kind getKind() const {
    return (Kind)(Value & 0x7);
  }

  CharUnits getVCallOffset() const {
    assert(getKind() == CK_VCallOffset && "Invalid component kind!");

    return getOffset();
  }

  CharUnits getVBaseOffset() const {
    assert(getKind() == CK_VBaseOffset && "Invalid component kind!");

    return getOffset();
  }

  CharUnits getOffsetToTop() const {
    assert(getKind() == CK_OffsetToTop && "Invalid component kind!");

    return getOffset();
  }

  const CXXRecordDecl *getRTTIDecl() const {
    assert(getKind() == CK_RTTI && "Invalid component kind!");

    return reinterpret_cast<CXXRecordDecl *>(getPointer());
  }

  const CXXMethodDecl *getFunctionDecl() const {
    assert(getKind() == CK_FunctionPointer);

    return reinterpret_cast<CXXMethodDecl *>(getPointer());
  }

  const CXXDestructorDecl *getDestructorDecl() const {
    assert((getKind() == CK_CompleteDtorPointer ||
            getKind() == CK_DeletingDtorPointer) && "Invalid component kind!");

    return reinterpret_cast<CXXDestructorDecl *>(getPointer());
  }

  const CXXMethodDecl *getUnusedFunctionDecl() const {
    assert(getKind() == CK_UnusedFunctionPointer);

    return reinterpret_cast<CXXMethodDecl *>(getPointer());
  }

private:
  VTableComponent(Kind ComponentKind, CharUnits Offset) {
    assert((ComponentKind == CK_VCallOffset ||
            ComponentKind == CK_VBaseOffset ||
            ComponentKind == CK_OffsetToTop) && "Invalid component kind!");
    assert(Offset.getQuantity() < (1LL << 56) && "Offset is too big!");
    assert(Offset.getQuantity() >= -(1LL << 56) && "Offset is too small!");

    Value = (uint64_t(Offset.getQuantity()) << 3) | ComponentKind;
  }

  VTableComponent(Kind ComponentKind, uintptr_t Ptr) {
    assert((ComponentKind == CK_RTTI ||
            ComponentKind == CK_FunctionPointer ||
            ComponentKind == CK_CompleteDtorPointer ||
            ComponentKind == CK_DeletingDtorPointer ||
            ComponentKind == CK_UnusedFunctionPointer) &&
            "Invalid component kind!");

    assert((Ptr & 7) == 0 && "Pointer not sufficiently aligned!");

    Value = Ptr | ComponentKind;
  }

  CharUnits getOffset() const {
    assert((getKind() == CK_VCallOffset || getKind() == CK_VBaseOffset ||
            getKind() == CK_OffsetToTop) && "Invalid component kind!");

    return CharUnits::fromQuantity(Value >> 3);
  }

  uintptr_t getPointer() const {
    assert((getKind() == CK_RTTI ||
            getKind() == CK_FunctionPointer ||
            getKind() == CK_CompleteDtorPointer ||
            getKind() == CK_DeletingDtorPointer ||
            getKind() == CK_UnusedFunctionPointer) &&
           "Invalid component kind!");

    return static_cast<uintptr_t>(Value & ~7ULL);
  }

  explicit VTableComponent(uint64_t Value)
    : Value(Value) { }

  /// The kind is stored in the lower 3 bits of the value. For offsets, we
  /// make use of the facts that classes can't be larger than 2^55 bytes,
  /// so we store the offset in the lower part of the 61 bits that remain.
  /// (The reason that we're not simply using a PointerIntPair here is that we
  /// need the offsets to be 64-bit, even when on a 32-bit machine).
  int64_t Value;
};

class VTableLayout {
public:
  typedef std::pair<uint64_t, ThunkInfo> VTableThunkTy;

  typedef const VTableComponent *vtable_component_iterator;
  typedef const VTableThunkTy *vtable_thunk_iterator;

  typedef llvm::DenseMap<BaseSubobject, uint64_t> AddressPointsMapTy;
private:
  uint64_t NumVTableComponents;
  std::unique_ptr<VTableComponent[]> VTableComponents;

  /// \brief Contains thunks needed by vtables, sorted by indices.
  uint64_t NumVTableThunks;
  std::unique_ptr<VTableThunkTy[]> VTableThunks;

  /// \brief Address points for all vtables.
  AddressPointsMapTy AddressPoints;

  bool IsMicrosoftABI;

public:
  VTableLayout(uint64_t NumVTableComponents,
               const VTableComponent *VTableComponents,
               uint64_t NumVTableThunks,
               const VTableThunkTy *VTableThunks,
               const AddressPointsMapTy &AddressPoints,
               bool IsMicrosoftABI);
  ~VTableLayout();

  uint64_t getNumVTableComponents() const {
    return NumVTableComponents;
  }

  vtable_component_iterator vtable_component_begin() const {
    return VTableComponents.get();
  }

  vtable_component_iterator vtable_component_end() const {
    return VTableComponents.get() + NumVTableComponents;
  }

  uint64_t getNumVTableThunks() const { return NumVTableThunks; }

  vtable_thunk_iterator vtable_thunk_begin() const {
    return VTableThunks.get();
  }

  vtable_thunk_iterator vtable_thunk_end() const {
    return VTableThunks.get() + NumVTableThunks;
  }

  uint64_t getAddressPoint(BaseSubobject Base) const {
    assert(AddressPoints.count(Base) &&
           "Did not find address point!");

    uint64_t AddressPoint = AddressPoints.lookup(Base);
    assert(AddressPoint != 0 || IsMicrosoftABI);
    (void)IsMicrosoftABI;

    return AddressPoint;
  }

  const AddressPointsMapTy &getAddressPoints() const {
    return AddressPoints;
  }
};

class VTableContextBase {
public:
  typedef SmallVector<ThunkInfo, 1> ThunkInfoVectorTy;

  bool isMicrosoft() const { return IsMicrosoftABI; }

  virtual ~VTableContextBase() {}

protected:
  typedef llvm::DenseMap<const CXXMethodDecl *, ThunkInfoVectorTy> ThunksMapTy;

  /// \brief Contains all thunks that a given method decl will need.
  ThunksMapTy Thunks;

  /// Compute and store all vtable related information (vtable layout, vbase
  /// offset offsets, thunks etc) for the given record decl.
  virtual void computeVTableRelatedInformation(const CXXRecordDecl *RD) = 0;

  VTableContextBase(bool MS) : IsMicrosoftABI(MS) {}

public:
  virtual const ThunkInfoVectorTy *getThunkInfo(GlobalDecl GD) {
    const CXXMethodDecl *MD = cast<CXXMethodDecl>(GD.getDecl()->getCanonicalDecl());
    computeVTableRelatedInformation(MD->getParent());

    // This assumes that all the destructors present in the vtable
    // use exactly the same set of thunks.
    ThunksMapTy::const_iterator I = Thunks.find(MD);
    if (I == Thunks.end()) {
      // We did not find a thunk for this method.
      return nullptr;
    }

    return &I->second;
  }

  bool IsMicrosoftABI;
};

class ItaniumVTableContext : public VTableContextBase {
private:

  /// \brief Contains the index (relative to the vtable address point)
  /// where the function pointer for a virtual function is stored.
  typedef llvm::DenseMap<GlobalDecl, int64_t> MethodVTableIndicesTy;
  MethodVTableIndicesTy MethodVTableIndices;

  typedef llvm::DenseMap<const CXXRecordDecl *, const VTableLayout *>
    VTableLayoutMapTy;
  VTableLayoutMapTy VTableLayouts;

  typedef std::pair<const CXXRecordDecl *,
                    const CXXRecordDecl *> ClassPairTy;

  /// \brief vtable offsets for offsets of virtual bases of a class.
  ///
  /// Contains the vtable offset (relative to the address point) in chars
  /// where the offsets for virtual bases of a class are stored.
  typedef llvm::DenseMap<ClassPairTy, CharUnits>
    VirtualBaseClassOffsetOffsetsMapTy;
  VirtualBaseClassOffsetOffsetsMapTy VirtualBaseClassOffsetOffsets;

  void computeVTableRelatedInformation(const CXXRecordDecl *RD) override;

public:
  ItaniumVTableContext(ASTContext &Context);
  ~ItaniumVTableContext() override;

  const VTableLayout &getVTableLayout(const CXXRecordDecl *RD) {
    computeVTableRelatedInformation(RD);
    assert(VTableLayouts.count(RD) && "No layout for this record decl!");

    return *VTableLayouts[RD];
  }

  VTableLayout *
  createConstructionVTableLayout(const CXXRecordDecl *MostDerivedClass,
                                 CharUnits MostDerivedClassOffset,
                                 bool MostDerivedClassIsVirtual,
                                 const CXXRecordDecl *LayoutClass);

  /// \brief Locate a virtual function in the vtable.
  ///
  /// Return the index (relative to the vtable address point) where the
  /// function pointer for the given virtual function is stored.
  uint64_t getMethodVTableIndex(GlobalDecl GD);

  /// Return the offset in chars (relative to the vtable address point) where
  /// the offset of the virtual base that contains the given base is stored,
  /// otherwise, if no virtual base contains the given class, return 0. 
  ///
  /// Base must be a virtual base class or an unambiguous base.
  CharUnits getVirtualBaseOffsetOffset(const CXXRecordDecl *RD,
                                       const CXXRecordDecl *VBase);

  static bool classof(const VTableContextBase *VT) {
    return !VT->isMicrosoft();
  }
};

/// Holds information about the inheritance path to a virtual base or function
/// table pointer.  A record may contain as many vfptrs or vbptrs as there are
/// base subobjects.
struct VPtrInfo {
  typedef SmallVector<const CXXRecordDecl *, 1> BasePath;

  VPtrInfo(const CXXRecordDecl *RD)
      : ReusingBase(RD), BaseWithVPtr(RD), NextBaseToMangle(RD) {}

  // Copy constructor.
  // FIXME: Uncomment when we've moved to C++11.
  // VPtrInfo(const VPtrInfo &) = default;

  /// The vtable will hold all of the virtual bases or virtual methods of
  /// ReusingBase.  This may or may not be the same class as VPtrSubobject.Base.
  /// A derived class will reuse the vptr of the first non-virtual base
  /// subobject that has one.
  const CXXRecordDecl *ReusingBase;

  /// BaseWithVPtr is at this offset from its containing complete object or
  /// virtual base.
  CharUnits NonVirtualOffset;

  /// The vptr is stored inside this subobject.
  const CXXRecordDecl *BaseWithVPtr;

  /// The bases from the inheritance path that got used to mangle the vbtable
  /// name.  This is not really a full path like a CXXBasePath.  It holds the
  /// subset of records that need to be mangled into the vbtable symbol name in
  /// order to get a unique name.
  BasePath MangledPath;

  /// The next base to push onto the mangled path if this path is ambiguous in a
  /// derived class.  If it's null, then it's already been pushed onto the path.
  const CXXRecordDecl *NextBaseToMangle;

  /// The set of possibly indirect vbases that contain this vbtable.  When a
  /// derived class indirectly inherits from the same vbase twice, we only keep
  /// vtables and their paths from the first instance.
  BasePath ContainingVBases;

  /// This holds the base classes path from the complete type to the first base
  /// with the given vfptr offset, in the base-to-derived order.  Only used for
  /// vftables.
  BasePath PathToBaseWithVPtr;

  /// Static offset from the top of the most derived class to this vfptr,
  /// including any virtual base offset.  Only used for vftables.
  CharUnits FullOffsetInMDC;

  /// The vptr is stored inside the non-virtual component of this virtual base.
  const CXXRecordDecl *getVBaseWithVPtr() const {
    return ContainingVBases.empty() ? nullptr : ContainingVBases.front();
  }
};

typedef SmallVector<VPtrInfo *, 2> VPtrInfoVector;

/// All virtual base related information about a given record decl.  Includes
/// information on all virtual base tables and the path components that are used
/// to mangle them.
struct VirtualBaseInfo {
  ~VirtualBaseInfo() { llvm::DeleteContainerPointers(VBPtrPaths); }

  /// A map from virtual base to vbtable index for doing a conversion from the
  /// the derived class to the a base.
  llvm::DenseMap<const CXXRecordDecl *, unsigned> VBTableIndices;

  /// Information on all virtual base tables used when this record is the most
  /// derived class.
  VPtrInfoVector VBPtrPaths;
};

class MicrosoftVTableContext : public VTableContextBase {
public:
  struct MethodVFTableLocation {
    /// If nonzero, holds the vbtable index of the virtual base with the vfptr.
    uint64_t VBTableIndex;

    /// If nonnull, holds the last vbase which contains the vfptr that the
    /// method definition is adjusted to.
    const CXXRecordDecl *VBase;

    /// This is the offset of the vfptr from the start of the last vbase, or the
    /// complete type if there are no virtual bases.
    CharUnits VFPtrOffset;

    /// Method's index in the vftable.
    uint64_t Index;

    MethodVFTableLocation()
        : VBTableIndex(0), VBase(nullptr), VFPtrOffset(CharUnits::Zero()),
          Index(0) {}

    MethodVFTableLocation(uint64_t VBTableIndex, const CXXRecordDecl *VBase,
                          CharUnits VFPtrOffset, uint64_t Index)
        : VBTableIndex(VBTableIndex), VBase(VBase),
          VFPtrOffset(VFPtrOffset), Index(Index) {}

    bool operator<(const MethodVFTableLocation &other) const {
      if (VBTableIndex != other.VBTableIndex) {
        assert(VBase != other.VBase);
        return VBTableIndex < other.VBTableIndex;
      }
      return std::tie(VFPtrOffset, Index) <
             std::tie(other.VFPtrOffset, other.Index);
    }
  };

private:
  ASTContext &Context;

  typedef llvm::DenseMap<GlobalDecl, MethodVFTableLocation>
    MethodVFTableLocationsTy;
  MethodVFTableLocationsTy MethodVFTableLocations;

  typedef llvm::DenseMap<const CXXRecordDecl *, VPtrInfoVector *>
    VFPtrLocationsMapTy;
  VFPtrLocationsMapTy VFPtrLocations;

  typedef std::pair<const CXXRecordDecl *, CharUnits> VFTableIdTy;
  typedef llvm::DenseMap<VFTableIdTy, const VTableLayout *> VFTableLayoutMapTy;
  VFTableLayoutMapTy VFTableLayouts;

  llvm::DenseMap<const CXXRecordDecl *, VirtualBaseInfo *> VBaseInfo;

  void enumerateVFPtrs(const CXXRecordDecl *ForClass, VPtrInfoVector &Result);

  void computeVTableRelatedInformation(const CXXRecordDecl *RD) override;

  void dumpMethodLocations(const CXXRecordDecl *RD,
                           const MethodVFTableLocationsTy &NewMethods,
                           raw_ostream &);

  const VirtualBaseInfo *
  computeVBTableRelatedInformation(const CXXRecordDecl *RD);

  void computeVTablePaths(bool ForVBTables, const CXXRecordDecl *RD,
                          VPtrInfoVector &Paths);

public:
  MicrosoftVTableContext(ASTContext &Context)
      : VTableContextBase(/*MS=*/true), Context(Context) {}

  ~MicrosoftVTableContext() override;

  const VPtrInfoVector &getVFPtrOffsets(const CXXRecordDecl *RD);

  const VTableLayout &getVFTableLayout(const CXXRecordDecl *RD,
                                       CharUnits VFPtrOffset);

  const MethodVFTableLocation &getMethodVFTableLocation(GlobalDecl GD);

  const ThunkInfoVectorTy *getThunkInfo(GlobalDecl GD) override {
    // Complete destructors don't have a slot in a vftable, so no thunks needed.
    if (isa<CXXDestructorDecl>(GD.getDecl()) &&
        GD.getDtorType() == Dtor_Complete)
      return nullptr;
    return VTableContextBase::getThunkInfo(GD);
  }

  /// \brief Returns the index of VBase in the vbtable of Derived.
  /// VBase must be a morally virtual base of Derived.
  /// The vbtable is an array of i32 offsets.  The first entry is a self entry,
  /// and the rest are offsets from the vbptr to virtual bases.
  unsigned getVBTableIndex(const CXXRecordDecl *Derived,
                           const CXXRecordDecl *VBase);

  const VPtrInfoVector &enumerateVBTables(const CXXRecordDecl *RD);

  static bool classof(const VTableContextBase *VT) { return VT->isMicrosoft(); }
};

} // namespace clang

#endif
