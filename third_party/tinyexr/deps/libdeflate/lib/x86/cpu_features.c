/*
 * x86/cpu_features.c - feature detection for x86 CPUs
 *
 * Copyright 2016 Eric Biggers
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

#include "../cpu_features_common.h" /* must be included first */
#include "cpu_features.h"

#ifdef X86_CPU_FEATURES_KNOWN
/* Runtime x86 CPU feature detection is supported. */

/* Execute the CPUID instruction. */
static inline void
cpuid(u32 leaf, u32 subleaf, u32 *a, u32 *b, u32 *c, u32 *d)
{
#ifdef _MSC_VER
	int result[4];

	__cpuidex(result, leaf, subleaf);
	*a = result[0];
	*b = result[1];
	*c = result[2];
	*d = result[3];
#else
	__asm__ volatile("cpuid" : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
			 : "a" (leaf), "c" (subleaf));
#endif
}

/* Read an extended control register. */
static inline u64
read_xcr(u32 index)
{
#ifdef _MSC_VER
	return _xgetbv(index);
#else
	u32 d, a;

	/*
	 * Execute the "xgetbv" instruction.  Old versions of binutils do not
	 * recognize this instruction, so list the raw bytes instead.
	 *
	 * This must be 'volatile' to prevent this code from being moved out
	 * from under the check for OSXSAVE.
	 */
	__asm__ volatile(".byte 0x0f, 0x01, 0xd0" :
			 "=d" (d), "=a" (a) : "c" (index));

	return ((u64)d << 32) | a;
#endif
}

static const struct cpu_feature x86_cpu_feature_table[] = {
	{X86_CPU_FEATURE_SSE2,		"sse2"},
	{X86_CPU_FEATURE_PCLMULQDQ,	"pclmulqdq"},
	{X86_CPU_FEATURE_AVX,		"avx"},
	{X86_CPU_FEATURE_AVX2,		"avx2"},
	{X86_CPU_FEATURE_BMI2,		"bmi2"},
	{X86_CPU_FEATURE_ZMM,		"zmm"},
	{X86_CPU_FEATURE_AVX512BW,	"avx512bw"},
	{X86_CPU_FEATURE_AVX512VL,	"avx512vl"},
	{X86_CPU_FEATURE_VPCLMULQDQ,	"vpclmulqdq"},
	{X86_CPU_FEATURE_AVX512VNNI,	"avx512_vnni"},
	{X86_CPU_FEATURE_AVXVNNI,	"avx_vnni"},
};

volatile u32 libdeflate_x86_cpu_features = 0;

static inline bool
os_supports_avx512(u64 xcr0)
{
#ifdef __APPLE__
	/*
	 * The Darwin kernel had a bug where it could corrupt the opmask
	 * registers.  See
	 * https://community.intel.com/t5/Software-Tuning-Performance/MacOS-Darwin-kernel-bug-clobbers-AVX-512-opmask-register-state/m-p/1327259
	 * Darwin also does not initially set the XCR0 bits for AVX512, but they
	 * are set if the thread tries to use AVX512 anyway.  Thus, to safely
	 * and consistently use AVX512 on macOS we'd need to check the kernel
	 * version as well as detect AVX512 support using a macOS-specific
	 * method.  We don't bother with this, especially given Apple's
	 * transition to arm64.
	 */
	return false;
#else
	return (xcr0 & 0xe6) == 0xe6;
#endif
}

/*
 * Don't use 512-bit vectors (ZMM registers) on Intel CPUs before Rocket Lake
 * and Sapphire Rapids, due to the overly-eager downclocking which can reduce
 * the performance of workloads that use ZMM registers only occasionally.
 */
static inline bool
allow_512bit_vectors(const u32 manufacturer[3], u32 family, u32 model)
{
#ifdef TEST_SUPPORT__DO_NOT_USE
	return true;
#endif
	if (memcmp(manufacturer, "GenuineIntel", 12) != 0)
		return true;
	if (family != 6)
		return true;
	switch (model) {
	case 85: /* Skylake (Server), Cascade Lake, Cooper Lake */
	case 106: /* Ice Lake (Server) */
	case 108: /* Ice Lake (Server) */
	case 126: /* Ice Lake (Client) */
	case 140: /* Tiger Lake */
	case 141: /* Tiger Lake */
		return false;
	}
	return true;
}

/* Initialize libdeflate_x86_cpu_features. */
void libdeflate_init_x86_cpu_features(void)
{
	u32 max_leaf;
	u32 manufacturer[3];
	u32 family, model;
	u32 a, b, c, d;
	u64 xcr0 = 0;
	u32 features = 0;

	/* EAX=0: Highest Function Parameter and Manufacturer ID */
	cpuid(0, 0, &max_leaf, &manufacturer[0], &manufacturer[2],
	      &manufacturer[1]);
	if (max_leaf < 1)
		goto out;

	/* EAX=1: Processor Info and Feature Bits */
	cpuid(1, 0, &a, &b, &c, &d);
	family = (a >> 8) & 0xf;
	model = (a >> 4) & 0xf;
	if (family == 6 || family == 0xf)
		model += (a >> 12) & 0xf0;
	if (family == 0xf)
		family += (a >> 20) & 0xff;
	if (d & (1 << 26))
		features |= X86_CPU_FEATURE_SSE2;
	/*
	 * No known CPUs have pclmulqdq without sse4.1, so in practice code
	 * targeting pclmulqdq can use sse4.1 instructions.  But to be safe,
	 * explicitly check for both the pclmulqdq and sse4.1 bits.
	 */
	if ((c & (1 << 1)) && (c & (1 << 19)))
		features |= X86_CPU_FEATURE_PCLMULQDQ;
	if (c & (1 << 27))
		xcr0 = read_xcr(0);
	if ((c & (1 << 28)) && ((xcr0 & 0x6) == 0x6))
		features |= X86_CPU_FEATURE_AVX;

	if (max_leaf < 7)
		goto out;

	/* EAX=7, ECX=0: Extended Features */
	cpuid(7, 0, &a, &b, &c, &d);
	if (b & (1 << 8))
		features |= X86_CPU_FEATURE_BMI2;
	if ((xcr0 & 0x6) == 0x6) {
		if (b & (1 << 5))
			features |= X86_CPU_FEATURE_AVX2;
		if (c & (1 << 10))
			features |= X86_CPU_FEATURE_VPCLMULQDQ;
	}
	if (os_supports_avx512(xcr0)) {
		if (allow_512bit_vectors(manufacturer, family, model))
			features |= X86_CPU_FEATURE_ZMM;
		if (b & (1 << 30))
			features |= X86_CPU_FEATURE_AVX512BW;
		if (b & (1U << 31))
			features |= X86_CPU_FEATURE_AVX512VL;
		if (c & (1 << 11))
			features |= X86_CPU_FEATURE_AVX512VNNI;
	}

	/* EAX=7, ECX=1: Extended Features */
	cpuid(7, 1, &a, &b, &c, &d);
	if ((a & (1 << 4)) && ((xcr0 & 0x6) == 0x6))
		features |= X86_CPU_FEATURE_AVXVNNI;

out:
	disable_cpu_features_for_testing(&features, x86_cpu_feature_table,
					 ARRAY_LEN(x86_cpu_feature_table));

	libdeflate_x86_cpu_features = features | X86_CPU_FEATURES_KNOWN;
}

#endif /* X86_CPU_FEATURES_KNOWN */
