#include <gli/sampler2d.hpp>
#include <gli/comparison.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtc/epsilon.hpp>
#include <ctime>
#include <limits>
#include <array>

namespace wrap_clamp_to_border
{
	int test()
	{
		int Error(0);

		glm::vec4 const Orange(1.0f, 0.5f, 0.0f, 1.0f);
		glm::vec4 const Blue(0.0f, 0.5f, 1.0f, 1.0f);

		gli::texture2d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32), 1);
		Texture.clear(glm::packUnorm4x8(Orange));

		gli::fsampler2D SamplerA(Texture, gli::WRAP_CLAMP_TO_BORDER, gli::FILTER_LINEAR, gli::FILTER_LINEAR, Blue);

		{
			std::array<gli::fsampler2D::normalized_type, 8> SampleCoord{
			{
				gli::fsampler2D::normalized_type( 0.5f, 0.5f),
				gli::fsampler2D::normalized_type( 0.1f, 0.1f),
				gli::fsampler2D::normalized_type( 0.5f, 0.1f),
				gli::fsampler2D::normalized_type( 0.1f, 0.5f),
				gli::fsampler2D::normalized_type( 0.5f, 0.9f),
				gli::fsampler2D::normalized_type( 0.9f, 0.9f),
				gli::fsampler2D::normalized_type( 0.9f, 0.5f),
				gli::fsampler2D::normalized_type( 0.1f, 0.9f)
			}};

			for(std::size_t i = 0, n = SampleCoord.size(); i < n; ++i)
			{
				gli::vec4 const Texel = SamplerA.texture_lod(SampleCoord[i], 0.0f);
				Error += glm::all(glm::epsilonEqual(Texel, Orange, 0.01f)) ? 0 : 1;
			}
		}
		{
			std::array<gli::fsampler2D::normalized_type, 8> SampleCoord{
			{
				gli::fsampler2D::normalized_type( 0.5f,-0.5f),
				gli::fsampler2D::normalized_type(-0.5f,-0.5f),
				gli::fsampler2D::normalized_type(-0.5f, 0.5f),
				gli::fsampler2D::normalized_type( 1.5f, 0.5f),
				gli::fsampler2D::normalized_type( 1.5f, 1.5f),
				gli::fsampler2D::normalized_type( 0.5f, 1.5f),
				gli::fsampler2D::normalized_type( 1.5f,-0.5f),
				gli::fsampler2D::normalized_type(-0.5f, 1.5f)
			}};

			for(std::size_t i = 0, n = SampleCoord.size(); i < n; ++i)
			{
				gli::vec4 const Texel = SamplerA.texture_lod(SampleCoord[i], 0.0f);
				Error += glm::all(glm::epsilonEqual(Texel, Blue, 0.01f)) ? 0 : 1;
			}
		}

		return Error;
	}
}//namespace wrap_clamp_to_border

namespace wrap_mirror
{
	int test()
	{
		int Error(0);

		glm::vec4 const Orange(1.0f, 0.5f, 0.0f, 1.0f);
		glm::vec4 const Blue(0.0f, 0.5f, 1.0f, 1.0f);

		gli::texture2d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32), 1);
		Texture.clear(glm::packUnorm4x8(Orange));

		gli::fsampler2D Sampler(Texture, gli::WRAP_REPEAT, gli::FILTER_LINEAR, gli::FILTER_LINEAR, Blue);

		{
			std::array<gli::fsampler2D::normalized_type, 16> SampleCoord{
			{
				gli::fsampler2D::normalized_type( 0.5f, 0.5f),
				gli::fsampler2D::normalized_type( 0.1f, 0.1f),
				gli::fsampler2D::normalized_type( 0.5f, 0.1f),
				gli::fsampler2D::normalized_type( 0.1f, 0.5f),
				gli::fsampler2D::normalized_type( 0.5f, 0.9f),
				gli::fsampler2D::normalized_type( 0.9f, 0.9f),
				gli::fsampler2D::normalized_type( 0.9f, 0.5f),
				gli::fsampler2D::normalized_type( 0.1f, 0.9f),
				gli::fsampler2D::normalized_type( 0.5f,-0.5f),
				gli::fsampler2D::normalized_type(-0.5f,-0.5f),
				gli::fsampler2D::normalized_type(-0.5f, 0.5f),
				gli::fsampler2D::normalized_type( 1.5f, 0.5f),
				gli::fsampler2D::normalized_type( 1.5f, 1.5f),
				gli::fsampler2D::normalized_type( 0.5f, 1.5f),
				gli::fsampler2D::normalized_type( 1.5f,-0.5f),
				gli::fsampler2D::normalized_type(-0.5f, 1.5f)
			}};

			for(std::size_t i = 0, n = SampleCoord.size(); i < n; ++i)
			{
				gli::vec4 const Texel = Sampler.texture_lod(SampleCoord[i], 0.0f);
				Error += glm::all(glm::epsilonEqual(Texel, Orange, 0.01f)) ? 0 : 1;
			}
		}

		return Error;
	}
}//namespace wrap_mirror

int main()
{
	int Error(0);

	Error += wrap_clamp_to_border::test();
	Error += wrap_mirror::test();

	return Error;
}

