#include "./s3tc.hpp"

namespace gli{
namespace detail
{
	inline void flip(image ImageDst, image ImageSrc, size_t BlockSize)
	{
		size_t const LineSize = BlockSize * ImageDst.extent().x;

		for(int y = 0; y < ImageDst.extent().y; ++y)
		{
			size_t OffsetDst = LineSize * y;
			size_t OffsetSrc = ImageSrc.size() - (LineSize * (y + 1));

			memcpy(
				ImageDst.data<gli::byte>() + OffsetDst,
				ImageSrc.data<gli::byte>() + OffsetSrc,
				LineSize);
		}
	}

	inline void flip_block_s3tc(uint8* BlockDst, uint8* BlockSrc, format Format, bool HeightTwo)
	{
		// There is no distinction between RGB and RGBA in DXT-compressed textures,
		// it is used only to tell OpenGL how to interpret the data.
		// Moreover, in DXT1 (which does not contain an alpha channel), transparency can be emulated
		// using Color0 and Color1 on a per-compression-block basis.
		// There is no difference in how textures with and without transparency are laid out in the file,
		// so they can be flipped using the same method.
		if(Format == FORMAT_RGB_DXT1_UNORM_BLOCK8 || Format == FORMAT_RGB_DXT1_SRGB_BLOCK8
		|| Format == FORMAT_RGBA_DXT1_UNORM_BLOCK8 || Format == FORMAT_RGBA_DXT1_SRGB_BLOCK8)
		{
			dxt1_block* Src = reinterpret_cast<dxt1_block*>(BlockSrc);
			dxt1_block* Dst = reinterpret_cast<dxt1_block*>(BlockDst);

			if(HeightTwo)
			{
				Dst->Color0 = Src->Color0;
				Dst->Color1 = Src->Color1;
				Dst->Row[0] = Src->Row[1];
				Dst->Row[1] = Src->Row[0];
				Dst->Row[2] = Src->Row[2];
				Dst->Row[3] = Src->Row[3];

				return;
			}

			Dst->Color0 = Src->Color0;
			Dst->Color1 = Src->Color1;
			Dst->Row[0] = Src->Row[3];
			Dst->Row[1] = Src->Row[2];
			Dst->Row[2] = Src->Row[1];
			Dst->Row[3] = Src->Row[0];

			return;
		}

		// DXT3
		if(Format == FORMAT_RGBA_DXT3_UNORM_BLOCK16 || Format == FORMAT_RGBA_DXT3_SRGB_BLOCK16)
		{
			dxt3_block* Src = reinterpret_cast<dxt3_block*>(BlockSrc);
			dxt3_block* Dst = reinterpret_cast<dxt3_block*>(BlockDst);

			if(HeightTwo)
			{
				Dst->AlphaRow[0] = Src->AlphaRow[1];
				Dst->AlphaRow[1] = Src->AlphaRow[0];
				Dst->AlphaRow[2] = Src->AlphaRow[2];
				Dst->AlphaRow[3] = Src->AlphaRow[3];
				Dst->Color0 = Src->Color0;
				Dst->Color1 = Src->Color1;
				Dst->Row[0] = Src->Row[1];
				Dst->Row[1] = Src->Row[0];
				Dst->Row[2] = Src->Row[2];
				Dst->Row[3] = Src->Row[3];

				return;
			}

			Dst->AlphaRow[0] = Src->AlphaRow[3];
			Dst->AlphaRow[1] = Src->AlphaRow[2];
			Dst->AlphaRow[2] = Src->AlphaRow[1];
			Dst->AlphaRow[3] = Src->AlphaRow[0];
			Dst->Color0 = Src->Color0;
			Dst->Color1 = Src->Color1;
			Dst->Row[0] = Src->Row[3];
			Dst->Row[1] = Src->Row[2];
			Dst->Row[2] = Src->Row[1];
			Dst->Row[3] = Src->Row[0];

			return;
		}

		// DXT5
		if(Format == FORMAT_RGBA_DXT5_UNORM_BLOCK16 || Format == FORMAT_RGBA_DXT5_SRGB_BLOCK16)
		{
			dxt5_block* Src = reinterpret_cast<dxt5_block*>(BlockSrc);
			dxt5_block* Dst = reinterpret_cast<dxt5_block*>(BlockDst);

			if(HeightTwo)
			{
				Dst->Alpha[0] = Src->Alpha[0];
				Dst->Alpha[1] = Src->Alpha[1];
				// operator+ has precedence over operator>> and operator<<, hence the parentheses. very important!
				// the values below are bitmasks used to retrieve alpha values according to the DXT specification
				// 0xF0 == 0b11110000 and 0xF == 0b1111
				Dst->AlphaBitmap[0] = ((Src->AlphaBitmap[1] & 0xF0) >> 4) + ((Src->AlphaBitmap[2] & 0xF) << 4);
				Dst->AlphaBitmap[1] = ((Src->AlphaBitmap[2] & 0xF0) >> 4) + ((Src->AlphaBitmap[0] & 0xF) << 4);
				Dst->AlphaBitmap[2] = ((Src->AlphaBitmap[0] & 0xF0) >> 4) + ((Src->AlphaBitmap[1] & 0xF) << 4);
				Dst->AlphaBitmap[3] = Src->AlphaBitmap[3];
				Dst->AlphaBitmap[4] = Src->AlphaBitmap[4];
				Dst->AlphaBitmap[5] = Src->AlphaBitmap[5];
				Dst->Color0 = Src->Color0;
				Dst->Color1 = Src->Color1;
				Dst->Row[0] = Src->Row[1];
				Dst->Row[1] = Src->Row[0];
				Dst->Row[2] = Src->Row[2];
				Dst->Row[3] = Src->Row[3];

				return;
			}

			Dst->Alpha[0] = Src->Alpha[0];
			Dst->Alpha[1] = Src->Alpha[1];
			// operator+ has precedence over operator>> and operator<<, hence the parentheses. very important!
			// the values below are bitmasks used to retrieve alpha values according to the DXT specification
			// 0xF0 == 0b11110000 and 0xF == 0b1111
			Dst->AlphaBitmap[0] = ((Src->AlphaBitmap[4] & 0xF0) >> 4) + ((Src->AlphaBitmap[5] & 0xF) << 4);
			Dst->AlphaBitmap[1] = ((Src->AlphaBitmap[5] & 0xF0) >> 4) + ((Src->AlphaBitmap[3] & 0xF) << 4);
			Dst->AlphaBitmap[2] = ((Src->AlphaBitmap[3] & 0xF0) >> 4) + ((Src->AlphaBitmap[4] & 0xF) << 4);
			Dst->AlphaBitmap[3] = ((Src->AlphaBitmap[1] & 0xF0) >> 4) + ((Src->AlphaBitmap[2] & 0xF) << 4);
			Dst->AlphaBitmap[4] = ((Src->AlphaBitmap[2] & 0xF0) >> 4) + ((Src->AlphaBitmap[0] & 0xF) << 4);
			Dst->AlphaBitmap[5] = ((Src->AlphaBitmap[0] & 0xF0) >> 4) + ((Src->AlphaBitmap[1] & 0xF) << 4);
			Dst->Color0 = Src->Color0;
			Dst->Color1 = Src->Color1;
			Dst->Row[0] = Src->Row[3];
			Dst->Row[1] = Src->Row[2];
			Dst->Row[2] = Src->Row[1];
			Dst->Row[3] = Src->Row[0];

			return;
		}

		// invalid format specified (unknown S3TC format?)
		assert(false);
	}

