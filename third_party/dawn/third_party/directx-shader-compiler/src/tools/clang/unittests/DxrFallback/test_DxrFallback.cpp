#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/dxcapi.impl.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/dxcapi.h"
#include "dxc/dxcdxrfallbackcompiler.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"

#include "ShaderTester.h"
#include "defaultTestFilePath.h"
#undef IGNORE
#undef OPAQUE
#include "testFiles/testTraversal.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace dxc;
using namespace llvm;
using namespace hlsl;

const int DEBUG_OUTPUT_LEVEL = 1;

std::string ws2s(const std::wstring &wide) {
  return std::string(wide.begin(), wide.end());
}

std::wstring s2ws(const std::string &str) {
  return std::wstring(str.begin(), str.end());
}

void printErrors(CComPtr<IDxcOperationResult> pResult) {
  CComPtr<IDxcBlobEncoding> pErrorBuffer;
  IFT(pResult->GetErrorBuffer(&pErrorBuffer));
  const char *pStart = (const char *)pErrorBuffer->GetBufferPointer();
  std::string msg(pStart);
  std::cerr << msg;

  HRESULT status;
  pResult->GetStatus(&status);
  // IFTMSG(status, msg);
}

void CompileToDxilFromFile(DllLoader &dxcSupport, LPCWSTR pShaderTextFilePath,
                           LPCWSTR pEntryPoint, LPCWSTR pTargetProfile,
                           LPCWSTR *pArgs, UINT32 argCount,
                           const DxcDefine *pDefines, UINT32 defineCount,
                           IDxcBlob **ppBlob) {
  CComPtr<IDxcLibrary> pLibrary;
  IFT(dxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));

  CComPtr<IDxcIncludeHandler> dxcIncludeHandler;
  IFT(pLibrary->CreateIncludeHandler(&dxcIncludeHandler));

  UINT32 codePage(0);
  CComPtr<IDxcBlobEncoding> pTextBlob(nullptr);
  IFT(pLibrary->CreateBlobFromFile(pShaderTextFilePath, &codePage, &pTextBlob));

  CComPtr<IDxcCompiler> pCompiler;
  IFT(dxcSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));

  CComPtr<IDxcOperationResult> pResult;
  IFT(pCompiler->Compile(pTextBlob, pShaderTextFilePath, pEntryPoint,
                         pTargetProfile, pArgs, argCount, pDefines, defineCount,
                         dxcIncludeHandler, &pResult));

  HRESULT resultCode;
  CComPtr<IDxcBlobEncoding> pErrorBuffer;
  IFT(pResult->GetStatus(&resultCode));
  IFT(pResult->GetErrorBuffer(&pErrorBuffer));
  if (SUCCEEDED(resultCode)) {
    IFT(pResult->GetResult((IDxcBlob **)ppBlob));
  } else {
    printErrors(pResult);
  }
}

bool DxrCompile(DllLoader &dxrFallbackSupport, const std::string &entryName,
                std::vector<IDxcBlob *> &libs,
                const std::vector<std::string> &shaderNames,
                std::vector<DxcShaderInfo> &shaderIds, bool findCalledShaders,
                IDxcBlob **ppResultBlob) {
  CComPtr<IDxcDxrFallbackCompiler> pCompiler;
  IFT(dxrFallbackSupport.CreateInstance(CLSID_DxcDxrFallbackCompiler,
                                        &pCompiler));

  std::vector<std::wstring> shaderNamesW(shaderNames.size());
  std::vector<LPCWSTR> shaderNamePtrs(shaderNames.size());
  for (size_t i = 0; i < shaderNames.size(); ++i) {
    shaderNamesW[i] = s2ws(shaderNames[i]);
    shaderNamePtrs[i] = shaderNamesW[i].c_str();
  }

  const UINT maxAttributeSize = 32;
  shaderIds.resize(shaderNames.size());
  CComPtr<IDxcOperationResult> pCompileResult;
  CComPtr<IDxcBlob> pCompiledCollection;
  std::vector<DxcShaderBytecode> bytecode(libs.size());
  for (UINT i = 0; i < libs.size(); i++) {
    bytecode[i] = {(LPBYTE)libs[i]->GetBufferPointer(),
                   (UINT32)libs[i]->GetBufferSize()};
  }

  IFT(pCompiler->SetFindCalledShaders(findCalledShaders));
  IFT(pCompiler->SetDebugOutput(DEBUG_OUTPUT_LEVEL));
  IFT(pCompiler->Compile(bytecode.data(), libs.size(), shaderNamePtrs.data(),
                         shaderIds.data(), shaderNamePtrs.size(),
                         maxAttributeSize, &pCompileResult));
  pCompileResult->GetResult(&pCompiledCollection);

  IDxcBlob *compiledCollections[] = {pCompiledCollection};
  CComPtr<IDxcOperationResult> pResult;
  IFT(pCompiler->Link(s2ws(entryName).c_str(), compiledCollections,
                      ARRAYSIZE(compiledCollections), shaderNamePtrs.data(),
                      shaderIds.data(), shaderNamePtrs.size(), maxAttributeSize,
                      1024, &pResult));

  HRESULT status;
  IFT(pResult->GetStatus(&status));
  IFT(pResult->GetResult(ppResultBlob));
  if (SUCCEEDED(status)) {
    return true;
  } else {
    std::cerr << "Compile errors\n";
    printErrors(pResult);
    return false;
  }
}

