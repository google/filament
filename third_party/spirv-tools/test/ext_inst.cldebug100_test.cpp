// Copyright (c) 2017-2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <vector>

#include "OpenCLDebugInfo100.h"
#include "gmock/gmock.h"
#include "source/util/string_utils.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"

// This file tests the correctness of encoding and decoding of instructions
// involving the OpenCL.DebugInfo.100 extended instruction set.
// Validation is not checked here.

namespace spvtools {
namespace {

using spvtest::Concatenate;
using spvtest::MakeInstruction;
using testing::Eq;
using utils::MakeVector;

// Test values of enums vs. what is written in the spec.

TEST(ExtInstCLDebugInfo, InstructionValues) {
  EXPECT_EQ(0, OpenCLDebugInfo100DebugInfoNone);
  EXPECT_EQ(1, OpenCLDebugInfo100DebugCompilationUnit);
  EXPECT_EQ(2, OpenCLDebugInfo100DebugTypeBasic);
  EXPECT_EQ(3, OpenCLDebugInfo100DebugTypePointer);
  EXPECT_EQ(4, OpenCLDebugInfo100DebugTypeQualifier);
  EXPECT_EQ(5, OpenCLDebugInfo100DebugTypeArray);
  EXPECT_EQ(6, OpenCLDebugInfo100DebugTypeVector);
  EXPECT_EQ(7, OpenCLDebugInfo100DebugTypedef);
  EXPECT_EQ(8, OpenCLDebugInfo100DebugTypeFunction);
  EXPECT_EQ(9, OpenCLDebugInfo100DebugTypeEnum);
  EXPECT_EQ(10, OpenCLDebugInfo100DebugTypeComposite);
  EXPECT_EQ(11, OpenCLDebugInfo100DebugTypeMember);
  EXPECT_EQ(12, OpenCLDebugInfo100DebugTypeInheritance);
  EXPECT_EQ(13, OpenCLDebugInfo100DebugTypePtrToMember);
  EXPECT_EQ(14, OpenCLDebugInfo100DebugTypeTemplate);
  EXPECT_EQ(15, OpenCLDebugInfo100DebugTypeTemplateParameter);
  EXPECT_EQ(16, OpenCLDebugInfo100DebugTypeTemplateTemplateParameter);
  EXPECT_EQ(17, OpenCLDebugInfo100DebugTypeTemplateParameterPack);
  EXPECT_EQ(18, OpenCLDebugInfo100DebugGlobalVariable);
  EXPECT_EQ(19, OpenCLDebugInfo100DebugFunctionDeclaration);
  EXPECT_EQ(20, OpenCLDebugInfo100DebugFunction);
  EXPECT_EQ(21, OpenCLDebugInfo100DebugLexicalBlock);
  EXPECT_EQ(22, OpenCLDebugInfo100DebugLexicalBlockDiscriminator);
  EXPECT_EQ(23, OpenCLDebugInfo100DebugScope);
  EXPECT_EQ(24, OpenCLDebugInfo100DebugNoScope);
  EXPECT_EQ(25, OpenCLDebugInfo100DebugInlinedAt);
  EXPECT_EQ(26, OpenCLDebugInfo100DebugLocalVariable);
  EXPECT_EQ(27, OpenCLDebugInfo100DebugInlinedVariable);
  EXPECT_EQ(28, OpenCLDebugInfo100DebugDeclare);
  EXPECT_EQ(29, OpenCLDebugInfo100DebugValue);
  EXPECT_EQ(30, OpenCLDebugInfo100DebugOperation);
  EXPECT_EQ(31, OpenCLDebugInfo100DebugExpression);
  EXPECT_EQ(32, OpenCLDebugInfo100DebugMacroDef);
  EXPECT_EQ(33, OpenCLDebugInfo100DebugMacroUndef);
  EXPECT_EQ(34, OpenCLDebugInfo100DebugImportedEntity);
  EXPECT_EQ(35, OpenCLDebugInfo100DebugSource);
}

TEST(ExtInstCLDebugInfo, InfoFlagValues) {
  EXPECT_EQ(1 << 0, OpenCLDebugInfo100FlagIsProtected);
  EXPECT_EQ(1 << 1, OpenCLDebugInfo100FlagIsPrivate);
  EXPECT_EQ(((1 << 0) | (1 << 1)), OpenCLDebugInfo100FlagIsPublic);
  EXPECT_EQ(1 << 2, OpenCLDebugInfo100FlagIsLocal);
  EXPECT_EQ(1 << 3, OpenCLDebugInfo100FlagIsDefinition);
  EXPECT_EQ(1 << 4, OpenCLDebugInfo100FlagFwdDecl);
  EXPECT_EQ(1 << 5, OpenCLDebugInfo100FlagArtificial);
  EXPECT_EQ(1 << 6, OpenCLDebugInfo100FlagExplicit);
  EXPECT_EQ(1 << 7, OpenCLDebugInfo100FlagPrototyped);
  EXPECT_EQ(1 << 8, OpenCLDebugInfo100FlagObjectPointer);
  EXPECT_EQ(1 << 9, OpenCLDebugInfo100FlagStaticMember);
  EXPECT_EQ(1 << 10, OpenCLDebugInfo100FlagIndirectVariable);
  EXPECT_EQ(1 << 11, OpenCLDebugInfo100FlagLValueReference);
  EXPECT_EQ(1 << 12, OpenCLDebugInfo100FlagRValueReference);
  EXPECT_EQ(1 << 13, OpenCLDebugInfo100FlagIsOptimized);
  EXPECT_EQ(1 << 14, OpenCLDebugInfo100FlagIsEnumClass);
  EXPECT_EQ(1 << 15, OpenCLDebugInfo100FlagTypePassByValue);
  EXPECT_EQ(1 << 16, OpenCLDebugInfo100FlagTypePassByReference);
}

TEST(ExtInstCLDebugInfo, BaseTypeAttributeEndodingValues) {
  EXPECT_EQ(0, OpenCLDebugInfo100Unspecified);
  EXPECT_EQ(1, OpenCLDebugInfo100Address);
  EXPECT_EQ(2, OpenCLDebugInfo100Boolean);
  EXPECT_EQ(3, OpenCLDebugInfo100Float);
  EXPECT_EQ(4, OpenCLDebugInfo100Signed);
  EXPECT_EQ(5, OpenCLDebugInfo100SignedChar);
  EXPECT_EQ(6, OpenCLDebugInfo100Unsigned);
  EXPECT_EQ(7, OpenCLDebugInfo100UnsignedChar);
}

TEST(ExtInstCLDebugInfo, CompositeTypeValues) {
  EXPECT_EQ(0, OpenCLDebugInfo100Class);
  EXPECT_EQ(1, OpenCLDebugInfo100Structure);
  EXPECT_EQ(2, OpenCLDebugInfo100Union);
}

TEST(ExtInstCLDebugInfo, TypeQualifierValues) {
  EXPECT_EQ(0, OpenCLDebugInfo100ConstType);
  EXPECT_EQ(1, OpenCLDebugInfo100VolatileType);
  EXPECT_EQ(2, OpenCLDebugInfo100RestrictType);
  EXPECT_EQ(3, OpenCLDebugInfo100AtomicType);
}

TEST(ExtInstCLDebugInfo, DebugOperationValues) {
  EXPECT_EQ(0, OpenCLDebugInfo100Deref);
  EXPECT_EQ(1, OpenCLDebugInfo100Plus);
  EXPECT_EQ(2, OpenCLDebugInfo100Minus);
  EXPECT_EQ(3, OpenCLDebugInfo100PlusUconst);
  EXPECT_EQ(4, OpenCLDebugInfo100BitPiece);
  EXPECT_EQ(5, OpenCLDebugInfo100Swap);
  EXPECT_EQ(6, OpenCLDebugInfo100Xderef);
  EXPECT_EQ(7, OpenCLDebugInfo100StackValue);
  EXPECT_EQ(8, OpenCLDebugInfo100Constu);
  EXPECT_EQ(9, OpenCLDebugInfo100Fragment);
}

TEST(ExtInstCLDebugInfo, ImportedEntityValues) {
  EXPECT_EQ(0, OpenCLDebugInfo100ImportedModule);
  EXPECT_EQ(1, OpenCLDebugInfo100ImportedDeclaration);
}

// Test round trip through assembler and disassembler.

struct InstructionCase {
  uint32_t opcode;
  std::string name;
  std::string operands;
  std::vector<uint32_t> expected_operands;
};

using ExtInstCLDebugInfo100RoundTripTest =
    spvtest::TextToBinaryTestBase<::testing::TestWithParam<InstructionCase>>;
using ExtInstCLDebugInfo100RoundTripTestExplicit = spvtest::TextToBinaryTest;

TEST_P(ExtInstCLDebugInfo100RoundTripTest, ParameterizedExtInst) {
  const std::string input =
      "%1 = OpExtInstImport \"OpenCL.DebugInfo.100\"\n"
      "%3 = OpExtInst %2 %1 " +
      GetParam().name + GetParam().operands + "\n";
  // First make sure it assembles correctly.
  std::cout << input << std::endl;
  EXPECT_THAT(
      CompiledInstructions(input),
      Eq(Concatenate(
          {MakeInstruction(spv::Op::OpExtInstImport, {1},
                           MakeVector("OpenCL.DebugInfo.100")),
           MakeInstruction(spv::Op::OpExtInst, {2, 3, 1, GetParam().opcode},
                           GetParam().expected_operands)})))
      << input;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(input), input) << input;
}

#define EPREFIX "Debug"

#define CASE_0(Enum)                                               \
  {                                                                \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, "", {} \
  }

#define CASE_ILL(Enum, L0, L1)                              \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 " #L0 " " #L1, {                               \
      4, L0, L1                                             \
    }                                                       \
  }

