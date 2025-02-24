/*
 * Copyright (c) 2021 The Khronos Group Inc.
 * Copyright (c) 2021 Valve Corporation
 * Copyright (c) 2021 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

// This needs to be defined first, or else we'll get redefinitions on NTSTATUS values
#if defined(_WIN32)
#define UMDF_USING_NTSTATUS
#include <ntstatus.h>
#endif

#include <windows.h>
#include <debugapi.h>

#include "shim.h"

#include "detours.h"

static PlatformShim platform_shim;

extern "C" {

static LibraryWrapper gdi32_dll;

using PFN_GetSidSubAuthority = PDWORD(__stdcall *)(PSID pSid, DWORD nSubAuthority);
static PFN_GetSidSubAuthority fpGetSidSubAuthority = GetSidSubAuthority;

PDWORD __stdcall ShimGetSidSubAuthority(PSID, DWORD) { return &platform_shim.elevation_level; }

static PFN_LoaderEnumAdapters2 fpEnumAdapters2 = nullptr;
static PFN_LoaderQueryAdapterInfo fpQueryAdapterInfo = nullptr;

NTSTATUS APIENTRY ShimEnumAdapters2(LoaderEnumAdapters2 *adapters) {
    if (adapters == nullptr) {
        return STATUS_INVALID_PARAMETER;
    }
    if (platform_shim.d3dkmt_adapters.size() == 0) {
        if (adapters->adapters != nullptr) adapters->adapter_count = 0;
        return STATUS_SUCCESS;
    }
    if (adapters->adapters != nullptr) {
        for (size_t i = 0; i < platform_shim.d3dkmt_adapters.size(); i++) {
            adapters->adapters[i].handle = platform_shim.d3dkmt_adapters[i].hAdapter;
            adapters->adapters[i].luid = platform_shim.d3dkmt_adapters[i].adapter_luid;
        }
        adapters->adapter_count = static_cast<ULONG>(platform_shim.d3dkmt_adapters.size());
    } else {
        adapters->adapter_count = static_cast<ULONG>(platform_shim.d3dkmt_adapters.size());
    }
    return STATUS_SUCCESS;
}
NTSTATUS APIENTRY ShimQueryAdapterInfo(const LoaderQueryAdapterInfo *query_info) {
    if (query_info == nullptr || query_info->private_data == nullptr) {
        return STATUS_INVALID_PARAMETER;
    }
    auto handle = query_info->handle;
    auto it = std::find_if(platform_shim.d3dkmt_adapters.begin(), platform_shim.d3dkmt_adapters.end(),
                           [handle](D3DKMT_Adapter const &adapter) { return handle == adapter.hAdapter; });
    if (it == platform_shim.d3dkmt_adapters.end()) {
        return STATUS_INVALID_PARAMETER;
    }
    auto &adapter = *it;
    auto *reg_info = reinterpret_cast<LoaderQueryRegistryInfo *>(query_info->private_data);

    std::vector<std::wstring> *paths = nullptr;
    if (wcsstr(reg_info->value_name, L"DriverName") != nullptr) {  // looking for drivers
        paths = &adapter.driver_paths;
    } else if (wcsstr(reg_info->value_name, L"ImplicitLayers") != nullptr) {  // looking for implicit layers
        paths = &adapter.implicit_layer_paths;
    } else if (wcsstr(reg_info->value_name, L"ExplicitLayers") != nullptr) {  // looking for explicit layers
        paths = &adapter.explicit_layer_paths;
    }

    reg_info->status = LOADER_QUERY_REGISTRY_STATUS_SUCCESS;
    if (reg_info->output_value_size == 0) {
        // final null terminator size
        ULONG size = 2;

        // size is in bytes, so multiply path size + 1 (for null terminator) by size of wchar (basically, 2).
        for (auto const &path : *paths) size += static_cast<ULONG>((path.length() + 1) * sizeof(wchar_t));
        reg_info->output_value_size = size;
        if (size != 2) {
            // only want to write data if there is path data to write
            reg_info->status = LOADER_QUERY_REGISTRY_STATUS_BUFFER_OVERFLOW;
        }
    } else if (reg_info->output_value_size > 2) {
        size_t index = 0;
        for (auto const &path : *paths) {
            for (auto w : path) {
                reg_info->output_string[index++] = w;
            }
            reg_info->output_string[index++] = L'\0';
        }
        // make sure there is a null terminator
        reg_info->output_string[index++] = L'\0';

        reg_info->status = LOADER_QUERY_REGISTRY_STATUS_SUCCESS;
    }

    return STATUS_SUCCESS;
}

// clang-format off
static CONFIGRET(WINAPI *REAL_CM_Get_Device_ID_List_SizeW)(PULONG pulLen, PCWSTR pszFilter, ULONG ulFlags) = CM_Get_Device_ID_List_SizeW;
static CONFIGRET(WINAPI *REAL_CM_Get_Device_ID_ListW)(PCWSTR pszFilter, PZZWSTR Buffer, ULONG BufferLen, ULONG ulFlags) = CM_Get_Device_ID_ListW;
static CONFIGRET(WINAPI *REAL_CM_Locate_DevNodeW)(PDEVINST pdnDevInst, DEVINSTID_W pDeviceID, ULONG ulFlags) =  CM_Locate_DevNodeW;
static CONFIGRET(WINAPI *REAL_CM_Get_DevNode_Status)(PULONG pulStatus, PULONG pulProblemNumber, DEVINST dnDevInst, ULONG ulFlags) =  CM_Get_DevNode_Status;
static CONFIGRET(WINAPI *REAL_CM_Get_Device_IDW)(DEVINST dnDevInst, PWSTR Buffer, ULONG BufferLen, ULONG ulFlags) =  CM_Get_Device_IDW;
static CONFIGRET(WINAPI *REAL_CM_Get_Child)(PDEVINST pdnDevInst, DEVINST dnDevInst, ULONG ulFlags) =  CM_Get_Child;
static CONFIGRET(WINAPI *REAL_CM_Get_DevNode_Registry_PropertyW)(DEVINST dnDevInst, ULONG ulProperty, PULONG pulRegDataType, PVOID Buffer, PULONG pulLength, ULONG ulFlags) =  CM_Get_DevNode_Registry_PropertyW;
static CONFIGRET(WINAPI *REAL_CM_Get_Sibling)(PDEVINST pdnDevInst, DEVINST dnDevInst, ULONG ulFlags) = CM_Get_Sibling;
// clang-format on

CONFIGRET WINAPI SHIM_CM_Get_Device_ID_List_SizeW(PULONG pulLen, [[maybe_unused]] PCWSTR pszFilter,
                                                  [[maybe_unused]] ULONG ulFlags) {
    if (pulLen == nullptr) {
        return CR_INVALID_POINTER;
    }
    *pulLen = static_cast<ULONG>(platform_shim.CM_device_ID_list.size());
    return CR_SUCCESS;
}
CONFIGRET WINAPI SHIM_CM_Get_Device_ID_ListW([[maybe_unused]] PCWSTR pszFilter, PZZWSTR Buffer, ULONG BufferLen,
                                             [[maybe_unused]] ULONG ulFlags) {
    if (Buffer != NULL) {
        if (BufferLen < platform_shim.CM_device_ID_list.size()) return CR_BUFFER_SMALL;
        for (size_t i = 0; i < BufferLen; i++) {
            Buffer[i] = platform_shim.CM_device_ID_list[i];
        }
    }
    return CR_SUCCESS;
}
// TODO
CONFIGRET WINAPI SHIM_CM_Locate_DevNodeW(PDEVINST, DEVINSTID_W, ULONG) { return CR_FAILURE; }
// TODO
CONFIGRET WINAPI SHIM_CM_Get_DevNode_Status(PULONG, PULONG, DEVINST, ULONG) { return CR_FAILURE; }
// TODO
CONFIGRET WINAPI SHIM_CM_Get_Device_IDW(DEVINST, PWSTR, ULONG, ULONG) { return CR_FAILURE; }
// TODO
CONFIGRET WINAPI SHIM_CM_Get_Child(PDEVINST, DEVINST, ULONG) { return CR_FAILURE; }
// TODO
CONFIGRET WINAPI SHIM_CM_Get_DevNode_Registry_PropertyW(DEVINST, ULONG, PULONG, PVOID, PULONG, ULONG) { return CR_FAILURE; }
// TODO
CONFIGRET WINAPI SHIM_CM_Get_Sibling(PDEVINST, DEVINST, ULONG) { return CR_FAILURE; }

static LibraryWrapper dxgi_module;
typedef HRESULT(APIENTRY *PFN_CreateDXGIFactory1)(REFIID riid, void **ppFactory);

PFN_CreateDXGIFactory1 RealCreateDXGIFactory1;

HRESULT __stdcall ShimGetDesc1(IDXGIAdapter1 *pAdapter,
                               /* [annotation][out] */
                               _Out_ DXGI_ADAPTER_DESC1 *pDesc) {
    if (pAdapter == nullptr || pDesc == nullptr) return DXGI_ERROR_INVALID_CALL;

    for (const auto &[index, adapter] : platform_shim.dxgi_adapters) {
        if (&adapter.adapter_instance == pAdapter) {
            *pDesc = adapter.desc1;
            return S_OK;
        }
    }
    return DXGI_ERROR_INVALID_CALL;
}
ULONG __stdcall ShimIDXGIFactory1Release(IDXGIFactory1 *) { return S_OK; }
ULONG __stdcall ShimIDXGIFactory6Release(IDXGIFactory6 *) { return S_OK; }
ULONG __stdcall ShimRelease(IDXGIAdapter1 *) { return S_OK; }

