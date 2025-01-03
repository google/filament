#include <gli/reduce.hpp>
#include <glm/vector_relational.hpp>

namespace
{
	inline gli::u8vec4 abs_diff(gli::u8vec4 const & A, gli::u8vec4 const & B)
	{
		return gli::abs(A - B);
	}
}

namespace reduce1d
{
	int test()
	{
		int Error = 0;
		
		gli::texture1d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(32));
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		gli::texture1d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(32));
		TextureB.clear(gli::u8vec4(255, 127, 0, 255));
		
		gli::u8vec4 const MaxAbsDiff = gli::reduce<gli::u8vec4>(TextureA, TextureB, abs_diff, gli::max);
		Error += gli::all(glm::equal(MaxAbsDiff, gli::u8vec4(0))) ? 0 : 1;
		
		return Error;
	}
}//namespace reduce1d

namespace reduce1d_array
{
	int test()
	{
		int Error = 0;
		
		gli::texture1d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(32));
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		gli::texture1d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture1d::extent_type(32));
		TextureB.clear(gli::u8vec4(255, 127, 0, 255));
		
		gli::u8vec4 const MaxAbsDiff = gli::reduce<gli::u8vec4>(TextureA, TextureB, abs_diff, gli::max);
		Error += gli::all(glm::equal(MaxAbsDiff, gli::u8vec4(0))) ? 0 : 1;
		
		return Error;
	}
}//namespace reduce1d_array

namespace reduce2d
{
	int test()
	{
		int Error = 0;
		
		gli::texture2d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32));
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		gli::texture2d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32));
		TextureB.clear(gli::u8vec4(255, 127, 0, 255));
		
		gli::u8vec4 const MaxAbsDiff = gli::reduce<gli::u8vec4>(TextureA, TextureB, abs_diff, gli::max);
		Error += gli::all(glm::equal(MaxAbsDiff, gli::u8vec4(0))) ? 0 : 1;
		
		return Error;
	}
}//namespace reduce2d

namespace reduce2d_array
{
	int test()
	{
		int Error = 0;
		
		gli::texture2d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32));
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		gli::texture2d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture2d::extent_type(32));
		TextureB.clear(gli::u8vec4(255, 127, 0, 255));
		
		gli::u8vec4 const MaxAbsDiff = gli::reduce<gli::u8vec4>(TextureA, TextureB, abs_diff, gli::max);
		Error += gli::all(glm::equal(MaxAbsDiff, gli::u8vec4(0))) ? 0 : 1;
		
		return Error;
	}
}//namespace reduce2d_array

namespace reduce3d
{
	int test()
	{
		int Error = 0;
		
		gli::texture3d TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(32));
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		gli::texture3d TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture3d::extent_type(32));
		TextureB.clear(gli::u8vec4(255, 127, 0, 255));
		
		gli::u8vec4 const MaxAbsDiff = gli::reduce<gli::u8vec4>(TextureA, TextureB, abs_diff, gli::max);
		Error += gli::all(glm::equal(MaxAbsDiff, gli::u8vec4(0))) ? 0 : 1;
		
		return Error;
	}
}//namespace reduce3d

namespace reduce_cube
{
	int test()
	{
		int Error = 0;
		
		gli::texture_cube TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(32));
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		gli::texture_cube TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube::extent_type(32));
		TextureB.clear(gli::u8vec4(255, 127, 0, 255));
		
		gli::u8vec4 const MaxAbsDiff = gli::reduce<gli::u8vec4>(TextureA, TextureB, abs_diff, gli::max);
		Error += gli::all(glm::equal(MaxAbsDiff, gli::u8vec4(0))) ? 0 : 1;
		
		return Error;
	}
}//namespace reduce_cube

namespace reduce_cube_array
{
	int test()
	{
		int Error = 0;
		
		gli::texture_cube_array TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube_array::extent_type(32), 2);
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		gli::texture_cube_array TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, gli::texture_cube_array::extent_type(32), 2);
		TextureB.clear(gli::u8vec4(255, 127, 0, 255));
		
		gli::u8vec4 const MaxAbsDiff = gli::reduce<gli::u8vec4>(TextureA, TextureB, abs_diff, gli::max);
		Error += gli::all(glm::equal(MaxAbsDiff, gli::u8vec4(0))) ? 0 : 1;
		
		return Error;
	}
}//namespace reduce_cube_array

int main()
{
	int Error = 0;

	Error += reduce1d::test();
	Error += reduce1d_array::test();
	Error += reduce2d::test();
	Error += reduce2d_array::test();
	Error += reduce3d::test();
	Error += reduce_cube::test();
	Error += reduce_cube_array::test();

	return Error;
}