#define CASE_IL(Enum, L0)                                                 \
  {                                                                       \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " %4 " #L0, { \
      4, L0                                                               \
    }                                                                     \
  }

#define CASE_I(Enum)                                                     \
  {                                                                      \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " %4", { 4 } \
  }

#define CASE_II(Enum)                                                          \
  {                                                                            \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " %4 %5", { 4, 5 } \
  }

#define CASE_III(Enum)                                                     \
  {                                                                        \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " %4 %5 %6", { \
      4, 5, 6                                                              \
    }                                                                      \
  }

#define CASE_IIII(Enum)                                                       \
  {                                                                           \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " %4 %5 %6 %7", { \
      4, 5, 6, 7                                                              \
    }                                                                         \
  }

#define CASE_IIIII(Enum)                                                       \
  {                                                                            \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " %4 %5 %6 %7 %8", \
    {                                                                          \
      4, 5, 6, 7, 8                                                            \
    }                                                                          \
  }

#define CASE_IIIIII(Enum)                                   \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 %7 %8 %9", {                             \
      4, 5, 6, 7, 8, 9                                      \
    }                                                       \
  }

#define CASE_IIIIIII(Enum)                                  \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 %7 %8 %9 %10", {                         \
      4, 5, 6, 7, 8, 9, 10                                  \
    }                                                       \
  }

