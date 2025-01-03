#include <gli/texture.hpp>
#include <gli/texture1d.hpp>
#include <gli/comparison.hpp>

namespace swizzle
{
	int run()
	{
		int Error(0);

		{
			gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(1), 1, 1, 1);
			gli::texture::swizzles_type const Swizzles = Texture.swizzles();
			Error += Swizzles == gli::swizzles(gli::SWIZZLE_RED, gli::SWIZZLE_GREEN, gli::SWIZZLE_BLUE, gli::SWIZZLE_ALPHA) ? 0 : 1;
		}

		{
			gli::texture Texture(gli::TARGET_2D, gli::FORMAT_BGRA8_UNORM_PACK8, gli::texture::extent_type(1), 1, 1, 1);
			gli::texture::swizzles_type const Swizzles = Texture.swizzles();
			Error += Swizzles == gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA) ? 0 : 1;
		}

		{
			gli::texture Texture(gli::TARGET_2D, gli::FORMAT_BGRA8_UNORM_PACK8, gli::texture::extent_type(1), 1, 1, 1, gli::swizzles(gli::SWIZZLE_RED, gli::SWIZZLE_GREEN, gli::SWIZZLE_BLUE, gli::SWIZZLE_ALPHA));
			gli::texture::swizzles_type const Swizzles = Texture.swizzles();
			Error += Swizzles == gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA) ? 0 : 1;
		}

		{
			gli::texture Texture(gli::TARGET_2D, gli::FORMAT_BGRA8_UNORM_PACK8, gli::texture::extent_type(1), 1, 1, 1, gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));
			gli::texture::swizzles_type const Swizzles = Texture.swizzles();
			Error += Swizzles == gli::swizzles(gli::SWIZZLE_RED, gli::SWIZZLE_GREEN, gli::SWIZZLE_BLUE, gli::SWIZZLE_ALPHA) ? 0 : 1;
		}

		return Error;
	}
}//namespace swizzle

namespace texture1d
{
	int run()
	{
		int Error(0);

		gli::texture TextureA(gli::TARGET_1D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 1, 1), 1, 1, 1);
		TextureA.clear(glm::u8vec4(255, 127, 0, 192));

		{
			gli::texture1d TextureA1(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(4), 1);
			Error += TextureA.swizzles() == TextureA1.swizzles() ? 0 : 1;

			gli::texture1d TextureA2(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(4), 1, gli::swizzles(gli::SWIZZLE_RED, gli::SWIZZLE_GREEN, gli::SWIZZLE_BLUE, gli::SWIZZLE_ALPHA));
			Error += TextureA.swizzles() == TextureA2.swizzles() ? 0 : 1;

			Error += TextureA1.swizzles() == TextureA2.swizzles() ? 0 : 1;
		}

		gli::texture TextureB(gli::TARGET_1D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 1, 1), 1, 1, 1);
		TextureB.clear(glm::u8vec4(0, 127, 255, 192));
		TextureB.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture1d TextureC(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(4), 1);
		TextureC.clear(glm::u8vec4(255, 127, 0, 192));

		Error += TextureA == TextureC ? 0 : 1;

		gli::texture1d TextureD(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(4), 1);
		TextureD.clear(glm::u8vec4(0, 127, 255, 192));
		TextureD.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureD ? 0 : 1;

		return Error;
	}
}//namespace texture1d

namespace texture1d_array
{
	int run()
	{
		int Error(0);

		gli::texture TextureA(gli::TARGET_1D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 1, 1), 2, 1, 4);
		TextureA.clear(glm::u8vec4(255, 127, 0, 192));

		{
			gli::texture1d_array TextureA1(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d_array::extent_type(4), 1, 1);
			Error += TextureA.swizzles() == TextureA1.swizzles() ? 0 : 1;

			gli::texture1d_array TextureA2(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d_array::extent_type(4), 1, 1, gli::swizzles(gli::SWIZZLE_RED, gli::SWIZZLE_GREEN, gli::SWIZZLE_BLUE, gli::SWIZZLE_ALPHA));
			Error += TextureA.swizzles() == TextureA2.swizzles() ? 0 : 1;

			Error += TextureA1.swizzles() == TextureA2.swizzles() ? 0 : 1;
		}

