namespace gli
{
	inline storage_linear::storage_linear()
		: Layers(0)
		, Faces(0)
		, Levels(0)
		, BlockSize(0)
		, BlockCount(0)
		, BlockExtent(0)
		, Extent(0)
	{}

	inline storage_linear::storage_linear(format_type Format, extent_type const& Extent, size_type Layers, size_type Faces, size_type Levels)
		: Layers(Layers)
		, Faces(Faces)
		, Levels(Levels)
		, BlockSize(gli::block_size(Format))
		, BlockCount(glm::ceilMultiple(Extent, gli::block_extent(Format)) / gli::block_extent(Format))
		, BlockExtent(gli::block_extent(Format))
		, Extent(Extent)
	{
		GLI_ASSERT(Layers > 0);
		GLI_ASSERT(Faces > 0);
		GLI_ASSERT(Levels > 0);
		GLI_ASSERT(glm::all(glm::greaterThan(Extent, extent_type(0))));

		this->Data.resize(this->layer_size(0, Faces - 1, 0, Levels - 1) * Layers, 0);
	}

	inline bool storage_linear::empty() const
	{
		return this->Data.empty();
	}

	inline storage_linear::size_type storage_linear::layers() const
	{
		return this->Layers;
	}

	inline storage_linear::size_type storage_linear::faces() const
	{
		return this->Faces;
	}

	inline storage_linear::size_type storage_linear::levels() const
	{
		return this->Levels;
	}

	inline storage_linear::size_type storage_linear::block_size() const
	{
		return this->BlockSize;
	}

	inline storage_linear::extent_type storage_linear::block_extent() const
	{
		return this->BlockExtent;
	}

	inline storage_linear::extent_type storage_linear::block_count(size_type Level) const
	{
		GLI_ASSERT(Level >= 0 && Level < this->Levels);

		return glm::ceilMultiple(this->extent(Level), BlockExtent) / BlockExtent;
	}

	inline storage_linear::extent_type storage_linear::extent(size_type Level) const
	{
		GLI_ASSERT(Level >= 0 && Level < this->Levels);

		return glm::max(this->Extent >> storage_linear::extent_type(static_cast<storage_linear::extent_type::value_type>(Level)), storage_linear::extent_type(1));
	}

	inline storage_linear::size_type storage_linear::size() const
	{
		GLI_ASSERT(!this->empty());

		return static_cast<size_type>(this->Data.size());
	}

	inline storage_linear::data_type* storage_linear::data()
	{
		GLI_ASSERT(!this->empty());

		return &this->Data[0];
	}

	inline storage_linear::data_type const* const storage_linear::data() const
	{
		GLI_ASSERT(!this->empty());

		return &this->Data[0];
	}

	inline storage_linear::size_type storage_linear::base_offset(size_type Layer, size_type Face, size_type Level) const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(Layer >= 0 && Layer < this->layers() && Face >= 0 && Face < this->faces() && Level >= 0 && Level < this->levels());

		size_type const LayerSize = this->layer_size(0, this->faces() - 1, 0, this->levels() - 1);
		size_type const FaceSize = this->face_size(0, this->levels() - 1);
		size_type BaseOffset = LayerSize * Layer + FaceSize * Face;

		for(size_type LevelIndex = 0, LevelCount = Level; LevelIndex < LevelCount; ++LevelIndex)
			BaseOffset += this->level_size(LevelIndex);

		return BaseOffset;
	}

	inline storage_linear::size_type storage_linear::image_offset(extent1d const& Coord, extent1d const& Extent) const
	{
		GLI_ASSERT(glm::all(glm::lessThan(Coord, Extent)));
		return static_cast<size_t>(Coord.x);
	}

	inline storage_linear::size_type storage_linear::image_offset(extent2d const& Coord, extent2d const& Extent) const
	{
		GLI_ASSERT(glm::all(glm::lessThan(Coord, Extent)));
		return static_cast<size_t>(Coord.x + Coord.y * Extent.x);
	}

	inline storage_linear::size_type storage_linear::image_offset(extent3d const& Coord, extent3d const& Extent) const
	{
		GLI_ASSERT(glm::all(glm::lessThan(Coord, Extent)));
		return static_cast<storage_linear::size_type>(Coord.x + Coord.y * Extent.x + Coord.z * Extent.x * Extent.y);
	}

	inline void storage_linear::copy(
		storage_linear const& StorageSrc,
		size_t LayerSrc, size_t FaceSrc, size_t LevelSrc, extent_type const& BlockIndexSrc,
		size_t LayerDst, size_t FaceDst, size_t LevelDst, extent_type const& BlockIndexDst,
		extent_type const& BlockCount)
	{
		storage_linear::size_type const BaseOffsetSrc = StorageSrc.base_offset(LayerSrc, FaceSrc, LevelSrc);
		storage_linear::size_type const BaseOffsetDst = this->base_offset(LayerDst, FaceDst, LevelDst);
		storage_linear::data_type const* const ImageSrc = StorageSrc.data() + BaseOffsetSrc;
		storage_linear::data_type* const ImageDst = this->data() + BaseOffsetDst;

		for(size_t BlockIndexZ = 0, BlockCountZ = BlockCount.z; BlockIndexZ < BlockCountZ; ++BlockIndexZ)
		for(size_t BlockIndexY = 0, BlockCountY = BlockCount.y; BlockIndexY < BlockCountY; ++BlockIndexY)
		{
			extent_type const BlockIndex(0, BlockIndexY, BlockIndexZ);
			gli::size_t const OffsetSrc = StorageSrc.image_offset(BlockIndexSrc + BlockIndex, StorageSrc.extent(LevelSrc)) * StorageSrc.block_size();
			gli::size_t const OffsetDst = this->image_offset(BlockIndexDst + BlockIndex, this->extent(LevelDst)) * this->block_size();
			storage_linear::data_type const* const DataSrc = ImageSrc + OffsetSrc;
			storage_linear::data_type* DataDst = ImageDst + OffsetDst;
			memcpy(DataDst, DataSrc, this->block_size() * BlockCount.x);
		}
	}

	inline storage_linear::size_type storage_linear::level_size(size_type Level) const
	{
		GLI_ASSERT(Level >= 0 && Level < this->levels());

		return this->BlockSize * glm::compMul(this->block_count(Level));
	}

	inline storage_linear::size_type storage_linear::face_size(size_type BaseLevel, size_type MaxLevel) const
	{
		GLI_ASSERT(MaxLevel >= 0 && MaxLevel < this->levels());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < this->levels());
		GLI_ASSERT(BaseLevel <= MaxLevel);

		size_type FaceSize(0);

		// The size of a face is the sum of the size of each level.
		for(storage_linear::size_type Level(BaseLevel); Level <= MaxLevel; ++Level)
			FaceSize += this->level_size(Level);

		return FaceSize;
	}

	inline storage_linear::size_type storage_linear::layer_size(
		size_type BaseFace, size_type MaxFace,
		size_type BaseLevel, size_type MaxLevel) const
	{
		GLI_ASSERT(MaxFace >= 0 && MaxFace < this->faces());
		GLI_ASSERT(BaseFace >= 0 && BaseFace < this->faces());
		GLI_ASSERT(MaxLevel >= 0 && MaxLevel < this->levels());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < this->levels());

		// The size of a layer is the sum of the size of each face.
		// All the faces have the same size.
		return this->face_size(BaseLevel, MaxLevel) * (MaxFace - BaseFace + 1);
	}
}//namespace gli