class Tester {
public:
  Tester(const std::string &deviceName, const std::string &path)
      : m_deviceName(s2ws(deviceName)), m_path(path) {
    dxc::EnsureEnabled(m_dxcSupport);
    m_dxrFallbackSupport.InitializeForDll("DxrFallbackCompiler.dll",
                                          "DxcCreateDxrFallbackCompiler");
  }

  void setFiles(const std::vector<std::string> &files) {
    std::vector<std::string> filesWithLib(files);
    filesWithLib.push_back(m_testLibFilename);
    m_inputBlobs.clear();
    m_inputBlobPtrs.clear();
    for (auto &filename : filesWithLib) {
      CComPtr<IDxcBlob> pInput;
      LPCWSTR args[] = {L"-O3"};
      CompileToDxilFromFile(m_dxcSupport, s2ws(m_path + filename).c_str(), L"",
                            L"lib_6_3", args, _countof(args), nullptr, 0,
                            &pInput);
      m_inputBlobs.push_back(pInput);
      m_inputBlobPtrs.push_back(pInput);
    }
  }

protected:
  DXCLibraryDllLoader m_dxcSupport;
  SpecificDllLoader m_dxrFallbackSupport;
  std::wstring m_deviceName;
  std::vector<CComPtr<IDxcBlob>> m_inputBlobs;
  std::vector<IDxcBlob *> m_inputBlobPtrs;
  std::string m_path;
  std::string m_testLibFilename = "testLib.hlsl";
  std::string m_entryName = "CSMain";

  int runTest(CComPtr<IDxcBlob> pShader, int initialShaderId,
              const std::vector<int> &input,
              const std::vector<int> &expectedOutput) {
    std::vector<int> output;
    std::unique_ptr<ShaderTester> tester(ShaderTester::New(pShader));
    tester->setDevice(m_deviceName);
    tester->runShader(initialShaderId, input, output);
    int numFailed = checkResult(output, expectedOutput) ? 0 : 1;
    if (numFailed) {
      std::cout << "input:";
      for (size_t i = 0; i < input.size(); ++i)
        std::cout << " " << input[i];
      std::cout << "\n";
    }
    std::cout << "\n";

    return numFailed;
  }

  bool checkResult(const std::vector<int> &output,
                   const std::vector<int> &expectedOutput) {
    const unsigned count = output.empty() ? 0 : output[0];
    std::cout << count << ": ";

    // print result
    for (unsigned i = 0; i < count; ++i)
      std::cout << output[i + 1] << " ";
    std::cout << "\n";

    bool passed = false;
    if (count == expectedOutput.size()) {
      passed = true;
      for (size_t i = 0; i < expectedOutput.size(); ++i) {
        if (output[i + 1] != expectedOutput[i]) {
          passed = false;
          break;
        }
      }
    }

    if (!passed) {
      std::cout << expectedOutput.size() << ": ";
      for (size_t i = 0; i < expectedOutput.size(); ++i)
        std::cout << expectedOutput[i] << " ";
      std::cout << "\n";
    }

    std::cout << (passed ? "PASSED" : "FAILED") << "\n";

    return passed;
  }
};

class RtCompilerTester : public Tester {
public:
  struct TestWithEntryPoint {
    std::string entryPoint;
    std::vector<int> expectedOutput;
  };

