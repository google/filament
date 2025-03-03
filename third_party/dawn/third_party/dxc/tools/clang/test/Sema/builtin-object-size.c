// RUN: %clang_cc1 -fsyntax-only -verify %s
// RUN: %clang_cc1 -fsyntax-only -triple x86_64-apple-darwin9 -verify %s

int a[10];

int f0() {
  return __builtin_object_size(&a); // expected-error {{too few arguments to function}}
}
int f1() {
  return (__builtin_object_size(&a, 0) + 
          __builtin_object_size(&a, 1) + 
          __builtin_object_size(&a, 2) + 
          __builtin_object_size(&a, 3));
}
int f2() {
  return __builtin_object_size(&a, -1); // expected-error {{argument should be a value from 0 to 3}}
}
int f3() {
  return __builtin_object_size(&a, 4); // expected-error {{argument should be a value from 0 to 3}}
}


// rdar://6252231 - cannot call vsnprintf with va_list on x86_64
void f4(const char *fmt, ...) {
 __builtin_va_list args;
 __builtin___vsnprintf_chk (0, 42, 0, 11, fmt, args); // expected-warning {{'__builtin___vsnprintf_chk' will always overflow destination buffer}}
}

// rdar://18334276
typedef __typeof__(sizeof(int)) size_t;
void * memcset(void *restrict dst, int src, size_t n);
void * memcpy(void *restrict dst, const void *restrict src, size_t n);

#define memset(dest, src, len) __builtin___memset_chk(dest, src, len, __builtin_object_size(dest, 0))
#define memcpy(dest, src, len) __builtin___memcpy_chk(dest, src, len, __builtin_object_size(dest, 0))
#define memcpy1(dest, src, len) __builtin___memcpy_chk(dest, src, len, __builtin_object_size(dest, 4))
#define NULL ((void *)0)

void f5(void)
{
  char buf[10];
  memset((void *)0x100000000ULL, 0, 0x1000);
  memcpy((char *)NULL + 0x10000, buf, 0x10);
  memcpy1((char *)NULL + 0x10000, buf, 0x10); // expected-error {{argument should be a value from 0 to 3}}
}

// rdar://18431336
void f6(void)
{
  char b[5];
  char buf[10];
  __builtin___memccpy_chk (buf, b, '\0', sizeof(b), __builtin_object_size (buf, 0));
  __builtin___memccpy_chk (b, buf, '\0', sizeof(buf), __builtin_object_size (b, 0));  // expected-warning {{'__builtin___memccpy_chk' will always overflow destination buffer}}
}
