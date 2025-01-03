#include <gli/texture.hpp>
#include <gli/sampler2d.hpp>
#include <gli/levels.hpp>
#include <gli/comparison.hpp>
#include <gli/save.hpp>
#include <gli/load.hpp>
#include <gli/copy.hpp>
#include <gli/generate_mipmaps.hpp>
#include <glm/gtc/epsilon.hpp>
#include <ctime>
#include <memory>

namespace alloc
{
	int run()
	{
		int Error(0);

		std::vector<int> Sizes;
		Sizes.push_back(16);
		Sizes.push_back(32);
		Sizes.push_back(15);
		Sizes.push_back(17);
		Sizes.push_back(1);

		for(std::size_t TargetIndex = gli::TARGET_FIRST; TargetIndex <= gli::TARGET_LAST; ++TargetIndex)
		for(std::size_t FormatIndex = gli::FORMAT_FIRST; FormatIndex <= gli::FORMAT_LAST; ++FormatIndex)
		{
			gli::format const Format = static_cast<gli::format>(FormatIndex);
			gli::target const Target = static_cast<gli::target>(TargetIndex);
			gli::texture::size_type const Faces = gli::is_target_cube(Target) ? 6 : 1;

			if(gli::is_compressed(Format) && gli::is_target_1d(Target))
				continue;

			for(std::size_t SizeIndex = 0; SizeIndex < Sizes.size(); ++SizeIndex)
			{
				gli::texture::extent_type Size(Sizes[SizeIndex]);

				gli::texture TextureA(Target, Format, Size, 1, Faces, gli::levels(Size));
				gli::texture TextureB(Target, Format, Size, 1, Faces, gli::levels(Size));

				Error += TextureA == TextureB ? 0 : 1;
			}
		}

		return Error;
	}
}//namespace alloc

namespace clear
{
	int run()
	{
		int Error(0);

		glm::u8vec4 const Orange(255, 127, 0, 255);

		gli::texture::extent_type Size(16, 16, 1);
		gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, Size, 1, 1, gli::levels(Size));

		Texture.clear<glm::u8vec4>(Orange);

		return Error;
	}
}//namespace

namespace query
{
	int run()
	{
		int Error(0);

		gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGBA8_UINT_PACK8, gli::texture::extent_type(1), 1, 1, 1);

		Error += Texture.size() == sizeof(glm::u8vec4) * 1 ? 0 : 1;
		Error += Texture.format() == gli::FORMAT_RGBA8_UINT_PACK8 ? 0 : 1;
		Error += Texture.levels() == 1 ? 0 : 1;
		Error += !Texture.empty() ? 0 : 1;
		Error += Texture.extent() == gli::texture::extent_type(1) ? 0 : 1;

		return Error;
	}
}//namespace

namespace tex_access
{
	int run()
	{
		int Error(0);

		{
			gli::texture1d Texture(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture1d::extent_type(2), 2);
			GLI_ASSERT(!Texture.empty());

			gli::image Image0 = Texture[0];
			gli::image Image1 = Texture[1];

			std::size_t Size0 = Image0.size();
			std::size_t Size1 = Image1.size();

			Error += Size0 == sizeof(glm::u8vec4) * 2 ? 0 : 1;
			Error += Size1 == sizeof(glm::u8vec4) * 1 ? 0 : 1;

			*Image0.data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);
			*Image1.data<glm::u8vec4>() = glm::u8vec4(0, 127, 255, 255);

			glm::u8vec4 * PointerA = Image0.data<glm::u8vec4>();
			glm::u8vec4 * PointerB = Image1.data<glm::u8vec4>();

			glm::u8vec4 * Pointer0 = Texture.data<glm::u8vec4>() + 0;
			glm::u8vec4 * Pointer1 = Texture.data<glm::u8vec4>() + 2;

			Error += PointerA == Pointer0 ? 0 : 1;
			Error += PointerB == Pointer1 ? 0 : 1;

			glm::u8vec4 ColorA = *Image0.data<glm::u8vec4>();
			glm::u8vec4 ColorB = *Image1.data<glm::u8vec4>();

			glm::u8vec4 Color0 = *Pointer0;
			glm::u8vec4 Color1 = *Pointer1;

			Error += ColorA == Color0 ? 0 : 1;
			Error += ColorB == Color1 ? 0 : 1;

			Error += glm::all(glm::equal(Color0, glm::u8vec4(255, 127, 0, 255))) ? 0 : 1;
			Error += glm::all(glm::equal(Color1, glm::u8vec4(0, 127, 255, 255))) ? 0 : 1;
		}

