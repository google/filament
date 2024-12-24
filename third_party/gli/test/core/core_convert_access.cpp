#include <gli/sampler2d.hpp>
#include <glm/gtc/epsilon.hpp>

template<typename textureType>
struct texture
{
	typedef gli::detail::accessFunc<textureType, gli::u8vec1> u8vec1access;
	typedef gli::detail::accessFunc<textureType, gli::u8vec2> u8vec2access;
	typedef gli::detail::accessFunc<textureType, gli::u8vec3> u8vec3access;
	typedef gli::detail::accessFunc<textureType, gli::u8vec4> u8vec4access;

	static int test()
	{
		int Error = 0;

		{
			textureType Texture(gli::FORMAT_R8_UNORM_PACK8, typename textureType::extent_type(1), 1);

			gli::u8vec1 const Color(127);
			u8vec1access::store(Texture, typename textureType::extent_type(0), 0, 0, 0, Color);
			gli::u8vec1 const Texel = u8vec1access::load(Texture, typename textureType::extent_type(0), 0, 0, 0);

			Error += Color == Texel ? 0 : 1;
		}

		{
			textureType Texture(gli::FORMAT_RG8_UNORM_PACK8, typename textureType::extent_type(1), 1);

			gli::u8vec2 const Color(255, 127);
			u8vec2access::store(Texture, typename textureType::extent_type(0), 0, 0, 0, Color);
			gli::u8vec2 const Texel = u8vec2access::load(Texture, typename textureType::extent_type(0), 0, 0, 0);

			Error += Color == Texel ? 0 : 1;
		}

		{
			textureType Texture(gli::FORMAT_RGB8_UNORM_PACK8, typename textureType::extent_type(1), 1);

			gli::u8vec3 const Color(255, 127, 0);
			u8vec3access::store(Texture, typename textureType::extent_type(0), 0, 0, 0, Color);
			gli::u8vec3 const Texel = u8vec3access::load(Texture, typename textureType::extent_type(0), 0, 0, 0);

			Error += Color == Texel ? 0 : 1;
		}

		{
			textureType Texture(gli::FORMAT_RGBA8_UNORM_PACK8, typename textureType::extent_type(1), 1);

			gli::u8vec4 const Color(255, 127, 0, 255);
			u8vec4access::store(Texture, typename textureType::extent_type(0), 0, 0, 0, Color);
			gli::u8vec4 const Texel = u8vec4access::load(Texture, typename textureType::extent_type(0), 0, 0, 0);

			Error += Color == Texel ? 0 : 1;
		}

		return Error;
	}
};

int main()
{
	int Error = 0;

	Error += texture<gli::texture1d>::test();
	Error += texture<gli::texture1d_array>::test();
	Error += texture<gli::texture2d>::test();
	Error += texture<gli::texture2d_array>::test();
	Error += texture<gli::texture3d>::test();
	Error += texture<gli::texture_cube>::test();
	Error += texture<gli::texture_cube_array>::test();

	return Error;
}

