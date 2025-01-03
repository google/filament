![Alt](/doc/manual/logo-mini.png "GLI Logo")

# GLI 0.8.3 Manual

![Alt](/doc/manual/g-truc.png "G-Truc Logo")

---
## Table of Contents
+ [0. Licenses](#section0)
+ [1. Getting started](#section1)
+ [1.1. Setup](#section1_1)
+ [1.2. Dependencies](#section1_2)
+ [2. Code samples](#section2)
+ [2.1. Generating mipmaps and converting float textures to rgb9e5](#section2_1)
+ [2.2. Creating an OpenGL texture object from file](#section2_2)
+ [3. References](#section3)
+ [3.1. Equivalent for other languages](#section3_1)

---
## Licenses <a name="section0"></a>

### The Happy Bunny License (Modified MIT License)

Copyright (c) 2010 - 2020 G-Truc Creation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

Restrictions: By making use of the Software for military purposes, you
choose to make a Bunny unhappy.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

![](https://github.com/g-truc/glm/blob/manual/doc/manual/frontpage1.png)

### The MIT License

Copyright (c) 2010 - 2020 G-Truc Creation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

![](https://github.com/g-truc/glm/blob/manual/doc/manual/frontpage2.png)

---
## 1. Getting started <a name="section1"></a>
### 1.1. Setup <a name="section1_1"></a>

GLI is a header only library. Hence, there is nothing to build to use it. To use GLI, merely include &lt;gli/gli.hpp&gt; header.

Core GLI features can be included using individual headers to allow faster user program compilations.

```cpp
#include <gli/clear.hpp> // clear, clear_level, clear_layer
#include <gli/comparison.hpp> // == and != operators on textures and images
#include <gli/convert.hpp> // convert a texture from a format to another
#include <gli/copy.hpp> // copy a texture or subset of a texture to another texture
#include <gli/duplicate.hpp> // duplicate the data of a texture, allocating a new texture storage
#include <gli/dx.hpp> // facilitate the use of GLI with Direct3D API
#include <gli/format.hpp> // list of the supported formats
#include <gli/generate_mipmaps.hpp> // generating the mipmaps of a texture
#include <gli/gl.hpp> // translate GLI enums to OpenGL enums
#include <gli/image.hpp> // use images, a representation of a single texture level.
#include <gli/levels.hpp> // compute the number of mipmaps levels necessary to create a mipmap complete texture.
#include <gli/load.hpp> // load DDS, KTX or KMG textures from files or memory.
#include <gli/load_dds.hpp> // load DDS textures from files or memory.
#include <gli/load_kmg.hpp> // load KMG textures from files or memory.
#include <gli/load_ktx.hpp> // load KTX textures from files or memory.
#include <gli/make_texture.hpp> // helper functions to create generic texture
#include <gli/reduce.hpp> // include to perform reduction operations.
#include <gli/sampler.hpp> // enumations for texture sampling
#include <gli/sampler_cube.hpp> // class to sample cube texture
#include <gli/sampler_cube_array.hpp> // class to sample cube array texture
#include <gli/sampler1d.hpp> // class to sample 1d texture
#include <gli/sampler1d_array.hpp> // class to sample 1d array texture
#include <gli/sampler2d.hpp> // class to sample 2d texture
#include <gli/sampler2d_array.hpp> // class to sample 2d array texture
#include <gli/sampler3d.hpp> // class to sample 3d texture
#include <gli/save.hpp> // save either a DDS, KTX or KMG file
#include <gli/save_dds.hpp> // save a DDS texture file
#include <gli/save_kmg.hpp> // save a KMG texture file
#include <gli/save_ktx.hpp> // save a KTX texture file
#include <gli/target.hpp> // helper function to query property of a generic texture
#include <gli/texture.hpp> // generic texture class that can represent any kind of texture
#include <gli/texture_cube.hpp> // representation of a cube texture
#include <gli/texture_cube_array.hpp> // representation of a cube array texture
#include <gli/texture1d.hpp> // representation of a 1d texture
#include <gli/texture1d_array.hpp> // representation of a 1d array texture
#include <gli/texture2d.hpp> // representation of a 2d texture
#include <gli/texture2d_array.hpp> // representation of a 2d array texture
#include <gli/texture3d.hpp> // representation of a 3d texture
#include <gli/transform.hpp> // perform operation on source data to destination data
#include <gli/type.hpp> // extent*d types
#include <gli/view.hpp> // create a texture view, same storage but a different scope or interpretation of the data
```

### 1.2. Dependencies <a name="section1_2"></a>

GLI does not depend on external libraries or headers such as `<GL/gl.h>`, [`<GL/glcorearb.h>`](http://www.opengl.org/registry/api/GL/glcorearb.h), `<GLES3/gl3.h>`, `<GL/glu.h>`, or `<windows.h>`.

---
## 2. Code samples <a name="section2"></a>

### 2.1. Generating mipmaps and converting float textures to rgb9e5 <a name="section2_1"></a>

```cpp
#include <gli/texture2d.hpp>
#include <gli/convert.hpp>
#include <gli/generate_mipmaps.hpp>

bool convert_rgb32f_rgb9e5(const char* FilenameSrc, const char* FilenameDst)
{
	if(FilenameDst == NULL)
		return false;
	if(std::strstr(FilenameDst, ".dds") > 0 || std::strstr(FilenameDst, ".ktx") > 0)
		return false;

	gli::texture2d TextureSource(gli::load(FilenameSrc));
	if(TextureSource.empty())
		return false;
	if(TextureSource.format() != gli::FORMAT_RGB16_SFLOAT_PACK16 && TextureSource.format() != gli::FORMAT_RGB32_SFLOAT_PACK32)
		return false;

	gli::texture2d TextureMipmaped = gli::generate_mipmaps(TextureSource, gli::FILTER_LINEAR);

	gli::texture2d TextureConverted = gli::convert(TextureMipmaped, gli::FORMAT_RGB9E5_UFLOAT_PACK32);

	gli::save(TextureConverted, FilenameDst);

	return true;
}
```

### 2.2. Creating an OpenGL texture object from file <a name="section2_2"></a>

```cpp
#include <gli/gli.hpp>

/// Filename can be KTX or DDS files
GLuint create_texture(char const* Filename)
{
	gli::texture Texture = gli::load(Filename);
	if(Texture.empty())
		return 0;

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
	GLenum Target = GL.translate(Texture.target());

	GLuint TextureName = 0;
	glGenTextures(1, &TextureName);
	glBindTexture(Target, TextureName);
	glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

	glm::tvec3<GLsizei> const Extent(Texture.extent());
	GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

	switch(Texture.target())
	{
	case gli::TARGET_1D:
		glTexStorage1D(
			Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x);
		break;
	case gli::TARGET_1D_ARRAY:
	case gli::TARGET_2D:
	case gli::TARGET_CUBE:
		glTexStorage2D(
			Target, static_cast<GLint>(Texture.levels()), Format.Internal,
			Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
		break;
	case gli::TARGET_2D_ARRAY:
	case gli::TARGET_3D:
	case gli::TARGET_CUBE_ARRAY:
		glTexStorage3D(
			Target, static_cast<GLint>(Texture.levels()), Format.Internal,
			Extent.x, Extent.y,
			Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
		break;
	default:
		assert(0);
		break;
	}

	for(std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
	for(std::size_t Face = 0; Face < Texture.faces(); ++Face)
	for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
	{
		GLsizei const LayerGL = static_cast<GLsizei>(Layer);
		glm::tvec3<GLsizei> Extent(Texture.extent(Level));
		Target = gli::is_target_cube(Texture.target())
			? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
			: Target;

		switch(Texture.target())
		{
		case gli::TARGET_1D:
			if(gli::is_compressed(Texture.format()))
				glCompressedTexSubImage1D(
					Target, static_cast<GLint>(Level), 0, Extent.x,
					Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
					Texture.data(Layer, Face, Level));
			else
				glTexSubImage1D(
					Target, static_cast<GLint>(Level), 0, Extent.x,
					Format.External, Format.Type,
					Texture.data(Layer, Face, Level));
			break;
		case gli::TARGET_1D_ARRAY:
		case gli::TARGET_2D:
		case gli::TARGET_CUBE:
			if(gli::is_compressed(Texture.format()))
				glCompressedTexSubImage2D(
					Target, static_cast<GLint>(Level),
					0, 0,
					Extent.x,
					Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
					Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
					Texture.data(Layer, Face, Level));
			else
				glTexSubImage2D(
					Target, static_cast<GLint>(Level),
					0, 0,
					Extent.x,
					Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
					Format.External, Format.Type,
					Texture.data(Layer, Face, Level));
			break;
		case gli::TARGET_2D_ARRAY:
		case gli::TARGET_3D:
		case gli::TARGET_CUBE_ARRAY:
			if(gli::is_compressed(Texture.format()))
				glCompressedTexSubImage3D(
					Target, static_cast<GLint>(Level),
					0, 0, 0,
					Extent.x, Extent.y,
					Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
					Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
					Texture.data(Layer, Face, Level));
			else
				glTexSubImage3D(
					Target, static_cast<GLint>(Level),
					0, 0, 0,
					Extent.x, Extent.y,
					Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
					Format.External, Format.Type,
					Texture.data(Layer, Face, Level));
			break;
		default: assert(0); break;
		}
	}
	return TextureName;
}
```

---
## <a name="section3"></a> 3. References

### <a name="section3_1"></a> 3.1. Equivalent for other languages

* [JVM OpenGL Image (GLI)](https://github.com/kotlin-graphics/gli)