		{
			gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGBA8_UINT_PACK8, gli::texture::extent_type(1), 1, 1, 1);

			std::size_t SizeA = Texture.size();
			Error += SizeA == sizeof(glm::u8vec4) * 1 ? 0 : 1;

			*Texture.data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

			glm::u8vec4 * Pointer0 = Texture.data<glm::u8vec4>() + 0;;
			glm::u8vec4 Color0 = *Pointer0;

			Error += glm::all(glm::equal(Color0, glm::u8vec4(255, 127, 0, 255))) ? 0 : 1;
		}

		return Error;
	}
}//namespace

namespace size
{
	struct test
	{
		test(
			gli::format const & Format,
			gli::texture::extent_type const & Dimensions,
			gli::texture::size_type const & Size) :
			Format(Format),
			Dimensions(Dimensions),
			Size(Size)
		{}

		gli::format Format;
		gli::texture::extent_type Dimensions;
		gli::texture::size_type Size;
	};

	int run()
	{
		int Error(0);

		std::vector<test> Tests;
		Tests.push_back(test(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture::extent_type(1), 4));
		Tests.push_back(test(gli::FORMAT_R8_UINT_PACK8, gli::texture::extent_type(1), 1));

		for(std::size_t i = 0; i < Tests.size(); ++i)
		{
			gli::texture Texture(
				gli::TARGET_2D,
				Tests[i].Format,
				gli::texture::extent_type(1),
				gli::texture::size_type(1),
				gli::texture::size_type(1),
				gli::texture::size_type(1));

			Error += Texture.size() == Tests[i].Size ? 0 : 1;
			GLI_ASSERT(!Error);
		}

		return Error;
	}
}//namespace size

namespace specialize
{
	int run()
	{
		int Error(0);

		gli::texture Texture(gli::TARGET_1D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(1), 1, 1, 1);
		gli::texture1d Texture1D(Texture);
		gli::texture1d_array Texture1DArray(Texture);
		gli::texture2d Texture2D(Texture);
		gli::texture2d_array Texture2DArray(Texture);
		gli::texture3d Texture3D(Texture);
		gli::texture_cube TextureCube(Texture);
		gli::texture_cube_array TextureCubeArray(Texture);

		Error += Texture == Texture1D ? 0 : 1;
		Error += Texture != Texture1DArray ? 0 : 1;
		Error += Texture != Texture2D ? 0 : 1;
		Error += Texture != Texture2DArray ? 0 : 1;
		Error += Texture != Texture3D ? 0 : 1;
		Error += Texture != TextureCube ? 0 : 1;
		Error += Texture != TextureCubeArray ? 0 : 1;

		gli::texture Texture1D_B(Texture1D);
		gli::texture Texture1DArray_B(Texture1DArray);
		gli::texture Texture2D_B(Texture2D);
		gli::texture Texture2DArray_B(Texture2DArray);
		gli::texture Texture3D_B(Texture3D);
		gli::texture TextureCube_B(TextureCube);
		gli::texture TextureCubeArray_B(TextureCubeArray);

		Error += Texture == Texture1D_B ? 0 : 1;
		Error += Texture != Texture1DArray_B ? 0 : 1;
		Error += Texture != Texture2D_B ? 0 : 1;
		Error += Texture != Texture2DArray_B ? 0 : 1;
		Error += Texture != Texture3D_B ? 0 : 1;
		Error += Texture != TextureCube_B ? 0 : 1;
		Error += Texture != TextureCubeArray_B ? 0 : 1;

		Error += Texture1D == Texture1D_B ? 0 : 1;
		Error += Texture1DArray == Texture1DArray_B ? 0 : 1;
		Error += Texture2D == Texture2D_B ? 0 : 1;
		Error += Texture2DArray == Texture2DArray_B ? 0 : 1;
		Error += Texture3D == Texture3D_B ? 0 : 1;
		Error += TextureCube == TextureCube_B ? 0 : 1;
		Error += TextureCubeArray == TextureCubeArray_B ? 0 : 1;

		return Error;
	}
}//namespace specialize

