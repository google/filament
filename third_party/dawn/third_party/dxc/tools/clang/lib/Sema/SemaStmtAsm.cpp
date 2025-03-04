//===--- SemaStmtAsm.cpp - Semantic Analysis for Asm Statements -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements semantic analysis for inline asm statements.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecordLayout.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Initialization.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/ScopeInfo.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
using namespace clang;
using namespace sema;

/// CheckAsmLValue - GNU C has an extremely ugly extension whereby they silently
/// ignore "noop" casts in places where an lvalue is required by an inline asm.
/// We emulate this behavior when -fheinous-gnu-extensions is specified, but
/// provide a strong guidance to not use it.
///
/// This method checks to see if the argument is an acceptable l-value and
/// returns false if it is a case we can handle.
static bool CheckAsmLValue(const Expr *E, Sema &S) {
  // Type dependent expressions will be checked during instantiation.
  if (E->isTypeDependent())
    return false;

  if (E->isLValue())
    return false;  // Cool, this is an lvalue.

  // Okay, this is not an lvalue, but perhaps it is the result of a cast that we
  // are supposed to allow.
  const Expr *E2 = E->IgnoreParenNoopCasts(S.Context);
  if (E != E2 && E2->isLValue()) {
    if (!S.getLangOpts().HeinousExtensions)
      S.Diag(E2->getLocStart(), diag::err_invalid_asm_cast_lvalue)
        << E->getSourceRange();
    else
      S.Diag(E2->getLocStart(), diag::warn_invalid_asm_cast_lvalue)
        << E->getSourceRange();
    // Accept, even if we emitted an error diagnostic.
    return false;
  }

  // None of the above, just randomly invalid non-lvalue.
  return true;
}

/// isOperandMentioned - Return true if the specified operand # is mentioned
/// anywhere in the decomposed asm string.
static bool isOperandMentioned(unsigned OpNo,
                         ArrayRef<GCCAsmStmt::AsmStringPiece> AsmStrPieces) {
  for (unsigned p = 0, e = AsmStrPieces.size(); p != e; ++p) {
    const GCCAsmStmt::AsmStringPiece &Piece = AsmStrPieces[p];
    if (!Piece.isOperand()) continue;

    // If this is a reference to the input and if the input was the smaller
    // one, then we have to reject this asm.
    if (Piece.getOperandNo() == OpNo)
      return true;
  }
  return false;
}

static bool CheckNakedParmReference(Expr *E, Sema &S) {
  FunctionDecl *Func = dyn_cast<FunctionDecl>(S.CurContext);
  if (!Func)
    return false;
  if (!Func->hasAttr<NakedAttr>())
    return false;

  SmallVector<Expr*, 4> WorkList;
  WorkList.push_back(E);
  while (WorkList.size()) {
    Expr *E = WorkList.pop_back_val();
    if (isa<CXXThisExpr>(E)) {
      S.Diag(E->getLocStart(), diag::err_asm_naked_this_ref);
      S.Diag(Func->getAttr<NakedAttr>()->getLocation(), diag::note_attribute);
      return true;
    }
    if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(E)) {
      if (isa<ParmVarDecl>(DRE->getDecl())) {
        S.Diag(DRE->getLocStart(), diag::err_asm_naked_parm_ref);
        S.Diag(Func->getAttr<NakedAttr>()->getLocation(), diag::note_attribute);
        return true;
      }
    }
    for (Stmt *Child : E->children()) {
      if (Expr *E = dyn_cast_or_null<Expr>(Child))
        WorkList.push_back(E);
    }
  }
  return false;
}