  RtCompilerTester(const std::string &deviceName, const std::string &path)
      : Tester(deviceName, path) {}

  // Returns the number of failures
  int runTestsWithEntryPoints(const std::vector<TestWithEntryPoint> &tests) {
    int numFailed = 0;
    for (auto &test : tests) {
      std::cout << test.entryPoint << "\n";

      std::vector<std::string> shaderNames = {test.entryPoint};
      std::vector<int> input;
      std::vector<DxcShaderInfo> shaderIds;
      CComPtr<IDxcBlob> pComputeShader;
      if (DxrCompile(m_dxrFallbackSupport, m_entryName, m_inputBlobPtrs,
                     shaderNames, shaderIds, true, &pComputeShader))
        numFailed += runTest(pComputeShader, shaderIds[0].Identifier, input,
                             test.expectedOutput);
    }

    return numFailed;
  }

  // The first shader is the entry shader. The shaderId of the shader at
  // indirectShaderIdx is placed in const memory.
  //
  // Returns the number of failures.
  int runSingleTest(const std::vector<std::string> &shaderNames,
                    const std::vector<int> &input,
                    const std::vector<int> &expectedOutput) {
    std::vector<DxcShaderInfo> shaderIds(shaderNames.size());
    CComPtr<IDxcBlob> pComputeShader;
    if (!DxrCompile(m_dxrFallbackSupport, m_entryName, m_inputBlobPtrs,
                    shaderNames, shaderIds, false, &pComputeShader))
      return 1;

    for (size_t i = 0; i < shaderNames.size(); ++i)
      std::cout << shaderNames[i] << ":" << shaderIds[i].Identifier << " ";
    std::cout << "\n";

    return runTest(pComputeShader, shaderIds[0].Identifier, input,
                   expectedOutput);
  }

  void compileTest(const std::vector<std::string> &shaderNames,
                   const std::string &entryName) {
    std::vector<DxcShaderInfo> shaderIds(shaderNames.size());
    CComPtr<IDxcBlob> pOutput;
    if (DxrCompile(m_dxrFallbackSupport, entryName, m_inputBlobPtrs,
                   shaderNames, shaderIds, false, &pOutput))
      std::cout << "Compile succeeded\n";
    else
      std::cout << "Compile failed\n";

    for (size_t i = 0; i < shaderNames.size(); ++i)
      std::cout << shaderNames[i] << ":" << shaderIds[i].Identifier << " ";
    std::cout << "\n";
  }
};

int asint(float v) { return *(int *)&v; }

float asfloat(int v) { return *(float *)&v; }

class Leaf {
public:
  int leafType;
};

class Instance : public Leaf {
public:
  int instIdx;
  int instId;
  int instFlags;
};

class Primitive : public Leaf {
public:
  int primIdx;
  int geomIdx;
  int geomOpaque;
};

class Triangle : public Primitive {
public:
  float t, u, v, d;
  int anyHitRet;
};

class Custom : public Primitive {
public:
  struct Hit {
    float t;
    int hitKind;
    int attr0, attr1;
    int anyHitRet;
  };

  std::vector<Hit> hits;
};

Instance *inst(int instFlags = 0, int instIdx = 0, int instId = 0) {
  Instance *inst = new Instance; // TODO: make this not leak
  inst->leafType = LEAF_INST;
  inst->instFlags = instFlags;
  inst->instIdx = instIdx;
  inst->instId = instId;
  return inst;
}

Triangle *tri(float t, float u, float v, int anyHitRet = OPAQUE, float d = 1,
              int primIdx = 0, int geomIdx = 0) {
  Triangle *tri = new Triangle; // TODO: make this not leak
  tri->leafType = LEAF_TRIS;
  tri->t = t;
  tri->u = u;
  tri->v = v;
  tri->d = d;
  tri->anyHitRet = anyHitRet;
  tri->primIdx = primIdx;
  tri->geomIdx = geomIdx;
  tri->geomOpaque = (anyHitRet == OPAQUE);
  return tri;
}

Custom *custom(const std::vector<Custom::Hit> &hits, int geomOpaque = 0,
               int primIdx = 0, int geomIdx = 0) {
  Custom *c = new Custom;
  c->hits = hits;
  c->leafType = LEAF_CUSTOM;
  c->primIdx = primIdx;
  c->geomIdx = geomIdx;
  c->geomOpaque = geomOpaque;
  return c;
}

