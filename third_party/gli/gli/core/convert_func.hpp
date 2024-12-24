#pragma once

#include "../type.hpp"
#include "../texture1d.hpp"
#include "../texture1d_array.hpp"
#include "../texture2d.hpp"
#include "../texture2d_array.hpp"
#include "../texture3d.hpp"
#include "../texture_cube.hpp"
#include "../texture_cube_array.hpp"
#include "./s3tc.hpp"
#include "./bc.hpp"
#include <glm/gtc/packing.hpp>
#include <glm/gtc/color_space.hpp>
#include <limits>

namespace gli{
namespace detail
{
	enum convertMode
	{
		CONVERT_MODE_DEFAULT,
		CONVERT_MODE_CAST,
		CONVERT_MODE_NORM,
		CONVERT_MODE_SRGB,
		CONVERT_MODE_HALF,
		CONVERT_MODE_RGB9E5,
		CONVERT_MODE_RG11B10F,
		CONVERT_MODE_RGB10A2UNORM,
		CONVERT_MODE_RGB10A2SNORM,
		CONVERT_MODE_RGB10A2USCALE,
		CONVERT_MODE_RGB10A2SSCALE,
		CONVERT_MODE_RGB10A2UINT,
		CONVERT_MODE_RGB10A2SINT,
		CONVERT_MODE_44UNORM,
		CONVERT_MODE_44SCALED,
		CONVERT_MODE_4444UNORM,
		CONVERT_MODE_4444SCALED,
		CONVERT_MODE_565UNORM,
		CONVERT_MODE_565SCALED,
		CONVERT_MODE_5551UNORM,
		CONVERT_MODE_5551SCALED,
		CONVERT_MODE_332UNORM,
		CONVERT_MODE_DXT1UNORM,
		CONVERT_MODE_DXT3UNORM,
		CONVERT_MODE_DXT5UNORM,
		CONVERT_MODE_BC1UNORM = CONVERT_MODE_DXT1UNORM,
		CONVERT_MODE_BC2UNORM = CONVERT_MODE_DXT3UNORM,
		CONVERT_MODE_BC3UNORM = CONVERT_MODE_DXT5UNORM,
		CONVERT_MODE_BC4UNORM,
		CONVERT_MODE_BC4SNORM,
		CONVERT_MODE_BC5UNORM,
		CONVERT_MODE_BC5SNORM
	};

	template <typename textureType, typename genType>
	struct accessFunc
	{};

	template <typename genType>
	struct accessFunc<texture1d, genType>
	{
		static genType load(texture1d const & Texture, texture1d::extent_type const & TexelCoord, texture1d::size_type Layer, texture1d::size_type Face, texture1d::size_type Level)
		{
			GLI_ASSERT(Layer == 0 && Face == 0);
			return Texture.load<genType>(TexelCoord, Level);
		}

		static void store(texture1d & Texture, texture1d::extent_type const & TexelCoord, texture1d::size_type Layer, texture1d::size_type Face, texture1d::size_type Level, genType const & Texel)
		{
			GLI_ASSERT(Layer == 0 && Face == 0);
			Texture.store<genType>(TexelCoord, Level, Texel);
		}
	};

	template <typename genType>
	struct accessFunc<texture1d_array, genType>
	{
		static genType load(texture1d_array const& Texture, texture1d_array::extent_type const& TexelCoord, texture1d_array::size_type Layer, texture1d_array::size_type Face, texture1d_array::size_type Level)
		{
			GLI_ASSERT(Face == 0);
			return Texture.load<genType>(TexelCoord, Layer, Level);
		}

		static void store(texture1d_array& Texture, texture1d_array::extent_type const& TexelCoord, texture1d_array::size_type Layer, texture1d_array::size_type Face, texture1d_array::size_type Level, genType const& Texel)
		{
			GLI_ASSERT(Face == 0);
			Texture.store<genType>(TexelCoord, Layer, Level, Texel);
		}
	};

	template <typename genType>
	struct accessFunc<texture2d, genType>
	{
		static genType load(texture2d const & Texture, texture2d::extent_type const & TexelCoord, texture2d::size_type Layer, texture2d::size_type Face, texture2d::size_type Level)
		{
			GLI_ASSERT(Layer == 0 && Face == 0);
			return Texture.load<genType>(TexelCoord, Level);
		}

		static void store(texture2d & Texture, texture2d::extent_type const & TexelCoord, texture2d::size_type Layer, texture2d::size_type Face, texture2d::size_type Level, genType const & Texel)
		{
			GLI_ASSERT(Layer == 0 && Face == 0);
			Texture.store<genType>(TexelCoord, Level, Texel);
		}
	};

	template <typename genType>
	struct accessFunc<texture2d_array, genType>
	{
		static genType load(texture2d_array const & Texture, texture2d_array::extent_type const & TexelCoord, texture2d_array::size_type Layer, texture2d_array::size_type Face, texture2d_array::size_type Level)
		{
			GLI_ASSERT(Face == 0);
			return Texture.load<genType>(TexelCoord, Layer, Level);
		}

		static void store(texture2d_array & Texture, texture2d_array::extent_type const & TexelCoord, texture2d_array::size_type Layer, texture2d_array::size_type Face, texture2d_array::size_type Level, genType const & Texel)
		{
			GLI_ASSERT(Face == 0);
			Texture.store<genType>(TexelCoord, Layer, Level, Texel);
		}
	};

	template <typename genType>
	struct accessFunc<texture3d, genType>
	{
		static genType load(texture3d const & Texture, texture3d::extent_type const & TexelCoord, texture3d::size_type Layer, texture3d::size_type Face, texture3d::size_type Level)
		{
			GLI_ASSERT(Layer == 0 && Face == 0);
			return Texture.load<genType>(TexelCoord, Level);
		}

		static void store(texture3d & Texture, texture3d::extent_type const & TexelCoord, texture3d::size_type Layer, texture3d::size_type Face, texture3d::size_type Level, genType const & Texel)
		{
			GLI_ASSERT(Layer == 0 && Face == 0);
			Texture.store<genType>(TexelCoord, Level, Texel);
		}
	};

	template <typename genType>
	struct accessFunc<texture_cube, genType>
	{
		static genType load(texture_cube const& Texture, texture_cube::extent_type const& TexelCoord, texture_cube::size_type Layer, texture_cube::size_type Face, texture_cube::size_type Level)
		{
			GLI_ASSERT(Layer == 0);
			return Texture.load<genType>(TexelCoord, Face, Level);
		}

		static void store(texture_cube& Texture, texture_cube::extent_type const& TexelCoord, texture_cube::size_type Layer, texture_cube::size_type Face, texture_cube::size_type Level, genType const& Texel)
		{
			GLI_ASSERT(Layer == 0);
			Texture.store<genType>(TexelCoord, Face, Level, Texel);
		}
	};

	template <typename genType>
	struct accessFunc<texture_cube_array, genType>
	{
		static genType load(texture_cube_array const & Texture, texture_cube_array::extent_type const & TexelCoord, texture_cube_array::size_type Layer, texture_cube_array::size_type Face, texture_cube_array::size_type Level)
		{
			return Texture.load<genType>(TexelCoord, Layer, Face, Level);
		}

		static void store(texture_cube_array & Texture, texture_cube_array::extent_type const & TexelCoord, texture_cube_array::size_type Layer, texture_cube_array::size_type Face, texture_cube_array::size_type Level, genType const & Texel)
		{
			Texture.store<genType>(TexelCoord, Layer, Face, Level, Texel);
		}
	};

	// convertFunc class