	inline void flip_s3tc(image ImageDst, image ImageSrc, format Format)
	{
		if(ImageSrc.extent().y == 1)
		{
			memcpy(ImageDst.data(),
				   ImageSrc.data(),
				   ImageSrc.size());
			return;
		}

		std::size_t const XBlocks = ImageSrc.extent().x <= 4 ? 1 : ImageSrc.extent().x / 4;
		if(ImageSrc.extent().y == 2)
		{
			for(std::size_t i_block = 0; i_block < XBlocks; ++i_block)
				flip_block_s3tc(ImageDst.data<uint8>() + i_block * block_size(Format), ImageSrc.data<uint8>() + i_block * block_size(Format), Format, true);

			return;
		}

		std::size_t const MaxYBlock = ImageSrc.extent().y / 4 - 1;
		for(std::size_t i_row = 0; i_row <= MaxYBlock; ++i_row)
			for(std::size_t i_block = 0; i_block < XBlocks; ++i_block)
				flip_block_s3tc(ImageDst.data<uint8>() + (MaxYBlock - i_row) * block_size(Format) * XBlocks + i_block * block_size(Format), ImageSrc.data<uint8>() + i_row * block_size(Format) * XBlocks + i_block * block_size(Format), Format, false);
	}

}//namespace detail

/*
template <>
inline image flip(image const & Image)
{

}
*/

template <>
inline texture2d flip(texture2d const& Texture)
{
	GLI_ASSERT(!gli::is_compressed(Texture.format()) || gli::is_s3tc_compressed(Texture.format()));

	texture2d Flip(Texture.format(), Texture.extent(), Texture.levels());

	if(!is_compressed(Texture.format()))
	{
		texture2d::size_type const BlockSize = block_size(Texture.format());

		for(texture2d::size_type Level = 0; Level < Flip.levels(); ++Level)
			detail::flip(Flip[Level], Texture[Level], BlockSize);
	}
	else
		for(texture2d::size_type Level = 0; Level < Flip.levels(); ++Level)
			detail::flip_s3tc(Flip[Level], Texture[Level], Texture.format());

	return Flip;
}

template <>
inline texture2d_array flip(texture2d_array const& Texture)
{
	GLI_ASSERT(!gli::is_compressed(Texture.format()) || gli::is_s3tc_compressed(Texture.format()));

	texture2d_array Flip(Texture.format(), Texture.extent(), Texture.layers(), Texture.levels());

	if(!gli::is_compressed(Texture.format()))
	{
		texture2d_array::size_type const BlockSize = block_size(Texture.format());

		for(texture2d_array::size_type Layer = 0; Layer < Flip.layers(); ++Layer)
		for(texture2d_array::size_type Level = 0; Level < Flip.levels(); ++Level)
			detail::flip(Flip[Layer][Level], Texture[Layer][Level], BlockSize);
	}
	else
		for(texture2d_array::size_type Layer = 0; Layer < Flip.layers(); ++Layer)
		for(texture2d_array::size_type Level = 0; Level < Flip.levels(); ++Level)
			detail::flip_s3tc(Flip[Layer][Level], Texture[Layer][Level], Texture.format());

	return Flip;
}

template <>
inline texture_cube flip(texture_cube const & Texture)
{
	GLI_ASSERT(!gli::is_compressed(Texture.format()) || gli::is_s3tc_compressed(Texture.format()));

	texture_cube Flip(Texture.format(), Texture.extent(), Texture.levels());

	if(!gli::is_compressed(Texture.format()))
	{
		texture_cube::size_type const BlockSize = block_size(Texture.format());

		for(texture_cube::size_type Face = 0; Face < Flip.faces(); ++Face)
		for(texture_cube::size_type Level = 0; Level < Flip.levels(); ++Level)
			detail::flip(Flip[Face][Level], Texture[Face][Level], BlockSize);
	}
	else
		for(texture_cube::size_type Face = 0; Face < Flip.faces(); ++Face)
		for(texture_cube::size_type Level = 0; Level < Flip.levels(); ++Level)
			detail::flip_s3tc(Flip[Face][Level], Texture[Face][Level], Texture.format());

	return Flip;
}

template <>
inline texture_cube_array flip(texture_cube_array const & Texture)
{
	assert(!is_compressed(Texture.format()) || is_s3tc_compressed(Texture.format()));

	texture_cube_array Flip(Texture.format(), Texture.extent(), Texture.layers(), Texture.levels());

	if(!is_compressed(Texture.format()))
	{
		gli::size_t const BlockSize = block_size(Texture.format());

		for(std::size_t Layer = 0; Layer < Flip.layers(); ++Layer)
		for(std::size_t Face = 0; Face < Flip.faces(); ++Face)
		for(std::size_t Level = 0; Level < Flip.levels(); ++Level)
			detail::flip(Flip[Layer][Face][Level], Texture[Layer][Face][Level], BlockSize);
	}
	else
		for(std::size_t Layer = 0; Layer < Flip.layers(); ++Layer)
		for(std::size_t Face = 0; Face < Flip.faces(); ++Face)
		for(std::size_t Level = 0; Level < Flip.levels(); ++Level)
			detail::flip_s3tc(Flip[Layer][Face][Level], Texture[Layer][Face][Level], Texture.format());

	return Flip;
}

template <>
inline texture flip(texture const & Texture)
{
	switch(Texture.target())
	{
	case TARGET_2D:
		return flip(texture2d(Texture));

	case TARGET_2D_ARRAY:
		return flip(texture2d_array(Texture));

	case TARGET_CUBE:
		return flip(texture_cube(Texture));

	case TARGET_CUBE_ARRAY:
		return flip(texture_cube_array(Texture));

	default:
		assert(false && "Texture target does not support flipping.");
		return Texture;
	}
}

}//namespace gli
