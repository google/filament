// RUN: %clang -target i386-unknown-unknown -march=core2 -msse4 -x c -E -dM -o - %s | FileCheck --check-prefix=SSE4 %s

// SSE4: #define __SSE2_MATH__ 1
// SSE4: #define __SSE2__ 1
// SSE4: #define __SSE3__ 1
// SSE4: #define __SSE4_1__ 1
// SSE4: #define __SSE4_2__ 1
// SSE4: #define __SSE_MATH__ 1
// SSE4: #define __SSE__ 1
// SSE4: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=core2 -msse4.1 -mno-sse4 -x c -E -dM -o - %s | FileCheck --check-prefix=NOSSE4 %s

// NOSSE4-NOT: #define __SSE4_1__ 1

// RUN: %clang -target i386-unknown-unknown -march=core2 -msse4 -mno-sse2 -x c -E -dM -o - %s | FileCheck --check-prefix=SSE %s

// SSE-NOT: #define __SSE2_MATH__ 1
// SSE-NOT: #define __SSE2__ 1
// SSE-NOT: #define __SSE3__ 1
// SSE-NOT: #define __SSE4_1__ 1
// SSE-NOT: #define __SSE4_2__ 1
// SSE: #define __SSE_MATH__ 1
// SSE: #define __SSE__ 1
// SSE-NOT: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentium-m -x c -E -dM -o - %s | FileCheck --check-prefix=SSE2 %s

// SSE2: #define __SSE2_MATH__ 1
// SSE2: #define __SSE2__ 1
// SSE2-NOT: #define __SSE3__ 1
// SSE2-NOT: #define __SSE4_1__ 1
// SSE2-NOT: #define __SSE4_2__ 1
// SSE2: #define __SSE_MATH__ 1
// SSE2: #define __SSE__ 1
// SSE2-NOT: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentium-m -mno-sse -mavx -x c -E -dM -o - %s | FileCheck --check-prefix=AVX %s

// AVX: #define __AVX__ 1
// AVX: #define __SSE2_MATH__ 1
// AVX: #define __SSE2__ 1
// AVX: #define __SSE3__ 1
// AVX: #define __SSE4_1__ 1
// AVX: #define __SSE4_2__ 1
// AVX: #define __SSE_MATH__ 1
// AVX: #define __SSE__ 1
// AVX: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentium-m -mxop -mno-avx -x c -E -dM -o - %s | FileCheck --check-prefix=SSE4A %s

// SSE4A: #define __SSE2_MATH__ 1
// SSE4A: #define __SSE2__ 1
// SSE4A: #define __SSE3__ 1
// SSE4A: #define __SSE4A__ 1
// SSE4A: #define __SSE4_1__ 1
// SSE4A: #define __SSE4_2__ 1
// SSE4A: #define __SSE_MATH__ 1
// SSE4A: #define __SSE__ 1
// SSE4A: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512f -x c -E -dM -o - %s | FileCheck --check-prefix=AVX512F %s

// AVX512F: #define __AVX2__ 1
// AVX512F: #define __AVX512F__ 1
// AVX512F: #define __AVX__ 1
// AVX512F: #define __SSE2_MATH__ 1
// AVX512F: #define __SSE2__ 1
// AVX512F: #define __SSE3__ 1
// AVX512F: #define __SSE4_1__ 1
// AVX512F: #define __SSE4_2__ 1
// AVX512F: #define __SSE_MATH__ 1
// AVX512F: #define __SSE__ 1
// AVX512F: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512cd -x c -E -dM -o - %s | FileCheck --check-prefix=AVX512CD %s

// AVX512CD: #define __AVX2__ 1
// AVX512CD: #define __AVX512CD__ 1
// AVX512CD: #define __AVX512F__ 1
// AVX512CD: #define __AVX__ 1
// AVX512CD: #define __SSE2_MATH__ 1
// AVX512CD: #define __SSE2__ 1
// AVX512CD: #define __SSE3__ 1
// AVX512CD: #define __SSE4_1__ 1
// AVX512CD: #define __SSE4_2__ 1
// AVX512CD: #define __SSE_MATH__ 1
// AVX512CD: #define __SSE__ 1
// AVX512CD: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512er -x c -E -dM -o - %s | FileCheck --check-prefix=AVX512ER %s

