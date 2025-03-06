//===--- PreprocessingRecord.h - Record of Preprocessing --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the PreprocessingRecord class, which maintains a record
//  of what occurred during preprocessing.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_LEX_PREPROCESSINGRECORD_H
#define LLVM_CLANG_LEX_PREPROCESSINGRECORD_H

#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/PPCallbacks.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/iterator.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Compiler.h"
#include <vector>

namespace clang {
  class IdentifierInfo;
  class MacroInfo;
  class PreprocessingRecord;
}

/// \brief Allocates memory within a Clang preprocessing record.
void* operator new(size_t bytes, clang::PreprocessingRecord& PR,
                   unsigned alignment = 8) throw();

/// \brief Frees memory allocated in a Clang preprocessing record.
void operator delete(void *ptr, clang::PreprocessingRecord &PR,
                     unsigned) throw();

namespace clang {
  class MacroDefinitionRecord;
  class FileEntry;

  /// \brief Base class that describes a preprocessed entity, which may be a
  /// preprocessor directive or macro expansion.
  class PreprocessedEntity {
  public:
    /// \brief The kind of preprocessed entity an object describes.
    enum EntityKind {
      /// \brief Indicates a problem trying to load the preprocessed entity.
      InvalidKind,

      /// \brief A macro expansion.
      MacroExpansionKind,
      
      /// \defgroup Preprocessing directives
      /// @{
      
      /// \brief A macro definition.
      MacroDefinitionKind,
      
      /// \brief An inclusion directive, such as \c \#include, \c
      /// \#import, or \c \#include_next.
      InclusionDirectiveKind,

      /// @}

      FirstPreprocessingDirective = MacroDefinitionKind,
      LastPreprocessingDirective = InclusionDirectiveKind
    };

  private:
    /// \brief The kind of preprocessed entity that this object describes.
    EntityKind Kind;
    
    /// \brief The source range that covers this preprocessed entity.
    SourceRange Range;
    
  protected:
    PreprocessedEntity(EntityKind Kind, SourceRange Range)
      : Kind(Kind), Range(Range) { }

    friend class PreprocessingRecord;

  public:
    /// \brief Retrieve the kind of preprocessed entity stored in this object.
    EntityKind getKind() const { return Kind; }
    
    /// \brief Retrieve the source range that covers this entire preprocessed 
    /// entity.
    SourceRange getSourceRange() const LLVM_READONLY { return Range; }

    /// \brief Returns true if there was a problem loading the preprocessed
    /// entity.
    bool isInvalid() const { return Kind == InvalidKind; }

    // Only allow allocation of preprocessed entities using the allocator 
    // in PreprocessingRecord or by doing a placement new.
    void* operator new(size_t bytes, PreprocessingRecord& PR,
                       unsigned alignment = 8) throw() {
      return ::operator new(bytes, PR, alignment);
    }
    
    void* operator new(size_t bytes, void* mem) throw() {
      return mem;
    }
    
    void operator delete(void* ptr, PreprocessingRecord& PR, 
                         unsigned alignment) throw() {
      return ::operator delete(ptr, PR, alignment);
    }
    
    void operator delete(void*, std::size_t) throw() { }
    void operator delete(void*, void*) throw() { }
    
  private:
    // Make vanilla 'new' and 'delete' illegal for preprocessed entities.
    void* operator new(size_t bytes) throw();
    void operator delete(void* data) throw();
  };
  
  /// \brief Records the presence of a preprocessor directive.
  class PreprocessingDirective : public PreprocessedEntity {
  public:
    PreprocessingDirective(EntityKind Kind, SourceRange Range) 
      : PreprocessedEntity(Kind, Range) { }
    
    // Implement isa/cast/dyncast/etc.
    static bool classof(const PreprocessedEntity *PD) { 
      return PD->getKind() >= FirstPreprocessingDirective &&
             PD->getKind() <= LastPreprocessingDirective;
    }
  };

