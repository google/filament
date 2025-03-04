// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef _WIN32
#include <wsl/winadapter.h>
#endif

#include <directx/d3d12.h>
#include <directx/d3d12video.h>
#include <directx/dxcore.h>
#include <directx/d3dx12.h>
#include "dxguids/dxguids.h"

int main()
{
    IDXCoreAdapter *adapter = nullptr;
    ID3D12Device *device = nullptr;

    {
        IDXCoreAdapterFactory *factory = nullptr;
        if (FAILED(DXCoreCreateAdapterFactory(&factory)))
            return -1;

        IDXCoreAdapterList *list = nullptr;
        if (FAILED(factory->CreateAdapterList(1, &DXCORE_ADAPTER_ATTRIBUTE_D3D12_CORE_COMPUTE, &list)))
            return -1;
        
        if (FAILED(list->GetAdapter(0, &adapter)))
            return -1;
    }

    return D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
}