StmtResult Sema::ActOnGCCAsmStmt(SourceLocation AsmLoc, bool IsSimple,
                                 bool IsVolatile, unsigned NumOutputs,
                                 unsigned NumInputs, IdentifierInfo **Names,
                                 MultiExprArg constraints, MultiExprArg Exprs,
                                 Expr *asmString, MultiExprArg clobbers,
                                 SourceLocation RParenLoc) {
  unsigned NumClobbers = clobbers.size();
  StringLiteral **Constraints =
    reinterpret_cast<StringLiteral**>(constraints.data());
  StringLiteral *AsmString = cast<StringLiteral>(asmString);
  StringLiteral **Clobbers = reinterpret_cast<StringLiteral**>(clobbers.data());

  SmallVector<TargetInfo::ConstraintInfo, 4> OutputConstraintInfos;

  // The parser verifies that there is a string literal here.
  assert(AsmString->isAscii());

  bool ValidateConstraints =
      DeclAttrsMatchCUDAMode(getLangOpts(), getCurFunctionDecl());

  for (unsigned i = 0; i != NumOutputs; i++) {
    StringLiteral *Literal = Constraints[i];
    assert(Literal->isAscii());

    StringRef OutputName;
    if (Names[i])
      OutputName = Names[i]->getName();

    TargetInfo::ConstraintInfo Info(Literal->getString(), OutputName);
    if (ValidateConstraints &&
        !Context.getTargetInfo().validateOutputConstraint(Info))
      return StmtError(Diag(Literal->getLocStart(),
                            diag::err_asm_invalid_output_constraint)
                       << Info.getConstraintStr());

    ExprResult ER = CheckPlaceholderExpr(Exprs[i]);
    if (ER.isInvalid())
      return StmtError();
    Exprs[i] = ER.get();

    // Check that the output exprs are valid lvalues.
    Expr *OutputExpr = Exprs[i];

    // Referring to parameters is not allowed in naked functions.
    if (CheckNakedParmReference(OutputExpr, *this))
      return StmtError();

    // Bitfield can't be referenced with a pointer.
    if (Info.allowsMemory() && OutputExpr->refersToBitField())
      return StmtError(Diag(OutputExpr->getLocStart(),
                            diag::err_asm_bitfield_in_memory_constraint)
                       << 1
                       << Info.getConstraintStr()
                       << OutputExpr->getSourceRange());

    OutputConstraintInfos.push_back(Info);

    // If this is dependent, just continue.
    if (OutputExpr->isTypeDependent())
      continue;

    Expr::isModifiableLvalueResult IsLV =
        OutputExpr->isModifiableLvalue(Context, /*Loc=*/nullptr);
    switch (IsLV) {
    case Expr::MLV_Valid:
      // Cool, this is an lvalue.
      break;
    case Expr::MLV_ArrayType:
      // This is OK too.
      break;
    case Expr::MLV_LValueCast: {
      const Expr *LVal = OutputExpr->IgnoreParenNoopCasts(Context);
      if (!getLangOpts().HeinousExtensions) {
        Diag(LVal->getLocStart(), diag::err_invalid_asm_cast_lvalue)
            << OutputExpr->getSourceRange();
      } else {
        Diag(LVal->getLocStart(), diag::warn_invalid_asm_cast_lvalue)
            << OutputExpr->getSourceRange();
      }
      // Accept, even if we emitted an error diagnostic.
      break;
    }
    case Expr::MLV_IncompleteType:
    case Expr::MLV_IncompleteVoidType:
      if (RequireCompleteType(OutputExpr->getLocStart(), Exprs[i]->getType(),
                              diag::err_dereference_incomplete_type))
        return StmtError();
      LLVM_FALLTHROUGH; // HLSL Change
    default:
      return StmtError(Diag(OutputExpr->getLocStart(),
                            diag::err_asm_invalid_lvalue_in_output)
                       << OutputExpr->getSourceRange());
    }

    unsigned Size = Context.getTypeSize(OutputExpr->getType());
    if (!Context.getTargetInfo().validateOutputSize(Literal->getString(),
                                                    Size))
      return StmtError(Diag(OutputExpr->getLocStart(),
                            diag::err_asm_invalid_output_size)
                       << Info.getConstraintStr());
  }

  SmallVector<TargetInfo::ConstraintInfo, 4> InputConstraintInfos;

  for (unsigned i = NumOutputs, e = NumOutputs + NumInputs; i != e; i++) {
    StringLiteral *Literal = Constraints[i];
    assert(Literal->isAscii());

    StringRef InputName;
    if (Names[i])
      InputName = Names[i]->getName();

    TargetInfo::ConstraintInfo Info(Literal->getString(), InputName);
    if (ValidateConstraints &&
        !Context.getTargetInfo().validateInputConstraint(
            OutputConstraintInfos.data(), NumOutputs, Info)) {
      return StmtError(Diag(Literal->getLocStart(),
                            diag::err_asm_invalid_input_constraint)
                       << Info.getConstraintStr());
    }

    ExprResult ER = CheckPlaceholderExpr(Exprs[i]);
    if (ER.isInvalid())
      return StmtError();
    Exprs[i] = ER.get();

    Expr *InputExpr = Exprs[i];

    // Referring to parameters is not allowed in naked functions.
    if (CheckNakedParmReference(InputExpr, *this))
      return StmtError();

    // Bitfield can't be referenced with a pointer.
    if (Info.allowsMemory() && InputExpr->refersToBitField())
      return StmtError(Diag(InputExpr->getLocStart(),
                            diag::err_asm_bitfield_in_memory_constraint)
                       << 0
                       << Info.getConstraintStr()
                       << InputExpr->getSourceRange());

    // Only allow void types for memory constraints.
    if (Info.allowsMemory() && !Info.allowsRegister()) {
      if (CheckAsmLValue(InputExpr, *this))
        return StmtError(Diag(InputExpr->getLocStart(),
                              diag::err_asm_invalid_lvalue_in_input)
                         << Info.getConstraintStr()
                         << InputExpr->getSourceRange());
    } else if (Info.requiresImmediateConstant() && !Info.allowsRegister()) {
      if (!InputExpr->isValueDependent()) {
        llvm::APSInt Result;
        if (!InputExpr->EvaluateAsInt(Result, Context))
           return StmtError(
               Diag(InputExpr->getLocStart(), diag::err_asm_immediate_expected)
                << Info.getConstraintStr() << InputExpr->getSourceRange());
         if (Result.slt(Info.getImmConstantMin()) ||
             Result.sgt(Info.getImmConstantMax()))
           return StmtError(Diag(InputExpr->getLocStart(),
                                 diag::err_invalid_asm_value_for_constraint)
                            << Result.toString(10) << Info.getConstraintStr()
                            << InputExpr->getSourceRange());
      }

    } else {
      ExprResult Result = DefaultFunctionArrayLvalueConversion(Exprs[i]);
      if (Result.isInvalid())
        return StmtError();

      Exprs[i] = Result.get();
    }

    if (Info.allowsRegister()) {
      if (InputExpr->getType()->isVoidType()) {
        return StmtError(Diag(InputExpr->getLocStart(),
                              diag::err_asm_invalid_type_in_input)
          << InputExpr->getType() << Info.getConstraintStr()
          << InputExpr->getSourceRange());
      }
    }

    InputConstraintInfos.push_back(Info);

    const Type *Ty = Exprs[i]->getType().getTypePtr();
    if (Ty->isDependentType())
      continue;

    if (!Ty->isVoidType() || !Info.allowsMemory())
      if (RequireCompleteType(InputExpr->getLocStart(), Exprs[i]->getType(),
                              diag::err_dereference_incomplete_type))
        return StmtError();

    unsigned Size = Context.getTypeSize(Ty);
    if (!Context.getTargetInfo().validateInputSize(Literal->getString(),
                                                   Size))
      return StmtError(Diag(InputExpr->getLocStart(),
                            diag::err_asm_invalid_input_size)
                       << Info.getConstraintStr());
  }

  // Check that the clobbers are valid.
  for (unsigned i = 0; i != NumClobbers; i++) {
    StringLiteral *Literal = Clobbers[i];
    assert(Literal->isAscii());

    StringRef Clobber = Literal->getString();

    if (!Context.getTargetInfo().isValidClobber(Clobber))
      return StmtError(Diag(Literal->getLocStart(),
                  diag::err_asm_unknown_register_name) << Clobber);
  }

  GCCAsmStmt *NS =
    new (Context) GCCAsmStmt(Context, AsmLoc, IsSimple, IsVolatile, NumOutputs,
                             NumInputs, Names, Constraints, Exprs.data(),
                             AsmString, NumClobbers, Clobbers, RParenLoc);
  // Validate the asm string, ensuring it makes sense given the operands we
  // have.
  SmallVector<GCCAsmStmt::AsmStringPiece, 8> Pieces;
  unsigned DiagOffs;
  if (unsigned DiagID = NS->AnalyzeAsmString(Pieces, Context, DiagOffs)) {
    Diag(getLocationOfStringLiteralByte(AsmString, DiagOffs), DiagID)
           << AsmString->getSourceRange();
    return StmtError();
  }

  // Validate constraints and modifiers.
  for (unsigned i = 0, e = Pieces.size(); i != e; ++i) {
    GCCAsmStmt::AsmStringPiece &Piece = Pieces[i];
    if (!Piece.isOperand()) continue;

    // Look for the correct constraint index.
    unsigned ConstraintIdx = Piece.getOperandNo();
    unsigned NumOperands = NS->getNumOutputs() + NS->getNumInputs();

    // Look for the (ConstraintIdx - NumOperands + 1)th constraint with
    // modifier '+'.
    if (ConstraintIdx >= NumOperands) {
      unsigned I = 0, E = NS->getNumOutputs();

      for (unsigned Cnt = ConstraintIdx - NumOperands; I != E; ++I)
        if (OutputConstraintInfos[I].isReadWrite() && Cnt-- == 0) {
          ConstraintIdx = I;
          break;
        }

      assert(I != E && "Invalid operand number should have been caught in "
                       " AnalyzeAsmString");
    }

    // Now that we have the right indexes go ahead and check.
    StringLiteral *Literal = Constraints[ConstraintIdx];
    const Type *Ty = Exprs[ConstraintIdx]->getType().getTypePtr();
    if (Ty->isDependentType() || Ty->isIncompleteType())
      continue;

    unsigned Size = Context.getTypeSize(Ty);
    std::string SuggestedModifier;
    if (!Context.getTargetInfo().validateConstraintModifier(
            Literal->getString(), Piece.getModifier(), Size,
            SuggestedModifier)) {
      Diag(Exprs[ConstraintIdx]->getLocStart(),
           diag::warn_asm_mismatched_size_modifier);

      if (!SuggestedModifier.empty()) {
        auto B = Diag(Piece.getRange().getBegin(),
                      diag::note_asm_missing_constraint_modifier)
                 << SuggestedModifier;
        SuggestedModifier = "%" + SuggestedModifier + Piece.getString();
        B.AddFixItHint(FixItHint::CreateReplacement(Piece.getRange(),
                                                    SuggestedModifier));
      }
    }
  }

  // Validate tied input operands for type mismatches.
  unsigned NumAlternatives = ~0U;
  for (unsigned i = 0, e = OutputConstraintInfos.size(); i != e; ++i) {
    TargetInfo::ConstraintInfo &Info = OutputConstraintInfos[i];
    StringRef ConstraintStr = Info.getConstraintStr();
    unsigned AltCount = ConstraintStr.count(',') + 1;
    if (NumAlternatives == ~0U)
      NumAlternatives = AltCount;
    else if (NumAlternatives != AltCount)
      return StmtError(Diag(NS->getOutputExpr(i)->getLocStart(),
                            diag::err_asm_unexpected_constraint_alternatives)
                       << NumAlternatives << AltCount);
  }
  for (unsigned i = 0, e = InputConstraintInfos.size(); i != e; ++i) {
    TargetInfo::ConstraintInfo &Info = InputConstraintInfos[i];
    StringRef ConstraintStr = Info.getConstraintStr();
    unsigned AltCount = ConstraintStr.count(',') + 1;
    if (NumAlternatives == ~0U)
      NumAlternatives = AltCount;
    else if (NumAlternatives != AltCount)
      return StmtError(Diag(NS->getInputExpr(i)->getLocStart(),
                            diag::err_asm_unexpected_constraint_alternatives)
                       << NumAlternatives << AltCount);

    // If this is a tied constraint, verify that the output and input have
    // either exactly the same type, or that they are int/ptr operands with the
    // same size (int/long, int*/long, are ok etc).
    if (!Info.hasTiedOperand()) continue;

    unsigned TiedTo = Info.getTiedOperand();
    unsigned InputOpNo = i+NumOutputs;
    Expr *OutputExpr = Exprs[TiedTo];
    Expr *InputExpr = Exprs[InputOpNo];

    if (OutputExpr->isTypeDependent() || InputExpr->isTypeDependent())
      continue;

    QualType InTy = InputExpr->getType();
    QualType OutTy = OutputExpr->getType();
    if (Context.hasSameType(InTy, OutTy))
      continue;  // All types can be tied to themselves.

    // Decide if the input and output are in the same domain (integer/ptr or
    // floating point.
    enum AsmDomain {
      AD_Int, AD_FP, AD_Other
    } InputDomain, OutputDomain;

    if (InTy->isIntegerType() || InTy->isPointerType())
      InputDomain = AD_Int;
    else if (InTy->isRealFloatingType())
      InputDomain = AD_FP;
    else
      InputDomain = AD_Other;

    if (OutTy->isIntegerType() || OutTy->isPointerType())
      OutputDomain = AD_Int;
    else if (OutTy->isRealFloatingType())
      OutputDomain = AD_FP;
    else
      OutputDomain = AD_Other;

    // They are ok if they are the same size and in the same domain.  This
    // allows tying things like:
    //   void* to int*
    //   void* to int            if they are the same size.
    //   double to long double   if they are the same size.
    //
    uint64_t OutSize = Context.getTypeSize(OutTy);
    uint64_t InSize = Context.getTypeSize(InTy);
    if (OutSize == InSize && InputDomain == OutputDomain &&
        InputDomain != AD_Other)
      continue;

    // If the smaller input/output operand is not mentioned in the asm string,
    // then we can promote the smaller one to a larger input and the asm string
    // won't notice.
    bool SmallerValueMentioned = false;

    // If this is a reference to the input and if the input was the smaller
    // one, then we have to reject this asm.
    if (isOperandMentioned(InputOpNo, Pieces)) {
      // This is a use in the asm string of the smaller operand.  Since we
      // codegen this by promoting to a wider value, the asm will get printed
      // "wrong".
      SmallerValueMentioned |= InSize < OutSize;
    }
    if (isOperandMentioned(TiedTo, Pieces)) {
      // If this is a reference to the output, and if the output is the larger
      // value, then it's ok because we'll promote the input to the larger type.
      SmallerValueMentioned |= OutSize < InSize;
    }

    // If the smaller value wasn't mentioned in the asm string, and if the
    // output was a register, just extend the shorter one to the size of the
    // larger one.
    if (!SmallerValueMentioned && InputDomain != AD_Other &&
        OutputConstraintInfos[TiedTo].allowsRegister())
      continue;

    // Either both of the operands were mentioned or the smaller one was
    // mentioned.  One more special case that we'll allow: if the tied input is
    // integer, unmentioned, and is a constant, then we'll allow truncating it
    // down to the size of the destination.
    if (InputDomain == AD_Int && OutputDomain == AD_Int &&
        !isOperandMentioned(InputOpNo, Pieces) &&
        InputExpr->isEvaluatable(Context)) {
      CastKind castKind =
        (OutTy->isBooleanType() ? CK_IntegralToBoolean : CK_IntegralCast);
      InputExpr = ImpCastExprToType(InputExpr, OutTy, castKind).get();
      Exprs[InputOpNo] = InputExpr;
      NS->setInputExpr(i, InputExpr);
      continue;
    }

    Diag(InputExpr->getLocStart(),
         diag::err_asm_tying_incompatible_types)
      << InTy << OutTy << OutputExpr->getSourceRange()
      << InputExpr->getSourceRange();
    return StmtError();
  }

  return NS;
}

