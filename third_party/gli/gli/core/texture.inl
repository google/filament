#include <cstring>

namespace gli
{
	inline texture::texture()
		: Storage(nullptr)
		, Target(static_cast<gli::target>(TARGET_INVALID))
		, Format(gli::FORMAT_UNDEFINED)
		, BaseLayer(0), MaxLayer(0)
		, BaseFace(0), MaxFace(0)
		, BaseLevel(0), MaxLevel(0)
		, Swizzles(SWIZZLE_ZERO)
		, Cache(cache::DEFAULT)
	{}

	inline texture::texture
	(
		target_type Target,
		format_type Format,
		extent_type const& Extent,
		size_type Layers,
		size_type Faces,
		size_type Levels,
		swizzles_type const& Swizzles
	)
		: Storage(std::make_shared<storage_type>(Format, Extent, Layers, Faces, Levels))
		, Target(Target)
		, Format(Format)
		, BaseLayer(0), MaxLayer(Layers - 1)
		, BaseFace(0), MaxFace(Faces - 1)
		, BaseLevel(0), MaxLevel(Levels - 1)
		, Swizzles(Swizzles)
		, Cache(*Storage, Format, this->base_layer(), this->layers(), this->base_face(), this->max_face(), this->base_level(), this->max_level())
	{
		GLI_ASSERT(Target != TARGET_CUBE || (Target == TARGET_CUBE && Extent.x == Extent.y));
		GLI_ASSERT(Target != TARGET_CUBE_ARRAY || (Target == TARGET_CUBE_ARRAY && Extent.x == Extent.y));
	}

	inline texture::texture
	(
		texture const& Texture,
		target_type Target,
		format_type Format,
		size_type BaseLayer, size_type MaxLayer,
		size_type BaseFace, size_type MaxFace,
		size_type BaseLevel, size_type MaxLevel,
		swizzles_type const& Swizzles
	)
		: Storage(Texture.Storage)
		, Target(Target)
		, Format(Format)
		, BaseLayer(BaseLayer), MaxLayer(MaxLayer)
		, BaseFace(BaseFace), MaxFace(MaxFace)
		, BaseLevel(BaseLevel), MaxLevel(MaxLevel)
		, Swizzles(Swizzles)
		, Cache(*Storage, Format, this->base_layer(), this->layers(), this->base_face(), this->max_face(), this->base_level(), this->max_level())
	{
		GLI_ASSERT(block_size(Format) == block_size(Texture.format()));
		GLI_ASSERT(Target != TARGET_1D || (Target == TARGET_1D && this->layers() == 1 && this->faces() == 1 && this->extent().y == 1 && this->extent().z == 1));
		GLI_ASSERT(Target != TARGET_1D_ARRAY || (Target == TARGET_1D_ARRAY && this->layers() >= 1 && this->faces() == 1 && this->extent().y == 1 && this->extent().z == 1));
		GLI_ASSERT(Target != TARGET_2D || (Target == TARGET_2D && this->layers() == 1 && this->faces() == 1 && this->extent().y >= 1 && this->extent().z == 1));
		GLI_ASSERT(Target != TARGET_2D_ARRAY || (Target == TARGET_2D_ARRAY && this->layers() >= 1 && this->faces() == 1 && this->extent().y >= 1 && this->extent().z == 1));
		GLI_ASSERT(Target != TARGET_3D || (Target == TARGET_3D && this->layers() == 1 && this->faces() == 1 && this->extent().y >= 1 && this->extent().z >= 1));
		GLI_ASSERT(Target != TARGET_CUBE || (Target == TARGET_CUBE && this->layers() == 1 && this->faces() >= 1 && this->extent().y >= 1 && this->extent().z == 1));
		GLI_ASSERT(Target != TARGET_CUBE_ARRAY || (Target == TARGET_CUBE_ARRAY && this->layers() >= 1 && this->faces() >= 1 && this->extent().y >= 1 && this->extent().z == 1));
	}

