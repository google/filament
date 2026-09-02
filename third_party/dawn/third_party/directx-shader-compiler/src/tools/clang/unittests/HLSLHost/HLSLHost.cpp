///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HLSLHost.cpp                                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides a Win32 application that can act as a host for HLSL programs.    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/microcom.h"
#include "llvm/Support/Compiler.h" // for LLVM_FALLTHROUGH
#include <algorithm>
#include <atlcoll.h>
#include <comdef.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <string>
#include <unordered_map>
#include <vector>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

#if 0

Pending work for rendering hosting.

- Pass width / height information to the test DLL.
- Clean up all TODOs markers.

#endif

// Forward declarations.
class ServerFactory;

// RAII helpers.
class HhEvent {
public:
  HANDLE m_handle = INVALID_HANDLE_VALUE;

public:
  HRESULT Init() {
    if (m_handle == INVALID_HANDLE_VALUE) {
      m_handle = CreateEvent(nullptr, TRUE, FALSE, nullptr);
      if (m_handle == INVALID_HANDLE_VALUE) {
        return HRESULT_FROM_WIN32(GetLastError());
      }
    }
    return S_OK;
  }
  void SetEvent() { ::SetEvent(m_handle); }
  void ResetEvent() { ::ResetEvent(m_handle); }
  ~HhEvent() {
    if (m_handle != INVALID_HANDLE_VALUE) {
      CloseHandle(m_handle);
    }
  }
};

class HhCriticalSection {
private:
  CRITICAL_SECTION m_cs;

public:
  HhCriticalSection() { InitializeCriticalSection(&m_cs); }
  ~HhCriticalSection() { DeleteCriticalSection(&m_cs); }
  class Lock {
  private:
    CRITICAL_SECTION *m_pLock;

  public:
    Lock() = delete;
    Lock(const Lock &) = delete;
    Lock(Lock &&other) { std::swap(m_pLock, other.m_pLock); }
    Lock(CRITICAL_SECTION *pLock) : m_pLock(pLock) {
      EnterCriticalSection(m_pLock);
    }
    ~Lock() {
      if (m_pLock)
        LeaveCriticalSection(m_pLock);
    }
  };
  Lock LockCS() { return Lock(&m_cs); }
};

class ClassObjectRegistration {
private:
  DWORD m_reg = 0;
  HRESULT m_hr = E_FAIL;

public:
  HRESULT Register(REFCLSID rclsid, IUnknown *pUnk, DWORD dwClsContext,
                   DWORD flags) {
    m_hr = CoRegisterClassObject(rclsid, pUnk, dwClsContext, flags, &m_reg);
    return m_hr;
  }
  ~ClassObjectRegistration() {
    if (SUCCEEDED(m_hr))
      CoRevokeClassObject(m_reg);
  }
};

class CoInit {
private:
  HRESULT m_hr = E_FAIL;

public:
  HRESULT Initialize(DWORD dwCoInit) {
    m_hr = CoInitializeEx(nullptr, dwCoInit);
    return m_hr;
  }
  ~CoInit() {
    if (SUCCEEDED(m_hr))
      CoUninitialize();
  }
};

// Globals.
static const GUID
    CLSID_HLSLHostServer = // {7FD7A859-6C6B-4352-8F1E-C67BB62E774B}
    {0x7fd7a859,
     0x6c6b,
     0x4352,
     {0x8f, 0x1e, 0xc6, 0x7b, 0xb6, 0x2e, 0x77, 0x4b}};
static HhEvent g_ShutdownServerEvent;

static const DWORD GetPidMsgId = 1;
static const DWORD ShutdownMsgId = 2;
static const DWORD StartRendererMsgId = 3;
static const DWORD StopRendererMsgId = 4;
static const DWORD SetPayloadMsgId = 5;
static const DWORD ReadLogMsgId = 6;
static const DWORD SetSizeMsgId = 7;
static const DWORD SetParentWndMsgId = 8;
static const DWORD GetPidMsgReplyId = 100 + GetPidMsgId;
static const DWORD ReadLogMsgReplyId = 100 + ReadLogMsgId;