struct Payload {
  int val;
  int primIdx;

  bool operator!=(const Payload &other) {
    return this->val != other.val || this->primIdx != other.primIdx;
  }
};

std::ostream &operator<<(std::ostream &out, Payload payload) {
  out << "{" << payload.val << "," << payload.primIdx << "}";
  return out;
}

class TestData {
public:
  std::string name;
  std::vector<int> input;
  std::vector<int> expected;
  std::vector<std::string> shaders;
  std::map<std::string, std::vector<int>> shaderIdSlots;
  static int count;

  TestData(const std::string &name) : name(name) { count++; }

  void setShaderIds(std::vector<DxcShaderInfo> &shaderIds) {
    for (size_t i = 0; i < shaders.size(); ++i) {
      for (auto &slot : shaderIdSlots[shaders[i]])
        input[slot] = shaderIds[i].Identifier;
    }
  }

  struct CommittedPrim {
    const Primitive *prim;
    float t;
    const Custom::Hit *hit;
  };

  void simulate(Payload expectedPayload, const std::vector<Leaf *> &leaves,
                int rayFlags = 0) {
    shaders = {"raygen",   "chTri",    "ahTri", "intersection",
               "ahCustom", "chCustom", "miss",  "Fallback_TraceRay"};

    expect(RAYGEN);
    Payload payload = {1000, -1};

    traceRay(rayFlags);
    expect(TRACERAY);

    bool terminate = false;
    CommittedPrim committed = {nullptr, -1, nullptr};
    int instIdx = -1, instId = 0, instFlags = 0;
    for (Leaf *leaf : leaves) {
      if (leaf->leafType == LEAF_INST) {
        const Instance *i =
            (Instance *)(leaf); // TODO: Why does dynamic_cast<Instance*> not
                                // work here?
        instIdx = i->instIdx;
        instId = i->instId;
        instFlags = i->instFlags;
        leafInst(instIdx, instId, instFlags);
      } else {
        const Primitive *prim = (Primitive *)leaf;
        leafPrim(prim);
        bool opaque = isOpaque(prim->geomOpaque, instFlags, rayFlags);
        if (cull(opaque, rayFlags))
          continue;

        if (leaf->leafType == LEAF_TRIS) {
          const Triangle *tri = (Triangle *)leaf;
          triangle(tri);
          float d = (instFlags & INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE)
                        ? -tri->d
                        : tri->d;
          if ((committed.prim && (tri->t >= committed.t)) ||
              -d * computeCullFaceDir(instFlags, rayFlags) < 0)
            continue;

          if (opaque) {
            committed = {tri, tri->t, nullptr};
          } else {
            shader("ahTri");
            expect({ANYHIT, (int)tri->u, (int)tri->v});
            payload.val += 100;
            anyHitRet(tri->anyHitRet);
            if (tri->anyHitRet == TERMINATE) {
              committed = {tri, tri->t, nullptr};
              terminate = true;
              break;
            } else if (tri->anyHitRet == IGNORE) {
              // do nothing
            } else // ACCEPT)
            {
              committed = {tri, tri->t, nullptr};
            }
          }
        } else if (leaf->leafType == LEAF_CUSTOM) {
          const Custom *c = (Custom *)leaf;
          shader("ahCustom");
          shader("intersection");
          expect(INTERSECT + 1);
          for (auto &hit : c->hits) {
            customHit(hit);
            if (committed.prim && hit.t >= committed.t)
              continue;
            if (!opaque) {
              expect({ANYHIT + 1, hit.attr0, hit.attr1});
              payload.val += 100;
              anyHitRet(hit.anyHitRet);
              if (hit.anyHitRet == TERMINATE) {
                committed = {c, hit.t, &hit};
                terminate = true;
                break;
              } else if (hit.anyHitRet == IGNORE) {
                // do nothing
                continue;
              }
              // ACCEPT - fall through
            }
            committed = {c, hit.t, &hit};
            if (rayFlags & RAY_FLAG_TERMINATE_ON_FIRST_HIT) {
              terminate = true;
              break;
            }
          }
          if (!terminate)
            endHits();
        }
      }

      if ((rayFlags & RAY_FLAG_TERMINATE_ON_FIRST_HIT) && committed.prim) {
        terminate = true;
        break;
      }
    }
    if (!terminate) {
      if (instIdx != -1)
        endAccel();
      endAccel();
    }

    if (!(committed.prim && (rayFlags & RAY_FLAG_SKIP_CLOSEST_HIT_SHADER))) {
      const char *ch = "chTri";
      if (committed.prim && committed.prim->leafType == LEAF_CUSTOM)
        ch = "chCustom";
      shade(ch, "miss");
      if (committed.prim) {
        if (committed.prim->leafType == LEAF_TRIS) {
          const Triangle *tri = (const Triangle *)committed.prim;
          expect({CLOSESTHIT, (int)tri->u, (int)tri->v});
        } else {
          expect({CLOSESTHIT + 1, committed.hit->attr0, committed.hit->attr1});
        }
        payload.val += 10;
        payload.primIdx = committed.prim->primIdx;
      } else {
        expect(MISS);
        payload.val += 1;
      }
    }

    expect(payload.val);
    expect(payload.primIdx);
    if (payload != expectedPayload)
      std::cout << count << ": simulated payload " << payload
                << " does not match expected " << expectedPayload << "\n";
  }

