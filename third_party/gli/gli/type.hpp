/// @brief Include to use basic GLI types.
/// @file gli/type.hpp

#pragma once

// STD
#include <cstddef>

// GLM
#define GLM_FORCE_EXPLICIT_CTOR
#include <glm/glm.hpp>
#include <glm/ext/vector_int1.hpp>
#include <glm/ext/scalar_uint_sized.hpp>

#if GLM_COMPILER & GLM_COMPILER_VC
#	define GLI_FORCE_INLINE __forceinline
#elif GLM_COMPILER & (GLM_COMPILER_GCC | GLM_COMPILER_APPLE_CLANG | GLM_COMPILER_LLVM)
#	define GLI_FORCE_INLINE inline __attribute__((__always_inline__))
#else
#	define GLI_FORCE_INLINE inline
#endif//GLM_COMPILER

#define GLI_DISABLE_ASSERT 0

#if defined(NDEBUG) || GLI_DISABLE_ASSERT
#	define GLI_ASSERT(test)
#else
#	define GLI_ASSERT(test) assert((test))
#endif

namespace gli
{
	using namespace glm;

	using std::size_t;
	typedef glm::uint8 byte;

	typedef ivec1 extent1d;
	typedef ivec2 extent2d;
	typedef ivec3 extent3d;
	typedef ivec4 extent4d;

	template <typename T, qualifier P>
	inline vec<4, T, P> make_vec4(vec<1, T, P> const & v)
	{
		return vec<4, T, P>(v.x, static_cast<T>(0), static_cast<T>(0), static_cast<T>(1));
	}

	template <typename T, qualifier P>
	inline vec<4, T, P> make_vec4(vec<2, T, P> const & v)
	{
		return vec<4, T, P>(v.x, v.y, static_cast<T>(0), static_cast<T>(1));
	}

	template <typename T, qualifier P>
	inline vec<4, T, P> make_vec4(vec<3, T, P> const & v)
	{
		return vec<4, T, P>(v.x, v.y, v.z, static_cast<T>(1));
	}

	template <typename T, qualifier P>
	inline vec<4, T, P> make_vec4(vec<4, T, P> const & v)
	{
		return v;
	}
}//namespace gli
