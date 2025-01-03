#include <gli/sampler1d.hpp>
#include <glm/gtc/epsilon.hpp>

namespace filter1d
{
	int test()
	{
		int Error = 0;

		gli::vec4 const ColorFill(1.0f, 0.5f, 0.0f, 1.0f);
		gli::vec4 const ColorBorder(0.0f, 0.5f, 1.0f, 1.0f);

		gli::texture1d Texture(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(2), 1);
		Texture.clear(glm::packUnorm<gli::u8>(ColorFill));

		{
			gli::fsampler1D Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_LINEAR, gli::FILTER_LINEAR, ColorBorder);

			gli::vec4 const TexelA = Sampler.texture_lod(gli::fsampler1D::normalized_type(+0.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelA, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelB = Sampler.texture_lod(gli::fsampler1D::normalized_type(-0.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelB, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelC = Sampler.texture_lod(gli::fsampler1D::normalized_type(+1.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelC, ColorFill, 0.01f)) ? 0 : 1;
		}
		{
			gli::fsampler1D Sampler(Texture, gli::WRAP_CLAMP_TO_BORDER, gli::FILTER_LINEAR, gli::FILTER_LINEAR, ColorBorder);

			gli::vec4 const TexelA = Sampler.texture_lod(gli::fsampler1D::normalized_type(+0.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelA, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelB = Sampler.texture_lod(gli::fsampler1D::normalized_type(-1.0f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelB, ColorBorder, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelC = Sampler.texture_lod(gli::fsampler1D::normalized_type(+2.0f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelC, ColorBorder, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelD = Sampler.texture_lod(gli::fsampler1D::normalized_type(-0.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelD, (ColorFill + ColorBorder) * 0.5f, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelE = Sampler.texture_lod(gli::fsampler1D::normalized_type(1.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelE, (ColorFill + ColorBorder) * 0.5f, 0.01f)) ? 0 : 1;
		}
		{
			gli::fsampler1D Sampler(Texture, gli::WRAP_CLAMP_TO_EDGE, gli::FILTER_NEAREST, gli::FILTER_NEAREST, ColorBorder);
			gli::vec4 const TexelA = Sampler.texture_lod(gli::fsampler1D::normalized_type(+0.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelA, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelB = Sampler.texture_lod(gli::fsampler1D::normalized_type(-0.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelB, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelC = Sampler.texture_lod(gli::fsampler1D::normalized_type(+1.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelC, ColorFill, 0.01f)) ? 0 : 1;
		}
		{
			gli::fsampler1D Sampler(Texture, gli::WRAP_CLAMP_TO_BORDER, gli::FILTER_NEAREST, gli::FILTER_NEAREST, ColorBorder);

			gli::vec4 const TexelA = Sampler.texture_lod(gli::fsampler1D::normalized_type(0.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelA, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelB = Sampler.texture_lod(gli::fsampler1D::normalized_type(-0.4f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelB, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelC = Sampler.texture_lod(gli::fsampler1D::normalized_type(-0.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelC, ColorBorder, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelD = Sampler.texture_lod(gli::fsampler1D::normalized_type(-0.6f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelD, ColorBorder, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelE = Sampler.texture_lod(gli::fsampler1D::normalized_type(1.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelE, ColorBorder, 0.01f)) ? 0 : 1;
		}

		{
			gli::fsampler1D Sampler(Texture, gli::WRAP_MIRROR_REPEAT, gli::FILTER_NEAREST, gli::FILTER_NEAREST, ColorBorder);

			gli::vec4 const TexelA = Sampler.texture_lod(gli::fsampler1D::normalized_type(0.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelA, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelB = Sampler.texture_lod(gli::fsampler1D::normalized_type(-0.4f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelB, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelC = Sampler.texture_lod(gli::fsampler1D::normalized_type(-0.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelC, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelD = Sampler.texture_lod(gli::fsampler1D::normalized_type(-0.6f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelD, ColorFill, 0.01f)) ? 0 : 1;

			gli::vec4 const TexelE = Sampler.texture_lod(gli::fsampler1D::normalized_type(1.5f), 0.0f);
			Error += gli::all(gli::epsilonEqual(TexelE, ColorFill, 0.01f)) ? 0 : 1;
		}

		return Error;
	}
}//namespace filter1d

int main()
{
	int Error(0);

	Error += filter1d::test();

	return Error;
}

