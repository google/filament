// RUN: %dxilver 1.2 | %dxc -enable-16bit-types -E test -T vs_6_2 %s | FileCheck %s

// CHECK: @test

// To test with the classic compiler, run
// fxc.exe /T ps_5_1 literals.hlsl

// without also putting them in a static assertion

// __decltype is the GCC way of saying 'decltype', but doesn't require C++11
#define VERIFY_FXC
#ifdef VERIFY_FXC
#define VERIFY_TYPES(typ, exp) {typ _tmp_var_ = exp;}
#else
#endif

float overload1(float v) { return (float)100; }             /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
int overload1(int v) { return (int)200; }                   /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
uint overload1(uint v) { return (uint)300; }                /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
bool overload1(bool v) { return (bool)400; }                /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
double overload1(double v) { return (double)500; }          /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
int64_t overload1(int64_t v) { return (int64_t)600; }                   /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
uint64_t overload1(uint64_t v) { return (uint64_t)700; }                /* expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} expected-note {{candidate function}} fxc-pass {{}} */
half overload1(half v) { return (half)800; }

float overload2(float v1, float v2) { return (float)100; }
int overload2(int v1, int v2) { return (int)200; }
uint overload2(uint v1, uint v2) { return (uint)300; }
bool overload2(bool v1, bool v2) { return (bool)400; }
double overload2(double v1, double v2) { return (double)500; }
int64_t overload2(int64_t v1, int64_t v2) { return (int64_t)600; }
uint64_t overload2(uint64_t v1, uint64_t v2) { return (uint64_t)700; }
half overload2(half v1, half v2) { return (half)800; }

min16float m16f;
min16float4x4 m16f4x4;

float test() :Foo {
  // Ambiguous due to literal int and literal float:
  // Single-digit literals take a special path:

  // ambiguous to fxc, since it ignores 'L' on literal int

  // Not ambiguous due to literal suffix:
  VERIFY_TYPES(float, overload1(1.5f));
  VERIFY_TYPES(float, overload1(1.5F));
  VERIFY_TYPES(double, overload1(1.5l));
  VERIFY_TYPES(double, overload1(1.5L));
  VERIFY_TYPES(uint, overload1(2u));
  VERIFY_TYPES(uint, overload1(2U));
  VERIFY_TYPES(uint, overload1(2UL));
  VERIFY_TYPES(uint64_t, overload1(2ull));
  VERIFY_TYPES(uint64_t, overload1(2ULL));
  VERIFY_TYPES(int64_t, overload1(2ll));
  VERIFY_TYPES(int64_t, overload1(2LL));
  VERIFY_TYPES(half, overload1(1.0h));
  VERIFY_TYPES(half, overload1(1.0H));

  
  // Not ambiguous due to one literal and one specific:
  VERIFY_TYPES(float, overload2(1.5, 1.5f));
  VERIFY_TYPES(float, overload2(1.5f, 1.5));
  VERIFY_TYPES(double, overload2(1.5, 1.5l));
  VERIFY_TYPES(double, overload2(1.5l, 1.5));
  VERIFY_TYPES(uint, overload2(2, 2u));
  VERIFY_TYPES(uint, overload2(2u, 2));
  VERIFY_TYPES(uint, overload2(2, 2ul));
  VERIFY_TYPES(uint, overload2(2ul, 2));
  VERIFY_TYPES(half, overload2(1.0h, 1.0));;
  VERIFY_TYPES(half, overload2(1.0, 1.0H));
  
  VERIFY_TYPES(uint64_t, overload2(2, 2ull));
  VERIFY_TYPES(uint64_t, overload2(2ull, 2));
  VERIFY_TYPES(uint64_t, overload2(2, 2ULL));
  VERIFY_TYPES(uint64_t, overload2(2ULL, 2));  
  
  VERIFY_TYPES(int64_t, overload2(2, 2ll));
  VERIFY_TYPES(int64_t, overload2(2ll, 2));
  VERIFY_TYPES(int64_t, overload2(2, 2LL));
  VERIFY_TYPES(int64_t, overload2(2LL, 2));

  // Verify that intrinsics will accept 64-bit overloads properly.
  VERIFY_TYPES(uint64_t, abs(100ULL));
  VERIFY_TYPES(uint, countbits(101ULL));
  VERIFY_TYPES(uint2, countbits(uint64_t2(104ULL, 105ULL)));
  VERIFY_TYPES(uint, firstbithigh(106ULL));
  VERIFY_TYPES(uint2, firstbithigh(uint64_t2(107ULL, 108ULL)));
  VERIFY_TYPES(uint, firstbitlow(109ULL));
  VERIFY_TYPES(uint2, firstbitlow(uint64_t2(110ULL, 111ULL)));
  VERIFY_TYPES(uint, reversebits(113u));
  VERIFY_TYPES(uint64_t, reversebits(114ULL));

  // fxc thinks these are ambiguous since it ignores the 'l' suffix:

  VERIFY_TYPES(double, overload1(2ULL * 1.5));
  VERIFY_TYPES(double, overload1(2LL * 1.5));

  // ensure operator combinations produce the right literal types and concrete types
  VERIFY_TYPES(float, 1.5 * 2 * 1.5f);
  VERIFY_TYPES(float, 1.5 * 2 * 1.5F);
  VERIFY_TYPES(double, 1.5 * 2 * 1.5l);
  VERIFY_TYPES(double, 1.5 * 2 * 1.5L);
  VERIFY_TYPES(int, 2 * 2L);
  VERIFY_TYPES(uint, 2 * 2U);
  VERIFY_TYPES(uint, 2 * 2UL);
  VERIFY_TYPES(int64_t, 2 * 2LL);
  VERIFY_TYPES(uint64_t, 2 * 2ULL);
  VERIFY_TYPES(half, 1.0 * 2 * 1.5h);
  VERIFY_TYPES(half, 1.0 * 2 * 1.5H);

  // Specific width int forces float to be specific-width
  VERIFY_TYPES(float, 1.5 * 2 * 2L);
  VERIFY_TYPES(float, 1.5 * 2 * 2U);
  VERIFY_TYPES(float, 1.5 * 2 * 2UL);
  VERIFY_TYPES(float, 1.5 * 2 * 2L);
  VERIFY_TYPES(float, m16f * (1.5 * 2 * 2L));

  // Combination of literal float + literal int to literal float, then combined with
  // min-precision float matrix to produce min-precision float matrix - ensures that
  // literal combination results in literal that doesn't override the min-precision type.
  VERIFY_TYPES(min16float4x4, m16f4x4 * (0.5 + 1));

  return 0.0f;
}