ExprResult Sema::LookupInlineAsmIdentifier(CXXScopeSpec &SS,
                                           SourceLocation TemplateKWLoc,
                                           UnqualifiedId &Id,
                                           llvm::InlineAsmIdentifierInfo &Info,
                                           bool IsUnevaluatedContext) {
  Info.clear();

  if (IsUnevaluatedContext)
    PushExpressionEvaluationContext(UnevaluatedAbstract,
                                    ReuseLambdaContextDecl);

  ExprResult Result = ActOnIdExpression(getCurScope(), SS, TemplateKWLoc, Id,
                                        /*trailing lparen*/ false,
                                        /*is & operand*/ false,
                                        /*CorrectionCandidateCallback=*/nullptr,
                                        /*IsInlineAsmIdentifier=*/ true);

  if (IsUnevaluatedContext)
    PopExpressionEvaluationContext();

  if (!Result.isUsable()) return Result;

  Result = CheckPlaceholderExpr(Result.get());
  if (!Result.isUsable()) return Result;

  // Referring to parameters is not allowed in naked functions.
  if (CheckNakedParmReference(Result.get(), *this))
    return ExprError();

  QualType T = Result.get()->getType();

  // For now, reject dependent types.
  if (T->isDependentType()) {
    Diag(Id.getLocStart(), diag::err_asm_incomplete_type) << T;
    return ExprError();
  }

  // Any sort of function type is fine.
  if (T->isFunctionType()) {
    return Result;
  }

  // Otherwise, it needs to be a complete type.
  if (RequireCompleteExprType(Result.get(), diag::err_asm_incomplete_type)) {
    return ExprError();
  }

  // Compute the type size (and array length if applicable?).
  Info.Type = Info.Size = Context.getTypeSizeInChars(T).getQuantity();
  if (T->isArrayType()) {
    const ArrayType *ATy = Context.getAsArrayType(T);
    Info.Type = Context.getTypeSizeInChars(ATy->getElementType()).getQuantity();
    Info.Length = Info.Size / Info.Type;
  }

  // We can work with the expression as long as it's not an r-value.
  if (!Result.get()->isRValue())
    Info.IsVarDecl = true;

  return Result;
}