IDXGIAdapter1 *setup_and_get_IDXGIAdapter1(DXGIAdapter &adapter) {
    adapter.adapter_vtbl_instance.GetDesc1 = ShimGetDesc1;
    adapter.adapter_vtbl_instance.Release = ShimRelease;
    adapter.adapter_instance.lpVtbl = &adapter.adapter_vtbl_instance;
    return &adapter.adapter_instance;
}

HRESULT __stdcall ShimEnumAdapters1_1([[maybe_unused]] IDXGIFactory1 *This,
                                      /* [in] */ UINT Adapter,
                                      /* [annotation][out] */
                                      _COM_Outptr_ IDXGIAdapter1 **ppAdapter) {
    if (Adapter >= platform_shim.dxgi_adapters.size()) {
        return DXGI_ERROR_INVALID_CALL;
    }
    if (ppAdapter != nullptr) {
        *ppAdapter = setup_and_get_IDXGIAdapter1(platform_shim.dxgi_adapters.at(Adapter));
    }
    return S_OK;
}

HRESULT __stdcall ShimEnumAdapters1_6([[maybe_unused]] IDXGIFactory6 *This,
                                      /* [in] */ UINT Adapter,
                                      /* [annotation][out] */
                                      _COM_Outptr_ IDXGIAdapter1 **ppAdapter) {
    if (Adapter >= platform_shim.dxgi_adapters.size()) {
        return DXGI_ERROR_INVALID_CALL;
    }
    if (ppAdapter != nullptr) {
        *ppAdapter = setup_and_get_IDXGIAdapter1(platform_shim.dxgi_adapters.at(Adapter));
    }
    return S_OK;
}

