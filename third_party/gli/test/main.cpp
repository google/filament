#define GLM_SWIZZLE_XYZW
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>

#include <gli/gli.hpp>
#include <gli/gtx/fetch.hpp>
#include <gli/gtx/gradient.hpp>
#include <gli/gtx/loader.hpp>

#include "bug.hpp"
#include "core.hpp"

#include <vector>

//#include <boost/format.hpp>

bool test_image_wip()
{
	//gli::wip::texture2D<glm::u8vec3, gli::wip::plain> Texture;
	//gli::wip::texture2D<glm::u8vec3, gli::wip::plain>::image Mipmap = Texture[0];

	//glm::vec2 Texcoord(0);
	//Texture[0](Texcoord);

	//gli::wip::plain<glm::u8vec3> Surface;
	//gli::wip::fetch(Surface);
	//gli::wip::fetch(Texture);

	return true;
}

bool test_image_export()
{
	//gli::texture2D Texture = gli::load<gli::TGA>("../test_rgb8.tga");
	//gli::texture2D TextureMipmaped = gli::generateMipmaps(Texture, 0);

	//gli::save(TextureMipmaped, 0, "../test0.tga");
	//gli::save(TextureMipmaped, 1, "../test1.tga");
	//gli::save(TextureMipmaped, 2, "../test2.tga");
	//gli::save(TextureMipmaped, 3, "../test3.tga");

	return true;
}

bool test_image_export_dds()
{
	{
		gli::texture2D Texture = gli::loadTGA("../test_rgb8.tga");
		assert(!Texture.empty());
		gli::saveTGA(Texture, "../test_tga2tgaEXT.tga");
	}
	{
		gli::texture2D Texture = gli::loadTGA("../test_rgb8.tga");
		assert(!Texture.empty());
		gli::saveDDS9(Texture, "../test_tga2ddsEXT.dds");
	}
	{
		gli::texture2D Texture = gli::loadDDS9("../test_rgb8.dds");
		assert(!Texture.empty());
		gli::saveDDS9(Texture, "../test_dds2tgaEXT.tga");
	}
	{
		gli::texture2D Texture = gli::loadDDS9("../test_rgb8.dds");
		assert(!Texture.empty());
		gli::saveDDS9(Texture, "../test_dds2ddsEXT.dds");
	}
	{
		gli::texture2D Texture = gli::loadDDS9("../test_dxt1.dds");
		assert(!Texture.empty());
		gli::saveDDS9(Texture, "../test_dxt2dxtEXT.dds");
	}
	{
		gli::texture2D Texture = gli::loadDDS10("../test_bc1.dds");
		assert(!Texture.empty());
		gli::saveDDS10(Texture, "../test_bc12bc1EXT.dds");
	}

	////////////////////////
	//{
	//	gli::texture2D Texture = gli::load("../test_rgb8.tga");
	//	assert(!Texture.empty());
	//	gli::save(Texture, "../test_tga2tga.tga");
	//}
	//{
	//	gli::texture2D Texture = gli::load("../test_rgb8.tga");
	//	assert(!Texture.empty());
	//	gli::save(Texture, "../test_tga2dds.dds");
	//}
	//{
	//	gli::texture2D Texture = gli::load("../test_rgb8.dds");
	//	assert(!Texture.empty());
	//	gli::save(Texture, "../test_dds2tga.tga");
	//}
	//{
	//	gli::texture2D Texture = gli::load("../test_rgb8.dds");
	//	assert(!Texture.empty());
	//	gli::save(Texture, "../test_dds2dds.dds");
	//}
	//{
	//	gli::texture2D Texture = gli::load("../test_dxt1.dds");
	//	assert(!Texture.empty());
	//	gli::save(Texture, "../test_dxt2dxt.dds");
	//}

	return true;
}