bool Sema::LookupInlineAsmField(StringRef Base, StringRef Member,
                                unsigned &Offset, SourceLocation AsmLoc) {
  Offset = 0;
  LookupResult BaseResult(*this, &Context.Idents.get(Base), SourceLocation(),
                          LookupOrdinaryName);

  if (!LookupName(BaseResult, getCurScope()))
    return true;

  if (!BaseResult.isSingleResult())
    return true;

  const RecordType *RT = nullptr;
  NamedDecl *FoundDecl = BaseResult.getFoundDecl();
  if (VarDecl *VD = dyn_cast<VarDecl>(FoundDecl))
    RT = VD->getType()->getAs<RecordType>();
  else if (TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(FoundDecl)) {
    MarkAnyDeclReferenced(TD->getLocation(), TD, /*OdrUse=*/false);
    RT = TD->getUnderlyingType()->getAs<RecordType>();
  } else if (TypeDecl *TD = dyn_cast<TypeDecl>(FoundDecl))
    RT = TD->getTypeForDecl()->getAs<RecordType>();
  if (!RT)
    return true;

  if (RequireCompleteType(AsmLoc, QualType(RT, 0), 0))
    return true;

  LookupResult FieldResult(*this, &Context.Idents.get(Member), SourceLocation(),
                           LookupMemberName);

  if (!LookupQualifiedName(FieldResult, RT->getDecl()))
    return true;

  // FIXME: Handle IndirectFieldDecl?
  FieldDecl *FD = dyn_cast<FieldDecl>(FieldResult.getFoundDecl());
  if (!FD)
    return true;

  const ASTRecordLayout &RL = Context.getASTRecordLayout(RT->getDecl());
  unsigned i = FD->getFieldIndex();
  CharUnits Result = Context.toCharUnitsFromBits(RL.getFieldOffset(i));
  Offset = (unsigned)Result.getQuantity();

  return false;
}

