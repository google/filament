#include <cstdio>
#include "../load_dds.hpp"
#include "file.hpp"

namespace gli{
namespace detail
{
	inline d3d10_resource_dimension get_dimension(gli::target Target)
	{
		static d3d10_resource_dimension Table[] = //TARGET_COUNT
		{
			D3D10_RESOURCE_DIMENSION_TEXTURE1D,		//TARGET_1D,
			D3D10_RESOURCE_DIMENSION_TEXTURE1D,		//TARGET_1D_ARRAY,
			D3D10_RESOURCE_DIMENSION_TEXTURE2D,		//TARGET_2D,
			D3D10_RESOURCE_DIMENSION_TEXTURE2D,		//TARGET_2D_ARRAY,
			D3D10_RESOURCE_DIMENSION_TEXTURE3D,		//TARGET_3D,
			D3D10_RESOURCE_DIMENSION_TEXTURE2D,		//TARGET_RECT,
			D3D10_RESOURCE_DIMENSION_TEXTURE2D,		//TARGET_RECT_ARRAY,
			D3D10_RESOURCE_DIMENSION_TEXTURE2D,		//TARGET_CUBE,
			D3D10_RESOURCE_DIMENSION_TEXTURE2D		//TARGET_CUBE_ARRAY
		};
		static_assert(sizeof(Table) / sizeof(Table[0]) == TARGET_COUNT, "Table needs to be updated");

		return Table[Target];
	}
	
	inline dx::d3dfmt get_fourcc(bool RequireDX10Header, gli::format Format, dx::format const& DXFormat)
	{
		if(RequireDX10Header)
		{
			detail::formatInfo const & FormatInfo = detail::get_format_info(Format);
			
			if(FormatInfo.Flags & detail::CAP_DDS_GLI_EXT_BIT)
				return dx::D3DFMT_GLI1;
			else
				return dx::D3DFMT_DX10;
		}
		else
		{
			return (DXFormat.DDPixelFormat & dx::DDPF_FOURCC) ? DXFormat.D3DFormat : dx::D3DFMT_UNKNOWN;
		}
	}
}//namespace detail