// AVX512ER: #define __AVX2__ 1
// AVX512ER: #define __AVX512ER__ 1
// AVX512ER: #define __AVX512F__ 1
// AVX512ER: #define __AVX__ 1
// AVX512ER: #define __SSE2_MATH__ 1
// AVX512ER: #define __SSE2__ 1
// AVX512ER: #define __SSE3__ 1
// AVX512ER: #define __SSE4_1__ 1
// AVX512ER: #define __SSE4_2__ 1
// AVX512ER: #define __SSE_MATH__ 1
// AVX512ER: #define __SSE__ 1
// AVX512ER: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512pf -x c -E -dM -o - %s | FileCheck --check-prefix=AVX512PF %s

// AVX512PF: #define __AVX2__ 1
// AVX512PF: #define __AVX512F__ 1
// AVX512PF: #define __AVX512PF__ 1
// AVX512PF: #define __AVX__ 1
// AVX512PF: #define __SSE2_MATH__ 1
// AVX512PF: #define __SSE2__ 1
// AVX512PF: #define __SSE3__ 1
// AVX512PF: #define __SSE4_1__ 1
// AVX512PF: #define __SSE4_2__ 1
// AVX512PF: #define __SSE_MATH__ 1
// AVX512PF: #define __SSE__ 1
// AVX512PF: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512dq -x c -E -dM -o - %s | FileCheck --check-prefix=AVX512DQ %s

// AVX512DQ: #define __AVX2__ 1
// AVX512DQ: #define __AVX512DQ__ 1
// AVX512DQ: #define __AVX512F__ 1
// AVX512DQ: #define __AVX__ 1
// AVX512DQ: #define __SSE2_MATH__ 1
// AVX512DQ: #define __SSE2__ 1
// AVX512DQ: #define __SSE3__ 1
// AVX512DQ: #define __SSE4_1__ 1
// AVX512DQ: #define __SSE4_2__ 1
// AVX512DQ: #define __SSE_MATH__ 1
// AVX512DQ: #define __SSE__ 1
// AVX512DQ: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512bw -x c -E -dM -o - %s | FileCheck --check-prefix=AVX512BW %s

// AVX512BW: #define __AVX2__ 1
// AVX512BW: #define __AVX512BW__ 1
// AVX512BW: #define __AVX512F__ 1
// AVX512BW: #define __AVX__ 1
// AVX512BW: #define __SSE2_MATH__ 1
// AVX512BW: #define __SSE2__ 1
// AVX512BW: #define __SSE3__ 1
// AVX512BW: #define __SSE4_1__ 1
// AVX512BW: #define __SSE4_2__ 1
// AVX512BW: #define __SSE_MATH__ 1
// AVX512BW: #define __SSE__ 1
// AVX512BW: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512vl -x c -E -dM -o - %s | FileCheck --check-prefix=AVX512VL %s

// AVX512VL: #define __AVX2__ 1
// AVX512VL: #define __AVX512F__ 1
// AVX512VL: #define __AVX512VL__ 1
// AVX512VL: #define __AVX__ 1
// AVX512VL: #define __SSE2_MATH__ 1
// AVX512VL: #define __SSE2__ 1
// AVX512VL: #define __SSE3__ 1
// AVX512VL: #define __SSE4_1__ 1
// AVX512VL: #define __SSE4_2__ 1
// AVX512VL: #define __SSE_MATH__ 1
// AVX512VL: #define __SSE__ 1
// AVX512VL: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mavx512pf -mno-avx512f -x c -E -dM -o - %s | FileCheck --check-prefix=AVX512F2 %s

// AVX512F2: #define __AVX2__ 1
// AVX512F2-NOT: #define __AVX512F__ 1
// AVX512F2-NOT: #define __AVX512PF__ 1
// AVX512F2: #define __AVX__ 1
// AVX512F2: #define __SSE2_MATH__ 1
// AVX512F2: #define __SSE2__ 1
// AVX512F2: #define __SSE3__ 1
// AVX512F2: #define __SSE4_1__ 1
// AVX512F2: #define __SSE4_2__ 1
// AVX512F2: #define __SSE_MATH__ 1
// AVX512F2: #define __SSE__ 1
// AVX512F2: #define __SSSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -msse4.2 -x c -E -dM -o - %s | FileCheck --check-prefix=SSE42POPCNT %s

// SSE42POPCNT: #define __POPCNT__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-popcnt -msse4.2 -x c -E -dM -o - %s | FileCheck --check-prefix=SSE42NOPOPCNT %s

// SSE42NOPOPCNT-NOT: #define __POPCNT__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mpopcnt -mno-sse4.2 -x c -E -dM -o - %s | FileCheck --check-prefix=NOSSE42POPCNT %s

// NOSSE42POPCNT: #define __POPCNT__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -msse -x c -E -dM -o - %s | FileCheck --check-prefix=SSEMMX %s

