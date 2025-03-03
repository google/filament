//===--- ExternalASTSource.h - Abstract External AST Interface --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the ExternalASTSource interface, which enables
//  construction of AST nodes from some external source.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_AST_EXTERNALASTSOURCE_H
#define LLVM_CLANG_AST_EXTERNALASTSOURCE_H

#include "clang/AST/CharUnits.h"
#include "clang/AST/DeclBase.h"
#include "llvm/ADT/DenseMap.h"

namespace clang {

class ASTConsumer;
class CXXBaseSpecifier;
class CXXCtorInitializer;
class DeclarationName;
class ExternalSemaSource; // layering violation required for downcasting
class FieldDecl;
class Module;
class NamedDecl;
class RecordDecl;
class Selector;
class Stmt;
class TagDecl;

/// \brief Enumeration describing the result of loading information from
/// an external source.
enum ExternalLoadResult {
  /// \brief Loading the external information has succeeded.
  ELR_Success,
  
  /// \brief Loading the external information has failed.
  ELR_Failure,
  
  /// \brief The external information has already been loaded, and therefore
  /// no additional processing is required.
  ELR_AlreadyLoaded
};

/// \brief Abstract interface for external sources of AST nodes.
///
/// External AST sources provide AST nodes constructed from some
/// external source, such as a precompiled header. External AST
/// sources can resolve types and declarations from abstract IDs into
/// actual type and declaration nodes, and read parts of declaration
/// contexts.
class ExternalASTSource : public RefCountedBase<ExternalASTSource> {
  /// Generation number for this external AST source. Must be increased
  /// whenever we might have added new redeclarations for existing decls.
  uint32_t CurrentGeneration;

  /// \brief Whether this AST source also provides information for
  /// semantic analysis.
  bool SemaSource;

  friend class ExternalSemaSource;

public:
  ExternalASTSource() : CurrentGeneration(0), SemaSource(false) { }

  virtual ~ExternalASTSource();

  /// \brief RAII class for safely pairing a StartedDeserializing call
  /// with FinishedDeserializing.
  class Deserializing {
    ExternalASTSource *Source;
  public:
    explicit Deserializing(ExternalASTSource *source) : Source(source) {
      assert(Source);
      Source->StartedDeserializing();
    }
    ~Deserializing() {
      Source->FinishedDeserializing();
    }
  };

  /// \brief Get the current generation of this AST source. This number
  /// is incremented each time the AST source lazily extends an existing
  /// entity.
  uint32_t getGeneration() const { return CurrentGeneration; }

  /// \brief Resolve a declaration ID into a declaration, potentially
  /// building a new declaration.
  ///
  /// This method only needs to be implemented if the AST source ever
  /// passes back decl sets as VisibleDeclaration objects.
  ///
  /// The default implementation of this method is a no-op.
  virtual Decl *GetExternalDecl(uint32_t ID);

  /// \brief Resolve a selector ID into a selector.
  ///
  /// This operation only needs to be implemented if the AST source
  /// returns non-zero for GetNumKnownSelectors().
  ///
  /// The default implementation of this method is a no-op.
  virtual Selector GetExternalSelector(uint32_t ID);

  /// \brief Returns the number of selectors known to the external AST
  /// source.
  ///
  /// The default implementation of this method is a no-op.
  virtual uint32_t GetNumExternalSelectors();

  /// \brief Resolve the offset of a statement in the decl stream into
  /// a statement.
  ///
  /// This operation is meant to be used via a LazyOffsetPtr.  It only
  /// needs to be implemented if the AST source uses methods like
  /// FunctionDecl::setLazyBody when building decls.
  ///
  /// The default implementation of this method is a no-op.
  virtual Stmt *GetExternalDeclStmt(uint64_t Offset);

  /// \brief Resolve the offset of a set of C++ constructor initializers in
  /// the decl stream into an array of initializers.
  ///
  /// The default implementation of this method is a no-op.
  virtual CXXCtorInitializer **GetExternalCXXCtorInitializers(uint64_t Offset);

  /// \brief Resolve the offset of a set of C++ base specifiers in the decl
  /// stream into an array of specifiers.
  ///
  /// The default implementation of this method is a no-op.
  virtual CXXBaseSpecifier *GetExternalCXXBaseSpecifiers(uint64_t Offset);

  /// \brief Update an out-of-date identifier.
  virtual void updateOutOfDateIdentifier(IdentifierInfo &II) { }