  /// \brief Record the location of a macro definition.
  class MacroDefinitionRecord : public PreprocessingDirective {
    /// \brief The name of the macro being defined.
    const IdentifierInfo *Name;

  public:
    explicit MacroDefinitionRecord(const IdentifierInfo *Name,
                                   SourceRange Range)
        : PreprocessingDirective(MacroDefinitionKind, Range), Name(Name) {}

    /// \brief Retrieve the name of the macro being defined.
    const IdentifierInfo *getName() const { return Name; }

    /// \brief Retrieve the location of the macro name in the definition.
    SourceLocation getLocation() const { return getSourceRange().getBegin(); }
    
    // Implement isa/cast/dyncast/etc.
    static bool classof(const PreprocessedEntity *PE) {
      return PE->getKind() == MacroDefinitionKind;
    }
  };
  
  /// \brief Records the location of a macro expansion.
  class MacroExpansion : public PreprocessedEntity {
    /// \brief The definition of this macro or the name of the macro if it is
    /// a builtin macro.
    llvm::PointerUnion<IdentifierInfo *, MacroDefinitionRecord *> NameOrDef;

  public:
    MacroExpansion(IdentifierInfo *BuiltinName, SourceRange Range)
        : PreprocessedEntity(MacroExpansionKind, Range),
          NameOrDef(BuiltinName) {}

    MacroExpansion(MacroDefinitionRecord *Definition, SourceRange Range)
        : PreprocessedEntity(MacroExpansionKind, Range), NameOrDef(Definition) {
    }

    /// \brief True if it is a builtin macro.
    bool isBuiltinMacro() const { return NameOrDef.is<IdentifierInfo *>(); }

    /// \brief The name of the macro being expanded.
    const IdentifierInfo *getName() const {
      if (MacroDefinitionRecord *Def = getDefinition())
        return Def->getName();
      return NameOrDef.get<IdentifierInfo *>();
    }

    /// \brief The definition of the macro being expanded. May return null if
    /// this is a builtin macro.
    MacroDefinitionRecord *getDefinition() const {
      return NameOrDef.dyn_cast<MacroDefinitionRecord *>();
    }

    // Implement isa/cast/dyncast/etc.
    static bool classof(const PreprocessedEntity *PE) {
      return PE->getKind() == MacroExpansionKind;
    }
  };

  /// \brief Record the location of an inclusion directive, such as an
  /// \c \#include or \c \#import statement.
  class InclusionDirective : public PreprocessingDirective {
  public:
    /// \brief The kind of inclusion directives known to the
    /// preprocessor.
    enum InclusionKind {
      /// \brief An \c \#include directive.
      Include,
      /// \brief An Objective-C \c \#import directive.
      Import,
      /// \brief A GNU \c \#include_next directive.
      IncludeNext,
      /// \brief A Clang \c \#__include_macros directive.
      IncludeMacros
    };

  private:
    /// \brief The name of the file that was included, as written in
    /// the source.
    StringRef FileName;

    /// \brief Whether the file name was in quotation marks; otherwise, it was
    /// in angle brackets.
    unsigned InQuotes : 1;

    /// \brief The kind of inclusion directive we have.
    ///
    /// This is a value of type InclusionKind.
    unsigned Kind : 2;

    /// \brief Whether the inclusion directive was automatically turned into
    /// a module import.
    unsigned ImportedModule : 1;

    /// \brief The file that was included.
    const FileEntry *File;

  public:
    InclusionDirective(PreprocessingRecord &PPRec,
                       InclusionKind Kind, StringRef FileName, 
                       bool InQuotes, bool ImportedModule,
                       const FileEntry *File, SourceRange Range);
    
    /// \brief Determine what kind of inclusion directive this is.
    InclusionKind getKind() const { return static_cast<InclusionKind>(Kind); }
    
    /// \brief Retrieve the included file name as it was written in the source.
    StringRef getFileName() const { return FileName; }
    
