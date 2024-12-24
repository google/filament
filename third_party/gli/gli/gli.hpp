/// @brief Include to include everything in GLI which is not recommendated due to compilation time cost.
/// @file gli/gli.hpp
/// @mainpage OpenGL Image (GLI)
///
/// [OpenGL Image](http://gli.g-truc.net/) (*GLI*) is a header only C++ image library for graphics software.
/// (*GLI*) provides classes and functions to load image files ([KTX](https://www.khronos.org/opengles/sdk/tools/KTX/) and [DDS](https://msdn.microsoft.com/en-us/library/windows/desktop/bb943990%28v=vs.85%29.aspx)),
/// facilitate graphics APIs texture creation, compare textures, access texture texels, sample textures, convert textures, generate mipmaps, etc.
///
/// This library works perfectly with [OpenGL](https://www.opengl.org) or [Vulkan](https://www.khronos.org/vulkan) but it also ensures interoperability with other third party libraries and SDK.
/// It is a good candidate for software rendering (raytracing / rasterisation), image processing, image based software testing or any development context that requires a simple and convenient image library.
///
/// *GLI* is written in C++11. It is a platform independent library with no dependence and it supports the following compilers:
/// - [Apple Clang 4.0](https://developer.apple.com/library/mac/documentation/CompilerTools/Conceptual/LLVMCompilerOverview/index.html) and higher
/// - [GCC](http://gcc.gnu.org/) 4.6 and higher
/// - [Intel C++ Composer](https://software.intel.com/en-us/intel-compilers) XE 2013 and higher
/// - [LLVM](http://llvm.org/) 3.2 and higher
/// - [Visual C++](http://www.visualstudio.com/) 2010 and higher
/// - Any conform C++11 compiler
///
/// For more information about *GLI*, please have a look at the [API reference documentation](http://gli.g-truc.net/0.8.0/api/index.html).
/// The source code and the documentation are licensed under the [Happy Bunny License (Modified MIT) or the MIT License](copying.md).
///
/// Thanks for contributing to the project by [submitting issues](https://github.com/g-truc/gli/issues) for bug reports and feature requests. Any feedback is welcome at [gli@g-truc.net](mailto://gli@g-truc.net).

#pragma once

#define GLI_VERSION					84
#define GLI_VERSION_MAJOR			0
#define GLI_VERSION_MINOR			8
#define GLI_VERSION_PATCH			4
#define GLI_VERSION_REVISION		0

/// Namespace where all the classes and functions provided by GLI are exposed
namespace gli
{

}//namespace gli

#include "format.hpp"
#include "target.hpp"
#include "levels.hpp"

#include "image.hpp"
#include "texture.hpp"
#include "texture1d.hpp"
#include "texture1d_array.hpp"
#include "texture2d.hpp"
#include "texture2d_array.hpp"
#include "texture3d.hpp"
#include "texture_cube.hpp"
#include "texture_cube_array.hpp"

#include "sampler1d.hpp"
#include "sampler1d_array.hpp"
#include "sampler2d.hpp"
#include "sampler2d_array.hpp"
#include "sampler3d.hpp"
#include "sampler_cube.hpp"
#include "sampler_cube_array.hpp"

#include "duplicate.hpp"
#include "convert.hpp"
#include "view.hpp"
#include "comparison.hpp"

#include "reduce.hpp"
#include "transform.hpp"

#include "load.hpp"
#include "save.hpp"

#include "gl.hpp"
#include "dx.hpp"

#include "./core/flip.hpp"