  /// \brief Find all declarations with the given name in the given context,
  /// and add them to the context by calling SetExternalVisibleDeclsForName
  /// or SetNoExternalVisibleDeclsForName.
  /// \return \c true if any declarations might have been found, \c false if
  /// we definitely have no declarations with tbis name.
  ///
  /// The default implementation of this method is a no-op returning \c false.
  virtual bool
  FindExternalVisibleDeclsByName(const DeclContext *DC, DeclarationName Name);

  /// \brief Ensures that the table of all visible declarations inside this
  /// context is up to date.
  ///
  /// The default implementation of this function is a no-op.
  virtual void completeVisibleDeclsMap(const DeclContext *DC);

  /// \brief Retrieve the module that corresponds to the given module ID.
  virtual Module *getModule(unsigned ID) { return nullptr; }

  /// \brief Holds everything needed to generate debug info for an
  /// imported module or precompiled header file.
  struct ASTSourceDescriptor {
    std::string ModuleName;
    std::string Path;
    std::string ASTFile;
    uint64_t Signature;
  };

  /// \brief Return a descriptor for the corresponding module, if one exists.
  virtual llvm::Optional<ASTSourceDescriptor> getSourceDescriptor(unsigned ID);
  /// \brief Return a descriptor for the module.
  virtual ASTSourceDescriptor getSourceDescriptor(const Module &M);

  /// \brief Finds all declarations lexically contained within the given
  /// DeclContext, after applying an optional filter predicate.
  ///
  /// \param isKindWeWant a predicate function that returns true if the passed
  /// declaration kind is one we are looking for. If NULL, all declarations
  /// are returned.
  ///
  /// \return an indication of whether the load succeeded or failed.
  ///
  /// The default implementation of this method is a no-op.
  virtual ExternalLoadResult FindExternalLexicalDecls(const DeclContext *DC,
                                        bool (*isKindWeWant)(Decl::Kind),
                                        SmallVectorImpl<Decl*> &Result);

  /// \brief Finds all declarations lexically contained within the given
  /// DeclContext.
  ///
  /// \return true if an error occurred
  ExternalLoadResult FindExternalLexicalDecls(const DeclContext *DC,
                                SmallVectorImpl<Decl*> &Result) {
    return FindExternalLexicalDecls(DC, nullptr, Result);
  }

  template <typename DeclTy>
  ExternalLoadResult FindExternalLexicalDeclsBy(const DeclContext *DC,
                                  SmallVectorImpl<Decl*> &Result) {
    return FindExternalLexicalDecls(DC, DeclTy::classofKind, Result);
  }

  /// \brief Get the decls that are contained in a file in the Offset/Length
  /// range. \p Length can be 0 to indicate a point at \p Offset instead of
  /// a range.
  virtual void FindFileRegionDecls(FileID File, unsigned Offset,
                                   unsigned Length,
                                   SmallVectorImpl<Decl *> &Decls);

  /// \brief Gives the external AST source an opportunity to complete
  /// the redeclaration chain for a declaration. Called each time we
  /// need the most recent declaration of a declaration after the
  /// generation count is incremented.
  virtual void CompleteRedeclChain(const Decl *D);

  /// \brief Gives the external AST source an opportunity to complete
  /// an incomplete type.
  virtual void CompleteType(TagDecl *Tag);

  /// \brief Gives the external AST source an opportunity to complete an
  /// incomplete Objective-C class.
  ///
  /// This routine will only be invoked if the "externally completed" bit is
  /// set on the ObjCInterfaceDecl via the function
  /// \c ObjCInterfaceDecl::setExternallyCompleted().
  virtual void CompleteType(ObjCInterfaceDecl *Class);

  /// \brief Loads comment ranges.
  virtual void ReadComments();

  /// \brief Notify ExternalASTSource that we started deserialization of
  /// a decl or type so until FinishedDeserializing is called there may be
  /// decls that are initializing. Must be paired with FinishedDeserializing.
  ///
  /// The default implementation of this method is a no-op.
  virtual void StartedDeserializing();

  /// \brief Notify ExternalASTSource that we finished the deserialization of
  /// a decl or type. Must be paired with StartedDeserializing.
  ///
  /// The default implementation of this method is a no-op.
  virtual void FinishedDeserializing();