#define CASE_IIILLI(Enum, L0, L1)                           \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 " #L0 " " #L1 " %7", {                   \
      4, 5, 6, L0, L1, 7                                    \
    }                                                       \
  }

#define CASE_IIILLIF(Enum, L0, L1, Fstr, Fnum)              \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 " #L0 " " #L1 " %7 " Fstr, {             \
      4, 5, 6, L0, L1, 7, Fnum                              \
    }                                                       \
  }

#define CASE_IIILLIFL(Enum, L0, L1, Fstr, Fnum, L2)         \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 " #L0 " " #L1 " %7 " Fstr " " #L2, {     \
      4, 5, 6, L0, L1, 7, Fnum, L2                          \
    }                                                       \
  }

#define CASE_IIILLIL(Enum, L0, L1, L2)                      \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 " #L0 " " #L1 " %7 " #L2, {              \
      4, 5, 6, L0, L1, 7, L2                                \
    }                                                       \
  }

#define CASE_IE(Enum, E0)                                                 \
  {                                                                       \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " %4 " #E0, { \
      4, uint32_t(OpenCLDebugInfo100##E0)                                 \
    }                                                                     \
  }

#define CASE_IEIILLI(Enum, E0, L1, L2)                      \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 " #E0 " %5 %6 " #L1 " " #L2 " %7", {           \
      4, uint32_t(OpenCLDebugInfo100##E0), 5, 6, L1, L2, 7  \
    }                                                       \
  }

#define CASE_IIE(Enum, E0)                                                   \
  {                                                                          \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " %4 %5 " #E0, { \
      4, 5, uint32_t(OpenCLDebugInfo100##E0)                                 \
    }                                                                        \
  }

#define CASE_ISF(Enum, S0, Fstr, Fnum)                      \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 " #S0 " " Fstr, {                              \
      4, uint32_t(spv::StorageClass::S0), Fnum              \
    }                                                       \
  }

#define CASE_LII(Enum, L0)                                                    \
  {                                                                           \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " " #L0 " %4 %5", \
    {                                                                         \
      L0, 4, 5                                                                \
    }                                                                         \
  }

#define CASE_LLIe(Enum, L0, L1, RawEnumName, RawEnumValue)  \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " " #L0 " " #L1 " %4 " RawEnumName, {               \
      L0, L1, 4, (uint32_t)RawEnumValue                     \
    }                                                       \
  }

#define CASE_ILI(Enum, L0)                                                    \
  {                                                                           \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " %4 " #L0 " %5", \
    {                                                                         \
      4, L0, 5                                                                \
    }                                                                         \
  }

#define CASE_ILII(Enum, L0)                                 \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 " #L0 " %5 %6", {                              \
      4, L0, 5, 6                                           \
    }                                                       \
  }

#define CASE_ILLII(Enum, L0, L1)                            \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 " #L0 " " #L1 " %5 %6", {                      \
      4, L0, L1, 5, 6                                       \
    }                                                       \
  }

#define CASE_IIILLIIF(Enum, L0, L1, Fstr, Fnum)             \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 " #L0 " " #L1 " %7 %8 " Fstr, {          \
      4, 5, 6, L0, L1, 7, 8, Fnum                           \
    }                                                       \
  }

#define CASE_IIILLIIFII(Enum, L0, L1, Fstr, Fnum)            \
  {                                                          \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,  \
        " %4 %5 %6 " #L0 " " #L1 " %7 %8 " Fstr " %9 %10", { \
      4, 5, 6, L0, L1, 7, 8, Fnum, 9, 10                     \
    }                                                        \
  }

#define CASE_IIILLIIFIIII(Enum, L0, L1, Fstr, Fnum)                  \
  {                                                                  \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,          \
        " %4 %5 %6 " #L0 " " #L1 " %7 %8 " Fstr " %9 %10 %11 %12", { \
      4, 5, 6, L0, L1, 7, 8, Fnum, 9, 10, 11, 12                     \
    }                                                                \
  }

#define CASE_IIILLIIFIIIIII(Enum, L0, L1, Fstr, Fnum)                        \
  {                                                                          \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,                  \
        " %4 %5 %6 " #L0 " " #L1 " %7 %8 " Fstr " %9 %10 %11 %12 %13 %14", { \
      4, 5, 6, L0, L1, 7, 8, Fnum, 9, 10, 11, 12, 13, 14                     \
    }                                                                        \
  }

