#include <gli/gli.hpp>
#include <ctime>
#include <cstdio>

int test_create_texture_storage()
{
	int Error(0);

	gli::texture2d Texture(gli::FORMAT_RGBA8_UINT_PACK8, gli::texture2d::extent_type(256));
	Texture.clear(gli::u8vec4(255, 127, 0, 255));
	gli::texture2d::size_type Levels = Texture.levels();

	Error += Levels > 1 ? 0 : 1;

	GLI_ASSERT(!Texture.empty());

	void const * Pointer = Texture[0].data();

	Error += Pointer != 0 ? 0 : 1;

	glm::u8vec4 TexelA = Texture[0].data<glm::u8vec4>()[0];
	glm::u8vec4 TexelB = Texture[0].data<glm::u8vec4>()[1];
	glm::u8vec4 TexelC = Texture[0].data<glm::u8vec4>()[2];
	glm::u8vec4 TexelD = Texture[0].data<glm::u8vec4>()[3];

	Error += TexelA == gli::u8vec4(255, 127, 0, 255) ? 0 : 1;
	Error += TexelB == gli::u8vec4(255, 127, 0, 255) ? 0 : 1;
	Error += TexelC == gli::u8vec4(255, 127, 0, 255) ? 0 : 1;
	Error += TexelD == gli::u8vec4(255, 127, 0, 255) ? 0 : 1;

	return Error;
}

int test_reset_memset_zero()
{
	std::vector<int> Data;
	Data.resize(1 << 21);

	std::clock_t LastTime = std::clock();

	for(std::size_t j = 0; j < (1 << 4); ++j)
		memset(&Data[0], 0, Data.size());

	std::clock_t Time = std::clock();

	printf("test_reset_memset_zero: %lu\n", Time - LastTime);

	return 0;
}

int test_reset_loop_zero()
{
	std::vector<int> Data;
	Data.resize(1 << 21);

	std::clock_t LastTime = std::clock();

	for(std::size_t j = 0, n = (1 << 4); j < n; ++j)
	for(std::size_t i = 0, m = Data.size(); i < m; ++i)
		Data[i] = 0;

	std::clock_t Time = std::clock();

	printf("test_reset_loop_zero: %lu\n", Time - LastTime);

	return 0;
}

int test_floorMultiple()
{
	int Error(0);

	int const A = glm::floorMultiple(3, 4);
	int const B = glm::floorMultiple(6, 4);
	int const C = glm::floorMultiple(8, 4);
	int const D = glm::floorMultiple(9, 4);

	Error += A == 0 ? 0 : 1;
	Error += B == 4 ? 0 : 1;
	Error += C == 8 ? 0 : 1;
	Error += D == 8 ? 0 : 1;

	return Error;
}

int main()
{
	int Error(0);

	Error += test_floorMultiple();
	Error += test_reset_memset_zero();
	Error += test_reset_loop_zero();
	Error += test_create_texture_storage();
	
	return Error;
}
