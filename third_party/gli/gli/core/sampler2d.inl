#include "clear.hpp"
#include <glm/gtc/integer.hpp>
#include <glm/exponential.hpp>
#include <glm/vector_relational.hpp>

namespace gli
{
	template <typename T, qualifier P>
	inline sampler2d<T, P>::sampler2d(texture_type const & Texture, wrap Wrap, filter Mip, filter Min, texel_type const & BorderColor)
		: sampler(Wrap, Texture.levels() > 1 ? Mip : FILTER_NEAREST, Min)
		, Texture(Texture)
		, Convert(detail::convert<texture_type, T, P>::call(this->Texture.format()))
		, BorderColor(BorderColor)
		, Filter(detail::get_filter<filter_type, detail::DIMENSION_2D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, T>(Mip, Min, is_border(Wrap)))
	{
		GLI_ASSERT(!Texture.empty());
		GLI_ASSERT((!std::numeric_limits<T>::is_iec559 && Mip == FILTER_NEAREST && Min == FILTER_NEAREST) || std::numeric_limits<T>::is_iec559);
	}

	template <typename T, qualifier P>
	inline typename sampler2d<T, P>::texture_type const & sampler2d<T, P>::operator()() const
	{
		return this->Texture;
	}

	template <typename T, qualifier P>
	inline typename sampler2d<T, P>::texel_type sampler2d<T, P>::texel_fetch(extent_type const & TexelCoord, size_type const & Level) const
	{
		GLI_ASSERT(!this->Texture.empty());
		GLI_ASSERT(this->Convert.Fetch);

		return this->Convert.Fetch(this->Texture, TexelCoord, 0, 0, Level);
	}

	template <typename T, qualifier P>
	inline void sampler2d<T, P>::texel_write(extent_type const & TexelCoord, size_type const & Level, texel_type const & Texel)
	{
		GLI_ASSERT(!this->Texture.empty());
		GLI_ASSERT(this->Convert.Write);

		this->Convert.Write(this->Texture, TexelCoord, 0, 0, Level, Texel);
	}

	template <typename T, qualifier P>
	inline void sampler2d<T, P>::clear(texel_type const & Color)
	{
		GLI_ASSERT(!this->Texture.empty());
		GLI_ASSERT(this->Convert.Write);

		detail::clear<texture_type, T, P>::call(this->Texture, this->Convert.Write, Color);
	}

	template <typename T, qualifier P>
	inline typename sampler2d<T, P>::texel_type sampler2d<T, P>::texture_lod(normalized_type const & SampleCoord, level_type Level) const
	{
		GLI_ASSERT(!this->Texture.empty());
		GLI_ASSERT(std::numeric_limits<T>::is_iec559);
		GLI_ASSERT(this->Filter && this->Convert.Fetch);

		normalized_type const SampleCoordWrap(this->Wrap(SampleCoord.x), this->Wrap(SampleCoord.y));
		return this->Filter(this->Texture, this->Convert.Fetch, SampleCoordWrap, size_type(0), size_type(0), Level, this->BorderColor);
	}

	template <typename T, qualifier P>
	inline typename sampler2d<T, P>::texel_type sampler2d<T, P>::texture_grad(normalized_type const & SampleCoord, normalized_type const& dPdx, normalized_type const& dPdy) const
	{
		GLI_ASSERT(!this->Texture.empty());
		GLI_ASSERT(std::numeric_limits<T>::is_iec559);
		GLI_ASSERT(this->Filter && this->Convert.Fetch);

		normalized_type const SampleCoordWrap(this->Wrap(SampleCoord.x), this->Wrap(SampleCoord.y));

		extent_type const TextureSize = this->Texture.extent(0);

		int const LevelCount = glm::log2<int>(max(TextureSize.x, TextureSize.y));
		T const d = max(dot(dPdx, dPdx), dot(dPdy, dPdy));
		T const Clamped = clamp(d, static_cast<T>(1), static_cast<T>(pow(2, (LevelCount - 1) * 2)));

		T const Level = static_cast<T>(0.5) * glm::log2<T>(Clamped);

		return this->Filter(this->Texture, this->Convert.Fetch, SampleCoordWrap, size_type(0), size_type(0), Level, this->BorderColor);
	}

	template <typename T, qualifier P>
	inline void sampler2d<T, P>::generate_mipmaps(filter Minification)
	{
		this->generate_mipmaps(this->Texture.base_level(), this->Texture.max_level(), Minification);
	}

	template <typename T, qualifier P>
	inline void sampler2d<T, P>::generate_mipmaps(size_type BaseLevel, size_type MaxLevel, filter Minification)
	{
		GLI_ASSERT(!this->Texture.empty());
		GLI_ASSERT(!is_compressed(this->Texture.format()));
		GLI_ASSERT(this->Texture.base_level() <= BaseLevel && BaseLevel <= MaxLevel && MaxLevel <= this->Texture.max_level());
		GLI_ASSERT(this->Convert.Fetch && this->Convert.Write);
		GLI_ASSERT(Minification >= FILTER_FIRST && Minification <= FILTER_LAST);

		detail::generate_mipmaps_2d<texture_type, T, fetch_type, write_type, normalized_type, texel_type>(
			this->Texture, this->Convert.Fetch, this->Convert.Write, 0, 0, 0, 0, BaseLevel, MaxLevel, Minification);
	}
}//namespace gli

