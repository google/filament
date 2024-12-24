namespace gli{
namespace detail
{
	inline void duplicate_images
	(
		texture const & Src, texture & Dst,
		texture::size_type BaseLayer, texture::size_type MaxLayer,
		texture::size_type BaseFace, texture::size_type MaxFace,
		texture::size_type BaseLevel, texture::size_type MaxLevel
	)
	{
		GLI_ASSERT(BaseLayer >= 0 && BaseLayer <= MaxLayer && MaxLayer < Src.layers());
		GLI_ASSERT(BaseFace >= 0 && BaseFace <= MaxFace && MaxFace < Src.faces());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel <= MaxLevel && MaxLevel < Src.levels());

		texture::size_type LevelsSize = 0;
		for(texture::size_type LevelIndex = 0; LevelIndex < MaxLevel - BaseLevel + 1; ++LevelIndex)
		{
			GLI_ASSERT(Dst.size(LevelIndex) == Src.size(LevelIndex));
			LevelsSize += Dst.size(LevelIndex);
		}

		for(texture::size_type LayerIndex = 0, LayerCount = MaxLayer - BaseLayer + 1; LayerIndex < LayerCount; ++LayerIndex)
		for(texture::size_type FaceIndex = 0, FaceCount = MaxFace - BaseFace + 1; FaceIndex < FaceCount; ++FaceIndex)
		{
			memcpy(Dst.data(LayerIndex, FaceIndex, BaseLevel), Src.data(BaseLayer + LayerIndex, BaseFace + FaceIndex, BaseLevel), LevelsSize);
		}
	}
}//namespace detail

	inline image duplicate(image const & Image)
	{
		image Result(Image.format(), Image.extent());

		memcpy(Result.data(), Image.data(), Image.size());
		
		return Result;
	}

	template <>
	inline texture duplicate(texture const & Texture)
	{
		texture Duplicate(
			Texture.target(),
			Texture.format(),
			Texture.extent(),
			Texture.layers(),
			Texture.faces(),
			Texture.levels());

		detail::duplicate_images(
			Texture, Duplicate,
			0, Texture.layers() - 1,
			0, Texture.faces() - 1,
			0, Texture.levels() - 1);

		return Duplicate;
	}

	template <typename texType>
	inline texture duplicate(texType const & Texture)
	{
		texture Duplicate(
			Texture.target(),
			Texture.format(),
			Texture.texture::extent(),
			Texture.layers(),
			Texture.faces(),
			Texture.levels());

		detail::duplicate_images(
			Texture, Duplicate,
			0, Texture.layers() - 1,
			0, Texture.faces() - 1,
			0, Texture.levels() - 1);

		return Duplicate;
	}

	template <typename texType>
	inline texture duplicate(texType const & Texture, typename texType::format_type Format)
	{
		GLI_ASSERT(block_size(Texture.format()) == block_size(Format));

		texture Duplicate(
			Texture.target(),
			Format,
			Texture.extent(),
			Texture.layers(),
			Texture.faces(),
			Texture.levels());

		detail::duplicate_images(
			Texture, Duplicate,
			0, Texture.layers() - 1,
			0, Texture.faces() - 1,
			0, Texture.levels() - 1);

		return Duplicate;
	}

	inline texture duplicate
	(
		texture1d const & Texture,
		texture1d::size_type BaseLevel, texture1d::size_type MaxLevel
	)
	{
		GLI_ASSERT(BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseLevel < Texture.levels());
		GLI_ASSERT(MaxLevel < Texture.levels());
	
		texture1d Duplicate(
			Texture.format(),
			Texture.extent(BaseLevel),
			MaxLevel - BaseLevel + 1);

		memcpy(Duplicate.data(), Texture.data(0, 0, BaseLevel), Duplicate.size());

		return Duplicate;
	}

	inline texture duplicate
	(
		texture1d_array const & Texture,
		texture1d_array::size_type BaseLayer, texture1d_array::size_type MaxLayer,
		texture1d_array::size_type BaseLevel, texture1d_array::size_type MaxLevel
	)
	{
		GLI_ASSERT(BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseLevel < Texture.levels());
		GLI_ASSERT(MaxLevel < Texture.levels());
		GLI_ASSERT(BaseLayer <= MaxLayer);
		GLI_ASSERT(BaseLayer < Texture.layers());
		GLI_ASSERT(MaxLayer < Texture.layers());

		texture1d_array Duplicate(
			Texture.format(),
			Texture[BaseLayer].extent(BaseLevel),
			MaxLayer - BaseLayer + 1,
			MaxLevel - BaseLevel + 1);

		for(texture1d_array::size_type Layer = 0; Layer < Duplicate.layers(); ++Layer)
			memcpy(Duplicate.data(Layer, 0, 0), Texture.data(Layer + BaseLayer, 0, BaseLevel), Duplicate[Layer].size());

		return Duplicate;
	}

	inline texture duplicate
	(
		texture2d const & Texture,
		texture2d::size_type BaseLevel, texture2d::size_type MaxLevel
	)
	{
		GLI_ASSERT(BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseLevel < Texture.levels());
		GLI_ASSERT(MaxLevel < Texture.levels());
	
		texture2d Duplicate(
			Texture.format(),
			Texture.extent(BaseLevel),
			MaxLevel - BaseLevel + 1);

		memcpy(Duplicate.data(), Texture.data(0, 0, BaseLevel), Duplicate.size());

		return Duplicate;
	}

	inline texture duplicate
	(
		texture2d_array const & Texture,
		texture2d_array::size_type BaseLayer, texture2d_array::size_type MaxLayer,
		texture2d_array::size_type BaseLevel, texture2d_array::size_type MaxLevel
	)
	{
		GLI_ASSERT(BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseLevel < Texture.levels());
		GLI_ASSERT(MaxLevel < Texture.levels());
		GLI_ASSERT(BaseLayer <= MaxLayer);
		GLI_ASSERT(BaseLayer < Texture.layers());
		GLI_ASSERT(MaxLayer < Texture.layers());

		texture2d_array Duplicate(
			Texture.format(),
			Texture.extent(BaseLevel),
			MaxLayer - BaseLayer + 1,
			MaxLevel - BaseLevel + 1);

		for(texture2d_array::size_type Layer = 0; Layer < Duplicate.layers(); ++Layer)
			memcpy(Duplicate.data(Layer, 0, 0), Texture.data(Layer + BaseLayer, 0, BaseLevel), Duplicate[Layer].size());

		return Duplicate;
	}

	inline texture duplicate
	(
		texture3d const & Texture,
		texture3d::size_type BaseLevel, texture3d::size_type MaxLevel
	)
	{
		GLI_ASSERT(BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseLevel < Texture.levels());
		GLI_ASSERT(MaxLevel < Texture.levels());

		texture3d Duplicate(
			Texture.format(),
			Texture.extent(BaseLevel),
			MaxLevel - BaseLevel + 1);

		memcpy(Duplicate.data(), Texture.data(0, 0, BaseLevel), Duplicate.size());

		return Duplicate;
	}

	inline texture duplicate
	(
		texture_cube const & Texture,
		texture_cube::size_type BaseFace, texture_cube::size_type MaxFace,
		texture_cube::size_type BaseLevel, texture_cube::size_type MaxLevel
	)
	{
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < Texture.levels() && BaseLevel <= MaxLevel && MaxLevel < Texture.levels());
		GLI_ASSERT(BaseFace <= MaxFace);
		GLI_ASSERT(BaseFace < Texture.faces());
		GLI_ASSERT(MaxFace < Texture.faces());

		texture_cube Duplicate(
			Texture.format(),
			Texture[BaseFace].extent(BaseLevel),
			MaxLevel - BaseLevel + 1);

		for(texture_cube::size_type Face = 0; Face < Duplicate.faces(); ++Face)
			memcpy(Duplicate[Face].data(), Texture[Face + BaseFace][BaseLevel].data(), Duplicate[Face].size());

		return Duplicate;
	}

	inline texture duplicate
	(
		texture_cube_array const & Texture,
		texture_cube_array::size_type BaseLayer, texture_cube_array::size_type MaxLayer,
		texture_cube_array::size_type BaseFace, texture_cube_array::size_type MaxFace,
		texture_cube_array::size_type BaseLevel, texture_cube_array::size_type MaxLevel
	)
	{
		GLI_ASSERT(BaseLevel <= MaxLevel);
		GLI_ASSERT(BaseLevel < Texture.levels());
		GLI_ASSERT(MaxLevel < Texture.levels());
		GLI_ASSERT(BaseFace <= MaxFace);
		GLI_ASSERT(BaseFace < Texture.faces());
		GLI_ASSERT(MaxFace < Texture.faces());
		GLI_ASSERT(BaseLayer <= MaxLayer);
		GLI_ASSERT(BaseLayer < Texture.layers());
		GLI_ASSERT(MaxLayer < Texture.layers());

		texture_cube_array Duplicate(
			Texture.format(),
			Texture[BaseLayer][BaseFace].extent(BaseLevel),
			MaxLayer - BaseLayer + 1,
			MaxLevel - BaseLevel + 1);

		for(texture_cube_array::size_type Layer = 0; Layer < Duplicate.layers(); ++Layer)
		for(texture_cube_array::size_type Face = 0; Face < Duplicate[Layer].faces(); ++Face)
			memcpy(Duplicate[Layer][Face].data(), Texture[Layer + BaseLayer][Face + BaseFace][BaseLevel].data(), Duplicate[Layer][Face].size());

		return Duplicate;
	}
}//namespace gli
