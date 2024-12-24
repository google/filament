//////////////////////////////////////////////////////////////////////////////////
/// OpenGL Image (gli.g-truc.net)
///
/// Copyright (c) 2008 - 2015 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @ref core
/// @file gli/test/core/core_load.cpp
/// @date 2013-11-25 / 2015-08-08
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <gli/gli.hpp>

namespace
{
	struct params
	{
		params(std::string const & Filename, gli::format Format)
			: Filename(Filename)
			, Format(Format)
		{}

		std::string Filename;
		gli::format Format;
	};
}//namespace

int main()
{
	int Error = 0;
	
	for(int TargetIndex = gli::TARGET_FIRST, TargetCount = gli::TARGET_3D; TargetIndex < TargetCount; ++TargetIndex)
	for(int FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_LAST; FormatIndex < FormatCount; ++FormatIndex)
	{
		gli::target Target = static_cast<gli::target>(TargetIndex);
		gli::format Format = static_cast<gli::format>(FormatIndex);
		
		if((gli::is_compressed(Format) && (gli::is_target_1d(Target) || Target == gli::TARGET_3D)) || gli::is_target_rect(Target))
			continue;
			
		gli::size_t const Layers = gli::is_target_array(Target) ? 2 : 1;
		gli::size_t const Faces = gli::is_target_cube(Target) ? 6 : 1;
		gli::ivec3 const BlockDimensions = gli::block_dimensions(Format);
			
		gli::texture Texture(Target, Format, BlockDimensions * gli::ivec3(BlockDimensions.y, BlockDimensions.x, 1), Layers, Faces, 2);
		Texture.clear();
			
		gli::save(Texture, "gen_test.dds");
		gli::texture TextureDDS(gli::load("gen_test.dds"));
		Error += Texture == TextureDDS ? 0 : 1;
			
		gli::save(Texture, "gen_test.ktx");
		gli::texture TextureKTX(gli::load("gen_test.ktx"));
		Error += Texture == TextureKTX ? 0 : 1;
			
		gli::save(Texture, "gen_test.kmg");
		gli::texture TextureKMG(gli::load("gen_test.kmg"));
		Error += Texture == TextureKMG ? 0 : 1;

		assert(!Error);
	}
	
	return Error;
}
