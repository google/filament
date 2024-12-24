#include <gli/transform.hpp>

namespace
{
	gli::u8vec4 average(gli::u8vec4 const & A, gli::u8vec4 const & B)
	{
		return gli::u8vec4((gli::uvec4(A) + gli::uvec4(B)) / gli::uvec4(2));
	}
}//namespace

namespace transform
{
	template <typename texture_type>
	int test()
	{
		int Error = 0;
		
		texture_type TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::extent_type(4));
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		texture_type TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::extent_type(4));
		TextureB.clear(gli::u8vec4(255, 127, 64, 192));
		texture_type TextureO(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::extent_type(4));
		
		gli::transform<gli::u8vec4>(TextureO, TextureA, TextureB, average);
		
		gli::u8vec4 const * const data = TextureO.template data<gli::u8vec4>();
		for(gli::texture1d::size_type TexelIndex = 0, TexelCount = TextureO.template size<gli::u8vec4>(); TexelIndex < TexelCount; ++TexelIndex)
		{
			Error += *(data + TexelIndex) == gli::u8vec4(255, 127, 32, 223) ? 0 : 1;
			GLI_ASSERT(!Error);
		}
		
		return Error;
	}
}//namespace transform

namespace transform_array
{
	template <typename texture_type>
	int test()
	{
		int Error = 0;
		
		texture_type TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::extent_type(4), 2);
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		texture_type TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::extent_type(4), 2);
		TextureB.clear(gli::u8vec4(255, 127, 64, 192));
		texture_type TextureO(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::extent_type(4), 2);
		
		gli::transform<gli::u8vec4>(TextureO, TextureA, TextureB, average);
		
		gli::u8vec4 const * const data = TextureO.template data<gli::u8vec4>();
		for(typename texture_type::size_type TexelIndex = 0, TexelCount = TextureO.template size<gli::u8vec4>(); TexelIndex < TexelCount; ++TexelIndex)
		{
			Error += *(data + TexelIndex) == gli::u8vec4(255, 127, 32, 223) ? 0 : 1;
			GLI_ASSERT(!Error);
		}
		
		return Error;
	}
}//namespace transform_array

int main()
{
	int Error = 0;
	
	Error += transform::test<gli::texture1d>();
	Error += transform_array::test<gli::texture1d_array>();
	Error += transform::test<gli::texture2d>();
	Error += transform_array::test<gli::texture2d_array>();
	Error += transform::test<gli::texture3d>();
	Error += transform::test<gli::texture_cube>();
	Error += transform_array::test<gli::texture_cube_array>();
	
	return Error;
}