    /// \brief Determine whether the included file name was written in quotes;
    /// otherwise, it was written in angle brackets.
    bool wasInQuotes() const { return InQuotes; }

    /// \brief Determine whether the inclusion directive was automatically
    /// turned into a module import.
    bool importedModule() const { return ImportedModule; }
    
    /// \brief Retrieve the file entry for the actual file that was included
    /// by this directive.
    const FileEntry *getFile() const { return File; }
        
    // Implement isa/cast/dyncast/etc.
    static bool classof(const PreprocessedEntity *PE) {
      return PE->getKind() == InclusionDirectiveKind;
    }
  };
  
  /// \brief An abstract class that should be subclassed by any external source
  /// of preprocessing record entries.
  class ExternalPreprocessingRecordSource {
  public:
    virtual ~ExternalPreprocessingRecordSource();
    
    /// \brief Read a preallocated preprocessed entity from the external source.
    ///
    /// \returns null if an error occurred that prevented the preprocessed
    /// entity from being loaded.
    virtual PreprocessedEntity *ReadPreprocessedEntity(unsigned Index) = 0;

    /// \brief Returns a pair of [Begin, End) indices of preallocated
    /// preprocessed entities that \p Range encompasses.
    virtual std::pair<unsigned, unsigned>
        findPreprocessedEntitiesInRange(SourceRange Range) = 0;

    /// \brief Optionally returns true or false if the preallocated preprocessed
    /// entity with index \p Index came from file \p FID.
    virtual Optional<bool> isPreprocessedEntityInFileID(unsigned Index,
                                                        FileID FID) {
      return None;
    }
  };
  
  /// \brief A record of the steps taken while preprocessing a source file,
  /// including the various preprocessing directives processed, macros 
  /// expanded, etc.
  class PreprocessingRecord : public PPCallbacks {
    SourceManager &SourceMgr;
    
    /// \brief Allocator used to store preprocessing objects.
    llvm::BumpPtrAllocator BumpAlloc;

    /// \brief The set of preprocessed entities in this record, in order they
    /// were seen.
    std::vector<PreprocessedEntity *> PreprocessedEntities;
    
    /// \brief The set of preprocessed entities in this record that have been
    /// loaded from external sources.
    ///
    /// The entries in this vector are loaded lazily from the external source,
    /// and are referenced by the iterator using negative indices.
    std::vector<PreprocessedEntity *> LoadedPreprocessedEntities;

    /// \brief The set of ranges that were skipped by the preprocessor,
    std::vector<SourceRange> SkippedRanges;

    /// \brief Global (loaded or local) ID for a preprocessed entity.
    /// Negative values are used to indicate preprocessed entities
    /// loaded from the external source while non-negative values are used to
    /// indicate preprocessed entities introduced by the current preprocessor.
    /// Value -1 corresponds to element 0 in the loaded entities vector,
    /// value -2 corresponds to element 1 in the loaded entities vector, etc.
    /// Value 0 is an invalid value, the index to local entities is 1-based,
    /// value 1 corresponds to element 0 in the local entities vector,
    /// value 2 corresponds to element 1 in the local entities vector, etc.
    class PPEntityID {
      int ID;
      explicit PPEntityID(int ID) : ID(ID) {}
      friend class PreprocessingRecord;
    public:
      PPEntityID() : ID(0) {}
    };

    static PPEntityID getPPEntityID(unsigned Index, bool isLoaded) {
      return isLoaded ? PPEntityID(-int(Index)-1) : PPEntityID(Index+1);
    }

    /// \brief Mapping from MacroInfo structures to their definitions.
    llvm::DenseMap<const MacroInfo *, MacroDefinitionRecord *> MacroDefinitions;

    /// \brief External source of preprocessed entities.
    ExternalPreprocessingRecordSource *ExternalSource;

    /// \brief Retrieve the preprocessed entity at the given ID.
    PreprocessedEntity *getPreprocessedEntity(PPEntityID PPID);

