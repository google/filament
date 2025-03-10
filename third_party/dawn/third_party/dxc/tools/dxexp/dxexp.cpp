///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxexp.cpp                                                                 //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides a command-line tool to detect the status of D3D experimental     //
// feature support for experimental shaders.                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <atlbase.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

// A more recent Windows SDK than currently required is needed for these.
typedef HRESULT(WINAPI *D3D12EnableExperimentalFeaturesFn)(
    UINT NumFeatures, __in_ecount(NumFeatures) const IID *pIIDs,
    __in_ecount_opt(NumFeatures) void *pConfigurationStructs,
    __in_ecount_opt(NumFeatures) UINT *pConfigurationStructSizes);

static const GUID D3D12ExperimentalShaderModelsID =
    {/* 76f5573e-f13a-40f5-b297-81ce9e18933f */
     0x76f5573e,
     0xf13a,
     0x40f5,
     {0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f}};

static HRESULT AtlCheck(HRESULT hr) {
  if (FAILED(hr))
    AtlThrow(hr);
  return hr;
}

// Not defined in Creators Update version of d3d12.h:
#if WDK_NTDDI_VERSION <= NTDDI_WIN10_RS2
#define D3D12_FEATURE_D3D12_OPTIONS3 ((D3D12_FEATURE)21)
typedef enum D3D12_COMMAND_LIST_SUPPORT_FLAGS {
  D3D12_COMMAND_LIST_SUPPORT_FLAG_NONE = 0,
  D3D12_COMMAND_LIST_SUPPORT_FLAG_DIRECT =
      (1 << D3D12_COMMAND_LIST_TYPE_DIRECT),
  D3D12_COMMAND_LIST_SUPPORT_FLAG_BUNDLE =
      (1 << D3D12_COMMAND_LIST_TYPE_BUNDLE),
  D3D12_COMMAND_LIST_SUPPORT_FLAG_COMPUTE =
      (1 << D3D12_COMMAND_LIST_TYPE_COMPUTE),
  D3D12_COMMAND_LIST_SUPPORT_FLAG_COPY = (1 << D3D12_COMMAND_LIST_TYPE_COPY),
  D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_DECODE = (1 << 4),
  D3D12_COMMAND_LIST_SUPPORT_FLAG_VIDEO_PROCESS = (1 << 5)
} D3D12_COMMAND_LIST_SUPPORT_FLAGS;

typedef enum D3D12_VIEW_INSTANCING_TIER {
  D3D12_VIEW_INSTANCING_TIER_NOT_SUPPORTED = 0,
  D3D12_VIEW_INSTANCING_TIER_1 = 1,
  D3D12_VIEW_INSTANCING_TIER_2 = 2,
  D3D12_VIEW_INSTANCING_TIER_3 = 3
} D3D12_VIEW_INSTANCING_TIER;

typedef struct D3D12_FEATURE_DATA_D3D12_OPTIONS3 {
  BOOL CopyQueueTimestampQueriesSupported;
  BOOL CastingFullyTypedFormatSupported;
  DWORD WriteBufferImmediateSupportFlags;
  D3D12_VIEW_INSTANCING_TIER ViewInstancingTier;
  BOOL BarycentricsSupported;
} D3D12_FEATURE_DATA_D3D12_OPTIONS3;
#endif

#ifndef NTDDI_WIN10_RS3
#define NTDDI_WIN10_RS3 0x0A000004
#endif

#if WDK_NTDDI_VERSION <= NTDDI_WIN10_RS3
#define D3D12_FEATURE_D3D12_OPTIONS4 ((D3D12_FEATURE)23)
typedef enum D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER {
  D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_0,
  D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_1,
} D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER;

typedef struct D3D12_FEATURE_DATA_D3D12_OPTIONS4 {
  BOOL ReservedBufferPlacementSupported;
  D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER SharedResourceCompatibilityTier;
  BOOL Native16BitShaderOpsSupported;
} D3D12_FEATURE_DATA_D3D12_OPTIONS4;
#endif

#ifndef NTDDI_WIN10_RS4
#define NTDDI_WIN10_RS4 0x0A000005
#endif

#if WDK_NTDDI_VERSION <= NTDDI_WIN10_RS4
#define D3D12_FEATURE_D3D12_OPTIONS5 ((D3D12_FEATURE)27)
typedef enum D3D12_RENDER_PASS_TIER {
  D3D12_RENDER_PASS_TIER_0 = 0,
  D3D12_RENDER_PASS_TIER_1 = 1,
  D3D12_RENDER_PASS_TIER_2 = 2
} D3D12_RENDER_PASS_TIER;