		gli::texture TextureB(gli::TARGET_1D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 1, 1), 2, 1, 4);
		TextureB.clear(glm::u8vec4(0, 127, 255, 192));
		TextureB.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture1d_array TextureC(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d_array::extent_type(4), 2, 4);
		TextureC.clear(glm::u8vec4(255, 127, 0, 192));

		Error += TextureA == TextureC ? 0 : 1;

		gli::texture1d_array TextureD(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d_array::extent_type(4), 2, 4);
		TextureD.clear(glm::u8vec4(0, 127, 255, 192));
		TextureD.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureC == TextureD ? 0 : 1;

		return Error;
	}
}//namespace texture1d_array

namespace texture2d
{
	int run()
	{
		int Error(0);

		gli::texture TextureA(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 1, 2);
		TextureA.clear(glm::u8vec4(255, 127, 0, 192));

		{
			gli::texture2d TextureA1(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4), 1);
			Error += TextureA.swizzles() == TextureA1.swizzles() ? 0 : 1;

			gli::texture2d TextureA2(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4), 1, gli::swizzles(gli::SWIZZLE_RED, gli::SWIZZLE_GREEN, gli::SWIZZLE_BLUE, gli::SWIZZLE_ALPHA));
			Error += TextureA.swizzles() == TextureA2.swizzles() ? 0 : 1;

			Error += TextureA1.swizzles() == TextureA2.swizzles() ? 0 : 1;
		}

		gli::texture TextureB(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 1, 2);
		TextureB.clear(glm::u8vec4(0, 127, 255, 192));
		TextureB.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture2d TextureC(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4, 4), 2);
		TextureC.clear(glm::u8vec4(255, 127, 0, 192));

		Error += TextureA == TextureC ? 0 : 1;

		gli::texture2d TextureD(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4, 4), 2);
		TextureD.clear(glm::u8vec4(0, 127, 255, 192));
		TextureD.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureC == TextureD ? 0 : 1;

		return Error;
	}
}//namespace texture2d

namespace texture2d_array
{
	int run()
	{
		int Error(0);

		gli::texture TextureA(gli::TARGET_2D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 2, 1, 4);
		TextureA.clear(glm::u8vec4(255, 127, 0, 192));

		{
			gli::texture2d_array TextureA1(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d_array::extent_type(4), 1, 1);
			Error += TextureA.swizzles() == TextureA1.swizzles() ? 0 : 1;

			gli::texture2d_array TextureA2(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d_array::extent_type(4), 1, 1, gli::swizzles(gli::SWIZZLE_RED, gli::SWIZZLE_GREEN, gli::SWIZZLE_BLUE, gli::SWIZZLE_ALPHA));
			Error += TextureA.swizzles() == TextureA2.swizzles() ? 0 : 1;

			Error += TextureA1.swizzles() == TextureA2.swizzles() ? 0 : 1;
		}

		gli::texture TextureB(gli::TARGET_2D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 2, 1, 4);
		TextureB.clear(glm::u8vec4(0, 127, 255, 192));
		TextureB.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture2d_array TextureC(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4, 4), 2, 4);
		TextureC.clear(glm::u8vec4(255, 127, 0, 192));

		Error += TextureA == TextureC ? 0 : 1;

		gli::texture2d_array TextureD(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4, 4), 2, 4);
		TextureD.clear(glm::u8vec4(0, 127, 255, 192));
		TextureD.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureC == TextureD ? 0 : 1;

		return Error;
	}
}//namespace texture2d_array

namespace texture3d
{
	int run()
	{
		int Error(0);

		gli::texture TextureA(gli::TARGET_3D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 4), 1, 1, 2);
		TextureA.clear(glm::u8vec4(255, 127, 0, 192));

		{
			gli::texture3d TextureA1(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(4), 1);
			Error += TextureA.swizzles() == TextureA1.swizzles() ? 0 : 1;

			gli::texture3d TextureA2(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(4), 1, gli::swizzles(gli::SWIZZLE_RED, gli::SWIZZLE_GREEN, gli::SWIZZLE_BLUE, gli::SWIZZLE_ALPHA));
			Error += TextureA.swizzles() == TextureA2.swizzles() ? 0 : 1;

			Error += TextureA1.swizzles() == TextureA2.swizzles() ? 0 : 1;
		}

