#include <gli/texture.hpp>
#include <gli/texture2d.hpp>
#include <gli/comparison.hpp>
#include <gli/view.hpp>
#include <gli/duplicate.hpp>

namespace can_compute_texture_size
{
	int main()
	{
		int Error = 0;

		// Scenario: Compute the size of a specialized 2d texture
		{
			gli::texture2d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(2), 1);

			Error += Texture.size() == 2 * 2 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size(0) == 2 * 2 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size<glm::u8vec4>() == 2 * 2 ? 0 : 1;
			Error += Texture.size<glm::u8vec4>(0) == 2 * 2 ? 0 : 1;
		}

		// Scenario: Compute the size of a generic 2d texture
		{
			gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(2, 2, 1), 1, 1, 1);

			Error += Texture.size() == 2 * 2 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size(0) == 2 * 2 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size<glm::u8vec4>() == 2 * 2 ? 0 : 1;
			Error += Texture.size<glm::u8vec4>(0) == 2 * 2 ? 0 : 1;
		}

		// Scenario: Compute the size of a specialized 2d texture with a mipmap chain
		{
			gli::texture2d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(2));

			Error += Texture.size() == 5 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size(0) == 4 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size(1) == 1 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size<glm::u8vec4>() == 5 ? 0 : 1;
			Error += Texture.size<glm::u8vec4>(0) == 4 ? 0 : 1;
			Error += Texture.size<glm::u8vec4>(1) == 1 ? 0 : 1;
		}

		// Scenario: Compute the size of a generic 2d texture with a mipmap chain
		{
			gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(2, 2, 1), 1, 1, 2);

			Error += Texture.size() == 5 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size(0) == 4 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size(1) == 1 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size<glm::u8vec4>() == 5 ? 0 : 1;
			Error += Texture.size<glm::u8vec4>(0) == 4 ? 0 : 1;
			Error += Texture.size<glm::u8vec4>(1) == 1 ? 0 : 1;
		}

		return Error;
	}
}//namespace can_compute_texture_size

namespace can_compute_view_size
{
	int main()
	{
		int Error = 0;

		// Scenario: Compute the size of a specialized 2d texture with a mipmap chain
		{
			gli::texture2d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4));
			gli::texture2d View(gli::view(Texture, 1, 2));

			Error += View.size() == 5 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += View.size(0) == 4 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += View.size(1) == 1 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += View.size<glm::u8vec4>() == 5 ? 0 : 1;
			Error += View.size<glm::u8vec4>(0) == 4 ? 0 : 1;
			Error += View.size<glm::u8vec4>(1) == 1 ? 0 : 1;
		}

		// Scenario: Compute the size of a generic 2d texture with a mipmap chain
		{
			gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(4, 4, 1), 1, 1, 3);
			gli::texture View(gli::view(gli::texture2d(Texture), 1, 2));

			Error += View.size() == 5 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += View.size(0) == 4 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += View.size(1) == 1 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += View.size<glm::u8vec4>() == 5 ? 0 : 1;
			Error += View.size<glm::u8vec4>(0) == 4 ? 0 : 1;
			Error += View.size<glm::u8vec4>(1) == 1 ? 0 : 1;
		}

		// Scenario: Compute the size of a specialized 2d array texture with a mipmap chain
		{
			gli::texture2d_array Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(4), 2);
			gli::texture2d_array View(gli::view(Texture, 1, 1, 1, 2));

			Error += View.size() == 5 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += View.size(0) == 4 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += View.size(1) == 1 * sizeof(glm::u8vec4) ? 0 : 1;
			Error += View.size<glm::u8vec4>() == 5 ? 0 : 1;
			Error += View.size<glm::u8vec4>(0) == 4 ? 0 : 1;
			Error += View.size<glm::u8vec4>(1) == 1 ? 0 : 1;
		}

		return Error;
	}
}//namespace can_compute_view_size

