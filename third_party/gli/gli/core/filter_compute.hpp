#pragma once

#include "filter.hpp"
#include "coord.hpp"
#include <glm/gtc/integer.hpp>

namespace gli{
namespace detail
{
	enum dimension
	{
		DIMENSION_1D,
		DIMENSION_2D,
		DIMENSION_3D
	};

	template <typename T>
	struct interpolate
	{
		typedef float type;
	};

	template <>
	struct interpolate<double>
	{
		typedef double type;
	};

	template <>
	struct interpolate<long double>
	{
		typedef long double type;
	};

	template <dimension Dimension, typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type>
	struct filterBase
	{
		typedef typename texture_type::size_type size_type;
		typedef typename texture_type::extent_type extent_type;

		typedef texel_type(*filterFunc)(
			texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap,
			size_type Layer, size_type Face, interpolate_type Level,
			texel_type const & BorderColor);
	};

	template <dimension Dimension, typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type, bool is_float = true, bool support_border = true>
	struct nearest : public filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_nearest<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap, size_type Layer, size_type Face, size_type Level, texel_type const & BorderColor)
		{
			extent_type const TexelDim(Texture.extent(Level));
			normalized_type const TexelLast(normalized_type(TexelDim) - normalized_type(1));

			//extent_type const TexelCoord(SampleCoordWrap * TexelLast + interpolate_type(0.5));
			extent_type const TexelCoord = extent_type(round(SampleCoordWrap * TexelLast));
			typename extent_type::bool_type const UseTexelCoord = in_interval(TexelCoord, extent_type(0), TexelDim - 1);

			texel_type Texel(BorderColor);
			if(all(UseTexelCoord))
				Texel = Fetch(Texture, TexelCoord, Layer, Face, Level);