  /// \brief Function that will be invoked when we begin parsing a new
  /// translation unit involving this external AST source.
  ///
  /// The default implementation of this method is a no-op.
  virtual void StartTranslationUnit(ASTConsumer *Consumer);

  /// \brief Print any statistics that have been gathered regarding
  /// the external AST source.
  ///
  /// The default implementation of this method is a no-op.
  virtual void PrintStats();
  
  
  /// \brief Perform layout on the given record.
  ///
  /// This routine allows the external AST source to provide an specific 
  /// layout for a record, overriding the layout that would normally be
  /// constructed. It is intended for clients who receive specific layout
  /// details rather than source code (such as LLDB). The client is expected
  /// to fill in the field offsets, base offsets, virtual base offsets, and
  /// complete object size.
  ///
  /// \param Record The record whose layout is being requested.
  ///
  /// \param Size The final size of the record, in bits.
  ///
  /// \param Alignment The final alignment of the record, in bits.
  ///
  /// \param FieldOffsets The offset of each of the fields within the record,
  /// expressed in bits. All of the fields must be provided with offsets.
  ///
  /// \param BaseOffsets The offset of each of the direct, non-virtual base
  /// classes. If any bases are not given offsets, the bases will be laid 
  /// out according to the ABI.
  ///
  /// \param VirtualBaseOffsets The offset of each of the virtual base classes
  /// (either direct or not). If any bases are not given offsets, the bases will be laid 
  /// out according to the ABI.
  /// 
  /// \returns true if the record layout was provided, false otherwise.
  virtual bool layoutRecordType(
      const RecordDecl *Record, uint64_t &Size, uint64_t &Alignment,
      llvm::DenseMap<const FieldDecl *, uint64_t> &FieldOffsets,
      llvm::DenseMap<const CXXRecordDecl *, CharUnits> &BaseOffsets,
      llvm::DenseMap<const CXXRecordDecl *, CharUnits> &VirtualBaseOffsets);

  //===--------------------------------------------------------------------===//
  // Queries for performance analysis.
  //===--------------------------------------------------------------------===//
  
  struct MemoryBufferSizes {
    size_t malloc_bytes;
    size_t mmap_bytes;
    
    MemoryBufferSizes(size_t malloc_bytes, size_t mmap_bytes)
    : malloc_bytes(malloc_bytes), mmap_bytes(mmap_bytes) {}
  };
  
  /// Return the amount of memory used by memory buffers, breaking down
  /// by heap-backed versus mmap'ed memory.
  MemoryBufferSizes getMemoryBufferSizes() const {
    MemoryBufferSizes sizes(0, 0);
    getMemoryBufferSizes(sizes);
    return sizes;
  }

  virtual void getMemoryBufferSizes(MemoryBufferSizes &sizes) const;

protected:
  static DeclContextLookupResult
  SetExternalVisibleDeclsForName(const DeclContext *DC,
                                 DeclarationName Name,
                                 ArrayRef<NamedDecl*> Decls);

  static DeclContextLookupResult
  SetNoExternalVisibleDeclsForName(const DeclContext *DC,
                                   DeclarationName Name);

  /// \brief Increment the current generation.
  uint32_t incrementGeneration(ASTContext &C);
};

/// \brief A lazy pointer to an AST node (of base type T) that resides
/// within an external AST source.
///
/// The AST node is identified within the external AST source by a
/// 63-bit offset, and can be retrieved via an operation on the
/// external AST source itself.
template<typename T, typename OffsT, T* (ExternalASTSource::*Get)(OffsT Offset)>
struct LazyOffsetPtr {
  /// \brief Either a pointer to an AST node or the offset within the
  /// external AST source where the AST node can be found.
  ///
  /// If the low bit is clear, a pointer to the AST node. If the low
  /// bit is set, the upper 63 bits are the offset.
  mutable uint64_t Ptr;

public:
  LazyOffsetPtr() : Ptr(0) { }

  explicit LazyOffsetPtr(T *Ptr) : Ptr(reinterpret_cast<uint64_t>(Ptr)) { }
  explicit LazyOffsetPtr(uint64_t Offset) : Ptr((Offset << 1) | 0x01) {
    assert((Offset << 1 >> 1) == Offset && "Offsets must require < 63 bits");
    if (Offset == 0)
      Ptr = 0;
  }

  LazyOffsetPtr &operator=(T *Ptr) {
    this->Ptr = reinterpret_cast<uint64_t>(Ptr);
    return *this;
  }

