// Copyright 2026 LunarG Inc.
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

#include "gmock/gmock.h"
#include "spirv-tools/libspirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;

using ValidatePipe = spvtest::ValidateBase<bool>;

const spv_target_env pipe_version = SPV_ENV_UNIVERSAL_1_1;

std::string GenerateShaderCode(const std::string& body) {
  std::ostringstream ss;
  ss << R"(
OpCapability Kernel
OpCapability Addresses
OpCapability Linkage
OpCapability Pipes
OpCapability GenericPointer
OpCapability PipeStorage
OpCapability Int64
OpMemoryModel Physical64 OpenCL
OpEntryPoint Kernel %main "main"

%bool = OpTypeBool
%uint = OpTypeInt 32 0
%uint64 = OpTypeInt 64 0

%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_4 = OpConstant %uint 4
%uint64_4 = OpConstant %uint64 4
%uint_null = OpConstantNull %uint

%_ptr_Generic_uint = OpTypePointer Generic %uint
%_ptr_Function_uint = OpTypePointer Function %uint

%pipe_storage =  OpTypePipeStorage
%reserved_id = OpTypeReserveId

%void = OpTypeVoid
%read_pipe_type = OpTypePipe ReadOnly
%write_pipe_type = OpTypePipe WriteOnly
%read_write_pipe_type = OpTypePipe ReadWrite
%fn = OpTypeFunction %void %read_pipe_type %write_pipe_type %read_write_pipe_type

%main = OpFunction %void None %fn
%read_pipe = OpFunctionParameter %read_pipe_type
%write_pipe = OpFunctionParameter %write_pipe_type
%read_write_pipe = OpFunctionParameter %read_write_pipe_type
%label = OpLabel

%func_var = OpVariable %_ptr_Function_uint Function
OpStore %func_var %uint_null Aligned 4
%generic_ptr = OpPtrCastToGeneric %_ptr_Generic_uint %func_var

%const_pipe_storage = OpConstantPipeStorage %pipe_storage 4 4 32
)";

  ss << body;

  ss << R"(
OpReturn
OpFunctionEnd)";
  return ss.str();
}

TEST_F(ValidatePipe, PipeReadWriteGood) {
  const std::string ss = R"(
    %x = OpReadPipe %uint %read_pipe %generic_ptr %uint_4 %uint_4
    %y = OpWritePipe %uint %write_pipe %generic_ptr %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(pipe_version));
}

TEST_F(ValidatePipe, ReadPipeResultType) {
  const std::string ss = R"(
    %x = OpReadPipe %uint64 %read_pipe %generic_ptr %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result Type must be a 32-bit int scalar"));
}

TEST_F(ValidatePipe, ReadPipePipeType) {
  const std::string ss = R"(
    %x = OpReadPipe %uint %func_var %generic_ptr %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pipe must be a type of OpTypePipe"));
}

TEST_F(ValidatePipe, ReadPipeAccessQualifier) {
  const std::string ss = R"(
    %x = OpReadPipe %uint %write_pipe %generic_ptr %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Pipe must have a OpTypePipe with ReadOnly access qualifier"));
}

TEST_F(ValidatePipe, WritePipeAccessQualifier) {
  const std::string ss = R"(
    %x = OpWritePipe %uint %read_pipe %generic_ptr %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Pipe must have a OpTypePipe with WriteOnly access qualifier"));
}

TEST_F(ValidatePipe, ReadPipePacketSizeInt64) {
  const std::string ss = R"(
    %x = OpReadPipe %uint %read_pipe %generic_ptr %uint64_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Packet Size must be a 32-bit scalar integer"));
}

TEST_F(ValidatePipe, ReadPipePacketAlignmentInt64) {
  const std::string ss = R"(
    %x = OpReadPipe %uint %read_pipe %generic_ptr %uint_4 %uint64_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Packet Alignment must be a 32-bit scalar integer"));
}

TEST_F(ValidatePipe, ReadPipePointerNotPoint) {
  const std::string ss = R"(
    %x = OpReadPipe %uint %read_pipe %read_pipe %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pointer must be a type of OpTypePointer"));
}

TEST_F(ValidatePipe, ReadPipePointerNotGeneric) {
  const std::string ss = R"(
    %x = OpReadPipe %uint %read_pipe %func_var %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Pointer must be a OpTypePointer with a Generic storage class"));
}

TEST_F(ValidatePipe, PipeQueryGood) {
  const std::string ss = R"(
    %x = OpGetNumPipePackets %uint %read_pipe %uint_4 %uint_4
    %y = OpGetMaxPipePackets %uint %write_pipe %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(pipe_version));
}

