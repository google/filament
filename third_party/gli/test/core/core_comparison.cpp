#include <gli/gli.hpp>

#include <map>

int test_texture1D()
{
	int Error(0);

	std::vector<glm::u8vec4> Color(6);
	Color.push_back(glm::u8vec4(255,   0,   0, 255));
	Color.push_back(glm::u8vec4(255, 127,   0, 255));
	Color.push_back(glm::u8vec4(255, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255, 255, 255));
	Color.push_back(glm::u8vec4(  0,   0, 255, 255));

	gli::texture1d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(32), gli::levels(32));

	{
		gli::texture1d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(32), gli::levels(32));

		Error += TextureA == TextureB ? 0 : 1;
		GLI_ASSERT(!Error);
		Error += TextureA != TextureB ? 1 : 0;
		GLI_ASSERT(!Error);
	}

	{
		gli::texture1d TextureC(TextureA);

		Error += TextureA == TextureC ? 0 : 1;
		GLI_ASSERT(!Error);
		Error += TextureA != TextureC ? 1 : 0;
		GLI_ASSERT(!Error);
	}

	{
		gli::texture1d TextureD(TextureA, TextureA.base_level(), TextureA.max_level());

		Error += TextureA == TextureD ? 0 : 1;
		GLI_ASSERT(!Error);
		Error += TextureA != TextureD ? 1 : 0;
		GLI_ASSERT(!Error);
	}

	{
		gli::texture1d TextureE(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(32));

		*TextureE[TextureE.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureE ? 0 : 1;
		GLI_ASSERT(!Error);
		Error += TextureA == TextureE ? 1 : 0;
		GLI_ASSERT(!Error);
	}

	{
		gli::texture1d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(32), 1);

		*TextureB[TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		GLI_ASSERT(!Error);
		Error += TextureA == TextureB ? 1 : 0;
		GLI_ASSERT(!Error);
	}

	{
		gli::texture1d TextureB(gli::FORMAT_RGBA8_SNORM_PACK8, gli::texture1d::extent_type(32));

		*TextureB[TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		GLI_ASSERT(!Error);
		Error += TextureA == TextureB ? 1 : 0;
		GLI_ASSERT(!Error);
	}

	{
		gli::texture1d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(64));

		gli::texture1d TextureC(TextureB, TextureB.base_level() + 1, TextureB.max_level());

		Error += TextureA == TextureC ? 0 : 1;
		GLI_ASSERT(!Error);
		Error += TextureA != TextureC ? 1 : 0;
		GLI_ASSERT(!Error);
	}

	return Error;
}

int test_texture1DArray()
{
	int Error(0);

	std::vector<glm::u8vec4> Color(6);
	Color.push_back(glm::u8vec4(255,   0,   0, 255));
	Color.push_back(glm::u8vec4(255, 127,   0, 255));
	Color.push_back(glm::u8vec4(255, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255, 255, 255));
	Color.push_back(glm::u8vec4(  0,   0, 255, 255));

	gli::texture1d_array TextureA(
		gli::FORMAT_RGBA8_UNORM_PACK8,
		gli::texture1d::extent_type(32),
		gli::texture1d::size_type(1));

	{
		gli::texture1d_array TextureB(
			gli::FORMAT_RGBA8_UNORM_PACK8,
			gli::texture1d::extent_type(32),
			gli::texture1d::size_type(1));

		Error += TextureA == TextureB ? 0 : 1;
		Error += TextureA != TextureB ? 1 : 0;
	}

	{
		gli::texture1d_array TextureC(TextureA);

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	{
		gli::texture1d_array TextureD(TextureA, 
			TextureA.base_layer(),
			TextureA.max_layer(),
			TextureA.base_level(),
			TextureA.max_level());

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureA != TextureD ? 1 : 0;
	}

	{
		gli::texture1d_array TextureE(
			gli::FORMAT_RGBA8_UNORM_PACK8,
			gli::texture1d_array::extent_type(32),
			gli::texture1d::size_type(1));

		*TextureE[0][TextureE.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureE ? 0 : 1;
		Error += TextureA == TextureE ? 1 : 0;
	}

	{
		gli::texture1d_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d_array::extent_type(32), 1, 1);

		*TextureB[TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture1d_array TextureB(gli::FORMAT_RGBA8_SNORM_PACK8, gli::texture1d_array::extent_type(32), 1);

		*TextureB[0][TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture1d_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d_array::extent_type(64), 1);

		gli::texture1d_array TextureC(TextureB,
			TextureB.base_layer(), TextureB.max_layer(),
			TextureB.base_level() + 1, TextureB.max_level());

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	return Error;
}

int test_texture2D()
{
	int Error(0);

	std::vector<glm::u8vec4> Color(6);
	Color.push_back(glm::u8vec4(255,   0,   0, 255));
	Color.push_back(glm::u8vec4(255, 127,   0, 255));
	Color.push_back(glm::u8vec4(255, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255, 255, 255));
	Color.push_back(glm::u8vec4(  0,   0, 255, 255));

	gli::texture2d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32));

	{
		gli::texture2d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32));

		Error += TextureA == TextureB ? 0 : 1;
		Error += TextureA != TextureB ? 1 : 0;
	}

	{
		gli::texture2d TextureC(TextureA);

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	{
		gli::texture2d TextureD(gli::view(
			TextureA,
			TextureA.base_level(), TextureA.max_level()));

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureA != TextureD ? 1 : 0;
	}

	{
		gli::texture2d TextureE(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32));

		*TextureE[TextureE.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureE ? 0 : 1;
		Error += TextureA == TextureE ? 1 : 0;
	}

	{
		gli::texture2d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32), 1);

		*TextureB[TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture2d TextureB(gli::FORMAT_RGBA8_SNORM_PACK8, gli::texture2d::extent_type(32));

		*TextureB[TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture2d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(64));

		gli::texture2d TextureC(gli::view(
			TextureB,
			TextureB.base_level() + 1, TextureB.max_level()));

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	return Error;
}

