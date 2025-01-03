#include <gli/gli.hpp>

static_assert(sizeof(gli::detail::dxt1_block) == 8, "DXT1-compressed block must be of size 8.");
static_assert(sizeof(gli::detail::dxt3_block) == 16, "DXT3-compressed block must be of size 16.");
static_assert(sizeof(gli::detail::dxt5_block) == 16, "DXT5-compressed block must be of size 16.");

template <typename texture, typename genType>
int test_texture
(
	texture const & Texture,
	genType const & ClearColor,
	genType const & FirstColor
)
{
	int Error = 0;

	texture TextureA(gli::duplicate(Texture));
	TextureA.clear();
	*TextureA.template data<genType>() = FirstColor;

	texture TextureB = gli::flip(TextureA);
	Error += TextureA != TextureB ? 0 : 1;

	texture TextureC = gli::flip(TextureB);
	Error += TextureC == TextureA ? 0 : 1;

	return Error;
}

int main()
{
	int Error = 0;

	gli::texture2d::extent_type const TextureSize(32);
	gli::size_t const Levels = gli::levels(TextureSize);

	Error += test_texture(
		gli::texture2d(gli::FORMAT_RGB8_UNORM_PACK8, TextureSize, Levels),
		glm::u8vec3(255, 128, 0), glm::u8vec3(0, 128, 255));

	Error += test_texture(
		gli::texture2d(gli::FORMAT_RGBA8_UNORM_PACK8, TextureSize, Levels),
		glm::u8vec4(255, 128, 0, 255), glm::u8vec4(0, 128, 255, 255));

	Error += test_texture(
		gli::texture2d(gli::FORMAT_RGBA32_SFLOAT_PACK32, TextureSize, Levels),
		glm::f32vec4(1.0, 0.5, 0.0, 1.0), glm::f32vec4(0.0, 0.5, 1.0, 1.0));

	Error += test_texture(
		gli::texture2d_array(gli::FORMAT_RGBA8_UNORM_PACK8, TextureSize, 4, Levels),
		glm::u8vec4(255, 128, 0, 255), glm::u8vec4(0, 128, 255, 255));

	Error += test_texture(
		gli::texture2d_array(gli::FORMAT_RGBA32_SFLOAT_PACK32, TextureSize, 4, Levels),
		glm::f32vec4(1.0, 0.5, 0.0, 1.0), glm::f32vec4(0.0, 0.5, 1.0, 1.0));

	Error += test_texture(
		gli::texture2d(gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, TextureSize, Levels),
		gli::detail::dxt1_block{63721, 255, 228, 144, 64, 0},
		gli::detail::dxt1_block{2516, 215, 152, 173, 215, 106});

	Error += test_texture(
		gli::texture2d(gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, TextureSize, Levels),
		gli::detail::dxt1_block{63721, 255, 228, 144, 64, 0},
		gli::detail::dxt1_block{2516, 215, 152, 173, 215, 106});

	Error += test_texture(
		gli::texture2d(gli::FORMAT_RGBA_DXT3_UNORM_BLOCK16, TextureSize, Levels),
		gli::detail::dxt3_block{12514, 1512, 12624, 16614, 63712, 255, 228, 144, 64, 0},
		gli::detail::dxt3_block{36125, 2416, 46314, 10515, 2516, 215, 152, 173, 215, 106});

	Error += test_texture(
		gli::texture2d(gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16, TextureSize, Levels),
		gli::detail::dxt5_block{255, 0, 64, 30, 50, 45, 242, 68, 63712, 255, 228, 144, 64, 0},
		gli::detail::dxt5_block{0, 255, 62, 144, 228, 214, 59, 200, 2516, 215, 152, 173, 215, 106});

	return Error;
}
