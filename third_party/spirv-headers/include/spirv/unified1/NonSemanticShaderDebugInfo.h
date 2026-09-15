// SPDX-FileCopyrightText: 2018-2026 The Khronos Group Inc.
// SPDX-License-Identifier: MIT
//
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/

#ifndef SPIRV_UNIFIED1_NonSemanticShaderDebugInfo_H_
#define SPIRV_UNIFIED1_NonSemanticShaderDebugInfo_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    NonSemanticShaderDebugInfoVersion = 101,
    NonSemanticShaderDebugInfoVersion_BitWidthPadding = 0x7fffffff
};
enum {
    NonSemanticShaderDebugInfoRevision = 1,
    NonSemanticShaderDebugInfoRevision_BitWidthPadding = 0x7fffffff
};

enum NonSemanticShaderDebugInfoInstructions {
    NonSemanticShaderDebugInfoDebugInfoNone = 0,
    NonSemanticShaderDebugInfoDebugCompilationUnit = 1,
    NonSemanticShaderDebugInfoDebugTypeBasic = 2,
    NonSemanticShaderDebugInfoDebugTypePointer = 3,
    NonSemanticShaderDebugInfoDebugTypeQualifier = 4,
    NonSemanticShaderDebugInfoDebugTypeArray = 5,
    NonSemanticShaderDebugInfoDebugTypeVector = 6,
    NonSemanticShaderDebugInfoDebugTypedef = 7,
    NonSemanticShaderDebugInfoDebugTypeFunction = 8,
    NonSemanticShaderDebugInfoDebugTypeEnum = 9,
    NonSemanticShaderDebugInfoDebugTypeComposite = 10,
    NonSemanticShaderDebugInfoDebugTypeMember = 11,
    NonSemanticShaderDebugInfoDebugTypeInheritance = 12,
    NonSemanticShaderDebugInfoDebugTypePtrToMember = 13,
    NonSemanticShaderDebugInfoDebugTypeTemplate = 14,
    NonSemanticShaderDebugInfoDebugTypeTemplateParameter = 15,
    NonSemanticShaderDebugInfoDebugTypeTemplateTemplateParameter = 16,
    NonSemanticShaderDebugInfoDebugTypeTemplateParameterPack = 17,
    NonSemanticShaderDebugInfoDebugGlobalVariable = 18,
    NonSemanticShaderDebugInfoDebugFunctionDeclaration = 19,
    NonSemanticShaderDebugInfoDebugFunction = 20,
    NonSemanticShaderDebugInfoDebugLexicalBlock = 21,
    NonSemanticShaderDebugInfoDebugLexicalBlockDiscriminator = 22,
    NonSemanticShaderDebugInfoDebugScope = 23,
    NonSemanticShaderDebugInfoDebugNoScope = 24,
    NonSemanticShaderDebugInfoDebugInlinedAt = 25,
    NonSemanticShaderDebugInfoDebugLocalVariable = 26,
    NonSemanticShaderDebugInfoDebugInlinedVariable = 27,
    NonSemanticShaderDebugInfoDebugDeclare = 28,
    NonSemanticShaderDebugInfoDebugValue = 29,
    NonSemanticShaderDebugInfoDebugOperation = 30,
    NonSemanticShaderDebugInfoDebugExpression = 31,
    NonSemanticShaderDebugInfoDebugMacroDef = 32,
    NonSemanticShaderDebugInfoDebugMacroUndef = 33,
    NonSemanticShaderDebugInfoDebugImportedEntity = 34,
    NonSemanticShaderDebugInfoDebugSource = 35,
    NonSemanticShaderDebugInfoDebugFunctionDefinition = 101,
    NonSemanticShaderDebugInfoDebugSourceContinued = 102,
    NonSemanticShaderDebugInfoDebugLine = 103,
    NonSemanticShaderDebugInfoDebugNoLine = 104,
    NonSemanticShaderDebugInfoDebugBuildIdentifier = 105,
    NonSemanticShaderDebugInfoDebugStoragePath = 106,
    NonSemanticShaderDebugInfoDebugEntryPoint = 107,
    NonSemanticShaderDebugInfoDebugTypeMatrix = 108,
    NonSemanticShaderDebugInfoDebugTypeVectorIdEXT = 109,
    NonSemanticShaderDebugInfoDebugTypeCooperativeMatrixKHR = 110,
    NonSemanticShaderDebugInfoInstructionsMax = 0x7fffffff
};