#define CASE_IEILLIIIF(Enum, E0, L0, L1, Fstr, Fnum)                \
  {                                                                 \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,         \
        " %4 " #E0 " %5 " #L0 " " #L1 " %6 %7 %8 " Fstr, {          \
      4, uint32_t(OpenCLDebugInfo100##E0), 5, L0, L1, 6, 7, 8, Fnum \
    }                                                               \
  }

#define CASE_IEILLIIIFI(Enum, E0, L0, L1, Fstr, Fnum)                  \
  {                                                                    \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,            \
        " %4 " #E0 " %5 " #L0 " " #L1 " %6 %7 %8 " Fstr " %9", {       \
      4, uint32_t(OpenCLDebugInfo100##E0), 5, L0, L1, 6, 7, 8, Fnum, 9 \
    }                                                                  \
  }

#define CASE_IEILLIIIFII(Enum, E0, L0, L1, Fstr, Fnum)                     \
  {                                                                        \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,                \
        " %4 " #E0 " %5 " #L0 " " #L1 " %6 %7 %8 " Fstr " %9 %10", {       \
      4, uint32_t(OpenCLDebugInfo100##E0), 5, L0, L1, 6, 7, 8, Fnum, 9, 10 \
    }                                                                      \
  }

#define CASE_IEILLIIIFIII(Enum, E0, L0, L1, Fstr, Fnum)                        \
  {                                                                            \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,                    \
        " %4 " #E0 " %5 " #L0 " " #L1 " %6 %7 %8 " Fstr " %9 %10 %11", {       \
      4, uint32_t(OpenCLDebugInfo100##E0), 5, L0, L1, 6, 7, 8, Fnum, 9, 10, 11 \
    }                                                                          \
  }

#define CASE_IEILLIIIFIIII(Enum, E0, L0, L1, Fstr, Fnum)                     \
  {                                                                          \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,                  \
        " %4 " #E0 " %5 " #L0 " " #L1 " %6 %7 %8 " Fstr " %9 %10 %11 %12", { \
      4, uint32_t(OpenCLDebugInfo100##E0), 5, L0, L1, 6, 7, 8, Fnum, 9, 10,  \
          11, 12                                                             \
    }                                                                        \
  }

#define CASE_IIILLIIIF(Enum, L0, L1, Fstr, Fnum)            \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 " #L0 " " #L1 " %7 %8 %9 " Fstr, {       \
      4, 5, 6, L0, L1, 7, 8, 9, Fnum                        \
    }                                                       \
  }

#define CASE_IIILLIIIFI(Enum, L0, L1, Fstr, Fnum)            \
  {                                                          \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,  \
        " %4 %5 %6 " #L0 " " #L1 " %7 %8 %9 " Fstr " %10", { \
      4, 5, 6, L0, L1, 7, 8, 9, Fnum, 10                     \
    }                                                        \
  }

#define CASE_IIIIF(Enum, Fstr, Fnum)                        \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 %7 " Fstr, {                             \
      4, 5, 6, 7, Fnum                                      \
    }                                                       \
  }

#define CASE_IIILL(Enum, L0, L1)                            \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 " #L0 " " #L1, {                         \
      4, 5, 6, L0, L1                                       \
    }                                                       \
  }

#define CASE_IIIILL(Enum, L0, L1)                           \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 %6 %7 " #L0 " " #L1, {                      \
      4, 5, 6, 7, L0, L1                                    \
    }                                                       \
  }

#define CASE_IILLI(Enum, L0, L1)                            \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 " #L0 " " #L1 " %6", {                      \
      4, 5, L0, L1, 6                                       \
    }                                                       \
  }

#define CASE_IILLII(Enum, L0, L1)                           \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 " #L0 " " #L1 " %6 %7", {                   \
      4, 5, L0, L1, 6, 7                                    \
    }                                                       \
  }

#define CASE_IILLIII(Enum, L0, L1)                          \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 " #L0 " " #L1 " %6 %7 %8", {                \
      4, 5, L0, L1, 6, 7, 8                                 \
    }                                                       \
  }

#define CASE_IILLIIII(Enum, L0, L1)                         \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " %4 %5 " #L0 " " #L1 " %6 %7 %8 %9", {             \
      4, 5, L0, L1, 6, 7, 8, 9                              \
    }                                                       \
  }

#define CASE_IIILLIIFLI(Enum, L0, L1, Fstr, Fnum, L2)            \
  {                                                              \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,      \
        " %4 %5 %6 " #L0 " " #L1 " %7 %8 " Fstr " " #L2 " %9", { \
      4, 5, 6, L0, L1, 7, 8, Fnum, L2, 9                         \
    }                                                            \
  }

#define CASE_IIILLIIFLII(Enum, L0, L1, Fstr, Fnum, L2)               \
  {                                                                  \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum,          \
        " %4 %5 %6 " #L0 " " #L1 " %7 %8 " Fstr " " #L2 " %9 %10", { \
      4, 5, 6, L0, L1, 7, 8, Fnum, L2, 9, 10                         \
    }                                                                \
  }

#define CASE_E(Enum, E0)                                               \
  {                                                                    \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " " #E0, { \
      uint32_t(OpenCLDebugInfo100##E0)                                 \
    }                                                                  \
  }

#define CASE_EI(Enum, E0)                                                    \
  {                                                                          \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " " #E0 " %4", { \
      uint32_t(OpenCLDebugInfo100##E0), 4                                    \
    }                                                                        \
  }