    /// \brief Retrieve the loaded preprocessed entity at the given index.
    PreprocessedEntity *getLoadedPreprocessedEntity(unsigned Index);
    
    /// \brief Determine the number of preprocessed entities that were
    /// loaded (or can be loaded) from an external source.
    unsigned getNumLoadedPreprocessedEntities() const {
      return LoadedPreprocessedEntities.size();
    }

    /// \brief Returns a pair of [Begin, End) indices of local preprocessed
    /// entities that \p Range encompasses.
    std::pair<unsigned, unsigned>
      findLocalPreprocessedEntitiesInRange(SourceRange Range) const;
    unsigned findBeginLocalPreprocessedEntity(SourceLocation Loc) const;
    unsigned findEndLocalPreprocessedEntity(SourceLocation Loc) const;

    /// \brief Allocate space for a new set of loaded preprocessed entities.
    ///
    /// \returns The index into the set of loaded preprocessed entities, which
    /// corresponds to the first newly-allocated entity.
    unsigned allocateLoadedEntities(unsigned NumEntities);

    /// \brief Register a new macro definition.
    void RegisterMacroDefinition(MacroInfo *Macro, MacroDefinitionRecord *Def);

  public:
    /// \brief Construct a new preprocessing record.
    explicit PreprocessingRecord(SourceManager &SM);

    /// \brief Allocate memory in the preprocessing record.
    void *Allocate(unsigned Size, unsigned Align = 8) {
      return BumpAlloc.Allocate(Size, Align);
    }
    
    /// \brief Deallocate memory in the preprocessing record.
    void Deallocate(void *Ptr) { }

    size_t getTotalMemory() const;

    SourceManager &getSourceManager() const { return SourceMgr; }

    /// Iteration over the preprocessed entities.
    ///
    /// In a complete iteration, the iterator walks the range [-M, N),
    /// where negative values are used to indicate preprocessed entities
    /// loaded from the external source while non-negative values are used to
    /// indicate preprocessed entities introduced by the current preprocessor.
    /// However, to provide iteration in source order (for, e.g., chained
    /// precompiled headers), dereferencing the iterator flips the negative
    /// values (corresponding to loaded entities), so that position -M
    /// corresponds to element 0 in the loaded entities vector, position -M+1
    /// corresponds to element 1 in the loaded entities vector, etc. This
    /// gives us a reasonably efficient, source-order walk.
    ///
    /// We define this as a wrapping iterator around an int. The
    /// iterator_adaptor_base class forwards the iterator methods to basic
    /// integer arithmetic.
    class iterator : public llvm::iterator_adaptor_base<
                         iterator, int, std::random_access_iterator_tag,
                         PreprocessedEntity *, int, PreprocessedEntity *,
                         PreprocessedEntity *> {
      PreprocessingRecord *Self;

      iterator(PreprocessingRecord *Self, int Position)
          : iterator::iterator_adaptor_base(Position), Self(Self) {}
      friend class PreprocessingRecord;

    public:
      iterator() : iterator(nullptr, 0) {}

      PreprocessedEntity *operator*() const {
        bool isLoaded = this->I < 0;
        unsigned Index = isLoaded ?
            Self->LoadedPreprocessedEntities.size() + this->I : this->I;
        PPEntityID ID = Self->getPPEntityID(Index, isLoaded);
        return Self->getPreprocessedEntity(ID);
      }
      PreprocessedEntity *operator->() const { return **this; }
    };

    /// \brief Begin iterator for all preprocessed entities.
    iterator begin() {
      return iterator(this, -(int)LoadedPreprocessedEntities.size());
    }

    /// \brief End iterator for all preprocessed entities.
    iterator end() {
      return iterator(this, PreprocessedEntities.size());
    }

    /// \brief Begin iterator for local, non-loaded, preprocessed entities.
    iterator local_begin() {
      return iterator(this, 0);
    }