			return Texel;
		}
	};

	template <dimension Dimension, typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type>
	struct nearest<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, true, false> : public filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_nearest<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap, size_type Layer, size_type Face, size_type Level, texel_type const & BorderColor)
		{
			normalized_type const TexelLast(normalized_type(Texture.extent(Level)) - normalized_type(1));
			extent_type const TexelCoord(SampleCoordWrap * TexelLast + interpolate_type(0.5));
			//extent_type const TexelCoord = extent_type(round(SampleCoordWrap * TexelLast));

			return Fetch(Texture, TexelCoord, Layer, Face, Level);
		}
	};

	template <dimension Dimension, typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type, bool is_float = true, bool support_border = true>
	struct linear : public filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;

		static texel_type call(texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap, size_type Layer, size_type Face, size_type Level, texel_type const& BorderColor)
		{
			return texel_type(0);
		}
	};

	template <typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type>
	struct linear<DIMENSION_1D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, true, true> : public filterBase<DIMENSION_1D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<DIMENSION_1D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_linear_border<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap, size_type Layer, size_type Face, size_type Level, texel_type const & BorderColor)
		{
			coord_type const& Coord = make_coord_linear_border(Texture.extent(Level), SampleCoordWrap);

			texel_type Texel0(BorderColor);
			if(Coord.UseTexelFloor.s)
				Texel0 = Fetch(Texture, extent_type(Coord.TexelFloor.s), Layer, Face, Level);

			texel_type Texel1(BorderColor);
			if(Coord.UseTexelCeil.s)
				Texel1 = Fetch(Texture, extent_type(Coord.TexelCeil.s), Layer, Face, Level);

			return mix(Texel0, Texel1, Coord.Blend.s);
		}
	};

	template <typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type>
	struct linear<DIMENSION_1D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, true, false> : public filterBase<DIMENSION_1D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<DIMENSION_1D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_linear<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap, size_type Layer, size_type Face, size_type Level, texel_type const & BorderColor)
		{
			coord_type const& Coord = make_coord_linear(Texture.extent(Level), SampleCoordWrap);

			texel_type const Texel0 = Fetch(Texture, extent_type(Coord.TexelFloor.s), Layer, Face, Level);
			texel_type const Texel1 = Fetch(Texture, extent_type(Coord.TexelCeil.s), Layer, Face, Level);

			return mix(Texel0, Texel1, Coord.Blend.s);
		}
	};

	template <typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type>
	struct linear<DIMENSION_2D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, true, true> : public filterBase<DIMENSION_2D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<DIMENSION_2D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_linear_border<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const& Texture, fetch_type Fetch, normalized_type const& SampleCoordWrap, size_type Layer, size_type Face, size_type Level, texel_type const& BorderColor)
		{
			coord_type const& Coord = make_coord_linear_border(Texture.extent(Level), SampleCoordWrap);

			texel_type Texel00(BorderColor);
			if(Coord.UseTexelFloor.s && Coord.UseTexelFloor.t)
				Texel00 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelFloor.t), Layer, Face, Level);

			texel_type Texel10(BorderColor);
			if(Coord.UseTexelCeil.s && Coord.UseTexelFloor.t)
				Texel10 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelFloor.t), Layer, Face, Level);

			texel_type Texel11(BorderColor);
			if(Coord.UseTexelCeil.s && Coord.UseTexelCeil.t)
				Texel11 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelCeil.t), Layer, Face, Level);

			texel_type Texel01(BorderColor);
			if(Coord.UseTexelFloor.s && Coord.UseTexelCeil.t)
				Texel01 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelCeil.t), Layer, Face, Level);

			texel_type const ValueA(mix(Texel00, Texel10, Coord.Blend.s));
			texel_type const ValueB(mix(Texel01, Texel11, Coord.Blend.s));
			return mix(ValueA, ValueB, Coord.Blend.t);
		}
	};

	template <typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type>
	struct linear<DIMENSION_2D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, true, false> : public filterBase<DIMENSION_2D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<DIMENSION_2D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_linear<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const& Texture, fetch_type Fetch, normalized_type const& SampleCoordWrap, size_type Layer, size_type Face, size_type Level, texel_type const& BorderColor)
		{
			coord_type const& Coord = make_coord_linear(Texture.extent(Level), SampleCoordWrap);

			texel_type const& Texel00 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelFloor.t), Layer, Face, Level);
			texel_type const& Texel10 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelFloor.t), Layer, Face, Level);
			texel_type const& Texel11 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelCeil.t), Layer, Face, Level);
			texel_type const& Texel01 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelCeil.t), Layer, Face, Level);

			texel_type const ValueA(mix(Texel00, Texel10, Coord.Blend.s));
			texel_type const ValueB(mix(Texel01, Texel11, Coord.Blend.s));
			return mix(ValueA, ValueB, Coord.Blend.t);
		}
	};

	template <typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type>
	struct linear<DIMENSION_3D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, true, true> : public filterBase<DIMENSION_3D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<DIMENSION_3D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_linear_border<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const& Texture, fetch_type Fetch, normalized_type const& SampleCoordWrap, size_type Layer, size_type Face, size_type Level, texel_type const& BorderColor)
		{
			coord_type const& Coord = make_coord_linear_border(Texture.extent(Level), SampleCoordWrap);

			texel_type Texel000(BorderColor);
			if(Coord.UseTexelFloor.s && Coord.UseTexelFloor.t && Coord.UseTexelFloor.p)
				Texel000 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelFloor.t, Coord.TexelFloor.p), Layer, Face, Level);

			texel_type Texel100(BorderColor);
			if(Coord.UseTexelCeil.s && Coord.UseTexelFloor.t && Coord.UseTexelFloor.p)
				Texel100 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelFloor.t, Coord.TexelFloor.p), Layer, Face, Level);

			texel_type Texel110(BorderColor);
			if(Coord.UseTexelCeil.s && Coord.UseTexelCeil.t && Coord.UseTexelFloor.p)
				Texel110 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelCeil.t, Coord.TexelFloor.p), Layer, Face, Level);

			texel_type Texel010(BorderColor);
			if(Coord.UseTexelFloor.s && Coord.UseTexelCeil.t && Coord.UseTexelFloor.p)
				Texel010 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelCeil.t, Coord.TexelFloor.p), Layer, Face, Level);

			texel_type Texel001(BorderColor);
			if (Coord.UseTexelFloor.s && Coord.UseTexelFloor.t && Coord.UseTexelCeil.p)
				Texel001 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelFloor.t, Coord.TexelCeil.p), Layer, Face, Level);

			texel_type Texel101(BorderColor);
			if(Coord.UseTexelCeil.s && Coord.UseTexelFloor.t && Coord.UseTexelCeil.p)
				Texel101 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelFloor.t, Coord.TexelCeil.p), Layer, Face, Level);

			texel_type Texel111(BorderColor);
			if(Coord.UseTexelCeil.s && Coord.UseTexelCeil.t && Coord.UseTexelCeil.p)
				Texel111 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelCeil.t, Coord.TexelCeil.p), Layer, Face, Level);

			texel_type Texel011(BorderColor);
			if(Coord.UseTexelFloor.s && Coord.UseTexelCeil.t && Coord.UseTexelCeil.p)
				Texel011 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelCeil.t, Coord.TexelCeil.p), Layer, Face, Level);

			texel_type const ValueA(mix(Texel000, Texel100, Coord.Blend.s));
			texel_type const ValueB(mix(Texel010, Texel110, Coord.Blend.s));

			texel_type const ValueC(mix(Texel001, Texel101, Coord.Blend.s));
			texel_type const ValueD(mix(Texel011, Texel111, Coord.Blend.s));

			texel_type const ValueE(mix(ValueA, ValueB, Coord.Blend.t));
			texel_type const ValueF(mix(ValueC, ValueD, Coord.Blend.t));

			return mix(ValueE, ValueF, Coord.Blend.p);
		}
	};

	template <typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type>
	struct linear<DIMENSION_3D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, true, false> : public filterBase<DIMENSION_3D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<DIMENSION_3D, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_linear<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap, size_type Layer, size_type Face, size_type Level, texel_type const & BorderColor)
		{
			coord_type const & Coord = make_coord_linear(Texture.extent(Level), SampleCoordWrap);

			texel_type const & Texel000 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelFloor.t, Coord.TexelFloor.p), Layer, Face, Level);
			texel_type const & Texel100 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelFloor.t, Coord.TexelFloor.p), Layer, Face, Level);
			texel_type const & Texel110 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelCeil.t, Coord.TexelFloor.p), Layer, Face, Level);
			texel_type const & Texel010 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelCeil.t, Coord.TexelFloor.p), Layer, Face, Level);
			texel_type const & Texel001 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelFloor.t, Coord.TexelCeil.p), Layer, Face, Level);
			texel_type const & Texel101 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelFloor.t, Coord.TexelCeil.p), Layer, Face, Level);
			texel_type const & Texel111 = Fetch(Texture, extent_type(Coord.TexelCeil.s, Coord.TexelCeil.t, Coord.TexelCeil.p), Layer, Face, Level);
			texel_type const & Texel011 = Fetch(Texture, extent_type(Coord.TexelFloor.s, Coord.TexelCeil.t, Coord.TexelCeil.p), Layer, Face, Level);

			texel_type const ValueA(mix(Texel000, Texel100, Coord.Blend.s));
			texel_type const ValueB(mix(Texel010, Texel110, Coord.Blend.s));

			texel_type const ValueC(mix(Texel001, Texel101, Coord.Blend.s));
			texel_type const ValueD(mix(Texel011, Texel111, Coord.Blend.s));

			texel_type const ValueE(mix(ValueA, ValueB, Coord.Blend.t));
			texel_type const ValueF(mix(ValueC, ValueD, Coord.Blend.t));

			return mix(ValueE, ValueF, Coord.Blend.p);
		}
	};

	template <dimension Dimension, typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type, bool is_float, bool support_border>
	struct nearest_mipmap_nearest : public filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_linear_border<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap, size_type Layer, size_type Face, interpolate_type Level, texel_type const & BorderColor)
		{
			return nearest<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, is_float, support_border>::call(Texture, Fetch, SampleCoordWrap, Layer, Face, glm::iround(Level), BorderColor);
		}
	};

	template <dimension Dimension, typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type, bool is_float, bool support_border>
	struct nearest_mipmap_linear : public filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_linear_border<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap, size_type Layer, size_type Face, interpolate_type Level, texel_type const & BorderColor)
		{
			texel_type const MinTexel = nearest<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, is_float, support_border>::call(Texture, Fetch, SampleCoordWrap, Layer, Face, static_cast<size_type>(floor(Level)), BorderColor);
			texel_type const MaxTexel = nearest<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, is_float, support_border>::call(Texture, Fetch, SampleCoordWrap, Layer, Face, static_cast<size_type>(ceil(Level)), BorderColor);
			return mix(MinTexel, MaxTexel, fract(Level));
		}
	};

	template <dimension Dimension, typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type, bool is_float, bool support_border>
	struct linear_mipmap_nearest : public filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_linear_border<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap, size_type Layer, size_type Face, interpolate_type Level, texel_type const & BorderColor)
		{
			return linear<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, is_float, support_border>::call(Texture, Fetch, SampleCoordWrap, Layer, Face, glm::iround(Level), BorderColor);
		}
	};

	template <dimension Dimension, typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type, bool is_float, bool support_border>
	struct linear_mipmap_linear : public filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type>
	{
		typedef filterBase<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type> base_type;
		typedef typename base_type::size_type size_type;
		typedef typename base_type::extent_type extent_type;
		typedef coord_linear_border<extent_type, normalized_type> coord_type;

		static texel_type call(texture_type const & Texture, fetch_type Fetch, normalized_type const & SampleCoordWrap, size_type Layer, size_type Face, interpolate_type Level, texel_type const & BorderColor)
		{
			size_type const FloorLevel = static_cast<size_type>(floor(Level));
			size_type const CeilLevel = static_cast<size_type>(ceil(Level));
			texel_type const MinTexel = linear<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, is_float, support_border>::call(Texture, Fetch, SampleCoordWrap, Layer, Face, FloorLevel, BorderColor);
			texel_type const MaxTexel = linear<Dimension, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, is_float, support_border>::call(Texture, Fetch, SampleCoordWrap, Layer, Face, CeilLevel, BorderColor);
			return mix(MinTexel, MaxTexel, fract(Level));
		}
	};

	template <typename filter_type, dimension Dimensions, typename texture_type, typename interpolate_type, typename normalized_type, typename fetch_type, typename texel_type, typename T>
	inline filter_type get_filter(filter Mip, filter Min, bool Border)
	{
		static filter_type Table[][FILTER_COUNT][2] =
		{
			{
				{
					nearest_mipmap_nearest<Dimensions, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, std::numeric_limits<T>::is_iec559, false>::call,
					nearest_mipmap_nearest<Dimensions, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, std::numeric_limits<T>::is_iec559, true>::call
				},
				{
					linear_mipmap_nearest<Dimensions, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, std::numeric_limits<T>::is_iec559, false>::call,
					linear_mipmap_nearest<Dimensions, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, std::numeric_limits<T>::is_iec559, true>::call
				}
			},
			{
				{
					nearest_mipmap_linear<Dimensions, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, std::numeric_limits<T>::is_iec559, false>::call,
					nearest_mipmap_linear<Dimensions, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, std::numeric_limits<T>::is_iec559, true>::call
				},
				{
					linear_mipmap_linear<Dimensions, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, std::numeric_limits<T>::is_iec559, false>::call,
					linear_mipmap_linear<Dimensions, texture_type, interpolate_type, normalized_type, fetch_type, texel_type, std::numeric_limits<T>::is_iec559, true>::call
				}
			}
		};
		static_assert(sizeof(Table) / sizeof(Table[0]) == FILTER_COUNT, "GLI ERROR: 'Table' doesn't match the number of supported filters");

		GLI_ASSERT(Table[Mip - FILTER_FIRST][Min - FILTER_FIRST][Border ? 1 : 0]);

		return Table[Mip - FILTER_FIRST][Min - FILTER_FIRST][Border ? 1 : 0];
	}
}//namespace detail
}//namespace gli

