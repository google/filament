#pragma once

#include "convert_func.hpp"

namespace gli{
namespace detail
{
	template <typename textureType, typename T, qualifier P>
	struct clear
	{
		static void call(textureType & Texture, typename convert<textureType, T, P>::writeFunc Write, vec<4, T, P> const& Color)
		{
			GLI_ASSERT(Write);

			texture const ConvertTexel(Texture.target(), Texture.format(), texture::extent_type(1), 1, 1, 1);
			textureType Texel(ConvertTexel);
			Write(Texel, typename textureType::extent_type(0), 0, 0, 0, Color);

			size_t const BlockSize(block_size(Texture.format()));
			for(size_t BlockIndex = 0, BlockCount = Texture.size() / BlockSize; BlockIndex < BlockCount; ++BlockIndex)
				memcpy(static_cast<std::uint8_t*>(Texture.data()) + BlockSize * BlockIndex, Texel.data(), BlockSize);
		}
	};
}//namespace detail
}//namespace gli
