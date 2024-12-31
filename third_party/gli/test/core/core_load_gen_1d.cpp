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
	
	for(int FormatIndex = gli::FORMAT_FIRST, FormatCount = gli::FORMAT_LAST; FormatIndex < FormatCount; ++FormatIndex)
	{
		gli::target const Target = gli::TARGET_1D;
		gli::format Format = gli::FORMAT_BGR8_UNORM_PACK8;//static_cast<gli::format>(FormatIndex);
		
		if((gli::is_compressed(Format) && (gli::is_target_1d(Target) || Target == gli::TARGET_3D)) || gli::is_target_rect(Target))
			continue;
			
		gli::size_t const Layers = gli::is_target_array(Target) ? 2 : 1;
		gli::size_t const Faces = gli::is_target_cube(Target) ? 6 : 1;
		gli::ivec3 const BlockExtent = gli::block_extent(Format);
			
		gli::texture Texture(Target, Format, BlockExtent * gli::ivec3(BlockExtent.y, BlockExtent.x, 1), Layers, Faces, 2);
		Texture.clear();
			
		gli::save(Texture, "test1d.dds");
		gli::texture TextureDDS(gli::load("test1d.dds"));
		Error += Texture == TextureDDS ? 0 : 1;
			
		gli::save(Texture, "test1d.ktx");
		gli::texture TextureKTX(gli::load("test1d.ktx"));
		Error += Texture == TextureKTX ? 0 : 1;
			
		gli::save(Texture, "test1d.kmg");
		gli::texture TextureKMG(gli::load("test1d.kmg"));
		Error += Texture == TextureKMG ? 0 : 1;

		GLI_ASSERT(!Error);
	}
	
	return Error;
}
