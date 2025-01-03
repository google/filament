/// @brief Include to perform arithmetic per texel between two textures.
/// @file gli/transform.hpp

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
	struct transform_func
	{
		typedef vec_type(*type)(vec_type const & A, vec_type const & B);
	};

	/// Compute per-texel operations using a user defined function.
	///
	/// @param Out Output texture.
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function.
	template <typename vec_type>
	void transform(texture1d & Out, texture1d const & In0, texture1d const & In1, typename transform_func<vec_type>::type TexelFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param Out Output texture.
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function.
	template <typename vec_type>
	void transform(texture1d_array & Out, texture1d_array const & In0, texture1d_array const & In1, typename transform_func<vec_type>::type TexelFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param Out Output texture.
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function.
	template <typename vec_type>
	void transform(texture2d & Out, texture2d const & In0, texture2d const & In1, typename transform_func<vec_type>::type TexelFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param Out Output texture.
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function.
	template <typename vec_type>
	void transform(texture2d_array & Out, texture2d_array const & In0, texture2d_array const & In1, typename transform_func<vec_type>::type TexelFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param Out Output texture.
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function.
	template <typename vec_type>
	void transform(texture3d & Out, texture3d const & In0, texture3d const & In1, typename transform_func<vec_type>::type TexelFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param Out Output texture.
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function.
	template <typename vec_type>
	void transform(texture_cube & Out, texture_cube const & In0, texture_cube const & In1, typename transform_func<vec_type>::type TexelFunc);

	/// Compute per-texel operations using a user defined function.
	///
	/// @param Out Output texture.
	/// @param In0 First input texture.
	/// @param In1 Second input texture.
	/// @param TexelFunc Pointer to a binary function.
	template <typename vec_type>
	void transform(texture_cube_array & Out, texture_cube_array const & In0, texture_cube_array const & In1, typename transform_func<vec_type>::type TexelFunc);
	
}//namespace gli

#include "./core/transform.inl"