StmtResult Sema::ActOnMSAsmStmt(SourceLocation AsmLoc, SourceLocation LBraceLoc,
                                ArrayRef<Token> AsmToks,
                                StringRef AsmString,
                                unsigned NumOutputs, unsigned NumInputs,
                                ArrayRef<StringRef> Constraints,
                                ArrayRef<StringRef> Clobbers,
                                ArrayRef<Expr*> Exprs,
                                SourceLocation EndLoc) {
  bool IsSimple = (NumOutputs != 0 || NumInputs != 0);
  getCurFunction()->setHasBranchProtectedScope();
  MSAsmStmt *NS =
    new (Context) MSAsmStmt(Context, AsmLoc, LBraceLoc, IsSimple,
                            /*IsVolatile*/ true, AsmToks, NumOutputs, NumInputs,
                            Constraints, Exprs, AsmString,
                            Clobbers, EndLoc);
  return NS;
}

LabelDecl *Sema::GetOrCreateMSAsmLabel(StringRef ExternalLabelName,
                                       SourceLocation Location,
                                       bool AlwaysCreate) {
  LabelDecl* Label = LookupOrCreateLabel(PP.getIdentifierInfo(ExternalLabelName),
                                         Location);

  if (Label->isMSAsmLabel()) {
    // If we have previously created this label implicitly, mark it as used.
    Label->markUsed(Context);
  } else {
    // Otherwise, insert it, but only resolve it if we have seen the label itself.
    std::string InternalName;
    llvm::raw_string_ostream OS(InternalName);
    // Create an internal name for the label.  The name should not be a valid mangled
    // name, and should be unique.  We use a dot to make the name an invalid mangled
    // name.
    OS << "__MSASMLABEL_." << MSAsmLabelNameCounter++ << "__" << ExternalLabelName;
    Label->setMSAsmLabel(OS.str());
  }
  if (AlwaysCreate) {
    // The label might have been created implicitly from a previously encountered
    // goto statement.  So, for both newly created and looked up labels, we mark
    // them as resolved.
    Label->setMSAsmLabelResolved();
  }
  // Adjust their location for being able to generate accurate diagnostics.
  Label->setLocation(Location);

  return Label;
}
