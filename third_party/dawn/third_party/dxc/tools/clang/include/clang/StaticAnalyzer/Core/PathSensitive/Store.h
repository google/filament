//== Store.h - Interface for maps from Locations to Values ------*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defined the types Store and StoreManager.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_STORE_H
#define LLVM_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_STORE_H

#include "clang/StaticAnalyzer/Core/PathSensitive/MemRegion.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/SValBuilder.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/StoreRef.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/Optional.h"

namespace clang {

class Stmt;
class Expr;
class ObjCIvarDecl;
class CXXBasePath;
class StackFrameContext;

namespace ento {

class CallEvent;
class ProgramState;
class ProgramStateManager;
class ScanReachableSymbols;

typedef llvm::DenseSet<SymbolRef> InvalidatedSymbols;

class StoreManager {
protected:
  SValBuilder &svalBuilder;
  ProgramStateManager &StateMgr;

  /// MRMgr - Manages region objects associated with this StoreManager.
  MemRegionManager &MRMgr;
  ASTContext &Ctx;

  StoreManager(ProgramStateManager &stateMgr);

public:
  virtual ~StoreManager() {}

  /// Return the value bound to specified location in a given state.
  /// \param[in] store The analysis state.
  /// \param[in] loc The symbolic memory location.
  /// \param[in] T An optional type that provides a hint indicating the
  ///   expected type of the returned value.  This is used if the value is
  ///   lazily computed.
  /// \return The value bound to the location \c loc.
  virtual SVal getBinding(Store store, Loc loc, QualType T = QualType()) = 0;

  /// Return a state with the specified value bound to the given location.
  /// \param[in] store The analysis state.
  /// \param[in] loc The symbolic memory location.
  /// \param[in] val The value to bind to location \c loc.
  /// \return A pointer to a ProgramState object that contains the same
  ///   bindings as \c state with the addition of having the value specified
  ///   by \c val bound to the location given for \c loc.
  virtual StoreRef Bind(Store store, Loc loc, SVal val) = 0;

  virtual StoreRef BindDefault(Store store, const MemRegion *R, SVal V);

  /// \brief Create a new store with the specified binding removed.
  /// \param ST the original store, that is the basis for the new store.
  /// \param L the location whose binding should be removed.
  virtual StoreRef killBinding(Store ST, Loc L) = 0;

  /// getInitialStore - Returns the initial "empty" store representing the
  ///  value bindings upon entry to an analyzed function.
  virtual StoreRef getInitialStore(const LocationContext *InitLoc) = 0;

  /// getRegionManager - Returns the internal RegionManager object that is
  ///  used to query and manipulate MemRegion objects.
  MemRegionManager& getRegionManager() { return MRMgr; }

  virtual Loc getLValueVar(const VarDecl *VD, const LocationContext *LC) {
    return svalBuilder.makeLoc(MRMgr.getVarRegion(VD, LC));
  }

  Loc getLValueCompoundLiteral(const CompoundLiteralExpr *CL,
                               const LocationContext *LC) {
    return loc::MemRegionVal(MRMgr.getCompoundLiteralRegion(CL, LC));
  }

  virtual SVal getLValueIvar(const ObjCIvarDecl *decl, SVal base);

  virtual SVal getLValueField(const FieldDecl *D, SVal Base) {
    return getLValueFieldOrIvar(D, Base);
  }

  virtual SVal getLValueElement(QualType elementType, NonLoc offset, SVal Base);

  // FIXME: This should soon be eliminated altogether; clients should deal with
  // region extents directly.
  virtual DefinedOrUnknownSVal getSizeInElements(ProgramStateRef state, 
                                                 const MemRegion *region,
                                                 QualType EleTy) {
    return UnknownVal();
  }

  /// ArrayToPointer - Used by ExprEngine::VistCast to handle implicit
  ///  conversions between arrays and pointers.
  virtual SVal ArrayToPointer(Loc Array, QualType ElementTy) = 0;

  /// Evaluates a chain of derived-to-base casts through the path specified in
  /// \p Cast.
  SVal evalDerivedToBase(SVal Derived, const CastExpr *Cast);

  /// Evaluates a chain of derived-to-base casts through the specified path.
  SVal evalDerivedToBase(SVal Derived, const CXXBasePath &CastPath);

  /// Evaluates a derived-to-base cast through a single level of derivation.
  SVal evalDerivedToBase(SVal Derived, QualType DerivedPtrType,
                         bool IsVirtual);

  /// \brief Evaluates C++ dynamic_cast cast.
  /// The callback may result in the following 3 scenarios:
  ///  - Successful cast (ex: derived is subclass of base).
  ///  - Failed cast (ex: derived is definitely not a subclass of base).
  ///  - We don't know (base is a symbolic region and we don't have 
  ///    enough info to determine if the cast will succeed at run time).
  /// The function returns an SVal representing the derived class; it's
  /// valid only if Failed flag is set to false.
  SVal evalDynamicCast(SVal Base, QualType DerivedPtrType, bool &Failed);

  const ElementRegion *GetElementZeroRegion(const MemRegion *R, QualType T);

  /// castRegion - Used by ExprEngine::VisitCast to handle casts from
  ///  a MemRegion* to a specific location type.  'R' is the region being
  ///  casted and 'CastToTy' the result type of the cast.
  const MemRegion *castRegion(const MemRegion *region, QualType CastToTy);

