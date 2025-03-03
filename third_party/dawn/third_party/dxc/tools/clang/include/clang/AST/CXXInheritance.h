//===------ CXXInheritance.h - C++ Inheritance ------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides routines that help analyzing C++ inheritance hierarchies.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_CXXINHERITANCE_H
#define LLVM_CLANG_AST_CXXINHERITANCE_H

#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclarationName.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeOrdering.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include <cassert>
#include <list>
#include <map>

namespace clang {
  
class CXXBaseSpecifier;
class CXXMethodDecl;
class CXXRecordDecl;
class NamedDecl;
  
/// \brief Represents an element in a path from a derived class to a
/// base class. 
/// 
/// Each step in the path references the link from a
/// derived class to one of its direct base classes, along with a
/// base "number" that identifies which base subobject of the
/// original derived class we are referencing.
struct CXXBasePathElement {
  /// \brief The base specifier that states the link from a derived
  /// class to a base class, which will be followed by this base
  /// path element.
  const CXXBaseSpecifier *Base;
  
  /// \brief The record decl of the class that the base is a base of.
  const CXXRecordDecl *Class;
  
  /// \brief Identifies which base class subobject (of type
  /// \c Base->getType()) this base path element refers to. 
  ///
  /// This value is only valid if \c !Base->isVirtual(), because there
  /// is no base numbering for the zero or one virtual bases of a
  /// given type.
  int SubobjectNumber;
};

/// \brief Represents a path from a specific derived class
/// (which is not represented as part of the path) to a particular
/// (direct or indirect) base class subobject.
///
/// Individual elements in the path are described by the \c CXXBasePathElement 
/// structure, which captures both the link from a derived class to one of its
/// direct bases and identification describing which base class
/// subobject is being used.
class CXXBasePath : public SmallVector<CXXBasePathElement, 4> {
public:
  CXXBasePath() : Access(AS_public) {}

  /// \brief The access along this inheritance path.  This is only
  /// calculated when recording paths.  AS_none is a special value
  /// used to indicate a path which permits no legal access.
  AccessSpecifier Access;

  /// \brief The set of declarations found inside this base class
  /// subobject.
  DeclContext::lookup_result Decls;

  void clear() {
    SmallVectorImpl<CXXBasePathElement>::clear();
    Access = AS_public;
  }
};

/// BasePaths - Represents the set of paths from a derived class to
/// one of its (direct or indirect) bases. For example, given the
/// following class hierarchy:
///
/// @code
/// class A { };
/// class B : public A { };
/// class C : public A { };
/// class D : public B, public C{ };
/// @endcode
///
/// There are two potential BasePaths to represent paths from D to a
/// base subobject of type A. One path is (D,0) -> (B,0) -> (A,0)
/// and another is (D,0)->(C,0)->(A,1). These two paths actually
/// refer to two different base class subobjects of the same type,
/// so the BasePaths object refers to an ambiguous path. On the
/// other hand, consider the following class hierarchy:
///
/// @code
/// class A { };
/// class B : public virtual A { };
/// class C : public virtual A { };
/// class D : public B, public C{ };
/// @endcode
///
/// Here, there are two potential BasePaths again, (D, 0) -> (B, 0)
/// -> (A,v) and (D, 0) -> (C, 0) -> (A, v), but since both of them
/// refer to the same base class subobject of type A (the virtual
/// one), there is no ambiguity.
class CXXBasePaths {
  /// \brief The type from which this search originated.
  CXXRecordDecl *Origin;
  
  /// Paths - The actual set of paths that can be taken from the
  /// derived class to the same base class.
  std::list<CXXBasePath> Paths;
  
  /// ClassSubobjects - Records the class subobjects for each class
  /// type that we've seen. The first element in the pair says
  /// whether we found a path to a virtual base for that class type,
  /// while the element contains the number of non-virtual base
  /// class subobjects for that class type. The key of the map is
  /// the cv-unqualified canonical type of the base class subobject.
  llvm::SmallDenseMap<QualType, std::pair<bool, unsigned>, 8> ClassSubobjects;
  
  /// FindAmbiguities - Whether Sema::IsDerivedFrom should try find
  /// ambiguous paths while it is looking for a path from a derived
  /// type to a base type.
  bool FindAmbiguities;
  
  /// RecordPaths - Whether Sema::IsDerivedFrom should record paths
  /// while it is determining whether there are paths from a derived
  /// type to a base type.
  bool RecordPaths;
  