  LazyOffsetPtr &operator=(uint64_t Offset) {
    assert((Offset << 1 >> 1) == Offset && "Offsets must require < 63 bits");
    if (Offset == 0)
      Ptr = 0;
    else
      Ptr = (Offset << 1) | 0x01;

    return *this;
  }

  /// \brief Whether this pointer is non-NULL.
  ///
  /// This operation does not require the AST node to be deserialized.
  explicit operator bool() const { return Ptr != 0; }

  /// \brief Whether this pointer is non-NULL.
  ///
  /// This operation does not require the AST node to be deserialized.
  bool isValid() const { return Ptr != 0; }

  /// \brief Whether this pointer is currently stored as an offset.
  bool isOffset() const { return Ptr & 0x01; }

  /// \brief Retrieve the pointer to the AST node that this lazy pointer
  ///
  /// \param Source the external AST source.
  ///
  /// \returns a pointer to the AST node.
  T* get(ExternalASTSource *Source) const {
    if (isOffset()) {
      assert(Source &&
             "Cannot deserialize a lazy pointer without an AST source");
      Ptr = reinterpret_cast<uint64_t>((Source->*Get)(Ptr >> 1));
    }
    return reinterpret_cast<T*>(Ptr);
  }
};

/// \brief A lazy value (of type T) that is within an AST node of type Owner,
/// where the value might change in later generations of the external AST
/// source.
template<typename Owner, typename T, void (ExternalASTSource::*Update)(Owner)>
struct LazyGenerationalUpdatePtr {
  /// A cache of the value of this pointer, in the most recent generation in
  /// which we queried it.
  struct LazyData {
    LazyData(ExternalASTSource *Source, T Value)
        : ExternalSource(Source), LastGeneration(0), LastValue(Value) {}
    ExternalASTSource *ExternalSource;
    uint32_t LastGeneration;
    T LastValue;
  };

  // Our value is represented as simply T if there is no external AST source.
  typedef llvm::PointerUnion<T, LazyData*> ValueType;
  ValueType Value;

  LazyGenerationalUpdatePtr(ValueType V) : Value(V) {}

  // Defined in ASTContext.h
  static ValueType makeValue(const ASTContext &Ctx, T Value);

public:
  explicit LazyGenerationalUpdatePtr(const ASTContext &Ctx, T Value = T())
      : Value(makeValue(Ctx, Value)) {}

  /// Create a pointer that is not potentially updated by later generations of
  /// the external AST source.
  enum NotUpdatedTag { NotUpdated };
  LazyGenerationalUpdatePtr(NotUpdatedTag, T Value = T())
      : Value(Value) {}

  /// Forcibly set this pointer (which must be lazy) as needing updates.
  void markIncomplete() {
    Value.template get<LazyData *>()->LastGeneration = 0;
  }

  /// Set the value of this pointer, in the current generation.
  void set(T NewValue) {
    if (LazyData *LazyVal = Value.template dyn_cast<LazyData*>()) {
      LazyVal->LastValue = NewValue;
      return;
    }
    Value = NewValue;
  }

  /// Set the value of this pointer, for this and all future generations.
  void setNotUpdated(T NewValue) { Value = NewValue; }

  /// Get the value of this pointer, updating its owner if necessary.
  T get(Owner O) {
    if (LazyData *LazyVal = Value.template dyn_cast<LazyData*>()) {
      if (LazyVal->LastGeneration != LazyVal->ExternalSource->getGeneration()) {
        LazyVal->LastGeneration = LazyVal->ExternalSource->getGeneration();
        (LazyVal->ExternalSource->*Update)(O);
      }
      return LazyVal->LastValue;
    }
    return Value.template get<T>();
  }

  /// Get the most recently computed value of this pointer without updating it.
  T getNotUpdated() const {
    if (LazyData *LazyVal = Value.template dyn_cast<LazyData*>())
      return LazyVal->LastValue;
    return Value.template get<T>();
  }

  void *getOpaqueValue() { return Value.getOpaqueValue(); }
  static LazyGenerationalUpdatePtr getFromOpaqueValue(void *Ptr) {
    return LazyGenerationalUpdatePtr(ValueType::getFromOpaqueValue(Ptr));
  }
};
} // end namespace clang

