//===--- DebugTypeVisitor.cpp - SPIR-V type to debug type impl ---*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <sstream>

#include "DebugTypeVisitor.h"
#include "LowerTypeVisitor.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/SPIRV/SpirvBuilder.h"
#include "clang/SPIRV/SpirvModule.h"

namespace clang {
namespace spirv {

void addTemplateTypeAndItsParamsToModule(SpirvModule *module,
                                         SpirvDebugTypeTemplate *tempType) {
  assert(module && "module is nullptr");
  assert(tempType && "tempType is nullptr");

  for (auto *param : tempType->getParams()) {
    module->addDebugInfo(param);
  }
  module->addDebugInfo(tempType);
}

void DebugTypeVisitor::setDefaultDebugInfo(SpirvDebugInstruction *instr) {
  instr->setAstResultType(astContext.VoidTy);
  instr->setResultType(context.getVoidType());
  instr->setInstructionSet(
      spvBuilder.getDebugInfoExtInstSet(spvOptions.debugInfoVulkan));
}

SpirvDebugInfoNone *DebugTypeVisitor::getDebugInfoNone() {
  auto *debugNone = spvBuilder.getOrCreateDebugInfoNone();
  setDefaultDebugInfo(debugNone);
  return debugNone;
}

SpirvDebugTypeComposite *DebugTypeVisitor::createDebugTypeComposite(
    const SpirvType *type, const SourceLocation &loc, uint32_t tag) {
  const auto &sm = astContext.getSourceManager();
  uint32_t line = sm.getPresumedLineNumber(loc);
  uint32_t column = sm.getPresumedColumnNumber(loc);
  StringRef linkageName = type->getName();

  // TODO: Update linkageName using astContext.createMangleContext().
  std::string name = type->getName();

  RichDebugInfo *debugInfo = &spvContext.getDebugInfo().begin()->second;
  const char *file = sm.getPresumedLoc(loc).getFilename();
  if (file) {
    auto &debugInfoMap = spvContext.getDebugInfo();
    auto it = debugInfoMap.find(file);
    if (it != debugInfoMap.end()) {
      debugInfo = &it->second;
    } else {
      auto *dbgSrc = spvBuilder.createDebugSource(file);
      setDefaultDebugInfo(dbgSrc);
      auto dbgCompUnit = spvBuilder.getModule()->getDebugCompilationUnit();
      setDefaultDebugInfo(dbgCompUnit);
      debugInfo =
          &debugInfoMap.insert({file, RichDebugInfo(dbgSrc, dbgCompUnit)})
               .first->second;
    }
  }
  return spvContext.getDebugTypeComposite(
      type, name, debugInfo->source, line, column,
      /* parent */ debugInfo->compilationUnit, linkageName, 3u, tag);
}

void DebugTypeVisitor::addDebugTypeForMemberVariables(
    SpirvDebugTypeComposite *debugTypeComposite, const StructType *type,
    llvm::function_ref<SourceLocation()> location, unsigned numBases) {
  llvm::SmallVector<SpirvDebugInstruction *, 4> members;
  uint32_t compositeSizeInBits = 0;
  const auto &sm = astContext.getSourceManager();
  for (auto &field : type->getFields()) {
    // Skip base classes
    // TODO: Handle class inheritance correctly.
    if (numBases != 0) {
      --numBases;
      continue;
    }

    // Lower this member's debug type.
    auto *memberDebugType = lowerToDebugType(field.type);

    // TODO: We are currently in the discussion about how to handle
    // a variable type with unknown physical layout. Add proper flags
    // or operations for variables with the unknown physical layout.
    // For example, we do not have physical layout for a local variable.

    // Get offset (in bits) of this member within the composite.
    uint32_t offsetInBits =
        field.offset.hasValue() ? *field.offset * 8 : compositeSizeInBits;
    // Get size (in bits) of this member within the composite.
    uint32_t sizeInBits = field.sizeInBytes.hasValue()
                              ? *field.sizeInBytes * 8
                              : memberDebugType->getSizeInBits();

    const SourceLocation loc = location();
    uint32_t line = sm.getPresumedLineNumber(loc);
    uint32_t column = sm.getPresumedColumnNumber(loc);

    // TODO: Replace 2u and 3u with valid flags when debug info extension is
    // placed in SPIRV-Header.
    auto *debugInstr = spvContext.getDebugTypeMember(
        field.name, memberDebugType, debugTypeComposite->getSource(), line,
        column, debugTypeComposite,
        /* flags */ 3u, offsetInBits, sizeInBits, /* value */ nullptr);
    assert(debugInstr);

    setDefaultDebugInfo(debugInstr);
    members.push_back(debugInstr);

    compositeSizeInBits = offsetInBits + sizeInBits;
  }
  debugTypeComposite->setMembers(members);
  debugTypeComposite->setSizeInBits(compositeSizeInBits);
}

void DebugTypeVisitor::lowerDebugTypeMembers(
    SpirvDebugTypeComposite *debugTypeComposite, const StructType *type,
    const DeclContext *decl) {
  if (const auto *recordDecl = dyn_cast<RecordDecl>(decl)) {
    auto fieldIter = recordDecl->field_begin();
    auto fieldEnd = recordDecl->field_end();
    unsigned numBases = 0;
    if (const auto *cxxRecordDecl = dyn_cast<CXXRecordDecl>(recordDecl))
      numBases = cxxRecordDecl->getNumBases();
    addDebugTypeForMemberVariables(
        debugTypeComposite, type,
        [&fieldIter, &fieldEnd]() {
          assert(fieldIter != fieldEnd);
          (void)fieldEnd;
          auto location = fieldIter->getLocation();
          ++fieldIter;
          return location;
        },
        numBases);
  } else if (const auto *hlslBufferDecl = dyn_cast<HLSLBufferDecl>(decl)) {
    auto subDeclIter = hlslBufferDecl->decls_begin();
    auto subDeclEnd = hlslBufferDecl->decls_end();
    addDebugTypeForMemberVariables(
        debugTypeComposite, type,
        [&subDeclIter, &subDeclEnd]() {
          assert(subDeclIter != subDeclEnd);
          (void)subDeclEnd;
          auto location = subDeclIter->getLocation();
          ++subDeclIter;
          return location;
        },
        0);
  } else {
    assert(false && "Uknown DeclContext for DebugTypeMember generation");
  }

  // Note:
  //    Generating forward references is possible for non-semantic debug info,
  //    but not when using OpenCL.DebugInfo.100.
  //    Doing so would go against the SPIR-V spec.
  //    See https://github.com/KhronosGroup/SPIRV-Registry/issues/203
  if (!spvOptions.debugInfoVulkan)
    return;

  // Push member functions to DebugTypeComposite Members operand.
  for (auto *subDecl : decl->decls()) {
    if (const auto *methodDecl = dyn_cast<FunctionDecl>(subDecl)) {
      // TODO: if dbgFunction is NULL, it is a member function without
      // function calls. We have to generate its type and insert it to
      // members.
      if (auto *dbgFunction = spvContext.getDebugFunctionForDecl(methodDecl)) {
        dbgFunction->setParent(debugTypeComposite);
        debugTypeComposite->appendMember(dbgFunction);
      }
    }
  }
}

SpirvDebugTypeTemplate *DebugTypeVisitor::lowerDebugTypeTemplate(
    const ClassTemplateSpecializationDecl *templateDecl,
    SpirvDebugTypeComposite *debugTypeComposite) {
  // Reuse already lowered DebugTypeTemplate.
  auto *debugTypeTemplate = spvContext.getDebugTypeTemplate(templateDecl);
  if (debugTypeTemplate != nullptr)
    return debugTypeTemplate;

  llvm::SmallVector<SpirvDebugTypeTemplateParameter *, 2> tempTypeParams;
  const auto &argList = templateDecl->getTemplateArgs();
  for (unsigned i = 0; i < argList.size(); ++i) {
    // Reuse already lowered DebugTypeTemplateParameter.
    auto *debugTypeTemplateParam =
        spvContext.getDebugTypeTemplateParameter(&argList[i]);
    if (debugTypeTemplateParam != nullptr) {
      tempTypeParams.push_back(debugTypeTemplateParam);
      continue;
    }

    // TODO: Handle other kinds e.g., value, template template type.
    if (argList[i].getKind() != clang::TemplateArgument::ArgKind::Type)
      continue;

    // Lower DebugTypeTemplateParameter.
    const auto *spvType = spvTypeVisitor.lowerType(
        argList[i].getAsType(), currentDebugInstructionLayoutRule, llvm::None,
        debugTypeComposite->getSourceLocation());
    debugTypeTemplateParam = spvContext.createDebugTypeTemplateParameter(
        &argList[i], "TemplateParam", lowerToDebugType(spvType),
        getDebugInfoNone(), debugTypeComposite->getSource(),
        debugTypeComposite->getLine(), debugTypeComposite->getColumn());
    tempTypeParams.push_back(debugTypeTemplateParam);
    setDefaultDebugInfo(debugTypeTemplateParam);
  }

  debugTypeTemplate = spvContext.createDebugTypeTemplate(
      templateDecl, debugTypeComposite, tempTypeParams);
  setDefaultDebugInfo(debugTypeTemplate);
  return debugTypeTemplate;
}

SpirvDebugType *
DebugTypeVisitor::lowerToDebugTypeComposite(const SpirvType *type) {
  const auto *decl = spvContext.getStructDeclForSpirvType(type);
  assert(decl != nullptr && "Lowering DebugTypeComposite needs DeclContext");

  uint32_t tag = 1u;
  if (const auto *recordDecl = dyn_cast<RecordDecl>(decl)) {
    if (recordDecl->isStruct())
      tag = 1;
    else if (recordDecl->isClass())
      tag = 0;
    else if (recordDecl->isUnion())
      tag = 2;
    else
      assert(!"DebugTypeComposite must be a struct, class, or union.");
  }
  SourceLocation loc = {};
  if (const auto *declDecl = dyn_cast<Decl>(decl))
    loc = declDecl->getLocation();
  auto *debugTypeComposite = createDebugTypeComposite(type, loc, tag);
  setDefaultDebugInfo(debugTypeComposite);

  if (const auto *templateDecl =
          dyn_cast<ClassTemplateSpecializationDecl>(decl)) {
    // The size of an opaque type must be DebugInfoNone and its name must
    // start with "@".
    debugTypeComposite->markAsOpaqueType(getDebugInfoNone());
    return lowerDebugTypeTemplate(templateDecl, debugTypeComposite);
  } else {
    // If SpirvType is StructType, it is a normal struct/class. Otherwise,
    // it must be an image or a sampler type that is an opaque type.
    if (const StructType *structType = dyn_cast<StructType>(type))
      lowerDebugTypeMembers(debugTypeComposite, structType, decl);
    else
      debugTypeComposite->markAsOpaqueType(getDebugInfoNone());
    return debugTypeComposite;
  }
}

SpirvDebugType *DebugTypeVisitor::lowerToDebugType(const SpirvType *spirvType) {
  SpirvDebugType *debugType = nullptr;

  switch (spirvType->getKind()) {
  case SpirvType::TK_Bool: {
    llvm::StringRef name = "bool";
    // TODO: Should we use 1 bit for booleans or 32 bits?
    uint32_t size = 32;
    // TODO: Use enums rather than uint32_t.
    uint32_t encoding = 2u;
    SpirvConstant *sizeInstruction = spvBuilder.getConstantInt(
        astContext.UnsignedIntTy, llvm::APInt(32, size));
    sizeInstruction->setResultType(spvContext.getUIntType(32));
    debugType = spvContext.getDebugTypeBasic(spirvType, name, sizeInstruction,
                                             encoding);
    break;
  }
  case SpirvType::TK_Integer: {
    auto *intType = dyn_cast<IntegerType>(spirvType);
    const uint32_t size = intType->getBitwidth();
    const bool isSigned = intType->isSignedInt();
    SpirvConstant *sizeInstruction = spvBuilder.getConstantInt(
        astContext.UnsignedIntTy, llvm::APInt(32, size));
    sizeInstruction->setResultType(spvContext.getUIntType(32));
    // TODO: Use enums rather than uint32_t.
    uint32_t encoding = isSigned ? 4u : 6u;
    std::string debugName = "";
    if (size == 32) {
      debugName = isSigned ? "int" : "uint";
    } else {
      std::ostringstream stream;
      stream << (isSigned ? "int" : "uint") << size << "_t";
      debugName = stream.str();
    }
    debugType = spvContext.getDebugTypeBasic(spirvType, debugName,
                                             sizeInstruction, encoding);
    break;
  }
  case SpirvType::TK_Float: {
    auto *floatType = dyn_cast<FloatType>(spirvType);
    const uint32_t size = floatType->getBitwidth();
    SpirvConstant *sizeInstruction = spvBuilder.getConstantInt(
        astContext.UnsignedIntTy, llvm::APInt(32, size));
    sizeInstruction->setResultType(spvContext.getUIntType(32));
    // TODO: Use enums rather than uint32_t.
    uint32_t encoding = 3u;
    std::string debugName = "";
    if (size == 32) {
      debugName = "float";
    } else {
      std::ostringstream stream;
      stream << "float" << size << "_t";
      debugName = stream.str();
    }
    debugType = spvContext.getDebugTypeBasic(spirvType, debugName,
                                             sizeInstruction, encoding);
    break;
  }
  case SpirvType::TK_Image:
  case SpirvType::TK_Sampler:
  case SpirvType::TK_Struct: {
    debugType = lowerToDebugTypeComposite(spirvType);
    break;
  }
  // TODO: Add DebugTypeComposite for class and union.
  // TODO: Add DebugTypeEnum.
  case SpirvType::TK_Array: {
    auto *arrType = dyn_cast<ArrayType>(spirvType);
    SpirvDebugInstruction *elemDebugType =
        lowerToDebugType(arrType->getElementType());

    llvm::SmallVector<uint32_t, 4> counts;
    if (auto *dbgArrType = dyn_cast<SpirvDebugTypeArray>(elemDebugType)) {
      counts.insert(counts.end(), dbgArrType->getElementCount().begin(),
                    dbgArrType->getElementCount().end());
      elemDebugType = dbgArrType->getElementType();
    }
    counts.push_back(arrType->getElementCount());

    debugType = spvContext.getDebugTypeArray(spirvType, elemDebugType, counts);
    break;
  }
  case SpirvType::TK_RuntimeArray: {
    auto *arrType = dyn_cast<RuntimeArrayType>(spirvType);
    SpirvDebugInstruction *elemDebugType =
        lowerToDebugType(arrType->getElementType());

    llvm::SmallVector<uint32_t, 4> counts;
    counts.push_back(0u);

    debugType = spvContext.getDebugTypeArray(spirvType, elemDebugType, counts);
    break;
  }
  case SpirvType::TK_NodePayloadArrayAMD: {
    auto *arrType = dyn_cast<NodePayloadArrayType>(spirvType);
    SpirvDebugInstruction *elemDebugType =
        lowerToDebugType(arrType->getElementType());

    llvm::SmallVector<uint32_t, 4> counts;
    counts.push_back(0u);

    debugType = spvContext.getDebugTypeArray(spirvType, elemDebugType, counts);
    break;
  }
  case SpirvType::TK_Vector: {
    auto *vecType = dyn_cast<VectorType>(spirvType);
    SpirvDebugInstruction *elemDebugType =
        lowerToDebugType(vecType->getElementType());
    debugType = spvContext.getDebugTypeVector(spirvType, elemDebugType,
                                              vecType->getElementCount());
    break;
  }
  case SpirvType::TK_Matrix: {
    auto *matType = dyn_cast<MatrixType>(spirvType);
    if (spvOptions.debugInfoVulkan) {
      SpirvDebugInstruction *vecDebugType =
          lowerToDebugType(matType->getVecType());
      debugType = spvContext.getDebugTypeMatrix(spirvType, vecDebugType,
                                                matType->numCols());
    } else {
      SpirvDebugInstruction *elemDebugType =
          lowerToDebugType(matType->getElementType());
      debugType = spvContext.getDebugTypeArray(
          spirvType, elemDebugType, {matType->numRows(), matType->numCols()});
    }
    break;
  }
  case SpirvType::TK_Pointer: {
    debugType = lowerToDebugType(
        dyn_cast<SpirvPointerType>(spirvType)->getPointeeType());
    break;
  }
  case SpirvType::TK_Function: {
    auto *fnType = dyn_cast<FunctionType>(spirvType);
    // Special case: There is no DebugType for void. So if the function return
    // type is void, we set it to nullptr.
    SpirvDebugType *returnType = nullptr;
    if (!isa<VoidType>(fnType->getReturnType())) {
      auto *loweredRetTy = lowerToDebugType(fnType->getReturnType());
      returnType = dyn_cast<SpirvDebugType>(loweredRetTy);
      assert(returnType && "Function return type info must be SpirvDebugType");
    }
    llvm::SmallVector<SpirvDebugType *, 4> params;
    for (const auto *paramType : fnType->getParamTypes()) {
      params.push_back(dyn_cast<SpirvDebugType>(lowerToDebugType(paramType)));
    }
    // TODO: Add mechanism to properly calculate the flags.
    // The info needed probably resides in clang::FunctionDecl.
    // This info can either be stored in the SpirvFunction class. Or,
    // alternatively the info can be stored in the SpirvContext.
    const uint32_t flags = 3u;
    debugType =
        spvContext.getDebugTypeFunction(spirvType, flags, returnType, params);
    break;
  }
  case SpirvType::TK_AccelerationStructureNV: {
    debugType = lowerToDebugTypeComposite(spirvType);
    break;
  }
  }

  if (!debugType) {
    emitError("Fail to lower SpirvType %0 to a debug type")
        << spirvType->getName();
    return nullptr;
  }

  setDefaultDebugInfo(debugType);
  return debugType;
}

bool DebugTypeVisitor::visitInstruction(SpirvInstruction *instr) {
  if (auto *debugInstr = dyn_cast<SpirvDebugInstruction>(instr)) {
    setDefaultDebugInfo(debugInstr);

    // The following instructions are the only debug instructions that contain a
    // debug type:
    // DebugGlobalVariable
    // DebugLocalVariable
    // DebugFunction
    if (isa<SpirvDebugGlobalVariable>(debugInstr) ||
        isa<SpirvDebugLocalVariable>(debugInstr)) {
      currentDebugInstructionLayoutRule = debugInstr->getLayoutRule();
      const SpirvType *spirvType = debugInstr->getDebugSpirvType();
      assert(spirvType != nullptr);
      SpirvDebugInstruction *debugType = lowerToDebugType(spirvType);
      debugInstr->setDebugType(debugType);
    }
    if (auto *debugFunction = dyn_cast<SpirvDebugFunction>(debugInstr)) {
      currentDebugInstructionLayoutRule = SpirvLayoutRule::Void;
      const SpirvType *spirvType =
          debugFunction->getSpirvFunction()->getFunctionType();
      if (spirvType) {
        SpirvDebugInstruction *debugType = lowerToDebugType(spirvType);
        debugInstr->setDebugType(debugType);
      }
    }
  }

  return true;
}

bool DebugTypeVisitor::visit(SpirvModule *module, Phase phase) {
  if (phase == Phase::Done)
    spvContext.moveDebugTypesToModule(module);
  return true;
}

} // namespace spirv
} // namespace clang
