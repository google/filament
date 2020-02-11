// Copyright (c) 2019 The Khronos Group Inc.
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

#include "gmock/gmock.h"
#include "test/link/linker_fixture.h"

namespace spvtools {
namespace {

using TypeMatch = spvtest::LinkerTest;

// Basic types
#define PartInt(D, N) D(N) " = OpTypeInt 32 0"
#define PartFloat(D, N) D(N) " = OpTypeFloat 32"
#define PartOpaque(D, N) D(N) " = OpTypeOpaque \"bar\""
#define PartSampler(D, N) D(N) " = OpTypeSampler"
#define PartEvent(D, N) D(N) " = OpTypeEvent"
#define PartDeviceEvent(D, N) D(N) " = OpTypeDeviceEvent"
#define PartReserveId(D, N) D(N) " = OpTypeReserveId"
#define PartQueue(D, N) D(N) " = OpTypeQueue"
#define PartPipe(D, N) D(N) " = OpTypePipe ReadWrite"
#define PartPipeStorage(D, N) D(N) " = OpTypePipeStorage"
#define PartNamedBarrier(D, N) D(N) " = OpTypeNamedBarrier"

// Compound types
#define PartVector(DR, DA, N, T) DR(N) " = OpTypeVector " DA(T) " 3"
#define PartMatrix(DR, DA, N, T) DR(N) " = OpTypeMatrix " DA(T) " 4"
#define PartImage(DR, DA, N, T) \
  DR(N) " = OpTypeImage " DA(T) " 2D 0 0 0 0 Rgba32f"
#define PartSampledImage(DR, DA, N, T) DR(N) " = OpTypeSampledImage " DA(T)
#define PartArray(DR, DA, N, T) DR(N) " = OpTypeArray " DA(T) " " DA(const)
#define PartRuntimeArray(DR, DA, N, T) DR(N) " = OpTypeRuntimeArray " DA(T)
#define PartStruct(DR, DA, N, T) DR(N) " = OpTypeStruct " DA(T) " " DA(T)
#define PartPointer(DR, DA, N, T) DR(N) " = OpTypePointer Workgroup " DA(T)
#define PartFunction(DR, DA, N, T) DR(N) " = OpTypeFunction " DA(T) " " DA(T)

#define CheckDecoRes(S) "[[" #S ":%\\w+]]"
#define CheckDecoArg(S) "[[" #S "]]"
#define InstDeco(S) "%" #S

#define MatchPart1(F, N) \
  "; CHECK: " Part##F(CheckDecoRes, N) "\n" Part##F(InstDeco, N) "\n"
#define MatchPart2(F, N, T)                                           \
  "; CHECK: " Part##F(CheckDecoRes, CheckDecoArg, N, T) "\n" Part##F( \
      InstDeco, InstDeco, N, T) "\n"

#define MatchF(N, CODE)                                         \
  TEST_F(TypeMatch, N) {                                        \
    const std::string base =                                    \
        "OpCapability Linkage\n"                                \
        "OpCapability NamedBarrier\n"                           \
        "OpCapability PipeStorage\n"                            \
        "OpCapability Pipes\n"                                  \
        "OpCapability DeviceEnqueue\n"                          \
        "OpCapability Kernel\n"                                 \
        "OpCapability Shader\n"                                 \
        "OpCapability Addresses\n"                              \
        "OpDecorate %var LinkageAttributes \"foo\" "            \
        "{Import,Export}\n"                                     \
        "; CHECK: [[baseint:%\\w+]] = OpTypeInt 32 1\n"         \
        "%baseint = OpTypeInt 32 1\n"                           \
        "; CHECK: [[const:%\\w+]] = OpConstant [[baseint]] 3\n" \
        "%const = OpConstant %baseint 3\n" CODE                 \
        "; CHECK: OpVariable [[type]] Uniform\n"                \
        "%var = OpVariable %type Uniform";                      \
    ExpandAndMatch(base);                                       \
  }

#define Match1(T) MatchF(Type##T, MatchPart1(T, type))
#define Match2(T, A) \
  MatchF(T##OfType##A, MatchPart1(A, a) MatchPart2(T, type, a))
#define Match3(T, A, B)   \
  MatchF(T##Of##A##Of##B, \
         MatchPart1(B, b) MatchPart2(A, a, b) MatchPart2(T, type, a))

// clang-format off
// Basic types
Match1(Int)
Match1(Float)
Match1(Opaque)
Match1(Sampler)
Match1(Event)
Match1(DeviceEvent)
Match1(ReserveId)
Match1(Queue)
Match1(Pipe)
Match1(PipeStorage)
Match1(NamedBarrier)

// Simpler (restricted) compound types
Match2(Vector, Float)
Match3(Matrix, Vector, Float)
Match2(Image, Float)

// Unrestricted compound types
#define MatchCompounds1(A) \
  Match2(RuntimeArray, A)  \
  Match2(Struct, A)        \
  Match2(Pointer, A)       \
  Match2(Function, A)      \
  Match2(Array, A)
#define MatchCompounds2(A, B) \
  Match3(RuntimeArray, A, B)  \
  Match3(Struct, A, B)        \
  Match3(Pointer, A, B)       \
  Match3(Function, A, B)      \
  Match3(Array, A, B)

MatchCompounds1(Float)
MatchCompounds2(Array, Float)
MatchCompounds2(RuntimeArray, Float)
MatchCompounds2(Struct, Float)
MatchCompounds2(Pointer, Float)
MatchCompounds2(Function, Float)
// clang-format on

// ForwardPointer tests, which don't fit into the previous mold
#define MatchFpF(N, CODE)                                             \
  MatchF(N,                                                           \
         "; CHECK: OpTypeForwardPointer [[type:%\\w+]] Workgroup\n"   \
         "OpTypeForwardPointer %type Workgroup\n" CODE                \
         "; CHECK: [[type]] = OpTypePointer Workgroup [[realtype]]\n" \
         "%type = OpTypePointer Workgroup %realtype\n")
#define MatchFp1(T) MatchFpF(ForwardPointerOf##T, MatchPart1(T, realtype))
#define MatchFp2(T, A) \
  MatchFpF(ForwardPointerOf##T, MatchPart1(A, a) MatchPart2(T, realtype, a))

    // clang-format off
MatchFp1(Float)
MatchFp2(Array, Float)
MatchFp2(RuntimeArray, Float)
MatchFp2(Struct, Float)
MatchFp2(Function, Float)
// clang-format on

}  // namespace
}  // namespace spvtools