  void traceRay(unsigned rayFlags) { input.push_back(rayFlags); }

  void shade(const std::string &closestHit, const std::string &miss) {
    shader(closestHit);
    shader(miss);
  }

  void leafPrim(const Primitive *prim) {
    input.push_back(prim->leafType);
    input.push_back(pack(prim->primIdx, prim->geomIdx, prim->geomOpaque));
  }

  void triangle(const Triangle *tr) {
    input.push_back(asint(tr->t));
    input.push_back(asint(tr->u));
    input.push_back(asint(tr->v));
    input.push_back(asint(tr->d));
  }

  void customHit(const Custom::Hit &hit) {
    input.push_back(asint(hit.t));
    input.push_back(hit.hitKind);
    input.push_back(hit.attr0);
    input.push_back(hit.attr1);
  }

  void leafInst(int instIdx, int instId, int instFlags) {
    input.push_back(LEAF_INST);

    input.push_back(instIdx);
    input.push_back(instId);
    input.push_back(instFlags);
  }

  void shader(const std::string &shaderName) {
    shaderIdSlots[shaderName].push_back(input.size()); // fix up later
    input.push_back(-1);
  }

  void anyHitRet(int val) { input.push_back(val); }

  void endHits() { input.push_back(-1); }

  void endAccel() { input.push_back(LEAF_DONE); }

  void expect(int val) { expected.push_back(val); }

  void expect(const std::vector<int> &vals) {
    expected.insert(expected.end(), vals.begin(), vals.end());
  }

  void expect(float val) { expected.push_back(asint(val)); }
};

int TestData::count = -1;

class TraversalTester : public Tester {
public:
  TraversalTester(const std::string &deviceName, const std::string &path)
      : Tester(deviceName, path) {
    setFiles({"testTraversal.hlsl", "testTraversal2.hlsl"});
  }

  int run(const std::vector<TestData *> &tests) {
    int failedTests = 0;
    int testIndex = 0;
    for (auto td : tests) {
      std::cout << testIndex++ << " " << td->name << std::endl;

      std::vector<DxcShaderInfo> shaderIds(td->shaders.size());
      CComPtr<IDxcBlob> pComputeShader;
      if (!DxrCompile(m_dxrFallbackSupport, m_entryName, m_inputBlobPtrs,
                      td->shaders, shaderIds, false, &pComputeShader)) {
        failedTests++;
        continue;
      }

      td->setShaderIds(shaderIds);
      for (size_t i = 0; i < td->shaders.size(); ++i)
        std::cout << td->shaders[i] << ":" << shaderIds[i].Identifier << " ";
      std::cout << "\n";

      failedTests += runTest(pComputeShader, shaderIds[0].Identifier, td->input,
                             td->expected);

      delete td;
    }
    return failedTests;
  }
};

TestData *test_nohit(Payload expectedPayload) {
  TestData *td = new TestData("nohit");
  td->simulate(expectedPayload, {});
  return td;
}

TestData *test_instance_nohit(Payload expectedPayload) {
  TestData *td = new TestData("instance_nohit");
  td->simulate(expectedPayload, {inst()});
  return td;
}

TestData *test_tri(Payload expectedPayload, int anyHitRet, int instFlags = 0,
                   int rayFlags = 0, float d = 1) {
  TestData *td = new TestData("trihit");
  td->simulate(expectedPayload,
               {
                   inst(instFlags),
                   tri(1, 55, 66, anyHitRet, d),
               },
               rayFlags);
  return td;
}

