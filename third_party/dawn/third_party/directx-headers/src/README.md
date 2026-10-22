# DirectX Headers

This repository hosts the official Direct3D 12 headers. These headers are made available under the MIT license rather than the traditional Windows SDK license.

Additionally, this repository hosts several helpers for using these headers.

Make sure that you visit the [DirectX Landing Page](https://devblogs.microsoft.com/directx/landing-page/) for more resources for DirectX developers.

## Directory Structure

* `/`: Build files are available here for quick integration. CMake is provided, and can be referenced either via `subdirectory()` or after installation to a system location. Meson is also available for inclusion as a subproject/wrap.
* `/include/directx`: These files are the core headers for using D3D12, plus d3dx12.h, which is a helper and does not cross the boundaries of the D3D12 API.
* `/include/wsl`: These files are provided as a shim to be able to include the D3D12 headers from a Linux build environment, without requiring the rest of the Windows SDK.
* `/include/dxguids`: This header allows an application to use `uuidof<T>()` consistently between Windows and WSL, instead of `__uuidof()`.
* `/src/dxguids.cpp`: This cpp file can be used as a replacement for linking against `dxguid.lib` on Windows, and as a convenient translation unit to define GUIDs without multiple definitions for WSL.
* `/test`: Simple CMake/Meson projects for validating the headers can be included in a given environment

## Use on Windows

Note that these headers may conflict with the headers from the Windows SDK, depending on include ordering. These headers should be added to the include directory list before the SDK, and should be included before other graphics headers (e.g. `d3d11.h`) from the Windows SDK. Otherwise, the corresponding header from the Windows SDK may be included first, and will define the include guards which prevents these headers from being used.

## Use on WSL

Note: WSL support is not intended for general purpose application development. At this time, the only recommended usage is for frameworks wishing to provide hardware acceleration for a Linux graphics/compute API in a WSL2 virtualization environment.

Note: WSL support is only available for 64-bit binaries.

The headers in the `/include/wsl` directory provide alternative definitions to macros and typedefs normally found in the Windows SDK. For the most part, they should be straightforward, but there are a couple to call attention to:

|Type|Reason|
|---|---|
|`LONG`/`ULONG`|On 64-bit Windows, a `long` is 4 bytes, but on Linux it is typically 8 bytes. The D3D12 ABI for WSL uses `long` and therefore these should be 8 bytes.|
|`WCHAR`/`WCSTR`|On Windows, a `wchar_t` is 2 bytes, but on Linux it is typically 4 bytes. The D3D12 ABI for WSL uses the native 4-byte `wchar_t`, to enable applications and the runtime to use the system C library to perform string manipulation.|

Additionally, APIs taking `HANDLE` (`void*`) for Win32 types should instead use `reinterpret_cast<HANDLE>(fd)` for an appropriate type of file descriptor. For `ID3D12Fence::SetEventOnCompletion` this should be an `eventfd`, and for shared resources will be an opaque fd.

## Ways to consume

There are various ways to consume the headers in this project:

* Manually: Just copy the headers somewhere and point your project at them.
* CMake subproject: Add this entire project as a subdirectory of your larger project, e.g. as a git submodule, and `add_subdirectory` into it. Use the resulting `DirectX-Headers` and/or `DirectX-Guids` targets as a link dependency
* Installed CMake: After building/installing this project, it can be found through CMake's `find_package` functionality and will expose the same `DirectX-Headers` and `DirectX-Guids` targets.
* FetchContent CMake (3.11+): Fetch this library using Git and easily add it to your project.
* Meson subproject/wrap: Add this entire project as a subproject of your larger project, and use `subproject` or `dependency` to consume it.
* Pkg-config: Use Meson to build this project, and the resulting installed package can be found via pkg-config.
* vcpkg: A vcpkg port has [been added](https://github.com/microsoft/vcpkg/tree/master/ports/directx-headers).
* NuGet: Download the [DirectX 12 Agility SDK](https://devblogs.microsoft.com/directx/announcing-dx12agility/) from [NuGet.org](https://www.nuget.org/packages/Microsoft.Direct3D.D3D12)
  * For more info about the Agility SDK, see the [getting started guide](https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/)

Contributions for new mechanisms are welcome.

## Contributing

This project welcomes contributions and suggestions. Most contributions require you to agree to a Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft trademarks or logos is subject to and must follow [Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general). Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship. Any use of third-party trademarks or logos are subject to those third-party's policies.
