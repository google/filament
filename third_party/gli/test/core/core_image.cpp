#include <gli/image.hpp>
#include <gli/comparison.hpp>

int test_image_ctor()
{
	int Error(0);

	gli::image ImageA(gli::FORMAT_RGBA8_UINT_PACK8, gli::image::extent_type(4, 4, 1));
	gli::image ImageB(gli::FORMAT_RGBA8_UINT_PACK8, gli::image::extent_type(4, 4, 1));
	gli::image ImageC = ImageA;
	gli::image ImageD(ImageA, gli::FORMAT_RGBA8_UNORM_PACK8);
	gli::image ImageE(ImageD, gli::FORMAT_RGBA8_UNORM_PACK8);

	Error += ImageA == ImageB ? 0 : 1;
	Error += ImageC == ImageB ? 0 : 1;
	Error += ImageA == ImageE ? 0 : 1;

	return Error;
}

int test_image_data()
{
	int Error(0);

	gli::image ImageA;
	Error += ImageA.empty() ? 0 : 1;
	GLI_ASSERT(!Error);

	gli::image ImageB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::image::extent_type(1, 1, 1));
	Error += ImageB.size() == sizeof(glm::u8vec4) ? 0 : 1;

	*ImageB.data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);
	Error += !ImageB.empty() ? 0 : 1;
	GLI_ASSERT(!Error);

	return Error;
}

int test_image_query()
{
	int Error(0);

	gli::image Image(gli::FORMAT_RGBA8_UINT_PACK8, gli::image::extent_type(1, 1, 1));

	Error += Image.size() == sizeof(glm::u8vec4) ? 0 : 1;
	Error += !Image.empty() ? 0 : 1;
	Error += Image.extent().x == 1 ? 0 : 1;
	Error += Image.extent().y == 1 ? 0 : 1;
	Error += Image.extent().z == 1 ? 0 : 1;

	return Error;
}

namespace fetch
{
	int test()
	{
		int Error(0);

		gli::image Image(gli::FORMAT_RGBA8_UINT_PACK8, gli::image::extent_type(4, 2, 1));
		*(Image.data<glm::u8vec4>() + 0) = glm::u8vec4(255,   0,   0, 255);
		*(Image.data<glm::u8vec4>() + 1) = glm::u8vec4(255, 128,   0, 255);
		*(Image.data<glm::u8vec4>() + 2) = glm::u8vec4(255, 255,   0, 255);
		*(Image.data<glm::u8vec4>() + 3) = glm::u8vec4(128, 255,   0, 255);
		*(Image.data<glm::u8vec4>() + 4) = glm::u8vec4(  0, 255,   0, 255);
		*(Image.data<glm::u8vec4>() + 5) = glm::u8vec4(  0, 255, 255, 255);
		*(Image.data<glm::u8vec4>() + 6) = glm::u8vec4(  0,   0, 255, 255);
		*(Image.data<glm::u8vec4>() + 7) = glm::u8vec4(255,   0, 255, 255);

		glm::u8vec4 Data0 = Image.load<glm::u8vec4>(gli::image::extent_type(0, 0, 0));
		glm::u8vec4 Data1 = Image.load<glm::u8vec4>(gli::image::extent_type(1, 0, 0));
		glm::u8vec4 Data2 = Image.load<glm::u8vec4>(gli::image::extent_type(2, 0, 0));
		glm::u8vec4 Data3 = Image.load<glm::u8vec4>(gli::image::extent_type(3, 0, 0));
		glm::u8vec4 Data4 = Image.load<glm::u8vec4>(gli::image::extent_type(0, 1, 0));
		glm::u8vec4 Data5 = Image.load<glm::u8vec4>(gli::image::extent_type(1, 1, 0));
		glm::u8vec4 Data6 = Image.load<glm::u8vec4>(gli::image::extent_type(2, 1, 0));
		glm::u8vec4 Data7 = Image.load<glm::u8vec4>(gli::image::extent_type(3, 1, 0));

		Error += glm::all(glm::equal(Data0, glm::u8vec4(255,   0,   0, 255))) ? 0 : 1;
		Error += glm::all(glm::equal(Data1, glm::u8vec4(255, 128,   0, 255))) ? 0 : 1;
		Error += glm::all(glm::equal(Data2, glm::u8vec4(255, 255,   0, 255))) ? 0 : 1;
		Error += glm::all(glm::equal(Data3, glm::u8vec4(128, 255,   0, 255))) ? 0 : 1;
		Error += glm::all(glm::equal(Data4, glm::u8vec4(  0, 255,   0, 255))) ? 0 : 1;
		Error += glm::all(glm::equal(Data5, glm::u8vec4(  0, 255, 255, 255))) ? 0 : 1;
		Error += glm::all(glm::equal(Data6, glm::u8vec4(  0,   0, 255, 255))) ? 0 : 1;
		Error += glm::all(glm::equal(Data7, glm::u8vec4(255,   0, 255, 255))) ? 0 : 1;

		return Error;
	}
}//namespace fetch

int main()
{
	int Error(0);

	Error += test_image_ctor();
	Error += test_image_data();
	Error += test_image_query();
	Error += fetch::test();
		
	return Error;
}