#define CASE_EII(Enum, E0)                                                    \
  {                                                                           \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " " #E0 " %4 %5", \
    {                                                                         \
      uint32_t(OpenCLDebugInfo100##E0), 4, 5                                  \
    }                                                                         \
  }

#define CASE_EIII(Enum, E0)                                 \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " " #E0 " %4 %5 %6", {                              \
      uint32_t(OpenCLDebugInfo100##E0), 4, 5, 6             \
    }                                                       \
  }

#define CASE_EIIII(Enum, E0)                                \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " " #E0 " %4 %5 %6 %7", {                           \
      uint32_t(OpenCLDebugInfo100##E0), 4, 5, 6, 7          \
    }                                                       \
  }

#define CASE_EIIIII(Enum, E0)                               \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " " #E0 " %4 %5 %6 %7 %8", {                        \
      uint32_t(OpenCLDebugInfo100##E0), 4, 5, 6, 7, 8       \
    }                                                       \
  }

#define CASE_EL(Enum, E0, L0)                                                  \
  {                                                                            \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, " " #E0 " " #L0, { \
      uint32_t(OpenCLDebugInfo100##E0), L0                                     \
    }                                                                          \
  }

#define CASE_ELL(Enum, E0, L0, L1)                          \
  {                                                         \
    uint32_t(OpenCLDebugInfo100Debug##Enum), EPREFIX #Enum, \
        " " #E0 " " #L0 " " #L1, {                          \
      uint32_t(OpenCLDebugInfo100##E0), L0, L1              \
    }                                                       \
  }

// OpenCL.DebugInfo.100 4.1 Missing Debugging Information
INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugInfoNone,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_0(InfoNone),  // enum value 0
                         })));

// OpenCL.DebugInfo.100 4.2 Compilation Unit
INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugCompilationUnit, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        CASE_LLIe(CompilationUnit, 100, 42, "HLSL", spv::SourceLanguage::HLSL),
    })));

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugSource, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        // TODO(dneto): Should this be a list of sourc texts,
        // to accommodate length limits?
        CASE_I(Source),
        CASE_II(Source),
    })));

// OpenCL.DebugInfo.100 4.3 Type instructions
INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypeBasic,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_IIE(TypeBasic, Unspecified),
                             CASE_IIE(TypeBasic, Address),
                             CASE_IIE(TypeBasic, Boolean),
                             CASE_IIE(TypeBasic, Float),
                             CASE_IIE(TypeBasic, Signed),
                             CASE_IIE(TypeBasic, SignedChar),
                             CASE_IIE(TypeBasic, Unsigned),
                             CASE_IIE(TypeBasic, UnsignedChar),
                         })));

