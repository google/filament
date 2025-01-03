#include "../sampler1d.hpp"
#include "../sampler1d_array.hpp"
#include "../sampler2d.hpp"
#include "../sampler2d_array.hpp"
#include "../sampler3d.hpp"
#include "../sampler_cube.hpp"
#include "../sampler_cube_array.hpp"

namespace gli
{
	template <typename val_type>
	struct binary_func
	{
		typedef vec<4, val_type>(*type)(vec<4, val_type> const& A, vec<4, val_type> const& B);
	};

namespace detail
{
	inline bool are_compatible(texture const& A, texture const& B)
	{
		return all(equal(A.extent(), B.extent())) && A.levels() == B.levels() && A.faces() == B.faces() && A.layers() == B.layers();
	}

	template <typename val_type>
	struct compute_sampler_reduce_1d
	{
		typedef typename binary_func<val_type>::type func_type;
		typedef texture1d::size_type size_type;
		typedef texture1d::extent_type extent_type;

		static vec<4, val_type> call(texture1d const& A, texture1d const& B, binary_func<val_type> TexelFunc, binary_func<val_type> ReduceFunc)
		{
			GLI_ASSERT(are_compatible(A, B));

			sampler1d<val_type> const SamplerA(A, gli::WRAP_CLAMP_TO_EDGE), SamplerB(B, gli::WRAP_CLAMP_TO_EDGE);
			extent_type TexelIndex(0);
			vec<4, val_type> Result(TexelFunc(SamplerA.template fetch(TexelIndex, 0), SamplerB.template fetch(TexelIndex, 0)));

			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						SamplerA.template fetch(TexelIndex, LevelIndex),
						SamplerB.template fetch(TexelIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};

	template <typename val_type>
	struct compute_sampler_reduce_1d_array
	{
		typedef typename binary_func<val_type>::type func_type;
		typedef texture1d_array::size_type size_type;
		typedef texture1d_array::extent_type extent_type;

		static vec<4, val_type> call(texture1d_array const& A, texture1d_array const& B, binary_func<val_type> TexelFunc, binary_func<val_type> ReduceFunc)
		{
			GLI_ASSERT(are_compatible(A, B));

			sampler1d_array<val_type> const SamplerA(A, gli::WRAP_CLAMP_TO_EDGE), SamplerB(B, gli::WRAP_CLAMP_TO_EDGE);
			extent_type TexelIndex(0);
			vec<4, val_type> Result(TexelFunc(SamplerA.template fetch(TexelIndex, 0, 0), SamplerB.template fetch(TexelIndex, 0, 0)));

			for(size_type LayerIndex = 0, LayerCount = A.layers(); LayerIndex < LayerCount; ++LayerIndex)
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						SamplerA.template fetch(TexelIndex, LayerIndex, LevelIndex),
						SamplerB.template fetch(TexelIndex, LayerIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};

	template <typename val_type>
	struct compute_sampler_reduce_2d
	{
		typedef typename binary_func<val_type>::type func_type;
		typedef texture2d::size_type size_type;
		typedef texture2d::extent_type extent_type;

		static vec<4, val_type> call(texture2d const& A, texture2d const& B, binary_func<val_type> TexelFunc, binary_func<val_type> ReduceFunc)
		{
			GLI_ASSERT(are_compatible(A, B));

			sampler2d<val_type> const SamplerA(A, gli::WRAP_CLAMP_TO_EDGE), SamplerB(B, gli::WRAP_CLAMP_TO_EDGE);
			extent_type TexelIndex(0);
			vec<4, val_type> Result(TexelFunc(SamplerA.template fetch(TexelIndex, 0), SamplerB.template fetch(TexelIndex, 0)));

			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.y = 0; TexelIndex.y < TexelCount.y; ++TexelIndex.y)
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						SamplerA.template fetch(TexelIndex, LevelIndex),
						SamplerB.template fetch(TexelIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};

	template <typename val_type>
	struct compute_sampler_reduce_2d_array
	{
		typedef typename binary_func<val_type>::type func_type;
		typedef texture2d_array::size_type size_type;
		typedef texture2d_array::extent_type extent_type;

		static vec<4, val_type> call(texture2d_array const& A, texture2d_array const& B, binary_func<val_type> TexelFunc, binary_func<val_type> ReduceFunc)
		{
			GLI_ASSERT(are_compatible(A, B));

			sampler2d_array<val_type> const SamplerA(A, gli::WRAP_CLAMP_TO_EDGE), SamplerB(B, gli::WRAP_CLAMP_TO_EDGE);
			extent_type TexelIndex(0);
			vec<4, val_type> Result(TexelFunc(SamplerA.template fetch(TexelIndex, 0, 0), SamplerB.template fetch(TexelIndex, 0, 0)));

			for(size_type LayerIndex = 0, LayerCount = A.layers(); LayerIndex < LayerCount; ++LayerIndex)
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.y = 0; TexelIndex.y < TexelCount.y; ++TexelIndex.y)
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						SamplerA.template fetch(TexelIndex, LayerIndex, LevelIndex),
						SamplerB.template fetch(TexelIndex, LayerIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};

	template <typename val_type>
	struct compute_sampler_reduce_3d
	{
		typedef typename binary_func<val_type>::type func_type;
		typedef texture3d::size_type size_type;
		typedef texture3d::extent_type extent_type;

		static vec<4, val_type> call(texture3d const& A, texture3d const& B, binary_func<val_type> TexelFunc, binary_func<val_type> ReduceFunc)
		{
			GLI_ASSERT(are_compatible(A, B));

			sampler3d<val_type> const SamplerA(A, gli::WRAP_CLAMP_TO_EDGE), SamplerB(B, gli::WRAP_CLAMP_TO_EDGE);
			extent_type TexelIndex(0);
			vec<4, val_type> Result(TexelFunc(SamplerA.template fetch(TexelIndex, 0), SamplerB.template fetch(TexelIndex, 0)));

			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.z = 0; TexelIndex.z < TexelCount.z; ++TexelIndex.z)
				for(TexelIndex.y = 0; TexelIndex.y < TexelCount.y; ++TexelIndex.y)
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						SamplerA.template fetch(TexelIndex, LevelIndex),
						SamplerB.template fetch(TexelIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};

	template <typename val_type>
	struct compute_sampler_reduce_cube
	{
		typedef typename binary_func<val_type>::type func_type;
		typedef texture_cube::size_type size_type;
		typedef texture_cube::extent_type extent_type;

		static vec<4, val_type> call(texture_cube const& A, texture_cube const& B, binary_func<val_type> TexelFunc, binary_func<val_type> ReduceFunc)
		{
			GLI_ASSERT(are_compatible(A, B));

			sampler_cube<val_type> const SamplerA(A, gli::WRAP_CLAMP_TO_EDGE), SamplerB(B, gli::WRAP_CLAMP_TO_EDGE);
			extent_type TexelIndex(0);
			vec<4, val_type> Result(TexelFunc(SamplerA.template fetch(TexelIndex, 0, 0), SamplerB.template fetch(TexelIndex, 0, 0)));

			for(size_type FaceIndex = 0, FaceCount = A.faces(); FaceIndex < FaceCount; ++FaceIndex)
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.y = 0; TexelIndex.y < TexelCount.y; ++TexelIndex.y)
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						SamplerA.template fetch(TexelIndex, FaceIndex, LevelIndex),
						SamplerB.template fetch(TexelIndex, FaceIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};

	template <typename val_type>
	struct compute_sampler_reduce_cube_array
	{
		typedef typename binary_func<val_type>::type func_type;
		typedef texture_cube_array::size_type size_type;
		typedef texture_cube_array::extent_type extent_type;

		static vec<4, val_type> call(texture_cube_array const& A, texture_cube_array const& B, binary_func<val_type> TexelFunc, binary_func<val_type> ReduceFunc)
		{
			GLI_ASSERT(are_compatible(A, B));

			sampler_cube_array<val_type> const SamplerA(A, gli::WRAP_CLAMP_TO_EDGE), SamplerB(B, gli::WRAP_CLAMP_TO_EDGE);
			extent_type TexelIndex(0);
			vec<4, val_type> Result(TexelFunc(SamplerA.template fetch(TexelIndex, 0, 0, 0), SamplerB.template fetch(TexelIndex, 0, 0, 0)));

			for(size_type LayerIndex = 0, LayerCount = A.layers(); LayerIndex < LayerCount; ++LayerIndex)
			for(size_type FaceIndex = 0, FaceCount = A.faces(); FaceIndex < FaceCount; ++FaceIndex)
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.y = 0; TexelIndex.y < TexelCount.y; ++TexelIndex.y)
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						SamplerA.template fetch(TexelIndex, LayerIndex, FaceIndex, LevelIndex),
						SamplerB.template fetch(TexelIndex, LayerIndex, FaceIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};
}//namespace detail

namespace detail
{
	template <typename vec_type>
	struct compute_reduce_1d
	{
		typedef typename reduce_func<vec_type>::type func_type;
		typedef texture1d::size_type size_type;
		typedef texture1d::extent_type extent_type;
		
		static vec_type call(texture1d const& A, texture1d const& B, func_type TexelFunc, func_type ReduceFunc)
		{
			GLI_ASSERT(all(equal(A.extent(), B.extent())));
			GLI_ASSERT(A.levels() == B.levels());
			GLI_ASSERT(A.size() == B.size());

			extent_type TexelIndex(0);
			vec_type Result(TexelFunc(
				A.template load<vec_type>(TexelIndex, 0),
				B.template load<vec_type>(TexelIndex, 0)));
			
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						A.template load<vec_type>(TexelIndex, LevelIndex),
						B.template load<vec_type>(TexelIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};
	
	template <typename vec_type>
	struct compute_reduce_1d_array
	{
		typedef typename reduce_func<vec_type>::type func_type;
		typedef texture1d_array::size_type size_type;
		typedef texture1d_array::extent_type extent_type;
		
		static vec_type call(texture1d_array const& A, texture1d_array const& B, func_type TexelFunc, func_type ReduceFunc)
		{
			GLI_ASSERT(all(equal(A.extent(), B.extent())));
			GLI_ASSERT(A.levels() == B.levels());
			GLI_ASSERT(A.size() == B.size());
			
			extent_type TexelIndex(0);
			vec_type Result(TexelFunc(
				A.template load<vec_type>(TexelIndex, 0),
				B.template load<vec_type>(TexelIndex, 0)));
			
			for(size_type LayerIndex = 0, LayerCount = A.layers(); LayerIndex < LayerCount; ++LayerIndex)
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						A.template load<vec_type>(TexelIndex, LayerIndex, LevelIndex),
						B.template load<vec_type>(TexelIndex, LayerIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};
	
	template <typename vec_type>
	struct compute_reduce_2d
	{
		typedef typename reduce_func<vec_type>::type func_type;
		typedef texture2d::size_type size_type;
		typedef texture2d::extent_type extent_type;
		
		static vec_type call(texture2d const& A, texture2d const& B, func_type TexelFunc, func_type ReduceFunc)
		{
			GLI_ASSERT(all(equal(A.extent(), B.extent())));
			GLI_ASSERT(A.levels() == B.levels());
			GLI_ASSERT(A.size() == B.size());
			
			extent_type TexelIndex(0);
			vec_type Result(TexelFunc(
				A.template load<vec_type>(TexelIndex, 0),
				B.template load<vec_type>(TexelIndex, 0)));
			
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.y = 0; TexelIndex.y < TexelCount.y; ++TexelIndex.y)
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						A.template load<vec_type>(TexelIndex, LevelIndex),
						B.template load<vec_type>(TexelIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};
	
	template <typename vec_type>
	struct compute_reduce_2d_array
	{
		typedef typename reduce_func<vec_type>::type func_type;
		typedef texture2d_array::size_type size_type;
		typedef texture2d_array::extent_type extent_type;
		
		static vec_type call(texture2d_array const& A, texture2d_array const& B, func_type TexelFunc, func_type ReduceFunc)
		{
			GLI_ASSERT(all(equal(A.extent(), B.extent())));
			GLI_ASSERT(A.levels() == B.levels());
			GLI_ASSERT(A.size() == B.size());
			
			extent_type TexelIndex(0);
			vec_type Result(TexelFunc(
				A.template load<vec_type>(TexelIndex, 0, 0),
				B.template load<vec_type>(TexelIndex, 0, 0)));
			
			for(size_type LayerIndex = 0, LayerCount = A.layers(); LayerIndex < LayerCount; ++LayerIndex)
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.y = 0; TexelIndex.y < TexelCount.y; ++TexelIndex.y)
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						A.template load<vec_type>(TexelIndex, LayerIndex, LevelIndex),
						B.template load<vec_type>(TexelIndex, LayerIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};
	
	template <typename vec_type>
	struct compute_reduce_3d
	{
		typedef typename reduce_func<vec_type>::type func_type;
		typedef texture3d::size_type size_type;
		typedef texture3d::extent_type extent_type;
		
		static vec_type call(texture3d const& A, texture3d const& B, func_type TexelFunc, func_type ReduceFunc)
		{
			GLI_ASSERT(all(equal(A.extent(), B.extent())));
			GLI_ASSERT(A.levels() == B.levels());
			GLI_ASSERT(A.size() == B.size());
			
			extent_type TexelIndex(0);
			vec_type Result(TexelFunc(
				A.template load<vec_type>(TexelIndex, 0),
				B.template load<vec_type>(TexelIndex, 0)));
			
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.z = 0; TexelIndex.z < TexelCount.z; ++TexelIndex.z)
				for(TexelIndex.y = 0; TexelIndex.y < TexelCount.y; ++TexelIndex.y)
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						A.template load<vec_type>(TexelIndex, LevelIndex),
						B.template load<vec_type>(TexelIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};
	
	template <typename vec_type>
	struct compute_reduce_cube
	{
		typedef typename reduce_func<vec_type>::type func_type;
		typedef texture_cube::size_type size_type;
		typedef texture_cube::extent_type extent_type;
		
		static vec_type call(texture_cube const& A, texture_cube const& B, func_type TexelFunc, func_type ReduceFunc)
		{
			GLI_ASSERT(all(equal(A.extent(), B.extent())));
			GLI_ASSERT(A.levels() == B.levels());
			GLI_ASSERT(A.size() == B.size());
			
			extent_type TexelIndex(0);
			vec_type Result(TexelFunc(
				A.load<vec_type>(TexelIndex, 0, 0),
				B.load<vec_type>(TexelIndex, 0, 0)));
			
			for(size_type FaceIndex = 0, FaceCount = A.faces(); FaceIndex < FaceCount; ++FaceIndex)
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.y = 0; TexelIndex.y < TexelCount.y; ++TexelIndex.y)
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						A.template load<vec_type>(TexelIndex, FaceIndex, LevelIndex),
						B.template load<vec_type>(TexelIndex, FaceIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};
	
	template <typename vec_type>
	struct compute_reduce_cube_array
	{
		typedef typename reduce_func<vec_type>::type func_type;
		typedef texture_cube_array::size_type size_type;
		typedef texture_cube_array::extent_type extent_type;
		
		static vec_type call(texture_cube_array const& A, texture_cube_array const& B, func_type TexelFunc, func_type ReduceFunc)
		{
			GLI_ASSERT(all(equal(A.extent(), B.extent())));
			GLI_ASSERT(A.levels() == B.levels());
			GLI_ASSERT(A.size() == B.size());
			
			extent_type TexelIndex(0);
			vec_type Result(TexelFunc(
				A.load<vec_type>(TexelIndex, 0, 0, 0),
				B.load<vec_type>(TexelIndex, 0, 0 ,0)));
			
			for(size_type LayerIndex = 0, LayerCount = A.layers(); LayerIndex < LayerCount; ++LayerIndex)
			for(size_type FaceIndex = 0, FaceCount = A.faces(); FaceIndex < FaceCount; ++FaceIndex)
			for(size_type LevelIndex = 0, LevelCount = A.levels(); LevelIndex < LevelCount; ++LevelIndex)
			{
				extent_type const TexelCount(A.extent(LevelIndex));
				for(TexelIndex.y = 0; TexelIndex.y < TexelCount.y; ++TexelIndex.y)
				for(TexelIndex.x = 0; TexelIndex.x < TexelCount.x; ++TexelIndex.x)
				{
					Result = ReduceFunc(Result, TexelFunc(
						A.template load<vec_type>(TexelIndex, LayerIndex, FaceIndex, LevelIndex),
						B.template load<vec_type>(TexelIndex, LayerIndex, FaceIndex, LevelIndex)));
				}
			}
			
			return Result;
		}
	};
}//namepsace detail

template <typename vec_type>
inline vec_type reduce(texture1d const& In0, texture1d const& In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc)
{
	return detail::compute_reduce_1d<vec_type>::call(In0, In1, TexelFunc, ReduceFunc);
}

template <typename vec_type>
inline vec_type reduce(texture1d_array const& In0, texture1d_array const& In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc)
{
	return detail::compute_reduce_1d_array<vec_type>::call(In0, In1, TexelFunc, ReduceFunc);
}

template <typename vec_type>
inline vec_type reduce(texture2d const& In0, texture2d const& In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc)
{
	return detail::compute_reduce_2d<vec_type>::call(In0, In1, TexelFunc, ReduceFunc);
}

template <typename vec_type>
inline vec_type reduce(texture2d_array const& In0, texture2d_array const& In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc)
{
	return detail::compute_reduce_2d_array<vec_type>::call(In0, In1, TexelFunc, ReduceFunc);
}

template <typename vec_type>
inline vec_type reduce(texture3d const& In0, texture3d const& In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc)
{
	return detail::compute_reduce_3d<vec_type>::call(In0, In1, TexelFunc, ReduceFunc);
}

template <typename vec_type>
inline vec_type reduce(texture_cube const& In0, texture_cube const& In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc)
{
	return detail::compute_reduce_cube<vec_type>::call(In0, In1, TexelFunc, ReduceFunc);
}

template <typename vec_type>
inline vec_type reduce(texture_cube_array const& In0, texture_cube_array const& In1, typename reduce_func<vec_type>::type TexelFunc, typename reduce_func<vec_type>::type ReduceFunc)
{
	return detail::compute_reduce_cube_array<vec_type>::call(In0, In1, TexelFunc, ReduceFunc);
}
}//namespace gli

