// Begin X86/GCC/Linux tests ----------------
//
// RUN: %clang -march=i386 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_I386_M32
// CHECK_I386_M32: #define __i386 1
// CHECK_I386_M32: #define __i386__ 1
// CHECK_I386_M32: #define __tune_i386__ 1
// CHECK_I386_M32: #define i386 1
// RUN: not %clang -march=i386 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_I386_M64
// CHECK_I386_M64: error:
//
// RUN: %clang -march=i486 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_I486_M32
// CHECK_I486_M32: #define __i386 1
// CHECK_I486_M32: #define __i386__ 1
// CHECK_I486_M32: #define __i486 1
// CHECK_I486_M32: #define __i486__ 1
// CHECK_I486_M32: #define __tune_i486__ 1
// CHECK_I486_M32: #define i386 1
// RUN: not %clang -march=i486 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_I486_M64
// CHECK_I486_M64: error:
//
// RUN: %clang -march=i586 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_I586_M32
// CHECK_I586_M32: #define __i386 1
// CHECK_I586_M32: #define __i386__ 1
// CHECK_I586_M32: #define __i586 1
// CHECK_I586_M32: #define __i586__ 1
// CHECK_I586_M32: #define __pentium 1
// CHECK_I586_M32: #define __pentium__ 1
// CHECK_I586_M32: #define __tune_i586__ 1
// CHECK_I586_M32: #define __tune_pentium__ 1
// CHECK_I586_M32: #define i386 1
// RUN: not %clang -march=i586 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_I586_M64
// CHECK_I586_M64: error:
//
// RUN: %clang -march=pentium -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM_M32
// CHECK_PENTIUM_M32: #define __i386 1
// CHECK_PENTIUM_M32: #define __i386__ 1
// CHECK_PENTIUM_M32: #define __i586 1
// CHECK_PENTIUM_M32: #define __i586__ 1
// CHECK_PENTIUM_M32: #define __pentium 1
// CHECK_PENTIUM_M32: #define __pentium__ 1
// CHECK_PENTIUM_M32: #define __tune_i586__ 1
// CHECK_PENTIUM_M32: #define __tune_pentium__ 1
// CHECK_PENTIUM_M32: #define i386 1
// RUN: not %clang -march=pentium -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM_M64
// CHECK_PENTIUM_M64: error:
//
// RUN: %clang -march=pentium-mmx -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM_MMX_M32
// CHECK_PENTIUM_MMX_M32: #define __MMX__ 1
// CHECK_PENTIUM_MMX_M32: #define __i386 1
// CHECK_PENTIUM_MMX_M32: #define __i386__ 1
// CHECK_PENTIUM_MMX_M32: #define __i586 1
// CHECK_PENTIUM_MMX_M32: #define __i586__ 1
// CHECK_PENTIUM_MMX_M32: #define __pentium 1
// CHECK_PENTIUM_MMX_M32: #define __pentium__ 1
// CHECK_PENTIUM_MMX_M32: #define __pentium_mmx__ 1
// CHECK_PENTIUM_MMX_M32: #define __tune_i586__ 1
// CHECK_PENTIUM_MMX_M32: #define __tune_pentium__ 1
// CHECK_PENTIUM_MMX_M32: #define __tune_pentium_mmx__ 1
// CHECK_PENTIUM_MMX_M32: #define i386 1
// RUN: not %clang -march=pentium-mmx -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM_MMX_M64
// CHECK_PENTIUM_MMX_M64: error:
//
// RUN: %clang -march=winchip-c6 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_WINCHIP_C6_M32
// CHECK_WINCHIP_C6_M32: #define __MMX__ 1
// CHECK_WINCHIP_C6_M32: #define __i386 1
// CHECK_WINCHIP_C6_M32: #define __i386__ 1
// CHECK_WINCHIP_C6_M32: #define __i486 1
// CHECK_WINCHIP_C6_M32: #define __i486__ 1
// CHECK_WINCHIP_C6_M32: #define __tune_i486__ 1
// CHECK_WINCHIP_C6_M32: #define i386 1
// RUN: not %clang -march=winchip-c6 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_WINCHIP_C6_M64
// CHECK_WINCHIP_C6_M64: error:
//
// RUN: %clang -march=winchip2 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_WINCHIP2_M32
// CHECK_WINCHIP2_M32: #define __3dNOW__ 1
// CHECK_WINCHIP2_M32: #define __MMX__ 1
// CHECK_WINCHIP2_M32: #define __i386 1
// CHECK_WINCHIP2_M32: #define __i386__ 1
// CHECK_WINCHIP2_M32: #define __i486 1
// CHECK_WINCHIP2_M32: #define __i486__ 1
// CHECK_WINCHIP2_M32: #define __tune_i486__ 1
// CHECK_WINCHIP2_M32: #define i386 1
// RUN: not %clang -march=winchip2 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_WINCHIP2_M64
// CHECK_WINCHIP2_M64: error:
//
// RUN: %clang -march=c3 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_C3_M32
// CHECK_C3_M32: #define __3dNOW__ 1
// CHECK_C3_M32: #define __MMX__ 1
// CHECK_C3_M32: #define __i386 1
// CHECK_C3_M32: #define __i386__ 1
// CHECK_C3_M32: #define __i486 1
// CHECK_C3_M32: #define __i486__ 1
// CHECK_C3_M32: #define __tune_i486__ 1
// CHECK_C3_M32: #define i386 1
// RUN: not %clang -march=c3 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_C3_M64
// CHECK_C3_M64: error:
//
// RUN: %clang -march=c3-2 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_C3_2_M32
// CHECK_C3_2_M32: #define __MMX__ 1
// CHECK_C3_2_M32: #define __SSE__ 1
// CHECK_C3_2_M32: #define __i386 1
// CHECK_C3_2_M32: #define __i386__ 1
// CHECK_C3_2_M32: #define __i686 1
// CHECK_C3_2_M32: #define __i686__ 1
// CHECK_C3_2_M32: #define __pentiumpro 1
// CHECK_C3_2_M32: #define __pentiumpro__ 1
// CHECK_C3_2_M32: #define __tune_i686__ 1
// CHECK_C3_2_M32: #define __tune_pentium2__ 1
// CHECK_C3_2_M32: #define __tune_pentiumpro__ 1
// CHECK_C3_2_M32: #define i386 1
// RUN: not %clang -march=c3-2 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_C3_2_M64
// CHECK_C3_2_M64: error:
//
// RUN: %clang -march=i686 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_I686_M32
// CHECK_I686_M32: #define __i386 1
// CHECK_I686_M32: #define __i386__ 1
// CHECK_I686_M32: #define __i686 1
// CHECK_I686_M32: #define __i686__ 1
// CHECK_I686_M32: #define __pentiumpro 1
// CHECK_I686_M32: #define __pentiumpro__ 1
// CHECK_I686_M32: #define i386 1
// RUN: not %clang -march=i686 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_I686_M64
// CHECK_I686_M64: error:
//
// RUN: %clang -march=pentiumpro -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUMPRO_M32
// CHECK_PENTIUMPRO_M32: #define __i386 1
// CHECK_PENTIUMPRO_M32: #define __i386__ 1
// CHECK_PENTIUMPRO_M32: #define __i686 1
// CHECK_PENTIUMPRO_M32: #define __i686__ 1
// CHECK_PENTIUMPRO_M32: #define __pentiumpro 1
// CHECK_PENTIUMPRO_M32: #define __pentiumpro__ 1
// CHECK_PENTIUMPRO_M32: #define __tune_i686__ 1
// CHECK_PENTIUMPRO_M32: #define __tune_pentiumpro__ 1
// CHECK_PENTIUMPRO_M32: #define i386 1
// RUN: not %clang -march=pentiumpro -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUMPRO_M64
// CHECK_PENTIUMPRO_M64: error:
//
// RUN: %clang -march=pentium2 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM2_M32
// CHECK_PENTIUM2_M32: #define __MMX__ 1
// CHECK_PENTIUM2_M32: #define __i386 1
// CHECK_PENTIUM2_M32: #define __i386__ 1
// CHECK_PENTIUM2_M32: #define __i686 1
// CHECK_PENTIUM2_M32: #define __i686__ 1
// CHECK_PENTIUM2_M32: #define __pentiumpro 1
// CHECK_PENTIUM2_M32: #define __pentiumpro__ 1
// CHECK_PENTIUM2_M32: #define __tune_i686__ 1
// CHECK_PENTIUM2_M32: #define __tune_pentium2__ 1
// CHECK_PENTIUM2_M32: #define __tune_pentiumpro__ 1
// CHECK_PENTIUM2_M32: #define i386 1
// RUN: not %clang -march=pentium2 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM2_M64
// CHECK_PENTIUM2_M64: error:
//
// RUN: %clang -march=pentium3 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM3_M32
// CHECK_PENTIUM3_M32: #define __MMX__ 1
// CHECK_PENTIUM3_M32: #define __SSE__ 1
// CHECK_PENTIUM3_M32: #define __i386 1
// CHECK_PENTIUM3_M32: #define __i386__ 1
// CHECK_PENTIUM3_M32: #define __i686 1
// CHECK_PENTIUM3_M32: #define __i686__ 1
// CHECK_PENTIUM3_M32: #define __pentiumpro 1
// CHECK_PENTIUM3_M32: #define __pentiumpro__ 1
// CHECK_PENTIUM3_M32: #define __tune_i686__ 1
// CHECK_PENTIUM3_M32: #define __tune_pentium2__ 1
// CHECK_PENTIUM3_M32: #define __tune_pentium3__ 1
// CHECK_PENTIUM3_M32: #define __tune_pentiumpro__ 1
// CHECK_PENTIUM3_M32: #define i386 1
// RUN: not %clang -march=pentium3 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM3_M64
// CHECK_PENTIUM3_M64: error:
//
// RUN: %clang -march=pentium3m -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM3M_M32
// CHECK_PENTIUM3M_M32: #define __MMX__ 1
// CHECK_PENTIUM3M_M32: #define __SSE__ 1
// CHECK_PENTIUM3M_M32: #define __i386 1
// CHECK_PENTIUM3M_M32: #define __i386__ 1
// CHECK_PENTIUM3M_M32: #define __i686 1
// CHECK_PENTIUM3M_M32: #define __i686__ 1
// CHECK_PENTIUM3M_M32: #define __pentiumpro 1
// CHECK_PENTIUM3M_M32: #define __pentiumpro__ 1
// CHECK_PENTIUM3M_M32: #define __tune_i686__ 1
// CHECK_PENTIUM3M_M32: #define __tune_pentiumpro__ 1
// CHECK_PENTIUM3M_M32: #define i386 1
// RUN: not %clang -march=pentium3m -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM3M_M64
// CHECK_PENTIUM3M_M64: error:
//
// RUN: %clang -march=pentium-m -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM_M_M32
// CHECK_PENTIUM_M_M32: #define __MMX__ 1
// CHECK_PENTIUM_M_M32: #define __SSE2__ 1
// CHECK_PENTIUM_M_M32: #define __SSE__ 1
// CHECK_PENTIUM_M_M32: #define __i386 1
// CHECK_PENTIUM_M_M32: #define __i386__ 1
// CHECK_PENTIUM_M_M32: #define __i686 1
// CHECK_PENTIUM_M_M32: #define __i686__ 1
// CHECK_PENTIUM_M_M32: #define __pentiumpro 1
// CHECK_PENTIUM_M_M32: #define __pentiumpro__ 1
// CHECK_PENTIUM_M_M32: #define __tune_i686__ 1
// CHECK_PENTIUM_M_M32: #define __tune_pentiumpro__ 1
// CHECK_PENTIUM_M_M32: #define i386 1
// RUN: not %clang -march=pentium-m -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM_M_M64
// CHECK_PENTIUM_M_M64: error:
//
// RUN: %clang -march=pentium4 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM4_M32
// CHECK_PENTIUM4_M32: #define __MMX__ 1
// CHECK_PENTIUM4_M32: #define __SSE2__ 1
// CHECK_PENTIUM4_M32: #define __SSE__ 1
// CHECK_PENTIUM4_M32: #define __i386 1
// CHECK_PENTIUM4_M32: #define __i386__ 1
// CHECK_PENTIUM4_M32: #define __pentium4 1
// CHECK_PENTIUM4_M32: #define __pentium4__ 1
// CHECK_PENTIUM4_M32: #define __tune_pentium4__ 1
// CHECK_PENTIUM4_M32: #define i386 1
// RUN: not %clang -march=pentium4 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM4_M64
// CHECK_PENTIUM4_M64: error:
//
// RUN: %clang -march=pentium4m -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM4M_M32
// CHECK_PENTIUM4M_M32: #define __MMX__ 1
// CHECK_PENTIUM4M_M32: #define __SSE2__ 1
// CHECK_PENTIUM4M_M32: #define __SSE__ 1
// CHECK_PENTIUM4M_M32: #define __i386 1
// CHECK_PENTIUM4M_M32: #define __i386__ 1
// CHECK_PENTIUM4M_M32: #define __pentium4 1
// CHECK_PENTIUM4M_M32: #define __pentium4__ 1
// CHECK_PENTIUM4M_M32: #define __tune_pentium4__ 1
// CHECK_PENTIUM4M_M32: #define i386 1
// RUN: not %clang -march=pentium4m -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PENTIUM4M_M64
// CHECK_PENTIUM4M_M64: error:
//
// RUN: %clang -march=prescott -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PRESCOTT_M32
// CHECK_PRESCOTT_M32: #define __MMX__ 1
// CHECK_PRESCOTT_M32: #define __SSE2__ 1
// CHECK_PRESCOTT_M32: #define __SSE3__ 1
// CHECK_PRESCOTT_M32: #define __SSE__ 1
// CHECK_PRESCOTT_M32: #define __i386 1
// CHECK_PRESCOTT_M32: #define __i386__ 1
// CHECK_PRESCOTT_M32: #define __nocona 1
// CHECK_PRESCOTT_M32: #define __nocona__ 1
// CHECK_PRESCOTT_M32: #define __tune_nocona__ 1
// CHECK_PRESCOTT_M32: #define i386 1
// RUN: not %clang -march=prescott -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PRESCOTT_M64
// CHECK_PRESCOTT_M64: error:
//
// RUN: %clang -march=nocona -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_NOCONA_M32
// CHECK_NOCONA_M32: #define __MMX__ 1
// CHECK_NOCONA_M32: #define __SSE2__ 1
// CHECK_NOCONA_M32: #define __SSE3__ 1
// CHECK_NOCONA_M32: #define __SSE__ 1
// CHECK_NOCONA_M32: #define __i386 1
// CHECK_NOCONA_M32: #define __i386__ 1
// CHECK_NOCONA_M32: #define __nocona 1
// CHECK_NOCONA_M32: #define __nocona__ 1
// CHECK_NOCONA_M32: #define __tune_nocona__ 1
// CHECK_NOCONA_M32: #define i386 1
// RUN: %clang -march=nocona -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_NOCONA_M64
// CHECK_NOCONA_M64: #define __MMX__ 1
// CHECK_NOCONA_M64: #define __SSE2_MATH__ 1
// CHECK_NOCONA_M64: #define __SSE2__ 1
// CHECK_NOCONA_M64: #define __SSE3__ 1
// CHECK_NOCONA_M64: #define __SSE_MATH__ 1
// CHECK_NOCONA_M64: #define __SSE__ 1
// CHECK_NOCONA_M64: #define __amd64 1
// CHECK_NOCONA_M64: #define __amd64__ 1
// CHECK_NOCONA_M64: #define __nocona 1
// CHECK_NOCONA_M64: #define __nocona__ 1
// CHECK_NOCONA_M64: #define __tune_nocona__ 1
// CHECK_NOCONA_M64: #define __x86_64 1
// CHECK_NOCONA_M64: #define __x86_64__ 1
//
// RUN: %clang -march=core2 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_CORE2_M32
// CHECK_CORE2_M32: #define __MMX__ 1
// CHECK_CORE2_M32: #define __SSE2__ 1
// CHECK_CORE2_M32: #define __SSE3__ 1
// CHECK_CORE2_M32: #define __SSE__ 1
// CHECK_CORE2_M32: #define __SSSE3__ 1
// CHECK_CORE2_M32: #define __core2 1
// CHECK_CORE2_M32: #define __core2__ 1
// CHECK_CORE2_M32: #define __i386 1
// CHECK_CORE2_M32: #define __i386__ 1
// CHECK_CORE2_M32: #define __tune_core2__ 1
// CHECK_CORE2_M32: #define i386 1
// RUN: %clang -march=core2 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_CORE2_M64
// CHECK_CORE2_M64: #define __MMX__ 1
// CHECK_CORE2_M64: #define __SSE2_MATH__ 1
// CHECK_CORE2_M64: #define __SSE2__ 1
// CHECK_CORE2_M64: #define __SSE3__ 1
// CHECK_CORE2_M64: #define __SSE_MATH__ 1
// CHECK_CORE2_M64: #define __SSE__ 1
// CHECK_CORE2_M64: #define __SSSE3__ 1
// CHECK_CORE2_M64: #define __amd64 1
// CHECK_CORE2_M64: #define __amd64__ 1
// CHECK_CORE2_M64: #define __core2 1
// CHECK_CORE2_M64: #define __core2__ 1
// CHECK_CORE2_M64: #define __tune_core2__ 1
// CHECK_CORE2_M64: #define __x86_64 1
// CHECK_CORE2_M64: #define __x86_64__ 1
//
// RUN: %clang -march=corei7 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_COREI7_M32
// CHECK_COREI7_M32: #define __MMX__ 1
// CHECK_COREI7_M32: #define __POPCNT__ 1
// CHECK_COREI7_M32: #define __SSE2__ 1
// CHECK_COREI7_M32: #define __SSE3__ 1
// CHECK_COREI7_M32: #define __SSE4_1__ 1
// CHECK_COREI7_M32: #define __SSE4_2__ 1
// CHECK_COREI7_M32: #define __SSE__ 1
// CHECK_COREI7_M32: #define __SSSE3__ 1
// CHECK_COREI7_M32: #define __corei7 1
// CHECK_COREI7_M32: #define __corei7__ 1
// CHECK_COREI7_M32: #define __i386 1
// CHECK_COREI7_M32: #define __i386__ 1
// CHECK_COREI7_M32: #define __tune_corei7__ 1
// CHECK_COREI7_M32: #define i386 1
// RUN: %clang -march=corei7 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_COREI7_M64
// CHECK_COREI7_M64: #define __MMX__ 1
// CHECK_COREI7_M64: #define __POPCNT__ 1
// CHECK_COREI7_M64: #define __SSE2_MATH__ 1
// CHECK_COREI7_M64: #define __SSE2__ 1
// CHECK_COREI7_M64: #define __SSE3__ 1
// CHECK_COREI7_M64: #define __SSE4_1__ 1
// CHECK_COREI7_M64: #define __SSE4_2__ 1
// CHECK_COREI7_M64: #define __SSE_MATH__ 1
// CHECK_COREI7_M64: #define __SSE__ 1
// CHECK_COREI7_M64: #define __SSSE3__ 1
// CHECK_COREI7_M64: #define __amd64 1
// CHECK_COREI7_M64: #define __amd64__ 1
// CHECK_COREI7_M64: #define __corei7 1
// CHECK_COREI7_M64: #define __corei7__ 1
// CHECK_COREI7_M64: #define __tune_corei7__ 1
// CHECK_COREI7_M64: #define __x86_64 1
// CHECK_COREI7_M64: #define __x86_64__ 1
//
// RUN: %clang -march=corei7-avx -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_COREI7_AVX_M32
// CHECK_COREI7_AVX_M32: #define __AES__ 1
// CHECK_COREI7_AVX_M32: #define __AVX__ 1
// CHECK_COREI7_AVX_M32: #define __MMX__ 1
// CHECK_COREI7_AVX_M32: #define __PCLMUL__ 1
// CHECK_COREI7_AVX_M32-NOT: __RDRND__
// CHECK_COREI7_AVX_M32: #define __POPCNT__ 1
// CHECK_COREI7_AVX_M32: #define __SSE2__ 1
// CHECK_COREI7_AVX_M32: #define __SSE3__ 1
// CHECK_COREI7_AVX_M32: #define __SSE4_1__ 1
// CHECK_COREI7_AVX_M32: #define __SSE4_2__ 1
// CHECK_COREI7_AVX_M32: #define __SSE__ 1
// CHECK_COREI7_AVX_M32: #define __SSSE3__ 1
// CHECK_COREI7_AVX_M32: #define __corei7 1
// CHECK_COREI7_AVX_M32: #define __corei7__ 1
// CHECK_COREI7_AVX_M32: #define __i386 1
// CHECK_COREI7_AVX_M32: #define __i386__ 1
// CHECK_COREI7_AVX_M32: #define __tune_corei7__ 1
// CHECK_COREI7_AVX_M32: #define i386 1
// RUN: %clang -march=corei7-avx -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_COREI7_AVX_M64
// CHECK_COREI7_AVX_M64: #define __AES__ 1
// CHECK_COREI7_AVX_M64: #define __AVX__ 1
// CHECK_COREI7_AVX_M64: #define __MMX__ 1
// CHECK_COREI7_AVX_M64: #define __PCLMUL__ 1
// CHECK_COREI7_AVX_M64-NOT: __RDRND__
// CHECK_COREI7_AVX_M64: #define __POPCNT__ 1
// CHECK_COREI7_AVX_M64: #define __SSE2_MATH__ 1
// CHECK_COREI7_AVX_M64: #define __SSE2__ 1
// CHECK_COREI7_AVX_M64: #define __SSE3__ 1
// CHECK_COREI7_AVX_M64: #define __SSE4_1__ 1
// CHECK_COREI7_AVX_M64: #define __SSE4_2__ 1
// CHECK_COREI7_AVX_M64: #define __SSE_MATH__ 1
// CHECK_COREI7_AVX_M64: #define __SSE__ 1
// CHECK_COREI7_AVX_M64: #define __SSSE3__ 1
// CHECK_COREI7_AVX_M64: #define __amd64 1
// CHECK_COREI7_AVX_M64: #define __amd64__ 1
// CHECK_COREI7_AVX_M64: #define __corei7 1
// CHECK_COREI7_AVX_M64: #define __corei7__ 1
// CHECK_COREI7_AVX_M64: #define __tune_corei7__ 1
// CHECK_COREI7_AVX_M64: #define __x86_64 1
// CHECK_COREI7_AVX_M64: #define __x86_64__ 1
//
// RUN: %clang -march=core-avx-i -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_CORE_AVX_I_M32
// CHECK_CORE_AVX_I_M32: #define __AES__ 1
// CHECK_CORE_AVX_I_M32: #define __AVX__ 1
// CHECK_CORE_AVX_I_M32: #define __F16C__ 1
// CHECK_CORE_AVX_I_M32: #define __MMX__ 1
// CHECK_CORE_AVX_I_M32: #define __PCLMUL__ 1
// CHECK_CORE_AVX_I_M32: #define __RDRND__ 1
// CHECK_CORE_AVX_I_M32: #define __SSE2__ 1
// CHECK_CORE_AVX_I_M32: #define __SSE3__ 1
// CHECK_CORE_AVX_I_M32: #define __SSE4_1__ 1
// CHECK_CORE_AVX_I_M32: #define __SSE4_2__ 1
// CHECK_CORE_AVX_I_M32: #define __SSE__ 1
// CHECK_CORE_AVX_I_M32: #define __SSSE3__ 1
// CHECK_CORE_AVX_I_M32: #define __corei7 1
// CHECK_CORE_AVX_I_M32: #define __corei7__ 1
// CHECK_CORE_AVX_I_M32: #define __i386 1
// CHECK_CORE_AVX_I_M32: #define __i386__ 1
// CHECK_CORE_AVX_I_M32: #define __tune_corei7__ 1
// CHECK_CORE_AVX_I_M32: #define i386 1
// RUN: %clang -march=core-avx-i -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_CORE_AVX_I_M64
// CHECK_CORE_AVX_I_M64: #define __AES__ 1
// CHECK_CORE_AVX_I_M64: #define __AVX__ 1
// CHECK_CORE_AVX_I_M64: #define __F16C__ 1
// CHECK_CORE_AVX_I_M64: #define __MMX__ 1
// CHECK_CORE_AVX_I_M64: #define __PCLMUL__ 1
// CHECK_CORE_AVX_I_M64: #define __RDRND__ 1
// CHECK_CORE_AVX_I_M64: #define __SSE2_MATH__ 1
// CHECK_CORE_AVX_I_M64: #define __SSE2__ 1
// CHECK_CORE_AVX_I_M64: #define __SSE3__ 1
// CHECK_CORE_AVX_I_M64: #define __SSE4_1__ 1
// CHECK_CORE_AVX_I_M64: #define __SSE4_2__ 1
// CHECK_CORE_AVX_I_M64: #define __SSE_MATH__ 1
// CHECK_CORE_AVX_I_M64: #define __SSE__ 1
// CHECK_CORE_AVX_I_M64: #define __SSSE3__ 1
// CHECK_CORE_AVX_I_M64: #define __amd64 1
// CHECK_CORE_AVX_I_M64: #define __amd64__ 1
// CHECK_CORE_AVX_I_M64: #define __corei7 1
// CHECK_CORE_AVX_I_M64: #define __corei7__ 1
// CHECK_CORE_AVX_I_M64: #define __tune_corei7__ 1
// CHECK_CORE_AVX_I_M64: #define __x86_64 1
// CHECK_CORE_AVX_I_M64: #define __x86_64__ 1
//
// RUN: %clang -march=core-avx2 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_CORE_AVX2_M32
// CHECK_CORE_AVX2_M32: #define __AES__ 1
// CHECK_CORE_AVX2_M32: #define __AVX2__ 1
// CHECK_CORE_AVX2_M32: #define __AVX__ 1
// CHECK_CORE_AVX2_M32: #define __BMI2__ 1
// CHECK_CORE_AVX2_M32: #define __BMI__ 1
// CHECK_CORE_AVX2_M32: #define __F16C__ 1
// CHECK_CORE_AVX2_M32: #define __FMA__ 1
// CHECK_CORE_AVX2_M32: #define __LZCNT__ 1
// CHECK_CORE_AVX2_M32: #define __MMX__ 1
// CHECK_CORE_AVX2_M32: #define __PCLMUL__ 1
// CHECK_CORE_AVX2_M32: #define __POPCNT__ 1
// CHECK_CORE_AVX2_M32: #define __RDRND__ 1
// CHECK_CORE_AVX2_M32: #define __RTM__ 1
// CHECK_CORE_AVX2_M32: #define __SSE2__ 1
// CHECK_CORE_AVX2_M32: #define __SSE3__ 1
// CHECK_CORE_AVX2_M32: #define __SSE4_1__ 1
// CHECK_CORE_AVX2_M32: #define __SSE4_2__ 1
// CHECK_CORE_AVX2_M32: #define __SSE__ 1
// CHECK_CORE_AVX2_M32: #define __SSSE3__ 1
// CHECK_CORE_AVX2_M32: #define __corei7 1
// CHECK_CORE_AVX2_M32: #define __corei7__ 1
// CHECK_CORE_AVX2_M32: #define __i386 1
// CHECK_CORE_AVX2_M32: #define __i386__ 1
// CHECK_CORE_AVX2_M32: #define __tune_corei7__ 1
// CHECK_CORE_AVX2_M32: #define i386 1
// RUN: %clang -march=core-avx2 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_CORE_AVX2_M64
// CHECK_CORE_AVX2_M64: #define __AES__ 1
// CHECK_CORE_AVX2_M64: #define __AVX2__ 1
// CHECK_CORE_AVX2_M64: #define __AVX__ 1
// CHECK_CORE_AVX2_M64: #define __BMI2__ 1
// CHECK_CORE_AVX2_M64: #define __BMI__ 1
// CHECK_CORE_AVX2_M64: #define __F16C__ 1
// CHECK_CORE_AVX2_M64: #define __FMA__ 1
// CHECK_CORE_AVX2_M64: #define __LZCNT__ 1
// CHECK_CORE_AVX2_M64: #define __MMX__ 1
// CHECK_CORE_AVX2_M64: #define __PCLMUL__ 1
// CHECK_CORE_AVX2_M64: #define __POPCNT__ 1
// CHECK_CORE_AVX2_M64: #define __RDRND__ 1
// CHECK_CORE_AVX2_M64: #define __RTM__ 1
// CHECK_CORE_AVX2_M64: #define __SSE2_MATH__ 1
// CHECK_CORE_AVX2_M64: #define __SSE2__ 1
// CHECK_CORE_AVX2_M64: #define __SSE3__ 1
// CHECK_CORE_AVX2_M64: #define __SSE4_1__ 1
// CHECK_CORE_AVX2_M64: #define __SSE4_2__ 1
// CHECK_CORE_AVX2_M64: #define __SSE_MATH__ 1
// CHECK_CORE_AVX2_M64: #define __SSE__ 1
// CHECK_CORE_AVX2_M64: #define __SSSE3__ 1
// CHECK_CORE_AVX2_M64: #define __amd64 1
// CHECK_CORE_AVX2_M64: #define __amd64__ 1
// CHECK_CORE_AVX2_M64: #define __corei7 1
// CHECK_CORE_AVX2_M64: #define __corei7__ 1
// CHECK_CORE_AVX2_M64: #define __tune_corei7__ 1
// CHECK_CORE_AVX2_M64: #define __x86_64 1
// CHECK_CORE_AVX2_M64: #define __x86_64__ 1
//
// RUN: %clang -march=broadwell -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BROADWELL_M32
// CHECK_BROADWELL_M32: #define __ADX__ 1
// CHECK_BROADWELL_M32: #define __AES__ 1
// CHECK_BROADWELL_M32: #define __AVX2__ 1
// CHECK_BROADWELL_M32: #define __AVX__ 1
// CHECK_BROADWELL_M32: #define __BMI2__ 1
// CHECK_BROADWELL_M32: #define __BMI__ 1
// CHECK_BROADWELL_M32: #define __F16C__ 1
// CHECK_BROADWELL_M32: #define __FMA__ 1
// CHECK_BROADWELL_M32: #define __LZCNT__ 1
// CHECK_BROADWELL_M32: #define __MMX__ 1
// CHECK_BROADWELL_M32: #define __PCLMUL__ 1
// CHECK_BROADWELL_M32: #define __POPCNT__ 1
// CHECK_BROADWELL_M32: #define __RDRND__ 1
// CHECK_BROADWELL_M32: #define __RDSEED__ 1
// CHECK_BROADWELL_M32: #define __RTM__ 1
// CHECK_BROADWELL_M32: #define __SSE2__ 1
// CHECK_BROADWELL_M32: #define __SSE3__ 1
// CHECK_BROADWELL_M32: #define __SSE4_1__ 1
// CHECK_BROADWELL_M32: #define __SSE4_2__ 1
// CHECK_BROADWELL_M32: #define __SSE__ 1
// CHECK_BROADWELL_M32: #define __SSSE3__ 1
// CHECK_BROADWELL_M32: #define __corei7 1
// CHECK_BROADWELL_M32: #define __corei7__ 1
// CHECK_BROADWELL_M32: #define __i386 1
// CHECK_BROADWELL_M32: #define __i386__ 1
// CHECK_BROADWELL_M32: #define __tune_corei7__ 1
// CHECK_BROADWELL_M32: #define i386 1
// RUN: %clang -march=broadwell -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BROADWELL_M64
// CHECK_BROADWELL_M64: #define __ADX__ 1
// CHECK_BROADWELL_M64: #define __AES__ 1
// CHECK_BROADWELL_M64: #define __AVX2__ 1
// CHECK_BROADWELL_M64: #define __AVX__ 1
// CHECK_BROADWELL_M64: #define __BMI2__ 1
// CHECK_BROADWELL_M64: #define __BMI__ 1
// CHECK_BROADWELL_M64: #define __F16C__ 1
// CHECK_BROADWELL_M64: #define __FMA__ 1
// CHECK_BROADWELL_M64: #define __LZCNT__ 1
// CHECK_BROADWELL_M64: #define __MMX__ 1
// CHECK_BROADWELL_M64: #define __PCLMUL__ 1
// CHECK_BROADWELL_M64: #define __POPCNT__ 1
// CHECK_BROADWELL_M64: #define __RDRND__ 1
// CHECK_BROADWELL_M64: #define __RDSEED__ 1
// CHECK_BROADWELL_M64: #define __RTM__ 1
// CHECK_BROADWELL_M64: #define __SSE2_MATH__ 1
// CHECK_BROADWELL_M64: #define __SSE2__ 1
// CHECK_BROADWELL_M64: #define __SSE3__ 1
// CHECK_BROADWELL_M64: #define __SSE4_1__ 1
// CHECK_BROADWELL_M64: #define __SSE4_2__ 1
// CHECK_BROADWELL_M64: #define __SSE_MATH__ 1
// CHECK_BROADWELL_M64: #define __SSE__ 1
// CHECK_BROADWELL_M64: #define __SSSE3__ 1
// CHECK_BROADWELL_M64: #define __amd64 1
// CHECK_BROADWELL_M64: #define __amd64__ 1
// CHECK_BROADWELL_M64: #define __corei7 1
// CHECK_BROADWELL_M64: #define __corei7__ 1
// CHECK_BROADWELL_M64: #define __tune_corei7__ 1
// CHECK_BROADWELL_M64: #define __x86_64 1
// CHECK_BROADWELL_M64: #define __x86_64__ 1
//
// RUN: %clang -march=knl -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_KNL_M32
// CHECK_KNL_M32: #define __AES__ 1
// CHECK_KNL_M32: #define __AVX2__ 1
// CHECK_KNL_M32: #define __AVX512CD__ 1
// CHECK_KNL_M32: #define __AVX512ER__ 1
// CHECK_KNL_M32: #define __AVX512F__ 1
// CHECK_KNL_M32: #define __AVX512PF__ 1
// CHECK_KNL_M32: #define __AVX__ 1
// CHECK_KNL_M32: #define __BMI2__ 1
// CHECK_KNL_M32: #define __BMI__ 1
// CHECK_KNL_M32: #define __F16C__ 1
// CHECK_KNL_M32: #define __FMA__ 1
// CHECK_KNL_M32: #define __LZCNT__ 1
// CHECK_KNL_M32: #define __MMX__ 1
// CHECK_KNL_M32: #define __PCLMUL__ 1
// CHECK_KNL_M32: #define __POPCNT__ 1
// CHECK_KNL_M32: #define __RDRND__ 1
// CHECK_KNL_M32: #define __RTM__ 1
// CHECK_KNL_M32: #define __SSE2__ 1
// CHECK_KNL_M32: #define __SSE3__ 1
// CHECK_KNL_M32: #define __SSE4_1__ 1
// CHECK_KNL_M32: #define __SSE4_2__ 1
// CHECK_KNL_M32: #define __SSE__ 1
// CHECK_KNL_M32: #define __SSSE3__ 1
// CHECK_KNL_M32: #define __i386 1
// CHECK_KNL_M32: #define __i386__ 1
// CHECK_KNL_M32: #define __knl 1
// CHECK_KNL_M32: #define __knl__ 1
// CHECK_KNL_M32: #define __tune_knl__ 1
// CHECK_KNL_M32: #define i386 1