// The FlagIsPublic is value is (1 << 0) | (1 << 2) which is the same
// as the bitwise-OR of FlagIsProtected and FlagIsPrivate.
// The disassembler will emit the compound expression instead.
// There is no simple fix for this.  This enum is not really a mask
// for the bottom two bits.
TEST_F(ExtInstCLDebugInfo100RoundTripTestExplicit, FlagIsPublic) {
  const std::string prefix =
      "%1 = OpExtInstImport \"DebugInfo\"\n"
      "%3 = OpExtInst %2 %1 DebugTypePointer %4 Private ";
  const std::string input = prefix + "FlagIsPublic\n";
  const std::string expected = prefix + "FlagIsProtected|FlagIsPrivate\n";
  // First make sure it assembles correctly.
  EXPECT_THAT(CompiledInstructions(input),
              Eq(Concatenate(
                  {MakeInstruction(spv::Op::OpExtInstImport, {1},
                                   MakeVector("DebugInfo")),
                   MakeInstruction(spv::Op::OpExtInst,
                                   {2, 3, 1, OpenCLDebugInfo100DebugTypePointer,
                                    4, uint32_t(spv::StorageClass::Private),
                                    OpenCLDebugInfo100FlagIsPublic})})))
      << input;
  // Now check the round trip through the disassembler.
  EXPECT_THAT(EncodeAndDecodeSuccessfully(input), Eq(expected)) << input;
}

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugTypePointer, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({

        //// Use each flag independently.
        CASE_ISF(TypePointer, Private, "FlagIsProtected",
                 uint32_t(OpenCLDebugInfo100FlagIsProtected)),
        CASE_ISF(TypePointer, Private, "FlagIsPrivate",
                 uint32_t(OpenCLDebugInfo100FlagIsPrivate)),

        // FlagIsPublic is tested above.

        CASE_ISF(TypePointer, Private, "FlagIsLocal",
                 uint32_t(OpenCLDebugInfo100FlagIsLocal)),
        CASE_ISF(TypePointer, Private, "FlagIsDefinition",
                 uint32_t(OpenCLDebugInfo100FlagIsDefinition)),
        CASE_ISF(TypePointer, Private, "FlagFwdDecl",
                 uint32_t(OpenCLDebugInfo100FlagFwdDecl)),
        CASE_ISF(TypePointer, Private, "FlagArtificial",
                 uint32_t(OpenCLDebugInfo100FlagArtificial)),
        CASE_ISF(TypePointer, Private, "FlagExplicit",
                 uint32_t(OpenCLDebugInfo100FlagExplicit)),
        CASE_ISF(TypePointer, Private, "FlagPrototyped",
                 uint32_t(OpenCLDebugInfo100FlagPrototyped)),
        CASE_ISF(TypePointer, Private, "FlagObjectPointer",
                 uint32_t(OpenCLDebugInfo100FlagObjectPointer)),
        CASE_ISF(TypePointer, Private, "FlagStaticMember",
                 uint32_t(OpenCLDebugInfo100FlagStaticMember)),
        CASE_ISF(TypePointer, Private, "FlagIndirectVariable",
                 uint32_t(OpenCLDebugInfo100FlagIndirectVariable)),
        CASE_ISF(TypePointer, Private, "FlagLValueReference",
                 uint32_t(OpenCLDebugInfo100FlagLValueReference)),
        CASE_ISF(TypePointer, Private, "FlagIsOptimized",
                 uint32_t(OpenCLDebugInfo100FlagIsOptimized)),
        CASE_ISF(TypePointer, Private, "FlagIsEnumClass",
                 uint32_t(OpenCLDebugInfo100FlagIsEnumClass)),
        CASE_ISF(TypePointer, Private, "FlagTypePassByValue",
                 uint32_t(OpenCLDebugInfo100FlagTypePassByValue)),
        CASE_ISF(TypePointer, Private, "FlagTypePassByReference",
                 uint32_t(OpenCLDebugInfo100FlagTypePassByReference)),

        //// Use flags in combination, and try different storage classes.
        CASE_ISF(TypePointer, Function, "FlagIsProtected|FlagIsPrivate",
                 uint32_t(OpenCLDebugInfo100FlagIsProtected) |
                     uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
        CASE_ISF(
            TypePointer, Workgroup,
            "FlagIsPrivate|FlagFwdDecl|FlagIndirectVariable|FlagIsOptimized",
            uint32_t(OpenCLDebugInfo100FlagIsPrivate) |
                uint32_t(OpenCLDebugInfo100FlagFwdDecl) |
                uint32_t(OpenCLDebugInfo100FlagIndirectVariable) |
                uint32_t(OpenCLDebugInfo100FlagIsOptimized)),

    })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypeQualifier,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_IE(TypeQualifier, ConstType),
                             CASE_IE(TypeQualifier, VolatileType),
                             CASE_IE(TypeQualifier, RestrictType),
                             CASE_IE(TypeQualifier, AtomicType),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypeArray,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_II(TypeArray),
                             CASE_III(TypeArray),
                             CASE_IIII(TypeArray),
                             CASE_IIIII(TypeArray),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypeVector,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_IL(TypeVector, 2),
                             CASE_IL(TypeVector, 3),
                             CASE_IL(TypeVector, 4),
                             CASE_IL(TypeVector, 16),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypedef,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_IIILLI(Typedef, 12, 13),
                             CASE_IIILLI(Typedef, 14, 99),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypeFunction,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_EI(TypeFunction, FlagIsProtected),
                             CASE_EII(TypeFunction, FlagIsDefinition),
                             CASE_EIII(TypeFunction, FlagArtificial),
                             CASE_EIIII(TypeFunction, FlagExplicit),
                             CASE_EIIIII(TypeFunction, FlagIsPrivate),
                         })));

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugTypeEnum, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        CASE_IIILLIIFII(
            TypeEnum, 12, 13,
            "FlagIsPrivate|FlagFwdDecl|FlagIndirectVariable|FlagIsOptimized",
            uint32_t(OpenCLDebugInfo100FlagIsPrivate) |
                uint32_t(OpenCLDebugInfo100FlagFwdDecl) |
                uint32_t(OpenCLDebugInfo100FlagIndirectVariable) |
                uint32_t(OpenCLDebugInfo100FlagIsOptimized)),
        CASE_IIILLIIFIIII(TypeEnum, 17, 18, "FlagStaticMember",
                          uint32_t(OpenCLDebugInfo100FlagStaticMember)),
        CASE_IIILLIIFIIIIII(TypeEnum, 99, 1, "FlagStaticMember",
                            uint32_t(OpenCLDebugInfo100FlagStaticMember)),
    })));

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugTypeComposite, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        CASE_IEILLIIIF(
            TypeComposite, Class, 12, 13,
            "FlagIsPrivate|FlagFwdDecl|FlagIndirectVariable|FlagIsOptimized",
            uint32_t(OpenCLDebugInfo100FlagIsPrivate) |
                uint32_t(OpenCLDebugInfo100FlagFwdDecl) |
                uint32_t(OpenCLDebugInfo100FlagIndirectVariable) |
                uint32_t(OpenCLDebugInfo100FlagIsOptimized)),
        // Cover all tag values: Class, Structure, Union
        CASE_IEILLIIIF(TypeComposite, Class, 12, 13, "FlagIsPrivate",
                       uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
        CASE_IEILLIIIF(TypeComposite, Structure, 12, 13, "FlagIsPrivate",
                       uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
        CASE_IEILLIIIF(TypeComposite, Union, 12, 13, "FlagIsPrivate",
                       uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
        // Now add members
        CASE_IEILLIIIFI(TypeComposite, Class, 9, 10, "FlagIsPrivate",
                        uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
        CASE_IEILLIIIFII(TypeComposite, Class, 9, 10, "FlagIsPrivate",
                         uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
        CASE_IEILLIIIFIII(TypeComposite, Class, 9, 10, "FlagIsPrivate",
                          uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
        CASE_IEILLIIIFIIII(TypeComposite, Class, 9, 10, "FlagIsPrivate",
                           uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
    })));

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugTypeMember, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        CASE_IIILLIIIF(TypeMember, 12, 13, "FlagIsPrivate",
                       uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
        CASE_IIILLIIIF(TypeMember, 99, 100, "FlagIsPrivate|FlagFwdDecl",
                       uint32_t(OpenCLDebugInfo100FlagIsPrivate) |
                           uint32_t(OpenCLDebugInfo100FlagFwdDecl)),
        // Add the optional Id argument.
        CASE_IIILLIIIFI(TypeMember, 12, 13, "FlagIsPrivate",
                        uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
    })));

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugTypeInheritance, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        CASE_IIIIF(TypeInheritance, "FlagIsPrivate",
                   uint32_t(OpenCLDebugInfo100FlagIsPrivate)),
        CASE_IIIIF(TypeInheritance, "FlagIsPrivate|FlagFwdDecl",
                   uint32_t(OpenCLDebugInfo100FlagIsPrivate) |
                       uint32_t(OpenCLDebugInfo100FlagFwdDecl)),
    })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypePtrToMember,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_II(TypePtrToMember),
                         })));