// SSEMMX: #define __MMX__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -msse -mno-sse -x c -E -dM -o - %s | FileCheck --check-prefix=SSENOSSEMMX %s

// SSENOSSEMMX-NOT: #define __MMX__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -msse -mno-mmx -x c -E -dM -o - %s | FileCheck --check-prefix=SSENOMMX %s

// SSENOMMX-NOT: #define __MMX__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mf16c -x c -E -dM -o - %s | FileCheck --check-prefix=F16C %s

// F16C: #define __AVX__ 1
// F16C: #define __F16C__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mf16c -mno-avx -x c -E -dM -o - %s | FileCheck --check-prefix=F16CNOAVX %s

// F16CNOAVX-NOT: #define __AVX__ 1
// F16CNOAVX-NOT: #define __F16C__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentiumpro -mpclmul -x c -E -dM -o - %s | FileCheck --check-prefix=PCLMUL %s

// PCLMUL: #define __PCLMUL__ 1
// PCLMUL: #define __SSE2__ 1
// PCLMUL-NOT: #define __SSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentiumpro -mpclmul -mno-sse2 -x c -E -dM -o - %s | FileCheck --check-prefix=PCLMULNOSSE2 %s

// PCLMULNOSSE2-NOT: #define __PCLMUL__ 1
// PCLMULNOSSE2-NOT: #define __SSE2__ 1
// PCLMULNOSSE2-NOT: #define __SSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentiumpro -maes -x c -E -dM -o - %s | FileCheck --check-prefix=AES %s

// AES: #define __AES__ 1
// AES: #define __SSE2__ 1
// AES-NOT: #define __SSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentiumpro -maes -mno-sse2 -x c -E -dM -o - %s | FileCheck --check-prefix=AESNOSSE2 %s

// AESNOSSE2-NOT: #define __AES__ 1
// AESNOSSE2-NOT: #define __SSE2__ 1
// AESNOSSE2-NOT: #define __SSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentiumpro -msha -x c -E -dM -o - %s | FileCheck --check-prefix=SHA %s

// SHA: #define __SHA__ 1
// SHA: #define __SSE2__ 1
// SHA-NOT: #define __SSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentiumpro -msha -mno-sha -x c -E -dM -o - %s | FileCheck --check-prefix=SHANOSHA %s

// SHANOSHA-NOT: #define __SHA__ 1
// SHANOSHA-NOT: #define __SSE2__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentiumpro -msha -mno-sse2 -x c -E -dM -o - %s | FileCheck --check-prefix=SHANOSSE2 %s

// SHANOSSE2-NOT: #define __SHA__ 1
// SHANOSSE2-NOT: #define __SSE2__ 1
// SHANOSSE2-NOT: #define __SSE3__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mtbm -x c -E -dM -o - %s | FileCheck --check-prefix=TBM %s

// TBM: #define __TBM__ 1

// RUN: %clang -target i386-unknown-unknown -march=bdver2 -mno-tbm -x c -E -dM -o - %s | FileCheck --check-prefix=NOTBM %s

// NOTBM-NOT: #define __TBM__ 1

// RUN: %clang -target i386-unknown-unknown -march=pentiumpro -mcx16 -x c -E -dM -o - %s | FileCheck --check-prefix=MCX16 %s

// MCX16: #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mprfchw -x c -E -dM -o - %s | FileCheck --check-prefix=PRFCHW %s

// PRFCHW: #define __PRFCHW__ 1

// RUN: %clang -target i386-unknown-unknown -march=btver2 -mno-prfchw -x c -E -dM -o - %s | FileCheck --check-prefix=NOPRFCHW %s

// NOPRFCHW-NOT: #define __PRFCHW__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -m3dnow -x c -E -dM -o - %s | FileCheck --check-prefix=3DNOWPRFCHW %s

// 3DNOWPRFCHW: #define __PRFCHW__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mno-prfchw -m3dnow -x c -E -dM -o - %s | FileCheck --check-prefix=3DNOWNOPRFCHW %s

// 3DNOWNOPRFCHW-NOT: #define __PRFCHW__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mprfchw -mno-3dnow -x c -E -dM -o - %s | FileCheck --check-prefix=NO3DNOWPRFCHW %s

// NO3DNOWPRFCHW: #define __PRFCHW__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -madx -x c -E -dM -o - %s | FileCheck --check-prefix=ADX %s

// ADX: #define __ADX__ 1

// RUN: %clang -target i386-unknown-unknown -march=atom -mrdseed -x c -E -dM -o - %s | FileCheck --check-prefix=RDSEED %s

// RDSEED: #define __RDSEED__ 1