// RUN: %clang -march=knl -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_KNL_M64
// CHECK_KNL_M64: #define __AES__ 1
// CHECK_KNL_M64: #define __AVX2__ 1
// CHECK_KNL_M64: #define __AVX512CD__ 1
// CHECK_KNL_M64: #define __AVX512ER__ 1
// CHECK_KNL_M64: #define __AVX512F__ 1
// CHECK_KNL_M64: #define __AVX512PF__ 1
// CHECK_KNL_M64: #define __AVX__ 1
// CHECK_KNL_M64: #define __BMI2__ 1
// CHECK_KNL_M64: #define __BMI__ 1
// CHECK_KNL_M64: #define __F16C__ 1
// CHECK_KNL_M64: #define __FMA__ 1
// CHECK_KNL_M64: #define __LZCNT__ 1
// CHECK_KNL_M64: #define __MMX__ 1
// CHECK_KNL_M64: #define __PCLMUL__ 1
// CHECK_KNL_M64: #define __POPCNT__ 1
// CHECK_KNL_M64: #define __RDRND__ 1
// CHECK_KNL_M64: #define __RTM__ 1
// CHECK_KNL_M64: #define __SSE2_MATH__ 1
// CHECK_KNL_M64: #define __SSE2__ 1
// CHECK_KNL_M64: #define __SSE3__ 1
// CHECK_KNL_M64: #define __SSE4_1__ 1
// CHECK_KNL_M64: #define __SSE4_2__ 1
// CHECK_KNL_M64: #define __SSE_MATH__ 1
// CHECK_KNL_M64: #define __SSE__ 1
// CHECK_KNL_M64: #define __SSSE3__ 1
// CHECK_KNL_M64: #define __amd64 1
// CHECK_KNL_M64: #define __amd64__ 1
// CHECK_KNL_M64: #define __knl 1
// CHECK_KNL_M64: #define __knl__ 1
// CHECK_KNL_M64: #define __tune_knl__ 1
// CHECK_KNL_M64: #define __x86_64 1
// CHECK_KNL_M64: #define __x86_64__ 1
//
// RUN: %clang -march=skx -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SKX_M32
// CHECK_SKX_M32: #define __AES__ 1
// CHECK_SKX_M32: #define __AVX2__ 1
// CHECK_SKX_M32: #define __AVX512BW__ 1
// CHECK_SKX_M32: #define __AVX512CD__ 1
// CHECK_SKX_M32: #define __AVX512DQ__ 1
// CHECK_SKX_M32: #define __AVX512F__ 1
// CHECK_SKX_M32: #define __AVX512VL__ 1
// CHECK_SKX_M32: #define __AVX__ 1
// CHECK_SKX_M32: #define __BMI2__ 1
// CHECK_SKX_M32: #define __BMI__ 1
// CHECK_SKX_M32: #define __F16C__ 1
// CHECK_SKX_M32: #define __FMA__ 1
// CHECK_SKX_M32: #define __LZCNT__ 1
// CHECK_SKX_M32: #define __MMX__ 1
// CHECK_SKX_M32: #define __PCLMUL__ 1
// CHECK_SKX_M32: #define __POPCNT__ 1
// CHECK_SKX_M32: #define __RDRND__ 1
// CHECK_SKX_M32: #define __RTM__ 1
// CHECK_SKX_M32: #define __SSE2__ 1
// CHECK_SKX_M32: #define __SSE3__ 1
// CHECK_SKX_M32: #define __SSE4_1__ 1
// CHECK_SKX_M32: #define __SSE4_2__ 1
// CHECK_SKX_M32: #define __SSE__ 1
// CHECK_SKX_M32: #define __SSSE3__ 1
// CHECK_SKX_M32: #define __i386 1
// CHECK_SKX_M32: #define __i386__ 1
// CHECK_SKX_M32: #define __skx 1
// CHECK_SKX_M32: #define __skx__ 1
// CHECK_SKX_M32: #define __tune_skx__ 1
// CHECK_SKX_M32: #define i386 1