typedef enum D3D12_RAYTRACING_TIER {
  D3D12_RAYTRACING_TIER_NOT_SUPPORTED = 0,
  D3D12_RAYTRACING_TIER_1_0 = 10 D3D12_RAYTRACING_TIER_1_1 = 11
} D3D12_RAYTRACING_TIER;

typedef struct D3D12_FEATURE_DATA_D3D12_OPTIONS5 {
  BOOL SRVOnlyTiledResourceTier3;
  D3D12_RENDER_PASS_TIER RenderPassesTier;
  D3D12_RAYTRACING_TIER RaytracingTier;
} D3D12_FEATURE_DATA_D3D12_OPTIONS5;
#endif

#ifndef NTDDI_WIN10_VB
#define NTDDI_WIN10_VB 0x0A000008
#endif

#if WDK_NTDDI_VERSION < NTDDI_WIN10_VB
#define D3D12_FEATURE_D3D12_OPTIONS7 ((D3D12_FEATURE)32)

typedef enum D3D12_MESH_SHADER_TIER {
  D3D12_MESH_SHADER_TIER_NOT_SUPPORTED = 0,
  D3D12_MESH_SHADER_TIER_1 = 10
} D3D12_MESH_SHADER_TIER;

typedef enum D3D12_SAMPLER_FEEDBACK_TIER {
  D3D12_SAMPLER_FEEDBACK_TIER_NOT_SUPPORTED = 0,
  D3D12_SAMPLER_FEEDBACK_TIER_0_9 = 90,
  D3D12_SAMPLER_FEEDBACK_TIER_1_0 = 100
} D3D12_SAMPLER_FEEDBACK_TIER;

typedef struct D3D12_FEATURE_DATA_D3D12_OPTIONS7 {
  D3D12_MESH_SHADER_TIER MeshShaderTier;
  D3D12_SAMPLER_FEEDBACK_TIER SamplerFeedbackTier;
} D3D12_FEATURE_DATA_D3D12_OPTIONS7;
#endif

#ifndef NTDDI_WIN10_FE
#define NTDDI_WIN10_FE 0x0A00000A
#endif

#if WDK_NTDDI_VERSION < NTDDI_WIN10_FE
#define D3D12_FEATURE_D3D12_OPTIONS9 ((D3D12_FEATURE)37)

typedef enum D3D12_WAVE_MMA_TIER {
  D3D12_WAVE_MMA_TIER_NOT_SUPPORTED = 0,
  D3D12_WAVE_MMA_TIER_1_0 = 10
} D3D12_WAVE_MMA_TIER;

typedef struct D3D12_FEATURE_DATA_D3D12_OPTIONS9 {
  BOOL MeshShaderPipelineStatsSupported;
  BOOL MeshShaderSupportsFullRangeRenderTargetArrayIndex;
  BOOL AtomicInt64OnTypedResourceSupported;
  BOOL AtomicInt64OnGroupSharedSupported;
  BOOL DerivativesInMeshAndAmplificationShadersSupported;
  D3D12_WAVE_MMA_TIER WaveMMATier;
} D3D12_FEATURE_DATA_D3D12_OPTIONS9;
#endif

#ifndef NTDDI_WIN10_NI
#define NTDDI_WIN10_NI 0x0A00000C
#endif

#if WDK_NTDDI_VERSION <= NTDDI_WIN10_NI
#define D3D12_FEATURE_D3D12_OPTIONS14 ((D3D12_FEATURE)43)
typedef struct D3D12_FEATURE_DATA_D3D12_OPTIONS14 {
  BOOL AdvancedTextureOpsSupported;
  BOOL WriteableMSAATexturesSupported;
  BOOL IndependentFrontAndBackStencilRefMaskSupported;
} D3D12_FEATURE_DATA_D3D12_OPTIONS14;
#endif

#pragma warning(disable : 4063)
#define D3D12_RAYTRACING_TIER_1_1 ((D3D12_RAYTRACING_TIER)11)
#define D3D_SHADER_MODEL_6_1 ((D3D_SHADER_MODEL)0x61)
#define D3D_SHADER_MODEL_6_2 ((D3D_SHADER_MODEL)0x62)
#define D3D_SHADER_MODEL_6_3 ((D3D_SHADER_MODEL)0x63)
#define D3D_SHADER_MODEL_6_4 ((D3D_SHADER_MODEL)0x64)
#define D3D_SHADER_MODEL_6_5 ((D3D_SHADER_MODEL)0x65)
#define D3D_SHADER_MODEL_6_6 ((D3D_SHADER_MODEL)0x66)
#define D3D_SHADER_MODEL_6_7 ((D3D_SHADER_MODEL)0x67)
#define D3D_SHADER_MODEL_6_8 ((D3D_SHADER_MODEL)0x68)

