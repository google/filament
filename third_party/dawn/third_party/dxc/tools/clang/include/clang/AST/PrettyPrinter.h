//===--- PrettyPrinter.h - Classes for aiding with AST printing -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the PrinterHelper interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_PRETTYPRINTER_H
#define LLVM_CLANG_AST_PRETTYPRINTER_H

#include "clang/Basic/LLVM.h"
#include "clang/Basic/LangOptions.h"

namespace clang {

class LangOptions;
class SourceManager;
class Stmt;
class TagDecl;

class PrinterHelper {
public:
  virtual ~PrinterHelper();
  virtual bool handledStmt(Stmt* E, raw_ostream& OS) = 0;
};

/// \brief Describes how types, statements, expressions, and
/// declarations should be printed.
struct PrintingPolicy {
  /// \brief Create a default printing policy for C.
  PrintingPolicy(const LangOptions &LO)
    : LangOpts(LO), Indentation(2), SuppressSpecifiers(false),
      SuppressTagKeyword(true), SuppressTag(false), SuppressScope(false), // HLSL Change - default SuppressTagKeyword to true (C++ default behavior)
      SuppressUnwrittenScope(false), SuppressInitializers(false),
      ConstantArraySizeAsWritten(false), AnonymousTagLocations(true),
      SuppressStrongLifetime(false), SuppressLifetimeQualifiers(false),
      Bool(LO.Bool), TerseOutput(false), PolishForDeclaration(false),
      Half(LO.HLSL || LO.Half), // HLSL Change - always print 'half' for HLSL
      MSWChar(LO.MicrosoftExt && !LO.WChar),
      IncludeNewlines(true),
      // HLSL Change Begin - hlsl print policy.
      HLSLSuppressUniformParameters(false), HLSLOnlyDecl(false),
      HLSLNoinlineMethod(false), HLSLOmitDefaultTemplateParams(false)
      // HLSL Change End.
      {}

  /// \brief What language we're printing.
  LangOptions LangOpts;

  /// \brief The number of spaces to use to indent each line.
  unsigned Indentation : 8;

  /// \brief Whether we should suppress printing of the actual specifiers for
  /// the given type or declaration.
  ///
  /// This flag is only used when we are printing declarators beyond
  /// the first declarator within a declaration group. For example, given:
  ///
  /// \code
  /// const int *x, *y;
  /// \endcode
  ///
  /// SuppressSpecifiers will be false when printing the
  /// declaration for "x", so that we will print "int *x"; it will be
  /// \c true when we print "y", so that we suppress printing the
  /// "const int" type specifier and instead only print the "*y".
  bool SuppressSpecifiers : 1;

  /// \brief Whether type printing should skip printing the tag keyword.
  ///
  /// This is used when printing the inner type of elaborated types,
  /// (as the tag keyword is part of the elaborated type):
  ///
  /// \code
  /// struct Geometry::Point;
  /// \endcode
  bool SuppressTagKeyword : 1;

  /// \brief Whether type printing should skip printing the actual tag type.
  ///
  /// This is used when the caller needs to print a tag definition in front
  /// of the type, as in constructs like the following:
  ///
  /// \code
  /// typedef struct { int x, y; } Point;
  /// \endcode
  bool SuppressTag : 1;

  /// \brief Suppresses printing of scope specifiers.
  bool SuppressScope : 1;

  /// \brief Suppress printing parts of scope specifiers that don't need
  /// to be written, e.g., for inline or anonymous namespaces.
  bool SuppressUnwrittenScope : 1;
  
  /// \brief Suppress printing of variable initializers.
  ///
  /// This flag is used when printing the loop variable in a for-range
  /// statement. For example, given:
  ///
  /// \code
  /// for (auto x : coll)
  /// \endcode
  ///
  /// SuppressInitializers will be true when printing "auto x", so that the
  /// internal initializer constructed for x will not be printed.
  bool SuppressInitializers : 1;

  /// \brief Whether we should print the sizes of constant array expressions
  /// as written in the sources.
  ///
  /// This flag is determines whether arrays types declared as
  ///
  /// \code
  /// int a[4+10*10];
  /// char a[] = "A string";
  /// \endcode
  ///
  /// will be printed as written or as follows:
  ///
  /// \code
  /// int a[104];
  /// char a[9] = "A string";
  /// \endcode
  bool ConstantArraySizeAsWritten : 1;
  
  /// \brief When printing an anonymous tag name, also print the location of
  /// that entity (e.g., "enum <anonymous at t.h:10:5>"). Otherwise, just 
  /// prints "(anonymous)" for the name.
  bool AnonymousTagLocations : 1;
  
  /// \brief When true, suppress printing of the __strong lifetime qualifier in
  /// ARC.
  unsigned SuppressStrongLifetime : 1;
  
  /// \brief When true, suppress printing of lifetime qualifier in
  /// ARC.
  unsigned SuppressLifetimeQualifiers : 1;
  
  /// \brief Whether we can use 'bool' rather than '_Bool', even if the language
  /// doesn't actually have 'bool' (because, e.g., it is defined as a macro).
  unsigned Bool : 1;

  /// \brief Provide a 'terse' output.
  ///
  /// For example, in this mode we don't print function bodies, class members,
  /// declarations inside namespaces etc.  Effectively, this should print
  /// only the requested declaration.
  unsigned TerseOutput : 1;
  
  /// \brief When true, do certain refinement needed for producing proper
  /// declaration tag; such as, do not print attributes attached to the declaration.
  ///
  unsigned PolishForDeclaration : 1;

  /// \brief When true, print the half-precision floating-point type as 'half'
  /// instead of '__fp16'
  unsigned Half : 1;

  /// \brief When true, print the built-in wchar_t type as __wchar_t. For use in
  /// Microsoft mode when wchar_t is not available.
  unsigned MSWChar : 1;

  /// \brief When true, include newlines after statements like "break", etc.
  unsigned IncludeNewlines : 1;

  // HLSL Change Begin
  /// \brief When true, exclude uniform function parameters
  unsigned HLSLSuppressUniformParameters : 1;
  /// \brief When true, only print function decl without function body.
  unsigned HLSLOnlyDecl : 1;
  /// \brief When true, print inline method define as outside struct scope define.
  unsigned HLSLNoinlineMethod : 1;
  /// \brief When true, omit default template parameter lists
  unsigned HLSLOmitDefaultTemplateParams : 1;
  // HLSL Change Ends
};

} // end namespace clang

#endif