// RUN: %clang -march=skx -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SKX_M64
// CHECK_SKX_M64: #define __AES__ 1
// CHECK_SKX_M64: #define __AVX2__ 1
// CHECK_SKX_M64: #define __AVX512BW__ 1
// CHECK_SKX_M64: #define __AVX512CD__ 1
// CHECK_SKX_M64: #define __AVX512DQ__ 1
// CHECK_SKX_M64: #define __AVX512F__ 1
// CHECK_SKX_M64: #define __AVX512VL__ 1
// CHECK_SKX_M64: #define __AVX__ 1
// CHECK_SKX_M64: #define __BMI2__ 1
// CHECK_SKX_M64: #define __BMI__ 1
// CHECK_SKX_M64: #define __F16C__ 1
// CHECK_SKX_M64: #define __FMA__ 1
// CHECK_SKX_M64: #define __LZCNT__ 1
// CHECK_SKX_M64: #define __MMX__ 1
// CHECK_SKX_M64: #define __PCLMUL__ 1
// CHECK_SKX_M64: #define __POPCNT__ 1
// CHECK_SKX_M64: #define __RDRND__ 1
// CHECK_SKX_M64: #define __RTM__ 1
// CHECK_SKX_M64: #define __SSE2_MATH__ 1
// CHECK_SKX_M64: #define __SSE2__ 1
// CHECK_SKX_M64: #define __SSE3__ 1
// CHECK_SKX_M64: #define __SSE4_1__ 1
// CHECK_SKX_M64: #define __SSE4_2__ 1
// CHECK_SKX_M64: #define __SSE_MATH__ 1
// CHECK_SKX_M64: #define __SSE__ 1
// CHECK_SKX_M64: #define __SSSE3__ 1
// CHECK_SKX_M64: #define __amd64 1
// CHECK_SKX_M64: #define __amd64__ 1
// CHECK_SKX_M64: #define __skx 1
// CHECK_SKX_M64: #define __skx__ 1
// CHECK_SKX_M64: #define __tune_skx__ 1
// CHECK_SKX_M64: #define __x86_64 1
// CHECK_SKX_M64: #define __x86_64__ 1
//
// RUN: %clang -march=atom -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATOM_M32
// CHECK_ATOM_M32: #define __MMX__ 1
// CHECK_ATOM_M32: #define __SSE2__ 1
// CHECK_ATOM_M32: #define __SSE3__ 1
// CHECK_ATOM_M32: #define __SSE__ 1
// CHECK_ATOM_M32: #define __SSSE3__ 1
// CHECK_ATOM_M32: #define __atom 1
// CHECK_ATOM_M32: #define __atom__ 1
// CHECK_ATOM_M32: #define __i386 1
// CHECK_ATOM_M32: #define __i386__ 1
// CHECK_ATOM_M32: #define __tune_atom__ 1
// CHECK_ATOM_M32: #define i386 1
// RUN: %clang -march=atom -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATOM_M64
// CHECK_ATOM_M64: #define __MMX__ 1
// CHECK_ATOM_M64: #define __SSE2_MATH__ 1
// CHECK_ATOM_M64: #define __SSE2__ 1
// CHECK_ATOM_M64: #define __SSE3__ 1
// CHECK_ATOM_M64: #define __SSE_MATH__ 1
// CHECK_ATOM_M64: #define __SSE__ 1
// CHECK_ATOM_M64: #define __SSSE3__ 1
// CHECK_ATOM_M64: #define __amd64 1
// CHECK_ATOM_M64: #define __amd64__ 1
// CHECK_ATOM_M64: #define __atom 1
// CHECK_ATOM_M64: #define __atom__ 1
// CHECK_ATOM_M64: #define __tune_atom__ 1
// CHECK_ATOM_M64: #define __x86_64 1
// CHECK_ATOM_M64: #define __x86_64__ 1
//
// RUN: %clang -march=slm -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SLM_M32
// CHECK_SLM_M32: #define __MMX__ 1
// CHECK_SLM_M32: #define __SSE2__ 1
// CHECK_SLM_M32: #define __SSE3__ 1
// CHECK_SLM_M32: #define __SSE4_1__ 1
// CHECK_SLM_M32: #define __SSE4_2__ 1
// CHECK_SLM_M32: #define __SSE__ 1
// CHECK_SLM_M32: #define __SSSE3__ 1
// CHECK_SLM_M32: #define __i386 1
// CHECK_SLM_M32: #define __i386__ 1
// CHECK_SLM_M32: #define __slm 1
// CHECK_SLM_M32: #define __slm__ 1
// CHECK_SLM_M32: #define __tune_slm__ 1
// CHECK_SLM_M32: #define i386 1
// RUN: %clang -march=slm -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SLM_M64
// CHECK_SLM_M64: #define __MMX__ 1
// CHECK_SLM_M64: #define __SSE2_MATH__ 1
// CHECK_SLM_M64: #define __SSE2__ 1
// CHECK_SLM_M64: #define __SSE3__ 1
// CHECK_SLM_M64: #define __SSE4_1__ 1
// CHECK_SLM_M64: #define __SSE4_2__ 1
// CHECK_SLM_M64: #define __SSE_MATH__ 1
// CHECK_SLM_M64: #define __SSE__ 1
// CHECK_SLM_M64: #define __SSSE3__ 1
// CHECK_SLM_M64: #define __amd64 1
// CHECK_SLM_M64: #define __amd64__ 1
// CHECK_SLM_M64: #define __slm 1
// CHECK_SLM_M64: #define __slm__ 1
// CHECK_SLM_M64: #define __tune_slm__ 1
// CHECK_SLM_M64: #define __x86_64 1
// CHECK_SLM_M64: #define __x86_64__ 1
//
// RUN: %clang -march=geode -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_GEODE_M32
// CHECK_GEODE_M32: #define __3dNOW_A__ 1
// CHECK_GEODE_M32: #define __3dNOW__ 1
// CHECK_GEODE_M32: #define __MMX__ 1
// CHECK_GEODE_M32: #define __geode 1
// CHECK_GEODE_M32: #define __geode__ 1
// CHECK_GEODE_M32: #define __i386 1
// CHECK_GEODE_M32: #define __i386__ 1
// CHECK_GEODE_M32: #define __tune_geode__ 1
// CHECK_GEODE_M32: #define i386 1
// RUN: not %clang -march=geode -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_GEODE_M64
// CHECK_GEODE_M64: error:
//
// RUN: %clang -march=k6 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_K6_M32
// CHECK_K6_M32: #define __MMX__ 1
// CHECK_K6_M32: #define __i386 1
// CHECK_K6_M32: #define __i386__ 1
// CHECK_K6_M32: #define __k6 1
// CHECK_K6_M32: #define __k6__ 1
// CHECK_K6_M32: #define __tune_k6__ 1
// CHECK_K6_M32: #define i386 1
// RUN: not %clang -march=k6 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_K6_M64
// CHECK_K6_M64: error:
//
// RUN: %clang -march=k6-2 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_K6_2_M32
// CHECK_K6_2_M32: #define __3dNOW__ 1
// CHECK_K6_2_M32: #define __MMX__ 1
// CHECK_K6_2_M32: #define __i386 1
// CHECK_K6_2_M32: #define __i386__ 1
// CHECK_K6_2_M32: #define __k6 1
// CHECK_K6_2_M32: #define __k6_2__ 1
// CHECK_K6_2_M32: #define __k6__ 1
// CHECK_K6_2_M32: #define __tune_k6_2__ 1
// CHECK_K6_2_M32: #define __tune_k6__ 1
// CHECK_K6_2_M32: #define i386 1
// RUN: not %clang -march=k6-2 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_K6_2_M64
// CHECK_K6_2_M64: error:
//
// RUN: %clang -march=k6-3 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_K6_3_M32
// CHECK_K6_3_M32: #define __3dNOW__ 1
// CHECK_K6_3_M32: #define __MMX__ 1
// CHECK_K6_3_M32: #define __i386 1
// CHECK_K6_3_M32: #define __i386__ 1
// CHECK_K6_3_M32: #define __k6 1
// CHECK_K6_3_M32: #define __k6_3__ 1
// CHECK_K6_3_M32: #define __k6__ 1
// CHECK_K6_3_M32: #define __tune_k6_3__ 1
// CHECK_K6_3_M32: #define __tune_k6__ 1
// CHECK_K6_3_M32: #define i386 1
// RUN: not %clang -march=k6-3 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_K6_3_M64
// CHECK_K6_3_M64: error:
//
// RUN: %clang -march=athlon -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_M32
// CHECK_ATHLON_M32: #define __3dNOW_A__ 1
// CHECK_ATHLON_M32: #define __3dNOW__ 1
// CHECK_ATHLON_M32: #define __MMX__ 1
// CHECK_ATHLON_M32: #define __athlon 1
// CHECK_ATHLON_M32: #define __athlon__ 1
// CHECK_ATHLON_M32: #define __i386 1
// CHECK_ATHLON_M32: #define __i386__ 1
// CHECK_ATHLON_M32: #define __tune_athlon__ 1
// CHECK_ATHLON_M32: #define i386 1
// RUN: not %clang -march=athlon -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_M64
// CHECK_ATHLON_M64: error:
//
// RUN: %clang -march=athlon-tbird -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_TBIRD_M32
// CHECK_ATHLON_TBIRD_M32: #define __3dNOW_A__ 1
// CHECK_ATHLON_TBIRD_M32: #define __3dNOW__ 1
// CHECK_ATHLON_TBIRD_M32: #define __MMX__ 1
// CHECK_ATHLON_TBIRD_M32: #define __athlon 1
// CHECK_ATHLON_TBIRD_M32: #define __athlon__ 1
// CHECK_ATHLON_TBIRD_M32: #define __i386 1
// CHECK_ATHLON_TBIRD_M32: #define __i386__ 1
// CHECK_ATHLON_TBIRD_M32: #define __tune_athlon__ 1
// CHECK_ATHLON_TBIRD_M32: #define i386 1
// RUN: not %clang -march=athlon-tbird -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_TBIRD_M64
// CHECK_ATHLON_TBIRD_M64: error:
//
// RUN: %clang -march=athlon-4 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_4_M32
// CHECK_ATHLON_4_M32: #define __3dNOW_A__ 1
// CHECK_ATHLON_4_M32: #define __3dNOW__ 1
// CHECK_ATHLON_4_M32: #define __MMX__ 1
// CHECK_ATHLON_4_M32: #define __SSE__ 1
// CHECK_ATHLON_4_M32: #define __athlon 1
// CHECK_ATHLON_4_M32: #define __athlon__ 1
// CHECK_ATHLON_4_M32: #define __athlon_sse__ 1
// CHECK_ATHLON_4_M32: #define __i386 1
// CHECK_ATHLON_4_M32: #define __i386__ 1
// CHECK_ATHLON_4_M32: #define __tune_athlon__ 1
// CHECK_ATHLON_4_M32: #define __tune_athlon_sse__ 1
// CHECK_ATHLON_4_M32: #define i386 1
// RUN: not %clang -march=athlon-4 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_4_M64
// CHECK_ATHLON_4_M64: error:
//
// RUN: %clang -march=athlon-xp -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_XP_M32
// CHECK_ATHLON_XP_M32: #define __3dNOW_A__ 1
// CHECK_ATHLON_XP_M32: #define __3dNOW__ 1
// CHECK_ATHLON_XP_M32: #define __MMX__ 1
// CHECK_ATHLON_XP_M32: #define __SSE__ 1
// CHECK_ATHLON_XP_M32: #define __athlon 1
// CHECK_ATHLON_XP_M32: #define __athlon__ 1
// CHECK_ATHLON_XP_M32: #define __athlon_sse__ 1
// CHECK_ATHLON_XP_M32: #define __i386 1
// CHECK_ATHLON_XP_M32: #define __i386__ 1
// CHECK_ATHLON_XP_M32: #define __tune_athlon__ 1
// CHECK_ATHLON_XP_M32: #define __tune_athlon_sse__ 1
// CHECK_ATHLON_XP_M32: #define i386 1
// RUN: not %clang -march=athlon-xp -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_XP_M64
// CHECK_ATHLON_XP_M64: error:
//
// RUN: %clang -march=athlon-mp -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_MP_M32
// CHECK_ATHLON_MP_M32: #define __3dNOW_A__ 1
// CHECK_ATHLON_MP_M32: #define __3dNOW__ 1
// CHECK_ATHLON_MP_M32: #define __MMX__ 1
// CHECK_ATHLON_MP_M32: #define __SSE__ 1
// CHECK_ATHLON_MP_M32: #define __athlon 1
// CHECK_ATHLON_MP_M32: #define __athlon__ 1
// CHECK_ATHLON_MP_M32: #define __athlon_sse__ 1
// CHECK_ATHLON_MP_M32: #define __i386 1
// CHECK_ATHLON_MP_M32: #define __i386__ 1
// CHECK_ATHLON_MP_M32: #define __tune_athlon__ 1
// CHECK_ATHLON_MP_M32: #define __tune_athlon_sse__ 1
// CHECK_ATHLON_MP_M32: #define i386 1
// RUN: not %clang -march=athlon-mp -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_MP_M64
// CHECK_ATHLON_MP_M64: error:
//
// RUN: %clang -march=x86-64 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_X86_64_M32
// CHECK_X86_64_M32: #define __MMX__ 1
// CHECK_X86_64_M32: #define __SSE2__ 1
// CHECK_X86_64_M32: #define __SSE__ 1
// CHECK_X86_64_M32: #define __i386 1
// CHECK_X86_64_M32: #define __i386__ 1
// CHECK_X86_64_M32: #define __k8 1
// CHECK_X86_64_M32: #define __k8__ 1
// CHECK_X86_64_M32: #define i386 1
// RUN: %clang -march=x86-64 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_X86_64_M64
// CHECK_X86_64_M64: #define __MMX__ 1
// CHECK_X86_64_M64: #define __SSE2_MATH__ 1
// CHECK_X86_64_M64: #define __SSE2__ 1
// CHECK_X86_64_M64: #define __SSE_MATH__ 1
// CHECK_X86_64_M64: #define __SSE__ 1
// CHECK_X86_64_M64: #define __amd64 1
// CHECK_X86_64_M64: #define __amd64__ 1
// CHECK_X86_64_M64: #define __k8 1
// CHECK_X86_64_M64: #define __k8__ 1
// CHECK_X86_64_M64: #define __x86_64 1
// CHECK_X86_64_M64: #define __x86_64__ 1
//
// RUN: %clang -march=k8 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_K8_M32
// CHECK_K8_M32: #define __3dNOW_A__ 1
// CHECK_K8_M32: #define __3dNOW__ 1
// CHECK_K8_M32: #define __MMX__ 1
// CHECK_K8_M32: #define __SSE2__ 1
// CHECK_K8_M32: #define __SSE__ 1
// CHECK_K8_M32: #define __i386 1
// CHECK_K8_M32: #define __i386__ 1
// CHECK_K8_M32: #define __k8 1
// CHECK_K8_M32: #define __k8__ 1
// CHECK_K8_M32: #define __tune_k8__ 1
// CHECK_K8_M32: #define i386 1
// RUN: %clang -march=k8 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_K8_M64
// CHECK_K8_M64: #define __3dNOW_A__ 1
// CHECK_K8_M64: #define __3dNOW__ 1
// CHECK_K8_M64: #define __MMX__ 1
// CHECK_K8_M64: #define __SSE2_MATH__ 1
// CHECK_K8_M64: #define __SSE2__ 1
// CHECK_K8_M64: #define __SSE_MATH__ 1
// CHECK_K8_M64: #define __SSE__ 1
// CHECK_K8_M64: #define __amd64 1
// CHECK_K8_M64: #define __amd64__ 1
// CHECK_K8_M64: #define __k8 1
// CHECK_K8_M64: #define __k8__ 1
// CHECK_K8_M64: #define __tune_k8__ 1
// CHECK_K8_M64: #define __x86_64 1
// CHECK_K8_M64: #define __x86_64__ 1
//
// RUN: %clang -march=k8-sse3 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_K8_SSE3_M32
// CHECK_K8_SSE3_M32: #define __3dNOW_A__ 1
// CHECK_K8_SSE3_M32: #define __3dNOW__ 1
// CHECK_K8_SSE3_M32: #define __MMX__ 1
// CHECK_K8_SSE3_M32: #define __SSE2__ 1
// CHECK_K8_SSE3_M32: #define __SSE3__ 1
// CHECK_K8_SSE3_M32: #define __SSE__ 1
// CHECK_K8_SSE3_M32: #define __i386 1
// CHECK_K8_SSE3_M32: #define __i386__ 1
// CHECK_K8_SSE3_M32: #define __k8 1
// CHECK_K8_SSE3_M32: #define __k8__ 1
// CHECK_K8_SSE3_M32: #define __tune_k8__ 1
// CHECK_K8_SSE3_M32: #define i386 1
// RUN: %clang -march=k8-sse3 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_K8_SSE3_M64
// CHECK_K8_SSE3_M64: #define __3dNOW_A__ 1
// CHECK_K8_SSE3_M64: #define __3dNOW__ 1
// CHECK_K8_SSE3_M64: #define __MMX__ 1
// CHECK_K8_SSE3_M64: #define __SSE2_MATH__ 1
// CHECK_K8_SSE3_M64: #define __SSE2__ 1
// CHECK_K8_SSE3_M64: #define __SSE3__ 1
// CHECK_K8_SSE3_M64: #define __SSE_MATH__ 1
// CHECK_K8_SSE3_M64: #define __SSE__ 1
// CHECK_K8_SSE3_M64: #define __amd64 1
// CHECK_K8_SSE3_M64: #define __amd64__ 1
// CHECK_K8_SSE3_M64: #define __k8 1
// CHECK_K8_SSE3_M64: #define __k8__ 1
// CHECK_K8_SSE3_M64: #define __tune_k8__ 1
// CHECK_K8_SSE3_M64: #define __x86_64 1
// CHECK_K8_SSE3_M64: #define __x86_64__ 1
//
// RUN: %clang -march=opteron -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_OPTERON_M32
// CHECK_OPTERON_M32: #define __3dNOW_A__ 1
// CHECK_OPTERON_M32: #define __3dNOW__ 1
// CHECK_OPTERON_M32: #define __MMX__ 1
// CHECK_OPTERON_M32: #define __SSE2__ 1
// CHECK_OPTERON_M32: #define __SSE__ 1
// CHECK_OPTERON_M32: #define __i386 1
// CHECK_OPTERON_M32: #define __i386__ 1
// CHECK_OPTERON_M32: #define __k8 1
// CHECK_OPTERON_M32: #define __k8__ 1
// CHECK_OPTERON_M32: #define __tune_k8__ 1
// CHECK_OPTERON_M32: #define i386 1
// RUN: %clang -march=opteron -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_OPTERON_M64
// CHECK_OPTERON_M64: #define __3dNOW_A__ 1
// CHECK_OPTERON_M64: #define __3dNOW__ 1
// CHECK_OPTERON_M64: #define __MMX__ 1
// CHECK_OPTERON_M64: #define __SSE2_MATH__ 1
// CHECK_OPTERON_M64: #define __SSE2__ 1
// CHECK_OPTERON_M64: #define __SSE_MATH__ 1
// CHECK_OPTERON_M64: #define __SSE__ 1
// CHECK_OPTERON_M64: #define __amd64 1
// CHECK_OPTERON_M64: #define __amd64__ 1
// CHECK_OPTERON_M64: #define __k8 1
// CHECK_OPTERON_M64: #define __k8__ 1
// CHECK_OPTERON_M64: #define __tune_k8__ 1
// CHECK_OPTERON_M64: #define __x86_64 1
// CHECK_OPTERON_M64: #define __x86_64__ 1
//
// RUN: %clang -march=opteron-sse3 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_OPTERON_SSE3_M32
// CHECK_OPTERON_SSE3_M32: #define __3dNOW_A__ 1
// CHECK_OPTERON_SSE3_M32: #define __3dNOW__ 1
// CHECK_OPTERON_SSE3_M32: #define __MMX__ 1
// CHECK_OPTERON_SSE3_M32: #define __SSE2__ 1
// CHECK_OPTERON_SSE3_M32: #define __SSE3__ 1
// CHECK_OPTERON_SSE3_M32: #define __SSE__ 1
// CHECK_OPTERON_SSE3_M32: #define __i386 1
// CHECK_OPTERON_SSE3_M32: #define __i386__ 1
// CHECK_OPTERON_SSE3_M32: #define __k8 1
// CHECK_OPTERON_SSE3_M32: #define __k8__ 1
// CHECK_OPTERON_SSE3_M32: #define __tune_k8__ 1
// CHECK_OPTERON_SSE3_M32: #define i386 1
// RUN: %clang -march=opteron-sse3 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_OPTERON_SSE3_M64
// CHECK_OPTERON_SSE3_M64: #define __3dNOW_A__ 1
// CHECK_OPTERON_SSE3_M64: #define __3dNOW__ 1
// CHECK_OPTERON_SSE3_M64: #define __MMX__ 1
// CHECK_OPTERON_SSE3_M64: #define __SSE2_MATH__ 1
// CHECK_OPTERON_SSE3_M64: #define __SSE2__ 1
// CHECK_OPTERON_SSE3_M64: #define __SSE3__ 1
// CHECK_OPTERON_SSE3_M64: #define __SSE_MATH__ 1
// CHECK_OPTERON_SSE3_M64: #define __SSE__ 1
// CHECK_OPTERON_SSE3_M64: #define __amd64 1
// CHECK_OPTERON_SSE3_M64: #define __amd64__ 1
// CHECK_OPTERON_SSE3_M64: #define __k8 1
// CHECK_OPTERON_SSE3_M64: #define __k8__ 1
// CHECK_OPTERON_SSE3_M64: #define __tune_k8__ 1
// CHECK_OPTERON_SSE3_M64: #define __x86_64 1
// CHECK_OPTERON_SSE3_M64: #define __x86_64__ 1
//
// RUN: %clang -march=athlon64 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON64_M32
// CHECK_ATHLON64_M32: #define __3dNOW_A__ 1
// CHECK_ATHLON64_M32: #define __3dNOW__ 1
// CHECK_ATHLON64_M32: #define __MMX__ 1
// CHECK_ATHLON64_M32: #define __SSE2__ 1
// CHECK_ATHLON64_M32: #define __SSE__ 1
// CHECK_ATHLON64_M32: #define __i386 1
// CHECK_ATHLON64_M32: #define __i386__ 1
// CHECK_ATHLON64_M32: #define __k8 1
// CHECK_ATHLON64_M32: #define __k8__ 1
// CHECK_ATHLON64_M32: #define __tune_k8__ 1
// CHECK_ATHLON64_M32: #define i386 1
// RUN: %clang -march=athlon64 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON64_M64
// CHECK_ATHLON64_M64: #define __3dNOW_A__ 1
// CHECK_ATHLON64_M64: #define __3dNOW__ 1
// CHECK_ATHLON64_M64: #define __MMX__ 1
// CHECK_ATHLON64_M64: #define __SSE2_MATH__ 1
// CHECK_ATHLON64_M64: #define __SSE2__ 1
// CHECK_ATHLON64_M64: #define __SSE_MATH__ 1
// CHECK_ATHLON64_M64: #define __SSE__ 1
// CHECK_ATHLON64_M64: #define __amd64 1
// CHECK_ATHLON64_M64: #define __amd64__ 1
// CHECK_ATHLON64_M64: #define __k8 1
// CHECK_ATHLON64_M64: #define __k8__ 1
// CHECK_ATHLON64_M64: #define __tune_k8__ 1
// CHECK_ATHLON64_M64: #define __x86_64 1
// CHECK_ATHLON64_M64: #define __x86_64__ 1
//
// RUN: %clang -march=athlon64-sse3 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON64_SSE3_M32
// CHECK_ATHLON64_SSE3_M32: #define __3dNOW_A__ 1
// CHECK_ATHLON64_SSE3_M32: #define __3dNOW__ 1
// CHECK_ATHLON64_SSE3_M32: #define __MMX__ 1
// CHECK_ATHLON64_SSE3_M32: #define __SSE2__ 1
// CHECK_ATHLON64_SSE3_M32: #define __SSE3__ 1
// CHECK_ATHLON64_SSE3_M32: #define __SSE__ 1
// CHECK_ATHLON64_SSE3_M32: #define __i386 1
// CHECK_ATHLON64_SSE3_M32: #define __i386__ 1
// CHECK_ATHLON64_SSE3_M32: #define __k8 1
// CHECK_ATHLON64_SSE3_M32: #define __k8__ 1
// CHECK_ATHLON64_SSE3_M32: #define __tune_k8__ 1
// CHECK_ATHLON64_SSE3_M32: #define i386 1
// RUN: %clang -march=athlon64-sse3 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON64_SSE3_M64
// CHECK_ATHLON64_SSE3_M64: #define __3dNOW_A__ 1
// CHECK_ATHLON64_SSE3_M64: #define __3dNOW__ 1
// CHECK_ATHLON64_SSE3_M64: #define __MMX__ 1
// CHECK_ATHLON64_SSE3_M64: #define __SSE2_MATH__ 1
// CHECK_ATHLON64_SSE3_M64: #define __SSE2__ 1
// CHECK_ATHLON64_SSE3_M64: #define __SSE3__ 1
// CHECK_ATHLON64_SSE3_M64: #define __SSE_MATH__ 1
// CHECK_ATHLON64_SSE3_M64: #define __SSE__ 1
// CHECK_ATHLON64_SSE3_M64: #define __amd64 1
// CHECK_ATHLON64_SSE3_M64: #define __amd64__ 1
// CHECK_ATHLON64_SSE3_M64: #define __k8 1
// CHECK_ATHLON64_SSE3_M64: #define __k8__ 1
// CHECK_ATHLON64_SSE3_M64: #define __tune_k8__ 1
// CHECK_ATHLON64_SSE3_M64: #define __x86_64 1
// CHECK_ATHLON64_SSE3_M64: #define __x86_64__ 1
//
// RUN: %clang -march=athlon-fx -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_FX_M32
// CHECK_ATHLON_FX_M32: #define __3dNOW_A__ 1
// CHECK_ATHLON_FX_M32: #define __3dNOW__ 1
// CHECK_ATHLON_FX_M32: #define __MMX__ 1
// CHECK_ATHLON_FX_M32: #define __SSE2__ 1
// CHECK_ATHLON_FX_M32: #define __SSE__ 1
// CHECK_ATHLON_FX_M32: #define __i386 1
// CHECK_ATHLON_FX_M32: #define __i386__ 1
// CHECK_ATHLON_FX_M32: #define __k8 1
// CHECK_ATHLON_FX_M32: #define __k8__ 1
// CHECK_ATHLON_FX_M32: #define __tune_k8__ 1
// CHECK_ATHLON_FX_M32: #define i386 1
// RUN: %clang -march=athlon-fx -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_ATHLON_FX_M64
// CHECK_ATHLON_FX_M64: #define __3dNOW_A__ 1
// CHECK_ATHLON_FX_M64: #define __3dNOW__ 1
// CHECK_ATHLON_FX_M64: #define __MMX__ 1
// CHECK_ATHLON_FX_M64: #define __SSE2_MATH__ 1
// CHECK_ATHLON_FX_M64: #define __SSE2__ 1
// CHECK_ATHLON_FX_M64: #define __SSE_MATH__ 1
// CHECK_ATHLON_FX_M64: #define __SSE__ 1
// CHECK_ATHLON_FX_M64: #define __amd64 1
// CHECK_ATHLON_FX_M64: #define __amd64__ 1
// CHECK_ATHLON_FX_M64: #define __k8 1
// CHECK_ATHLON_FX_M64: #define __k8__ 1
// CHECK_ATHLON_FX_M64: #define __tune_k8__ 1
// CHECK_ATHLON_FX_M64: #define __x86_64 1
// CHECK_ATHLON_FX_M64: #define __x86_64__ 1
// RUN: %clang -march=amdfam10 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_AMDFAM10_M32
// CHECK_AMDFAM10_M32: #define __3dNOW_A__ 1
// CHECK_AMDFAM10_M32: #define __3dNOW__ 1
// CHECK_AMDFAM10_M32: #define __LZCNT__ 1
// CHECK_AMDFAM10_M32: #define __MMX__ 1
// CHECK_AMDFAM10_M32: #define __POPCNT__ 1
// CHECK_AMDFAM10_M32: #define __SSE2_MATH__ 1
// CHECK_AMDFAM10_M32: #define __SSE2__ 1
// CHECK_AMDFAM10_M32: #define __SSE3__ 1
// CHECK_AMDFAM10_M32: #define __SSE4A__ 1
// CHECK_AMDFAM10_M32: #define __SSE_MATH__ 1
// CHECK_AMDFAM10_M32: #define __SSE__ 1
// CHECK_AMDFAM10_M32: #define __amdfam10 1
// CHECK_AMDFAM10_M32: #define __amdfam10__ 1
// CHECK_AMDFAM10_M32: #define __i386 1
// CHECK_AMDFAM10_M32: #define __i386__ 1
// CHECK_AMDFAM10_M32: #define __tune_amdfam10__ 1
// RUN: %clang -march=amdfam10 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_AMDFAM10_M64
// CHECK_AMDFAM10_M64: #define __3dNOW_A__ 1
// CHECK_AMDFAM10_M64: #define __3dNOW__ 1
// CHECK_AMDFAM10_M64: #define __LZCNT__ 1
// CHECK_AMDFAM10_M64: #define __MMX__ 1
// CHECK_AMDFAM10_M64: #define __POPCNT__ 1
// CHECK_AMDFAM10_M64: #define __SSE2_MATH__ 1
// CHECK_AMDFAM10_M64: #define __SSE2__ 1
// CHECK_AMDFAM10_M64: #define __SSE3__ 1
// CHECK_AMDFAM10_M64: #define __SSE4A__ 1
// CHECK_AMDFAM10_M64: #define __SSE_MATH__ 1
// CHECK_AMDFAM10_M64: #define __SSE__ 1
// CHECK_AMDFAM10_M64: #define __amd64 1
// CHECK_AMDFAM10_M64: #define __amd64__ 1
// CHECK_AMDFAM10_M64: #define __amdfam10 1
// CHECK_AMDFAM10_M64: #define __amdfam10__ 1
// CHECK_AMDFAM10_M64: #define __tune_amdfam10__ 1
// CHECK_AMDFAM10_M64: #define __x86_64 1
// CHECK_AMDFAM10_M64: #define __x86_64__ 1
// RUN: %clang -march=btver1 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BTVER1_M32
// CHECK_BTVER1_M32-NOT: #define __3dNOW_A__ 1
// CHECK_BTVER1_M32-NOT: #define __3dNOW__ 1
// CHECK_BTVER1_M32: #define __LZCNT__ 1
// CHECK_BTVER1_M32: #define __MMX__ 1
// CHECK_BTVER1_M32: #define __POPCNT__ 1
// CHECK_BTVER1_M32: #define __PRFCHW__ 1
// CHECK_BTVER1_M32: #define __SSE2_MATH__ 1
// CHECK_BTVER1_M32: #define __SSE2__ 1
// CHECK_BTVER1_M32: #define __SSE3__ 1
// CHECK_BTVER1_M32: #define __SSE4A__ 1
// CHECK_BTVER1_M32: #define __SSE_MATH__ 1
// CHECK_BTVER1_M32: #define __SSE__ 1
// CHECK_BTVER1_M32: #define __SSSE3__ 1
// CHECK_BTVER1_M32: #define __btver1 1
// CHECK_BTVER1_M32: #define __btver1__ 1
// CHECK_BTVER1_M32: #define __i386 1
// CHECK_BTVER1_M32: #define __i386__ 1
// CHECK_BTVER1_M32: #define __tune_btver1__ 1
// RUN: %clang -march=btver1 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BTVER1_M64
// CHECK_BTVER1_M64-NOT: #define __3dNOW_A__ 1
// CHECK_BTVER1_M64-NOT: #define __3dNOW__ 1
// CHECK_BTVER1_M64: #define __LZCNT__ 1
// CHECK_BTVER1_M64: #define __MMX__ 1
// CHECK_BTVER1_M64: #define __POPCNT__ 1
// CHECK_BTVER1_M64: #define __PRFCHW__ 1
// CHECK_BTVER1_M64: #define __SSE2_MATH__ 1
// CHECK_BTVER1_M64: #define __SSE2__ 1
// CHECK_BTVER1_M64: #define __SSE3__ 1
// CHECK_BTVER1_M64: #define __SSE4A__ 1
// CHECK_BTVER1_M64: #define __SSE_MATH__ 1
// CHECK_BTVER1_M64: #define __SSE__ 1
// CHECK_BTVER1_M64: #define __SSSE3__ 1
// CHECK_BTVER1_M64: #define __amd64 1
// CHECK_BTVER1_M64: #define __amd64__ 1
// CHECK_BTVER1_M64: #define __btver1 1
// CHECK_BTVER1_M64: #define __btver1__ 1
// CHECK_BTVER1_M64: #define __tune_btver1__ 1
// CHECK_BTVER1_M64: #define __x86_64 1
// CHECK_BTVER1_M64: #define __x86_64__ 1
// RUN: %clang -march=btver2 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BTVER2_M32
// CHECK_BTVER2_M32-NOT: #define __3dNOW_A__ 1
// CHECK_BTVER2_M32-NOT: #define __3dNOW__ 1
// CHECK_BTVER2_M32: #define __AES__ 1
// CHECK_BTVER2_M32: #define __AVX__ 1
// CHECK_BTVER2_M32: #define __BMI__ 1
// CHECK_BTVER2_M32: #define __F16C__ 1
// CHECK_BTVER2_M32: #define __LZCNT__ 1
// CHECK_BTVER2_M32: #define __MMX__ 1
// CHECK_BTVER2_M32: #define __PCLMUL__ 1
// CHECK_BTVER2_M32: #define __POPCNT__ 1
// CHECK_BTVER2_M32: #define __PRFCHW__ 1
// CHECK_BTVER2_M32: #define __SSE2_MATH__ 1
// CHECK_BTVER2_M32: #define __SSE2__ 1
// CHECK_BTVER2_M32: #define __SSE3__ 1
// CHECK_BTVER2_M32: #define __SSE4A__ 1
// CHECK_BTVER2_M32: #define __SSE_MATH__ 1
// CHECK_BTVER2_M32: #define __SSE__ 1
// CHECK_BTVER2_M32: #define __SSSE3__ 1
// CHECK_BTVER2_M32: #define __btver2 1
// CHECK_BTVER2_M32: #define __btver2__ 1
// CHECK_BTVER2_M32: #define __i386 1
// CHECK_BTVER2_M32: #define __i386__ 1
// CHECK_BTVER2_M32: #define __tune_btver2__ 1
// RUN: %clang -march=btver2 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BTVER2_M64
// CHECK_BTVER2_M64-NOT: #define __3dNOW_A__ 1
// CHECK_BTVER2_M64-NOT: #define __3dNOW__ 1
// CHECK_BTVER2_M64: #define __AES__ 1
// CHECK_BTVER2_M64: #define __AVX__ 1
// CHECK_BTVER2_M64: #define __BMI__ 1
// CHECK_BTVER2_M64: #define __F16C__ 1
// CHECK_BTVER2_M64: #define __LZCNT__ 1
// CHECK_BTVER2_M64: #define __MMX__ 1
// CHECK_BTVER2_M64: #define __PCLMUL__ 1
// CHECK_BTVER2_M64: #define __POPCNT__ 1
// CHECK_BTVER2_M64: #define __PRFCHW__ 1
// CHECK_BTVER2_M64: #define __SSE2_MATH__ 1
// CHECK_BTVER2_M64: #define __SSE2__ 1
// CHECK_BTVER2_M64: #define __SSE3__ 1
// CHECK_BTVER2_M64: #define __SSE4A__ 1
// CHECK_BTVER2_M64: #define __SSE_MATH__ 1
// CHECK_BTVER2_M64: #define __SSE__ 1
// CHECK_BTVER2_M64: #define __SSSE3__ 1
// CHECK_BTVER2_M64: #define __amd64 1
// CHECK_BTVER2_M64: #define __amd64__ 1
// CHECK_BTVER2_M64: #define __btver2 1
// CHECK_BTVER2_M64: #define __btver2__ 1
// CHECK_BTVER2_M64: #define __tune_btver2__ 1
// CHECK_BTVER2_M64: #define __x86_64 1
// CHECK_BTVER2_M64: #define __x86_64__ 1
// RUN: %clang -march=bdver1 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BDVER1_M32
// CHECK_BDVER1_M32-NOT: #define __3dNOW_A__ 1
// CHECK_BDVER1_M32-NOT: #define __3dNOW__ 1
// CHECK_BDVER1_M32: #define __AES__ 1
// CHECK_BDVER1_M32: #define __AVX__ 1
// CHECK_BDVER1_M32: #define __FMA4__ 1
// CHECK_BDVER1_M32: #define __LZCNT__ 1
// CHECK_BDVER1_M32: #define __MMX__ 1
// CHECK_BDVER1_M32: #define __PCLMUL__ 1
// CHECK_BDVER1_M32: #define __POPCNT__ 1
// CHECK_BDVER1_M32: #define __PRFCHW__ 1
// CHECK_BDVER1_M32: #define __SSE2_MATH__ 1
// CHECK_BDVER1_M32: #define __SSE2__ 1
// CHECK_BDVER1_M32: #define __SSE3__ 1
// CHECK_BDVER1_M32: #define __SSE4A__ 1
// CHECK_BDVER1_M32: #define __SSE4_1__ 1
// CHECK_BDVER1_M32: #define __SSE4_2__ 1
// CHECK_BDVER1_M32: #define __SSE_MATH__ 1
// CHECK_BDVER1_M32: #define __SSE__ 1
// CHECK_BDVER1_M32: #define __SSSE3__ 1
// CHECK_BDVER1_M32: #define __XOP__ 1
// CHECK_BDVER1_M32: #define __bdver1 1
// CHECK_BDVER1_M32: #define __bdver1__ 1
// CHECK_BDVER1_M32: #define __i386 1
// CHECK_BDVER1_M32: #define __i386__ 1
// CHECK_BDVER1_M32: #define __tune_bdver1__ 1
// RUN: %clang -march=bdver1 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BDVER1_M64
// CHECK_BDVER1_M64-NOT: #define __3dNOW_A__ 1
// CHECK_BDVER1_M64-NOT: #define __3dNOW__ 1
// CHECK_BDVER1_M64: #define __AES__ 1
// CHECK_BDVER1_M64: #define __AVX__ 1
// CHECK_BDVER1_M64: #define __FMA4__ 1
// CHECK_BDVER1_M64: #define __LZCNT__ 1
// CHECK_BDVER1_M64: #define __MMX__ 1
// CHECK_BDVER1_M64: #define __PCLMUL__ 1
// CHECK_BDVER1_M64: #define __POPCNT__ 1
// CHECK_BDVER1_M64: #define __PRFCHW__ 1
// CHECK_BDVER1_M64: #define __SSE2_MATH__ 1
// CHECK_BDVER1_M64: #define __SSE2__ 1
// CHECK_BDVER1_M64: #define __SSE3__ 1
// CHECK_BDVER1_M64: #define __SSE4A__ 1
// CHECK_BDVER1_M64: #define __SSE4_1__ 1
// CHECK_BDVER1_M64: #define __SSE4_2__ 1
// CHECK_BDVER1_M64: #define __SSE_MATH__ 1
// CHECK_BDVER1_M64: #define __SSE__ 1
// CHECK_BDVER1_M64: #define __SSSE3__ 1
// CHECK_BDVER1_M64: #define __XOP__ 1
// CHECK_BDVER1_M64: #define __amd64 1
// CHECK_BDVER1_M64: #define __amd64__ 1
// CHECK_BDVER1_M64: #define __bdver1 1
// CHECK_BDVER1_M64: #define __bdver1__ 1
// CHECK_BDVER1_M64: #define __tune_bdver1__ 1
// CHECK_BDVER1_M64: #define __x86_64 1
// CHECK_BDVER1_M64: #define __x86_64__ 1
// RUN: %clang -march=bdver2 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BDVER2_M32
// CHECK_BDVER2_M32-NOT: #define __3dNOW_A__ 1
// CHECK_BDVER2_M32-NOT: #define __3dNOW__ 1
// CHECK_BDVER2_M32: #define __AES__ 1
// CHECK_BDVER2_M32: #define __AVX__ 1
// CHECK_BDVER2_M32: #define __BMI__ 1
// CHECK_BDVER2_M32: #define __F16C__ 1
// CHECK_BDVER2_M32: #define __FMA4__ 1
// CHECK_BDVER2_M32: #define __FMA__ 1
// CHECK_BDVER2_M32: #define __LZCNT__ 1
// CHECK_BDVER2_M32: #define __MMX__ 1
// CHECK_BDVER2_M32: #define __PCLMUL__ 1
// CHECK_BDVER2_M32: #define __POPCNT__ 1
// CHECK_BDVER2_M32: #define __PRFCHW__ 1
// CHECK_BDVER2_M32: #define __SSE2_MATH__ 1
// CHECK_BDVER2_M32: #define __SSE2__ 1
// CHECK_BDVER2_M32: #define __SSE3__ 1
// CHECK_BDVER2_M32: #define __SSE4A__ 1
// CHECK_BDVER2_M32: #define __SSE4_1__ 1
// CHECK_BDVER2_M32: #define __SSE4_2__ 1
// CHECK_BDVER2_M32: #define __SSE_MATH__ 1
// CHECK_BDVER2_M32: #define __SSE__ 1
// CHECK_BDVER2_M32: #define __SSSE3__ 1
// CHECK_BDVER2_M32: #define __TBM__ 1
// CHECK_BDVER2_M32: #define __XOP__ 1
// CHECK_BDVER2_M32: #define __bdver2 1
// CHECK_BDVER2_M32: #define __bdver2__ 1
// CHECK_BDVER2_M32: #define __i386 1
// CHECK_BDVER2_M32: #define __i386__ 1
// CHECK_BDVER2_M32: #define __tune_bdver2__ 1
// RUN: %clang -march=bdver2 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BDVER2_M64
// CHECK_BDVER2_M64-NOT: #define __3dNOW_A__ 1
// CHECK_BDVER2_M64-NOT: #define __3dNOW__ 1
// CHECK_BDVER2_M64: #define __AES__ 1
// CHECK_BDVER2_M64: #define __AVX__ 1
// CHECK_BDVER2_M64: #define __BMI__ 1
// CHECK_BDVER2_M64: #define __F16C__ 1
// CHECK_BDVER2_M64: #define __FMA4__ 1
// CHECK_BDVER2_M64: #define __FMA__ 1
// CHECK_BDVER2_M64: #define __LZCNT__ 1
// CHECK_BDVER2_M64: #define __MMX__ 1
// CHECK_BDVER2_M64: #define __PCLMUL__ 1
// CHECK_BDVER2_M64: #define __POPCNT__ 1
// CHECK_BDVER2_M64: #define __PRFCHW__ 1
// CHECK_BDVER2_M64: #define __SSE2_MATH__ 1
// CHECK_BDVER2_M64: #define __SSE2__ 1
// CHECK_BDVER2_M64: #define __SSE3__ 1
// CHECK_BDVER2_M64: #define __SSE4A__ 1
// CHECK_BDVER2_M64: #define __SSE4_1__ 1
// CHECK_BDVER2_M64: #define __SSE4_2__ 1
// CHECK_BDVER2_M64: #define __SSE_MATH__ 1
// CHECK_BDVER2_M64: #define __SSE__ 1
// CHECK_BDVER2_M64: #define __SSSE3__ 1
// CHECK_BDVER2_M64: #define __TBM__ 1
// CHECK_BDVER2_M64: #define __XOP__ 1
// CHECK_BDVER2_M64: #define __amd64 1
// CHECK_BDVER2_M64: #define __amd64__ 1
// CHECK_BDVER2_M64: #define __bdver2 1
// CHECK_BDVER2_M64: #define __bdver2__ 1
// CHECK_BDVER2_M64: #define __tune_bdver2__ 1
// CHECK_BDVER2_M64: #define __x86_64 1
// CHECK_BDVER2_M64: #define __x86_64__ 1
// RUN: %clang -march=bdver3 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BDVER3_M32
// CHECK_BDVER3_M32-NOT: #define __3dNOW_A__ 1
// CHECK_BDVER3_M32-NOT: #define __3dNOW__ 1
// CHECK_BDVER3_M32: #define __AES__ 1
// CHECK_BDVER3_M32: #define __AVX__ 1
// CHECK_BDVER3_M32: #define __BMI__ 1
// CHECK_BDVER3_M32: #define __F16C__ 1
// CHECK_BDVER3_M32: #define __FMA4__ 1
// CHECK_BDVER3_M32: #define __FMA__ 1
// CHECK_BDVER3_M32: #define __FSGSBASE__ 1
// CHECK_BDVER3_M32: #define __LZCNT__ 1
// CHECK_BDVER3_M32: #define __MMX__ 1
// CHECK_BDVER3_M32: #define __PCLMUL__ 1
// CHECK_BDVER3_M32: #define __POPCNT__ 1
// CHECK_BDVER3_M32: #define __PRFCHW__ 1
// CHECK_BDVER3_M32: #define __SSE2_MATH__ 1
// CHECK_BDVER3_M32: #define __SSE2__ 1
// CHECK_BDVER3_M32: #define __SSE3__ 1
// CHECK_BDVER3_M32: #define __SSE4A__ 1
// CHECK_BDVER3_M32: #define __SSE4_1__ 1
// CHECK_BDVER3_M32: #define __SSE4_2__ 1
// CHECK_BDVER3_M32: #define __SSE_MATH__ 1
// CHECK_BDVER3_M32: #define __SSE__ 1
// CHECK_BDVER3_M32: #define __SSSE3__ 1
// CHECK_BDVER3_M32: #define __TBM__ 1
// CHECK_BDVER3_M32: #define __XOP__ 1
// CHECK_BDVER3_M32: #define __bdver3 1
// CHECK_BDVER3_M32: #define __bdver3__ 1
// CHECK_BDVER3_M32: #define __i386 1
// CHECK_BDVER3_M32: #define __i386__ 1
// CHECK_BDVER3_M32: #define __tune_bdver3__ 1
// RUN: %clang -march=bdver3 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BDVER3_M64
// CHECK_BDVER3_M64-NOT: #define __3dNOW_A__ 1
// CHECK_BDVER3_M64-NOT: #define __3dNOW__ 1
// CHECK_BDVER3_M64: #define __AES__ 1
// CHECK_BDVER3_M64: #define __AVX__ 1
// CHECK_BDVER3_M64: #define __BMI__ 1
// CHECK_BDVER3_M64: #define __F16C__ 1
// CHECK_BDVER3_M64: #define __FMA4__ 1
// CHECK_BDVER3_M64: #define __FMA__ 1
// CHECK_BDVER3_M64: #define __FSGSBASE__ 1
// CHECK_BDVER3_M64: #define __LZCNT__ 1
// CHECK_BDVER3_M64: #define __MMX__ 1
// CHECK_BDVER3_M64: #define __PCLMUL__ 1
// CHECK_BDVER3_M64: #define __POPCNT__ 1
// CHECK_BDVER3_M64: #define __PRFCHW__ 1
// CHECK_BDVER3_M64: #define __SSE2_MATH__ 1
// CHECK_BDVER3_M64: #define __SSE2__ 1
// CHECK_BDVER3_M64: #define __SSE3__ 1
// CHECK_BDVER3_M64: #define __SSE4A__ 1
// CHECK_BDVER3_M64: #define __SSE4_1__ 1
// CHECK_BDVER3_M64: #define __SSE4_2__ 1
// CHECK_BDVER3_M64: #define __SSE_MATH__ 1
// CHECK_BDVER3_M64: #define __SSE__ 1
// CHECK_BDVER3_M64: #define __SSSE3__ 1
// CHECK_BDVER3_M64: #define __TBM__ 1
// CHECK_BDVER3_M64: #define __XOP__ 1
// CHECK_BDVER3_M64: #define __amd64 1
// CHECK_BDVER3_M64: #define __amd64__ 1
// CHECK_BDVER3_M64: #define __bdver3 1
// CHECK_BDVER3_M64: #define __bdver3__ 1
// CHECK_BDVER3_M64: #define __tune_bdver3__ 1
// CHECK_BDVER3_M64: #define __x86_64 1
// CHECK_BDVER3_M64: #define __x86_64__ 1
// RUN: %clang -march=bdver4 -m32 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BDVER4_M32
// CHECK_BDVER4_M32-NOT: #define __3dNOW_A__ 1
// CHECK_BDVER4_M32-NOT: #define __3dNOW__ 1
// CHECK_BDVER4_M32: #define __AES__ 1
// CHECK_BDVER4_M32: #define __AVX2__ 1
// CHECK_BDVER4_M32: #define __AVX__ 1
// CHECK_BDVER4_M32: #define __BMI2__ 1
// CHECK_BDVER4_M32: #define __BMI__ 1
// CHECK_BDVER4_M32: #define __F16C__ 1
// CHECK_BDVER4_M32: #define __FMA4__ 1
// CHECK_BDVER4_M32: #define __FMA__ 1
// CHECK_BDVER4_M32: #define __FSGSBASE__ 1
// CHECK_BDVER4_M32: #define __LZCNT__ 1
// CHECK_BDVER4_M32: #define __MMX__ 1
// CHECK_BDVER4_M32: #define __PCLMUL__ 1
// CHECK_BDVER4_M32: #define __POPCNT__ 1
// CHECK_BDVER4_M32: #define __PRFCHW__ 1
// CHECK_BDVER4_M32: #define __SSE2_MATH__ 1
// CHECK_BDVER4_M32: #define __SSE2__ 1
// CHECK_BDVER4_M32: #define __SSE3__ 1
// CHECK_BDVER4_M32: #define __SSE4A__ 1
// CHECK_BDVER4_M32: #define __SSE4_1__ 1
// CHECK_BDVER4_M32: #define __SSE4_2__ 1
// CHECK_BDVER4_M32: #define __SSE_MATH__ 1
// CHECK_BDVER4_M32: #define __SSE__ 1
// CHECK_BDVER4_M32: #define __SSSE3__ 1
// CHECK_BDVER4_M32: #define __TBM__ 1
// CHECK_BDVER4_M32: #define __XOP__ 1
// CHECK_BDVER4_M32: #define __bdver4 1
// CHECK_BDVER4_M32: #define __bdver4__ 1
// CHECK_BDVER4_M32: #define __i386 1
// CHECK_BDVER4_M32: #define __i386__ 1
// CHECK_BDVER4_M32: #define __tune_bdver4__ 1
// RUN: %clang -march=bdver4 -m64 -E -dM %s -o - 2>&1 \
// RUN:     -target i386-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_BDVER4_M64
// CHECK_BDVER4_M64-NOT: #define __3dNOW_A__ 1
// CHECK_BDVER4_M64-NOT: #define __3dNOW__ 1
// CHECK_BDVER4_M64: #define __AES__ 1
// CHECK_BDVER4_M64: #define __AVX2__ 1
// CHECK_BDVER4_M64: #define __AVX__ 1
// CHECK_BDVER4_M64: #define __BMI2__ 1
// CHECK_BDVER4_M64: #define __BMI__ 1
// CHECK_BDVER4_M64: #define __F16C__ 1
// CHECK_BDVER4_M64: #define __FMA4__ 1
// CHECK_BDVER4_M64: #define __FMA__ 1
// CHECK_BDVER4_M64: #define __FSGSBASE__ 1
// CHECK_BDVER4_M64: #define __LZCNT__ 1
// CHECK_BDVER4_M64: #define __MMX__ 1
// CHECK_BDVER4_M64: #define __PCLMUL__ 1
// CHECK_BDVER4_M64: #define __POPCNT__ 1
// CHECK_BDVER4_M64: #define __PRFCHW__ 1
// CHECK_BDVER4_M64: #define __SSE2_MATH__ 1
// CHECK_BDVER4_M64: #define __SSE2__ 1
// CHECK_BDVER4_M64: #define __SSE3__ 1
// CHECK_BDVER4_M64: #define __SSE4A__ 1
// CHECK_BDVER4_M64: #define __SSE4_1__ 1
// CHECK_BDVER4_M64: #define __SSE4_2__ 1
// CHECK_BDVER4_M64: #define __SSE_MATH__ 1
// CHECK_BDVER4_M64: #define __SSE__ 1
// CHECK_BDVER4_M64: #define __SSSE3__ 1
// CHECK_BDVER4_M64: #define __TBM__ 1
// CHECK_BDVER4_M64: #define __XOP__ 1
// CHECK_BDVER4_M64: #define __amd64 1
// CHECK_BDVER4_M64: #define __amd64__ 1
// CHECK_BDVER4_M64: #define __bdver4 1
// CHECK_BDVER4_M64: #define __bdver4__ 1
// CHECK_BDVER4_M64: #define __tune_bdver4__ 1
// CHECK_BDVER4_M64: #define __x86_64 1
// CHECK_BDVER4_M64: #define __x86_64__ 1
//
// End X86/GCC/Linux tests ------------------

