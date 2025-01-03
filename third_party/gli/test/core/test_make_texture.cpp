#include <gli/texture.hpp>
#include <gli/make_texture.hpp>
#include <gli/texture2d.hpp>
#include <gli/comparison.hpp>

namespace is_make_texture_equivalent_to_ctor
{
	int main()
	{
		int Error = 0;

		{
			gli::texture TextureA = gli::make_texture1d(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent1d(4), 2);
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture1d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent1d(4), 2);
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 1, 1);
			gli::texture TextureC(gli::TARGET_1D, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 1, 1, 2);
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture1d_array(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent1d(4), 3, 2);
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture1d_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent1d(4), 3, 2);
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 1, 1);
			gli::texture TextureC(gli::TARGET_1D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 3, 1, 2);
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture2d(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 2);
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture2d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 2);
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 4, 1);
			gli::texture TextureC(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 1, 1, 2);
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture2d_array(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 3, 2);
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture2d_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 3, 2);
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 4, 1);
			gli::texture TextureC(gli::TARGET_2D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 3, 1, 2);
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture3d(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent3d(4), 2);
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture3d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent3d(4), 2);
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 4, 4);
			gli::texture TextureC(gli::TARGET_3D, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 1, 1, 2);
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture_cube(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 2);
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture_cube TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 2);
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 4, 1);
			gli::texture TextureC(gli::TARGET_CUBE, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 1, 6, 2);
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture_cube_array(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 3, 2);
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture_cube_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 3, 2);
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 4, 1);
			gli::texture TextureC(gli::TARGET_CUBE_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 3, 6, 2);
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		return Error;
	}
}//namespace is_make_texture_equivalent_to_ctor

namespace is_make_texture_support_mipmap_chain
{
	int main()
	{
		int Error = 0;

		{
			gli::texture TextureA = gli::make_texture1d(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent1d(4));
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture1d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent1d(4));
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 1, 1);
			gli::texture TextureC(gli::TARGET_1D, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 1, 1, gli::levels(ExtentC));
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture1d_array(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent1d(4), 3);
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture1d_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent1d(4), 3);
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 1, 1);
			gli::texture TextureC(gli::TARGET_1D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 3, 1, gli::levels(ExtentC));
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture2d(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4));
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture2d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4));
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 4, 1);
			gli::texture TextureC(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 1, 1, gli::levels(ExtentC));
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture2d_array(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 3);
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture2d_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 3);
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 4, 1);
			gli::texture TextureC(gli::TARGET_2D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 3, 1, gli::levels(ExtentC));
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture3d(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent3d(4));
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture3d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent3d(4));
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 4, 4);
			gli::texture TextureC(gli::TARGET_3D, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 1, 1, gli::levels(ExtentC));
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture_cube(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4));
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture_cube TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4));
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 4, 1);
			gli::texture TextureC(gli::TARGET_CUBE, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 1, 6, gli::levels(ExtentC));
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		{
			gli::texture TextureA = gli::make_texture_cube_array(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 3);
			TextureA.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture_cube_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::extent2d(4), 3);
			TextureB.clear(gli::u8vec4(255, 127, 0, 255));

			gli::texture::extent_type const ExtentC(4, 4, 1);
			gli::texture TextureC(gli::TARGET_CUBE_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, ExtentC, 3, 6, gli::levels(ExtentC));
			TextureC.clear(gli::u8vec4(255, 127, 0, 255));

			Error += TextureA == TextureB ? 0 : 1;
			Error += TextureA == TextureC ? 0 : 1;
			Error += TextureB == TextureC ? 0 : 1;
		}

		return Error;
	}
}//namespace is_make_texture_support_mipmap_chain

int main()
{
	int Error = 0;

	Error += is_make_texture_equivalent_to_ctor::main();
	Error += is_make_texture_support_mipmap_chain::main();

	return Error;
}

