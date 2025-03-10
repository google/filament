// RUN: %dxc -Tlib_6_3  -Wno-unused-value  -verify %s

// we use -Wno-unused-value because we generate some no-op expressions to yield errors
// without also putting them in a static assertion

// __decltype is the GCC way of saying 'decltype', but doesn't require C++11
// _Static_assert is the C11 way of saying 'static_assert', but doesn't require C++11
#ifdef VERIFY_FXC
#define _Static_assert(a,b,c) ;
#endif

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T vs_5_1 scalar-operators.hlsl
// with vs_2_0 (the default) min16float usage produces a complaint that it's not supported

struct f3_s    { float3 f3; };
struct mixed_s { float3 f3; SamplerState s; };
SamplerState g_SamplerState;
f3_s    g_f3_s;
mixed_s g_mixed_s;

float4 plain(float4 param4 /* : FOO */) /*: FOO */{
    bool        bools       = 0;
    int         ints        = 0;
    float       floats      = 0;
    SamplerState SamplerStates = g_SamplerState;
    f3_s f3_ss = g_f3_s;
    mixed_s mixed_ss = g_mixed_s;

  // NOTE: errors that were formerly 'type mismatch' or 'compilation aborted unexpectedly'
  // when dealing with built-in types, (SamplerState in these examples) are now 'operator
  // cannot be used with built-in type 'SamplerState'' or

  // some 'int or unsigned int type required' become 'scalar, vector, or matrix expected'

  // some 'cannot implicitly convert from 'SamplerState' to 'bool'' become 'type mismatch'

  (bools + SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools + f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools + mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints + SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints + f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints + mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats + SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats + f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats + mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates + bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates + ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates + floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates + SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates + f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates + mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss + bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss + ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss + floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss + SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss + f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss + mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss + bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss + ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss + floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss + SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss + f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss + mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools - SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools - f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools - mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints - SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints - f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints - mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats - SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats - f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats - mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates - bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates - ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates - floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates - SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates - f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates - mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss - bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss - ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss - floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss - SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss - f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss - mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss - bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss - ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss - floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss - SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss - f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss - mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools / SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools / f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools / mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints / SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints / f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints / mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats / SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats / f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats / mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates / bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates / ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates / floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates / SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates / f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates / mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss / bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss / ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss / floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss / SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss / f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss / mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss / bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss / ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss / floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss / SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss / f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss / mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools % SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools % f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools % mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints % SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints % f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints % mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats % SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats % f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats % mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates % bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates % ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates % floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates % SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates % f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates % mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss % bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss % ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss % floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss % SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss % f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss % mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss % bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss % ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss % floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss % SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss % f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss % mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools < SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools < f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools < mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints < SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints < f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints < mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats < SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats < f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats < mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates < bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates < ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates < floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates < SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates < f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates < mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss < bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss < ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss < floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss < SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss < f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss < mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss < bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss < ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss < floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss < SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss < f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss < mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools <= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools <= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools <= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints <= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints <= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints <= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats <= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats <= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats <= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates <= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates <= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates <= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates <= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates <= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates <= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss <= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss <= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss <= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss <= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss <= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss <= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss <= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss <= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss <= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss <= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss <= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss <= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools > SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools > f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools > mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints > SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints > f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints > mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats > SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats > f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats > mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates > bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates > ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates > floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates > SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates > f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates > mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss > bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss > ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss > floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss > SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss > f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss > mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss > bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss > ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss > floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss > SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss > f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss > mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools >= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools >= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools >= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints >= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints >= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints >= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats >= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats >= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats >= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates >= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates >= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates >= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates >= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates >= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates >= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss >= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss >= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss >= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss >= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss >= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss >= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss >= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss >= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss >= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss >= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss >= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss >= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools == SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (bools == f3_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (bools == mixed_ss); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (ints == SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (ints == f3_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (ints == mixed_ss); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (floats == SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (floats == f3_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (floats == mixed_ss); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (SamplerStates == bools); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (SamplerStates == ints); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (SamplerStates == floats); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (SamplerStates == SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-pass {{}}
  (SamplerStates == f3_ss); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (SamplerStates == mixed_ss); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss == bools); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss == ints); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss == floats); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss == SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss == f3_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss == mixed_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss == bools); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss == ints); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss == floats); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss == SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss == f3_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss == mixed_ss); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (bools != SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (bools != f3_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (bools != mixed_ss); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (ints != SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (ints != f3_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (ints != mixed_ss); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (floats != SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (floats != f3_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (floats != mixed_ss); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (SamplerStates != bools); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (SamplerStates != ints); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (SamplerStates != floats); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-error {{X3020: type mismatch}}
  (SamplerStates != SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} fxc-pass {{}}
  (SamplerStates != f3_ss); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (SamplerStates != mixed_ss); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss != bools); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss != ints); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss != floats); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss != SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss != f3_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} fxc-error {{X3020: type mismatch}}
  (f3_ss != mixed_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss != bools); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss != ints); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss != floats); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss != SamplerStates); // expected-error {{operator cannot be used with built-in type 'SamplerState'}} expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss != f3_ss); // expected-error {{operator cannot be used with user-defined type 'f3_s'}} expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (mixed_ss != mixed_ss); // expected-error {{operator cannot be used with user-defined type 'mixed_s'}} fxc-error {{X3020: type mismatch}}
  (bools << SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools << f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools << mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints << SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints << f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints << mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (floats << SamplerStates); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats << f3_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats << mixed_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates << bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates << ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates << floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates << SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates << f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates << mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss << bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss << ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss << floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss << SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss << f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss << mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss << bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss << ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss << floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss << SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss << f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss << mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools >> SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools >> f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools >> mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints >> SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints >> f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints >> mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (floats >> SamplerStates); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats >> f3_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats >> mixed_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >> bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >> ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >> floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >> SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >> f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >> mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >> bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >> ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >> floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >> SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >> f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >> mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >> bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >> ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >> floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >> SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >> f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >> mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools & SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools & f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools & mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints & SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints & f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints & mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (floats & SamplerStates); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats & f3_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats & mixed_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates & bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates & ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates & floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates & SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates & f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates & mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss & bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss & ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss & floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss & SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss & f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss & mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss & bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss & ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss & floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss & SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss & f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss & mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools | SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools | f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools | mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints | SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints | f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints | mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (floats | SamplerStates); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats | f3_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats | mixed_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates | bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates | ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates | floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates | SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates | f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates | mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss | bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss | ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss | floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss | SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss | f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss | mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss | bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss | ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss | floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss | SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss | f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss | mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools ^ SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools ^ f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools ^ mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints ^ SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints ^ f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints ^ mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (floats ^ SamplerStates); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats ^ f3_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats ^ mixed_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^ bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^ ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^ floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^ SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^ f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^ mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^ bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^ ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^ floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^ SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^ f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^ mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^ bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^ ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^ floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^ SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^ f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^ mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools && SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools && f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools && mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints && SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints && f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints && mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats && SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats && f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats && mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates && bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates && ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates && floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates && SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates && f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates && mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss && bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss && ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss && floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss && SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss && f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss && mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss && bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss && ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss && floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss && SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss && f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss && mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools || SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools || f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools || mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints || SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints || f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints || mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats || SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats || f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats || mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates || bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates || ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates || floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates || SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates || f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates || mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss || bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss || ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss || floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss || SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss || f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss || mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss || bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss || ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss || floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss || SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss || f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss || mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools = SamplerStates); // expected-error {{cannot convert from 'SamplerState' to 'bool'}} fxc-error {{X3017: cannot implicitly convert from 'SamplerState' to 'bool'}}
  (bools = f3_ss); // expected-error {{cannot implicitly convert from 'f3_s' to 'bool'}} fxc-error {{X3017: cannot convert from 'struct f3_s' to 'bool'}}
  (bools = mixed_ss); // expected-error {{cannot implicitly convert from 'mixed_s' to 'bool'}} fxc-error {{X3017: cannot convert from 'struct mixed_s' to 'bool'}}
  (ints = SamplerStates); // expected-error {{cannot convert from 'SamplerState' to 'int'}} fxc-error {{X3017: cannot implicitly convert from 'SamplerState' to 'int'}}
  (ints = f3_ss); // expected-error {{cannot implicitly convert from 'f3_s' to 'int'}} fxc-error {{X3017: cannot convert from 'struct f3_s' to 'int'}}
  (ints = mixed_ss); // expected-error {{cannot implicitly convert from 'mixed_s' to 'int'}} fxc-error {{X3017: cannot convert from 'struct mixed_s' to 'int'}}
  (floats = SamplerStates); // expected-error {{cannot convert from 'SamplerState' to 'float'}} fxc-error {{X3017: cannot implicitly convert from 'SamplerState' to 'float'}}
  (floats = f3_ss); // expected-error {{cannot implicitly convert from 'f3_s' to 'float'}} fxc-error {{X3017: cannot convert from 'struct f3_s' to 'float'}}
  (floats = mixed_ss); // expected-error {{cannot implicitly convert from 'mixed_s' to 'float'}} fxc-error {{X3017: cannot convert from 'struct mixed_s' to 'float'}}
  (SamplerStates = bools); // expected-error {{cannot convert from 'bool' to 'SamplerState'}} fxc-error {{X3017: cannot implicitly convert from 'bool' to 'SamplerState'}}
  (SamplerStates = ints); // expected-error {{cannot convert from 'int' to 'SamplerState'}} fxc-error {{X3017: cannot implicitly convert from 'int' to 'SamplerState'}}
  (SamplerStates = floats); // expected-error {{cannot convert from 'float' to 'SamplerState'}} fxc-error {{X3017: cannot implicitly convert from 'float' to 'SamplerState'}}
  // This confuses the semantic checks on template types, which are assumed to be only for built-in template-like constructs.
  // _Static_assert(std::is_same<SamplerState, __decltype(SamplerStates = SamplerStates)>::value, "");
  (SamplerStates = f3_ss); // expected-error {{cannot convert from 'f3_s' to 'SamplerState'}} fxc-error {{X3017: cannot implicitly convert from 'struct f3_s' to 'SamplerState'}}
  (SamplerStates = mixed_ss); // expected-error {{cannot convert from 'mixed_s' to 'SamplerState'}} fxc-error {{X3017: cannot implicitly convert from 'struct mixed_s' to 'SamplerState'}}
  (f3_ss = bools); // expected-error {{cannot implicitly convert from 'bool' to 'f3_s'}} fxc-error {{X3017: cannot convert from 'bool' to 'struct f3_s'}}
  (f3_ss = ints); // expected-error {{cannot implicitly convert from 'int' to 'f3_s'}} fxc-error {{X3017: cannot convert from 'int' to 'struct f3_s'}}
  (f3_ss = floats); // expected-error {{cannot implicitly convert from 'float' to 'f3_s'}} fxc-error {{X3017: cannot convert from 'float' to 'struct f3_s'}}
  (f3_ss = SamplerStates); // expected-error {{cannot convert from 'SamplerState' to 'f3_s'}} fxc-error {{X3017: cannot implicitly convert from 'SamplerState' to 'struct f3_s'}}
  _Static_assert(std::is_same<f3_s, __decltype(f3_ss = f3_ss)>::value, "");
  (f3_ss = mixed_ss); // expected-error {{cannot implicitly convert from 'mixed_s' to 'f3_s'}} fxc-error {{X3017: cannot convert from 'struct mixed_s' to 'struct f3_s'}}
  (mixed_ss = bools); // expected-error {{cannot implicitly convert from 'bool' to 'mixed_s'}} fxc-error {{X3017: cannot implicitly convert from 'bool' to 'struct mixed_s'}}
  (mixed_ss = ints); // expected-error {{cannot implicitly convert from 'int' to 'mixed_s'}} fxc-error {{X3017: cannot implicitly convert from 'int' to 'struct mixed_s'}}
  (mixed_ss = floats); // expected-error {{cannot implicitly convert from 'float' to 'mixed_s'}} fxc-error {{X3017: cannot implicitly convert from 'float' to 'struct mixed_s'}}
  (mixed_ss = SamplerStates); // expected-error {{cannot convert from 'SamplerState' to 'mixed_s'}} fxc-error {{X3017: cannot implicitly convert from 'SamplerState' to 'struct mixed_s'}}
  (mixed_ss = f3_ss); // expected-error {{cannot convert from 'f3_s' to 'mixed_s'}} fxc-error {{X3017: cannot implicitly convert from 'struct f3_s' to 'struct mixed_s'}}
  // This confuses the semantic checks on template types, which are assumed to be only for built-in template-like constructs.
  // _Static_assert(std::is_same<mixed_s, __decltype(mixed_ss = mixed_ss)>::value, "");
  (bools += SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools += f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools += mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints += SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints += f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints += mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats += SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats += f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats += mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates += bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates += ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates += floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates += SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates += f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates += mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss += bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss += ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss += floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss += SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss += f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss += mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss += bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss += ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss += floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss += SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss += f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss += mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools -= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools -= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools -= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints -= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints -= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints -= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats -= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats -= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats -= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates -= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates -= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates -= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates -= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates -= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates -= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss -= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss -= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss -= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss -= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss -= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss -= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss -= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss -= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss -= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss -= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss -= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss -= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools /= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools /= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools /= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints /= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints /= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints /= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats /= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats /= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats /= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates /= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates /= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates /= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates /= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates /= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates /= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss /= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss /= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss /= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss /= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss /= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss /= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss /= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss /= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss /= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss /= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss /= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss /= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools %= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools %= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools %= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints %= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints %= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (ints %= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats %= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats %= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (floats %= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates %= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates %= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates %= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates %= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates %= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (SamplerStates %= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss %= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss %= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss %= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss %= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss %= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (f3_ss %= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss %= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss %= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss %= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss %= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss %= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (mixed_ss %= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3022: scalar, vector, or matrix expected}}
  (bools <<= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools <<= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools <<= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints <<= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints <<= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints <<= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (floats <<= SamplerStates); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats <<= f3_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats <<= mixed_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates <<= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates <<= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates <<= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates <<= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates <<= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates <<= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss <<= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss <<= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss <<= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss <<= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss <<= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss <<= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss <<= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss <<= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss <<= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss <<= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss <<= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss <<= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools >>= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools >>= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools >>= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints >>= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints >>= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints >>= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (floats >>= SamplerStates); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats >>= f3_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats >>= mixed_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >>= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >>= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >>= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >>= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >>= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates >>= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >>= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >>= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >>= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >>= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >>= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss >>= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >>= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >>= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >>= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >>= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >>= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss >>= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools &= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools &= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools &= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints &= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints &= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints &= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (floats &= SamplerStates); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats &= f3_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats &= mixed_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates &= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates &= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates &= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates &= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates &= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates &= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss &= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss &= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss &= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss &= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss &= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss &= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss &= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss &= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss &= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss &= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss &= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss &= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools |= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools |= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools |= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints |= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints |= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints |= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (floats |= SamplerStates); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats |= f3_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats |= mixed_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates |= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates |= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates |= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates |= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates |= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates |= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss |= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss |= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss |= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss |= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss |= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss |= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss |= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss |= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss |= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss |= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss |= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss |= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools ^= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools ^= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (bools ^= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints ^= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints ^= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (ints ^= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (floats ^= SamplerStates); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats ^= f3_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (floats ^= mixed_ss); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (SamplerStates ^= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (f3_ss ^= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^= bools); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^= ints); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^= floats); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^= SamplerStates); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^= f3_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}
  (mixed_ss ^= mixed_ss); // expected-error {{scalar, vector, or matrix expected}} fxc-error {{X3082: int or unsigned int type required}}

};
