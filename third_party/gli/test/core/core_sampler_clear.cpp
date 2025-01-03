#include <gli/sampler2d.hpp>
#include <gli/comparison.hpp>
#include <glm/gtc/epsilon.hpp>

namespace rgba8unorm
{
	int test()
	{
		int Error = 0;

		glm::vec4 const Orange(1.0f, 0.5f, 0.0f, 1.0f);

		gli::texture2d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32), 1);
		gli::fsampler2D Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR);
		Sampler.clear(Orange);

		glm::u8vec4 const Texel = Texture.load<glm::u8vec4>(gli::extent2d(7), 0);

		Error += Texel == glm::u8vec4(255, 127, 0, 255) ? 0 : 1;

		return Error;
	}
}//namespace rgba8unorm

namespace rgba32sf
{
	int test()
	{
		int Error = 0;

		glm::f32vec4 const Orange(1.0f, 0.5f, 0.0f, 1.0f);

		gli::texture2d Texture(gli::FORMAT_RGBA32_SFLOAT_PACK32, gli::texture2d::extent_type(4), 1);
		gli::fsampler2D Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR);
		Sampler.clear(Orange);

		gli::f32vec4 const Texel0 = Texture.load<gli::f32vec4>(gli::extent2d(0), 0);
		gli::f32vec4 const Texel1 = Texture.load<gli::f32vec4>(gli::extent2d(2), 0);

		Error += glm::all(glm::epsilonEqual(Texel0, Orange, 0.01f)) ? 0 : 1;
		Error += glm::all(glm::epsilonEqual(Texel1, Orange, 0.01f)) ? 0 : 1;

		return Error;
	}
}//namespace rgba32sf

int main()
{
	int Error(0);

	Error += rgba32sf::test();
	Error += rgba8unorm::test();

	return Error;
}

