/// @ref ext_vector_int1_sized
/// @file glm/ext/vector_int1_sized.hpp
///
/// @defgroup ext_vector_int1_sized GLM_EXT_vector_int1_sized
/// @ingroup ext
///
/// Exposes sized signed integer vector types.
///
/// Include <glm/ext/vector_int1_sized.hpp> to use the features of this extension.
///
/// @see ext_scalar_int_sized
/// @see ext_vector_uint1_sized

#pragma once

#include "../ext/vector_int1.hpp"
#include "../ext/scalar_int_sized.hpp"

#if GLM_MESSAGES == GLM_ENABLE && !defined(GLM_EXT_INCLUDED)
#	pragma message("GLM: GLM_EXT_vector_int1_sized extension included")
#endif

namespace glm
{
	/// @addtogroup ext_vector_int1_sized
	/// @{

	/// 8 bit signed integer vector of 1 component type.
	///
	/// @see ext_vector_int1_sized
	typedef vec<1, int8, defaultp>	i8vec1;

	/// 16 bit signed integer vector of 1 component type.
	///
	/// @see ext_vector_int1_sized
	typedef vec<1, int16, defaultp>	i16vec1;

	/// 32 bit signed integer vector of 1 component type.
	///
	/// @see ext_vector_int1_sized
	typedef vec<1, int32, defaultp>	i32vec1;

	/// 64 bit signed integer vector of 1 component type.
	///
	/// @see ext_vector_int1_sized
	typedef vec<1, int64, defaultp>	i64vec1;

	/// @}
}//namespace glm