struct HhMessageHeader {
  DWORD Length;
  DWORD Kind;
};
struct HhGetPidMessage {
  HhMessageHeader Header;
};
struct HhSetSizeMessage {
  HhMessageHeader Header;
  DWORD Width;
  DWORD Height;
};
struct HhSetParentWndMessage {
  HhMessageHeader Header;
  UINT64 Handle;
};
struct HhGetPidReply {
  HhMessageHeader Header;
  DWORD Pid;
};
struct HhResultReply {
  HhMessageHeader Header;
  HRESULT hr;
};

// Logging and tracing.
static void HhTrace(LPCWSTR pMessage) { wprintf(L"%s\n", pMessage); }

template <typename TInterface, typename TObject>
HRESULT DoBasicQueryInterfaceWithRemote(TObject *self, REFIID iid,
                                        void **ppvObject) {
  if (ppvObject == nullptr)
    return E_POINTER;

  // Support INoMarshal to void GIT shenanigans.
  if (IsEqualIID(iid, __uuidof(IUnknown))) {
    *ppvObject = reinterpret_cast<IUnknown *>(self);
    reinterpret_cast<IUnknown *>(self)->AddRef();
    return S_OK;
  }

  if (IsEqualIID(iid, __uuidof(TInterface))) {
    *(TInterface **)ppvObject = self;
    self->AddRef();
    return S_OK;
  }

  return E_NOINTERFACE;
}

// Rendering.
ATOM g_RenderingWindowClass;
HhCriticalSection g_RenderingWindowClassCS;
LRESULT CALLBACK RendererWndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI RendererStart(LPVOID lpThreadParameter);
void __stdcall RendererLog(void *pRenderer, const wchar_t *pText);
typedef void(__stdcall *OutputStringFn)(void *pCtx, const wchar_t *pText);
typedef HRESULT(WINAPI *InitOpTestFn)(void *pStrCtx,
                                      OutputStringFn pOutputStrFn);
typedef HRESULT(WINAPI *RunOpTestFn)(void *pStrCtx, OutputStringFn pOutputStrFn,
                                     LPCSTR pText, ID3D12Device *pDevice,
                                     ID3D12CommandQueue *pCommandQueue,
                                     ID3D12Resource *pRenderTarget,
                                     char **pReadBackDump);
#define WM_RENDERER_SETPAYLOAD (WM_USER)
#define WM_RENDERER_QUIT (WM_USER + 1)

class Renderer {
private:
  // This state is accessed by a messaging thread.
  DWORD m_tid = 0;
  HANDLE m_thread = nullptr;
  // This state is used to coordinate the messaging and the rendering threads.
  HWND m_hwnd = nullptr;
  HhEvent m_threadReady;
  HRESULT m_threadStartResult = E_PENDING;
  UINT m_height = 0;
  UINT m_width = 0;
  OutputStringFn m_pLog;
  void *m_pLogCtx;
  bool m_userQuit = false;
  // This state is used by the rendering thread.
  CComPtr<ID3D12Device> m_device;
  CComPtr<ID3D12CommandQueue> m_commandQueue;
  CComPtr<IDXGISwapChain3> m_swapChain;
  UINT FrameCount = 2;
  UINT m_TargetDeviceIndex = 0;
  UINT m_frameIndex;
  UINT m_renderCount = 0;
  HMODULE m_TestDLL = NULL;
  RunOpTestFn m_pRunOpTestFn = nullptr;
  InitOpTestFn m_pInitOpTestFn = nullptr;
  LPVOID m_ShaderOpText = nullptr;
  CComHeapPtr<char> m_ResourceViewText;

