// RUN: %clang_cc1 -std=c89 -fsyntax-only -x c -E -dM %s | FileCheck --check-prefix=GNU-INLINE %s
// RUN: %clang_cc1 -std=c99 -fsyntax-only -x c -E -dM %s | FileCheck --check-prefix=STDC-INLINE %s
// RUN: %clang_cc1 -std=c99 -fgnu89-inline -fsyntax-only -x c -E -dM %s | FileCheck --check-prefix=GNU-INLINE %s
// RUN: %clang_cc1 -fsyntax-only -x c++ -E -dM %s | FileCheck --check-prefix=GNU-INLINE %s
// RUN: not %clang_cc1 -fgnu89-inline -fsyntax-only -x c++ %s 2>&1 | FileCheck --check-prefix=CXX %s

// CXX: '-fgnu89-inline' not allowed with 'C++/ObjC++'

// STDC-INLINE-NOT: __GNUC_GNU_INLINE__
// STDC-INLINE: #define __GNUC_STDC_INLINE__ 1
// STDC-INLINE-NOT: __GNUC_GNU_INLINE__

// GNU-INLINE-NOT: __GNUC_STDC_INLINE__
// GNU-INLINE: #define __GNUC_GNU_INLINE__ 1
// GNU-INLINE-NOT: __GNUC_STDC_INLINE__
