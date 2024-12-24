#pragma once

#include "filter_compute.hpp"

namespace gli{
namespace detail
{
	template <typename texture_type, typename sampler_value_type, typename fetch_func, typename write_func, typename normalized_type, typename texel_type>
	inline void generate_mipmaps_1d
	(
		texture_type & Texture, fetch_func Fetch, write_func Write,
		typename texture_type::size_type BaseLayer, typename texture_type::size_type MaxLayer,
		typename texture_type::size_type BaseFace, typename texture_type::size_type MaxFace,
		typename texture_type::size_type BaseLevel, typename texture_type::size_type MaxLevel,
		filter Min
	)
	{
		typedef typename detail::interpolate<sampler_value_type>::type interpolate_type;
		typedef typename texture_type::extent_type extent_type;
		typedef typename texture_type::size_type size_type;
		typedef typename extent_type::value_type component_type;
		typedef typename detail::filterBase<detail::DIMENSION_1D, texture_type, interpolate_type, normalized_type, fetch_func, texel_type>::filterFunc filter_func;

		filter_func const Filter = detail::get_filter<filter_func, detail::DIMENSION_1D, texture_type, interpolate_type, normalized_type, fetch_func, texel_type, sampler_value_type>(FILTER_NEAREST, Min, false);
		GLI_ASSERT(Filter);

		for(size_type Layer = BaseLayer; Layer <= MaxLayer; ++Layer)
		for(size_type Face = BaseFace; Face <= MaxFace; ++Face)
		for(size_type Level = BaseLevel; Level < MaxLevel; ++Level)
		{
			extent_type const& ExtentDst = Texture.extent(Level + 1);
			normalized_type const& Scale = normalized_type(1) / normalized_type(max(ExtentDst - extent_type(1), extent_type(1)));

			for(component_type i = 0; i < ExtentDst.x; ++i)
			{
				normalized_type const& SamplePosition(normalized_type(static_cast<typename normalized_type::value_type>(i)) * Scale);
				texel_type const& Texel = Filter(Texture, Fetch, SamplePosition, Layer, Face, static_cast<sampler_value_type>(Level), texel_type(0));
				Write(Texture, extent_type(i), Layer, Face, Level + 1, Texel);
			}
		}
	}

	template <typename texture_type, typename sampler_value_type, typename fetch_func, typename write_func, typename normalized_type, typename texel_type>
	inline void generate_mipmaps_2d
	(
		texture_type & Texture, fetch_func Fetch, write_func Write,
		typename texture_type::size_type BaseLayer, typename texture_type::size_type MaxLayer,
		typename texture_type::size_type BaseFace, typename texture_type::size_type MaxFace,
		typename texture_type::size_type BaseLevel, typename texture_type::size_type MaxLevel,
		filter Min
	)
	{
		typedef typename detail::interpolate<sampler_value_type>::type interpolate_type;
		typedef typename texture_type::extent_type extent_type;
		typedef typename texture_type::size_type size_type;
		typedef typename extent_type::value_type component_type;
		typedef typename detail::filterBase<detail::DIMENSION_2D, texture_type, interpolate_type, normalized_type, fetch_func, texel_type>::filterFunc filter_func;

		filter_func const Filter = detail::get_filter<filter_func, detail::DIMENSION_2D, texture_type, interpolate_type, normalized_type, fetch_func, texel_type, sampler_value_type>(FILTER_NEAREST, Min, false);
		GLI_ASSERT(Filter);

		for(size_type Layer = BaseLayer; Layer <= MaxLayer; ++Layer)
		for(size_type Face = BaseFace; Face <= MaxFace; ++Face)
		for(size_type Level = BaseLevel; Level < MaxLevel; ++Level)
		{
			extent_type const& ExtentDst = Texture.extent(Level + 1);
			normalized_type const& Scale = normalized_type(1) / normalized_type(max(ExtentDst - extent_type(1), extent_type(1)));

			for(component_type j = 0; j < ExtentDst.y; ++j)
			for(component_type i = 0; i < ExtentDst.x; ++i)
			{
				normalized_type const& SamplePosition(normalized_type(i, j) * Scale);
				texel_type const& Texel = Filter(Texture, Fetch, SamplePosition, Layer, Face, static_cast<sampler_value_type>(Level), texel_type(0));
				Write(Texture, extent_type(i, j), Layer, Face, Level + 1, Texel);
			}
		}
	}

	template <typename texture_type, typename sampler_value_type, typename fetch_func, typename write_func, typename normalized_type, typename texel_type>
	inline void generate_mipmaps_3d
	(
		texture_type & Texture, fetch_func Fetch, write_func Write,
		typename texture_type::size_type BaseLayer, typename texture_type::size_type MaxLayer,
		typename texture_type::size_type BaseFace, typename texture_type::size_type MaxFace,
		typename texture_type::size_type BaseLevel, typename texture_type::size_type MaxLevel,
		filter Min
	)
	{
		typedef typename detail::interpolate<sampler_value_type>::type interpolate_type;
		typedef typename texture_type::extent_type extent_type;
		typedef typename texture_type::size_type size_type;
		typedef typename extent_type::value_type component_type;
		typedef typename detail::filterBase<detail::DIMENSION_3D, texture_type, interpolate_type, normalized_type, fetch_func, texel_type>::filterFunc filter_func;

		filter_func const Filter = detail::get_filter<filter_func, detail::DIMENSION_3D, texture_type, interpolate_type, normalized_type, fetch_func, texel_type, sampler_value_type>(FILTER_NEAREST, Min, false);
		GLI_ASSERT(Filter);

		for(size_type Layer = BaseLayer; Layer <= MaxLayer; ++Layer)
		for(size_type Face = BaseFace; Face <= MaxFace; ++Face)
		for(size_type Level = BaseLevel; Level < MaxLevel; ++Level)
		{
			extent_type const& ExtentDst = Texture.extent(Level + 1);
			normalized_type const& Scale = normalized_type(1) / normalized_type(max(ExtentDst - extent_type(1), extent_type(1)));

			for(component_type k = 0; k < ExtentDst.z; ++k)
			for(component_type j = 0; j < ExtentDst.y; ++j)
			for(component_type i = 0; i < ExtentDst.x; ++i)
			{
				normalized_type const& SamplePosition(normalized_type(i, j, k) * Scale);
				texel_type const& Texel = Filter(Texture, Fetch, SamplePosition, Layer, Face, static_cast<sampler_value_type>(Level), texel_type(0));
				Write(Texture, extent_type(i, j, k), Layer, Face, Level + 1, Texel);
			}
		}
	}
}//namespace detail
}//namespace gli