// OpenCL.DebugInfo.100 4.4 Templates

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypeTemplate,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_II(TypeTemplate),
                             CASE_III(TypeTemplate),
                             CASE_IIII(TypeTemplate),
                             CASE_IIIII(TypeTemplate),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypeTemplateParameter,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_IIIILL(TypeTemplateParameter, 1, 2),
                             CASE_IIIILL(TypeTemplateParameter, 99, 102),
                             CASE_IIIILL(TypeTemplateParameter, 10, 7),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypeTemplateTemplateParameter,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_IIILL(TypeTemplateTemplateParameter, 1, 2),
                             CASE_IIILL(TypeTemplateTemplateParameter, 99, 102),
                             CASE_IIILL(TypeTemplateTemplateParameter, 10, 7),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugTypeTemplateParameterPack,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_IILLI(TypeTemplateParameterPack, 1, 2),
                             CASE_IILLII(TypeTemplateParameterPack, 99, 102),
                             CASE_IILLIII(TypeTemplateParameterPack, 10, 7),
                             CASE_IILLIIII(TypeTemplateParameterPack, 10, 7),
                         })));

// OpenCL.DebugInfo.100 4.5 Global Variables

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugGlobalVariable, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        CASE_IIILLIIIF(GlobalVariable, 1, 2, "FlagIsOptimized",
                       uint32_t(OpenCLDebugInfo100FlagIsOptimized)),
        CASE_IIILLIIIF(GlobalVariable, 42, 43, "FlagIsOptimized",
                       uint32_t(OpenCLDebugInfo100FlagIsOptimized)),
        CASE_IIILLIIIFI(GlobalVariable, 1, 2, "FlagIsOptimized",
                        uint32_t(OpenCLDebugInfo100FlagIsOptimized)),
        CASE_IIILLIIIFI(GlobalVariable, 42, 43, "FlagIsOptimized",
                        uint32_t(OpenCLDebugInfo100FlagIsOptimized)),
    })));

// OpenCL.DebugInfo.100 4.6 Functions

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugFunctionDeclaration,
    ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        CASE_IIILLIIF(FunctionDeclaration, 1, 2, "FlagIsOptimized",
                      uint32_t(OpenCLDebugInfo100FlagIsOptimized)),
        CASE_IIILLIIF(FunctionDeclaration, 42, 43, "FlagFwdDecl",
                      uint32_t(OpenCLDebugInfo100FlagFwdDecl)),
    })));

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugFunction, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        CASE_IIILLIIFLI(Function, 1, 2, "FlagIsOptimized",
                        uint32_t(OpenCLDebugInfo100FlagIsOptimized), 3),
        CASE_IIILLIIFLI(Function, 42, 43, "FlagFwdDecl",
                        uint32_t(OpenCLDebugInfo100FlagFwdDecl), 44),
        // Add the optional declaration Id.
        CASE_IIILLIIFLII(Function, 1, 2, "FlagIsOptimized",
                         uint32_t(OpenCLDebugInfo100FlagIsOptimized), 3),
        CASE_IIILLIIFLII(Function, 42, 43, "FlagFwdDecl",
                         uint32_t(OpenCLDebugInfo100FlagFwdDecl), 44),
    })));

// OpenCL.DebugInfo.100 4.7 Local Information

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugLexicalBlock,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_ILLII(LexicalBlock, 1, 2),
                             CASE_ILLII(LexicalBlock, 42, 43),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugLexicalBlockDiscriminator,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_ILI(LexicalBlockDiscriminator, 1),
                             CASE_ILI(LexicalBlockDiscriminator, 42),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugScope,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_I(Scope),
                             CASE_II(Scope),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugNoScope,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_0(NoScope),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugInlinedAt,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_LII(InlinedAt, 1),
                             CASE_LII(InlinedAt, 42),
                         })));