  /// DetectVirtual - Whether Sema::IsDerivedFrom should abort the search
  /// if it finds a path that goes across a virtual base. The virtual class
  /// is also recorded.
  bool DetectVirtual;
  
  /// ScratchPath - A BasePath that is used by Sema::lookupInBases
  /// to help build the set of paths.
  CXXBasePath ScratchPath;

  /// DetectedVirtual - The base class that is virtual.
  const RecordType *DetectedVirtual;
  
  /// \brief Array of the declarations that have been found. This
  /// array is constructed only if needed, e.g., to iterate over the
  /// results within LookupResult.
  NamedDecl **DeclsFound;
  unsigned NumDeclsFound;
  
  friend class CXXRecordDecl;
  
  void ComputeDeclsFound();

  bool lookupInBases(ASTContext &Context, 
                     const CXXRecordDecl *Record,
                     CXXRecordDecl::BaseMatchesCallback *BaseMatches, 
                     void *UserData);
public:
  typedef std::list<CXXBasePath>::iterator paths_iterator;
  typedef std::list<CXXBasePath>::const_iterator const_paths_iterator;
  typedef NamedDecl **decl_iterator;
  
  /// BasePaths - Construct a new BasePaths structure to record the
  /// paths for a derived-to-base search.
  explicit CXXBasePaths(bool FindAmbiguities = true,
                        bool RecordPaths = true,
                        bool DetectVirtual = true)
    : FindAmbiguities(FindAmbiguities), RecordPaths(RecordPaths),
      DetectVirtual(DetectVirtual), DetectedVirtual(nullptr),
      DeclsFound(nullptr), NumDeclsFound(0) { }
  
  ~CXXBasePaths() { delete [] DeclsFound; }
  
  paths_iterator begin() { return Paths.begin(); }
  paths_iterator end()   { return Paths.end(); }
  const_paths_iterator begin() const { return Paths.begin(); }
  const_paths_iterator end()   const { return Paths.end(); }
  
  CXXBasePath&       front()       { return Paths.front(); }
  const CXXBasePath& front() const { return Paths.front(); }
  
  typedef llvm::iterator_range<decl_iterator> decl_range;
  decl_range found_decls();
  
  /// \brief Determine whether the path from the most-derived type to the
  /// given base type is ambiguous (i.e., it refers to multiple subobjects of
  /// the same base type).
  bool isAmbiguous(CanQualType BaseType);
  
  /// \brief Whether we are finding multiple paths to detect ambiguities.
  bool isFindingAmbiguities() const { return FindAmbiguities; }
  
  /// \brief Whether we are recording paths.
  bool isRecordingPaths() const { return RecordPaths; }
  
  /// \brief Specify whether we should be recording paths or not.
  void setRecordingPaths(bool RP) { RecordPaths = RP; }
  
  /// \brief Whether we are detecting virtual bases.
  bool isDetectingVirtual() const { return DetectVirtual; }
  
  /// \brief The virtual base discovered on the path (if we are merely
  /// detecting virtuals).
  const RecordType* getDetectedVirtual() const {
    return DetectedVirtual;
  }

  /// \brief Retrieve the type from which this base-paths search
  /// began
  CXXRecordDecl *getOrigin() const { return Origin; }
  void setOrigin(CXXRecordDecl *Rec) { Origin = Rec; }
  
  /// \brief Clear the base-paths results.
  void clear();
  
  /// \brief Swap this data structure's contents with another CXXBasePaths 
  /// object.
  void swap(CXXBasePaths &Other);
};

/// \brief Uniquely identifies a virtual method within a class
/// hierarchy by the method itself and a class subobject number.
struct UniqueVirtualMethod {
  UniqueVirtualMethod()
    : Method(nullptr), Subobject(0), InVirtualSubobject(nullptr) { }

  UniqueVirtualMethod(CXXMethodDecl *Method, unsigned Subobject,
                      const CXXRecordDecl *InVirtualSubobject)
    : Method(Method), Subobject(Subobject), 
      InVirtualSubobject(InVirtualSubobject) { }

  /// \brief The overriding virtual method.
  CXXMethodDecl *Method;

  /// \brief The subobject in which the overriding virtual method
  /// resides.
  unsigned Subobject;

  /// \brief The virtual base class subobject of which this overridden
  /// virtual method is a part. Note that this records the closest
  /// derived virtual base class subobject.
  const CXXRecordDecl *InVirtualSubobject;