// Begin PPC/GCC/Linux tests ----------------
// RUN: %clang -mvsx -E -dM %s -o - 2>&1 \
// RUN:     -target powerpc64-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PPC_VSX_M64
//
// CHECK_PPC_VSX_M64: #define __VSX__
//
// RUN: %clang -mpower8-vector -E -dM %s -o - 2>&1 \
// RUN:     -target powerpc64-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PPC_POWER8_VECTOR_M64
//
// CHECK_PPC_POWER8_VECTOR_M64: #define __POWER8_VECTOR__
//
// RUN: %clang -mcrypto -E -dM %s -o - 2>&1 \
// RUN:     -target powerpc64-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PPC_CRYPTO_M64
//
// CHECK_PPC_CRYPTO_M64: #define __CRYPTO__
//
// RUN: %clang -mcpu=pwr8 -E -dM %s -o - 2>&1 \
// RUN:     -target powerpc64-unknown-unknown \
// RUN:   | FileCheck %s -check-prefix=CHECK_PPC_GCC_ATOMICS
// RUN: %clang -E -dM %s -o - 2>&1 \
// RUN:     -target powerpc64le-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_PPC_GCC_ATOMICS
//
// CHECK_PPC_GCC_ATOMICS: #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1
// CHECK_PPC_GCC_ATOMICS: #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2
// CHECK_PPC_GCC_ATOMICS: #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
// CHECK_PPC_GCC_ATOMICS: #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
//
// End PPC/GCC/Linux tests ------------------