bool test_image_fetch()
{
	gli::texture2D Texture = gli::loadTGA("../test.tga");
	if(!Texture.empty())
	{
		gli::texture2D::dimensions_type Size = Texture[0].dimensions();

		glm::u8vec3 TexelA = gli::textureLod<glm::u8vec3>(Texture, gli::texture2D::texcoord_type(0.0f, 0.0f), 0);
		//glm::u8vec3 TexelB = gli::textureLod<glm::u8vec3>(Texture, gli::texture2D::texcoord_type(0.5f, 0.5f), 0);

		glm::u8vec3 TexelC = gli::texelFetch<glm::u8vec3>(Texture, gli::texture2D::dimensions_type(7, 7), 0);
		glm::u8vec3 TexelD = gli::texelFetch<glm::u8vec3>(Texture, gli::texture2D::dimensions_type(7, 0), 0);
		glm::u8vec3 TexelE = gli::texelFetch<glm::u8vec3>(Texture, gli::texture2D::dimensions_type(0, 7), 0);
	}

	return true;
}

bool test_image_gradient()
{
	{
		gli::texture2D Texture = gli::radial(glm::uvec2(256), glm::vec2(0.25f), 128.0f, glm::vec2(0.5f));
		gli::saveTGA(Texture, "../gradient_radial.tga");
	}

	{
		gli::texture2D Texture = gli::linear(glm::uvec2(256), glm::vec2(0.25f), glm::vec2(0.75f));
		gli::saveTGA(Texture, "../gradient_linear.tga");
	}

	return true;
}

int main()
{
	glm::vec4 v1(1, 2, 3, 4);
	glm::vec4 v2;
	glm::vec4 v3;
	glm::vec4 v4;
	v2.wyxz = v1.zyxw;
	v3 = v1.xzyw;
	v4.xzyw = v1;

	gli::texture2D TextureBC7 = gli::load("../kueken256-bc7.dds");
	gli::texture2D TextureBC7_0 = gli::load("../kueken256-bc7-0.dds");
	TextureBC7[0] = TextureBC7_0[0];
	gli::save(TextureBC7, "../kueken256-bc7-saved.dds");

	{
		gli::texture2D TextureLoad[] =
		{
			gli::loadDDS10("../kueken256-rgb8_BC7.dds"), 
			gli::loadDDS10("../kueken128-rgb8_BC7.dds"), 
			gli::loadDDS10("../kueken64-rgb8_BC7.dds"), 
			gli::loadDDS10("../kueken32-rgb8_BC7.dds"), 
			gli::loadDDS10("../kueken16-rgb8_BC7.dds"), 
			gli::loadDDS10("../kueken8-rgb8_BC7.dds"), 
			gli::loadDDS10("../kueken4-rgb8_BC7.dds"), 
			gli::loadDDS10("../kueken4-rgb8_BC7.dds"), 
			gli::loadDDS10("../kueken4-rgb8_BC7.dds")
		};

		gli::texture2D Texture(sizeof(TextureLoad) / sizeof(gli::texture2D));

		for(gli::texture2D::level_type Level = 0; Level < Texture.levels(); ++Level)
			Texture[Level] = TextureLoad[Level][0];

		assert(!Texture.empty());
		gli::saveDDS10(Texture, "../kueken256-bc7.dds");
	}
	/*
	test_image_wip();
	test_image_fetch();
	test_image_gradient();
	test_image_export_dds();
	*/
	//test_image_export();

	//// Set texture2D
	//gli::wip::texture2D<glm::u8vec3> Texture = gli::wip::import_as(TEXTURE_DIFFUSE);
	//for(gli::wip::texture2D<glm::u8vec3>::level_type Level = 0; Level < Texture.levels(); ++Level)
	//{
	//	glTexImage2D(
	//		GL_TEXTURE_2D, 
	//		GLint(Level), 
	//		GL_RGB, 
	//		GLsizei(Image[Level]->size().x), 
	//		GLsizei(Image[Level]->size().y), 
	//		0,  
	//		GL_BGR, 
	//		GL_UNSIGNED_BYTE, 
	//		Image[Level]->data());
	//}

	return 0;
}
