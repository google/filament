/// @brief Include to perform reduction operations.
/// @file gli/reduce.hpp

#pragma once

#include "texture1d.hpp"
#include "texture1d_array.hpp"
#include "texture2d.hpp"
#include "texture2d_array.hpp"
#include "texture3d.hpp"
#include "texture_cube.hpp"
#include "texture_cube_array.hpp"

namespace gli
{
	template <typename vec_type>
	struct reduce_func
	{
		typedef vec_type(*type)(vec_type const & A, vec_type const & B);
	};

	/// Compute per-texel operations using a user defined function.
	///
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function for per texel operation.
	/// @param ReduceFunc Pointer to a binary function to reduce texels.
	template <typename vec_type>
	vec_type reduce(texture1d const & In0, texture1d const & In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function for per texel operation.
	/// @param ReduceFunc Pointer to a binary function to reduce texels.
	template <typename vec_type>
	vec_type reduce(texture1d_array const & In0, texture1d_array const & In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function for per texel operation.
	/// @param ReduceFunc Pointer to a binary function to reduce texels.
	template <typename vec_type>
	vec_type reduce(texture2d const & In0, texture2d const & In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function for per texel operation.
	/// @param ReduceFunc Pointer to a binary function to reduce texels.
	template <typename vec_type>
	vec_type reduce(texture2d_array const & In0, texture2d_array const & In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function for per texel operation.
	/// @param ReduceFunc Pointer to a binary function to reduce texels.
	template <typename vec_type>
	vec_type reduce(texture3d const & In0, texture3d const & In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function for per texel operation.
	/// @param ReduceFunc Pointer to a binary function to reduce texels.
	template <typename vec_type>
	vec_type reduce(texture_cube const & In0, texture_cube const & In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function for per texel operation.
	/// @param ReduceFunc Pointer to a binary function to reduce texels.
	template <typename vec_type>
	vec_type reduce(texture_cube_array const & In0, texture_cube_array const & In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function for per texel operation.
	/// @param ReduceFunc Pointer to a binary function to reduce texels.
	template <typename texture_type, typename vec_type>
	vec_type reduce(texture_type const & In0, texture_type const & In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc);
}//namespace gli

#include "./core/reduce.inl"