  virtual StoreRef removeDeadBindings(Store store, const StackFrameContext *LCtx,
                                      SymbolReaper& SymReaper) = 0;

  virtual bool includedInBindings(Store store,
                                  const MemRegion *region) const = 0;
  
  /// If the StoreManager supports it, increment the reference count of
  /// the specified Store object.
  virtual void incrementReferenceCount(Store store) {}

  /// If the StoreManager supports it, decrement the reference count of
  /// the specified Store object.  If the reference count hits 0, the memory
  /// associated with the object is recycled.
  virtual void decrementReferenceCount(Store store) {}

  typedef SmallVector<const MemRegion *, 8> InvalidatedRegions;

  /// invalidateRegions - Clears out the specified regions from the store,
  ///  marking their values as unknown. Depending on the store, this may also
  ///  invalidate additional regions that may have changed based on accessing
  ///  the given regions. Optionally, invalidates non-static globals as well.
  /// \param[in] store The initial store
  /// \param[in] Values The values to invalidate.
  /// \param[in] E The current statement being evaluated. Used to conjure
  ///   symbols to mark the values of invalidated regions.
  /// \param[in] Count The current block count. Used to conjure
  ///   symbols to mark the values of invalidated regions.
  /// \param[in] Call The call expression which will be used to determine which
  ///   globals should get invalidated.
  /// \param[in,out] IS A set to fill with any symbols that are no longer
  ///   accessible. Pass \c NULL if this information will not be used.
  /// \param[in] ITraits Information about invalidation for a particular 
  ///   region/symbol.
  /// \param[in,out] InvalidatedTopLevel A vector to fill with regions
  ////  explicitly being invalidated. Pass \c NULL if this
  ///   information will not be used.
  /// \param[in,out] Invalidated A vector to fill with any regions being
  ///   invalidated. This should include any regions explicitly invalidated
  ///   even if they do not currently have bindings. Pass \c NULL if this
  ///   information will not be used.
  virtual StoreRef invalidateRegions(Store store,
                                  ArrayRef<SVal> Values,
                                  const Expr *E, unsigned Count,
                                  const LocationContext *LCtx,
                                  const CallEvent *Call,
                                  InvalidatedSymbols &IS,
                                  RegionAndSymbolInvalidationTraits &ITraits,
                                  InvalidatedRegions *InvalidatedTopLevel,
                                  InvalidatedRegions *Invalidated) = 0;

  /// enterStackFrame - Let the StoreManager to do something when execution
  /// engine is about to execute into a callee.
  StoreRef enterStackFrame(Store store,
                           const CallEvent &Call,
                           const StackFrameContext *CalleeCtx);

  /// Finds the transitive closure of symbols within the given region.
  ///
  /// Returns false if the visitor aborted the scan.
  virtual bool scanReachableSymbols(Store S, const MemRegion *R,
                                    ScanReachableSymbols &Visitor) = 0;

  virtual void print(Store store, raw_ostream &Out,
                     const char* nl, const char *sep) = 0;

  class BindingsHandler {
  public:
    virtual ~BindingsHandler();
    virtual bool HandleBinding(StoreManager& SMgr, Store store,
                               const MemRegion *region, SVal val) = 0;
  };

  class FindUniqueBinding :
  public BindingsHandler {
    SymbolRef Sym;
    const MemRegion* Binding;
    bool First;

  public:
    FindUniqueBinding(SymbolRef sym)
      : Sym(sym), Binding(nullptr), First(true) {}

    bool HandleBinding(StoreManager& SMgr, Store store, const MemRegion* R,
                       SVal val) override;
    explicit operator bool() { return First && Binding; }
    const MemRegion *getRegion() { return Binding; }
  };

  /// iterBindings - Iterate over the bindings in the Store.
  virtual void iterBindings(Store store, BindingsHandler& f) = 0;

protected:
  const MemRegion *MakeElementRegion(const MemRegion *baseRegion,
                                     QualType pointeeTy, uint64_t index = 0);

  /// CastRetrievedVal - Used by subclasses of StoreManager to implement
  ///  implicit casts that arise from loads from regions that are reinterpreted
  ///  as another region.
  SVal CastRetrievedVal(SVal val, const TypedValueRegion *region, 
                        QualType castTy, bool performTestOnly = true);

private:
  SVal getLValueFieldOrIvar(const Decl *decl, SVal base);
};


inline StoreRef::StoreRef(Store store, StoreManager & smgr)
  : store(store), mgr(smgr) {
  if (store)
    mgr.incrementReferenceCount(store);
}

inline StoreRef::StoreRef(const StoreRef &sr) 
  : store(sr.store), mgr(sr.mgr)
{ 
  if (store)
    mgr.incrementReferenceCount(store);
}
  
inline StoreRef::~StoreRef() {
  if (store)
    mgr.decrementReferenceCount(store);
}
  
inline StoreRef &StoreRef::operator=(StoreRef const &newStore) {
  assert(&newStore.mgr == &mgr);
  if (store != newStore.store) {
    mgr.incrementReferenceCount(newStore.store);
    mgr.decrementReferenceCount(store);
    store = newStore.getStore();
  }
  return *this;
}

// FIXME: Do we need to pass ProgramStateManager anymore?
std::unique_ptr<StoreManager>
CreateRegionStoreManager(ProgramStateManager &StMgr);
std::unique_ptr<StoreManager>
CreateFieldsOnlyRegionStoreManager(ProgramStateManager &StMgr);

} // end GR namespace

} // end clang namespace

#endif