    /// \brief End iterator for local, non-loaded, preprocessed entities.
    iterator local_end() {
      return iterator(this, PreprocessedEntities.size());
    }

    /// \brief iterator range for the given range of loaded
    /// preprocessed entities.
    llvm::iterator_range<iterator> getIteratorsForLoadedRange(unsigned start,
                                                              unsigned count) {
      unsigned end = start + count;
      assert(end <= LoadedPreprocessedEntities.size());
      return llvm::make_range(
          iterator(this, int(start) - LoadedPreprocessedEntities.size()),
          iterator(this, int(end) - LoadedPreprocessedEntities.size()));
    }

    /// \brief Returns a range of preprocessed entities that source range \p R
    /// encompasses.
    ///
    /// \param R the range to look for preprocessed entities.
    ///
    llvm::iterator_range<iterator>
    getPreprocessedEntitiesInRange(SourceRange R);

    /// \brief Returns true if the preprocessed entity that \p PPEI iterator
    /// points to is coming from the file \p FID.
    ///
    /// Can be used to avoid implicit deserializations of preallocated
    /// preprocessed entities if we only care about entities of a specific file
    /// and not from files \#included in the range given at
    /// \see getPreprocessedEntitiesInRange.
    bool isEntityInFileID(iterator PPEI, FileID FID);

    /// \brief Add a new preprocessed entity to this record.
    PPEntityID addPreprocessedEntity(PreprocessedEntity *Entity);

    /// \brief Set the external source for preprocessed entities.
    void SetExternalSource(ExternalPreprocessingRecordSource &Source);

    /// \brief Retrieve the external source for preprocessed entities.
    ExternalPreprocessingRecordSource *getExternalSource() const {
      return ExternalSource;
    }

    /// \brief Retrieve the macro definition that corresponds to the given
    /// \c MacroInfo.
    MacroDefinitionRecord *findMacroDefinition(const MacroInfo *MI);

    /// \brief Retrieve all ranges that got skipped while preprocessing.
    const std::vector<SourceRange> &getSkippedRanges() const {
      return SkippedRanges;
    }
        
  private:
    void MacroExpands(const Token &Id, const MacroDefinition &MD,
                      SourceRange Range, const MacroArgs *Args) override;
    void MacroDefined(const Token &Id, const MacroDirective *MD) override;
    void MacroUndefined(const Token &Id, const MacroDefinition &MD) override;
    void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                            StringRef FileName, bool IsAngled,
                            CharSourceRange FilenameRange,
                            const FileEntry *File, StringRef SearchPath,
                            StringRef RelativePath,
                            const Module *Imported) override;
    void Ifdef(SourceLocation Loc, const Token &MacroNameTok,
               const MacroDefinition &MD) override;
    void Ifndef(SourceLocation Loc, const Token &MacroNameTok,
                const MacroDefinition &MD) override;
    /// \brief Hook called whenever the 'defined' operator is seen.
    void Defined(const Token &MacroNameTok, const MacroDefinition &MD,
                 SourceRange Range) override;

    void SourceRangeSkipped(SourceRange Range) override;

    void addMacroExpansion(const Token &Id, const MacroInfo *MI,
                           SourceRange Range);

    /// \brief Cached result of the last \see getPreprocessedEntitiesInRange
    /// query.
    struct {
      SourceRange Range;
      std::pair<int, int> Result;
    } CachedRangeQuery;

    std::pair<int, int> getPreprocessedEntitiesInRangeSlow(SourceRange R);

    friend class ASTReader;
    friend class ASTWriter;
  };
} // end namespace clang

inline void* operator new(size_t bytes, clang::PreprocessingRecord& PR,
                          unsigned alignment) throw() {
  return PR.Allocate(bytes, alignment);
}

inline void operator delete(void* ptr, clang::PreprocessingRecord& PR,
                            unsigned) throw() {
  PR.Deallocate(ptr);
}

#endif // LLVM_CLANG_LEX_PREPROCESSINGRECORD_H