TEST_F(ValidatePipe, GetNumPipePacketsResult) {
  const std::string ss = R"(
    %x = OpGetNumPipePackets %uint64 %read_pipe %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result Type must be a 32-bit int scalar"));
}

TEST_F(ValidatePipe, GetNumPipePacketsReadWrite) {
  const std::string ss = R"(
    %x = OpGetNumPipePackets %uint %read_write_pipe %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Pipe must have a OpTypePipe with ReadOnly or "
                        "WriteOnly access qualifier"));
}

TEST_F(ValidatePipe, GetNumPipePacketsPacketSize) {
  const std::string ss = R"(
    %x = OpGetNumPipePackets %uint %read_pipe %uint64_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Packet Size must be a 32-bit scalar integer"));
}

TEST_F(ValidatePipe, GetNumPipePacketsPacketAlignment) {
  const std::string ss = R"(
    %x = OpGetNumPipePackets %uint %read_pipe %uint_4 %uint64_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Packet Alignment must be a 32-bit scalar integer"));
}

TEST_F(ValidatePipe, PipeReservedReadWriteGood) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %reserved_id %read_pipe %uint_1 %uint_4 %uint_4
    %w = OpReserveWritePipePackets %reserved_id %write_pipe %uint_1 %uint_4 %uint_4

    %x = OpReservedReadPipe %uint %read_pipe %r %uint_null %generic_ptr %uint_4 %uint_4
    %y = OpReservedWritePipe %uint %write_pipe %w %uint_null %generic_ptr %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(pipe_version));
}

TEST_F(ValidatePipe, ReserveReadPipePacketsResult) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %uint %read_pipe %uint_1 %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result Type must be OpTypeReserveId"));
}

TEST_F(ValidatePipe, ReserveReadPipePacketsWritePipe) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %reserved_id %write_pipe %uint_1 %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Pipe must have a OpTypePipe with ReadOnly access qualifier"));
}

TEST_F(ValidatePipe, ReserveReadPipePacketsNumPacks) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %reserved_id %read_pipe %uint64_4 %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Num Packets must be a 32-bit scalar integer"));
}

TEST_F(ValidatePipe, ReservedReadPipeResult) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %reserved_id %read_pipe %uint_1 %uint_4 %uint_4
    %x = OpReservedReadPipe %reserved_id %read_pipe %r %uint_null %generic_ptr %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result Type must be a 32-bit int scalar"));
}

TEST_F(ValidatePipe, ReservedReadPipeReserveType) {
  const std::string ss = R"(
    %x = OpReservedReadPipe %uint %read_pipe %func_var %uint_null %generic_ptr %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Reserve Id type must be OpTypeReserveId"));
}

TEST_F(ValidatePipe, ReservedReadPipeIndexType) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %reserved_id %read_pipe %uint_1 %uint_4 %uint_4
    %x = OpReservedReadPipe %uint %read_pipe %r %uint64_4 %generic_ptr %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Index must be a 32-bit scalar integer"));
}

TEST_F(ValidatePipe, ReservedReadPipePacketAlignment) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %reserved_id %read_pipe %uint_1 %uint_4 %uint_4
    %x = OpReservedReadPipe %uint %read_pipe %r %uint_null %generic_ptr %uint_4 %uint64_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Packet Alignment must be a 32-bit scalar integer"));
}

TEST_F(ValidatePipe, CommitPipeGood) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %reserved_id %read_pipe %uint_1 %uint_4 %uint_4
    %w = OpReserveWritePipePackets %reserved_id %write_pipe %uint_1 %uint_4 %uint_4

    OpCommitReadPipe %read_pipe %r %uint_4 %uint_4
    OpCommitWritePipe %write_pipe %w %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(pipe_version));
}

TEST_F(ValidatePipe, CommitReadReservedId) {
  const std::string ss = R"(
        OpCommitReadPipe %read_pipe %func_var %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Reserve Id type must be OpTypeReserveId"));
}

TEST_F(ValidatePipe, CommitReadPacketSize) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %reserved_id %read_pipe %uint_1 %uint_4 %uint_4
         OpCommitReadPipe %read_pipe %r %uint64_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Packet Size must be a 32-bit scalar integer"));
}

TEST_F(ValidatePipe, IsValidReserveIdGood) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %reserved_id %read_pipe %uint_1 %uint_4 %uint_4
    %x = OpIsValidReserveId %bool %r
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(pipe_version));
}