struct TriHit {
  int anyHitRet;
  float t;
};

TestData *test_2tri(Payload expectedPayload, const TriHit &tri0,
                    const TriHit &tri1, int rayFlags = 0) {
  TestData *td = new TestData("trihit2");
  td->simulate(expectedPayload,
               {
                   inst(),
                   tri(tri0.t, (expectedPayload.primIdx == 0) ? 5555 : 55, 66,
                       tri0.anyHitRet, 1, 0),
                   tri(tri1.t, (expectedPayload.primIdx == 1) ? 5555 : 56, 67,
                       tri1.anyHitRet, 1, 1),
               },
               rayFlags);
  return td;
}

struct CustomHit2 {
  int anyHitRet;
  float t;
};

TestData *test_custom(Payload expectedPayload, int geomOpaque,
                      const std::vector<CustomHit2> hits, int instFlags = 0,
                      int rayFlags = 0) {
  TestData *td = new TestData("custom");
  std::vector<Custom::Hit> customHits;
  for (size_t i = 0; i < hits.size(); ++i) {
    const CustomHit2 &h = hits[i];
    customHits.push_back({h.t, 33, int(55 + i), int(66 + i), h.anyHitRet});
  }
  td->simulate(expectedPayload,
               {
                   inst(instFlags),
                   custom(customHits, geomOpaque),
               },
               rayFlags);
  return td;
}

void printUsageAndExit() {
  std::cerr
      << "Options:\n"
      << "  -h | --help                     Print this message\n"
      << "  -d | --device <name>            Name of device to use. Can be a "
         "prefix, e.g. WARP, AMD, etc.\n"
      << "  -p | --path <directory>         Base path for test input files.\n"
      << std::endl;

  exit(1);
}

