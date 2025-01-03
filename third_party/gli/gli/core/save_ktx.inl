#include <cstdio>
#include <glm/gtc/round.hpp>
#include "../load_ktx.hpp"
#include "file.hpp"

namespace gli{
namespace detail
{
	inline texture::size_type compute_ktx_storage_size(texture const & Texture)
	{
		texture::size_type const BlockSize = block_size(Texture.format());
		texture::size_type TotalSize = sizeof(detail::FOURCC_KTX10) + sizeof(detail::ktx_header10);

		for(texture::size_type Level = 0, Levels = Texture.levels(); Level < Levels; ++Level)
		{
			TotalSize += sizeof(std::uint32_t);

			for(texture::size_type Layer = 0, Layers = Texture.layers(); Layer < Layers; ++Layer)
			for(texture::size_type Face = 0, Faces = Texture.faces(); Face < Faces; ++Face)
			{
				texture::size_type const FaceSize = Texture.size(Level);
				texture::size_type const PaddedSize = std::max(BlockSize, glm::ceilMultiple(FaceSize, static_cast<texture::size_type>(4)));

				TotalSize += PaddedSize;
			}
		}

		return TotalSize;
	}
}//namespace detail

	inline bool save_ktx(texture const& Texture, std::vector<char>& Memory)
	{
		if(Texture.empty())
			return false;

		gl GL(gl::PROFILE_KTX);
		gl::format const& Format = GL.translate(Texture.format(), Texture.swizzles());
		target const Target = Texture.target();

		detail::formatInfo const& Desc = detail::get_format_info(Texture.format());

		Memory.resize(detail::compute_ktx_storage_size(Texture));

		std::memcpy(&Memory[0], detail::FOURCC_KTX10, sizeof(detail::FOURCC_KTX10));

		std::size_t Offset = sizeof(detail::FOURCC_KTX10);

		detail::ktx_header10& Header = *reinterpret_cast<detail::ktx_header10*>(&Memory[0] + Offset);
		Header.Endianness = 0x04030201;
		Header.GLType = Format.Type;
		Header.GLTypeSize = Format.Type == gl::TYPE_NONE ? 1 : Desc.BlockSize;
		Header.GLFormat = Format.External;
		Header.GLInternalFormat = Format.Internal;
		Header.GLBaseInternalFormat = Format.External;
		Header.PixelWidth = static_cast<std::uint32_t>(Texture.extent().x);
		Header.PixelHeight = !is_target_1d(Target) ? static_cast<std::uint32_t>(Texture.extent().y) : 0;
		Header.PixelDepth = Target == TARGET_3D ? static_cast<std::uint32_t>(Texture.extent().z) : 0;
		Header.NumberOfArrayElements = is_target_array(Target) ? static_cast<std::uint32_t>(Texture.layers()) : 0;
		Header.NumberOfFaces = is_target_cube(Target) ? static_cast<std::uint32_t>(Texture.faces()) : 1;
		Header.NumberOfMipmapLevels = static_cast<std::uint32_t>(Texture.levels());
		Header.BytesOfKeyValueData = 0;

		Offset += sizeof(detail::ktx_header10);

		for(texture::size_type Level = 0, Levels = Texture.levels(); Level < Levels; ++Level)
		{
			std::uint32_t& ImageSize = *reinterpret_cast<std::uint32_t*>(&Memory[0] + Offset);
			Offset += sizeof(std::uint32_t);

			for(texture::size_type Layer = 0, Layers = Texture.layers(); Layer < Layers; ++Layer)
			for(texture::size_type Face = 0, Faces = Texture.faces(); Face < Faces; ++Face)
			{
				texture::size_type const FaceSize = Texture.size(Level);

				std::memcpy(&Memory[0] + Offset, Texture.data(Layer, Face, Level), FaceSize);

				texture::size_type const PaddedSize = glm::ceilMultiple(FaceSize, static_cast<texture::size_type>(4));

				ImageSize += static_cast<std::uint32_t>(PaddedSize);
				Offset += PaddedSize;

				GLI_ASSERT(Offset <= Memory.size());
			}

			ImageSize = glm::ceilMultiple(ImageSize, static_cast<std::uint32_t>(4));
		}

		return true;
	}

	inline bool save_ktx(texture const& Texture, char const* Filename)
	{
		if(Texture.empty())
			return false;

		FILE* File = detail::open_file(Filename, "wb");
		if(!File)
			return false;

		std::vector<char> Memory;
		bool const Result = save_ktx(Texture, Memory);

		std::fwrite(&Memory[0], 1, Memory.size(), File);
		std::fclose(File);

		return Result;
	}

	inline bool save_ktx(texture const& Texture, std::string const& Filename)
	{
		return save_ktx(Texture, Filename.c_str());
	}
}//namespace gli
