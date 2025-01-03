#include <gli/comparison.hpp>
#include <gli/type.hpp>
#include <gli/view.hpp>
#include <gli/duplicate.hpp>
#include <gli/generate_mipmaps.hpp>

#include <glm/gtc/epsilon.hpp>

namespace generate_mipmaps
{
	template <typename genType>
	int test(gli::format Format, genType const & Black, genType const & Color, std::size_t Size, gli::filter Filter)
	{
		int Error = 0;

		gli::texture1d Texture(Format, gli::texture1d::extent_type(static_cast<gli::texture1d::extent_type::value_type>(Size)));
		Texture.clear(Black);
		Texture[0].clear(Color);

		genType const LoadC = Texture.load<genType>(gli::texture1d::extent_type(0), Texture.max_level());
		if(Texture.levels() > 1)
			Error += LoadC == Black ? 0 : 1;

		gli::texture1d TextureView(gli::view(Texture, 0, 0));
		gli::fsampler1D SamplerA(gli::texture1d(gli::duplicate(Texture)), gli::WRAP_CLAMP_TO_EDGE);
		SamplerA.generate_mipmaps(gli::FILTER_LINEAR);

		gli::texture1d MipmapsA = SamplerA();
		genType const LoadA = MipmapsA.load<genType>(gli::texture1d::extent_type(0), MipmapsA.max_level());
		Error += LoadA == Color ? 0 : 1;
		if(Texture.levels() > 1)
			Error += LoadA != LoadC ? 0 : 1;

		gli::texture1d MipmapViewA(gli::view(MipmapsA, 0, 0));
		Error += TextureView == MipmapViewA ? 0 : 1;

		// Mipmaps generation using the wrapper function
		gli::texture1d MipmapsB = gli::generate_mipmaps(gli::texture1d(gli::duplicate(Texture)), Filter);
		genType const LoadB = MipmapsB.load<genType>(gli::texture1d::extent_type(0), MipmapsB.max_level());
		Error += LoadB == Color ? 0 : 1;
		if(Texture.levels() > 1)
			Error += LoadB != LoadC ? 0 : 1;

		gli::texture1d MipmapViewB(gli::view(MipmapsB, 0, 0));
		Error += TextureView == MipmapViewB ? 0 : 1;

		Error += LoadA == LoadB ? 0 : 1;
		
		// Check levels
		Error += MipmapsA.max_level() == MipmapsB.max_level() ? 0 : 1;

		for(std::size_t i = 0; i < MipmapsA.max_level(); ++i)
		{
			genType const Load0 = MipmapsA.load<genType>(gli::texture1d::extent_type(0), i);
			genType const Load1 = MipmapsB.load<genType>(gli::texture1d::extent_type(0), i);
			Error += Load0 == Load1 ? 0 : 1;
		}

		// Compare custom mipmaps generation and wrapper mipmaps generation
		Error += MipmapViewA == MipmapViewB ? 0 : 1;

		return Error;
	}
}//namespace generate_mipmaps

int main()
{
	int Error = 0;

	std::vector<gli::filter> Filters;
	Filters.push_back(gli::FILTER_NEAREST);
	Filters.push_back(gli::FILTER_LINEAR);

	std::vector<gli::size_t> Sizes;
	Sizes.push_back(1);
	Sizes.push_back(2);
	Sizes.push_back(3);
	Sizes.push_back(15);
	Sizes.push_back(16);
	Sizes.push_back(17);
	Sizes.push_back(24);
	Sizes.push_back(32);

	for(std::size_t FilterIndex = 0, FilterCount = Filters.size(); FilterIndex < FilterCount; ++FilterIndex)
	for(std::size_t SizeIndex = 0, SizeCount = Sizes.size(); SizeIndex < SizeCount; ++SizeIndex)
	{
		Error += generate_mipmaps::test(gli::FORMAT_R16_SFLOAT_PACK16,
			gli::packHalf(glm::vec1(0.0f)),
			gli::packHalf(glm::vec1(1.0f)),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RG16_SFLOAT_PACK16,
			gli::packHalf(glm::vec2(0.0f, 0.0f)),
			gli::packHalf(glm::vec2(1.0f, 0.5f)),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RGB16_SFLOAT_PACK16,
			gli::packHalf(glm::vec3(0.0f, 0.0f, 0.0f)),
			gli::packHalf(glm::vec3(1.0f, 0.5f, 0.0f)),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RGBA16_SFLOAT_PACK16,
			gli::packHalf(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)),
			gli::packHalf(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_R32_SFLOAT_PACK32,
			glm::vec1(0.0f),
			glm::vec1(1.0f),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RG32_SFLOAT_PACK32,
			glm::vec2(0.0f, 0.0f),
			glm::vec2(1.0f, 0.5f),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RGB32_SFLOAT_PACK32,
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 0.5f, 0.0f),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RGBA32_SFLOAT_PACK32,
			glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
			glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RGBA4_UNORM_PACK16,
			gli::packUnorm4x4(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)),
			gli::packUnorm4x4(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RGBA8_UNORM_PACK8,
			glm::u8vec4(0, 0, 0, 0),
			glm::u8vec4(255, 127, 0, 255),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RGBA8_SNORM_PACK8,
			glm::i8vec4(0, 0, 0, 0),
			glm::i8vec4(127, 63, 0, 1),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RGB10A2_UNORM_PACK32,
			gli::packUnorm3x10_1x2(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)),
			gli::packUnorm3x10_1x2(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RGB10A2_SNORM_PACK32,
			gli::packSnorm3x10_1x2(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)),
			gli::packSnorm3x10_1x2(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)),
			Sizes[SizeIndex], Filters[FilterIndex]);

		Error += generate_mipmaps::test(gli::FORMAT_RGB9E5_UFLOAT_PACK32,
			gli::packF3x9_E1x5(glm::vec3(0.0f, 0.0f, 0.0f)),
			gli::packF3x9_E1x5(glm::vec3(1.0f, 0.5f, 0.0f)),
			Sizes[SizeIndex], Filters[FilterIndex]);
	}

	return Error;
}