		gli::texture TextureB(gli::TARGET_3D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 4), 1, 1, 2);
		TextureB.clear(glm::u8vec4(0, 127, 255, 192));
		TextureB.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture3d TextureC(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(4, 4, 4), 2);
		TextureC.clear(glm::u8vec4(255, 127, 0, 192));

		Error += TextureA == TextureC ? 0 : 1;

		gli::texture3d TextureD(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(4, 4, 4), 2);
		TextureD.clear(glm::u8vec4(0, 127, 255, 192));
		TextureD.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureC == TextureD ? 0 : 1;

		return Error;
	}
}//namespace texture3d

namespace texture_cube
{
	int run()
	{
		int Error(0);

		gli::texture TextureA(gli::TARGET_CUBE, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 6, 2);
		TextureA.clear(glm::u8vec4(255, 127, 0, 192));

		{
			gli::texture_cube TextureA1(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(4), 1);
			Error += TextureA.swizzles() == TextureA1.swizzles() ? 0 : 1;

			gli::texture_cube TextureA2(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(4), 1, gli::swizzles(gli::SWIZZLE_RED, gli::SWIZZLE_GREEN, gli::SWIZZLE_BLUE, gli::SWIZZLE_ALPHA));
			Error += TextureA.swizzles() == TextureA2.swizzles() ? 0 : 1;

			Error += TextureA1.swizzles() == TextureA2.swizzles() ? 0 : 1;
		}

		gli::texture TextureB(gli::TARGET_CUBE, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 6, 2);
		TextureB.clear(glm::u8vec4(0, 127, 255, 192));
		TextureB.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture_cube TextureC(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(4, 4), 2);
		TextureC.clear(glm::u8vec4(255, 127, 0, 192));

		Error += TextureA == TextureC ? 0 : 1;

		gli::texture_cube TextureD(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(4, 4), 2);
		TextureD.clear(glm::u8vec4(0, 127, 255, 192));
		TextureD.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureC == TextureD ? 0 : 1;

		return Error;
	}
}//namespace texture_cube

namespace texture_cube_array
{
	int run()
	{
		int Error(0);

		gli::texture TextureA(gli::TARGET_CUBE_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 3, 6, 2);
		TextureA.clear(glm::u8vec4(255, 127, 0, 192));

		{
			gli::texture_cube_array TextureA1(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube_array::extent_type(4), 1, 1);
			Error += TextureA.swizzles() == TextureA1.swizzles() ? 0 : 1;

			gli::texture_cube_array TextureA2(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube_array::extent_type(4), 1, 1, gli::swizzles(gli::SWIZZLE_RED, gli::SWIZZLE_GREEN, gli::SWIZZLE_BLUE, gli::SWIZZLE_ALPHA));
			Error += TextureA.swizzles() == TextureA2.swizzles() ? 0 : 1;

			Error += TextureA1.swizzles() == TextureA2.swizzles() ? 0 : 1;
		}

		gli::texture TextureB(gli::TARGET_CUBE_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 3, 6, 2);
		TextureB.clear(glm::u8vec4(0, 127, 255, 192));
		TextureB.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureB ? 0 : 1;

		gli::texture_cube_array TextureC(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(4, 4), 3, 2);
		TextureC.clear(glm::u8vec4(255, 127, 0, 192));

		Error += TextureA == TextureC ? 0 : 1;

		gli::texture_cube_array TextureD(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(4, 4), 3, 2);
		TextureD.clear(glm::u8vec4(0, 127, 255, 192));
		TextureD.swizzle<glm::u8vec4>(gli::swizzles(gli::SWIZZLE_BLUE, gli::SWIZZLE_GREEN, gli::SWIZZLE_RED, gli::SWIZZLE_ALPHA));

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureC == TextureD ? 0 : 1;

		return Error;
	}
}//namespace texture_cube_array

int main()
{
	int Error(0);

	Error += swizzle::run();
	Error += texture1d::run();
	Error += texture1d_array::run();
	Error += texture2d::run();
	Error += texture2d_array::run();
	Error += texture3d::run();
	Error += texture_cube::run();
	Error += texture_cube_array::run();

	GLI_ASSERT(!Error);

	return Error;
}