	inline texture::texture
	(
		texture const& Texture,
		target_type Target,
		format_type Format,
		swizzles_type const& Swizzles
	)
		: Storage(Texture.Storage)
		, Target(Target)
		, Format(Format)
		, BaseLayer(Texture.base_layer()), MaxLayer(Texture.max_layer())
		, BaseFace(Texture.base_face()), MaxFace(Texture.max_face())
		, BaseLevel(Texture.base_level()), MaxLevel(Texture.max_level())
		, Swizzles(Swizzles)
		, Cache(*Storage, Format, this->base_layer(), this->layers(), this->base_face(), this->max_face(), this->base_level(), this->max_level())
	{
		if(this->empty())
			return;

		GLI_ASSERT(Target != TARGET_1D || (Target == TARGET_1D && this->layers() == 1 && this->faces() == 1 && this->extent().y == 1 && this->extent().z == 1));
		GLI_ASSERT(Target != TARGET_1D_ARRAY || (Target == TARGET_1D_ARRAY && this->layers() >= 1 && this->faces() == 1 && this->extent().y == 1 && this->extent().z == 1));
		GLI_ASSERT(Target != TARGET_2D || (Target == TARGET_2D && this->layers() == 1 && this->faces() == 1 && this->extent().y >= 1 && this->extent().z == 1));
		GLI_ASSERT(Target != TARGET_2D_ARRAY || (Target == TARGET_2D_ARRAY && this->layers() >= 1 && this->faces() == 1 && this->extent().y >= 1 && this->extent().z == 1));
		GLI_ASSERT(Target != TARGET_3D || (Target == TARGET_3D && this->layers() == 1 && this->faces() == 1 && this->extent().y >= 1 && this->extent().z >= 1));
		GLI_ASSERT(Target != TARGET_CUBE || (Target == TARGET_CUBE && this->layers() == 1 && this->faces() >= 1 && this->extent().y >= 1 && this->extent().z == 1));
		GLI_ASSERT(Target != TARGET_CUBE_ARRAY || (Target == TARGET_CUBE_ARRAY && this->layers() >= 1 && this->faces() >= 1 && this->extent().y >= 1 && this->extent().z == 1));
	}

	inline bool texture::empty() const
	{
		if(this->Storage.get() == nullptr)
			return true;

		return this->Storage->empty();
	}

	inline texture::format_type texture::format() const
	{
		return this->Format;
	}

	inline texture::swizzles_type texture::swizzles() const
	{
		swizzles_type const FormatSwizzle = detail::get_format_info(this->format()).Swizzles;
		swizzles_type const CustomSwizzle = this->Swizzles;

		swizzles_type ResultSwizzle(SWIZZLE_ZERO);
		ResultSwizzle.r = is_channel(CustomSwizzle.r) ? FormatSwizzle[CustomSwizzle.r] : CustomSwizzle.r;
		ResultSwizzle.g = is_channel(CustomSwizzle.g) ? FormatSwizzle[CustomSwizzle.g] : CustomSwizzle.g;
		ResultSwizzle.b = is_channel(CustomSwizzle.b) ? FormatSwizzle[CustomSwizzle.b] : CustomSwizzle.b;
		ResultSwizzle.a = is_channel(CustomSwizzle.a) ? FormatSwizzle[CustomSwizzle.a] : CustomSwizzle.a;
		return ResultSwizzle;
	}

	inline texture::size_type texture::base_layer() const
	{
		return this->BaseLayer;
	}

	inline texture::size_type texture::max_layer() const
	{
		return this->MaxLayer;
	}

	inline texture::size_type texture::layers() const
	{
		if(this->empty())
			return 0;
		return this->max_layer() - this->base_layer() + 1;
	}

	inline texture::size_type texture::base_face() const
	{
		return this->BaseFace;
	}

	inline texture::size_type texture::max_face() const
	{
		return this->MaxFace;
	}

	inline texture::size_type texture::faces() const
	{
		if(this->empty())
			return 0;
		return this->max_face() - this->base_face() + 1;
	}

	inline texture::size_type texture::base_level() const
	{
		return this->BaseLevel;
	}

	inline texture::size_type texture::max_level() const
	{
		return this->MaxLevel;
	}

	inline texture::size_type texture::levels() const
	{
		if(this->empty())
			return 0;
		return this->max_level() - this->base_level() + 1;
	}

	inline texture::size_type texture::size() const
	{
		GLI_ASSERT(!this->empty());

		return this->Cache.get_memory_size();
	}

	template <typename gen_type>
	inline texture::size_type texture::size() const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(block_size(this->format()) == sizeof(gen_type));