HRESULT __stdcall ShimEnumAdapterByGpuPreference([[maybe_unused]] IDXGIFactory6 *This, _In_ UINT Adapter,
                                                 [[maybe_unused]] _In_ DXGI_GPU_PREFERENCE GpuPreference,
                                                 [[maybe_unused]] _In_ REFIID riid, _COM_Outptr_ void **ppvAdapter) {
    if (Adapter >= platform_shim.dxgi_adapters.size()) {
        return DXGI_ERROR_NOT_FOUND;
    }
    // loader always uses DXGI_GPU_PREFERENCE_UNSPECIFIED
    // Update the shim if this isn't the case
    assert(GpuPreference == DXGI_GPU_PREFERENCE::DXGI_GPU_PREFERENCE_UNSPECIFIED &&
           "Test shim assumes the GpuPreference is unspecified.");
    if (ppvAdapter != nullptr) {
        *ppvAdapter = setup_and_get_IDXGIAdapter1(platform_shim.dxgi_adapters.at(Adapter));
    }
    return S_OK;
}

static IDXGIFactory1 *get_IDXGIFactory1() {
    static IDXGIFactory1Vtbl vtbl{};
    vtbl.EnumAdapters1 = ShimEnumAdapters1_1;
    vtbl.Release = ShimIDXGIFactory1Release;
    static IDXGIFactory1 factory{};
    factory.lpVtbl = &vtbl;
    return &factory;
}

