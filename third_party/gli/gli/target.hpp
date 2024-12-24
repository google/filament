/// @brief Include to use the target enum and query properties of targets.
/// @file gli/target.hpp

#pragma once

namespace gli
{
	/// Texture target: type/shape of the texture storage_linear
	enum target
	{
		TARGET_1D = 0, TARGET_FIRST = TARGET_1D,
		TARGET_1D_ARRAY,
		TARGET_2D,
		TARGET_2D_ARRAY,
		TARGET_3D,
		TARGET_RECT,
		TARGET_RECT_ARRAY,
		TARGET_CUBE,
		TARGET_CUBE_ARRAY, TARGET_LAST = TARGET_CUBE_ARRAY
	};

	enum
	{
		TARGET_COUNT = TARGET_LAST - TARGET_FIRST + 1,
		TARGET_INVALID = -1
	};

	/// Check whether a target is a 1D target
	inline bool is_target_1d(target Target)
	{
		return Target == TARGET_1D || Target == TARGET_1D_ARRAY;
	}

	/// Check whether a target is an array target
	inline bool is_target_array(target Target)
	{
		return Target == TARGET_1D_ARRAY || Target == TARGET_2D_ARRAY || Target == TARGET_CUBE_ARRAY;
	}

	/// Check whether a target is a cube map target
	inline bool is_target_cube(target Target)
	{
		return Target == TARGET_CUBE || Target == TARGET_CUBE_ARRAY;
	}
	
	/// Check whether a target is a rectangle target
	inline bool is_target_rect(target Target)
	{
		return Target == TARGET_RECT || Target == TARGET_RECT_ARRAY;
	}
}//namespace gli