// Begin Sparc/GCC/Linux tests ----------------
//
// RUN: %clang -E -dM %s -o - 2>&1 \
// RUN:     -target sparc-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SPARC
//
// CHECK_SPARC: #define __BIG_ENDIAN__ 1
// CHECK_SPARC: #define __sparc 1
// CHECK_SPARC: #define __sparc__ 1
// CHECK_SPARC: #define __sparcv8 1

//
// RUN: %clang -E -dM %s -o - 2>&1 \
// RUN:     -target sparcel-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SPARCEL
//
// CHECK_SPARCEL: #define __LITTLE_ENDIAN__ 1
// CHECK_SPARCEL: #define __sparc 1
// CHECK_SPARCEL: #define __sparc__ 1
// CHECK_SPARCEL: #define __sparcv8 1
//
// RUN: %clang -E -dM %s -o - 2>&1 \
// RUN:     -target sparcv9-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SPARCV9
//
// CHECK_SPARCV9: #define __BIG_ENDIAN__ 1
// CHECK_SPARCV9: #define __sparc 1
// CHECK_SPARCV9: #define __sparc64__ 1
// CHECK_SPARCV9: #define __sparc__ 1
// CHECK_SPARCV9: #define __sparc_v9__ 1
// CHECK_SPARCV9: #define __sparcv9 1
// CHECK_SPARCV9: #define __sparcv9__ 1