	template <typename textureType, typename retType, length_t L, typename T, qualifier P, convertMode mode = CONVERT_MODE_CAST, bool isSamplerFloat = false>
	struct convertFunc
	{
		typedef accessFunc<textureType, vec<L, T, P> > access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			return make_vec4<retType, P>(vec<L, retType, P>(access::load(Texture, TexelCoord, Layer, Face, Level)));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			access::store(Texture, TexelCoord, Layer, Face, Level, vec<L, T, P>(Texel));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P, bool isSamplerFloat>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_DEFAULT, isSamplerFloat>
	{
		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			return vec<4, retType, P>(0, 0, 0, 1);
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_NORM, true>
	{
		typedef accessFunc<textureType, vec<L, T, P> > access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_NORM requires a float sampler");
			return make_vec4<retType, P>(compNormalize<retType>(access::load(Texture, TexelCoord, Layer, Face, Level)));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_NORM requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, compScale<T>(vec<L, retType, P>(Texel)));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_SRGB, true>
	{
		typedef accessFunc<textureType, vec<L, T, P> > access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_SRGB requires a float sampler");
			return make_vec4<retType, P>(convertSRGBToLinear(compNormalize<retType>(access::load(Texture, TexelCoord, Layer, Face, Level))));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_SRGB requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, gli::compScale<T>(convertLinearToSRGB(vec<L, retType, P>(Texel))));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_RGB9E5, true>
	{
		typedef accessFunc<textureType, uint32> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RGB9E5 requires a float sampler");
			return vec<4, retType, P>(unpackF3x9_E1x5(access::load(Texture, TexelCoord, Layer, Face, Level)), static_cast<retType>(1));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RGB9E5 requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packF3x9_E1x5(vec<3, float, P>(Texel)));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_RG11B10F, true>
	{
		typedef accessFunc<textureType, uint32> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RG11B10F requires a float sampler");
			return vec<4, retType, P>(unpackF2x11_1x10(access::load(Texture, TexelCoord, Layer, Face, Level)), static_cast<retType>(1));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RG11B10F requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packF2x11_1x10(vec<3, float, P>(Texel)));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_HALF, true>
	{
		typedef accessFunc<textureType, vec<L, uint16, P> > access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_HALF requires a float sampler");
			return make_vec4<retType, P>(vec<L, retType, P>(unpackHalf(access::load(Texture, TexelCoord, Layer, Face, Level))));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_HALF requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packHalf(vec<L, float, P>(Texel)));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_44UNORM, true>
	{
		typedef accessFunc<textureType, uint8> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_44UNORM requires a float sampler");
			return vec<4, retType, P>(vec<2, retType, P>(unpackUnorm2x4(access::load(Texture, TexelCoord, Layer, Face, Level))), static_cast<retType>(0), static_cast<retType>(1));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_44UNORM requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packUnorm2x4(vec<2, float, P>(Texel)));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_4444UNORM, true>
	{
		typedef accessFunc<textureType, uint16> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_4444UNORM requires a float sampler");
			return vec<4, retType, P>(unpackUnorm4x4(access::load(Texture, TexelCoord, Layer, Face, Level)));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_4444UNORM requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packUnorm4x4(vec<4, float, P>(Texel)));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_565UNORM, true>
	{
		typedef accessFunc<textureType, uint16> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_565UNORM requires a float sampler");
			return vec<4, retType, P>(unpackUnorm1x5_1x6_1x5(access::load(Texture, TexelCoord, Layer, Face, Level)), static_cast<retType>(1));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_565UNORM requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packUnorm1x5_1x6_1x5(vec<3, float, P>(Texel)));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_5551UNORM, true>
	{
		typedef accessFunc<textureType, uint16> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_5551UNORM requires a float sampler");
			return vec<4, retType, P>(unpackUnorm3x5_1x1(access::load(Texture, TexelCoord, Layer, Face, Level)));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_5551UNORM requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packUnorm3x5_1x1(vec<4, float, P>(Texel)));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_332UNORM, true>
	{
		typedef accessFunc<textureType, uint8> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_332UNORM requires a float sampler");
			return vec<4, retType, P>(unpackUnorm2x3_1x2(access::load(Texture, TexelCoord, Layer, Face, Level)), static_cast<retType>(1));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_332UNORM requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packUnorm2x3_1x2(vec<3, float, P>(Texel)));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_RGB10A2UNORM, true>
	{
		typedef accessFunc<textureType, uint32> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RGB10A2UNORM requires a float sampler");
			return vec<4, retType, P>(unpackUnorm3x10_1x2(access::load(Texture, TexelCoord, Layer, Face, Level)));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RGB10A2UNORM requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packUnorm3x10_1x2(vec<4, float, P>(Texel)));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_RGB10A2SNORM, true>
	{
		typedef accessFunc<textureType, uint32> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RGB10A2SNORM requires a float sampler");
			return vec<4, retType, P>(unpackSnorm3x10_1x2(access::load(Texture, TexelCoord, Layer, Face, Level)));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RGB10A2SNORM requires a float sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packSnorm3x10_1x2(Texel));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_RGB10A2USCALE, true>
	{
		typedef accessFunc<textureType, uint32> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RGB10A2USCALE requires a float sampler");
			glm::detail::u10u10u10u2 Unpack;
			Unpack.pack = access::load(Texture, TexelCoord, Layer, Face, Level);
			return vec<4, retType, P>(Unpack.data.x, Unpack.data.y, Unpack.data.z, Unpack.data.w);
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RGB10A2USCALE requires a float sampler");
			glm::detail::u10u10u10u2 Unpack;
			Unpack.data.x = static_cast<uint>(Texel.x);
			Unpack.data.y = static_cast<uint>(Texel.y);
			Unpack.data.z = static_cast<uint>(Texel.z);
			Unpack.data.w = static_cast<uint>(Texel.w);
			access::store(Texture, TexelCoord, Layer, Face, Level, Unpack.pack);
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_RGB10A2SSCALE, true>
	{
		typedef accessFunc<textureType, uint32> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RGB10A2SSCALE requires a float sampler");
			glm::detail::i10i10i10i2 Unpack;
			Unpack.pack = access::load(Texture, TexelCoord, Layer, Face, Level);
			return vec<4, retType, P>(Unpack.data.x, Unpack.data.y, Unpack.data.z, Unpack.data.w);
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_RGB10A2SSCALE requires a float sampler");
			glm::detail::i10i10i10i2 Unpack;
			Unpack.data.x = static_cast<int>(Texel.x);
			Unpack.data.y = static_cast<int>(Texel.y);
			Unpack.data.z = static_cast<int>(Texel.z);
			Unpack.data.w = static_cast<int>(Texel.w);
			access::store(Texture, TexelCoord, Layer, Face, Level, Unpack.pack);
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_RGB10A2UINT, false>
	{
		typedef accessFunc<textureType, uint32> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_integer, "CONVERT_MODE_RGB10A2UINT requires an integer sampler");
			return vec<4, retType, P>(unpackU3x10_1x2(access::load(Texture, TexelCoord, Layer, Face, Level)));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_integer, "CONVERT_MODE_RGB10A2UINT requires an integer sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packU3x10_1x2(Texel));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_RGB10A2SINT, false>
	{
		typedef accessFunc<textureType, uint32> access;

		static vec<4, retType, P> fetch(textureType const & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_integer, "CONVERT_MODE_RGB10A2SINT requires an integer sampler");
			return vec<4, retType, P>(unpackI3x10_1x2(access::load(Texture, TexelCoord, Layer, Face, Level)));
		}

		static void write(textureType & Texture, typename textureType::extent_type const & TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_integer, "CONVERT_MODE_RGB10A2SINT requires an integer sampler");
			access::store(Texture, TexelCoord, Layer, Face, Level, packI3x10_1x2(Texel));
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_DXT1UNORM, true>
	{
		typedef accessFunc<gli::texture2d, uint32> access;

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent1d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			return glm::vec<4, retType, P>(0, 0, 0, 1);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent2d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			gli::extent3d TexelCoord3d(TexelCoord, 0);
			return fetch(Texture, TexelCoord3d, Layer, Face, Level);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent3d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_DXT1UNORM requires an float sampler");
			
			if(Texture.target() == gli::TARGET_1D || Texture.target() == gli::TARGET_1D_ARRAY)
			{
				return glm::vec<4, retType, P>(0, 0, 0, 1);
			}
			
			const dxt1_block *Data = Texture.template data<dxt1_block>(Layer, Face, Level);
			const gli::extent3d &BlockExtent = block_extent(Texture.format());
			int WidthInBlocks = glm::max(1, Texture.extent(Level).x / BlockExtent.x);
			int BlocksInSlice = glm::max(1, Texture.extent(Level).y / BlockExtent.y) * WidthInBlocks;
			gli::extent3d BlockCoord(TexelCoord / BlockExtent);
			glm::ivec2 TexelCoordInBlock(TexelCoord.x - (BlockCoord.x * BlockExtent.x), TexelCoord.y - (BlockCoord.y * BlockExtent.y));
			
			const dxt1_block &Block = Data[BlockCoord.z * BlocksInSlice + (BlockCoord.y * WidthInBlocks + BlockCoord.x)];

			return decompress_dxt1(Block, TexelCoordInBlock);
		}

		static void write(textureType& Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_DXT1UNORM requires an float sampler");

			GLI_ASSERT("Writing to single texel of a DXT1 compressed image is not supported");
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_DXT3UNORM, true> {
		typedef accessFunc<gli::texture2d, uint32> access;

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent1d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			return glm::vec<4, retType, P>(0, 0, 0, 1);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent2d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			gli::extent3d TexelCoord3d(TexelCoord, 0);
			return fetch(Texture, TexelCoord3d, Layer, Face, Level);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent3d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_DXT3UNORM requires an float sampler");

			if(Texture.target() == gli::TARGET_1D || Texture.target() == gli::TARGET_1D_ARRAY)
			{
				return glm::vec<4, retType, P>(0, 0, 0, 1);
			}

			const dxt3_block *Data = Texture.template data<dxt3_block>(Layer, Face, Level);
			const gli::extent3d &BlockExtent = block_extent(Texture.format());
			int WidthInBlocks = glm::max(1, Texture.extent(Level).x / BlockExtent.x);
			int BlocksInSlice = glm::max(1, Texture.extent(Level).y / BlockExtent.y) * WidthInBlocks;
			gli::extent3d BlockCoord(TexelCoord / BlockExtent);
			glm::ivec2 TexelCoordInBlock(TexelCoord.x - (BlockCoord.x * BlockExtent.x), TexelCoord.y - (BlockCoord.y * BlockExtent.y));

			const dxt3_block &Block = Data[BlockCoord.z * BlocksInSlice + (BlockCoord.y * WidthInBlocks + BlockCoord.x)];

			return decompress_dxt3(Block, TexelCoordInBlock);
		}

		static void write(textureType& Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_DXT3UNORM requires an float sampler");

			GLI_ASSERT("Writing to single texel of a DXT3 compressed image is not supported");
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_DXT5UNORM, true> {
		typedef accessFunc<gli::texture2d, uint32> access;

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent1d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			return glm::vec<4, retType, P>(0, 0, 0, 1);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent2d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			gli::extent3d TexelCoord3d(TexelCoord, 0);
			return fetch(Texture, TexelCoord3d, Layer, Face, Level);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent3d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_DXT5UNORM requires an float sampler");

			if(Texture.target() == gli::TARGET_1D || Texture.target() == gli::TARGET_1D_ARRAY)
			{
				return glm::vec<4, retType, P>(0, 0, 0, 1);
			}

			const dxt5_block *Data = Texture.template data<dxt5_block>(Layer, Face, Level);
			const gli::extent3d &BlockExtent = block_extent(Texture.format());
			int WidthInBlocks = glm::max(1, Texture.extent(Level).x / BlockExtent.x);
			int BlocksInSlice = glm::max(1, Texture.extent(Level).y / BlockExtent.y) * WidthInBlocks;
			gli::extent3d BlockCoord(TexelCoord / BlockExtent);
			glm::ivec2 TexelCoordInBlock(TexelCoord.x - (BlockCoord.x * BlockExtent.x), TexelCoord.y - (BlockCoord.y * BlockExtent.y));

			const dxt5_block &Block = Data[BlockCoord.z * BlocksInSlice + (BlockCoord.y * WidthInBlocks + BlockCoord.x)];

			return decompress_dxt5(Block, TexelCoordInBlock);
		}

		static void write(textureType& Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_DXT5UNORM requires an float sampler");

			GLI_ASSERT("Writing to single texel of a DXT5 compressed image is not supported");
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_BC4UNORM, true> {
		typedef accessFunc<gli::texture2d, uint32> access;

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent1d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			return glm::vec<4, retType, P>(0, 0, 0, 1);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent2d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			gli::extent3d TexelCoord3d(TexelCoord, 0);
			return fetch(Texture, TexelCoord3d, Layer, Face, Level);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent3d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_BC4UNORM requires an float sampler");

			if(Texture.target() == gli::TARGET_1D || Texture.target() == gli::TARGET_1D_ARRAY)
			{
				return glm::vec<4, retType, P>(0, 0, 0, 1);
			}

			const bc4_block *Data = Texture.template data<bc4_block>(Layer, Face, Level);
			const gli::extent3d &BlockExtent = block_extent(Texture.format());
			int WidthInBlocks = glm::max(1, Texture.extent(Level).x / BlockExtent.x);
			int BlocksInSlice = glm::max(1, Texture.extent(Level).y / BlockExtent.y) * WidthInBlocks;
			gli::extent3d BlockCoord(TexelCoord / BlockExtent);
			glm::ivec2 TexelCoordInBlock(TexelCoord.x - (BlockCoord.x * BlockExtent.x), TexelCoord.y - (BlockCoord.y * BlockExtent.y));

			const bc4_block &Block = Data[BlockCoord.z * BlocksInSlice + (BlockCoord.y * WidthInBlocks + BlockCoord.x)];
			
			return decompress_bc4unorm(Block, TexelCoordInBlock);
		}

		static void write(textureType& Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_BC4UNORM requires an float sampler");

			GLI_ASSERT("Writing to single texel of a BC4 compressed image is not supported");
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_BC4SNORM, true> {
		typedef accessFunc<gli::texture2d, uint32> access;

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent1d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			return glm::vec<4, retType, P>(0, 0, 0, 1);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent2d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			gli::extent3d TexelCoord3d(TexelCoord, 0);
			return fetch(Texture, TexelCoord3d, Layer, Face, Level);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent3d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_BC4SNORM requires an float sampler");

			if(Texture.target() == gli::TARGET_1D || Texture.target() == gli::TARGET_1D_ARRAY)
			{
				return glm::vec<4, retType, P>(0, 0, 0, 1);
			}

			const bc4_block *Data = Texture.template data<bc4_block>(Layer, Face, Level);
			const gli::extent3d &BlockExtent = block_extent(Texture.format());
			int WidthInBlocks = glm::max(1, Texture.extent(Level).x / BlockExtent.x);
			int BlocksInSlice = glm::max(1, Texture.extent(Level).y / BlockExtent.y) * WidthInBlocks;
			gli::extent3d BlockCoord(TexelCoord / BlockExtent);
			glm::ivec2 TexelCoordInBlock(TexelCoord.x - (BlockCoord.x * BlockExtent.x), TexelCoord.y - (BlockCoord.y * BlockExtent.y));

			const bc4_block &Block = Data[BlockCoord.z * BlocksInSlice + (BlockCoord.y * WidthInBlocks + BlockCoord.x)];

			return decompress_bc4snorm(Block, TexelCoordInBlock);
		}

		static void write(textureType& Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_BC4SNORM requires an float sampler");

			GLI_ASSERT("Writing to single texel of a BC4 compressed image is not supported");
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_BC5UNORM, true> {
		typedef accessFunc<gli::texture2d, uint32> access;

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent1d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			return glm::vec<4, retType, P>(0, 0, 0, 1);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent2d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			gli::extent3d TexelCoord3d(TexelCoord, 0);
			return fetch(Texture, TexelCoord3d, Layer, Face, Level);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent3d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_BC5UNORM requires an float sampler");

			if(Texture.target() == gli::TARGET_1D || Texture.target() == gli::TARGET_1D_ARRAY)
			{
				return glm::vec<4, retType, P>(0, 0, 0, 1);
			}

			const bc5_block *Data = Texture.template data<bc5_block>(Layer, Face, Level);
			const gli::extent3d &BlockExtent = block_extent(Texture.format());
			int WidthInBlocks = glm::max(1, Texture.extent(Level).x / BlockExtent.x);
			int BlocksInSlice = glm::max(1, Texture.extent(Level).y / BlockExtent.y) * WidthInBlocks;
			gli::extent3d BlockCoord(TexelCoord / BlockExtent);
			glm::ivec2 TexelCoordInBlock(TexelCoord.x - (BlockCoord.x * BlockExtent.x), TexelCoord.y - (BlockCoord.y * BlockExtent.y));

			const bc5_block &Block = Data[BlockCoord.z * BlocksInSlice + (BlockCoord.y * WidthInBlocks + BlockCoord.x)];

			return decompress_bc5unorm(Block, TexelCoordInBlock);
		}

		static void write(textureType& Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_BC5UNORM requires an float sampler");

			GLI_ASSERT("Writing to single texel of a BC5 compressed image is not supported");
		}
	};

	template <typename textureType, typename retType, length_t L, typename T, qualifier P>
	struct convertFunc<textureType, retType, L, T, P, CONVERT_MODE_BC5SNORM, true> {
		typedef accessFunc<gli::texture2d, uint32> access;

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent1d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			return glm::vec<4, retType, P>(0, 0, 0, 1);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent2d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			gli::extent3d TexelCoord3d(TexelCoord, 0);
			return fetch(Texture, TexelCoord3d, Layer, Face, Level);
		}

		static vec<4, retType, P> fetch(textureType const& Texture, gli::extent3d const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_BC5SNORM requires an float sampler");

			if(Texture.target() == gli::TARGET_1D || Texture.target() == gli::TARGET_1D_ARRAY)
			{
				return glm::vec<4, retType, P>(0, 0, 0, 1);
			}

			const bc5_block *Data = Texture.template data<bc5_block>(Layer, Face, Level);
			const gli::extent3d &BlockExtent = block_extent(Texture.format());
			int WidthInBlocks = glm::max(1, Texture.extent(Level).x / BlockExtent.x);
			int BlocksInSlice = glm::max(1, Texture.extent(Level).y / BlockExtent.y) * WidthInBlocks;
			gli::extent3d BlockCoord(TexelCoord / BlockExtent);
			glm::ivec2 TexelCoordInBlock(TexelCoord.x - (BlockCoord.x * BlockExtent.x), TexelCoord.y - (BlockCoord.y * BlockExtent.y));

			const bc5_block &Block = Data[BlockCoord.z * BlocksInSlice + (BlockCoord.y * WidthInBlocks + BlockCoord.x)];

			return decompress_bc5snorm(Block, TexelCoordInBlock);
		}

		static void write(textureType& Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, retType, P> const & Texel)
		{
			static_assert(std::numeric_limits<retType>::is_iec559, "CONVERT_MODE_BC5SNORM requires an float sampler");

			GLI_ASSERT("Writing to single texel of a BC5 compressed image is not supported");
		}
	};

	template <typename textureType, typename samplerValType, qualifier P>
	struct convert
	{
		typedef vec<4, samplerValType, P>(*fetchFunc)(textureType const& Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level);
		typedef void(*writeFunc)(textureType & Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, samplerValType, P> const & Texel);

		template <length_t L, typename T, convertMode mode>
		struct conv
		{
			static vec<4, samplerValType, P> fetch(textureType const& Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level)
			{
				return convertFunc<textureType, samplerValType, L, T, P, mode, std::numeric_limits<samplerValType>::is_iec559>::fetch(Texture, TexelCoord, Layer, Face, Level);
			}

			static void write(textureType& Texture, typename textureType::extent_type const& TexelCoord, typename textureType::size_type Layer, typename textureType::size_type Face, typename textureType::size_type Level, vec<4, samplerValType, P> const & Texel)
			{
				convertFunc<textureType, samplerValType, L, T, P, mode, std::numeric_limits<samplerValType>::is_iec559>::write(Texture, TexelCoord, Layer, Face, Level, Texel);
			}
		};

		struct func
		{
			fetchFunc Fetch;
			writeFunc Write;
		};

		static func call(format Format)
		{
			static func Table[] =
			{
				{conv<2, u8, CONVERT_MODE_44UNORM>::fetch, conv<2, u8, CONVERT_MODE_44UNORM>::write},				// FORMAT_RG4_UNORM
				{conv<4, u8, CONVERT_MODE_4444UNORM>::fetch, conv<4, u8, CONVERT_MODE_4444UNORM>::write},			// FORMAT_RGBA4_UNORM
				{conv<4, u8, CONVERT_MODE_4444UNORM>::fetch, conv<4, u8, CONVERT_MODE_4444UNORM>::write},			// FORMAT_BGRA4_UNORM
				{conv<3, u8, CONVERT_MODE_565UNORM>::fetch, conv<3, u8, CONVERT_MODE_565UNORM>::write},				// FORMAT_R5G6B5_UNORM
				{conv<3, u8, CONVERT_MODE_565UNORM>::fetch, conv<3, u8, CONVERT_MODE_565UNORM>::write},				// FORMAT_B5G6R5_UNORM
				{conv<4, u8, CONVERT_MODE_5551UNORM>::fetch, conv<4, u8, CONVERT_MODE_5551UNORM>::write},			// FORMAT_RGB5A1_UNORM
				{conv<4, u8, CONVERT_MODE_5551UNORM>::fetch, conv<4, u8, CONVERT_MODE_5551UNORM>::write},			// FORMAT_BGR5A1_UNORM
				{conv<4, u8, CONVERT_MODE_5551UNORM>::fetch, conv<4, u8, CONVERT_MODE_5551UNORM>::write},			// FORMAT_A1RGB5_UNORM

				{conv<1, u8, CONVERT_MODE_NORM>::fetch, conv<1, u8, CONVERT_MODE_NORM>::write},						// FORMAT_R8_UNORM
				{conv<1, i8, CONVERT_MODE_NORM>::fetch, conv<1, i8, CONVERT_MODE_NORM>::write},						// FORMAT_R8_SNORM
				{conv<1, u8, CONVERT_MODE_CAST>::fetch, conv<1, u8, CONVERT_MODE_CAST>::write},						// FORMAT_R8_USCALED
				{conv<1, i8, CONVERT_MODE_CAST>::fetch, conv<1, i8, CONVERT_MODE_CAST>::write},						// FORMAT_R8_SSCALED
				{conv<1, u8, CONVERT_MODE_CAST>::fetch, conv<1, u8, CONVERT_MODE_CAST>::write},						// FORMAT_R8_UINT
				{conv<1, i8, CONVERT_MODE_CAST>::fetch, conv<1, i8, CONVERT_MODE_CAST>::write},						// FORMAT_R8_SINT
				{conv<1, u8, CONVERT_MODE_SRGB>::fetch, conv<1, u8, CONVERT_MODE_SRGB>::write},						// FORMAT_R8_SRGB

				{conv<2, u8, CONVERT_MODE_NORM>::fetch, conv<2, u8, CONVERT_MODE_NORM>::write},						// FORMAT_RG8_UNORM
				{conv<2, i8, CONVERT_MODE_NORM>::fetch, conv<2, i8, CONVERT_MODE_NORM>::write},						// FORMAT_RG8_SNORM
				{conv<2, u8, CONVERT_MODE_CAST>::fetch, conv<2, u8, CONVERT_MODE_CAST>::write},						// FORMAT_RG8_USCALED
				{conv<2, i8, CONVERT_MODE_CAST>::fetch, conv<2, i8, CONVERT_MODE_CAST>::write},						// FORMAT_RG8_SSCALED
				{conv<2, u8, CONVERT_MODE_CAST>::fetch, conv<2, u8, CONVERT_MODE_CAST>::write},						// FORMAT_RG8_UINT
				{conv<2, i8, CONVERT_MODE_CAST>::fetch, conv<2, i8, CONVERT_MODE_CAST>::write},						// FORMAT_RG8_SINT
				{conv<2, u8, CONVERT_MODE_SRGB>::fetch, conv<2, u8, CONVERT_MODE_SRGB>::write},						// FORMAT_RG8_SRGB

				{conv<3, u8, CONVERT_MODE_NORM>::fetch, conv<3, u8, CONVERT_MODE_NORM>::write},						// FORMAT_RGB8_UNORM
				{conv<3, i8, CONVERT_MODE_NORM>::fetch, conv<3, i8, CONVERT_MODE_NORM>::write},						// FORMAT_RGB8_SNORM
				{conv<3, u8, CONVERT_MODE_CAST>::fetch, conv<3, u8, CONVERT_MODE_CAST>::write},						// FORMAT_RGB8_USCALED
				{conv<3, i8, CONVERT_MODE_CAST>::fetch, conv<3, i8, CONVERT_MODE_CAST>::write},						// FORMAT_RGB8_SSCALED
				{conv<3, u8, CONVERT_MODE_CAST>::fetch, conv<3, u8, CONVERT_MODE_CAST>::write},						// FORMAT_RGB8_UINT
				{conv<3, i8, CONVERT_MODE_CAST>::fetch, conv<3, i8, CONVERT_MODE_CAST>::write},						// FORMAT_RGB8_SINT
				{conv<3, u8, CONVERT_MODE_SRGB>::fetch, conv<3, u8, CONVERT_MODE_SRGB>::write},						// FORMAT_RGB8_SRGB

				{conv<3, u8, CONVERT_MODE_NORM>::fetch, conv<3, u8, CONVERT_MODE_NORM>::write},						// FORMAT_BGR8_UNORM
				{conv<3, i8, CONVERT_MODE_NORM>::fetch, conv<3, i8, CONVERT_MODE_NORM>::write},						// FORMAT_BGR8_SNORM
				{conv<3, u8, CONVERT_MODE_CAST>::fetch, conv<3, u8, CONVERT_MODE_CAST>::write},						// FORMAT_BGR8_USCALED
				{conv<3, i8, CONVERT_MODE_CAST>::fetch, conv<3, i8, CONVERT_MODE_CAST>::write},						// FORMAT_BGR8_SSCALED
				{conv<3, u32, CONVERT_MODE_CAST>::fetch, conv<3, u32, CONVERT_MODE_CAST>::write},					// FORMAT_BGR8_UINT
				{conv<3, i32, CONVERT_MODE_CAST>::fetch, conv<3, i32, CONVERT_MODE_CAST>::write},					// FORMAT_BGR8_SINT
				{conv<3, u8, CONVERT_MODE_SRGB>::fetch, conv<3, u8, CONVERT_MODE_SRGB>::write},						// FORMAT_BGR8_SRGB

				{conv<4, u8, CONVERT_MODE_NORM>::fetch, conv<4, u8, CONVERT_MODE_NORM>::write},						// FORMAT_RGBA8_UNORM
				{conv<4, i8, CONVERT_MODE_NORM>::fetch, conv<4, i8, CONVERT_MODE_NORM>::write},						// FORMAT_RGBA8_SNORM
				{conv<4, u8, CONVERT_MODE_CAST>::fetch, conv<4, u8, CONVERT_MODE_CAST>::write},						// FORMAT_RGBA8_USCALED
				{conv<4, i8, CONVERT_MODE_CAST>::fetch, conv<4, i8, CONVERT_MODE_CAST>::write},						// FORMAT_RGBA8_SSCALED
				{conv<4, u8, CONVERT_MODE_CAST>::fetch, conv<4, u8, CONVERT_MODE_CAST>::write},						// FORMAT_RGBA8_UINT
				{conv<4, i8, CONVERT_MODE_CAST>::fetch, conv<4, i8, CONVERT_MODE_CAST>::write},						// FORMAT_RGBA8_SINT
				{conv<4, u8, CONVERT_MODE_SRGB>::fetch, conv<4, u8, CONVERT_MODE_SRGB>::write},						// FORMAT_RGBA8_SRGB

				{conv<4, u8, CONVERT_MODE_NORM>::fetch, conv<4, u8, CONVERT_MODE_NORM>::write},						// FORMAT_BGRA8_UNORM
				{conv<4, i8, CONVERT_MODE_NORM>::fetch, conv<4, i8, CONVERT_MODE_NORM>::write},						// FORMAT_BGRA8_SNORM
				{conv<4, u8, CONVERT_MODE_CAST>::fetch, conv<4, u8, CONVERT_MODE_CAST>::write},						// FORMAT_BGRA8_USCALED
				{conv<4, i8, CONVERT_MODE_CAST>::fetch, conv<4, i8, CONVERT_MODE_CAST>::write},						// FORMAT_BGRA8_SSCALED
				{conv<4, u8, CONVERT_MODE_CAST>::fetch, conv<4, u8, CONVERT_MODE_CAST>::write},						// FORMAT_BGRA8_UINT
				{conv<4, i8, CONVERT_MODE_CAST>::fetch, conv<4, i8, CONVERT_MODE_CAST>::write},						// FORMAT_BGRA8_SINT
				{conv<4, u8, CONVERT_MODE_SRGB>::fetch, conv<4, u8, CONVERT_MODE_SRGB>::write},						// FORMAT_BGRA8_SRGB

				{conv<4, u8, CONVERT_MODE_NORM>::fetch, conv<4, u8, CONVERT_MODE_NORM>::write},						// FORMAT_ABGR8_UNORM
				{conv<4, i8, CONVERT_MODE_NORM>::fetch, conv<4, i8, CONVERT_MODE_NORM>::write},						// FORMAT_ABGR8_SNORM
				{conv<4, u8, CONVERT_MODE_CAST>::fetch, conv<4, u8, CONVERT_MODE_CAST>::write},						// FORMAT_ABGR8_USCALED
				{conv<4, i8, CONVERT_MODE_CAST>::fetch, conv<4, i8, CONVERT_MODE_CAST>::write},						// FORMAT_ABGR8_SSCALED
				{conv<4, u8, CONVERT_MODE_CAST>::fetch, conv<4, u8, CONVERT_MODE_CAST>::write},						// FORMAT_ABGR8_UINT
				{conv<4, i8, CONVERT_MODE_CAST>::fetch, conv<4, i8, CONVERT_MODE_CAST>::write},						// FORMAT_ABGR8_SINT
				{conv<4, u8, CONVERT_MODE_SRGB>::fetch, conv<4, u8, CONVERT_MODE_SRGB>::write},						// FORMAT_ABGR8_SRGB

				{conv<4, u8, CONVERT_MODE_RGB10A2UNORM>::fetch, conv<4, u8, CONVERT_MODE_RGB10A2UNORM>::write},		// FORMAT_RGB10A2_UNORM
				{conv<4, i8, CONVERT_MODE_RGB10A2SNORM>::fetch, conv<4, i8, CONVERT_MODE_RGB10A2SNORM>::write},		// FORMAT_RGB10A2_SNORM
				{conv<4, u8, CONVERT_MODE_RGB10A2USCALE>::fetch, conv<4, u8, CONVERT_MODE_RGB10A2USCALE>::write},	// FORMAT_RGB10A2_USCALED
				{conv<4, i8, CONVERT_MODE_RGB10A2SSCALE>::fetch, conv<4, i8, CONVERT_MODE_RGB10A2SSCALE>::write},	// FORMAT_RGB10A2_SSCALED
				{conv<4, u8, CONVERT_MODE_RGB10A2UINT>::fetch, conv<4, u8, CONVERT_MODE_RGB10A2UINT>::write},		// FORMAT_RGB10A2_UINT
				{conv<4, i8, CONVERT_MODE_RGB10A2SINT>::fetch, conv<4, i8, CONVERT_MODE_RGB10A2SINT>::write},		// FORMAT_RGB10A2_SINT

				{conv<4, u8, CONVERT_MODE_RGB10A2UNORM>::fetch, conv<4, u8, CONVERT_MODE_RGB10A2UNORM>::write},		// FORMAT_BGR10A2_UNORM
				{conv<4, i8, CONVERT_MODE_RGB10A2SNORM>::fetch, conv<4, i8, CONVERT_MODE_RGB10A2SNORM>::write},		// FORMAT_BGR10A2_SNORM
				{conv<4, u8, CONVERT_MODE_RGB10A2USCALE>::fetch, conv<4, u8, CONVERT_MODE_RGB10A2USCALE>::write},	// FORMAT_BGR10A2_USCALED
				{conv<4, i8, CONVERT_MODE_RGB10A2SSCALE>::fetch, conv<4, i8, CONVERT_MODE_RGB10A2SSCALE>::write},	// FORMAT_BGR10A2_SSCALED
				{conv<4, u8, CONVERT_MODE_RGB10A2UINT>::fetch, conv<4, u8, CONVERT_MODE_RGB10A2UINT>::write},		// FORMAT_BGR10A2_UINT
				{conv<4, i8, CONVERT_MODE_RGB10A2SINT>::fetch, conv<4, i8, CONVERT_MODE_RGB10A2SINT>::write},		// FORMAT_BGR10A2_SINT

				{conv<1, u16, CONVERT_MODE_NORM>::fetch, conv<1, u16, CONVERT_MODE_NORM>::write},					// FORMAT_R16_UNORM_PACK16
				{conv<1, i16, CONVERT_MODE_NORM>::fetch, conv<1, i16, CONVERT_MODE_NORM>::write},					// FORMAT_R16_SNORM_PACK16
				{conv<1, u16, CONVERT_MODE_CAST>::fetch, conv<1, u16, CONVERT_MODE_CAST>::write},					// FORMAT_R16_USCALED_PACK16
				{conv<1, i16, CONVERT_MODE_CAST>::fetch, conv<1, i16, CONVERT_MODE_CAST>::write},					// FORMAT_R16_SSCALED_PACK16
				{conv<1, u16, CONVERT_MODE_CAST>::fetch, conv<1, u16, CONVERT_MODE_CAST>::write},					// FORMAT_R16_UINT_PACK16
				{conv<1, i16, CONVERT_MODE_CAST>::fetch, conv<1, i16, CONVERT_MODE_CAST>::write},					// FORMAT_R16_SINT_PACK16
				{conv<1, u16, CONVERT_MODE_HALF>::fetch, conv<1, u16, CONVERT_MODE_HALF>::write},					// FORMAT_R16_SFLOAT_PACK16

				{conv<2, u16, CONVERT_MODE_NORM>::fetch, conv<2, u16, CONVERT_MODE_NORM>::write},					// FORMAT_RG16_UNORM_PACK16
				{conv<2, i16, CONVERT_MODE_NORM>::fetch, conv<2, i16, CONVERT_MODE_NORM>::write},					// FORMAT_RG16_SNORM_PACK16
				{conv<2, u16, CONVERT_MODE_CAST>::fetch, conv<2, u16, CONVERT_MODE_CAST>::write},					// FORMAT_RG16_USCALED_PACK16
				{conv<2, i16, CONVERT_MODE_CAST>::fetch, conv<2, i16, CONVERT_MODE_CAST>::write},					// FORMAT_RG16_SSCALED_PACK16
				{conv<2, u16, CONVERT_MODE_CAST>::fetch, conv<2, u16, CONVERT_MODE_CAST>::write},					// FORMAT_RG16_UINT_PACK16
				{conv<2, i16, CONVERT_MODE_CAST>::fetch, conv<2, i16, CONVERT_MODE_CAST>::write},					// FORMAT_RG16_SINT_PACK16
				{conv<2, u16, CONVERT_MODE_HALF>::fetch, conv<2, u16, CONVERT_MODE_HALF>::write},					// FORMAT_RG16_SFLOAT_PACK16

				{conv<3, u16, CONVERT_MODE_NORM>::fetch, conv<3, u16, CONVERT_MODE_NORM>::write},					// FORMAT_RGB16_UNORM_PACK16
				{conv<3, i16, CONVERT_MODE_NORM>::fetch, conv<3, i16, CONVERT_MODE_NORM>::write},					// FORMAT_RGB16_SNORM_PACK16
				{conv<3, u16, CONVERT_MODE_CAST>::fetch, conv<3, u16, CONVERT_MODE_CAST>::write},					// FORMAT_RGB16_USCALED_PACK16
				{conv<3, i16, CONVERT_MODE_CAST>::fetch, conv<3, i16, CONVERT_MODE_CAST>::write},					// FORMAT_RGB16_SSCALED_PACK16
				{conv<3, u16, CONVERT_MODE_CAST>::fetch, conv<3, u16, CONVERT_MODE_CAST>::write},					// FORMAT_RGB16_UINT_PACK16
				{conv<3, i16, CONVERT_MODE_CAST>::fetch, conv<3, i16, CONVERT_MODE_CAST>::write},					// FORMAT_RGB16_SINT_PACK16
				{conv<3, u16, CONVERT_MODE_HALF>::fetch, conv<3, u16, CONVERT_MODE_HALF>::write},					// FORMAT_RGB16_SFLOAT_PACK16

				{conv<4, u16, CONVERT_MODE_NORM>::fetch, conv<4, u16, CONVERT_MODE_NORM>::write},					// FORMAT_RGBA16_UNORM_PACK16
				{conv<4, i16, CONVERT_MODE_NORM>::fetch, conv<4, i16, CONVERT_MODE_NORM>::write},					// FORMAT_RGBA16_SNORM_PACK16
				{conv<4, u16, CONVERT_MODE_CAST>::fetch, conv<4, u16, CONVERT_MODE_CAST>::write},					// FORMAT_RGBA16_USCALED_PACK16
				{conv<4, i16, CONVERT_MODE_CAST>::fetch, conv<4, i16, CONVERT_MODE_CAST>::write},					// FORMAT_RGBA16_SSCALED_PACK16
				{conv<4, u16, CONVERT_MODE_CAST>::fetch, conv<4, u16, CONVERT_MODE_CAST>::write},					// FORMAT_RGBA16_UINT_PACK16
				{conv<4, i16, CONVERT_MODE_CAST>::fetch, conv<4, i16, CONVERT_MODE_CAST>::write},					// FORMAT_RGBA16_SINT_PACK16
				{conv<4, u16, CONVERT_MODE_HALF>::fetch, conv<4, u16, CONVERT_MODE_HALF>::write},					// FORMAT_RGBA16_SFLOAT_PACK16

				{conv<1, u32, CONVERT_MODE_CAST>::fetch, conv<1, u32, CONVERT_MODE_CAST>::write},					// FORMAT_R32_UINT_PACK32
				{conv<1, i32, CONVERT_MODE_CAST>::fetch, conv<1, i32, CONVERT_MODE_CAST>::write},					// FORMAT_R32_SINT_PACK32
				{conv<1, f32, CONVERT_MODE_CAST>::fetch, conv<1, f32, CONVERT_MODE_CAST>::write},					// FORMAT_R32_SFLOAT_PACK32

				{conv<2, u32, CONVERT_MODE_CAST>::fetch, conv<2, u32, CONVERT_MODE_CAST>::write},					// FORMAT_RG32_UINT_PACK32
				{conv<2, i32, CONVERT_MODE_CAST>::fetch, conv<2, i32, CONVERT_MODE_CAST>::write},					// FORMAT_RG32_SINT_PACK32
				{conv<2, f32, CONVERT_MODE_CAST>::fetch, conv<2, f32, CONVERT_MODE_CAST>::write},					// FORMAT_RG32_SFLOAT_PACK32

				{conv<3, u32, CONVERT_MODE_CAST>::fetch, conv<3, u32, CONVERT_MODE_CAST>::write},					// FORMAT_RGB32_UINT_PACK32
				{conv<3, i32, CONVERT_MODE_CAST>::fetch, conv<3, i32, CONVERT_MODE_CAST>::write},					// FORMAT_RGB32_SINT_PACK32
				{conv<3, f32, CONVERT_MODE_CAST>::fetch, conv<3, f32, CONVERT_MODE_CAST>::write},					// FORMAT_RGB32_SFLOAT_PACK32

				{conv<4, u32, CONVERT_MODE_CAST>::fetch, conv<4, u32, CONVERT_MODE_CAST>::write},					// FORMAT_RGBA32_UINT_PACK32
				{conv<4, i32, CONVERT_MODE_CAST>::fetch, conv<4, i32, CONVERT_MODE_CAST>::write},					// FORMAT_RGBA32_SINT_PACK32
				{conv<4, f32, CONVERT_MODE_CAST>::fetch, conv<4, f32, CONVERT_MODE_CAST>::write},					// FORMAT_RGBA32_SFLOAT_PACK32

				{conv<1, u64, CONVERT_MODE_CAST>::fetch, conv<1, u64, CONVERT_MODE_CAST>::write},					// FORMAT_R64_UINT_PACK64
				{conv<1, i64, CONVERT_MODE_CAST>::fetch, conv<1, i64, CONVERT_MODE_CAST>::write},					// FORMAT_R64_SINT_PACK64
				{conv<1, f64, CONVERT_MODE_CAST>::fetch, conv<1, f64, CONVERT_MODE_CAST>::write},					// FORMAT_R64_SFLOAT_PACK64

				{conv<2, u64, CONVERT_MODE_CAST>::fetch, conv<2, u64, CONVERT_MODE_CAST>::write},					// FORMAT_RG64_UINT_PACK64
				{conv<2, i64, CONVERT_MODE_CAST>::fetch, conv<2, i64, CONVERT_MODE_CAST>::write},					// FORMAT_RG64_SINT_PACK64
				{conv<2, f64, CONVERT_MODE_CAST>::fetch, conv<2, f64, CONVERT_MODE_CAST>::write},					// FORMAT_RG64_SFLOAT_PACK64

				{conv<3, u64, CONVERT_MODE_CAST>::fetch, conv<3, u64, CONVERT_MODE_CAST>::write},					// FORMAT_RGB64_UINT_PACK64
				{conv<3, i64, CONVERT_MODE_CAST>::fetch, conv<3, i64, CONVERT_MODE_CAST>::write},					// FORMAT_RGB64_SINT_PACK64
				{conv<3, f64, CONVERT_MODE_CAST>::fetch, conv<3, f64, CONVERT_MODE_CAST>::write},					// FORMAT_RGB64_SFLOAT_PACK64

				{conv<4, u64, CONVERT_MODE_CAST>::fetch, conv<4, u64, CONVERT_MODE_CAST>::write},					// FORMAT_RGBA64_UINT_PACK64
				{conv<4, i64, CONVERT_MODE_CAST>::fetch, conv<4, i64, CONVERT_MODE_CAST>::write},					// FORMAT_RGBA64_SINT_PACK64
				{conv<4, f64, CONVERT_MODE_CAST>::fetch, conv<4, f64, CONVERT_MODE_CAST>::write},					// FORMAT_RGBA64_SFLOAT_PACK64

				{conv<1, u32, CONVERT_MODE_RG11B10F>::fetch, conv<1, u32, CONVERT_MODE_RG11B10F>::write},			// FORMAT_RG11B10_UFLOAT
				{conv<1, u32, CONVERT_MODE_RGB9E5>::fetch, conv<1, u32, CONVERT_MODE_RGB9E5>::write},				// FORMAT_RGB9E5_UFLOAT

				{conv<1, u16, CONVERT_MODE_DEFAULT>::fetch, conv<1, u16, CONVERT_MODE_DEFAULT>::write},				// FORMAT_D16_UNORM_PACK16
				{conv<1, u32, CONVERT_MODE_DEFAULT>::fetch, conv<1, u32, CONVERT_MODE_DEFAULT>::write},				// FORMAT_D24_UNORM
				{conv<1, f32, CONVERT_MODE_DEFAULT>::fetch, conv<1, f32, CONVERT_MODE_DEFAULT>::write},				// FORMAT_D32_SFLOAT_PACK32
				{conv<1, u8, CONVERT_MODE_DEFAULT>::fetch, conv<1, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_S8_UINT_PACK8
				{conv<2, u16, CONVERT_MODE_DEFAULT>::fetch, conv<2, u16, CONVERT_MODE_DEFAULT>::write},				// FORMAT_D16_UNORM_S8_UINT_PACK32
				{conv<2, u32, CONVERT_MODE_DEFAULT>::fetch, conv<2, u32, CONVERT_MODE_DEFAULT>::write},				// FORMAT_D24_UNORM_S8_UINT_PACK32
				{conv<2, u32, CONVERT_MODE_DEFAULT>::fetch, conv<2, u32, CONVERT_MODE_DEFAULT>::write},				// FORMAT_D32_SFLOAT_S8_UINT_PACK64

				{conv<3, u8, CONVERT_MODE_DXT1UNORM>::fetch, conv<3, u8, CONVERT_MODE_DXT1UNORM>::write},			// FORMAT_RGB_DXT1_UNORM_BLOCK8
				{conv<3, u8, CONVERT_MODE_DXT1UNORM>::fetch, conv<3, u8, CONVERT_MODE_DXT1UNORM>::write},			// FORMAT_RGB_DXT1_SRGB_BLOCK8
				{conv<4, u8, CONVERT_MODE_DXT1UNORM>::fetch, conv<4, u8, CONVERT_MODE_DXT1UNORM>::write},			// FORMAT_RGBA_DXT1_UNORM_BLOCK8
				{conv<4, u8, CONVERT_MODE_DXT1UNORM>::fetch, conv<4, u8, CONVERT_MODE_DXT1UNORM>::write},			// FORMAT_RGBA_DXT1_SRGB_BLOCK8
				{conv<4, u8, CONVERT_MODE_DXT3UNORM>::fetch, conv<4, u8, CONVERT_MODE_DXT3UNORM>::write},			// FORMAT_RGBA_DXT3_UNORM_BLOCK16
				{conv<4, u8, CONVERT_MODE_DXT3UNORM>::fetch, conv<4, u8, CONVERT_MODE_DXT3UNORM>::write},			// FORMAT_RGBA_DXT3_SRGB_BLOCK16
				{conv<4, u8, CONVERT_MODE_DXT5UNORM>::fetch, conv<4, u8, CONVERT_MODE_DXT5UNORM>::write},			// FORMAT_RGBA_DXT5_UNORM_BLOCK16
				{conv<4, u8, CONVERT_MODE_DXT5UNORM>::fetch, conv<4, u8, CONVERT_MODE_DXT5UNORM>::write},			// FORMAT_RGBA_DXT5_SRGB_BLOCK16
				{conv<1, u8, CONVERT_MODE_BC4UNORM>::fetch, conv<1, u8, CONVERT_MODE_BC4UNORM>::write},				// FORMAT_R_ATI1N_UNORM_BLOCK8
				{conv<1, u8, CONVERT_MODE_BC4SNORM>::fetch, conv<1, i8, CONVERT_MODE_BC4SNORM>::write},				// FORMAT_R_ATI1N_SNORM_BLOCK8
				{conv<2, u8, CONVERT_MODE_BC5UNORM>::fetch, conv<2, u8, CONVERT_MODE_BC5UNORM>::write},				// FORMAT_RG_ATI2N_UNORM_BLOCK16
				{conv<2, u8, CONVERT_MODE_BC5SNORM>::fetch, conv<2, i8, CONVERT_MODE_BC5SNORM>::write},				// FORMAT_RG_ATI2N_SNORM_BLOCK16
				{conv<3, f32, CONVERT_MODE_DEFAULT>::fetch, conv<3, f32, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGB_BP_UFLOAT_BLOCK16
				{conv<3, f32, CONVERT_MODE_DEFAULT>::fetch, conv<3, f32, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGB_BP_SFLOAT_BLOCK16
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_BP_UNORM_BLOCK16
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_BP_SRGB_BLOCK16

				{conv<3, u8, CONVERT_MODE_DEFAULT>::fetch, conv<3, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGB_ETC2_UNORM_BLOCK8
				{conv<3, u8, CONVERT_MODE_DEFAULT>::fetch, conv<3, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGB_ETC2_SRGB_BLOCK8
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_ETC2_A1_UNORM_BLOCK8
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_ETC2_A1_SRGB_BLOCK8
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_ETC2_UNORM_BLOCK16
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_ETC2_SRGB_BLOCK16
				{conv<1, u8, CONVERT_MODE_DEFAULT>::fetch, conv<1, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_R_EAC_UNORM_BLOCK8
				{conv<1, u8, CONVERT_MODE_DEFAULT>::fetch, conv<1, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_R_EAC_SNORM_BLOCK8
				{conv<2, u8, CONVERT_MODE_DEFAULT>::fetch, conv<2, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RG_EAC_UNORM_BLOCK16
				{conv<2, u8, CONVERT_MODE_DEFAULT>::fetch, conv<2, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RG_EAC_SNORM_BLOCK16

				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_4x4_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_4x4_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_5x4_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_5x4_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_5x5_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_5x5_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_6x5_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_6x5_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_6x6_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_6x6_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_8x5_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_8x5_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_8x6_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_8x6_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_8x8_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_8x8_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_10x5_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_10x5_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_10x6_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_10x6_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_10x8_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_10x8_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_10x10_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_10x10_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_12x10_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_12x10_SRGB
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_12x12_UNORM
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_ASTC_12x12_SRGB

				{conv<3, u8, CONVERT_MODE_DEFAULT>::fetch, conv<3, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGB_PVRTC1_8X8_UNORM_BLOCK32
				{conv<3, u8, CONVERT_MODE_DEFAULT>::fetch, conv<3, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGB_PVRTC1_8X8_SRGB_BLOCK32
				{conv<3, u8, CONVERT_MODE_DEFAULT>::fetch, conv<3, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGB_PVRTC1_16X8_UNORM_BLOCK32
				{conv<3, u8, CONVERT_MODE_DEFAULT>::fetch, conv<3, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGB_PVRTC1_16X8_SRGB_BLOCK32
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_PVRTC1_8X8_UNORM_BLOCK32
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_PVRTC1_8X8_SRGB_BLOCK32
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_PVRTC1_16X8_UNORM_BLOCK32
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_PVRTC1_16X8_SRGB_BLOCK32
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_PVRTC2_4X4_UNORM_BLOCK8
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_PVRTC2_4X4_SRGB_BLOCK8
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_PVRTC2_8X4_UNORM_BLOCK8
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_PVRTC2_8X4_SRGB_BLOCK8

				{conv<3, u8, CONVERT_MODE_DEFAULT>::fetch, conv<3, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGB_ETC_UNORM_BLOCK8
				{conv<3, u8, CONVERT_MODE_DEFAULT>::fetch, conv<3, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGB_ATC_UNORM_BLOCK8
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_ATCA_UNORM_BLOCK16
				{conv<4, u8, CONVERT_MODE_DEFAULT>::fetch, conv<4, u8, CONVERT_MODE_DEFAULT>::write},				// FORMAT_RGBA_ATCI_UNORM_BLOCK16

				{conv<1, u8, CONVERT_MODE_NORM>::fetch, conv<1, u8, CONVERT_MODE_NORM>::write},						// FORMAT_L8_UNORM_PACK8
				{conv<1, u8, CONVERT_MODE_NORM>::fetch, conv<1, u8, CONVERT_MODE_NORM>::write},						// FORMAT_A8_UNORM_PACK8
				{conv<2, u8, CONVERT_MODE_NORM>::fetch, conv<2, u8, CONVERT_MODE_NORM>::write},						// FORMAT_LA8_UNORM_PACK8
				{conv<1, u16, CONVERT_MODE_NORM>::fetch, conv<1, u16, CONVERT_MODE_NORM>::write},					// FORMAT_L16_UNORM_PACK16
				{conv<1, u16, CONVERT_MODE_NORM>::fetch, conv<1, u16, CONVERT_MODE_NORM>::write},					// FORMAT_A16_UNORM_PACK16
				{conv<2, u16, CONVERT_MODE_NORM>::fetch, conv<2, u16, CONVERT_MODE_NORM>::write},					// FORMAT_LA16_UNORM_PACK16

				{conv<4, u8, CONVERT_MODE_NORM>::fetch, conv<4, u8, CONVERT_MODE_NORM>::write},						// FORMAT_BGRX8_UNORM
				{conv<4, u8, CONVERT_MODE_SRGB>::fetch, conv<4, u8, CONVERT_MODE_SRGB>::write},						// FORMAT_BGRX8_SRGB

				{conv<3, u8, CONVERT_MODE_332UNORM>::fetch, conv<3, u8, CONVERT_MODE_332UNORM>::write}				// FORMAT_RG3B2_UNORM
			};
			static_assert(sizeof(Table) / sizeof(Table[0]) == FORMAT_COUNT, "Texel functions need to be updated");

			return Table[Format - FORMAT_FIRST];
		}
	};
}//namespace detail
}//namespace gli
