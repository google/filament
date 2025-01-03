#include "../gl.hpp"
#include "file.hpp"
#include <cstdio>
#include <cassert>

namespace gli{
namespace detail
{
	static unsigned char const FOURCC_KTX10[] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};
	static unsigned char const FOURCC_KTX20[] = {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A};

	struct ktx_header10
	{
		std::uint32_t Endianness;
		std::uint32_t GLType;
		std::uint32_t GLTypeSize;
		std::uint32_t GLFormat;
		std::uint32_t GLInternalFormat;
		std::uint32_t GLBaseInternalFormat;
		std::uint32_t PixelWidth;
		std::uint32_t PixelHeight;
		std::uint32_t PixelDepth;
		std::uint32_t NumberOfArrayElements;
		std::uint32_t NumberOfFaces;
		std::uint32_t NumberOfMipmapLevels;
		std::uint32_t BytesOfKeyValueData;
	};

	inline target get_target(ktx_header10 const& Header)
	{
		if(Header.NumberOfFaces > 1)
		{
			if(Header.NumberOfArrayElements > 0)
				return TARGET_CUBE_ARRAY;
			else
				return TARGET_CUBE;
		}
		else if(Header.NumberOfArrayElements > 0)
		{
			if(Header.PixelHeight == 0)
				return TARGET_1D_ARRAY;
			else
				return TARGET_2D_ARRAY;
		}
		else if(Header.PixelHeight == 0)
			return TARGET_1D;
		else if(Header.PixelDepth > 0)
			return TARGET_3D;
		else
			return TARGET_2D;
	}

	inline texture load_ktx10(char const* Data, std::size_t Size)
	{
		detail::ktx_header10 const & Header(*reinterpret_cast<detail::ktx_header10 const*>(Data));

		size_t Offset = sizeof(detail::ktx_header10);

		// Skip key value data
		Offset += Header.BytesOfKeyValueData;

		gl GL(gl::PROFILE_KTX);
		gli::format const Format = GL.find(
			static_cast<gli::gl::internal_format>(Header.GLInternalFormat),
			static_cast<gli::gl::external_format>(Header.GLFormat),
			static_cast<gli::gl::type_format>(Header.GLType));
		GLI_ASSERT(Format != gli::FORMAT_UNDEFINED);

		texture::size_type const BlockSize = block_size(Format);

		texture Texture(
			detail::get_target(Header),
			Format,
			texture::extent_type(
				Header.PixelWidth,
				std::max<texture::size_type>(Header.PixelHeight, 1),
				std::max<texture::size_type>(Header.PixelDepth, 1)),
			std::max<texture::size_type>(Header.NumberOfArrayElements, 1),
			std::max<texture::size_type>(Header.NumberOfFaces, 1),
			std::max<texture::size_type>(Header.NumberOfMipmapLevels, 1));

		for(texture::size_type Level = 0, Levels = Texture.levels(); Level < Levels; ++Level)
		{
			Offset += sizeof(std::uint32_t);

			for(texture::size_type Layer = 0, Layers = Texture.layers(); Layer < Layers; ++Layer)
			for(texture::size_type Face = 0, Faces = Texture.faces(); Face < Faces; ++Face)
			{
				texture::size_type const FaceSize = Texture.size(Level);

				std::memcpy(Texture.data(Layer, Face, Level), Data + Offset, FaceSize);

				Offset += std::max(BlockSize, glm::ceilMultiple(FaceSize, static_cast<texture::size_type>(4)));
			}
		}

		return Texture;
	}
}//namespace detail

	inline texture load_ktx(char const* Data, std::size_t Size)
	{
		GLI_ASSERT(Data && (Size >= sizeof(detail::ktx_header10)));

		// KTX10
		{
			if(memcmp(Data, detail::FOURCC_KTX10, sizeof(detail::FOURCC_KTX10)) == 0)
				return detail::load_ktx10(Data + sizeof(detail::FOURCC_KTX10), Size - sizeof(detail::FOURCC_KTX10));
		}

		return texture();
	}

	inline texture load_ktx(char const* Filename)
	{
		FILE* File = detail::open_file(Filename, "rb");
		if(!File)
			return texture();

		long Beg = std::ftell(File);
		std::fseek(File, 0, SEEK_END);
		long End = std::ftell(File);
		std::fseek(File, 0, SEEK_SET);

		std::vector<char> Data(static_cast<std::size_t>(End - Beg));

		std::fread(&Data[0], 1, Data.size(), File);
		std::fclose(File);

		return load_ktx(&Data[0], Data.size());
	}

	inline texture load_ktx(std::string const& Filename)
	{
		return load_ktx(Filename.c_str());
	}
}//namespace gli
