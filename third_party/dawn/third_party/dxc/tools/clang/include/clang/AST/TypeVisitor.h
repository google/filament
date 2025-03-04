//===--- TypeVisitor.h - Visitor for Type subclasses ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the TypeVisitor interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_TYPEVISITOR_H
#define LLVM_CLANG_AST_TYPEVISITOR_H

#include "clang/AST/Type.h"

namespace clang {

#define DISPATCH(CLASS) \
  return static_cast<ImplClass*>(this)-> \
           Visit##CLASS(static_cast<const CLASS*>(T))

/// \brief An operation on a type.
///
/// \tparam ImplClass Class implementing the operation. Must be inherited from
///         TypeVisitor.
/// \tparam RetTy %Type of result produced by the operation.
///
/// The class implements polymorphic operation on an object of type derived
/// from Type. The operation is performed by calling method Visit. It then
/// dispatches the call to function \c VisitFooType, if actual argument type
/// is \c FooType.
///
/// The class implements static polymorphism using Curiously Recurring
/// Template Pattern. It is designed to be a base class for some concrete
/// class:
///
/// \code
///     class SomeVisitor : public TypeVisitor<SomeVisitor,sometype> { ... };
///     ...
///     Type *atype = ...
///     ...
///     SomeVisitor avisitor;
///     sometype result = avisitor.Visit(atype);
/// \endcode
///
/// Actual treatment is made by methods of the derived class, TypeVisitor only
/// dispatches call to the appropriate method. If the implementation class
/// \c ImplClass provides specific action for some type, say
/// \c ConstantArrayType, it should define method
/// <tt>VisitConstantArrayType(const ConstantArrayType*)</tt>. Otherwise
/// \c TypeVisitor dispatches call to the method that handles parent type. In
/// this example handlers are tried in the sequence:
///
/// \li <tt>ImplClass::VisitConstantArrayType(const ConstantArrayType*)</tt>
/// \li <tt>ImplClass::VisitArrayType(const ArrayType*)</tt>
/// \li <tt>ImplClass::VisitType(const Type*)</tt>
/// \li <tt>TypeVisitor::VisitType(const Type*)</tt>
///
/// The first function of this sequence that is defined will handle object of
/// type \c ConstantArrayType.
template<typename ImplClass, typename RetTy=void>
class TypeVisitor {
public:

  /// \brief Performs the operation associated with this visitor object.
  RetTy Visit(const Type *T) {
    // Top switch stmt: dispatch to VisitFooType for each FooType.
    switch (T->getTypeClass()) {
#define ABSTRACT_TYPE(CLASS, PARENT)
#define TYPE(CLASS, PARENT) case Type::CLASS: DISPATCH(CLASS##Type);
#include "clang/AST/TypeNodes.def"
    }
    llvm_unreachable("Unknown type class!");
  }

  // If the implementation chooses not to implement a certain visit method, fall
  // back on superclass.
#define TYPE(CLASS, PARENT) RetTy Visit##CLASS##Type(const CLASS##Type *T) { \
  DISPATCH(PARENT);                                                          \
}
#include "clang/AST/TypeNodes.def"

  /// \brief Method called if \c ImpClass doesn't provide specific handler
  /// for some type class.
  RetTy VisitType(const Type*) { return RetTy(); }
};

#undef DISPATCH

}  // end namespace clang

#endif
