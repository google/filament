#include <cstring>

namespace gli{
namespace detail
{
	inline bool equalData(texture const & TextureA, texture const & TextureB)
	{
		GLI_ASSERT(TextureA.size() == TextureB.size());

		if(TextureA.data() == TextureB.data())
			return true;

		for(texture::size_type LayerIndex = 0, LayerCount = TextureA.layers(); LayerIndex < LayerCount; ++LayerIndex)
		for(texture::size_type FaceIndex = 0, FaceCount = TextureA.faces(); FaceIndex < FaceCount; ++FaceIndex)
		for(texture::size_type LevelIndex = 0, LevelCount = TextureA.levels(); LevelIndex < LevelCount; ++LevelIndex)
		{
			void const* PointerA = TextureA.data(LayerIndex, FaceIndex, LevelIndex);
			void const* PointerB = TextureB.data(LayerIndex, FaceIndex, LevelIndex);
			GLI_ASSERT(TextureA.size(LevelIndex) == TextureB.size(LevelIndex));
			if(std::memcmp(PointerA, PointerB, TextureA.size(LevelIndex)) != 0)
				return false;
		}

		return true;
	}
}//namespace detail

	inline bool operator==(image const & ImageA, image const & ImageB)
	{
		if(!glm::all(glm::equal(ImageA.extent(), ImageB.extent())))
			return false;
		if(ImageA.size() != ImageB.size())
			return false;

		return std::memcmp(ImageA.data(), ImageB.data(), ImageA.size()) == 0;
	}

	inline bool operator!=(image const & ImageA, image const & ImageB)
	{
		if(!glm::all(glm::equal(ImageA.extent(), ImageB.extent())))
			return true;
		if(ImageA.size() != ImageB.size())
			return true;

		return std::memcmp(ImageA.data(), ImageB.data(), ImageA.size()) != 0;
	}

	inline bool equal(texture const & TextureA, texture const & TextureB)
	{
		if(TextureA.empty() && TextureB.empty())
			return true;
		if(TextureA.empty() != TextureB.empty())
			return false;
		if(TextureA.target() != TextureB.target())
			return false;
		if(TextureA.layers() != TextureB.layers())
			return false;
		if(TextureA.faces() != TextureB.faces())
			return false;
		if(TextureA.levels() != TextureB.levels())
			return false;
		if(TextureA.format() != TextureB.format())
			return false;
		if(TextureA.size() != TextureB.size())
			return false;

		return detail::equalData(TextureA, TextureB);
	}

	inline bool notEqual(texture const & TextureA, texture const & TextureB)
	{
		if(TextureA.empty() && TextureB.empty())
			return false;
		if(TextureA.empty() != TextureB.empty())
			return true;
		if(TextureA.target() != TextureB.target())
			return true;
		if(TextureA.layers() != TextureB.layers())
			return true;
		if(TextureA.faces() != TextureB.faces())
			return true;
		if(TextureA.levels() != TextureB.levels())
			return true;
		if(TextureA.format() != TextureB.format())
			return true;
		if(TextureA.size() != TextureB.size())
			return true;

		return !detail::equalData(TextureA, TextureB);
	}

	inline bool operator==(texture const & A, texture const & B)
	{
		return gli::equal(A, B);
	}

	inline bool operator!=(texture const & A, texture const & B)
	{
		return gli::notEqual(A, B);
	}
}//namespace gli