static IDXGIFactory6 *get_IDXGIFactory6() {
    static IDXGIFactory6Vtbl vtbl{};
    vtbl.EnumAdapters1 = ShimEnumAdapters1_6;
    vtbl.EnumAdapterByGpuPreference = ShimEnumAdapterByGpuPreference;
    vtbl.Release = ShimIDXGIFactory6Release;
    static IDXGIFactory6 factory{};
    factory.lpVtbl = &vtbl;
    return &factory;
}

HRESULT __stdcall ShimCreateDXGIFactory1(REFIID riid, void **ppFactory) {
    if (riid == IID_IDXGIFactory1) {
        auto *factory = get_IDXGIFactory1();
        *ppFactory = factory;
        return S_OK;
    }
    if (riid == IID_IDXGIFactory6) {
        auto *factory = get_IDXGIFactory6();
        *ppFactory = factory;
        return S_OK;
    }
    assert(false && "new riid, update shim code to handle");
    return S_FALSE;
}

// Windows Registry shims
using PFN_RegOpenKeyExA = LSTATUS(__stdcall *)(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
static PFN_RegOpenKeyExA fpRegOpenKeyExA = RegOpenKeyExA;
using PFN_RegQueryValueExA = LSTATUS(__stdcall *)(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData,
                                                  LPDWORD lpcbData);
static PFN_RegQueryValueExA fpRegQueryValueExA = RegQueryValueExA;
using PFN_RegEnumValueA = LSTATUS(__stdcall *)(HKEY hKey, DWORD dwIndex, LPSTR lpValueName, LPDWORD lpcchValueName,
                                               LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
static PFN_RegEnumValueA fpRegEnumValueA = RegEnumValueA;

using PFN_RegCloseKey = LSTATUS(__stdcall *)(HKEY hKey);
static PFN_RegCloseKey fpRegCloseKey = RegCloseKey;

LSTATUS __stdcall ShimRegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, [[maybe_unused]] DWORD ulOptions,
                                    [[maybe_unused]] REGSAM samDesired, PHKEY phkResult) {
    if (HKEY_LOCAL_MACHINE != hKey && HKEY_CURRENT_USER != hKey) return ERROR_BADKEY;
    std::string hive = "";
    if (HKEY_LOCAL_MACHINE == hKey)
        hive = "HKEY_LOCAL_MACHINE";
    else if (HKEY_CURRENT_USER == hKey)
        hive = "HKEY_CURRENT_USER";
    if (hive == "") return ERROR_ACCESS_DENIED;

    platform_shim.created_keys.emplace_back(platform_shim.created_key_count++, hive + "\\" + lpSubKey);
    *phkResult = platform_shim.created_keys.back().get();
    return 0;
}
const std::string *get_path_of_created_key(HKEY hKey) {
    for (const auto &key : platform_shim.created_keys) {
        if (key.key == hKey) {
            return &key.path;
        }
    }
    return nullptr;
}
std::vector<RegistryEntry> *get_registry_vector(std::string const &path) {
    if (path == "HKEY_LOCAL_MACHINE\\" VK_DRIVERS_INFO_REGISTRY_LOC) return &platform_shim.hkey_local_machine_drivers;
    if (path == "HKEY_LOCAL_MACHINE\\" VK_ELAYERS_INFO_REGISTRY_LOC) return &platform_shim.hkey_local_machine_explicit_layers;
    if (path == "HKEY_LOCAL_MACHINE\\" VK_ILAYERS_INFO_REGISTRY_LOC) return &platform_shim.hkey_local_machine_implicit_layers;
    if (path == "HKEY_CURRENT_USER\\" VK_ELAYERS_INFO_REGISTRY_LOC) return &platform_shim.hkey_current_user_explicit_layers;
    if (path == "HKEY_CURRENT_USER\\" VK_ILAYERS_INFO_REGISTRY_LOC) return &platform_shim.hkey_current_user_implicit_layers;
    if (path == "HKEY_LOCAL_MACHINE\\" VK_SETTINGS_INFO_REGISTRY_LOC) return &platform_shim.hkey_local_machine_settings;
    if (path == "HKEY_CURRENT_USER\\" VK_SETTINGS_INFO_REGISTRY_LOC) return &platform_shim.hkey_current_user_settings;
    return nullptr;
}
LSTATUS __stdcall ShimRegQueryValueExA(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD) {
    // TODO:
    return ERROR_SUCCESS;
}
LSTATUS __stdcall ShimRegEnumValueA(HKEY hKey, DWORD dwIndex, LPSTR lpValueName, LPDWORD lpcchValueName,
                                    [[maybe_unused]] LPDWORD lpReserved, [[maybe_unused]] LPDWORD lpType, LPBYTE lpData,
                                    LPDWORD lpcbData) {
    const std::string *path = get_path_of_created_key(hKey);
    if (path == nullptr) return ERROR_NO_MORE_ITEMS;

    const auto *location_ptr = get_registry_vector(*path);
    if (location_ptr == nullptr) return ERROR_NO_MORE_ITEMS;
    const auto &location = *location_ptr;
    if (dwIndex >= location.size()) return ERROR_NO_MORE_ITEMS;

    std::string name = narrow(location[dwIndex].name);
    if (*lpcchValueName < name.size()) return ERROR_NO_MORE_ITEMS;
    for (size_t i = 0; i < name.size(); i++) {
        lpValueName[i] = name[i];
    }
    lpValueName[name.size()] = '\0';
    *lpcchValueName = static_cast<DWORD>(name.size() + 1);
    if (*lpcbData < sizeof(DWORD)) return ERROR_NO_MORE_ITEMS;
    DWORD *lpcbData_dword = reinterpret_cast<DWORD *>(lpData);
    *lpcbData_dword = location[dwIndex].value;
    *lpcbData = sizeof(DWORD);
    return ERROR_SUCCESS;
}
LSTATUS __stdcall ShimRegCloseKey(HKEY hKey) {
    for (size_t i = 0; i < platform_shim.created_keys.size(); i++) {
        if (platform_shim.created_keys[i].get() == hKey) {
            platform_shim.created_keys.erase(platform_shim.created_keys.begin() + i);
            return ERROR_SUCCESS;
        }
    }
    // means that RegCloseKey was called with an invalid key value (one that doesn't exist or has already been closed)
    exit(-1);
}

// Windows app package shims
using PFN_GetPackagesByPackageFamily = LONG(WINAPI *)(PCWSTR, UINT32 *, PWSTR *, UINT32 *, WCHAR *);
static PFN_GetPackagesByPackageFamily fpGetPackagesByPackageFamily = GetPackagesByPackageFamily;
using PFN_GetPackagePathByFullName = LONG(WINAPI *)(PCWSTR, UINT32 *, PWSTR);
static PFN_GetPackagePathByFullName fpGetPackagePathByFullName = GetPackagePathByFullName;

static constexpr wchar_t package_full_name[] = L"ThisIsARandomStringSinceTheNameDoesn'tMatter";
LONG WINAPI ShimGetPackagesByPackageFamily(_In_ PCWSTR packageFamilyName, _Inout_ UINT32 *count,
                                           _Out_writes_opt_(*count) PWSTR *packageFullNames, _Inout_ UINT32 *bufferLength,
                                           _Out_writes_opt_(*bufferLength) WCHAR *buffer) {
    if (!packageFamilyName || !count || !bufferLength) return ERROR_INVALID_PARAMETER;
    if (!platform_shim.app_package_path.empty() && wcscmp(packageFamilyName, L"Microsoft.D3DMappingLayers_8wekyb3d8bbwe") == 0) {
        if (*count > 0 && !packageFullNames) return ERROR_INVALID_PARAMETER;
        if (*bufferLength > 0 && !buffer) return ERROR_INVALID_PARAMETER;
        if (*count > 1) return ERROR_INVALID_PARAMETER;
        bool too_small = *count < 1 || *bufferLength < ARRAYSIZE(package_full_name);
        *count = 1;
        *bufferLength = ARRAYSIZE(package_full_name);
        if (too_small) return ERROR_INSUFFICIENT_BUFFER;

        for (size_t i = 0; i < sizeof(package_full_name) / sizeof(wchar_t); i++) {
            if (i >= *bufferLength) {
                break;
            }
            buffer[i] = package_full_name[i];
        }
        *packageFullNames = buffer;
        return 0;
    }
    *count = 0;
    *bufferLength = 0;
    return 0;
}

LONG WINAPI ShimGetPackagePathByFullName(_In_ PCWSTR packageFullName, _Inout_ UINT32 *pathLength,
                                         _Out_writes_opt_(*pathLength) PWSTR path) {
    if (!packageFullName || !pathLength) return ERROR_INVALID_PARAMETER;
    if (*pathLength > 0 && !path) return ERROR_INVALID_PARAMETER;
    if (wcscmp(packageFullName, package_full_name) != 0) {
        *pathLength = 0;
        return 0;
    }
    if (*pathLength < platform_shim.app_package_path.size() + 1) {
        *pathLength = static_cast<UINT32>(platform_shim.app_package_path.size() + 1);
        return ERROR_INSUFFICIENT_BUFFER;
    }
    for (size_t i = 0; i < platform_shim.app_package_path.length(); i++) {
        if (i >= *pathLength) {
            break;
        }
        path[i] = platform_shim.app_package_path.c_str()[i];
    }
    return 0;
}

using PFN_OutputDebugStringA = void(__stdcall *)(LPCSTR lpOutputString);
static PFN_OutputDebugStringA fp_OutputDebugStringA = OutputDebugStringA;

void __stdcall intercept_OutputDebugStringA(LPCSTR lpOutputString) {
    if (lpOutputString != nullptr) {
        platform_shim.fputs_stderr_log += lpOutputString;
    }
}

// Initialization
void WINAPI DetourFunctions() {
    if (!gdi32_dll) {
        gdi32_dll = LibraryWrapper("gdi32.dll");
        fpEnumAdapters2 = gdi32_dll.get_symbol("D3DKMTEnumAdapters2");
        if (fpEnumAdapters2 == nullptr) {
            std::cerr << "Failed to load D3DKMTEnumAdapters2\n";
            return;
        }
        fpQueryAdapterInfo = gdi32_dll.get_symbol("D3DKMTQueryAdapterInfo");
        if (fpQueryAdapterInfo == nullptr) {
            std::cerr << "Failed to load D3DKMTQueryAdapterInfo\n";
            return;
        }
    }
    if (!dxgi_module) {
        TCHAR systemPath[MAX_PATH] = "";
        GetSystemDirectory(systemPath, MAX_PATH);
        StringCchCat(systemPath, MAX_PATH, TEXT("\\dxgi.dll"));
        dxgi_module = LibraryWrapper(systemPath);
        RealCreateDXGIFactory1 = dxgi_module.get_symbol("CreateDXGIFactory1");
        if (RealCreateDXGIFactory1 == nullptr) {
            std::cerr << "Failed to load CreateDXGIFactory1\n";
        }
    }

    DetourRestoreAfterWith();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID &)fpGetSidSubAuthority, (PVOID)ShimGetSidSubAuthority);
    DetourAttach(&(PVOID &)fpEnumAdapters2, (PVOID)ShimEnumAdapters2);
    DetourAttach(&(PVOID &)fpQueryAdapterInfo, (PVOID)ShimQueryAdapterInfo);
    DetourAttach(&(PVOID &)REAL_CM_Get_Device_ID_List_SizeW, (PVOID)SHIM_CM_Get_Device_ID_List_SizeW);
    DetourAttach(&(PVOID &)REAL_CM_Get_Device_ID_ListW, (PVOID)SHIM_CM_Get_Device_ID_ListW);
    DetourAttach(&(PVOID &)REAL_CM_Get_Device_ID_ListW, (PVOID)SHIM_CM_Get_Device_ID_ListW);
    DetourAttach(&(PVOID &)REAL_CM_Locate_DevNodeW, (PVOID)SHIM_CM_Locate_DevNodeW);
    DetourAttach(&(PVOID &)REAL_CM_Get_DevNode_Status, (PVOID)SHIM_CM_Get_DevNode_Status);
    DetourAttach(&(PVOID &)REAL_CM_Get_Device_IDW, (PVOID)SHIM_CM_Get_Device_IDW);
    DetourAttach(&(PVOID &)REAL_CM_Get_Child, (PVOID)SHIM_CM_Get_Child);
    DetourAttach(&(PVOID &)REAL_CM_Get_DevNode_Registry_PropertyW, (PVOID)SHIM_CM_Get_DevNode_Registry_PropertyW);
    DetourAttach(&(PVOID &)REAL_CM_Get_Sibling, (PVOID)SHIM_CM_Get_Sibling);
    DetourAttach(&(PVOID &)RealCreateDXGIFactory1, (PVOID)ShimCreateDXGIFactory1);
    DetourAttach(&(PVOID &)fpRegOpenKeyExA, (PVOID)ShimRegOpenKeyExA);
    DetourAttach(&(PVOID &)fpRegQueryValueExA, (PVOID)ShimRegQueryValueExA);
    DetourAttach(&(PVOID &)fpRegEnumValueA, (PVOID)ShimRegEnumValueA);
    DetourAttach(&(PVOID &)fpRegCloseKey, (PVOID)ShimRegCloseKey);
    DetourAttach(&(PVOID &)fpGetPackagesByPackageFamily, (PVOID)ShimGetPackagesByPackageFamily);
    DetourAttach(&(PVOID &)fpGetPackagePathByFullName, (PVOID)ShimGetPackagePathByFullName);
    DetourAttach(&(PVOID &)fp_OutputDebugStringA, (PVOID)intercept_OutputDebugStringA);
    LONG error = DetourTransactionCommit();

    if (error != NO_ERROR) {
        std::cerr << "simple" << DETOURS_STRINGIFY(DETOURS_BITS) << ".dll:"
                  << " Error detouring function(): " << error << "\n";
    }
}