namespace load
{
	int run()
	{
		int Error = 0;

		// Texture 1D
		{
			gli::texture Texture(gli::TARGET_1D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(1), 1, 1, 1);
			Texture.clear(glm::u8vec4(225, 127, 0, 255));

			gli::save(Texture, "texture_1d.ktx");
			gli::save(Texture, "texture_1d.dds");
			gli::texture TextureKTX = gli::load("texture_1d.ktx");
			gli::texture TextureDDS = gli::load("texture_1d.dds");

			Error += Texture == TextureKTX ? 0 : 1;
			Error += Texture == TextureDDS ? 0 : 1;
		}

		// Texture 1D array
		{
			gli::texture Texture(gli::TARGET_1D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(1), 2, 1, 1);
			Texture.clear(glm::u8vec4(225, 127, 0, 255));
			gli::save(Texture, "texture_1d_array.ktx");
			gli::save(Texture, "texture_1d_array.dds");
			gli::texture TextureKTX = gli::load("texture_1d_array.ktx");
			gli::texture TextureDDS = gli::load("texture_1d_array.dds");

			Error += Texture == TextureKTX ? 0 : 1;
			Error += Texture == TextureDDS ? 0 : 1;
		}

		// Texture 2D
		{
			gli::texture Texture(gli::TARGET_2D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(1), 1, 1, 1);
			Texture.clear(glm::u8vec4(225, 127, 0, 255));

			gli::save(Texture, "texture_2d.ktx");
			gli::save(Texture, "texture_2d.dds");
			gli::texture TextureKTX = gli::load("texture_2d.ktx");
			gli::texture TextureDDS = gli::load("texture_2d.dds");

			Error += Texture == TextureKTX ? 0 : 1;
			Error += Texture == TextureDDS ? 0 : 1;
		}

		// Texture 2D array
		{
			gli::texture Texture(gli::TARGET_2D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(1), 2, 1, 1);
			Texture.clear(glm::u8vec4(225, 127, 0, 255));
			gli::save(Texture, "texture_2d_array.ktx");
			gli::save(Texture, "texture_2d_array.dds");
			gli::texture TextureKTX = gli::load("texture_2d_array.ktx");
			gli::texture TextureDDS = gli::load("texture_2d_array.dds");

			Error += Texture == TextureKTX ? 0 : 1;
			Error += Texture == TextureDDS ? 0 : 1;
		}

		// Texture 3D
		{
			gli::texture Texture(gli::TARGET_3D, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(1), 1, 1, 1);
			gli::save(Texture, "texture_3d.ktx");
			gli::save(Texture, "texture_3d.dds");
			gli::texture TextureKTX = gli::load("texture_3d.ktx");
			gli::texture TextureDDS = gli::load("texture_3d.dds");

			Error += Texture == TextureKTX ? 0 : 1;
			Error += Texture == TextureDDS ? 0 : 1;
		}

		// Texture cube
		{
			gli::texture Texture(gli::TARGET_CUBE, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(1), 1, 6, 1);
			Texture.clear(glm::u8vec4(225, 127, 0, 255));
			gli::save(Texture, "texture_cube.ktx");
			gli::save(Texture, "texture_cube.dds");
			gli::texture TextureKTX = gli::load("texture_cube.ktx");
			gli::texture TextureDDS = gli::load("texture_cube.dds");

			Error += Texture == TextureKTX ? 0 : 1;
			Error += Texture == TextureDDS ? 0 : 1;
		}

		// Texture cube array
		{
			gli::texture Texture(gli::TARGET_CUBE_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(1), 2, 6, 1);
			Texture.clear(glm::u8vec4(225, 127, 0, 255));
			gli::save(Texture, "texture_cube_array.ktx");
			gli::save(Texture, "texture_cube_array.dds");
			gli::texture TextureKTX = gli::load("texture_cube_array.ktx");
			gli::texture TextureDDS = gli::load("texture_cube_array.dds");

			Error += Texture == TextureKTX ? 0 : 1;
			Error += Texture == TextureDDS ? 0 : 1;
		}

		return Error;
	}
}//namespace load