  friend bool operator==(const UniqueVirtualMethod &X,
                         const UniqueVirtualMethod &Y) {
    return X.Method == Y.Method && X.Subobject == Y.Subobject &&
      X.InVirtualSubobject == Y.InVirtualSubobject;
  }

  friend bool operator!=(const UniqueVirtualMethod &X,
                         const UniqueVirtualMethod &Y) {
    return !(X == Y);
  }
};

/// \brief The set of methods that override a given virtual method in
/// each subobject where it occurs.
///
/// The first part of the pair is the subobject in which the
/// overridden virtual function occurs, while the second part of the
/// pair is the virtual method that overrides it (including the
/// subobject in which that virtual function occurs).
class OverridingMethods {
  typedef SmallVector<UniqueVirtualMethod, 4> ValuesT;
  typedef llvm::MapVector<unsigned, ValuesT> MapType;
  MapType Overrides;

public:
  // Iterate over the set of subobjects that have overriding methods.
  typedef MapType::iterator iterator;
  typedef MapType::const_iterator const_iterator;
  iterator begin() { return Overrides.begin(); }
  const_iterator begin() const { return Overrides.begin(); }
  iterator end() { return Overrides.end(); }
  const_iterator end() const { return Overrides.end(); }
  unsigned size() const { return Overrides.size(); }

  // Iterate over the set of overriding virtual methods in a given
  // subobject.
  typedef SmallVectorImpl<UniqueVirtualMethod>::iterator
    overriding_iterator;
  typedef SmallVectorImpl<UniqueVirtualMethod>::const_iterator
    overriding_const_iterator;

  // Add a new overriding method for a particular subobject.
  void add(unsigned OverriddenSubobject, UniqueVirtualMethod Overriding);

  // Add all of the overriding methods from "other" into overrides for
  // this method. Used when merging the overrides from multiple base
  // class subobjects.
  void add(const OverridingMethods &Other);

  // Replace all overriding virtual methods in all subobjects with the
  // given virtual method.
  void replaceAll(UniqueVirtualMethod Overriding);
};

/// \brief A mapping from each virtual member function to its set of
/// final overriders.
///
/// Within a class hierarchy for a given derived class, each virtual
/// member function in that hierarchy has one or more "final
/// overriders" (C++ [class.virtual]p2). A final overrider for a
/// virtual function "f" is the virtual function that will actually be
/// invoked when dispatching a call to "f" through the
/// vtable. Well-formed classes have a single final overrider for each
/// virtual function; in abstract classes, the final overrider for at
/// least one virtual function is a pure virtual function. Due to
/// multiple, virtual inheritance, it is possible for a class to have
/// more than one final overrider. Athough this is an error (per C++
/// [class.virtual]p2), it is not considered an error here: the final
/// overrider map can represent multiple final overriders for a
/// method, and it is up to the client to determine whether they are
/// problem. For example, the following class \c D has two final
/// overriders for the virtual function \c A::f(), one in \c C and one
/// in \c D:
///
/// \code
///   struct A { virtual void f(); };
///   struct B : virtual A { virtual void f(); };
///   struct C : virtual A { virtual void f(); };
///   struct D : B, C { };
/// \endcode
///
/// This data structure contains a mapping from every virtual
/// function *that does not override an existing virtual function* and
/// in every subobject where that virtual function occurs to the set
/// of virtual functions that override it. Thus, the same virtual
/// function \c A::f can actually occur in multiple subobjects of type
/// \c A due to multiple inheritance, and may be overridden by
/// different virtual functions in each, as in the following example:
///
/// \code
///   struct A { virtual void f(); };
///   struct B : A { virtual void f(); };
///   struct C : A { virtual void f(); };
///   struct D : B, C { };
/// \endcode
///
/// Unlike in the previous example, where the virtual functions \c
/// B::f and \c C::f both overrode \c A::f in the same subobject of
/// type \c A, in this example the two virtual functions both override
/// \c A::f but in *different* subobjects of type A. This is
/// represented by numbering the subobjects in which the overridden
/// and the overriding virtual member functions are located. Subobject
/// 0 represents the virtual base class subobject of that type, while
/// subobject numbers greater than 0 refer to non-virtual base class
/// subobjects of that type.
class CXXFinalOverriderMap
  : public llvm::MapVector<const CXXMethodDecl *, OverridingMethods> { };

/// \brief A set of all the primary bases for a class.
class CXXIndirectPrimaryBaseSet
  : public llvm::SmallSet<const CXXRecordDecl*, 32> { };

} // end namespace clang

#endif
