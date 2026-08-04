#ifndef LIB_X86_DECOMPRESS_IMPL_H
#define LIB_X86_DECOMPRESS_IMPL_H

#include "cpu_features.h"

/*
 * BMI2 optimized decompression function.
 *
 * With gcc and clang we just compile the whole function with
 * __attribute__((target("bmi2"))), and the compiler uses bmi2 automatically.
 *
 * With MSVC, there is no target function attribute, but it's still possible to
 * use bmi2 intrinsics explicitly.  Currently we mostly don't, but there's a
 * case in which we do (see below), so we at least take advantage of that.
 * However, MSVC from VS2017 (toolset v141) apparently miscompiles the _bzhi_*()
 * intrinsics.  It seems to be fixed in VS2022.  Hence, use MSVC_PREREQ(1930).
 */
#if defined(__GNUC__) || defined(__clang__) || MSVC_PREREQ(1930)
#  define deflate_decompress_bmi2	deflate_decompress_bmi2
#  define FUNCNAME			deflate_decompress_bmi2
#  define ATTRIBUTES			_target_attribute("bmi2")
   /*
    * Even with __attribute__((target("bmi2"))), gcc doesn't reliably use the
    * bzhi instruction for 'word & BITMASK(count)'.  So use the bzhi intrinsic
    * explicitly.  EXTRACT_VARBITS() is equivalent to 'word & BITMASK(count)';
    * EXTRACT_VARBITS8() is equivalent to 'word & BITMASK((u8)count)'.
    * Nevertheless, their implementation using the bzhi intrinsic is identical,
    * as the bzhi instruction truncates the count to 8 bits implicitly.
    */
#  ifndef __clang__
#    ifdef ARCH_X86_64
#      define EXTRACT_VARBITS(word, count)  _bzhi_u64((word), (count))
#      define EXTRACT_VARBITS8(word, count) _bzhi_u64((word), (count))
#    else
#      define EXTRACT_VARBITS(word, count)  _bzhi_u32((word), (count))
#      define EXTRACT_VARBITS8(word, count) _bzhi_u32((word), (count))
#    endif
#  endif
#  include "../decompress_template.h"
#endif

#if defined(deflate_decompress_bmi2) && HAVE_BMI2_NATIVE
#define DEFAULT_IMPL	deflate_decompress_bmi2
#else
static inline decompress_func_t
arch_select_decompress_func(void)
{
#ifdef deflate_decompress_bmi2
	if (HAVE_BMI2(get_x86_cpu_features()))
		return deflate_decompress_bmi2;
#endif
	return NULL;
}
#define arch_select_decompress_func	arch_select_decompress_func
#endif

#endif /* LIB_X86_DECOMPRESS_IMPL_H */