int main(int argc, const char *argv[]) {
  std::string deviceName = "";
  std::string basePath = DEFAULT_TEST_FILE_PATH;

  // Parse arguments
  std::vector<std::string> args;
  for (int i = 1; i < argc; ++i)
    args.push_back(argv[i]);
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "-h" || args[i] == "--help") {
      printUsageAndExit();
    } else if (args[i] == "-d" || args[i] == "--device") {
      deviceName = args[++i];
    } else if (args[i] == "-p" || args[i] == "--path") {
      basePath = args[++i];
    } else {
      std::cerr << "Bad arg:" << args[i] << std::endl;
      printUsageAndExit();
    }
  }

  try {
    if (!deviceName.empty())
      std::cout << "Testing on device " << deviceName << std::endl;

    int numFailed = 0;
    if (1) {
      RtCompilerTester tester(deviceName, basePath);
      tester.setFiles({"testShader1.hlsl", "testShader2.hlsl"});
      numFailed += tester.runTestsWithEntryPoints({
          {"no_call", {1, 1}},
          {"no_live_values", {1, 1, -99, 2, 2}},
          {"single_call", {-99, 1, 1}},
          {"single_call_in", {10}},
          {"single_call_out", {-99, 64, 64}},
          {"single_call_inout", {10, 64, 64}},
          {"single_call_inout_passthru", {-98, 10, 64, 64}},
          {"types", {-99, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8}},
          {"multiple_calls", {-99, 1, 1, -99, 4, 4, 2, 2}},
          {"multiple_calls_with_args", {1, 1, -99, 1, 1, -99, 2, 2, 3, 3}},
          {"branch", {-99, 64, 64}},
          {"no_branch", {10, 10}},
          {"loop", {-99, 1, 1, -99, -99, -99, -99, 5, 5}},
          {"recursive", {5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0, 1, 1}},
          {"use_buffer", {-99, 10, 10}},
          {"lower_intrinsics", {-99, 0, 0}},
          {"local_array", {-99, 4, 4}},
          {"dispatch_idx_and_dims", {0, 0, 1, 1}},
      });
      numFailed +=
          tester.runSingleTest({"indirect", "indirect_callee"}, {1002}, {-99});
      numFailed +=
          tester.runSingleTest({"raygen_tri", "chTri", "intersection",
                                "continuation", "Fallback_TraceRay"},
                               {1002}, {-98, -97, 555, 666, -99, 1010});
      numFailed += tester.runSingleTest(
          {"raygen_custom", "chCustom1", "chCustom2", "intersection",
           "continuation", "Fallback_TraceRay"},
          {1003, 1005}, {-98, -95, 19,   10,  11,   12,  13,  -100, -99, 500,
                         -96, 333, 444,  -99, 1010, -98, -95, 59,   50,  51,
                         52,  53,  -100, -99, 500,  -96, 333, 444,  -99, 1110});

      tester.setFiles({"testShader3.hlsl"});
      numFailed += tester.runSingleTest({"pass_struct", "Fallback_TraceRay"},
                                        {}, {-99, 1, 2, 3, 4, 5, 6, 7, 8, 11});

      tester.setFiles({"testShader4.hlsl"});
      numFailed +=
          tester.runSingleTest({"full_trace_ray", "Fallback_TraceRay"}, {},
                               {1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15});

      tester.setFiles({"testShader5.hlsl"});
      numFailed += tester.runSingleTest(
          {"raygen", "ch1", "ch2", "miss1", "miss2", "Fallback_TraceRay"},
          {1002, 1005, 1007, 1009, 1009},
          {-99, 100, 0, -99, 101, 1,   -99, 102, 2, -99,
           103, 3,   2, 1,   0,   -99, 103, 4,   0, 21111});
    }

    if (1) {
      // Expected payload is the number of invocations in the following shader
      // types:
      //   RG AH CH MS
      // These counts are store in each digit.
      TraversalTester tester(deviceName, basePath);
      numFailed += tester.run({
          test_nohit({1001, -1}),
          test_instance_nohit({1001, -1}),
          test_tri({1010, 0}, OPAQUE),
          test_tri({1110, 0}, ACCEPT),
          test_tri({1101, -1}, IGNORE),
          test_tri({1110, 0}, TERMINATE),

          test_tri({1001, -1}, OPAQUE, 0, RAY_FLAG_CULL_OPAQUE), // culling
          test_tri(
              {1010, 0}, OPAQUE, 0, 0,
              -1), // flipping direction doesn't matter without culling flags
          test_tri({1001, -1}, OPAQUE, 0, RAY_FLAG_CULL_FRONT_FACING_TRIANGLES,
                   1), // triangle culling
          test_tri({1001, -1}, OPAQUE, 0, RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
                   -1), // triangle culling
          test_tri({1010, 0}, OPAQUE, INSTANCE_FLAG_TRIANGLE_CULL_DISABLE,
                   RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
                   -1), // disable triangle culling
          test_tri({1010, 0}, OPAQUE,
                   INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
                   1), // flip winding
          test_tri({1001, -1}, OPAQUE,
                   INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE,
                   RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 1),
          test_tri({1010, 0}, ACCEPT, INSTANCE_FLAG_FORCE_OPAQUE),
          test_tri({1010, 0}, ACCEPT, 0, RAY_FLAG_FORCE_OPAQUE),
          test_tri({1110, 0}, OPAQUE, INSTANCE_FLAG_FORCE_NON_OPAQUE),
          test_tri({1110, 0}, OPAQUE, 0, RAY_FLAG_FORCE_NON_OPAQUE),
          test_tri(
              {1010, 0}, ACCEPT, INSTANCE_FLAG_FORCE_NON_OPAQUE,
              RAY_FLAG_FORCE_OPAQUE), // ray flags opaque overrides instance
          test_tri(
              {1110, 0}, OPAQUE, INSTANCE_FLAG_FORCE_OPAQUE,
              RAY_FLAG_FORCE_NON_OPAQUE), // ray flags opaque overrides instance
          test_tri({1010, 0}, OPAQUE, INSTANCE_FLAG_TRIANGLE_CULL_DISABLE, 0,
                   -1), // disable cull
          test_tri({1000, -1}, OPAQUE, 0, RAY_FLAG_SKIP_CLOSEST_HIT_SHADER),

          test_2tri({1010, 0}, {OPAQUE, 1},
                    {OPAQUE, 2}), // pick closest (first)
          test_2tri({1010, 1}, {OPAQUE, 2},
                    {OPAQUE, 1}), // pick closest (second)
          test_2tri({1010, 0}, {OPAQUE, 2}, {OPAQUE, 1},
                    RAY_FLAG_TERMINATE_ON_FIRST_HIT),
          test_2tri({1110, 0}, {ACCEPT, 1},
                    {ACCEPT, 2}), // pick closest (first)
          test_2tri({1210, 1}, {ACCEPT, 2},
                    {ACCEPT, 1}), // pick closest (second)
          test_2tri({1110, 0}, {ACCEPT, 2}, {ACCEPT, 1},
                    RAY_FLAG_TERMINATE_ON_FIRST_HIT),
          test_2tri({1210, 1}, {IGNORE, 1},
                    {ACCEPT, 2}), // ignore first (even though closer)
          test_2tri({1210, 0}, {ACCEPT, 2},
                    {IGNORE, 1}), // ignore second (even though closer)
          test_2tri({1110, 0}, {TERMINATE, 2}, {ACCEPT, 1}),

          test_custom({1010, 0}, 1, {{ACCEPT, 1}}),
          test_custom({1110, 0}, 0, {{ACCEPT, 1}}),
          test_custom({1101, -1}, 0, {{IGNORE, 1}}),
          test_custom({1110, 0}, 0, {{TERMINATE, 1}}),
          test_custom({1110, 0}, 0,
                      {{ACCEPT, 1},
                       {ACCEPT, 2}}), // closest first - no anyhit for second
          test_custom({1210, 0}, 0,
                      {{ACCEPT, 2}, {ACCEPT, 1}}), // closest second
          test_custom({1210, 0}, 0,
                      {{IGNORE, 1}, {ACCEPT, 2}}), // ignore closer hit
          test_custom({1201, -1}, 0, {{IGNORE, 2}, {IGNORE, 1}}), // ignore both
          test_custom(
              {1201, -1}, 0,
              {{IGNORE, 1}, {IGNORE, 2}}), // ignore both - anyhit for both
          test_custom({1110, 0}, 0,
                      {{TERMINATE, 2},
                       {ACCEPT, 1}}), // terminate ==> don't handle second

          test_custom({1001, -1}, 1, {{ACCEPT, 1}}, 0, RAY_FLAG_CULL_OPAQUE),
          test_custom({1110, 0}, 0, {{ACCEPT, 1}}, 0,
                      RAY_FLAG_CULL_OPAQUE), // no effect on non-opaque
          test_custom({1010, 0}, 1, {{ACCEPT, 1}}, 0,
                      RAY_FLAG_CULL_NON_OPAQUE), // no effect on non-opaque
          test_custom({1001, -1}, 0, {{ACCEPT, 1}}, 0,
                      RAY_FLAG_CULL_NON_OPAQUE),
          test_custom({1010, 0}, 0, {{IGNORE, 1}},
                      INSTANCE_FLAG_FORCE_OPAQUE), // no anyhit
          test_custom({1010, 0}, 0, {{IGNORE, 1}}, 0, RAY_FLAG_FORCE_OPAQUE),
          test_custom({1101, -1}, 1, {{IGNORE, 1}},
                      INSTANCE_FLAG_FORCE_NON_OPAQUE), // anyhit drops the hit
          test_custom({1101, -1}, 1, {{IGNORE, 1}}, 0,
                      RAY_FLAG_FORCE_NON_OPAQUE),
          test_custom(
              {1010, 0}, 0, {{IGNORE, 1}}, INSTANCE_FLAG_FORCE_NON_OPAQUE,
              RAY_FLAG_FORCE_OPAQUE), // ray flags opaque overrides instance
          test_custom(
              {1101, -1}, 1, {{IGNORE, 1}}, INSTANCE_FLAG_FORCE_OPAQUE,
              RAY_FLAG_FORCE_NON_OPAQUE), // ray flags opaque overrides instance
          test_custom({1001, -1}, 0, {{ACCEPT, 1}}, INSTANCE_FLAG_FORCE_OPAQUE,
                      RAY_FLAG_CULL_OPAQUE),
          test_custom({1100, -1}, 0, {{ACCEPT, 1}}, 0,
                      RAY_FLAG_SKIP_CLOSEST_HIT_SHADER),
          test_custom({1210, 0}, 0, {{IGNORE, 3}, {ACCEPT, 2}, {ACCEPT, 1}}, 0,
                      RAY_FLAG_TERMINATE_ON_FIRST_HIT),
      });
    }

    std::cout << "===============================================\n";
    if (numFailed == 0)
      std::cout << "PASSED\n";
    else {
      std::cout << "FAILED\n";
      std::cout << numFailed << " tests failed\n";
    }
  } catch (...) {
    printf("Failed - unknown error.\n");
    return 1;
  }
}