  HRESULT LoadTestDll() {
    if (m_TestDLL == NULL) {
      m_TestDLL = LoadLibrary("ClangHLSLTests.dll");
      m_pRunOpTestFn = (RunOpTestFn)GetProcAddress(m_TestDLL, "RunOpTest");
      m_pInitOpTestFn =
          (InitOpTestFn)GetProcAddress(m_TestDLL, "InitializeOpTests");
      HRESULT hrInit = m_pInitOpTestFn(this, RendererLog);
      if (FAILED(hrInit)) {
        CloseHandle(m_TestDLL);
        m_TestDLL = nullptr;
        return hrInit;
      }
    }
    return S_OK;
  }

  void HandleCopy() {
    LPCSTR OpMessage;
    UINT MessageType = MB_ICONERROR;
    if (m_ResourceViewText.m_pData == nullptr) {
      OpMessage = "No resources read back from a prior render.";
    } else {
      OpMessage = "Unable to copy resource data to clipboard.";
      if (OpenClipboard(m_hwnd)) {
        if (EmptyClipboard()) {
          HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE,
                                     1 + strlen(m_ResourceViewText.m_pData));
          if (hMem) {
            LPSTR pCopy = (LPSTR)GlobalLock(hMem);
            strcpy(pCopy, m_ResourceViewText.m_pData);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
            OpMessage = "Read back resources copied to clipboard.";
            MessageType = MB_ICONINFORMATION;
          }
        }
        CloseClipboard();
      }
    }
    MessageBox(m_hwnd, OpMessage, "Resource Copy", MessageType);
  }

  void HandleDeviceCycle() {
    ReleaseD3DResources();
    ++m_TargetDeviceIndex;
    SetupD3D();
  }

  void HandleHelp() {
    MessageBoxW(m_hwnd,
                L"Commands:\r\n"
                L"(C)opy Results\r\n"
                L"(D)evice Cycle\r\n"
                L"(H)elp (show this message)\r\n"
                L"(R)un\r\n"
                L"(Q)uit",
                L"HLSL Host Help", MB_OK);
  }

  HRESULT HandlePayload() {
    CComPtr<ID3D12Resource> pRT;
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    IFR(m_swapChain->GetBuffer(m_frameIndex, IID_PPV_ARGS(&pRT)));
    wchar_t ResName[32];
    StringCchPrintfW(ResName, _countof(ResName), L"SwapChain Buffer #%u",
                     m_frameIndex);
    pRT->SetName(ResName);
    StringCchPrintfW(ResName, _countof(ResName), L"Render %u\r\n",
                     ++m_renderCount);
    Log(ResName);
    m_ResourceViewText.Free();
    LPSTR pText = (LPSTR)InterlockedExchangePointer(&m_ShaderOpText, nullptr);
    HRESULT hr = m_pRunOpTestFn(this, RendererLog, pText, m_device,
                                m_commandQueue, pRT, &m_ResourceViewText);
    // If we can restore it, we're set; otherwise we should delete our stale
    // copy.
    if (nullptr !=
        InterlockedCompareExchangePointer(&m_ShaderOpText, pText, nullptr))
      free(pText);
    wchar_t ErrMsg[64];
    if (FAILED(hr)) {
      StringCchPrintfW(ErrMsg, _countof(ErrMsg),
                       L"Shader operation failed: 0x%08x\r\n", hr);
      Log(ErrMsg);
      return hr;
    }
    hr = m_swapChain->Present(1, 0);
    if (FAILED(hr)) {
      StringCchPrintfW(ErrMsg, _countof(ErrMsg), L"Present failed: 0x%08x\r\n",
                       hr);
      Log(ErrMsg);
      return hr;
    }
    return S_OK;
  }

  void ReleaseD3DResources() {
    m_device.Release();
    m_commandQueue.Release();
    m_swapChain.Release();
  }

  HRESULT SetupD3D() {
    IFR(LoadTestDll());

    CComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
      debugController->EnableDebugLayer();
    }

    CComPtr<IDXGIFactory4> factory;
    IFR(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

    CComPtr<IDXGIAdapter> adapter;
    if (m_TargetDeviceIndex > 0) {
      UINT hardwareIndex = m_TargetDeviceIndex - 1;
      HRESULT hrEnum = factory->EnumAdapters(hardwareIndex, &adapter);
      if (hrEnum == DXGI_ERROR_NOT_FOUND) {
        m_TargetDeviceIndex = 0; // cycle to WARP
      } else {
        IFR(hrEnum);
      }
    }
    if (m_TargetDeviceIndex == 0) {
      IFR(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)));
    }

    HRESULT hrCreate = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0,
                                         IID_PPV_ARGS(&m_device));
    IFR(SetWindowTextToDevice(hrCreate, m_hwnd, adapter, m_device));
    IFR(hrCreate);

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    IFR(m_device->CreateCommandQueue(&queueDesc,
                                     IID_PPV_ARGS(&m_commandQueue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    CComPtr<IDXGISwapChain1> swapChain;
    IFR(factory->CreateSwapChainForHwnd(m_commandQueue, m_hwnd, &swapChainDesc,
                                        nullptr, nullptr, &swapChain));

    // Do not support fullscreen transitions.
    IFR(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

    IFR(swapChain.QueryInterface(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    return S_OK;
  }

  HRESULT SetWindowTextToDevice(HRESULT hrCreate, HWND hwnd,
                                IDXGIAdapter *pAdapter, ID3D12Device *pDevice) {
    DXGI_ADAPTER_DESC AdapterDesc;
    D3D12_FEATURE_DATA_D3D12_OPTIONS1 DeviceOptions;
    D3D12_FEATURE_DATA_SHADER_MODEL DeviceSM;
    wchar_t title[200];
    IFR(pAdapter->GetDesc(&AdapterDesc));
    IFR(pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1,
                                     &DeviceOptions, sizeof(DeviceOptions)));
    DeviceSM.HighestShaderModel = D3D_SHADER_MODEL_6_0;
    IFR(pDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &DeviceSM,
                                     sizeof(DeviceSM)));
    IFR(StringCchPrintfW(
        title, _countof(title), L"%s%s - caps:%s%s%s",
        SUCCEEDED(hrCreate) ? L"" : L"(cannot create D3D12 device)",
        AdapterDesc.Description,
        (DeviceSM.HighestShaderModel >= D3D_SHADER_MODEL_6_0) ? L" SM6" : L"",
        DeviceOptions.WaveOps ? L" WaveOps" : L"",
        DeviceOptions.Int64ShaderOps ? L" I64" : L""));
    SetWindowTextW(hwnd, title);
    return S_OK;
  }

public:
  HRESULT Start(void *pLogCtx, OutputStringFn pLog) {
    if (SUCCEEDED(m_threadStartResult)) {
      return m_threadStartResult;
    }
    IFR(m_threadReady.Init());
    if (m_width == 0)
      m_width = 320;
    if (m_height == 0)
      m_height = 200;
    m_pLog = pLog;
    m_pLogCtx = pLogCtx;
    m_thread = CreateThread(nullptr, 0, RendererStart, this, 0, &m_tid);
    if (!m_thread)
      return HRESULT_FROM_WIN32(GetLastError());
    WaitForSingleObject(m_threadReady.m_handle, INFINITE);
    if (FAILED(m_threadStartResult)) {
      WaitForSingleObject(m_thread, INFINITE);
      CloseHandle(m_thread);
    }
    return m_threadStartResult;
  }
  void Run() {
    LPCSTR WindowClassName = "Renderer";
    HINSTANCE procInstance = GetModuleHandle(nullptr);
    {
      auto lock = g_RenderingWindowClassCS.LockCS();
      if (g_RenderingWindowClass == NULL) {
        WNDCLASS wndClass;
        ZeroMemory(&wndClass, sizeof(wndClass));
        wndClass.lpszClassName = WindowClassName;
        wndClass.hInstance = procInstance; // GetModuleHandle("HLSLHost.exe");
        wndClass.style = WS_OVERLAPPED;
        wndClass.cbWndExtra = sizeof(void *);
        wndClass.lpfnWndProc = RendererWndProc;
        ATOM atom = RegisterClass(&wndClass);
        if (atom == INVALID_ATOM) {
          m_threadStartResult = HRESULT_FROM_WIN32(GetLastError());
          m_threadReady.SetEvent();
          return;
        }
      }
    }
    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT windowRect = {0, 0, (LONG)m_width, (LONG)m_height};
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
    LPVOID lParam = this;
    m_hwnd = CreateWindow(WindowClassName, "Renderer", style, CW_USEDEFAULT,
                          CW_USEDEFAULT, windowRect.right - windowRect.left,
                          windowRect.bottom - windowRect.top, NULL, NULL,
                          procInstance, lParam);
    if (m_hwnd == NULL) {
      m_threadStartResult = HRESULT_FROM_WIN32(GetLastError());
      m_threadReady.SetEvent();
      return;
    }
    LONG_PTR l = (LONG_PTR)(Renderer *)this;
    SetWindowLongPtr(m_hwnd, 0, l);
    ShowWindow(m_hwnd, SW_NORMAL);

    m_threadStartResult = S_OK;
    m_threadReady.SetEvent();

    // Basic dispatch loop.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT) {
        break;
      }
    }
    if (m_userQuit) {
      g_ShutdownServerEvent.SetEvent();
    }
  }
  void SetPayload(LPCSTR pText) {
    LPSTR textCopy = strdup(pText);
    LPSTR oldText =
        (LPSTR)InterlockedExchangePointer(&m_ShaderOpText, textCopy);
    if (oldText != nullptr)
      free(oldText);
    PostMessage(m_hwnd, WM_RENDERER_SETPAYLOAD, 0, 0);
  }
  HRESULT SetSize(DWORD width, DWORD height) {
    RECT windowRect;
    GetWindowRect(m_hwnd, &windowRect);
    RECT client = {0, 0, (LONG)width, (LONG)height};
    AdjustWindowRect(&client, WS_OVERLAPPEDWINDOW, FALSE);
    SetWindowPos(m_hwnd, 0, windowRect.left, windowRect.top,
                 client.right - client.left, client.bottom - client.top,
                 SWP_NOZORDER);
    return S_OK;
  }
  HRESULT SetParentHwnd(HWND handle) {
    HWND prior = SetParent(m_hwnd, handle);
    if (prior == NULL) {
      return HRESULT_FROM_WIN32(GetLastError());
    }
    if (handle == NULL) {
      // Top-level, so restore original style.
      SetWindowLong(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    } else {
      // Child window, so set new style and reset position.
      SetWindowPos(m_hwnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
      SetWindowLong(m_hwnd, GWL_STYLE, WS_CHILD | WS_VISIBLE);
    }
    return S_OK;
  }
  void Stop() {
    if (m_hwnd != NULL) {
      PostMessage(m_hwnd, WM_RENDERER_QUIT, 0, 0);
      WaitForSingleObject(m_thread, INFINITE);
      CloseHandle(m_thread);
      m_threadStartResult = E_PENDING;
      m_threadReady.ResetEvent();
      m_thread = NULL;
      m_hwnd = NULL;
    }
  }
  LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SHOWWINDOW:
      if (m_device == nullptr) {
        SetupD3D();
      }
      break;
    case WM_SIZE:
      if (m_device) {
        RECT r;
        GetClientRect(hWnd, &r);
        m_width = r.right - r.left;
        m_height = r.bottom - r.top;
        HRESULT hr = m_swapChain->ResizeBuffers(FrameCount, 0, 0,
                                                DXGI_FORMAT_UNKNOWN, 0);
        Log(SUCCEEDED(hr) ? L"Swapchain buffers resized."
                          : L"Failed to resize swapchain buffers.");
      }
      break;
    case WM_KEYDOWN:
      if (wParam == 'Q') {
        m_userQuit = true;
        DestroyWindow(hWnd);
      }
      if (wParam == 'R') {
        HandlePayload();
      }
      if (wParam == 'C') {
        HandleCopy();
      }
      if (wParam == 'H') {
        HandleHelp();
      }
      if (wParam == 'D') {
        HandleDeviceCycle();
      }
      if (wParam == '2') {
        DXGI_MODE_DESC d;
        ZeroMemory(&d, sizeof(d));
        d.Height = 256;
        d.Width = 256;
        m_swapChain->ResizeTarget(&d);
      }
      break;
    case WM_DESTROY:
      ReleaseD3DResources();
      PostQuitMessage(0);
      break;
    case WM_RENDERER_SETPAYLOAD:
      HandlePayload();
      break;
    case WM_RENDERER_QUIT:
      DestroyWindow(hWnd);
      break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
  }
  void Log(const wchar_t *pLog) {
    if (m_pLog)
      m_pLog(m_pLogCtx, pLog);
  }
};

void __stdcall RendererLog(void *pCtx, const wchar_t *pText) {
  ((Renderer *)pCtx)->Log(pText);
}

LRESULT CALLBACK RendererWndProc(HWND hWnd, UINT msg, WPARAM wParam,
                                 LPARAM lParam) {
  if (msg == WM_CREATE) {
    return DefWindowProc(hWnd, msg, wParam, lParam);
  }
  LONG_PTR l = GetWindowLongPtr(hWnd, 0);
  return ((Renderer *)l)->HandleMessage(hWnd, msg, wParam, lParam);
}

DWORD WINAPI RendererStart(LPVOID lpThreadParameter) {
  Renderer *R = (Renderer *)lpThreadParameter;
  R->Run();
  return 0;
}

// Server object to handle messaging.
void __stdcall ServerObjLog(void *pCtx, const wchar_t *pText);
class ServerObj : public IStream {
private:
  DXC_MICROCOM_REF_FIELD(m_dwRef);
  Renderer m_renderer;
  HhCriticalSection m_cs;
  CAtlArray<wchar_t> m_pMessages;
  CAtlArray<BYTE> m_pLog;
  DWORD m_pLogStart = 0;

public:
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  ServerObj() : m_dwRef(0) {}
  ~ServerObj() {}
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterfaceWithRemote<IStream>(this, iid, ppvObject);
  }

  HRESULT AppendLog(LPCWSTR pLog) {
    size_t logCount = wcslen(pLog);
    HRESULT hr = S_OK;
    auto l = m_cs.LockCS();
    size_t count = m_pMessages.GetCount();
    if (!m_pMessages.SetCount(count + logCount))
      hr = E_OUTOFMEMORY;
    else
      memcpy(m_pMessages.GetData() + count, pLog, logCount * sizeof(wchar_t));
    return hr;
  }

  // Write a message back to the server user.
  HRESULT WriteMessage(const HhMessageHeader *pHeader) {
    HRESULT hr = S_OK;
    auto l = m_cs.LockCS();
    size_t count = m_pLog.GetCount();
    if (!m_pLog.SetCount(count + pHeader->Length))
      hr = E_OUTOFMEMORY;
    else
      memcpy(m_pLog.GetData() + count, pHeader, pHeader->Length);
    return hr;
  }

  void WriteRequestResultReply(UINT requestKind, HRESULT hr) {
    HhResultReply result;
    result.Header.Kind = requestKind + 100;
    result.Header.Length = sizeof(HhResultReply);
    result.hr = hr;
    WriteMessage(&result.Header);
  }

  void HandleMessage(const HhMessageHeader *pHeader, ULONG cb) {
    DWORD MsgKind = pHeader->Kind;
    switch (MsgKind) {
    case GetPidMsgId:
      HhTrace(L"GetPID message received");
      HhGetPidReply R;
      R.Header.Kind = GetPidMsgReplyId;
      R.Header.Length = sizeof(HhGetPidReply);
      R.Pid = GetCurrentProcessId();
      WriteMessage(&R.Header);
      break;
    case ShutdownMsgId:
      HhTrace(L"Shutdown message received");
      m_renderer.Stop();
      g_ShutdownServerEvent.SetEvent();
      break;
    case StartRendererMsgId:
      HhTrace(L"StartRenderer message received");
      WriteRequestResultReply(MsgKind, m_renderer.Start(this, ServerObjLog));
      break;
    case StopRendererMsgId:
      HhTrace(L"StopRenderer message received");
      m_renderer.Stop();
      WriteRequestResultReply(MsgKind, S_OK);
      break;
    case SetPayloadMsgId:
      LPCSTR pText;
      HhTrace(L"SetPayload message received");
      pText = (LPCSTR)(pHeader + 1);
      m_renderer.SetPayload(pText);
      WriteRequestResultReply(MsgKind, S_OK);
      break;
    case ReadLogMsgId: {
      // Do a single grow and write.
      HRESULT hr = S_OK;
      HhMessageHeader H;
      DWORD messageLen;
      DWORD messageLenInBytes;
      wchar_t nullTerm = L'\0';
      auto l = m_cs.LockCS();
      messageLen = (DWORD)m_pMessages.GetCount();
      messageLenInBytes = messageLen * sizeof(wchar_t);
      H.Length = sizeof(HhMessageHeader) + sizeof(messageLen) +
                 messageLenInBytes + sizeof(nullTerm);
      H.Kind = ReadLogMsgReplyId;
      size_t count = m_pLog.GetCount();
      size_t growBy = H.Length;
      if (!m_pLog.SetCount(count + growBy))
        hr = E_OUTOFMEMORY;
      else {
        LPBYTE pCursor = m_pLog.GetData() + count;
        memcpy(pCursor, &H, sizeof(H));
        pCursor += sizeof(H);
        memcpy(pCursor, &messageLen, sizeof(messageLen));
        pCursor += sizeof(messageLen);
        memcpy(pCursor, m_pMessages.GetData(), messageLenInBytes);
        pCursor += messageLenInBytes;
        memcpy(pCursor, &nullTerm, sizeof(nullTerm));
        m_pMessages.SetCount(0);
      }
      break;
    }
    case SetSizeMsgId: {
      if (cb < sizeof(HhSetSizeMessage)) {
        WriteRequestResultReply(MsgKind, E_INVALIDARG);
        return;
      }
      const HhSetSizeMessage *pSetSize = (const HhSetSizeMessage *)pHeader;
      WriteRequestResultReply(
          MsgKind, m_renderer.SetSize(pSetSize->Width, pSetSize->Height));
    }
      LLVM_FALLTHROUGH;
    case SetParentWndMsgId: {
      if (cb < sizeof(HhSetParentWndMessage)) {
        WriteRequestResultReply(MsgKind, E_INVALIDARG);
        return;
      }
      const HhSetParentWndMessage *pSetParent =
          (const HhSetParentWndMessage *)pHeader;
      WriteRequestResultReply(
          MsgKind, m_renderer.SetParentHwnd((HWND)pSetParent->Handle));
    }
    }
  }

  // ISequentialStream implementation.
  HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead) override {
    if (!pv)
      return E_POINTER;
    if (cb == 0)
      return S_OK;
    HRESULT hr = S_OK;
    auto l = m_cs.LockCS();
    size_t count = m_pLog.GetCount();
    size_t countLeft = count - m_pLogStart;
    if (cb > countLeft) {
      cb = countLeft;
      hr = S_FALSE;
    }
    if (pcbRead)
      *pcbRead = cb;
    memcpy(pv, m_pLog.GetData() + m_pLogStart, cb);
    m_pLogStart += cb;
    // If we have more than 2K of wasted space, shrink.
    if (m_pLogStart > 2048) {
      size_t newSize = count - m_pLogStart;
      memmove(m_pLog.GetData(), m_pLog.GetData() + m_pLogStart, newSize);
      m_pLog.SetCount(newSize);
      m_pLogStart = 0;
    }
    return hr;
  }

  HRESULT STDMETHODCALLTYPE Write(void const *pv, ULONG cb,
                                  ULONG *pcbWritten) override {
    if (!pv || !pcbWritten)
      return E_POINTER;
    if (cb == 0)
      return S_OK;
    if (cb < sizeof(HhMessageHeader)) {
      HhTrace(L"Message is smaller than sizeof(HhMessageHeader).");
      return E_FAIL;
    }
    HandleMessage((const HhMessageHeader *)pv, cb);
    *pcbWritten = cb;
    return S_OK;
  }

  // IStream implementation.
  HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER val) override {
    HhTrace(L"SetSize called - E_NOTIMPL");
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *,
                                   ULARGE_INTEGER *) override {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE Commit(DWORD) override { return E_NOTIMPL; }

  HRESULT STDMETHODCALLTYPE Revert(void) override { return E_NOTIMPL; }

  HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER,
                                       DWORD) override {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER,
                                         DWORD) override {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE Clone(IStream **) override { return E_NOTIMPL; }

  HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER, DWORD,
                                 ULARGE_INTEGER *) override {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE Stat(STATSTG *, DWORD) override {
    HhTrace(L"Stat called - E_NOTIMPL");
    return E_NOTIMPL;
  }
};
void __stdcall ServerObjLog(void *pCtx, const wchar_t *pText) {
  ((ServerObj *)pCtx)->AppendLog(pText);
}

