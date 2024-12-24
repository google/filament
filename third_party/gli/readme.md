![gli](/doc/manual/logo-mini.png)

[OpenGL Image](http://gli.g-truc.net/) (*GLI*) is a header only C++ image library for graphics software.

*GLI* provides classes and functions to load image files (*[KTX](https://www.khronos.org/opengles/sdk/tools/KTX/)* and *[DDS](https://msdn.microsoft.com/en-us/library/windows/desktop/bb943990%28v=vs.85%29.aspx)*), facilitate graphics APIs texture creation, compare textures, access texture texels, sample textures, convert textures, generate mipmaps, etc.

This library works perfectly with *[OpenGL](https://www.opengl.org)* or *[Vulkan](https://www.khronos.org/vulkan)* but it also ensures interoperability with other third party libraries and SDK.
It is a good candidate for software rendering (raytracing / rasterisation), image processing, image based software testing or any development context that requires a simple and convenient image library.

*GLI* is written in C++11. It is a platform independent library with no dependence and it supports the following compilers:
- [Apple Clang 6.0](https://developer.apple.com/library/mac/documentation/CompilerTools/Conceptual/LLVMCompilerOverview/index.html) and higher
- [GCC](http://gcc.gnu.org/) 4.7 and higher
- [Intel C++ Composer](https://software.intel.com/en-us/intel-compilers) XE 2013 and higher
- [LLVM](http://llvm.org/) 3.4 and higher
- [Visual C++](http://www.visualstudio.com/) 2013 and higher
- Any C++11 compiler

For more information about *GLI*, please have a look at the [manual](manual.md) and the [API reference documentation](http://gli.g-truc.net/0.8.2/api/index.html).
The source code and the documentation are licensed under the [Happy Bunny License (Modified MIT) or the MIT License](manual.md#section0).

Thanks for contributing to the project by [submitting pull requests](https://github.com/g-truc/glm/pulls).

```cpp
#include <gli/gli.hpp>

GLuint CreateTexture(char const* Filename)
{
	gli::texture Texture = gli::load(Filename);
	if(Texture.empty())
		return 0;

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
	GLenum Target = GL.translate(Texture.target());
	assert(gli::is_compressed(Texture.format()) && Target == gli::TARGET_2D);

	GLuint TextureName = 0;
	glGenTextures(1, &TextureName);
	glBindTexture(Target, TextureName);
	glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
	glTexParameteriv(Target, GL_TEXTURE_SWIZZLE_RGBA, &Format.Swizzles[0]);
	glTexStorage2D(Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x, Extent.y);
	for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
	{
		glm::tvec3<GLsizei> Extent(Texture.extent(Level));
		glCompressedTexSubImage2D(
			Target, static_cast<GLint>(Level), 0, 0, Extent.x, Extent.y,
			Format.Internal, static_cast<GLsizei>(Texture.size(Level)), Texture.data(0, 0, Level));
	}

	return TextureName;
}
```

## [Lastest release](https://github.com/g-truc/gli/releases/latest)

## Project Health

| Service | System | Compiler | Status |
| ------- | ------ | -------- | ------ |
| [Travis CI](https://travis-ci.org/g-truc/gli)| Linux 64 bits | Clang 3.9, Clang 7, Clang 9, Clang 10, GCC 4.8, GCC 7.4, GCC 9, GCC 10 | [![Travis CI](https://travis-ci.org/g-truc/gli.svg?branch=master)](https://travis-ci.org/g-truc/gli)
| [AppVeyor](https://ci.appveyor.com/project/Groovounet/gli)| Windows 32 and 64 | Visual Studio 2013 | [![AppVeyor](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?svg=true)](https://ci.appveyor.com/project/Groovounet/gli)

## Release notes
### [GLI 0.8.3.0](https://github.com/g-truc/gli/releases/latest) - 2017-XX-XX
#### Features:
- Added decompression and sampling of DXT1, DXT3, DXT5, ATI1N and ATI2N #110 #119
- Added depth and stencil format queries #119
- Added texture_grad to samplers

#### Fixes:
- Fixed R8 SRGB #120

---
### [GLI 0.8.2.0](https://github.com/g-truc/gli/releases/tag/0.8.2.0) - 2016-11-13
#### Features:
- Extend flip() for S3TC compressed textures #94
- Added format property queries #102

#### Improvements:
- Fixed texture operator=
- Added initial manual

#### Fixes:
- Fixed ATI2N swizzle parameters #121

---
### [GLI 0.8.1.1](https://github.com/g-truc/gli/releases/tag/0.8.1.1) - 2016-09-11
#### Improvements:
- Updated GLM to 0.9.8.0 release

#### Fixes:
- Fixed KTX cube maps saving
- Fixed texture::clear build

---
### [GLI 0.8.1.0](https://github.com/g-truc/gli/releases/tag/0.8.1.0) - 2016-03-16
#### Features:
- Added texture copy, no allocation involved, only transfer
- Added sub-image copy
- Added non-member clear
- Added make_texture* helper functions

#### Improvements:
- Added compressed npot textures support #73
- Added image access cache to generic textures
- Added luminance alpha format translation to OpenGL 3.3+ through RG swizzling. #93
- Improved performance when sampling using mipmaps
- Improved nearest filter without border using texture_lod performance (~2.5x faster with texture2d)
- Improved texture::data() (~18x with cube array, ~68x with 2D)
- Improved texture::size() (~3.1x with cube array, ~3.9x with 2D)
- Improved simultanous texture::size() and texture::extent() calls (~2.1x with cube array, ~2.3x with 2D)

#### Fixes:
- Fixed DX10 DDS saving of 3D textures #91
- Fixed BGRX translation with PROFILE_GL33 profile #92
- Fixed DDS9 loading of 3D texture detected as 2D texture #93

---
### [GLI 0.8.0.0](https://github.com/g-truc/gli/releases/tag/0.8.0.0) - 2016-02-16
#### Features:
- Added texture swizzle support #79
- Added texture memory swizzle support
- Added texture conversion from any uncompressed format to any uncompressed format
- Added texture lod
- Added texture mipmaps generation for uncompressed formats
- Added support for load and store
- Added support for many new formats
- Added sampler 1D, 2D, 3D, array and cube map
- Added sampler texel fetch and texel write
- Added sampler clear
- Added transform algorithm to compute arithmetic between texels
- Added reduce algorithm to compare all texels within an image

#### Improvements:
- Reordered formats to match Vulkan formats
- Improved OpenGL translation with for multiple profiles: KTX, ES2.0, ES3.0, GL3.2 and GL3.3
- Improved Doxygen documentation

#### Fixes:
- Fixed PVRTC2 support
- Fixed luminance and alpha translation to OpenGL #56
- Fixed DXGI_FORMAT_B8G8R8X8_UNORM_SRGB support #59
- Fixed FORMAT_RGBA8_UNORM DDS loading using DDPF_RGBA mode #60
- Fixed handling of DDS DDPF_ALPHAPIXELS #68
- Fixed images, better matching names and formats #78 #81 #80
- Fixed BC4U and BC5U files generated from MS DDS loader #82

#### Work in progress:
- Added KMG container support and spec proposal

---
### [GLI 0.7.0.0](https://github.com/g-truc/gli/releases/tag/0.7.0.0) - 2015-09-01
- Added KTX loading and saving
- Added gli::load for generic file loading, either DDS or KTX files depending on filename extensions
- Added gli::save for generic file saving, either DDS or KTX files depending on filename extensions
- Added texture views using different texture format, including compressed texture formats
- Added fine granularity includes
- Improved API documentation
- Much faster texture comparisons is non optimal cases. (Measured ~21x faster on Intel IVB)
- Explicitly handling of texture targets: fixed various cases of cubemap and texture arrays failing to load with DDS
- Fixed GCC build
- Fixed warnings
- Fixed saved DDS header size on #52

---
### [GLI 0.6.1.1](https://github.com/g-truc/gli/releases/tag/0.6.1.1) - 2015-07-18
- Updated API documentation
- Fixed link error

---
### [GLI 0.6.1.0](https://github.com/g-truc/gli/releases/tag/0.6.1.0) - 2015-07-18
- Fixed interface inconsistencies
- Improved clear(), data() and size() performance using caching
- Removed internal dependence to std::fstream
- Added FORMAT_BGRX8_UNORM and FORMAT_BGRX8_SRGB support #48, #43
- Improved FORMAT_RGB8_UNORM loading

---
### [GLI 0.6.0.0](https://github.com/g-truc/gli/releases/tag/0.6.0.0) - 2015-06-28
- Large refactoring
- Added loading DDS from memory
- Added saving DDS to memory
- Improved DDS coverage for R, RG, RGB and RGBA formats
- Added DDS ASTC, PVRTC, ATC and ETC support
- Added DDS alpha, luminance and alpha luminance support
- Added PVRTC2, ETC2 and EAC formats

---
### [GLI 0.5.1.1](https://github.com/g-truc/gli/releases/tag/0.5.1.1) - 2014-01-20
- Fixed swizzled RGB channel when reading back a DDS
- Fixed getMask* link error

---
### [GLI 0.5.1.0](https://github.com/g-truc/gli/releases/tag/0.5.1.0) - 2014-01-18
- Added flip function
- Added level_count function
- Fixed interaction with std::map (#33)
- Added texelFetch and texelWrite functions

---
### [GLI 0.5.0.0](https://github.com/g-truc/gli/releases/tag/0.5.0.0) - 2013-11-24
- Essencially a rewrite of the library
- Added explicit copies
- Added single memory allocation per texture storage
- Added texture views
- Added texture copies
- Added comparison operators
- Added clear

---
### GLI 0.4.1.0: 2013-03-10
- Added DDS saving
- Fixed GCC build
- Fixed XCode build

---
### GLI 0.4.0.0: 2013-01-28
- Large API refactoring
- Performance improvements at loading: 50x in debug and 50% in release build
- Added texture2DArray
- Added textureCube and textureCubeArray
- Added texture3D
- Added texture1D and texture1DArray
- Improved DDS loading support

---
### GLI 0.3.0.3: 2011-04-05
- Fixed bugs

---
### GLI 0.3.0.2: 2011-02-08
- Fixed bugs

---
### GLI 0.3.0.1: 2010-10-15
- Fixed bugs

---
### GLI 0.3.0.0: 2010-10-01
- Added DDS10 load and store (BC1 - BC7)
- Added extension system
- Added automatic OpenGL texture object creation from file

---
### GLI 0.2.2.0: 2010-09-07
- Added DDS exporter

---
### GLI 0.2.1.1: 2010-05-12
- Fixed GCC build

---
### GLI 0.2.1.0: 2010-02-15
- Added texelWrite function
- Fixed Visual Studio 2010 warnings
- Added readme.txt and copying.txt

---
### GLI 0.2.0.0: 2010-01-10
- Updated API
- Removed Boost dependency

---
### GLI 0.1.1.0: 2009-09-18
- Fixed DDS loader
- Added RGB8 to DDS loader
- Added component swizzle operation
- Added 32 bits integer components support
- Fixed texel fetch

---
### GLI 0.1.0.0: 2009-03-28
- Added TGA loader
- Added DDS loader
- Added duplicate, crop, partial copy
- Added mipmaps generation.