int test_texture2DArray()
{
	int Error(0);

	std::vector<glm::u8vec4> Color(6);
	Color.push_back(glm::u8vec4(255,   0,   0, 255));
	Color.push_back(glm::u8vec4(255, 127,   0, 255));
	Color.push_back(glm::u8vec4(255, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255, 255, 255));
	Color.push_back(glm::u8vec4(  0,   0, 255, 255));

	gli::texture2d_array const TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d_array::extent_type(32), 1);

	{
		gli::texture2d_array const TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d_array::extent_type(32), 1);

		Error += TextureA == TextureB ? 0 : 1;
		Error += TextureA != TextureB ? 1 : 0;
	}

	{
		gli::texture2d_array const TextureC(TextureA);

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	{
		gli::texture2d_array const TextureD(TextureA,
			TextureA.base_layer(), TextureA.max_layer(),
			TextureA.base_level(), TextureA.max_level());

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureA != TextureD ? 1 : 0;
	}

	{
		gli::texture2d_array const TextureE(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d_array::extent_type(32), 1);

		*TextureE[0][TextureE.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureE ? 0 : 1;
		Error += TextureA == TextureE ? 1 : 0;
	}

	{
		gli::texture2d_array const TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d_array::extent_type(32), 1, 1);

		*TextureB[TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture2d_array const TextureB(gli::FORMAT_RGBA8_SNORM_PACK8, gli::texture2d_array::extent_type(32), 1);

		*TextureB[0][TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture2d_array const TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d_array::extent_type(64), 1);

		gli::texture2d_array const TextureC(TextureB,
			TextureB.base_layer(), TextureB.max_layer(),
			TextureB.base_level() + 1, TextureB.max_level());

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	return Error;
}

int test_texture3D()
{
	int Error(0);

	std::vector<glm::u8vec4> Color(6);
	Color.push_back(glm::u8vec4(255,   0,   0, 255));
	Color.push_back(glm::u8vec4(255, 127,   0, 255));
	Color.push_back(glm::u8vec4(255, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255, 255, 255));
	Color.push_back(glm::u8vec4(  0,   0, 255, 255));

	gli::texture3d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(32));

	{
		gli::texture3d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(32));

		Error += TextureA == TextureB ? 0 : 1;
		Error += TextureA != TextureB ? 1 : 0;
	}

	{
		gli::texture3d TextureC(TextureA);

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	{
		gli::texture3d TextureD(TextureA, TextureA.base_level(), TextureA.max_level());

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureA != TextureD ? 1 : 0;
	}

	{
		gli::texture3d TextureE(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(32));

		*TextureE[TextureE.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureE ? 0 : 1;
		Error += TextureA == TextureE ? 1 : 0;
	}

	{
		gli::texture3d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(32), 1);

		*TextureB[TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture3d TextureB(gli::FORMAT_RGBA8_SNORM_PACK8, gli::texture3d::extent_type(32));

		*TextureB[TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture3d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(64));

		gli::texture3d TextureC(TextureB, TextureB.base_level() + 1, TextureB.max_level());

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	return Error;
}

int test_textureCube()
{
	int Error(0);

	std::vector<glm::u8vec4> Color(6);
	Color.push_back(glm::u8vec4(255,   0,   0, 255));
	Color.push_back(glm::u8vec4(255, 127,   0, 255));
	Color.push_back(glm::u8vec4(255, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255, 255, 255));
	Color.push_back(glm::u8vec4(  0,   0, 255, 255));

	gli::texture_cube::extent_type const Size(16);

	gli::texture_cube TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, Size);

	{
		gli::texture_cube TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, Size);

		Error += TextureA == TextureB ? 0 : 1;
		Error += TextureA != TextureB ? 1 : 0;
	}

	{
		gli::texture_cube TextureC(TextureA);

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	{
		gli::texture_cube TextureD(TextureA, 
			TextureA.base_face(), TextureA.max_face(),
			TextureA.base_level(), TextureA.max_level());

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureA != TextureD ? 1 : 0;
	}

	{
		gli::texture_cube TextureE(gli::FORMAT_RGBA8_UNORM_PACK8, Size);

		*TextureE[TextureE.faces() - 1][TextureE.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureE ? 0 : 1;
		Error += TextureA == TextureE ? 1 : 0;
	}

	{
		gli::texture_cube TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, Size, 1);

		*TextureB[TextureB.faces() - 1][TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture_cube TextureB(gli::FORMAT_RGBA8_SNORM_PACK8, Size);

		*TextureB[TextureB.faces() - 1][TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture_cube TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, Size << gli::texture_cube::extent_type(1));

		gli::texture_cube TextureC(TextureB, 
			TextureB.base_face(), TextureB.max_face(),
			TextureB.base_level() + 1, TextureB.max_level());

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	return Error;
}

int test_textureCubeArray()
{
	int Error(0);

	std::vector<glm::u8vec4> Color(6);
	Color.push_back(glm::u8vec4(255,   0,   0, 255));
	Color.push_back(glm::u8vec4(255, 127,   0, 255));
	Color.push_back(glm::u8vec4(255, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255,   0, 255));
	Color.push_back(glm::u8vec4(  0, 255, 255, 255));
	Color.push_back(glm::u8vec4(  0,   0, 255, 255));

	gli::texture_cube_array TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube_array::extent_type(32), 1);

	{
		gli::texture_cube_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube_array::extent_type(32), 1);

		Error += TextureA == TextureB ? 0 : 1;
		Error += TextureA != TextureB ? 1 : 0;
	}

	{
		gli::texture_cube_array TextureC(TextureA);

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	{
		gli::texture_cube_array TextureD(TextureA, 
			TextureA.base_layer(), TextureA.max_layer(),
			TextureA.base_face(), TextureA.max_face(),
			TextureA.base_level(), TextureA.max_level());

		Error += TextureA == TextureD ? 0 : 1;
		Error += TextureA != TextureD ? 1 : 0;
	}

	{
		gli::texture_cube_array TextureE(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube_array::extent_type(32), 1);

		*TextureE[0][TextureE.faces() - 1][TextureE.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureE ? 0 : 1;
		Error += TextureA == TextureE ? 1 : 0;
	}

	{
		gli::texture_cube_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube_array::extent_type(32), 1, 1);

		*TextureB[0][TextureB.faces() - 1][TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture_cube_array TextureB(gli::FORMAT_RGBA8_SNORM_PACK8, gli::texture_cube_array::extent_type(32), 1);

		*TextureB[0][TextureB.faces() - 1][TextureB.levels() - 1].data<glm::u8vec4>() = glm::u8vec4(255, 127, 0, 255);

		Error += TextureA != TextureB ? 0 : 1;
		Error += TextureA == TextureB ? 1 : 0;
	}

	{
		gli::texture_cube_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube_array::extent_type(64), 1);

		gli::texture_cube_array TextureC(TextureB, 
			TextureB.base_layer(), TextureB.max_layer(),
			TextureB.base_face(), TextureB.max_face(),
			TextureB.base_level() + 1, TextureB.max_level());

		Error += TextureA == TextureC ? 0 : 1;
		Error += TextureA != TextureC ? 1 : 0;
	}

	return Error;
}

class C
{};

int test_map()
{
	int Error(0);

	gli::texture2d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32));
	gli::texture2d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(64));
	
	std::map<int, gli::texture2d> Map;

	Map.insert(std::make_pair(0, TextureA));
	Map.insert(std::make_pair(0, TextureB));
	
	std::map<int, C> MapC;
	MapC.insert(std::make_pair(0, C()));
	MapC.insert(std::make_pair(0, C()));

	return Error;
}

int main()
{
	int Error(0);

	Error += test_texture1D();
	Error += test_texture1DArray();
	Error += test_texture2D();
	Error += test_texture2DArray();
	Error += test_texture3D();
	Error += test_textureCube();
	Error += test_textureCubeArray();
	Error += test_map();
		
	return Error;
}