TEST_F(ValidatePipe, IsValidReserveIdReservedId) {
  const std::string ss = R"(
    %x = OpIsValidReserveId %bool %func_var
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Reserve Id type must be OpTypeReserveId"));
}

TEST_F(ValidatePipe, IsValidReserveIdResult) {
  const std::string ss = R"(
    %r = OpReserveReadPipePackets %reserved_id %read_pipe %uint_1 %uint_4 %uint_4
    %x = OpIsValidReserveId %uint %r
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result Type must be a bool scalar"));
}

TEST_F(ValidatePipe, CreatePipeFromPipeStorageGood) {
  const std::string ss = R"(
    %x = OpCreatePipeFromPipeStorage %read_pipe_type %const_pipe_storage
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(pipe_version));
}

TEST_F(ValidatePipe, CreatePipeFromPipeStorageType) {
  const std::string ss = R"(
    %x = OpCreatePipeFromPipeStorage %uint %const_pipe_storage
  )";
  CompileSuccessfully(GenerateShaderCode(ss), SPV_ENV_UNIVERSAL_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result Type must be OpTypePipe"));
}

TEST_F(ValidatePipe, ConstantPipeStorageType) {
  const std::string ss = R"(
    %bad = OpConstantPipeStorage %uint 4 4 32
  )";
  CompileSuccessfully(GenerateShaderCode(ss), SPV_ENV_UNIVERSAL_1_1);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result Type must be OpTypePipeStorage"));
}

TEST_F(ValidatePipe, ConstantPipeStorageBlock) {
  const std::string ss = R"(
    OpCapability Kernel
    OpCapability Addresses
    OpCapability Linkage
    OpCapability Pipes
    OpCapability PipeStorage
    OpMemoryModel Physical64 OpenCL
    OpEntryPoint Kernel %main "main"
    %pipe_storage =  OpTypePipeStorage
    %const_pipe_storage = OpConstantPipeStorage %pipe_storage 4 4 32
  )";
  CompileSuccessfully(ss, pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_LAYOUT, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("ConstantPipeStorage must appear in a block"));
}

TEST_F(ValidatePipe, GroupGood) {
  const std::string ss = R"(
    %r = OpGroupReserveReadPipePackets %reserved_id %uint_2 %read_pipe %uint_1 %uint_4 %uint_4
    %w = OpGroupReserveWritePipePackets %reserved_id %uint_2 %write_pipe %uint_1 %uint_4 %uint_4

    OpGroupCommitReadPipe %uint_2 %read_pipe %r %uint_4 %uint_4
    OpGroupCommitWritePipe %uint_2 %write_pipe %w %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(pipe_version));
}

TEST_F(ValidatePipe, GroupReserveReadPipePacketsResult) {
  const std::string ss = R"(
    %r = OpGroupReserveReadPipePackets %uint %uint_2 %read_pipe %uint_1 %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Result Type must be OpTypeReserveId"));
}

TEST_F(ValidatePipe, GroupReserveReadPipePacketsWrite) {
  const std::string ss = R"(
    %r = OpGroupReserveReadPipePackets %reserved_id %uint_2 %read_write_pipe %uint_1 %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Pipe must have a OpTypePipe with ReadOnly access qualifier"));
}

TEST_F(ValidatePipe, GroupReserveReadPipePacketsPacketSize) {
  const std::string ss = R"(
    %r = OpGroupReserveReadPipePackets %reserved_id %uint_2 %read_pipe %uint_1 %uint64_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Packet Size must be a 32-bit scalar integer"));
}

TEST_F(ValidatePipe, GroupCommitReadPipeWrite) {
  const std::string ss = R"(
    %r = OpGroupReserveReadPipePackets %reserved_id %uint_2 %read_pipe %uint_1 %uint_4 %uint_4
    OpGroupCommitReadPipe %uint_2 %write_pipe %r %uint_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Pipe must have a OpTypePipe with ReadOnly access qualifier"));
}

TEST_F(ValidatePipe, GroupCommitReadPipePacketSize) {
  const std::string ss = R"(
    %r = OpGroupReserveReadPipePackets %reserved_id %uint_2 %read_pipe %uint_1 %uint_4 %uint_4
    OpGroupCommitReadPipe %uint_2 %read_pipe %r %uint64_4 %uint_4
  )";
  CompileSuccessfully(GenerateShaderCode(ss), pipe_version);
  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(pipe_version));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Packet Size must be a 32-bit scalar integer"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
