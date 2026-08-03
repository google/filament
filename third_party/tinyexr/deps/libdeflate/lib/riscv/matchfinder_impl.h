/*
 * riscv/matchfinder_impl.h - RISC-V implementations of matchfinder functions
 *
 * Copyright 2024 Eric Biggers
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LIB_RISCV_MATCHFINDER_IMPL_H
#define LIB_RISCV_MATCHFINDER_IMPL_H

#if defined(ARCH_RISCV) && defined(__riscv_vector)
#include <riscv_vector.h>

/*
 * Return the maximum number of 16-bit (mf_pos_t) elements that fit in 8 RISC-V
 * vector registers and also evenly divide the sizes of the matchfinder buffers.
 */
static forceinline size_t
riscv_matchfinder_vl(void)
{
	const size_t vl = __riscv_vsetvlmax_e16m8();

	STATIC_ASSERT(sizeof(mf_pos_t) == sizeof(s16));
	/*
	 * MATCHFINDER_SIZE_ALIGNMENT is a power of 2, as is 'vl' because the
	 * RISC-V Vector Extension requires that the vector register length
	 * (VLEN) be a power of 2.  Thus, a simple MIN() gives the correct
	 * answer here; rounding to a power of 2 is not required.
	 */
	STATIC_ASSERT((MATCHFINDER_SIZE_ALIGNMENT &
		       (MATCHFINDER_SIZE_ALIGNMENT - 1)) == 0);
	ASSERT((vl & (vl - 1)) == 0);
	return MIN(vl, MATCHFINDER_SIZE_ALIGNMENT / sizeof(mf_pos_t));
}

/* matchfinder_init() optimized using the RISC-V Vector Extension */
static forceinline void
matchfinder_init_rvv(mf_pos_t *p, size_t size)
{
	const size_t vl = riscv_matchfinder_vl();
	const vint16m8_t v = __riscv_vmv_v_x_i16m8(MATCHFINDER_INITVAL, vl);

	ASSERT(size > 0 && size % (vl * sizeof(p[0])) == 0);
	do {
		__riscv_vse16_v_i16m8(p, v, vl);
		p += vl;
		size -= vl * sizeof(p[0]);
	} while (size != 0);
}
#define matchfinder_init matchfinder_init_rvv

/* matchfinder_rebase() optimized using the RISC-V Vector Extension */
static forceinline void
matchfinder_rebase_rvv(mf_pos_t *p, size_t size)
{
	const size_t vl = riscv_matchfinder_vl();

	ASSERT(size > 0 && size % (vl * sizeof(p[0])) == 0);
	do {
		vint16m8_t v = __riscv_vle16_v_i16m8(p, vl);

		/*
		 * This should generate the vsadd.vx instruction
		 * (Vector Saturating Add, integer vector-scalar)
		 */
		v = __riscv_vsadd_vx_i16m8(v, (s16)-MATCHFINDER_WINDOW_SIZE,
					   vl);
		__riscv_vse16_v_i16m8(p, v, vl);
		p += vl;
		size -= vl * sizeof(p[0]);
	} while (size != 0);
}
#define matchfinder_rebase matchfinder_rebase_rvv

#endif /* ARCH_RISCV && __riscv_vector */

#endif /* LIB_RISCV_MATCHFINDER_IMPL_H */