namespace can_compute_npot_texture_size
{
	int main()
	{
		int Error = 0;

		{
			gli::storage_linear Storage(gli::FORMAT_RGBA8_UNORM_PACK8, gli::storage_linear::extent_type(12, 12, 1), 1, 1, gli::levels(12));

			gli::size_t const LevelSize0 = Storage.level_size(0);
			Error += LevelSize0 == 12 * 12 * sizeof(glm::u8vec4) ? 0 : 1;

			gli::size_t const LevelSize1 = Storage.level_size(1);
			Error += LevelSize1 == 6 * 6 * sizeof(glm::u8vec4) ? 0 : 1;

			gli::size_t const LevelSize2 = Storage.level_size(2);
			Error += LevelSize2 == 3 * 3 * sizeof(glm::u8vec4) ? 0 : 1;

			gli::size_t const LevelSize3 = Storage.level_size(3);
			Error += LevelSize3 == 1 * 1 * sizeof(glm::u8vec4) ? 0 : 1;
		}

		{
			gli::storage_linear Storage(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, gli::storage_linear::extent_type(12, 12, 1), 1, 1, gli::levels(12));
			gli::size_t const LevelSize0 = Storage.level_size(0);
			gli::size_t const LevelSizeA = 3 * 3 * gli::block_size(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8);
			Error += LevelSize0 == LevelSizeA ? 0 : 1;

			gli::size_t const LevelSize1 = Storage.level_size(1);
			gli::size_t const LevelSizeB = 2 * 2 * gli::block_size(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8);
			Error += LevelSize1 == LevelSizeB ? 0 : 1;

			gli::size_t const LevelSize2 = Storage.level_size(2);
			gli::size_t const LevelSizeC = 1 * 1 * gli::block_size(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8);
			Error += LevelSize2 == LevelSizeC ? 0 : 1;

			gli::size_t const LevelSize3 = Storage.level_size(3);
			Error += LevelSize3 == LevelSizeC ? 0 : 1;
		}

		{
			gli::texture::extent_type const BlockCountA = glm::max(gli::texture::extent_type(5, 5, 1) / gli::block_extent(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8), gli::texture::extent_type(1));

			gli::texture::extent_type const BlockCountB = glm::ceilMultiple(gli::texture::extent_type(5, 5, 1), gli::block_extent(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8)) / gli::block_extent(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8);
		}

		{
			gli::texture2d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(5));

			gli::texture2d::size_type const SizeA1 = Texture.size();
			gli::texture2d::size_type const SizeA2 = ((5 * 5) + (2 * 2) + 1) * sizeof(glm::u8vec4);
			Error += SizeA1 == SizeA2 ? 0 : 1;

			gli::texture2d::size_type const SizeB1 = Texture.size(0);
			gli::texture2d::size_type const SizeB2 = (5 * 5) * sizeof(glm::u8vec4);
			Error += SizeB1 == SizeB2 ? 0 : 1;

			gli::texture2d::size_type const SizeC1 = Texture.size(1);
			gli::texture2d::size_type const SizeC2 = (2 * 2) * sizeof(glm::u8vec4);
			Error += SizeC1 == SizeC2 ? 0 : 1;

			gli::texture2d::size_type const SizeD1 = Texture.size(2);
			gli::texture2d::size_type const SizeD2 = (1 * 1) * sizeof(glm::u8vec4);
			Error += SizeD1 == SizeD2 ? 0 : 1;
		}

		{
			gli::texture2d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(3));

			Error += Texture.size() == (3 * 3 + 1) * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size(0) == (3 * 3) * sizeof(glm::u8vec4) ? 0 : 1;
			Error += Texture.size(1) == (1 * 1) * sizeof(glm::u8vec4) ? 0 : 1;
		}

		{
			gli::texture2d Texture(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, gli::texture2d::extent_type(3), 1);

			Error += Texture.size() == gli::block_size(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8) ? 0 : 1;
		}

		{
			gli::texture2d Texture(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, gli::texture2d::extent_type(9, 5), 1);

			gli::texture2d::size_type const CurrentSize = Texture.size();
			gli::texture2d::size_type const ExpectedSize = gli::block_size(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8) * 3 * 2;

			Error += CurrentSize == ExpectedSize ? 0 : 1;
		}

		return Error;
	}
}//namespace can_compute_npot_texture_size

int main()
{
	int Error = 0;

	Error += can_compute_npot_texture_size::main();
	Error += can_compute_texture_size::main();
	Error += can_compute_view_size::main();

	return Error;
}