// Server startup and lifetime.
class ServerFactory : public IClassFactory {
private:
  DXC_MICROCOM_REF_FIELD(m_dwRef);

public:
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  ServerFactory() : m_dwRef(0) {}
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterfaceWithRemote<IClassFactory>(this, iid, ppvObject);
  }
  HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown *pUnk, REFIID riid,
                                           void **ppvObj) override {
    if (pUnk)
      return CLASS_E_NOAGGREGATION;
    CComPtr<ServerObj> obj = new (std::nothrow) ServerObj();
    if (obj.p == nullptr)
      return E_OUTOFMEMORY;
    return obj.p->QueryInterface(riid, ppvObj);
  }
  HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock) override {
    // TODO: implement
    return S_OK;
  }
};

HRESULT RunServer(REFCLSID rclsid) {
  HhTrace(L"Starting HLSL Host...");
  CoInit coInit;
  ClassObjectRegistration registration;
  IFR(coInit.Initialize(COINIT_MULTITHREADED));
  IFR(g_ShutdownServerEvent.Init());
  CComPtr<ServerFactory> pServerFactory = new (std::nothrow) ServerFactory();
  IFR(registration.Register(rclsid, pServerFactory, CLSCTX_LOCAL_SERVER,
                            REGCLS_MULTIPLEUSE));
  WaitForSingleObject(g_ShutdownServerEvent.m_handle, INFINITE);
  return S_OK;
}

HRESULT RunServer(const wchar_t *pCLSIDText) {
  CLSID clsid;
  if (pCLSIDText && *pCLSIDText) {
    IFR(CLSIDFromString(pCLSIDText, &clsid));
  } else {
    clsid = CLSID_HLSLHostServer;
  }
  return RunServer(clsid);
}

// Entry point for host process to render shaders.
int wmain(int argc, wchar_t *argv[]) {
  int resultCode;
  resultCode = SUCCEEDED(RunServer(nullptr)) ? 0 : 1;
  return resultCode;
}