	inline bool save_dds(texture const& Texture, std::vector<char>& Memory)
	{
		if(Texture.empty())
			return false;

		dx DX;
		dx::format const& DXFormat = DX.translate(Texture.format());

		bool const RequireDX10Header = DXFormat.D3DFormat == dx::D3DFMT_GLI1 || DXFormat.D3DFormat == dx::D3DFMT_DX10 || is_target_array(Texture.target()) || is_target_1d(Texture.target());

		Memory.resize(Texture.size() + sizeof(detail::FOURCC_DDS) + sizeof(detail::dds_header) + (RequireDX10Header ? sizeof(detail::dds_header10) : 0));

		memcpy(&Memory[0], detail::FOURCC_DDS, sizeof(detail::FOURCC_DDS));
		std::size_t Offset = sizeof(detail::FOURCC_DDS);

		detail::dds_header& Header = *reinterpret_cast<detail::dds_header*>(&Memory[0] + Offset);
		Offset += sizeof(detail::dds_header);

		detail::formatInfo const& Desc = detail::get_format_info(Texture.format());

		std::uint32_t Caps = detail::DDSD_CAPS | detail::DDSD_WIDTH | detail::DDSD_PIXELFORMAT | detail::DDSD_MIPMAPCOUNT;
		Caps |= !is_target_1d(Texture.target()) ? detail::DDSD_HEIGHT : 0;
		Caps |= Texture.target() == TARGET_3D ? detail::DDSD_DEPTH : 0;
		//Caps |= Storage.levels() > 1 ? detail::DDSD_MIPMAPCOUNT : 0;
		Caps |= (Desc.Flags & detail::CAP_COMPRESSED_BIT) ? detail::DDSD_LINEARSIZE : detail::DDSD_PITCH;

		std::uint32_t PitchInBytes = 0u;
		if( ( Desc.Flags & detail::CAP_COMPRESSED_BIT ) )
		{
			PitchInBytes = static_cast<std::uint32_t>(Texture.size() / Texture.faces());
		}
		else
		{
			const texture::extent_type& TextureExtent = Texture.extent();
			const std::uint32_t BitsPerPixel = detail::bits_per_pixel( Texture.format() );
			PitchInBytes = ( TextureExtent.x * BitsPerPixel ) / 8u;
		}

		memset(Header.Reserved1, 0, sizeof(Header.Reserved1));
		memset(Header.Reserved2, 0, sizeof(Header.Reserved2));
		Header.Size = sizeof(detail::dds_header);
		Header.Flags = Caps;
		Header.Width = static_cast<std::uint32_t>(Texture.extent().x);
		Header.Height = static_cast<std::uint32_t>(Texture.extent().y);
		Header.Pitch = PitchInBytes;
		Header.Depth = static_cast<std::uint32_t>(Texture.extent().z > 1 ? Texture.extent().z : 0);
		Header.MipMapLevels = static_cast<std::uint32_t>(Texture.levels());
		Header.Format.size = sizeof(detail::dds_pixel_format);
		Header.Format.flags = RequireDX10Header ? dx::DDPF_FOURCC : DXFormat.DDPixelFormat;
		Header.Format.fourCC = detail::get_fourcc(RequireDX10Header, Texture.format(), DXFormat);
		Header.Format.bpp = static_cast<std::uint32_t>(detail::bits_per_pixel(Texture.format()));
		Header.Format.Mask = DXFormat.Mask;
		//Header.surfaceFlags = detail::DDSCAPS_TEXTURE | (Storage.levels() > 1 ? detail::DDSCAPS_MIPMAP : 0);
		Header.SurfaceFlags = detail::DDSCAPS_TEXTURE | detail::DDSCAPS_MIPMAP;
		Header.CubemapFlags = 0;

		// Cubemap
		if(Texture.faces() > 1)
		{
			GLI_ASSERT(Texture.faces() == 6);
			Header.CubemapFlags |= detail::DDSCAPS2_CUBEMAP_ALLFACES | detail::DDSCAPS2_CUBEMAP;
		}

		// Texture3D
		if(Texture.extent().z > 1)
			Header.CubemapFlags |= detail::DDSCAPS2_VOLUME;

		if(RequireDX10Header)
		{
			detail::dds_header10& Header10 = *reinterpret_cast<detail::dds_header10*>(&Memory[0] + Offset);
			Offset += sizeof(detail::dds_header10);

			Header10.ArraySize = static_cast<std::uint32_t>(Texture.layers());
			Header10.ResourceDimension = detail::get_dimension(Texture.target());
			Header10.MiscFlag = Texture.faces() > 1 ? detail::D3D10_RESOURCE_MISC_TEXTURECUBE : 0;//Storage.levels() > 0 ? detail::D3D10_RESOURCE_MISC_GENERATE_MIPS : 0;
			Header10.Format = DXFormat.DXGIFormat;
			Header10.AlphaFlags = detail::DDS_ALPHA_MODE_UNKNOWN;
		}

		std::memcpy(&Memory[0] + Offset, Texture.data(), Texture.size());

		return true;
	}

	inline bool save_dds(texture const& Texture, char const* Filename)
	{
		if(Texture.empty())
			return false;

		FILE* File = detail::open_file(Filename, "wb");
		if(!File)
			return false;

		std::vector<char> Memory;
		bool const Result = save_dds(Texture, Memory);

		std::fwrite(&Memory[0], 1, Memory.size(), File);
		std::fclose(File);

		return Result;
	}

	inline bool save_dds(texture const& Texture, std::string const& Filename)
	{
		return save_dds(Texture, Filename.c_str());
	}
}//namespace gli