// Begin SystemZ/GCC/Linux tests ----------------
//
// RUN: %clang -march=z10 -E -dM %s -o - 2>&1 \
// RUN:     -target s390x-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SYSTEMZ_Z10
//
// CHECK_SYSTEMZ_Z10: #define __LONG_DOUBLE_128__ 1
// CHECK_SYSTEMZ_Z10: #define __s390__ 1
// CHECK_SYSTEMZ_Z10: #define __s390x__ 1
// CHECK_SYSTEMZ_Z10: #define __zarch__ 1
//
// RUN: %clang -march=zEC12 -E -dM %s -o - 2>&1 \
// RUN:     -target s390x-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SYSTEMZ_ZEC12
//
// CHECK_SYSTEMZ_ZEC12: #define __HTM__ 1
// CHECK_SYSTEMZ_ZEC12: #define __LONG_DOUBLE_128__ 1
// CHECK_SYSTEMZ_ZEC12: #define __s390__ 1
// CHECK_SYSTEMZ_ZEC12: #define __s390x__ 1
// CHECK_SYSTEMZ_ZEC12: #define __zarch__ 1
//
// RUN: %clang -mhtm -E -dM %s -o - 2>&1 \
// RUN:     -target s390x-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SYSTEMZ_HTM
//
// CHECK_SYSTEMZ_HTM: #define __HTM__ 1
//
// RUN: %clang -fzvector -E -dM %s -o - 2>&1 \
// RUN:     -target s390x-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SYSTEMZ_ZVECTOR
// RUN: %clang -mzvector -E -dM %s -o - 2>&1 \
// RUN:     -target s390x-unknown-linux \
// RUN:   | FileCheck %s -check-prefix=CHECK_SYSTEMZ_ZVECTOR
//
// CHECK_SYSTEMZ_ZVECTOR: #define __VEC__ 10301