// OpenCL.DebugInfo.100 4.8 Local Variables

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugLocalVariable, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        CASE_IIILLIF(LocalVariable, 1, 2, "FlagIsPrivate",
                     OpenCLDebugInfo100FlagIsPrivate),
        CASE_IIILLIF(LocalVariable, 4, 5, "FlagIsProtected",
                     OpenCLDebugInfo100FlagIsProtected),
        CASE_IIILLIFL(LocalVariable, 9, 99, "FlagIsProtected",
                      OpenCLDebugInfo100FlagIsProtected, 195),
        CASE_IIILLIFL(LocalVariable, 19, 199, "FlagIsPrivate",
                      OpenCLDebugInfo100FlagIsPrivate, 195),
    })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugInlinedVariable,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_II(InlinedVariable),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugDebugDeclare,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_III(Declare),
                         })));

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugDebugValue, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        CASE_IIII(Value),
        CASE_IIIII(Value),
        CASE_IIIIII(Value),
        // Test up to 3 id parameters. We can always try more.
        CASE_IIIIIII(Value),
    })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugDebugOperation,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_E(Operation, Deref),
                             CASE_E(Operation, Plus),
                             CASE_E(Operation, Minus),
                             CASE_EL(Operation, PlusUconst, 1),
                             CASE_EL(Operation, PlusUconst, 42),
                             CASE_ELL(Operation, BitPiece, 1, 2),
                             CASE_ELL(Operation, BitPiece, 4, 5),
                             CASE_E(Operation, Swap),
                             CASE_E(Operation, Xderef),
                             CASE_E(Operation, StackValue),
                             CASE_EL(Operation, Constu, 1),
                             CASE_EL(Operation, Constu, 42),
                             CASE_ELL(Operation, Fragment, 100, 200),
                             CASE_ELL(Operation, Fragment, 8, 9),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugDebugExpression,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_0(Expression),
                             CASE_I(Expression),
                             CASE_II(Expression),
                             CASE_III(Expression),
                             CASE_IIII(Expression),
                             CASE_IIIII(Expression),
                             CASE_IIIIII(Expression),
                             CASE_IIIIIII(Expression),
                         })));

// OpenCL.DebugInfo.100 4.9 Macros

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugMacroDef,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_ILI(MacroDef, 1),
                             CASE_ILI(MacroDef, 42),
                             CASE_ILII(MacroDef, 1),
                             CASE_ILII(MacroDef, 42),
                         })));

INSTANTIATE_TEST_SUITE_P(OpenCLDebugInfo100DebugMacroUndef,
                         ExtInstCLDebugInfo100RoundTripTest,
                         ::testing::ValuesIn(std::vector<InstructionCase>({
                             CASE_ILI(MacroUndef, 1),
                             CASE_ILI(MacroUndef, 42),
                         })));

// OpenCL.DebugInfo.100 4.10 Imported Entities

INSTANTIATE_TEST_SUITE_P(
    OpenCLDebugInfo100DebugImportedEntity, ExtInstCLDebugInfo100RoundTripTest,
    ::testing::ValuesIn(std::vector<InstructionCase>({
        // ID Name
        // Literal Tag
        // ID Source
        // ID Entity
        // Literal Number Line
        // Literal Number Column
        // ID Parent
        CASE_IEIILLI(ImportedEntity, ImportedModule, 67, 68),
        CASE_IEIILLI(ImportedEntity, ImportedDeclaration, 42, 43),
    })));

#undef EPREFIX
#undef CASE_0
#undef CASE_ILL
#undef CASE_IL
#undef CASE_I
#undef CASE_II
#undef CASE_III
#undef CASE_IIII
#undef CASE_IIIII
#undef CASE_IIIIII
#undef CASE_IIIIIII
#undef CASE_IIILLI
#undef CASE_IIILLIL
#undef CASE_IE
#undef CASE_IEIILLI
#undef CASE_IIE
#undef CASE_ISF
#undef CASE_LII
#undef CASE_LLIe
#undef CASE_ILI
#undef CASE_ILII
#undef CASE_ILLII
#undef CASE_IIILLIF
#undef CASE_IIILLIFL
#undef CASE_IIILLIIF
#undef CASE_IIILLIIFII
#undef CASE_IIILLIIFIIII
#undef CASE_IIILLIIFIIIIII
#undef CASE_IEILLIIIF
#undef CASE_IEILLIIIFI
#undef CASE_IEILLIIIFII
#undef CASE_IEILLIIIFIII
#undef CASE_IEILLIIIFIIII
#undef CASE_IIILLIIIF
#undef CASE_IIILLIIIFI
#undef CASE_IIIIF
#undef CASE_IIILL
#undef CASE_IIIILL
#undef CASE_IILLI
#undef CASE_IILLII
#undef CASE_IILLIII
#undef CASE_IILLIIII
#undef CASE_IIILLIIFLI
#undef CASE_IIILLIIFLII
#undef CASE_E
#undef CASE_EI
#undef CASE_EII
#undef CASE_EIII
#undef CASE_EIIII
#undef CASE_EIIIII
#undef CASE_EL
#undef CASE_ELL

}  // namespace
}  // namespace spvtools