namespace data
{
	int run()
	{
		int Error = 0;

		gli::texture Texture(gli::TARGET_2D_ARRAY, gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture::extent_type(1), 2, 1, 1);
		Error += gli::texture2d_array(Texture)[0].data() == Texture.data(0, 0, 0) ? 0 : 1;
		Error += gli::texture2d_array(Texture)[1].data() == Texture.data(1, 0, 0) ? 0 : 1;

		return Error;
	}
}//namespace data

namespace perf_generic_creation
{
	int main(std::size_t Iterations)
	{
		int Error = 0;

		std::clock_t TimeBegin = std::clock();

		for(std::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
		for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
		{
			gli::texture Texture(gli::TARGET_2D_ARRAY, static_cast<gli::format>(FormatIndex), gli::texture::extent_type(4, 4, 1), 1, 1, 3);
			Error += Texture.empty() ? 1 : 0;
		}

		std::clock_t TimeEnd = std::clock();
		printf("Generic texture creation performance test: %d\n", TimeEnd - TimeBegin);

		return Error;
	}
}//namespace perf_generic_creation

namespace perf_2d_array_creation
{
	int main(std::size_t Iterations)
	{
		int Error = 0;

		std::clock_t TimeBegin = std::clock();

		for(std::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
		for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
		{
			gli::texture2d_array Texture(static_cast<gli::format>(FormatIndex), gli::texture2d_array::extent_type(4), 1, 3);
			Error += Texture.empty() ? 1 : 0;
		}

		std::clock_t TimeEnd = std::clock();
		printf("2D array texture creation performance test: %d\n", TimeEnd - TimeBegin);

		return Error;
	}
}//namespace perf_2d_array_creation

namespace perf_2d_creation
{
	int main(std::size_t Iterations)
	{
		int Error = 0;

		std::clock_t TimeBegin = std::clock();

		for(std::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
		for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
		{
			gli::texture2d Texture(static_cast<gli::format>(FormatIndex), gli::texture2d::extent_type(4), 3);
			Error += Texture.empty() ? 1 : 0;
		}

		std::clock_t TimeEnd = std::clock();
		printf("2D texture creation performance test: %d\n", TimeEnd - TimeBegin);

		return Error;
	}
}//namespace perf_2d_creation

namespace perf_cube_array_creation
{
	int main(std::size_t Iterations)
	{
		int Error = 0;

		std::clock_t TimeBegin = std::clock();

		for(std::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
		for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
		{
			gli::texture_cube_array Texture(static_cast<gli::format>(FormatIndex), gli::texture2d_array::extent_type(4), 1, 3);
			Error += Texture.empty() ? 1 : 0;
		}

		std::clock_t TimeEnd = std::clock();
		printf("Cube array texture creation performance test: %d\n", TimeEnd - TimeBegin);

		return Error;
	}
}//namespace perf_cube_array_creation

namespace perf_cube_array_access
{
	int main(std::size_t Iterations)
	{
		int Error = 0;

		std::vector<std::shared_ptr<gli::texture_cube_array> > Textures(gli::FORMAT_COUNT);
		for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
		{
			Textures[FormatIndex].reset(new gli::texture_cube_array(static_cast<gli::format>(FormatIndex), gli::texture2d_array::extent_type(4), 3, 3));
			Error += Textures[FormatIndex]->empty() ? 1 : 0;
		}

		{
			std::clock_t TimeBegin = std::clock();

			for(gli::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
			for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
			{
				for(gli::size_t LayerIndex = 0, LayerCount = Textures[FormatIndex]->layers(); LayerIndex < LayerCount; ++LayerIndex)
				for(gli::size_t LevelIndex = 0, LevelCount = Textures[FormatIndex]->levels(); LevelIndex < LevelCount; ++LevelIndex)
				{
					void* BaseAddress = Textures[FormatIndex]->data(LayerIndex, 0, LevelIndex);
					Error += BaseAddress != nullptr ? 0 : 1;
				}
			}

			std::clock_t TimeEnd = std::clock();

			printf("Cube array texture data access performance test: %d\n", TimeEnd - TimeBegin);
		}

		{
			std::clock_t TimeBegin = std::clock();

			for(gli::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
			for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
			{
				for(gli::size_t LayerIndex = 0, LayerCount = Textures[FormatIndex]->layers(); LayerIndex < LayerCount; ++LayerIndex)
				for(gli::size_t LevelIndex = 0, LevelCount = Textures[FormatIndex]->levels(); LevelIndex < LevelCount; ++LevelIndex)
				{
					gli::size_t Size = Textures[FormatIndex]->size(LevelIndex);
					Error += Size != 0 ? 0 : 1;
				}
			}

			std::clock_t TimeEnd = std::clock();

			printf("Cube array texture size performance test: %d\n", TimeEnd - TimeBegin);
		}

		{
			std::clock_t TimeBegin = std::clock();

			for(gli::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
			for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
			{
				for(gli::size_t LayerIndex = 0, LayerCount = Textures[FormatIndex]->layers(); LayerIndex < LayerCount; ++LayerIndex)
				for(gli::size_t LevelIndex = 0, LevelCount = Textures[FormatIndex]->levels(); LevelIndex < LevelCount; ++LevelIndex)
				{
					gli::texture_cube_array::extent_type Extent = Textures[FormatIndex]->extent(LevelIndex);
					Error += Extent.x != 0 ? 0 : 1;
					Error += Extent.y != 0 ? 0 : 1;
				}
			}

			std::clock_t TimeEnd = std::clock();

			printf("Cube array texture extent access performance test: %d\n", TimeEnd - TimeBegin);
		}

		{
			std::clock_t TimeBegin = std::clock();

			for(gli::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
			for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
			{
				for(gli::size_t LayerIndex = 0, LayerCount = Textures[FormatIndex]->layers(); LayerIndex < LayerCount; ++LayerIndex)
				for(gli::size_t LevelIndex = 0, LevelCount = Textures[FormatIndex]->levels(); LevelIndex < LevelCount; ++LevelIndex)
				{
					gli::texture_cube_array::extent_type Extent = Textures[FormatIndex]->extent(LevelIndex);
					gli::size_t Size = Textures[FormatIndex]->size(LevelIndex);

					Error += Extent.x != 0 ? 0 : 1;
					Error += Extent.y != 0 ? 0 : 1;
					Error += Size != 0 ? 0 : 1;
				}
			}

			std::clock_t TimeEnd = std::clock();

			printf("Cube array texture extent and size access performance test: %d\n", TimeEnd - TimeBegin);
		}

		{
			std::clock_t TimeBegin = std::clock();

			for(gli::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
			for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
			{
				for(gli::size_t LayerIndex = 0, LayerCount = Textures[FormatIndex]->layers(); LayerIndex < LayerCount; ++LayerIndex)
				for(gli::size_t LevelIndex = 0, LevelCount = Textures[FormatIndex]->levels(); LevelIndex < LevelCount; ++LevelIndex)
				{
					gli::texture_cube_array::extent_type Extent = Textures[FormatIndex]->extent(LevelIndex);
					gli::size_t Size = Textures[FormatIndex]->size(LevelIndex);
					void* BaseAddress = Textures[FormatIndex]->data(LayerIndex, 0, LevelIndex);

					Error += Extent.x != 0 ? 0 : 1;
					Error += Extent.y != 0 ? 0 : 1;
					Error += Size != 0 ? 0 : 1;
					Error += BaseAddress != nullptr ? 0 : 1;
				}
			}

			std::clock_t TimeEnd = std::clock();

			printf("Cube array texture all access performance test: %d\n", TimeEnd - TimeBegin);
		}

		return Error;
	}
}//namespace perf_cube_array_access

namespace perf_texture2d_access
{
	int main(std::size_t Iterations)
	{
		int Error = 0;

		std::vector<std::shared_ptr<gli::texture2d> > Textures(gli::FORMAT_COUNT);
		for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
		{
			Textures[FormatIndex].reset(new gli::texture2d(static_cast<gli::format>(FormatIndex), gli::texture2d::extent_type(4), 9));
			Error += Textures[FormatIndex]->empty() ? 1 : 0;
		}

		{
			std::clock_t TimeBegin = std::clock();

			for(gli::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
			for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
			{
				for(gli::size_t LayerIndex = 0, LayerCount = Textures[FormatIndex]->layers(); LayerIndex < LayerCount; ++LayerIndex)
				for(gli::size_t LevelIndex = 0, LevelCount = Textures[FormatIndex]->levels(); LevelIndex < LevelCount; ++LevelIndex)
				{
					void* BaseAddress = Textures[FormatIndex]->data(LayerIndex, 0, LevelIndex);
					Error += BaseAddress != nullptr ? 0 : 1;
				}
			}

			std::clock_t TimeEnd = std::clock();

			printf("2d texture data access performance test: %d\n", TimeEnd - TimeBegin);
		}

		{
			std::clock_t TimeBegin = std::clock();

			for(gli::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
			for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
			{
				for(gli::size_t LayerIndex = 0, LayerCount = Textures[FormatIndex]->layers(); LayerIndex < LayerCount; ++LayerIndex)
				for(gli::size_t LevelIndex = 0, LevelCount = Textures[FormatIndex]->levels(); LevelIndex < LevelCount; ++LevelIndex)
				{
					gli::size_t Size = Textures[FormatIndex]->size(LevelIndex);
					Error += Size != 0 ? 0 : 1;
				}
			}

			std::clock_t TimeEnd = std::clock();

			printf("2d texture size performance test: %d\n", TimeEnd - TimeBegin);
		}

		{
			std::clock_t TimeBegin = std::clock();

			for(gli::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
			for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
			{
				for(gli::size_t LayerIndex = 0, LayerCount = Textures[FormatIndex]->layers(); LayerIndex < LayerCount; ++LayerIndex)
				for(gli::size_t LevelIndex = 0, LevelCount = Textures[FormatIndex]->levels(); LevelIndex < LevelCount; ++LevelIndex)
				{
					gli::texture2d::extent_type Extent = Textures[FormatIndex]->extent(LevelIndex);
					Error += Extent.x != 0 ? 0 : 1;
					Error += Extent.y != 0 ? 0 : 1;
				}
			}

			std::clock_t TimeEnd = std::clock();

			printf("2d texture extent access performance test: %d\n", TimeEnd - TimeBegin);
		}

		{
			std::clock_t TimeBegin = std::clock();

			for(gli::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
			for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
			{
				for(gli::size_t LayerIndex = 0, LayerCount = Textures[FormatIndex]->layers(); LayerIndex < LayerCount; ++LayerIndex)
				for(gli::size_t LevelIndex = 0, LevelCount = Textures[FormatIndex]->levels(); LevelIndex < LevelCount; ++LevelIndex)
				{
					gli::texture2d::extent_type Extent = Textures[FormatIndex]->extent(LevelIndex);
					gli::size_t Size = Textures[FormatIndex]->size(LevelIndex);

					Error += Extent.x != 0 ? 0 : 1;
					Error += Extent.y != 0 ? 0 : 1;
					Error += Size != 0 ? 0 : 1;
				}
			}

			std::clock_t TimeEnd = std::clock();

			printf("2d texture extent and size access performance test: %d\n", TimeEnd - TimeBegin);
		}

		{
			std::clock_t TimeBegin = std::clock();

			for(gli::size_t Index = 0, Count = Iterations; Index < Count; ++Index)
			for(std::size_t FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_COUNT; FormatIndex < FormatCount; ++FormatIndex)
			{
				for(gli::size_t LayerIndex = 0, LayerCount = Textures[FormatIndex]->layers(); LayerIndex < LayerCount; ++LayerIndex)
				for(gli::size_t LevelIndex = 0, LevelCount = Textures[FormatIndex]->levels(); LevelIndex < LevelCount; ++LevelIndex)
				{
					gli::texture2d::extent_type Extent = Textures[FormatIndex]->extent(LevelIndex);
					gli::size_t Size = Textures[FormatIndex]->size(LevelIndex);
					void* BaseAddress = Textures[FormatIndex]->data(LayerIndex, 0, LevelIndex);

					Error += Extent.x != 0 ? 0 : 1;
					Error += Extent.y != 0 ? 0 : 1;
					Error += Size != 0 ? 0 : 1;
					Error += BaseAddress != nullptr ? 0 : 1;
				}
			}

			std::clock_t TimeEnd = std::clock();

			printf("2d texture all access performance test: %d\n", TimeEnd - TimeBegin);
		}

		return Error;
	}
}//namespace perf_texture2d_access

namespace perf_texture_load
{
	int main(int Extent)
	{
		int Error = 0;

		gli::texture2d Texture(gli::FORMAT_R8_UNORM_PACK8, gli::texture2d::extent_type(Extent));
		Texture.clear(gli::u8(255));

		std::clock_t TimeBegin = std::clock();

		for(gli::texture2d::size_type LevelIndex = 0, LevelCount = Texture.levels(); LevelIndex < LevelCount; ++LevelIndex)
		{
			gli::texture2d::extent_type const Extent = Texture.extent(LevelIndex);
			for(gli::size_t y = 0; y < Extent.y; ++y)
			for(gli::size_t x = 0; x < Extent.x; ++x)
			{
				gli::u8 Texel = Texture.load<gli::u8>(gli::texture2d::extent_type(x, y), LevelIndex);
				Error += Texel == gli::u8(255) ? 0 : 1;
			}
		}

		std::clock_t TimeEnd = std::clock();
		printf("2D texture load performance test: %d\n", TimeEnd - TimeBegin);

		return Error;
	}
}//namespace perf_texture_load

namespace perf_texture_fetch
{
	int main(int Extent)
	{
		int Error = 0;

		gli::texture2d Texture(gli::FORMAT_R8_UNORM_PACK8, gli::texture2d::extent_type(Extent));
		Texture.clear(gli::u8(255));

		gli::sampler2d<float> Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE);

		std::clock_t TimeBegin = std::clock();

		for(gli::texture2d::size_type LevelIndex = 0, LevelCount = Texture.levels(); LevelIndex < LevelCount; ++LevelIndex)
		{
			gli::texture2d::extent_type const Extent = Texture.extent(LevelIndex);
			for(gli::size_t y = 0; y < Extent.y; ++y)
			for(gli::size_t x = 0; x < Extent.x; ++x)
			{
				gli::vec4 const& Texel = Sampler.texel_fetch(gli::texture2d::extent_type(x, y), LevelIndex);
				Error += gli::all(gli::epsilonEqual(Texel, gli::vec4(1, 0, 0, 1), 0.001f)) ? 0 : 1;
				assert(!Error);
			}
		}

		std::clock_t TimeEnd = std::clock();
		printf("2D texture fetch performance test: %d\n", TimeEnd - TimeBegin);

		return Error;
	}
}//namespace perf_texture_fetch

namespace perf_texture_lod_nearest
{
	int main(int Extent)
	{
		int Error = 0;

		gli::texture2d Texture(gli::FORMAT_R8_UNORM_PACK8, gli::texture2d::extent_type(Extent));
		Texture.clear(gli::u8(255));

		gli::sampler2d<float> Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_NEAREST, gli::FILTER_NEAREST);

		std::clock_t TimeBegin = std::clock();

		for(gli::texture2d::size_type LevelIndex = 0, LevelCount = Texture.levels(); LevelIndex < LevelCount; ++LevelIndex)
		{
			gli::texture2d::extent_type const Extent = Texture.extent(LevelIndex);
			for(gli::size_t y = 0; y < Extent.y; ++y)
			for(gli::size_t x = 0; x < Extent.x; ++x)
			{
				gli::vec4 const& Texel = Sampler.texture_lod(glm::vec2(x, y) / glm::vec2(Extent), static_cast<float>(LevelIndex));
				Error += gli::all(gli::epsilonEqual(Texel, gli::vec4(1, 0, 0, 1), 0.001f)) ? 0 : 1;
			}
		}

		std::clock_t TimeEnd = std::clock();
		printf("2D texture lod nearest performance test: %d\n", TimeEnd - TimeBegin);

		return Error;
	}
}//namespace perf_texture_lod_nearest

namespace perf_texture_lod_linear
{
	int main(int Extent)
	{
		int Error = 0;

		gli::texture2d Texture(gli::FORMAT_R8_UNORM_PACK8, gli::texture2d::extent_type(Extent));
		Texture.clear(gli::u8(255));

		gli::sampler2d<float> Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_NEAREST, gli::FILTER_LINEAR);

		std::clock_t TimeBegin = std::clock();

		for(gli::texture2d::size_type LevelIndex = 0, LevelCount = Texture.levels(); LevelIndex < LevelCount; ++LevelIndex)
		{
			gli::texture2d::extent_type const Extent = Texture.extent(LevelIndex);
			for(gli::size_t y = 0; y < Extent.y; ++y)
			for(gli::size_t x = 0; x < Extent.x; ++x)
			{
				gli::vec4 const& Texel = Sampler.texture_lod(glm::vec2(x, y) / glm::vec2(Extent), static_cast<float>(LevelIndex));
				Error += gli::all(gli::epsilonEqual(Texel, gli::vec4(1, 0, 0, 1), 0.001f)) ? 0 : 1;
			}
		}

		std::clock_t TimeEnd = std::clock();
		printf("2D texture lod linear performance test: %d\n", TimeEnd - TimeBegin);

		return Error;
	}
}//namespace perf_texture_lod_linear

namespace perf_generate_mipmaps_nearest
{
	int main(int Extent)
	{
		int Error = 0;

		gli::texture2d TextureSource(gli::FORMAT_R8_UNORM_PACK8, gli::texture2d::extent_type(Extent));
		TextureSource.clear(gli::u8(255));

		std::clock_t TimeBegin = std::clock();

		gli::texture2d TextureMipmaps = gli::generate_mipmaps(TextureSource, gli::FILTER_NEAREST);
		Error = *TextureMipmaps.data<glm::u8>(0, 0, TextureMipmaps.max_level()) == gli::u8(255) ? 0 : 1;

		std::clock_t TimeEnd = std::clock();

		printf("2D texture generate mipmaps nearest performance test: %d\n", TimeEnd - TimeBegin);

		return Error;
	}
}//namespace perf_generate_mipmaps_nearest

namespace perf_generate_mipmaps_linear
{
	int main(int Extent)
	{
		int Error = 0;

		gli::texture2d TextureSource(gli::FORMAT_R8_UNORM_PACK8, gli::texture2d::extent_type(Extent));
		TextureSource.clear(gli::u8(255));

		std::clock_t TimeBegin = std::clock();

		gli::texture2d TextureMipmaps = gli::generate_mipmaps(TextureSource, gli::FILTER_LINEAR);
		Error = *TextureMipmaps.data<glm::u8>(0, 0, TextureMipmaps.max_level()) == gli::u8(255) ? 0 : 1;

		std::clock_t TimeEnd = std::clock();

		printf("2D texture generate mipmaps linear performance test: %d\n", TimeEnd - TimeBegin);

		return Error;
	}
}//namespace perf_generate_mipmaps_linear

int main()
{
	int Error = 0;

	bool const DO_PERF_TEST = false;

	std::size_t const PERF_TEST_ACCESS_ITERATION = DO_PERF_TEST ? 100000 : 0;
	std::size_t const PERF_TEST_CREATION_ITERATION = DO_PERF_TEST ? 1000 : 0;

	Error += perf_texture_load::main(DO_PERF_TEST ? 8192 : 1024);
	Error += perf_texture_fetch::main(DO_PERF_TEST ? 8192 : 1024);
	Error += perf_texture_lod_nearest::main(DO_PERF_TEST ? 8192 : 1024);
	Error += perf_texture_lod_linear::main(DO_PERF_TEST ? 8192 : 1024);
	Error += perf_generate_mipmaps_nearest::main(DO_PERF_TEST ? 8192 : 1024);
	Error += perf_generate_mipmaps_linear::main(DO_PERF_TEST ? 8192 : 1024);
	Error += perf_texture2d_access::main(PERF_TEST_ACCESS_ITERATION);
	Error += perf_cube_array_access::main(PERF_TEST_ACCESS_ITERATION);
	Error += perf_generic_creation::main(PERF_TEST_CREATION_ITERATION);
	Error += perf_2d_array_creation::main(PERF_TEST_CREATION_ITERATION);
	Error += perf_2d_creation::main(PERF_TEST_CREATION_ITERATION);
	Error += perf_cube_array_creation::main(PERF_TEST_CREATION_ITERATION);

	Error += alloc::run();
	Error += size::run();
	Error += query::run();
	Error += clear::run();
	Error += tex_access::run();
	Error += specialize::run();
	Error += load::run();
	Error += data::run();

	return Error;
}

