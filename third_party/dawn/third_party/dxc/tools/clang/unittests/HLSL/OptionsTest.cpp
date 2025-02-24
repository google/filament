///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// OptionsTest.cpp                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides tests for the command-line options APIs.                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef UNICODE
#define UNICODE
#endif

#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"
#include <algorithm>
#include <cassert>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "dxc/Test/HLSLTestData.h"
#include "dxc/Test/HlslTestUtils.h"

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/dxcapi.use.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/raw_os_ostream.h"

#include <fstream>

using namespace std;
using namespace hlsl_test;
using namespace hlsl;
using namespace hlsl::options;

/// Use this class to construct MainArgs from constants. Handy to use because
/// DxcOpts will StringRef into it.
class MainArgsArr : public MainArgs {
public:
  template <size_t n>
  MainArgsArr(const wchar_t *(&arr)[n]) : MainArgs(n, arr) {}
};

#ifdef _WIN32
class OptionsTest {
#else
class OptionsTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(OptionsTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_METHOD(ReadOptionsWhenDefinesThenInit)
  TEST_METHOD(ReadOptionsWhenExtensionsThenOK)
  TEST_METHOD(ReadOptionsWhenHelpThenShortcut)
  TEST_METHOD(ReadOptionsWhenInvalidThenFail)
  TEST_METHOD(ReadOptionsConflict)
  TEST_METHOD(ReadOptionsWhenValidThenOK)
  TEST_METHOD(ReadOptionsWhenJoinedThenOK)
  TEST_METHOD(ReadOptionsWhenNoEntryThenOK)
  TEST_METHOD(ReadOptionsForOutputObject)

  TEST_METHOD(ReadOptionsForDxcWhenApiArgMissingThenFail)
  TEST_METHOD(ReadOptionsForApiWhenApiArgMissingThenOK)

  TEST_METHOD(ConvertWhenFailThenThrow)

  TEST_METHOD(CopyOptionsWhenSingleThenOK)
  // TEST_METHOD(CopyOptionsWhenMultipleThenOK)

  TEST_METHOD(ReadOptionsJoinedWithSpacesThenOK)
  TEST_METHOD(ReadOptionsNoNonLegacyCBuffer)

  TEST_METHOD(TestPreprocessOption)

  TEST_METHOD(SerializeDxilFlags)

  std::unique_ptr<DxcOpts> ReadOptsTest(const MainArgs &mainArgs,
                                        unsigned flagsToInclude,
                                        bool shouldFail = false,
                                        bool shouldMessage = false) {
    std::string errorString;
    llvm::raw_string_ostream errorStream(errorString);
    std::unique_ptr<DxcOpts> opts = llvm::make_unique<DxcOpts>();
    int result = ReadDxcOpts(getHlslOptTable(), flagsToInclude, mainArgs,
                             *(opts.get()), errorStream);
    EXPECT_EQ(shouldFail, result != 0);
    EXPECT_EQ(shouldMessage, !errorStream.str().empty());
    return opts;
  }

  // Test variant that verifies expected pass condition
  // and checks error output to match given message.
  void ReadOptsTest(const MainArgs &mainArgs, unsigned flagsToInclude,
                    bool shouldFail, const char *expectErrorMsg) {
    std::string errorString;
    llvm::raw_string_ostream errorStream(errorString);
    std::unique_ptr<DxcOpts> opts = llvm::make_unique<DxcOpts>();
    int result = ReadDxcOpts(getHlslOptTable(), flagsToInclude, mainArgs,
                             *(opts.get()), errorStream);
    EXPECT_EQ(shouldFail, result != 0);
    VERIFY_ARE_EQUAL_STR(expectErrorMsg, errorStream.str().c_str());
  }

  void ReadOptsTest(const MainArgs &mainArgs, unsigned flagsToInclude,
                    const char *expectErrorMsg) {
    ReadOptsTest(mainArgs, flagsToInclude, true /*shouldFail*/, expectErrorMsg);
  }
};

TEST_F(OptionsTest, ReadOptionsWhenExtensionsThenOK) {
  const wchar_t *Args[] = {
      L"exe.exe",   L"/E",        L"main",    L"/T",           L"ps_6_0",
      L"hlsl.hlsl", L"-external", L"foo.dll", L"-external-fn", L"CreateObj"};
  const wchar_t *ArgsNoLib[] = {L"exe.exe",      L"/E",       L"main",
                                L"/T",           L"ps_6_0",   L"hlsl.hlsl",
                                L"-external-fn", L"CreateObj"};
  const wchar_t *ArgsNoFn[] = {L"exe.exe",   L"/E",     L"main",
                               L"/T",        L"ps_6_0", L"hlsl.hlsl",
                               L"-external", L"foo.dll"};
  MainArgsArr ArgsArr(Args);
  std::unique_ptr<DxcOpts> o = ReadOptsTest(ArgsArr, DxcFlags);
  VERIFY_ARE_EQUAL_STR("CreateObj", o->ExternalFn.data());
  VERIFY_ARE_EQUAL_STR("foo.dll", o->ExternalLib.data());

  MainArgsArr ArgsNoLibArr(ArgsNoLib);
  ReadOptsTest(ArgsNoLibArr, DxcFlags, true, true);
  MainArgsArr ArgsNoFnArr(ArgsNoFn);
  ReadOptsTest(ArgsNoFnArr, DxcFlags, true, true);
}

TEST_F(OptionsTest, ReadOptionsForOutputObject) {
  const wchar_t *Args[] = {L"exe.exe", L"/E",        L"main", L"/T",
                           L"ps_6_0",  L"hlsl.hlsl", L"-Fo",  L"hlsl.dxbc"};
  MainArgsArr ArgsArr(Args);
  std::unique_ptr<DxcOpts> o = ReadOptsTest(ArgsArr, DxcFlags);
  VERIFY_ARE_EQUAL_STR("hlsl.dxbc", o->OutputObject.data());
}

TEST_F(OptionsTest, ReadOptionsConflict) {
  const wchar_t *matrixArgs[] = {L"exe.exe", L"/E",   L"main", L"/T",
                                 L"ps_6_0",  L"-Zpr", L"-Zpc", L"hlsl.hlsl"};
  MainArgsArr ArgsArr(matrixArgs);
  ReadOptsTest(
      ArgsArr, DxcFlags,
      "Cannot specify /Zpr and /Zpc together, use /? to get usage information");

  const wchar_t *controlFlowArgs[] = {L"exe.exe", L"/E",       L"main",
                                      L"/T",      L"ps_6_0",   L"-Gfa",
                                      L"-Gfp",    L"hlsl.hlsl"};
  MainArgsArr controlFlowArr(controlFlowArgs);
  ReadOptsTest(
      controlFlowArr, DxcFlags,
      "Cannot specify /Gfa and /Gfp together, use /? to get usage information");

  const wchar_t *libArgs[] = {L"exe.exe", L"/E",      L"main",
                              L"/T",      L"lib_6_1", L"hlsl.hlsl"};
  MainArgsArr libArr(libArgs);
  ReadOptsTest(
      libArr, DxcFlags,
      "Must disable validation for unsupported lib_6_1 or lib_6_2 targets.");
}

TEST_F(OptionsTest, ReadOptionsWhenHelpThenShortcut) {
  const wchar_t *Args[] = {L"exe.exe", L"--help", L"--unknown-flag"};
  MainArgsArr ArgsArr(Args);
  std::unique_ptr<DxcOpts> o = ReadOptsTest(ArgsArr, DxcFlags);
  EXPECT_EQ(true, o->ShowHelp);
}

TEST_F(OptionsTest, ReadOptionsWhenValidThenOK) {
  const wchar_t *Args[] = {L"exe.exe", L"/E",     L"main",
                           L"/T",      L"ps_6_0", L"hlsl.hlsl"};
  MainArgsArr ArgsArr(Args);
  std::unique_ptr<DxcOpts> o = ReadOptsTest(ArgsArr, DxcFlags);
  VERIFY_ARE_EQUAL_STR("main", o->EntryPoint.data());
  VERIFY_ARE_EQUAL_STR("ps_6_0", o->TargetProfile.data());
  VERIFY_ARE_EQUAL_STR("hlsl.hlsl", o->InputFile.data());
}

TEST_F(OptionsTest, ReadOptionsWhenJoinedThenOK) {
  const wchar_t *Args[] = {L"exe.exe", L"/Emain", L"/Tps_6_0", L"hlsl.hlsl"};
  MainArgsArr ArgsArr(Args);
  std::unique_ptr<DxcOpts> o = ReadOptsTest(ArgsArr, DxcFlags);
  VERIFY_ARE_EQUAL_STR("main", o->EntryPoint.data());
  VERIFY_ARE_EQUAL_STR("ps_6_0", o->TargetProfile.data());
  VERIFY_ARE_EQUAL_STR("hlsl.hlsl", o->InputFile.data());
}

TEST_F(OptionsTest, ReadOptionsWhenNoEntryThenOK) {
  // It's not an error to omit the entry function name, but it's not
  // set to 'main' on behalf of callers either.
  const wchar_t *Args[] = {L"exe.exe", L"/T", L"ps_6_0", L"hlsl.hlsl"};
  MainArgsArr ArgsArr(Args);
  std::unique_ptr<DxcOpts> o = ReadOptsTest(ArgsArr, DxcFlags);
  VERIFY_IS_TRUE(o->EntryPoint.empty());
}

TEST_F(OptionsTest, ReadOptionsWhenInvalidThenFail) {
  const wchar_t *ArgsNoTarget[] = {L"exe.exe", L"/E", L"main", L"hlsl.hlsl"};
  const wchar_t *ArgsNoInput[] = {L"exe.exe", L"/E", L"main", L"/T", L"ps_6_0"};
  const wchar_t *ArgsNoArg[] = {L"exe.exe", L"hlsl.hlsl", L"/E", L"main",
                                L"/T"};
  const wchar_t *ArgsUnknown[] = {L"exe.exe",
                                  L"hlsl.hlsl",
                                  L"/E",
                                  L"main",
                                  (L"/T"
                                   L"ps_6_0"),
                                  L"--unknown"};
  const wchar_t *ArgsUnknownButIgnore[] = {
      L"exe.exe", L"hlsl.hlsl", L"/E",        L"main",
      L"/T",      L"ps_6_0",    L"--unknown", L"-Qunused-arguments"};
  MainArgsArr ArgsNoTargetArr(ArgsNoTarget), ArgsNoInputArr(ArgsNoInput),
      ArgsNoArgArr(ArgsNoArg), ArgsUnknownArr(ArgsUnknown),
      ArgsUnknownButIgnoreArr(ArgsUnknownButIgnore);
  ReadOptsTest(ArgsNoTargetArr, DxcFlags, true, true);
  ReadOptsTest(ArgsNoInputArr, DxcFlags, true, true);
  ReadOptsTest(ArgsNoArgArr, DxcFlags, true, true);
  ReadOptsTest(ArgsUnknownArr, DxcFlags, true, true);
  ReadOptsTest(ArgsUnknownButIgnoreArr, DxcFlags);
}

TEST_F(OptionsTest, ReadOptionsWhenDefinesThenInit) {
  const wchar_t *ArgsNoDefines[] = {L"exe.exe", L"/T",   L"ps_6_0",
                                    L"/E",      L"main", L"hlsl.hlsl"};
  const wchar_t *ArgsOneDefine[] = {
      L"exe.exe", L"/DNAME1=1", L"/T", L"ps_6_0", L"/E", L"main", L"hlsl.hlsl"};
  const wchar_t *ArgsTwoDefines[] = {
      L"exe.exe", L"/DNAME1=1", L"/T", L"ps_6_0", L"/D",       L"NAME2=2",
      L"/E",      L"main",      L"/T", L"ps_6_0", L"hlsl.hlsl"};
  const wchar_t *ArgsEmptyDefine[] = {
      L"exe.exe", L"/DNAME1", L"hlsl.hlsl", L"/E", L"main", L"/T", L"ps_6_0",
  };

  MainArgsArr ArgsNoDefinesArr(ArgsNoDefines), ArgsOneDefineArr(ArgsOneDefine),
      ArgsTwoDefinesArr(ArgsTwoDefines), ArgsEmptyDefineArr(ArgsEmptyDefine);

  std::unique_ptr<DxcOpts> o;
  o = ReadOptsTest(ArgsNoDefinesArr, DxcFlags);
  EXPECT_EQ(0U, o->Defines.size());

  o = ReadOptsTest(ArgsOneDefineArr, DxcFlags);
  EXPECT_EQ(1U, o->Defines.size());
  EXPECT_STREQW(L"NAME1", o->Defines.data()[0].Name);
  EXPECT_STREQW(L"1", o->Defines.data()[0].Value);

  o = ReadOptsTest(ArgsTwoDefinesArr, DxcFlags);
  EXPECT_EQ(2U, o->Defines.size());
  EXPECT_STREQW(L"NAME1", o->Defines.data()[0].Name);
  EXPECT_STREQW(L"1", o->Defines.data()[0].Value);
  EXPECT_STREQW(L"NAME2", o->Defines.data()[1].Name);
  EXPECT_STREQW(L"2", o->Defines.data()[1].Value);

  o = ReadOptsTest(ArgsEmptyDefineArr, DxcFlags);
  EXPECT_EQ(1U, o->Defines.size());
  EXPECT_STREQW(L"NAME1", o->Defines.data()[0].Name);
  EXPECT_EQ(nullptr, o->Defines.data()[0].Value);
}

TEST_F(OptionsTest, ReadOptionsForDxcWhenApiArgMissingThenFail) {
  // When an argument specified through an API argument is not specified (eg the
  // target model), for the command-line dxc.exe tool, then the validation
  // should fail.
  const wchar_t *Args[] = {L"exe.exe", L"/E", L"main", L"hlsl.hlsl"};

  MainArgsArr mainArgsArr(Args);

  std::unique_ptr<DxcOpts> o;
  o = ReadOptsTest(mainArgsArr, DxcFlags, true, true);
}

TEST_F(OptionsTest, ReadOptionsForApiWhenApiArgMissingThenOK) {
  // When an argument specified through an API argument is not specified (eg the
  // target model), for an API, then the validation should not fail.
  const wchar_t *Args[] = {L"exe.exe", L"/E", L"main", L"hlsl.hlsl"};

  MainArgsArr mainArgsArr(Args);

  std::unique_ptr<DxcOpts> o;
  o = ReadOptsTest(mainArgsArr, CompilerFlags, false, false);
}

TEST_F(OptionsTest, ConvertWhenFailThenThrow) {
  std::wstring wstr;

  // Simple test to verify conversion works.
  EXPECT_EQ(true, Unicode::UTF8ToWideString("test", &wstr));
  EXPECT_STREQW(L"test", wstr.data());

  // Simple test to verify conversion works with actual UTF-8 and not just
  // ASCII. n with tilde is Unicode 0x00F1, encoded in UTF-8 as 0xC3 0xB1
  EXPECT_EQ(true, Unicode::UTF8ToWideString("\xC3\xB1", &wstr));
  EXPECT_STREQW(L"\x00F1", wstr.data());

  // Fail when the sequence is incomplete.
  EXPECT_EQ(false, Unicode::UTF8ToWideString("\xC3", &wstr));

  // Throw on failure.
  bool thrown = false;
  try {
    Unicode::UTF8ToWideStringOrThrow("\xC3");
  } catch (...) {
    thrown = true;
  }
  EXPECT_EQ(true, thrown);
}

TEST_F(OptionsTest, CopyOptionsWhenSingleThenOK) {
  const char *ArgsNoDefines[] = {"/T",   "ps_6_0",    "/E",
                                 "main", "hlsl.hlsl", "-unknown"};
  const llvm::opt::OptTable *table = getHlslOptTable();
  unsigned missingIndex = 0, missingArgCount = 0;
  llvm::opt::InputArgList args =
      table->ParseArgs(ArgsNoDefines, missingIndex, missingArgCount, DxcFlags);
  std::vector<std::wstring> outArgs;
  CopyArgsToWStrings(args, DxcFlags, outArgs);
  EXPECT_EQ(4U, outArgs.size()); // -unknown and hlsl.hlsl are missing
  VERIFY_ARE_NOT_EQUAL(outArgs.end(), std::find(outArgs.begin(), outArgs.end(),
                                                std::wstring(L"/T")));
  VERIFY_ARE_NOT_EQUAL(outArgs.end(), std::find(outArgs.begin(), outArgs.end(),
                                                std::wstring(L"ps_6_0")));
  VERIFY_ARE_NOT_EQUAL(outArgs.end(), std::find(outArgs.begin(), outArgs.end(),
                                                std::wstring(L"/E")));
  VERIFY_ARE_NOT_EQUAL(outArgs.end(), std::find(outArgs.begin(), outArgs.end(),
                                                std::wstring(L"main")));
  VERIFY_ARE_EQUAL(outArgs.end(), std::find(outArgs.begin(), outArgs.end(),
                                            std::wstring(L"hlsl.hlsl")));
}

TEST_F(OptionsTest, ReadOptionsJoinedWithSpacesThenOK) {
  {
    // Ensure parsing arguments in joined form with embedded spaces
    // between the option and the argument works, for these argument types:
    // - JoinedOrSeparateClass (-E, -T)
    // - SeparateClass (-external, -external-fn)
    const wchar_t *Args[] = {L"exe.exe",           L"-E main",
                             L"/T  ps_6_0",        L"hlsl.hlsl",
                             L"-external foo.dll", L"-external-fn  CreateObj"};
    MainArgsArr ArgsArr(Args);
    std::unique_ptr<DxcOpts> o = ReadOptsTest(ArgsArr, DxcFlags);
    VERIFY_ARE_EQUAL_STR("main", o->EntryPoint.data());
    VERIFY_ARE_EQUAL_STR("ps_6_0", o->TargetProfile.data());
    VERIFY_ARE_EQUAL_STR("CreateObj", o->ExternalFn.data());
    VERIFY_ARE_EQUAL_STR("foo.dll", o->ExternalLib.data());
  }

  {
    // Ignore trailing spaces in option name for JoinedOrSeparateClass
    // Otherwise error messages are not easy for user to interpret
    const wchar_t *Args[] = {L"exe.exe", L"-E ",    L"main",
                             L"/T  ",    L"ps_6_0", L"hlsl.hlsl"};
    MainArgsArr ArgsArr(Args);
    std::unique_ptr<DxcOpts> o = ReadOptsTest(ArgsArr, DxcFlags);
    VERIFY_ARE_EQUAL_STR("main", o->EntryPoint.data());
    VERIFY_ARE_EQUAL_STR("ps_6_0", o->TargetProfile.data());
  }
  {
    // Ignore trailing spaces in option name for SeparateClass
    // Otherwise error messages are not easy for user to interpret
    const wchar_t *Args[] = {L"exe.exe",    L"-E",      L"main",
                             L"/T",         L"ps_6_0",  L"hlsl.hlsl",
                             L"-external ", L"foo.dll", L"-external-fn  ",
                             L"CreateObj"};
    MainArgsArr ArgsArr(Args);
    std::unique_ptr<DxcOpts> o = ReadOptsTest(ArgsArr, DxcFlags);
    VERIFY_ARE_EQUAL_STR("CreateObj", o->ExternalFn.data());
    VERIFY_ARE_EQUAL_STR("foo.dll", o->ExternalLib.data());
  }
}

TEST_F(OptionsTest, ReadOptionsNoNonLegacyCBuffer) {
  {
    const wchar_t *Args[] = {L"exe.exe", L"/T  ", L"ps_6_0", L"hlsl.hlsl",
                             L"-no-legacy-cbuf-layout"};
    MainArgsArr ArgsArr(Args);
    ReadOptsTest(ArgsArr, DxcFlags, false /*shouldFail*/,
                 "warning: -no-legacy-cbuf-layout is no longer supported and "
                 "will be ignored. Future releases will not recognize it.\n");
  }

  {
    const wchar_t *Args[] = {L"exe.exe", L"/T  ", L"ps_6_0", L"hlsl.hlsl",
                             L"-not_use_legacy_cbuf_load"};
    MainArgsArr ArgsArr(Args);
    ReadOptsTest(ArgsArr, DxcFlags, false /*shouldFail*/,
                 "warning: -no-legacy-cbuf-layout is no longer supported and "
                 "will be ignored. Future releases will not recognize it.\n");
  }
}

static void VerifyPreprocessOption(llvm::StringRef command,
                                   const char *ExpectOutput,
                                   const char *ErrMsg) {
  std::string errorString;
  const llvm::opt::OptTable *optionTable = getHlslOptTable();
  llvm::SmallVector<llvm::StringRef, 4> args;
  command.split(args, " ", /*MaxSplit*/ -1, /*KeepEmpty*/ false);
  MainArgs argStrings(args);
  DxcOpts dxcOpts;
  llvm::raw_string_ostream errorStream(errorString);

  int retVal =
      ReadDxcOpts(optionTable, DxcFlags, argStrings, dxcOpts, errorStream);
  EXPECT_EQ(retVal, 0);
  EXPECT_STREQ(dxcOpts.Preprocess.c_str(), ExpectOutput);
  errorStream.flush();
  EXPECT_STREQ(errorString.c_str(), ErrMsg);
}

TEST_F(OptionsTest, TestPreprocessOption) {
  VerifyPreprocessOption("/T ps_6_0 -P input.hlsl", "input.i", "");
  VerifyPreprocessOption("/T ps_6_0 -Fi out.pp -P input.hlsl", "out.pp", "");
  VerifyPreprocessOption("/T ps_6_0 -P -Fi out.pp input.hlsl", "out.pp", "");
  const char *Warning =
      "warning: -P out.pp is deprecated, please use -P -Fi out.pp instead.\n";
  VerifyPreprocessOption("/T ps_6_0 -P out.pp input.hlsl", "out.pp", Warning);
  VerifyPreprocessOption("/T ps_6_0 input.hlsl -P out.pp ", "out.pp", Warning);
}

static void VerifySerializeDxilFlags(llvm::StringRef command,
                                     uint32_t ExpectFlags) {
  std::string errorString;
  const llvm::opt::OptTable *optionTable = getHlslOptTable();
  llvm::SmallVector<llvm::StringRef, 4> args;
  command.split(args, " ", /*MaxSplit*/ -1, /*KeepEmpty*/ false);
  args.emplace_back("-Tlib_6_3");
  args.emplace_back("input.hlsl");
  MainArgs argStrings(args);
  DxcOpts dxcOpts;
  llvm::raw_string_ostream errorStream(errorString);

  int retVal =
      ReadDxcOpts(optionTable, DxcFlags, argStrings, dxcOpts, errorStream);
  EXPECT_EQ(retVal, 0);
  errorStream.flush();
  EXPECT_EQ(errorString.empty(), true);
  EXPECT_EQ(
      static_cast<uint32_t>(hlsl::options::ComputeSerializeDxilFlags(dxcOpts)),
      ExpectFlags);
}

static uint32_t CombineFlags(llvm::ArrayRef<SerializeDxilFlags> flags) {
  uint32_t result = 0;
  for (SerializeDxilFlags f : flags)
    result |= static_cast<uint32_t>(f);
  return result;
}

struct SerializeDxilFlagsTest {
  const char *command;
  uint32_t flags;
};

using F = SerializeDxilFlags;

TEST_F(OptionsTest, SerializeDxilFlags) {
  // Test cases for SerializeDxilFlags
  // These cases are generated by group flags and do a full combination.
  // [("-Qstrip_rootsignature", {"F::StripRootSignature"}),\
      // ("", set())]
  // [("-Qkeep_reflect_in_dxil", set()),\
      // ("", {"F::StripReflectionFromDxilPart"})]
  // [("-Qstrip_reflect", {}), \
    // ("", {"F::IncludeReflectionPart"})]
  // [("-Zss -Zs", {"F::IncludeDebugNamePart","F::DebugNameDependOnSource"}),\
      // ("-Zsb", {"F::IncludeDebugNamePart"}), \
      // ("-FdDbgName.pdb", {"F::IncludeDebugNamePart"}), \
      // ("-Zi", {"F::IncludeDebugNamePart"}), \
      // ("-Zsb -Qembed_debug -Zi",
  // {"F::IncludeDebugInfoPart","F::IncludeDebugNamePart"}), \
      // ("-FdDbgName.pdb -Qembed_debug -Zi",
  // {"F::IncludeDebugInfoPart","F::IncludeDebugNamePart"}), \
      // ("", set())]

  SerializeDxilFlagsTest Tests[] = {
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil -Qstrip_reflect -Zss -Zs",
       CombineFlags({F::IncludeDebugNamePart, F::DebugNameDependOnSource,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil -Qstrip_reflect -Zsb",
       CombineFlags({F::IncludeDebugNamePart, F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil -Qstrip_reflect "
       "-FdDbgName.pdb",
       CombineFlags({F::IncludeDebugNamePart, F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil -Qstrip_reflect -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil -Qstrip_reflect -Zsb "
       "-Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeDebugInfoPart,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil -Qstrip_reflect "
       "-FdDbgName.pdb -Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeDebugInfoPart,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil -Qstrip_reflect ",
       CombineFlags({F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil  -Zss -Zs",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::DebugNameDependOnSource, F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil  -Zsb",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil  -FdDbgName.pdb",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil  -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil  -Zsb -Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::IncludeDebugInfoPart, F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil  -FdDbgName.pdb "
       "-Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::IncludeDebugInfoPart, F::StripRootSignature})},
      {"-Qstrip_rootsignature -Qkeep_reflect_in_dxil  ",
       CombineFlags({F::IncludeReflectionPart, F::StripRootSignature})},
      {"-Qstrip_rootsignature  -Qstrip_reflect -Zss -Zs",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::DebugNameDependOnSource, F::StripRootSignature})},
      {"-Qstrip_rootsignature  -Qstrip_reflect -Zsb",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature  -Qstrip_reflect -FdDbgName.pdb",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature  -Qstrip_reflect -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature  -Qstrip_reflect -Zsb -Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::IncludeDebugInfoPart, F::StripRootSignature})},
      {"-Qstrip_rootsignature  -Qstrip_reflect -FdDbgName.pdb -Qembed_debug "
       "-Zi",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::IncludeDebugInfoPart, F::StripRootSignature})},
      {"-Qstrip_rootsignature  -Qstrip_reflect ",
       CombineFlags({F::StripReflectionFromDxilPart, F::StripRootSignature})},
      {"-Qstrip_rootsignature   -Zss -Zs",
       CombineFlags({F::IncludeReflectionPart, F::IncludeDebugNamePart,
                     F::StripReflectionFromDxilPart, F::DebugNameDependOnSource,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature   -Zsb",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::StripReflectionFromDxilPart, F::StripRootSignature})},
      {"-Qstrip_rootsignature   -FdDbgName.pdb",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::StripReflectionFromDxilPart, F::StripRootSignature})},
      {"-Qstrip_rootsignature   -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::StripReflectionFromDxilPart, F::StripRootSignature})},
      {"-Qstrip_rootsignature   -Zsb -Qembed_debug -Zi",
       CombineFlags({F::IncludeReflectionPart, F::IncludeDebugNamePart,
                     F::StripReflectionFromDxilPart, F::IncludeDebugInfoPart,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature   -FdDbgName.pdb -Qembed_debug -Zi",
       CombineFlags({F::IncludeReflectionPart, F::IncludeDebugNamePart,
                     F::StripReflectionFromDxilPart, F::IncludeDebugInfoPart,
                     F::StripRootSignature})},
      {"-Qstrip_rootsignature   ",
       CombineFlags({F::IncludeReflectionPart, F::StripReflectionFromDxilPart,
                     F::StripRootSignature})},
      {"-Qkeep_reflect_in_dxil -Qstrip_reflect -Zss -Zs",
       CombineFlags({F::IncludeDebugNamePart, F::DebugNameDependOnSource})},
      {"-Qkeep_reflect_in_dxil -Qstrip_reflect -Zsb",
       CombineFlags({F::IncludeDebugNamePart})},
      {"-Qkeep_reflect_in_dxil -Qstrip_reflect -FdDbgName.pdb",
       CombineFlags({F::IncludeDebugNamePart})},
      {"-Qkeep_reflect_in_dxil -Qstrip_reflect -Zi",
       CombineFlags({F::IncludeDebugNamePart})},
      {"-Qkeep_reflect_in_dxil -Qstrip_reflect -Zsb -Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeDebugInfoPart})},
      {"-Qkeep_reflect_in_dxil -Qstrip_reflect -FdDbgName.pdb -Qembed_debug "
       "-Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeDebugInfoPart})},
      {"-Qkeep_reflect_in_dxil -Qstrip_reflect ",
       CombineFlags({SerializeDxilFlags::None})},
      {"-Qkeep_reflect_in_dxil  -Zss -Zs",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::DebugNameDependOnSource})},
      {"-Qkeep_reflect_in_dxil  -Zsb",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart})},
      {"-Qkeep_reflect_in_dxil  -FdDbgName.pdb",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart})},
      {"-Qkeep_reflect_in_dxil  -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart})},
      {"-Qkeep_reflect_in_dxil  -Zsb -Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::IncludeDebugInfoPart})},
      {"-Qkeep_reflect_in_dxil  -FdDbgName.pdb -Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::IncludeReflectionPart,
                     F::IncludeDebugInfoPart})},
      {"-Qkeep_reflect_in_dxil  ", CombineFlags({F::IncludeReflectionPart})},
      {"-Qstrip_reflect -Zss -Zs",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::DebugNameDependOnSource})},
      {"-Qstrip_reflect -Zsb",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart})},
      {"-Qstrip_reflect -FdDbgName.pdb",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart})},
      {"-Qstrip_reflect -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart})},
      {"-Qstrip_reflect -Zsb -Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::IncludeDebugInfoPart})},
      {"-Qstrip_reflect -FdDbgName.pdb -Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::IncludeDebugInfoPart})},
      {"-Qstrip_reflect ", CombineFlags({F::StripReflectionFromDxilPart})},
      {"-Zss -Zs",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::DebugNameDependOnSource, F::IncludeReflectionPart})},
      {"-Zsb",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::IncludeReflectionPart})},
      {"-FdDbgName.pdb",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::IncludeReflectionPart})},
      {"-Zi",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::IncludeReflectionPart})},
      {"-Zsb -Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::IncludeDebugInfoPart, F::IncludeReflectionPart})},
      {"-FdDbgName.pdb -Qembed_debug -Zi",
       CombineFlags({F::IncludeDebugNamePart, F::StripReflectionFromDxilPart,
                     F::IncludeDebugInfoPart, F::IncludeReflectionPart})},
      {"", CombineFlags(
               {F::StripReflectionFromDxilPart, F::IncludeReflectionPart})}};

  for (const auto &T : Tests) {
    VerifySerializeDxilFlags(T.command, T.flags);
  }
}
