# Vulkan Loader

This project provides the Khronos official Vulkan Loader for all platforms except [Android](https://android.googlesource.com/platform/frameworks/native/+/master/vulkan/)

## Introduction

Vulkan is an explicit API, enabling direct control over how GPUs actually work.
As such, Vulkan supports systems that have multiple GPUs, each running with a different driver, or ICD (Installable Client Driver).
Vulkan also supports multiple global contexts (instances, in Vulkan terminology).
The ICD loader is a library that is placed between a Vulkan application and any number of Vulkan drivers, in order to support multiple drivers and the instance-level functionality that works across these drivers.
Additionally, the loader manages inserting Vulkan layer libraries, such as validation layers, between an application and the drivers.

This repository contains the Vulkan loader that is used for Linux, Windows, MacOS, and iOS.
There is also a separate loader, maintained by Google, which is used on Android.

The following components are available in this repository:

- [ICD Loader](loader/)
- [Loader Documentation](docs/LoaderInterfaceArchitecture.md)
- [Tests](tests/)

## Contact Information

- [Charles Giessen](mailto:charles@lunarg.com)
- [Mark Young](mailto:marky@lunarg.com)

## Information for Developing or Contributing

Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file in this repository for more details.
Please see the [GOVERNANCE.md](GOVERNANCE.md) file in this repository for repository management details.

## How to Build and Run

[BUILD.md](BUILD.md) includes directions for building all components.

Architecture and interface information for the loader is in [docs/LoaderInterfaceArchitecture.md](docs/LoaderInterfaceArchitecture.md).

## Version Tagging Scheme

Updates to this repository which correspond to a new Vulkan specification release are tagged using the following format: `v<`_`version`_`>` (e.g., `v1.3.266`).

**Note**: Marked version releases have undergone thorough testing but do not imply the same quality level as SDK tags. SDK tags follow the `vulkan-sdk-<`_`version`_`>.<`_`patch`_`>` format (e.g., `vulkan-sdk-1.3.266.0`).

This scheme was adopted following the `1.3.266` Vulkan specification release.

## License

This work is released as open source under a Apache-style license from Khronos including a Khronos copyright.

## Acknowledgements

While this project has been developed primarily by LunarG, Inc., there are many other
companies and individuals making this possible: Valve Corporation, funding
project development; Khronos providing oversight and hosting of the project.
