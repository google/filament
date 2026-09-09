//===------- InitListHandler.h - Initializer List Handler -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//===----------------------------------------------------------------------===//
//
//  This file defines an initalizer list handler that takes in an InitListExpr
//  and emits the corresponding SPIR-V instructions for it.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_INITLISTHANDLER_H
#define LLVM_CLANG_LIB_SPIRV_INITLISTHANDLER_H

#include <deque>
#include <utility>
#include <vector>

#include "clang/AST/Expr.h"
#include "clang/Basic/Diagnostic.h"

#include "SpirvEmitter.h"

namespace clang {
namespace spirv {

/// The class for handling initializer lists.
///
/// Initializer lists in HLSL are very flexible; as long as the initializer
/// list provides the exact number of elements required by the type of the
/// object to be initialized, it will highly likely be accepted. To handle
/// such flexibility, composite extraction, recursive composite construction,
/// and proper casting is necessary for some cases. For example:
///
///   float4x4 mat = {scalar, vec1, vec2, vec3, vec2, float2(1, 2), 3, vec4};
/// The first element vector is composed of scalar, vec1, and vec2; the second
/// element vector is composed of vec3 and the first element of vec2; etc.
///
/// The AST is designed to omit the composite extraction and construction. It
/// also does not have casts to the expected types and from lvalues to rvalues.
///
/// Resulting from the above reasons, the logic for handling initalizer lists
/// are complicated. So we have this dedicated class for it. It is built on
/// top of the SpirvEmitter class and calls into SPIRVEmitter for normal
/// translation tasks. This gives better code structure.
///
/// The logic for handling initalizer lists is largely the following:
///
/// First we flatten() the given initalizer list recursively and put all
/// SPIR-V instructions for non-nitializer-list AST Exprs into the initializers
/// queue. This handles curly braces of even wired forms like
/// float2x2 mat = {{1.}, {2., {{3.}}}, 4.};
///
/// Then we construct the final SPIR-V composite from the initializer list
/// by traversing the type of the composite. This is done recursively in the
/// depth first search manner, using the type of the composite as the root.
///
/// When we reach a scalar type, we will try to decode a scalar value from the
/// front of the initializers queue. This may trigger composite extraction
/// since the front of the queue may be a vector/matrix. The leftover values
/// after the extraction should be retained for the next decoding. Thus, we need
/// another queue, scalars, to keep track of leftover unused scalar values.
/// To adjust properly, when decoding values for a given type, we first try
/// the scalar queue.
///
/// When we reach a composite type, we will try to construct a composite using
/// the scalar values previously extracted and retained in the scalars queue.
/// To optimize, if we have no leftover scalars and a value of the same type at
/// the front of the initializers queue, we use the value as a whole.
///
/// If the composite type is vector or matrix, we decompose() it into scalars as
/// explained above. If it is a struct or array type, the element type is not
/// guaranteed to be scalars. But still, we need to split them into their
/// elements. For such cases, we create OpCompositeExtract SPIR-V instructions
/// for all the elements and push them into the initializers queue.
class InitListHandler {
public:
  /// Constructs an InitListHandler which uses the given emitter for normal
  /// translation tasks. It will reuse the ModuleBuilder embedded in the given
  /// emitter.
  InitListHandler(ASTContext &ctx, SpirvEmitter &emitter);

  /// Processes the given InitListExpr and returns the <result-id> for the final
  /// SPIR-V value.
  SpirvInstruction *processInit(const InitListExpr *expr,
                                SourceRange rangeOverride = {});

  /// Casts the given Expr to the given toType and returns the <result-id> for
  /// the final SPIR-V value.
  SpirvInstruction *processCast(QualType toType, const Expr *expr);

private:
  /// \brief Wrapper method to create an error message and report it
  /// in the diagnostic engine associated with this consumer.
  template <unsigned N>
  DiagnosticBuilder emitError(const char (&message)[N], SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Error, message);
    return diags.Report(loc, diagId);
  }

  /// Construct a SPIR-V instruction whose type is |type| using |initializers|
  /// and returns the <result-id> for the final SPIR-V value of the given type.
  SpirvInstruction *doProcess(QualType type, SourceLocation srcLoc,
                              SourceRange range = {});

  /// Flattens the given InitListExpr and generates SPIR-V instructions for
  /// all non-InitListExpr AST nodes. Puts those generated SPIR-V instructions
  /// into |initializers|.
  void flatten(const InitListExpr *expr);

  /// Decomposes the given SpirvInstruction and puts all elements into the end
  /// of the scalars queue.
  void decompose(SpirvInstruction *inst, const SourceLocation &loc);

  /// If the next initializer is a struct, replaces it with OpCompositeExtract
  /// its members and returns true. Otherwise, does nothing and return false.
  bool tryToSplitStruct();
  /// If the next initializer is a constant array, replaces it with
  /// OpCompositeExtract to all its members and returns true. Otherwise, does
  /// nothing and return false.
  bool tryToSplitConstantArray();

  /// Emits the necessary SPIR-V instructions to create a SPIR-V value of the
  /// given type. The scalars and initializers queue will be used to fetch the
  /// next value.
  SpirvInstruction *createInitForType(QualType type, SourceLocation,
                                      SourceRange range = {});
  SpirvInstruction *createInitForBuiltinType(QualType type, SourceLocation);
  SpirvInstruction *createInitForVectorType(QualType elemType, uint32_t count,
                                            SourceLocation,
                                            SourceRange range = {});
  SpirvInstruction *createInitForMatrixType(QualType matrixType, SourceLocation,
                                            SourceRange range = {});
  SpirvInstruction *createInitForStructType(QualType type, SourceLocation,
                                            SourceRange range = {});
  SpirvInstruction *createInitForConstantArrayType(QualType type,
                                                   SourceLocation,
                                                   SourceRange range = {});
  SpirvInstruction *createInitForBufferOrImageType(QualType type,
                                                   SourceLocation);

private:
  ASTContext &astContext;
  SpirvEmitter &theEmitter;
  SpirvBuilder &spvBuilder;
  DiagnosticsEngine &diags;

  /// A queue keeping track of unused SPIR-V for initializers. Since we will
  /// only comsume initializers from the head of the queue and will not add new
  /// initializers to the tail of the queue, we use a vector (containing the
  /// reverse of the original intializer list) here and manipulate its tail.
  /// This is more efficient than using deque.
  std::vector<SpirvInstruction *> initializers;
  /// A queue keeping track of previously extracted but unused scalars.
  /// Each element is a pair, with the first element as the SPIR-V <result-id>
  /// and the second element as the AST type of the scalar value.
  std::deque<std::pair<SpirvInstruction *, QualType>> scalars;
};

} // end namespace spirv
} // end namespace clang

#endif