enum NonSemanticShaderDebugInfoDebugInfoFlags {
    NonSemanticShaderDebugInfoNone = 0x0000,
    NonSemanticShaderDebugInfoFlagIsProtected = 0x01,
    NonSemanticShaderDebugInfoFlagIsPrivate = 0x02,
    NonSemanticShaderDebugInfoFlagIsPublic = 0x03,
    NonSemanticShaderDebugInfoFlagIsLocal = 0x04,
    NonSemanticShaderDebugInfoFlagIsDefinition = 0x08,
    NonSemanticShaderDebugInfoFlagFwdDecl = 0x10,
    NonSemanticShaderDebugInfoFlagArtificial = 0x20,
    NonSemanticShaderDebugInfoFlagExplicit = 0x40,
    NonSemanticShaderDebugInfoFlagPrototyped = 0x80,
    NonSemanticShaderDebugInfoFlagObjectPointer = 0x100,
    NonSemanticShaderDebugInfoFlagStaticMember = 0x200,
    NonSemanticShaderDebugInfoFlagIndirectVariable = 0x400,
    NonSemanticShaderDebugInfoFlagLValueReference = 0x800,
    NonSemanticShaderDebugInfoFlagRValueReference = 0x1000,
    NonSemanticShaderDebugInfoFlagIsOptimized = 0x2000,
    NonSemanticShaderDebugInfoFlagIsEnumClass = 0x4000,
    NonSemanticShaderDebugInfoFlagTypePassByValue = 0x8000,
    NonSemanticShaderDebugInfoFlagTypePassByReference = 0x10000,
    NonSemanticShaderDebugInfoFlagUnknownPhysicalLayout = 0x20000,
    NonSemanticShaderDebugInfoDebugInfoFlagsMax = 0x7fffffff
};

enum NonSemanticShaderDebugInfoBuildIdentifierFlags {
    NonSemanticShaderDebugInfoIdentifierPossibleDuplicates = 0x01,
    NonSemanticShaderDebugInfoBuildIdentifierFlagsMax = 0x7fffffff
};

enum NonSemanticShaderDebugInfoDebugBaseTypeAttributeEncoding {
    NonSemanticShaderDebugInfoUnspecified = 0,
    NonSemanticShaderDebugInfoAddress = 1,
    NonSemanticShaderDebugInfoBoolean = 2,
    NonSemanticShaderDebugInfoFloat = 3,
    NonSemanticShaderDebugInfoSigned = 4,
    NonSemanticShaderDebugInfoSignedChar = 5,
    NonSemanticShaderDebugInfoUnsigned = 6,
    NonSemanticShaderDebugInfoUnsignedChar = 7,
    NonSemanticShaderDebugInfoDebugBaseTypeAttributeEncodingMax = 0x7fffffff
};

enum NonSemanticShaderDebugInfoDebugCompositeType {
    NonSemanticShaderDebugInfoClass = 0,
    NonSemanticShaderDebugInfoStructure = 1,
    NonSemanticShaderDebugInfoUnion = 2,
    NonSemanticShaderDebugInfoDebugCompositeTypeMax = 0x7fffffff
};

enum NonSemanticShaderDebugInfoDebugTypeQualifier {
    NonSemanticShaderDebugInfoConstType = 0,
    NonSemanticShaderDebugInfoVolatileType = 1,
    NonSemanticShaderDebugInfoRestrictType = 2,
    NonSemanticShaderDebugInfoAtomicType = 3,
    NonSemanticShaderDebugInfoDebugTypeQualifierMax = 0x7fffffff
};

enum NonSemanticShaderDebugInfoDebugOperation {
    NonSemanticShaderDebugInfoDeref = 0,
    NonSemanticShaderDebugInfoPlus = 1,
    NonSemanticShaderDebugInfoMinus = 2,
    NonSemanticShaderDebugInfoPlusUconst = 3,
    NonSemanticShaderDebugInfoBitPiece = 4,
    NonSemanticShaderDebugInfoSwap = 5,
    NonSemanticShaderDebugInfoXderef = 6,
    NonSemanticShaderDebugInfoStackValue = 7,
    NonSemanticShaderDebugInfoConstu = 8,
    NonSemanticShaderDebugInfoFragment = 9,
    NonSemanticShaderDebugInfoDebugOperationMax = 0x7fffffff
};

enum NonSemanticShaderDebugInfoDebugImportedEntity {
    NonSemanticShaderDebugInfoImportedModule = 0,
    NonSemanticShaderDebugInfoImportedDeclaration = 1,
    NonSemanticShaderDebugInfoDebugImportedEntityMax = 0x7fffffff
};


#ifdef __cplusplus
}
#endif

#endif // SPIRV_UNIFIED1_NonSemanticShaderDebugInfo_H_