#define DXEXP_HIGHEST_SHADER_MODEL D3D_SHADER_MODEL_6_8

static const char *BoolToStrJson(bool value) {
  return value ? "true" : "false";
}

static const char *BoolToStrText(bool value) { return value ? "YES" : "NO"; }

static const char *(*BoolToStr)(bool value);
static bool IsOutputJson;

#define json_printf(...)                                                       \
  if (IsOutputJson) {                                                          \
    printf(__VA_ARGS__);                                                       \
  }
#define text_printf(...)                                                       \
  if (!IsOutputJson) {                                                         \
    printf(__VA_ARGS__);                                                       \
  }

static const char *ShaderModelToStr(D3D_SHADER_MODEL SM) {
  switch ((UINT32)SM) {
  case D3D_SHADER_MODEL_5_1:
    return "5.1";
  case D3D_SHADER_MODEL_6_0:
    return "6.0";
  case D3D_SHADER_MODEL_6_1:
    return "6.1";
  case D3D_SHADER_MODEL_6_2:
    return "6.2";
  case D3D_SHADER_MODEL_6_3:
    return "6.3";
  case D3D_SHADER_MODEL_6_4:
    return "6.4";
  case D3D_SHADER_MODEL_6_5:
    return "6.5";
  case D3D_SHADER_MODEL_6_6:
    return "6.6";
  case D3D_SHADER_MODEL_6_7:
    return "6.7";
  case D3D_SHADER_MODEL_6_8:
    return "6.8";
  default:
    return "ERROR";
  }
}

static const char *ViewInstancingTierToStr(D3D12_VIEW_INSTANCING_TIER Tier) {
  switch (Tier) {
  case D3D12_VIEW_INSTANCING_TIER_NOT_SUPPORTED:
    return "NO";
  case D3D12_VIEW_INSTANCING_TIER_1:
    return "Tier1";
  case D3D12_VIEW_INSTANCING_TIER_2:
    return "Tier2";
  case D3D12_VIEW_INSTANCING_TIER_3:
    return "Tier3";
  default:
    return "ERROR";
  }
}

static const char *RaytracingTierToStr(D3D12_RAYTRACING_TIER Tier) {
  switch (Tier) {
  case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
    return "NO";
  case D3D12_RAYTRACING_TIER_1_0:
    return "1.0";
  case D3D12_RAYTRACING_TIER_1_1:
    return "1.1";
  default:
    return "ERROR";
  }
}

static const char *MeshShaderTierToStr(D3D12_MESH_SHADER_TIER Tier) {
  switch (Tier) {
  case D3D12_MESH_SHADER_TIER_NOT_SUPPORTED:
    return "NO";
  case D3D12_MESH_SHADER_TIER_1:
    return "1";
  default:
    return "ERROR";
  }
}

static const char *WaveMatrixTierToStr(D3D12_WAVE_MMA_TIER Tier) {
  switch (Tier) {
  case D3D12_WAVE_MMA_TIER_NOT_SUPPORTED:
    return "NO";
  case D3D12_WAVE_MMA_TIER_1_0:
    return "1";
  default:
    return "ERROR";
  }
}

static HRESULT
GetHighestShaderModel(ID3D12Device *pDevice,
                      D3D12_FEATURE_DATA_SHADER_MODEL &DeviceSM) {
  HRESULT hr = E_INVALIDARG;
  D3D_SHADER_MODEL SM = DXEXP_HIGHEST_SHADER_MODEL;
  while (hr == E_INVALIDARG && SM >= D3D_SHADER_MODEL_6_0) {
    DeviceSM.HighestShaderModel = SM;
    hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &DeviceSM,
                                      sizeof(DeviceSM));
    SM = (D3D_SHADER_MODEL)((UINT32)SM - 1);
  }
  return hr;
}