/// Specialize PointerLikeTypeTraits to allow LazyGenerationalUpdatePtr to be
/// placed into a PointerUnion.
namespace llvm {
template<typename Owner, typename T,
         void (clang::ExternalASTSource::*Update)(Owner)>
struct PointerLikeTypeTraits<
    clang::LazyGenerationalUpdatePtr<Owner, T, Update>> {
  typedef clang::LazyGenerationalUpdatePtr<Owner, T, Update> Ptr;
  static void *getAsVoidPointer(Ptr P) { return P.getOpaqueValue(); }
  static Ptr getFromVoidPointer(void *P) { return Ptr::getFromOpaqueValue(P); }
  enum {
    NumLowBitsAvailable = PointerLikeTypeTraits<T>::NumLowBitsAvailable - 1
  };
};
}

namespace clang {
/// \brief Represents a lazily-loaded vector of data.
///
/// The lazily-loaded vector of data contains data that is partially loaded
/// from an external source and partially added by local translation. The 
/// items loaded from the external source are loaded lazily, when needed for
/// iteration over the complete vector.
template<typename T, typename Source, 
         void (Source::*Loader)(SmallVectorImpl<T>&),
         unsigned LoadedStorage = 2, unsigned LocalStorage = 4>
class LazyVector {
  SmallVector<T, LoadedStorage> Loaded;
  SmallVector<T, LocalStorage> Local;

public:
  /// Iteration over the elements in the vector.
  ///
  /// In a complete iteration, the iterator walks the range [-M, N),
  /// where negative values are used to indicate elements
  /// loaded from the external source while non-negative values are used to
  /// indicate elements added via \c push_back().
  /// However, to provide iteration in source order (for, e.g., chained
  /// precompiled headers), dereferencing the iterator flips the negative
  /// values (corresponding to loaded entities), so that position -M
  /// corresponds to element 0 in the loaded entities vector, position -M+1
  /// corresponds to element 1 in the loaded entities vector, etc. This
  /// gives us a reasonably efficient, source-order walk.
  ///
  /// We define this as a wrapping iterator around an int. The
  /// iterator_adaptor_base class forwards the iterator methods to basic integer
  /// arithmetic.
  class iterator : public llvm::iterator_adaptor_base<
                       iterator, int, std::random_access_iterator_tag, T, int> {
    LazyVector *Self;

    iterator(LazyVector *Self, int Position)
        : iterator::iterator_adaptor_base(Position), Self(Self) {}

    bool isLoaded() const { return this->I < 0; }
    friend class LazyVector;

  public:
    iterator() : iterator(nullptr, 0) {}

    typename iterator::reference operator*() const {
      if (isLoaded())
        return Self->Loaded.end()[this->I];
      return Self->Local.begin()[this->I];
    }
  };

  iterator begin(Source *source, bool LocalOnly = false) {
    if (LocalOnly)
      return iterator(this, 0);
    
    if (source)
      (source->*Loader)(Loaded);
    return iterator(this, -(int)Loaded.size());
  }
  
  iterator end() {
    return iterator(this, Local.size());
  }
  
  void push_back(const T& LocalValue) {
    Local.push_back(LocalValue);
  }
  
  void erase(iterator From, iterator To) {
    if (From.isLoaded() && To.isLoaded()) {
      Loaded.erase(&*From, &*To);
      return;
    }

    if (From.isLoaded()) {
      Loaded.erase(&*From, Loaded.end());
      From = begin(nullptr, true);
    }

    Local.erase(&*From, &*To);
  }
};

/// \brief A lazy pointer to a statement.
typedef LazyOffsetPtr<Stmt, uint64_t, &ExternalASTSource::GetExternalDeclStmt>
  LazyDeclStmtPtr;

/// \brief A lazy pointer to a declaration.
typedef LazyOffsetPtr<Decl, uint32_t, &ExternalASTSource::GetExternalDecl>
  LazyDeclPtr;

/// \brief A lazy pointer to a set of CXXCtorInitializers.
typedef LazyOffsetPtr<CXXCtorInitializer *, uint64_t,
                      &ExternalASTSource::GetExternalCXXCtorInitializers>
  LazyCXXCtorInitializersPtr;

/// \brief A lazy pointer to a set of CXXBaseSpecifiers.
typedef LazyOffsetPtr<CXXBaseSpecifier, uint64_t,
                      &ExternalASTSource::GetExternalCXXBaseSpecifiers>
  LazyCXXBaseSpecifiersPtr;

} // end namespace clang

#endif