void DetachFunctions() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID &)fpGetSidSubAuthority, (PVOID)ShimGetSidSubAuthority);
    DetourDetach(&(PVOID &)fpEnumAdapters2, (PVOID)ShimEnumAdapters2);
    DetourDetach(&(PVOID &)fpQueryAdapterInfo, (PVOID)ShimQueryAdapterInfo);
    DetourDetach(&(PVOID &)REAL_CM_Get_Device_ID_List_SizeW, (PVOID)SHIM_CM_Get_Device_ID_List_SizeW);
    DetourDetach(&(PVOID &)REAL_CM_Get_Device_ID_ListW, (PVOID)SHIM_CM_Get_Device_ID_ListW);
    DetourDetach(&(PVOID &)REAL_CM_Locate_DevNodeW, (PVOID)SHIM_CM_Locate_DevNodeW);
    DetourDetach(&(PVOID &)REAL_CM_Get_DevNode_Status, (PVOID)SHIM_CM_Get_DevNode_Status);
    DetourDetach(&(PVOID &)REAL_CM_Get_Device_IDW, (PVOID)SHIM_CM_Get_Device_IDW);
    DetourDetach(&(PVOID &)REAL_CM_Get_Child, (PVOID)SHIM_CM_Get_Child);
    DetourDetach(&(PVOID &)REAL_CM_Get_DevNode_Registry_PropertyW, (PVOID)SHIM_CM_Get_DevNode_Registry_PropertyW);
    DetourDetach(&(PVOID &)REAL_CM_Get_Sibling, (PVOID)SHIM_CM_Get_Sibling);
    DetourDetach(&(PVOID &)RealCreateDXGIFactory1, (PVOID)ShimCreateDXGIFactory1);
    DetourDetach(&(PVOID &)fpRegOpenKeyExA, (PVOID)ShimRegOpenKeyExA);
    DetourDetach(&(PVOID &)fpRegQueryValueExA, (PVOID)ShimRegQueryValueExA);
    DetourDetach(&(PVOID &)fpRegEnumValueA, (PVOID)ShimRegEnumValueA);
    DetourDetach(&(PVOID &)fpRegCloseKey, (PVOID)ShimRegCloseKey);
    DetourDetach(&(PVOID &)fpGetPackagesByPackageFamily, (PVOID)ShimGetPackagesByPackageFamily);
    DetourDetach(&(PVOID &)fpGetPackagePathByFullName, (PVOID)ShimGetPackagePathByFullName);
    DetourDetach(&(PVOID &)fp_OutputDebugStringA, (PVOID)intercept_OutputDebugStringA);
    DetourTransactionCommit();
}

BOOL WINAPI DllMain([[maybe_unused]] HINSTANCE hinst, DWORD dwReason, [[maybe_unused]] LPVOID reserved) {
    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {
        DetourFunctions();
    } else if (dwReason == DLL_PROCESS_DETACH) {
        DetachFunctions();
    }
    return TRUE;
}
FRAMEWORK_EXPORT PlatformShim *get_platform_shim(std::vector<fs::FolderManager> *folders) {
    platform_shim = PlatformShim(folders);
    return &platform_shim;
}
}