static HRESULT PrintAdapters() {
  HRESULT hr = S_OK;
  char comma = ' ';
  json_printf("{ \"adapters\" : [\n");
  try {
    CComPtr<IDXGIFactory2> pFactory;
    AtlCheck(CreateDXGIFactory2(0, IID_PPV_ARGS(&pFactory)));
    UINT AdapterIndex = 0;
    for (;;) {
      CComPtr<IDXGIAdapter1> pAdapter;
      CComPtr<ID3D12Device> pDevice;
      HRESULT hrEnum = pFactory->EnumAdapters1(AdapterIndex, &pAdapter);
      if (hrEnum == DXGI_ERROR_NOT_FOUND)
        break;
      AtlCheck(hrEnum);
      DXGI_ADAPTER_DESC1 AdapterDesc;
      D3D12_FEATURE_DATA_D3D12_OPTIONS1 DeviceOptions;
      D3D12_FEATURE_DATA_D3D12_OPTIONS3 DeviceOptions3;
      D3D12_FEATURE_DATA_D3D12_OPTIONS4 DeviceOptions4;
      D3D12_FEATURE_DATA_D3D12_OPTIONS5 DeviceOptions5;
      D3D12_FEATURE_DATA_D3D12_OPTIONS7 DeviceOptions7;
      D3D12_FEATURE_DATA_D3D12_OPTIONS9 DeviceOptions9;
      D3D12_FEATURE_DATA_D3D12_OPTIONS14 DeviceOptions14;

      memset(&DeviceOptions, 0, sizeof(DeviceOptions));
      memset(&DeviceOptions3, 0, sizeof(DeviceOptions3));
      memset(&DeviceOptions4, 0, sizeof(DeviceOptions4));
      memset(&DeviceOptions5, 0, sizeof(DeviceOptions5));
      memset(&DeviceOptions7, 0, sizeof(DeviceOptions7));
      memset(&DeviceOptions9, 0, sizeof(DeviceOptions9));
      memset(&DeviceOptions14, 0, sizeof(DeviceOptions14));
      D3D12_FEATURE_DATA_SHADER_MODEL DeviceSM;
      AtlCheck(pAdapter->GetDesc1(&AdapterDesc));
      AtlCheck(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0,
                                 IID_PPV_ARGS(&pDevice)));
      AtlCheck(pDevice->CheckFeatureSupport(
          D3D12_FEATURE_D3D12_OPTIONS1, &DeviceOptions, sizeof(DeviceOptions)));
      pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3,
                                   &DeviceOptions3, sizeof(DeviceOptions3));
      pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4,
                                   &DeviceOptions4, sizeof(DeviceOptions4));
      pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,
                                   &DeviceOptions5, sizeof(DeviceOptions5));
      pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7,
                                   &DeviceOptions7, sizeof(DeviceOptions7));
      pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS9,
                                   &DeviceOptions9, sizeof(DeviceOptions9));
      pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS14,
                                   &DeviceOptions14, sizeof(DeviceOptions14));

      AtlCheck(GetHighestShaderModel(pDevice, DeviceSM));
      const char *Format =
          IsOutputJson
              ? "%c { \"name\": \"%S\", \"sm\": \"%s\", \"wave\": %s, \"i64\": "
                "%s, \"bary\": %s, \"view-inst\": \"%s\", \"16bit\": %s, "
                "\"raytracing\": \"%s\", \"mesh\": \"%s\", \"deriv-ms-as\": "
                "\"%s\", \"wave-matrix\" : \"%s\", \"ato\":\"%s\" }\n"
              : "%c %S - Highest SM [%s] Wave [%s] I64 [%s] Barycentrics [%s] "
                "View Instancing [%s] 16bit Support [%s] Raytracing [%s] "
                "Mesh Shaders [%s] Derivatives in Mesh/Amp Shaders [%s] "
                "Wave Matrix [%s] Advanced Texture Ops [%s]\n";
      printf(
          Format, comma, AdapterDesc.Description,
          ShaderModelToStr(DeviceSM.HighestShaderModel),
          BoolToStr(DeviceOptions.WaveOps),
          BoolToStr(DeviceOptions.Int64ShaderOps),
          BoolToStr(DeviceOptions3.BarycentricsSupported),
          ViewInstancingTierToStr(DeviceOptions3.ViewInstancingTier),
          BoolToStr(DeviceOptions4.Native16BitShaderOpsSupported),
          RaytracingTierToStr(DeviceOptions5.RaytracingTier),
          MeshShaderTierToStr(DeviceOptions7.MeshShaderTier),
          BoolToStr(
              DeviceOptions9.DerivativesInMeshAndAmplificationShadersSupported),
          WaveMatrixTierToStr(DeviceOptions9.WaveMMATier),
          BoolToStr(DeviceOptions14.AdvancedTextureOpsSupported));
      AdapterIndex++;
      comma = IsOutputJson ? ',' : ' ';
    }
  } catch (ATL::CAtlException &e) {
    hr = e.m_hr;
    json_printf("%c { \"err\": \"unable to print information for adapters - "
                "0x%08x\" }\n",
                comma, (unsigned int)hr);
    text_printf("%s", "Unable to print information for adapters.\n");
  }
  json_printf("  ] }\n");
  return hr;
}