		return this->size() / sizeof(gen_type);
	}

	inline texture::size_type texture::size(size_type Level) const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(Level >= 0 && Level < this->levels());

		return this->Cache.get_memory_size(Level);
	}

	template <typename gen_type>
	inline texture::size_type texture::size(size_type Level) const
	{
		GLI_ASSERT(block_size(this->format()) == sizeof(gen_type));

		return this->size(Level) / sizeof(gen_type);
	}

	inline void* texture::data()
	{
		GLI_ASSERT(!this->empty());

		return this->Cache.get_base_address(0, 0, 0);
	}

	inline void const* texture::data() const
	{
		GLI_ASSERT(!this->empty());

		return this->Cache.get_base_address(0, 0, 0);
	}

	template <typename gen_type>
	inline gen_type* texture::data()
	{
		GLI_ASSERT(block_size(this->format()) >= sizeof(gen_type));

		return reinterpret_cast<gen_type*>(this->data());
	}

	template <typename gen_type>
	inline gen_type const* texture::data() const
	{
		GLI_ASSERT(block_size(this->format()) >= sizeof(gen_type));

		return reinterpret_cast<gen_type const*>(this->data());
	}

	inline void* texture::data(size_type Layer, size_type Face, size_type Level)
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(Layer >= 0 && Layer < this->layers() && Face >= 0 && Face < this->faces() && Level >= 0 && Level < this->levels());

		return this->Cache.get_base_address(Layer, Face, Level);
	}

	inline void const* const texture::data(size_type Layer, size_type Face, size_type Level) const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(Layer >= 0 && Layer < this->layers() && Face >= 0 && Face < this->faces() && Level >= 0 && Level < this->levels());

		return this->Cache.get_base_address(Layer, Face, Level);
	}

	template <typename gen_type>
	inline gen_type* texture::data(size_type Layer, size_type Face, size_type Level)
	{
		GLI_ASSERT(block_size(this->format()) >= sizeof(gen_type));

		return reinterpret_cast<gen_type*>(this->data(Layer, Face, Level));
	}

	template <typename gen_type>
	inline gen_type const* const texture::data(size_type Layer, size_type Face, size_type Level) const
	{
		GLI_ASSERT(block_size(this->format()) >= sizeof(gen_type));

		return reinterpret_cast<gen_type const* const>(this->data(Layer, Face, Level));
	}

	inline texture::extent_type texture::extent(size_type Level) const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(Level >= 0 && Level < this->levels());

		return this->Cache.get_extent(Level);
	}

	inline void texture::clear()
	{
		GLI_ASSERT(!this->empty());

		memset(this->data(), 0, this->size());
	}

	template <typename gen_type>
	inline void texture::clear(gen_type const& Texel)
	{
		GLI_ASSERT(!gli::is_compressed(this->format()));
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(block_size(this->format()) == sizeof(gen_type));

		gen_type* Data = this->data<gen_type>();
		size_type const BlockCount = this->size<gen_type>();

		for(size_type BlockIndex = 0; BlockIndex < BlockCount; ++BlockIndex)
			*(Data + BlockIndex) = Texel;
	}

	template <typename gen_type>
	inline void texture::clear(size_type Layer, size_type Face, size_type Level, gen_type const& BlockData)
	{
		GLI_ASSERT(!gli::is_compressed(this->format()));
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(block_size(this->format()) == sizeof(gen_type));
		GLI_ASSERT(Layer >= 0 && Layer < this->layers() && Face >= 0 && Face < this->faces() && Level >= 0 && Level < this->levels());

		size_type const BlockCount = this->Storage->level_size(Level) / sizeof(gen_type);
		gen_type* Data = this->data<gen_type>(Layer, Face, Level);
		for(size_type BlockIndex = 0; BlockIndex < BlockCount; ++BlockIndex)
			*(Data + BlockIndex) = BlockData;
	}

	template <typename gen_type>
	inline void texture::clear
	(
		size_type Layer, size_type Face, size_type Level,
		extent_type const& TexelOffset, extent_type const& TexelExtent,
		gen_type const& BlockData
	)
	{
		storage_type::size_type const BaseOffset = this->Storage->base_offset(Layer, Face, Level);
		storage_type::data_type* const BaseAddress = this->Storage->data() + BaseOffset;

		extent_type BlockOffset(TexelOffset / this->Storage->block_extent());
		extent_type const BlockExtent(TexelExtent / this->Storage->block_extent() + BlockOffset);
		for(; BlockOffset.z < BlockExtent.z; ++BlockOffset.z)
		for(; BlockOffset.y < BlockExtent.y; ++BlockOffset.y)
		for(; BlockOffset.x < BlockExtent.x; ++BlockOffset.x)
		{
			gli::size_t const Offset = this->Storage->image_offset(BlockOffset, this->extent(Level)) * this->Storage->block_size();
			gen_type* const BlockAddress = reinterpret_cast<gen_type* const>(BaseAddress + Offset);
			*BlockAddress = BlockData;
		}
	}

	inline void texture::copy
	(
		texture const& TextureSrc,
		size_t LayerSrc, size_t FaceSrc, size_t LevelSrc,
		size_t LayerDst, size_t FaceDst, size_t LevelDst
	)
	{
		GLI_ASSERT(this->size(LevelDst) == TextureSrc.size(LevelSrc));
		GLI_ASSERT(LayerSrc < TextureSrc.layers());
		GLI_ASSERT(LayerDst < this->layers());
		GLI_ASSERT(FaceSrc < TextureSrc.faces());
		GLI_ASSERT(FaceDst < this->faces());
		GLI_ASSERT(LevelSrc < TextureSrc.levels());
		GLI_ASSERT(LevelDst < this->levels());

		memcpy(
			this->data(LayerDst, FaceDst, LevelDst),
			TextureSrc.data(LayerSrc, FaceSrc, LevelSrc),
			this->size(LevelDst));
	}

	inline void texture::copy
	(
		texture const& TextureSrc,
		size_t LayerSrc, size_t FaceSrc, size_t LevelSrc, texture::extent_type const& OffsetSrc,
		size_t LayerDst, size_t FaceDst, size_t LevelDst, texture::extent_type const& OffsetDst,
		texture::extent_type const& Extent
	)
	{
		storage_type::extent_type const BlockExtent = this->Storage->block_extent();
		this->Storage->copy(
			*TextureSrc.Storage,
			LayerSrc, FaceSrc, LevelSrc, OffsetSrc / BlockExtent,
			LayerDst, FaceDst, LevelDst, OffsetDst / BlockExtent,
			Extent / BlockExtent);
	}

	template <typename gen_type>
	inline void texture::swizzle(gli::swizzles const& Swizzles)
	{
		for(size_type TexelIndex = 0, TexelCount = this->size<gen_type>(); TexelIndex < TexelCount; ++TexelIndex)
		{
			gen_type& TexelDst = *(this->data<gen_type>() + TexelIndex);
			gen_type const TexelSrc = TexelDst;
			for(typename gen_type::length_type Component = 0; Component < TexelDst.length(); ++Component)
			{
				GLI_ASSERT(static_cast<typename gen_type::length_type>(Swizzles[Component]) < TexelDst.length());
				TexelDst[Component] = TexelSrc[Swizzles[Component]];
			}
		}
	}

	template <typename gen_type>
	inline gen_type texture::load(extent_type const& TexelCoord, size_type Layer,  size_type Face, size_type Level) const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(!is_compressed(this->format()));
		GLI_ASSERT(block_size(this->format()) == sizeof(gen_type));

		size_type const ImageOffset = this->Storage->image_offset(TexelCoord, this->extent(Level));
		GLI_ASSERT(ImageOffset < this->size<gen_type>(Level));

		return *(this->data<gen_type>(Layer, Face, Level) + ImageOffset);
	}

	template <typename gen_type>
	inline void texture::store(extent_type const& TexelCoord, size_type Layer,  size_type Face, size_type Level, gen_type const& Texel)
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(!is_compressed(this->format()));
		GLI_ASSERT(block_size(this->format()) == sizeof(gen_type));
		GLI_ASSERT(glm::all(glm::lessThan(TexelCoord, this->extent(Level))));

		size_type const ImageOffset = this->Storage->image_offset(TexelCoord, this->extent(Level));
		GLI_ASSERT(ImageOffset < this->size<gen_type>(Level));

		*(this->data<gen_type>(Layer, Face, Level) + ImageOffset) = Texel;
	}
}//namespace gli

