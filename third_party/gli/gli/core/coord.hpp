#pragma once

#include "../type.hpp"

namespace gli{
namespace detail
{
	template <length_t L, typename T, qualifier Q>
	inline vec<L, bool, Q> in_interval(vec<L, T, Q> const& Value, vec<L, T, Q> const& Min, vec<L, T, Q> const& Max)
	{
		return greaterThanEqual(Value, Min) && lessThanEqual(Value, Max);
	}

	template <typename extent_type, typename normalized_type>
	struct coord_nearest
	{
		extent_type Texel;
		typename extent_type::bool_type UseTexel;
	};

	template <typename extent_type, typename normalized_type>
	inline coord_nearest<extent_type, normalized_type> make_coord_nearest(extent_type const& TexelExtent, normalized_type const& SampleCoord)
	{
		normalized_type const TexelLast(normalized_type(TexelExtent) - normalized_type(1));

		coord_nearest<extent_type, normalized_type> Coord;
		Coord.Texel = extent_type(round(SampleCoord * TexelLast));
		Coord.UseTexel = in_interval(Coord.Texel, extent_type(0), TexelExtent - 1);
		return Coord;
	}

	template <typename extent_type, typename normalized_type>
	struct coord_linear
	{
		extent_type TexelFloor;
		extent_type TexelCeil;
		normalized_type Blend;
	};

	template <typename extent_type, typename normalized_type>
	struct coord_linear_border : public coord_linear<extent_type, normalized_type>
	{
		typename extent_type::bool_type UseTexelFloor;
		typename extent_type::bool_type UseTexelCeil;
	};

	template <typename extent_type, typename normalized_type>
	GLI_FORCE_INLINE coord_linear<extent_type, normalized_type> make_coord_linear(extent_type const& TexelExtent, normalized_type const& SampleCoord)
	{
		coord_linear<extent_type, normalized_type> Coord;

		normalized_type const TexelExtentF(TexelExtent);
		normalized_type const TexelLast = TexelExtentF - normalized_type(1);
		normalized_type const ScaledCoord(SampleCoord * TexelLast);
		normalized_type const ScaledCoordFloor = normalized_type(extent_type(ScaledCoord));
		normalized_type const ScaledCoordCeil = normalized_type(extent_type(ScaledCoord + normalized_type(0.5)));
		//normalized_type const ScaledCoordFloor(floor(ScaledCoord));
		//normalized_type const ScaledCoordCeil(ceil(ScaledCoord));

		Coord.Blend = ScaledCoord - ScaledCoordFloor;
		Coord.TexelFloor = extent_type(ScaledCoordFloor);
		Coord.TexelCeil = extent_type(ScaledCoordCeil);

		return Coord;
	}

	template <typename extent_type, typename normalized_type>
	GLI_FORCE_INLINE coord_linear_border<extent_type, normalized_type> make_coord_linear_border(extent_type const& TexelExtent, normalized_type const& SampleCoord)
	{
		coord_linear_border<extent_type, normalized_type> Coord;

		normalized_type const TexelExtentF(TexelExtent);
		normalized_type const TexelLast = TexelExtentF - normalized_type(1);
		normalized_type const ScaledCoord(SampleCoord * TexelLast);
		normalized_type const ScaledCoordFloor(floor(ScaledCoord));
		normalized_type const ScaledCoordCeil(ceil(ScaledCoord));

		Coord.Blend = ScaledCoord - ScaledCoordFloor;
		Coord.TexelFloor = extent_type(ScaledCoordFloor);
		Coord.TexelCeil = extent_type(ScaledCoordCeil);
		Coord.UseTexelFloor = in_interval(Coord.TexelFloor, extent_type(0), TexelExtent - 1);
		Coord.UseTexelCeil = in_interval(Coord.TexelCeil, extent_type(0), TexelExtent - 1);

		return Coord;
	}
}//namespace detail
}//namespace gli