// Return codes:
// 0 - experimental mode worked
// 1 - cannot load d3d12.dll
// 2 - cannot find D3D12EnableExperimentalFeatures
// 3 - experimental shader mode interface unsupported
// 4 - other error
int main(int argc, const char *argv[]) {
  BoolToStr = BoolToStrText;
  IsOutputJson = false;

  if (argc > 1) {
    const char *pArg = argv[1];
    if (0 == strcmp(pArg, "-?") || 0 == strcmp(pArg, "--?") ||
        0 == strcmp(pArg, "/?")) {
      printf("Checks the available of D3D support for experimental shader "
             "models.\n\n");
      printf("dxexp [-json]\n\n");
      printf("Sets errorlevel to 0 on success, non-zero for failure cases.\n");
      return 4;
    } else if (0 == strcmp(pArg, "-json") || 0 == strcmp(pArg, "/json")) {
      IsOutputJson = true;
      BoolToStr = BoolToStrJson;
    } else {
      printf("Unrecognized command line arguments.\n");
      return 4;
    }
  }

  DWORD err;
  HMODULE hRuntime;
  hRuntime = LoadLibraryW(L"d3d12.dll");
  if (hRuntime == NULL) {
    err = GetLastError();
    printf("Failed to load library d3d12.dll - Win32 error %u\n",
           (unsigned int)err);
    return 1;
  }

  json_printf("{ \"noexp\":\n");
  text_printf(
      "Adapter feature support without experimental shaders enabled:\n");

  if (FAILED(PrintAdapters())) {
    return 4;
  }
  FreeLibrary(hRuntime);

  json_printf(" , \"exp\":");
  text_printf(
      "-------------------------------------------------------------\n");

  hRuntime = LoadLibraryW(L"d3d12.dll");
  if (hRuntime == NULL) {
    err = GetLastError();
    printf("Failed to load library d3d12.dll - Win32 error %u\n",
           (unsigned int)err);
    return 1;
  }

  D3D12EnableExperimentalFeaturesFn pD3D12EnableExperimentalFeatures =
      (D3D12EnableExperimentalFeaturesFn)GetProcAddress(
          hRuntime, "D3D12EnableExperimentalFeatures");
  if (pD3D12EnableExperimentalFeatures == nullptr) {
    err = GetLastError();
    printf("Failed to find export 'D3D12EnableExperimentalFeatures' in "
           "d3d12.dll - Win32 error %u%s\n",
           (unsigned int)err,
           err == ERROR_PROC_NOT_FOUND
               ? " (The specified procedure could not be found.)"
               : "");
    printf("Consider verifying the operating system version - Creators Update "
           "or newer "
           "is currently required.\n");
    PrintAdapters();
    return 2;
  }

  HRESULT hr = pD3D12EnableExperimentalFeatures(
      1, &D3D12ExperimentalShaderModelsID, nullptr, nullptr);
  if (SUCCEEDED(hr)) {
    text_printf("Experimental shader model feature succeeded.\n");
    hr = PrintAdapters();
    json_printf("\n}\n");
    return (SUCCEEDED(hr)) ? 0 : 4;
  } else if (hr == E_NOINTERFACE) {
    text_printf(
        "Experimental shader model feature failed with error E_NOINTERFACE.\n");
    text_printf(
        "The most likely cause is that Windows Developer mode is not on.\n");
    text_printf("See "
                "https://msdn.microsoft.com/en-us/windows/uwp/get-started/"
                "enable-your-device-for-development\n");
    json_printf("{ \"err\": \"E_NOINTERFACE\" }");
    json_printf("\n}\n");
    return 3;
  } else if (hr == E_INVALIDARG) {
    text_printf(
        "Experimental shader model feature failed with error E_INVALIDARG.\n");
    text_printf("This means the configuration of a feature is incorrect, the "
                "set of features passed\n"
                "in are known to be incompatible with each other, or other "
                "errors occured,\n"
                "and is generally unexpected for the experimental shader model "
                "feature.\n");
    json_printf("{ \"err\": \"E_INVALIDARG\" }");
    json_printf("\n}\n");
    return 4;
  } else {
    text_printf("Experimental shader model feature failed with unexpected "
                "HRESULT 0x%08x.\n",
                (unsigned int)hr);
    json_printf("{ \"err\": \"0x%08x\" }", (unsigned int)hr);
    json_printf("\n}\n");
    return 4;
  }
}
