// RUN: %dxc -Tlib_6_3 -Wno-unused-value -verify %s

// we use -Wno-unused-value because we generate some no-op expressions to yield errors
// without also putting them in a static assertion

// __decltype is the GCC way of saying 'decltype', but doesn't require C++11
// _Static_assert is the C11 way of saying 'static_assert', but doesn't require C++11
#ifdef VERIFY_FXC
#define _Static_assert(a,b,c) ;
#endif

// :FXC_VERIFY_ARGUMENTS: /E plain /T vs_5_1

float4 plain(float4 param4 : FOO) : FOO {
    bool        bools       = 0;
    int         ints        = 0;
    uint        uints       = 0;
    dword       dwords      = 0;
    half        halfs       = 0;
    float       floats      = 0;
    double      doubles     = 0;
    min16float  min16floats = 0;
    min10float  min10floats = 0; // expected-warning {{'min10float' is promoted to 'min16float'}} fxc-pass {{}}
    min16int    min16ints   = 0;
    min12int    min12ints   = 0; // expected-warning {{'min12int' is promoted to 'min16int'}} fxc-pass {{}}
    min16uint   min16uints  = 0;

    // _Static_assert(std::is_same<bool, bool>::value, "bool, bool failed");
    _Static_assert(std::is_same<bool, __decltype(bools)>::value, "bool, __decltype(bools) failed");
    _Static_assert(std::is_same<bool, __decltype(bools < bools)>::value, "bool, __decltype(bools < bools) failed");


    // float result = ints + floats;
    _Static_assert(std::is_same<min16uint, __decltype(min16ints  + min16uints)>::value, "");

    // Promotion cases with addition.
    // Two unsigned types will widen to widest type.
    _Static_assert(std::is_same<uint, __decltype(uints      + min16uints)>::value, "");
    _Static_assert(std::is_same<uint, __decltype(min16uints + uints     )>::value, "");

    // Two signed types will widen to widest type, but remain minprecision if either is.
    _Static_assert(std::is_same<int, __decltype(ints      + min12ints)>::value, "");
    _Static_assert(std::is_same<int, __decltype(min12ints + ints     )>::value, "");

    // Mixed signed-unsigned will widen to largest unsigned.
    _Static_assert(std::is_same<uint, __decltype(ints  + uints)>::value, "");
    _Static_assert(std::is_same<uint, __decltype(uints + ints )>::value, "");
    _Static_assert(std::is_same<min16uint, __decltype(min16ints  + min16uints)>::value, "");
    _Static_assert(std::is_same<min16uint, __decltype(min16uints + min16ints )>::value, "");

    // Mixed integral/floating point will turn to floating-point.
    _Static_assert(std::is_same<float, __decltype(ints    + floats)>::value, "");
    _Static_assert(std::is_same<float, __decltype(uints   + floats)>::value, "");
    _Static_assert(std::is_same<double, __decltype(doubles + ints)>::value, "");
    _Static_assert(std::is_same<double, __decltype(uints   + doubles)>::value, "");

    // For two floating-point types, they will widen to the largest one.
    _Static_assert(std::is_same<double, __decltype(doubles + floats)>::value, "");
    _Static_assert(std::is_same<double, __decltype(floats  + doubles)>::value, "");
    _Static_assert(std::is_same<double, __decltype(doubles     + min16floats)>::value, "");
    _Static_assert(std::is_same<double, __decltype(min16floats + doubles)>::value, "");
    _Static_assert(std::is_same<double, __decltype(doubles     + min10floats)>::value, "");
    _Static_assert(std::is_same<double, __decltype(min10floats + doubles)>::value, "");
    _Static_assert(std::is_same<float , __decltype(floats      + min16floats)>::value, "");
    _Static_assert(std::is_same<float , __decltype(min16floats + floats)>::value, "");
    _Static_assert(std::is_same<float , __decltype(floats      + min10floats)>::value, "");
    _Static_assert(std::is_same<float , __decltype(min10floats + floats)>::value, "");

  _Static_assert(std::is_same<int, __decltype(bools + bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools + ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(bools + uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(bools + halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(bools + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(bools + doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(bools + min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(bools + min10floats)>::value, ""); // fxc-pass {{}}
  _Static_assert(std::is_same<min16int, __decltype(bools + min16ints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(bools + min12ints)>::value, "");     /* fxc-pass {{}} */
  _Static_assert(std::is_same<min16uint, __decltype(bools + min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints + bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints + ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints + uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(ints + halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(ints + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(ints + doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(ints + min16floats)>::value, "");  /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min10float, __decltype(ints + min10floats)>::value, "");  // expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<int, __decltype(ints + min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints + min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints + min16uints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints + bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints + ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints + uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(uints + halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(uints + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(uints + doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(uints + min16floats)>::value, ""); /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min10float, __decltype(uints + min10floats)>::value, "");  // expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<uint, __decltype(uints + min16ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints + min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints + min16uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(halfs + bools)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs + ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs + uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(halfs + halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(halfs + doubles)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs + min16floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs + min10floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs + min16ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs + min12ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs + min16uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats + bools)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats + ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats + uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats + halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(floats + doubles)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats + min16floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats + min10floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats + min16ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats + min12ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats + min16uints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + bools)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + ints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + uints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + halfs)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + doubles)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + min16floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + min10floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + min16ints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + min12ints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles + min16uints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats + bools)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats + ints)>::value, "");   /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min16float, __decltype(min16floats + uints)>::value, "");  /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<float, __decltype(min16floats + halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16floats + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min16floats + doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats + min16floats)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats + min10floats)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats + min16ints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats + min12ints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats + min16uints)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min10floats + bools)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats + ints)>::value, "");   // expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats + uints)>::value, "");  // expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<float, __decltype(min10floats + halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min10floats + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min10floats + doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min10floats + min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min10floats + min10floats)>::value, ""); // fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats + min16ints)>::value, "");  // expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats + min12ints)>::value, "");  // expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats + min16uints)>::value, ""); // expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints + bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(min16ints + ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16ints + uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16ints + halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16ints + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min16ints + doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16ints + min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min16ints + min10floats)>::value, "");  // expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints + min16ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints + min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16ints + min16uints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints + bools)>::value, "");          /* fxc-pass {{}} */
  _Static_assert(std::is_same<int, __decltype(min12ints + ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min12ints + uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min12ints + halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min12ints + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min12ints + doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min12ints + min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min12ints + min10floats)>::value, "");  // expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min16int, __decltype(min12ints + min16ints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints + min12ints)>::value, "");    // fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min12ints + min16uints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints + bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints + ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints + uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16uints + halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16uints + floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min16uints + doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16uints + min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min16uints + min10floats)>::value, "");   // expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints + min16ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints + min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints + min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools - bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools - ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(bools - uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(bools - halfs)>::value, "");                   /* expected-error {{static_assert failed ""}} fxc-pass {{}} */
  _Static_assert(std::is_same<float, __decltype(bools - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(bools - doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(bools - min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(bools - min10floats)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min16int, __decltype(bools - min16ints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(bools - min12ints)>::value, "");      /* fxc-pass {{}} */
  _Static_assert(std::is_same<min16uint, __decltype(bools - min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints - bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints - ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints - uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(ints - halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(ints - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(ints - doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(ints - min16floats)>::value, "");   /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min10float, __decltype(ints - min10floats)>::value, "");  // expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<int, __decltype(ints - min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints - min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints - min16uints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints - bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints - ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints - uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(uints - halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(uints - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(uints - doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(uints - min16floats)>::value, ""); /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min10float, __decltype(uints - min10floats)>::value, "");  // expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<uint, __decltype(uints - min16ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints - min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints - min16uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(halfs - bools)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs - ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs - uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(halfs - halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(halfs - doubles)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs - min16floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs - min10floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs - min16ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs - min12ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs - min16uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats - bools)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats - ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats - uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats - halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(floats - doubles)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats - min16floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats - min10floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats - min16ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats - min12ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats - min16uints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - bools)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - ints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - uints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - halfs)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - doubles)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - min16floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - min10floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - min16ints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - min12ints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles - min16uints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats - bools)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats - ints)>::value, "");   /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min16float, __decltype(min16floats - uints)>::value, "");  /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<float, __decltype(min16floats - halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16floats - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min16floats - doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats - min16floats)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats - min10floats)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats - min16ints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats - min12ints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats - min16uints)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min10floats - bools)>::value, "");   // fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats - ints)>::value, "");    // expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats - uints)>::value, "");   // expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<float, __decltype(min10floats - halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min10floats - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min10floats - doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min10floats - min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min10floats - min10floats)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats - min16ints)>::value, "");    // expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats - min12ints)>::value, "");    // expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats - min16uints)>::value, "");   // expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints - bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(min16ints - ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16ints - uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16ints - halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16ints - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min16ints - doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16ints - min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min16ints - min10floats)>::value, "");   // expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints - min16ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints - min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16ints - min16uints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints - bools)>::value, "");           /* fxc-pass {{}} */
  _Static_assert(std::is_same<int, __decltype(min12ints - ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min12ints - uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min12ints - halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min12ints - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min12ints - doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min12ints - min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min12ints - min10floats)>::value, "");   // expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min16int, __decltype(min12ints - min16ints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints - min12ints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min12ints - min16uints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints - bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints - ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints - uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16uints - halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16uints - floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min16uints - doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16uints - min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min16uints - min10floats)>::value, "");   // expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints - min16ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints - min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints - min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools / bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools / ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(bools / uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(bools / halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(bools / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(bools / doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(bools / min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(bools / min10floats)>::value, "");   // fxc-pass {{}}
  _Static_assert(std::is_same<min16int, __decltype(bools / min16ints)>::value, "");       /* expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-pass {{}} */
  _Static_assert(std::is_same<min12int, __decltype(bools / min12ints)>::value, "");       /* expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-pass {{}} */
  _Static_assert(std::is_same<min16uint, __decltype(bools / min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints / bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints / ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints / uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(ints / halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(ints / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(ints / doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(ints / min16floats)>::value, "");    /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min10float, __decltype(ints / min10floats)>::value, "");   // expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<int, __decltype(ints / min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints / min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints / min16uints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints / bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints / ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints / uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(uints / halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(uints / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(uints / doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(uints / min16floats)>::value, "");  /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min10float, __decltype(uints / min10floats)>::value, "");  // expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<uint, __decltype(uints / min16ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints / min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints / min16uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(halfs / bools)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs / ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs / uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(halfs / halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(halfs / doubles)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs / min16floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs / min10floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs / min16ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs / min12ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs / min16uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats / bools)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats / ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats / uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats / halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(floats / doubles)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats / min16floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats / min10floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats / min16ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats / min12ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats / min16uints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / bools)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / ints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / uints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / halfs)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / doubles)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / min16floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / min10floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / min16ints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / min12ints)>::value, "");
  _Static_assert(std::is_same<double, __decltype(doubles / min16uints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats / bools)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats / ints)>::value, "");   /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min16float, __decltype(min16floats / uints)>::value, "");  /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<float, __decltype(min16floats / halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16floats / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min16floats / doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats / min16floats)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats / min10floats)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats / min16ints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats / min12ints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats / min16uints)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min10floats / bools)>::value, "");   // fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats / ints)>::value, "");    // expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats / uints)>::value, "");   // expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<float, __decltype(min10floats / halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min10floats / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min10floats / doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min10floats / min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min10floats / min10floats)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats / min16ints)>::value, "");    // expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats / min12ints)>::value, "");    // expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats / min16uints)>::value, "");   // expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  // Note: binary operator promotes bool to int early, so this is an int division, which is legal.
  // TODO: double check fxc behavior for this, since it should be similar
  ints = (min16ints / bools); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints / bools)>::value, "");    // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-pass {{}}
  _Static_assert(std::is_same<int, __decltype(min16ints / ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16ints / uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16ints / halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16ints / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min16ints / doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16ints / min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min16ints / min10floats)>::value, "");  // expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  min16ints = (min16ints / min16ints); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  min16ints = (min16ints / min12ints); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  _Static_assert(std::is_same<min16uint, __decltype(min16ints / min16uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  // Note: binary operator promotes bool to int early, so this is an int division, which is legal.
  // TODO: double check fxc behavior for this, since it should be similar
  ints = (min12ints / bools); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints / bools)>::value, "");    // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-pass {{}}
  _Static_assert(std::is_same<int, __decltype(min12ints / ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min12ints / uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min12ints / halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min12ints / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min12ints / doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min12ints / min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min12ints / min10floats)>::value, "");  // expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  min12ints = (min12ints / min16ints); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} expected-warning {{conversion from larger type 'min16int' to smaller type 'min12int', possible loss of data}} fxc-error {{X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  min12ints = (min12ints / min12ints); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer division is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  _Static_assert(std::is_same<min16uint, __decltype(min12ints / min16uints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints / bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints / ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints / uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16uints / halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16uints / floats)>::value, "");
  _Static_assert(std::is_same<double, __decltype(min16uints / doubles)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16uints / min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min16uints / min10floats)>::value, "");  // expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints / min16ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints / min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints / min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools % bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools % ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(bools % uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(bools % halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(bools % floats)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  bools = (bools % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  _Static_assert(std::is_same<min16float, __decltype(bools % min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(bools % min10floats)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min16int, __decltype(bools % min16ints)>::value, "");      /* expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-pass {{}} */
  _Static_assert(std::is_same<min12int, __decltype(bools % min12ints)>::value, "");      /* expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-pass {{}} */
  _Static_assert(std::is_same<min16uint, __decltype(bools % min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints % bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints % ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints % uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(ints % halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(ints % floats)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  ints = (ints % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} expected-warning {{conversion from larger type 'double' to smaller type 'int', possible loss of data}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
  _Static_assert(std::is_same<min16float, __decltype(ints % min16floats)>::value, "");    /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min10float, __decltype(ints % min10floats)>::value, "");  // expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<int, __decltype(ints % min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints % min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints % min16uints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints % bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints % ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints % uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(uints % halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(uints % floats)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  uints = (uints % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} expected-warning {{conversion from larger type 'double' to smaller type 'uint', possible loss of data}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
  _Static_assert(std::is_same<min16float, __decltype(uints % min16floats)>::value, "");    /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min10float, __decltype(uints % min10floats)>::value, "");  // expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<uint, __decltype(uints % min16ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints % min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints % min16uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(halfs % bools)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs % ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs % uints)>::value, "");
  _Static_assert(std::is_same<half, __decltype(halfs % halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs % floats)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  halfs = (halfs % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} expected-warning {{conversion from larger type 'double' to smaller type 'half', possible loss of data}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
  _Static_assert(std::is_same<float, __decltype(halfs % min16floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs % min10floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs % min16ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs % min12ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(halfs % min16uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats % bools)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats % ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats % uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats % halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats % floats)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  floats = (floats % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} expected-warning {{conversion from larger type 'double' to smaller type 'float', possible loss of data}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
  _Static_assert(std::is_same<float, __decltype(floats % min16floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats % min10floats)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats % min16ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats % min12ints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(floats % min16uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % bools); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % ints); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % uints); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % halfs); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % floats); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % min16floats); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % min10floats); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % min16ints); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % min12ints); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  doubles = (doubles % min16uints); // expected-error {{modulo cannot be used with doubles, cast to float first}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}}
  _Static_assert(std::is_same<min16float, __decltype(min16floats % bools)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats % ints)>::value, "");    /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<min16float, __decltype(min16floats % uints)>::value, "");    /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<float, __decltype(min16floats % halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16floats % floats)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  min16floats = (min16floats % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} expected-warning {{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
  _Static_assert(std::is_same<min16float, __decltype(min16floats % min16floats)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats % min10floats)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats % min16ints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats % min12ints)>::value, "");
  _Static_assert(std::is_same<min16float, __decltype(min16floats % min16uints)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min10floats % bools)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats % ints)>::value, "");  // expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats % uints)>::value, "");  // expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<float, __decltype(min10floats % halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min10floats % floats)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  min10floats = (min10floats % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} expected-warning {{conversion from larger type 'double' to smaller type 'min10float', possible loss of data}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
  _Static_assert(std::is_same<min16float, __decltype(min10floats % min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min10floats % min10floats)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats % min16ints)>::value, "");  // expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats % min12ints)>::value, "");  // expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min10float, __decltype(min10floats % min16uints)>::value, "");  // expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  // Note: binary operator promotes bool to int early, so this is an int division, which is legal.
  // TODO: double check fxc behavior for this, since it should be similar
  ints = (min16ints % bools); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints % bools)>::value, "");    // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-pass {{}}
  _Static_assert(std::is_same<int, __decltype(min16ints % ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16ints % uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16ints % halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16ints % floats)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  min16ints = (min16ints % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} expected-warning {{conversion from larger type 'double' to smaller type 'min16int', possible loss of data}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
  _Static_assert(std::is_same<min16float, __decltype(min16ints % min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min16ints % min10floats)>::value, "");  // expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  min16ints = (min16ints % min16ints); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  min16ints = (min16ints % min12ints); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  _Static_assert(std::is_same<min16uint, __decltype(min16ints % min16uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  // Note: binary operator promotes bool to int early, so this is an int division, which is legal.
  // TODO: double check fxc behavior for this, since it should be similar
  ints = (min12ints % bools); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints % bools)>::value, "");    // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-pass {{}}
  _Static_assert(std::is_same<int, __decltype(min12ints % ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min12ints % uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min12ints % halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min12ints % floats)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  min16ints = (min12ints % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} expected-warning {{conversion from larger type 'double' to smaller type 'min16int', possible loss of data}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
  _Static_assert(std::is_same<min16float, __decltype(min12ints % min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min12ints % min10floats)>::value, "");  // expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  min16ints = (min12ints % min16ints); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.;compilation failed; no code produced;
  min16ints = (min12ints % min12ints); // expected-error {{signed integer division is not supported on minimum-precision types, cast to int to use 32-bit division}} fxc-error {{X3706: signed integer remainder is not supported on minimum-precision types. Cast to int to use 32-bit division.}}
  _Static_assert(std::is_same<min16uint, __decltype(min12ints % min16uints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints % bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints % ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints % uints)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16uints % halfs)>::value, "");
  _Static_assert(std::is_same<float, __decltype(min16uints % floats)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3684: modulo cannot be used with doubles, cast to float first;compilation failed; no code produced;
  min16ints = (min16uints % doubles); // expected-error {{modulo cannot be used with doubles, cast to float first}} expected-warning {{conversion from larger type 'double' to smaller type 'min16int', possible loss of data}} fxc-error {{X3684: modulo cannot be used with doubles, cast to float first}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}}
  _Static_assert(std::is_same<min16float, __decltype(min16uints % min16floats)>::value, "");
  _Static_assert(std::is_same<min10float, __decltype(min16uints % min10floats)>::value, "");  // expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints % min16ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints % min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints % min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools < bools)>::value, "bool, __decltype(bools < bools) failed");
  _Static_assert(std::is_same<bool, __decltype(bools < ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools < uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools < min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools < min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools < min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools < min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools < min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints < bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints < ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints < uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints < min16floats)>::value, "");              /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints < min10floats)>::value, "");              /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints < min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints < min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints < min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints < bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints < ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints < uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints < min16floats)>::value, "");             /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints < min10floats)>::value, "");             /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints < min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints < min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints < min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs < min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats < min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles < min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats < bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats < ints)>::value, "");              /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats < uints)>::value, "");             /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats < min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats < min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats < min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats < min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats < min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats < bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats < ints)>::value, "");              /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats < uints)>::value, "");             /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats < min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats < min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats < min16ints)>::value, "");         /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats < min12ints)>::value, "");         /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats < min16uints)>::value, "");        /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints < bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints < ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints < uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints < min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints < min10floats)>::value, "");         /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints < min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints < min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints < min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints < bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints < ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints < uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints < min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints < min10floats)>::value, "");         /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min12ints < min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints < min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints < min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints < bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints < ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints < uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints < halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints < floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints < doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints < min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints < min10floats)>::value, "");        /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16uints < min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints < min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints < min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools <= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints <= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints <= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints <= min16floats)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints <= min10floats)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints <= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints <= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints <= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints <= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints <= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints <= min16floats)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints <= min10floats)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints <= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints <= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints <= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs <= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats <= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles <= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats <= ints)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats <= uints)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats <= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats <= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats <= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats <= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats <= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats <= ints)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats <= uints)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats <= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats <= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats <= min16ints)>::value, "");        /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats <= min12ints)>::value, "");        /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats <= min16uints)>::value, "");       /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints <= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints <= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints <= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints <= min10floats)>::value, "");        /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints <= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints <= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints <= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints <= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints <= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints <= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints <= min10floats)>::value, "");        /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min12ints <= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints <= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints <= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints <= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints <= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints <= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints <= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints <= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints <= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints <= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints <= min10floats)>::value, "");       /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16uints <= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints <= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints <= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools > min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints > ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints > uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints > min16floats)>::value, "");              /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints > min10floats)>::value, "");              /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints > min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints > min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints > min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints > ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints > uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints > min16floats)>::value, "");             /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints > min10floats)>::value, "");             /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints > min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints > min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints > min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs > min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats > min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles > min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats > ints)>::value, "");              /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats > uints)>::value, "");             /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats > min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats > min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats > min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats > min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats > min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats > ints)>::value, "");              /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats > uints)>::value, "");             /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats > min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats > min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats > min16ints)>::value, "");         /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats > min12ints)>::value, "");         /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats > min16uints)>::value, "");        /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints > ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints > uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints > min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints > min10floats)>::value, "");         /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints > min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints > min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints > min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints > ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints > uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints > min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints > min10floats)>::value, "");         /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min12ints > min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints > min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints > min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints > bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints > ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints > uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints > halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints > floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints > doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints > min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints > min10floats)>::value, "");        /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16uints > min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints > min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints > min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools >= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints >= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints >= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints >= min16floats)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints >= min10floats)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints >= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints >= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints >= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints >= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints >= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints >= min16floats)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints >= min10floats)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints >= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints >= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints >= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs >= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats >= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles >= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats >= ints)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats >= uints)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats >= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats >= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats >= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats >= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats >= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats >= ints)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats >= uints)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats >= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats >= min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats >= min16ints)>::value, "");        /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats >= min12ints)>::value, "");        /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats >= min16uints)>::value, "");       /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints >= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints >= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints >= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints >= min10floats)>::value, "");        /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints >= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints >= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints >= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints >= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints >= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints >= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints >= min10floats)>::value, "");        /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min12ints >= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints >= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints >= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints >= bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints >= ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints >= uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints >= halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints >= floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints >= doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints >= min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints >= min10floats)>::value, "");       /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16uints >= min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints >= min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints >= min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools == min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints == ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints == uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints == min16floats)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints == min10floats)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints == min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints == min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints == min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints == ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints == uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints == min16floats)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints == min10floats)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints == min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints == min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints == min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs == min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats == min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles == min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats == ints)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats == uints)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats == min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats == min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats == min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats == min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats == min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats == ints)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats == uints)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats == min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats == min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats == min16ints)>::value, "");        /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats == min12ints)>::value, "");        /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats == min16uints)>::value, "");       /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints == ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints == uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints == min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints == min10floats)>::value, "");        /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints == min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints == min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints == min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints == ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints == uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints == min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints == min10floats)>::value, "");        /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min12ints == min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints == min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints == min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints == bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints == ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints == uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints == halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints == floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints == doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints == min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints == min10floats)>::value, "");       /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16uints == min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints == min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints == min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools != min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints != ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints != uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints != min16floats)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints != min10floats)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(ints != min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints != min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints != min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints != ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints != uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints != min16floats)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints != min10floats)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(uints != min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints != min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints != min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs != min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats != min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles != min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats != ints)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats != uints)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min16float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16floats != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats != min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats != min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats != min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats != min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats != min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats != ints)>::value, "");             /* expected-warning {{conversion from larger type 'int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats != uints)>::value, "");            /* expected-warning {{conversion from larger type 'uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats != min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats != min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats != min16ints)>::value, "");        /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats != min12ints)>::value, "");        /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min10floats != min16uints)>::value, "");       /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints != ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints != uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints != min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints != min10floats)>::value, "");        /* expected-warning {{conversion from larger type 'min16int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16ints != min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints != min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints != min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints != ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints != uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints != min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints != min10floats)>::value, "");        /* expected-warning {{conversion from larger type 'min12int' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min12ints != min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints != min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints != min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints != bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints != ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints != uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints != halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints != floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints != doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints != min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints != min10floats)>::value, "");       /* expected-warning {{conversion from larger type 'min16uint' to smaller type 'min10float', possible loss of data}} fxc-pass {{}} */
  _Static_assert(std::is_same<bool, __decltype(min16uints != min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints != min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints != min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools << bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools << ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools << uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<int, __decltype(bools << min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools << min12ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools << min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints << bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints << ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints << uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<int, __decltype(ints << min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints << min12ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints << min16uints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints << bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints << ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints << uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<uint, __decltype(uints << min16ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints << min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints << min16uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs << min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats << min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-57): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-58): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-57): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-58): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles << min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-57): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-58): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-61): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-62): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-61): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-62): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats << min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-57): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-58): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-61): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-62): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-61): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-62): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats << min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints << bools)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints << ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints << uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints << min16ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints << min12ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints << min16uints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints << bools)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints << ints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints << uints)>::value, "");  // fxc-pass {{}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints << min16ints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints << min12ints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints << min16uints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints << bools)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints << ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints << uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints << halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints << floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints << doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints << min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints << min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints << min16ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints << min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints << min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools >> bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools >> ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools >> uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<int, __decltype(bools >> min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools >> min12ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools >> min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints >> bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints >> ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints >> uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<int, __decltype(ints >> min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints >> min12ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints >> min16uints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints >> bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints >> ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints >> uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<uint, __decltype(uints >> min16ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints >> min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints >> min16uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs >> min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats >> min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-57): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-58): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-57): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-58): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles >> min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-57): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-58): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-61): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-62): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-61): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-62): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats >> min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-57): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-58): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-61): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-62): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-61): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-62): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats >> min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints >> bools)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints >> ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints >> uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints >> min16ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints >> min12ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints >> min16uints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints >> bools)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints >> ints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints >> uints)>::value, "");  // fxc-pass {{}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints >> min16ints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints >> min12ints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min12int, __decltype(min12ints >> min16uints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints >> bools)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints >> ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints >> uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints >> halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints >> floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints >> doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints >> min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints >> min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints >> min16ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints >> min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints >> min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools & bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools & ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(bools & uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(bools & min16ints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(bools & min12ints)>::value, "");    /* fxc-pass {{}} */
  _Static_assert(std::is_same<min16uint, __decltype(bools & min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints & bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints & ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints & uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-47): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-48): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<int, __decltype(ints & min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints & min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints & min16uints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints & bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints & ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints & uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<uint, __decltype(uints & min16ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints & min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints & min16uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-47): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-48): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs & min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats & min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles & min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats & min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats & min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints & bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(min16ints & ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16ints & uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints & min16ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints & min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16ints & min16uints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints & bools)>::value, "");    /* fxc-pass {{}} */
  _Static_assert(std::is_same<int, __decltype(min12ints & ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min12ints & uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min12ints & min16ints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints & min12ints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min12ints & min16uints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints & bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints & ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints & uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints & halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints & floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints & doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints & min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints & min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints & min16ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints & min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints & min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools | bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools | ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(bools | uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(bools | min16ints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(bools | min12ints)>::value, "");    /* fxc-pass {{}} */
  _Static_assert(std::is_same<min16uint, __decltype(bools | min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints | bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints | ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints | uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-47): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-48): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<int, __decltype(ints | min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints | min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints | min16uints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints | bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints | ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints | uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<uint, __decltype(uints | min16ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints | min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints | min16uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-47): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-48): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs | min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats | min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles | min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats | min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats | min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints | bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(min16ints | ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16ints | uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints | min16ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints | min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16ints | min16uints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints | bools)>::value, "");    /* fxc-pass {{}} */
  _Static_assert(std::is_same<int, __decltype(min12ints | ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min12ints | uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min12ints | min16ints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints | min12ints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min12ints | min16uints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints | bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints | ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints | uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints | halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints | floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints | doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints | min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints | min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints | min16ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints | min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints | min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools ^ bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(bools ^ ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(bools ^ uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  bools = (bools ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(bools ^ min16ints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(bools ^ min12ints)>::value, "");    /* fxc-pass {{}} */
  _Static_assert(std::is_same<min16uint, __decltype(bools ^ min16uints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints ^ bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints ^ ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints ^ uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-47): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-48): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  ints = (ints ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<int, __decltype(ints ^ min16ints)>::value, "");
  _Static_assert(std::is_same<int, __decltype(ints ^ min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(ints ^ min16uints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints ^ bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints ^ ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints ^ uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  uints = (uints ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<uint, __decltype(uints ^ min16ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints ^ min12ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(uints ^ min16uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-47): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-48): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  halfs = (halfs ^ min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-48): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-49): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  floats = (floats ^ min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-49): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-50): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-50): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-51): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-51): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-52): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  doubles = (doubles ^ min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16floats = (min16floats ^ min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ bools); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-56): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-57): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-60): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-61): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ min16ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ min12ints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min10floats = (min10floats ^ min16uints); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints ^ bools)>::value, "");
  _Static_assert(std::is_same<int, __decltype(min16ints ^ ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16ints ^ uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16ints = (min16ints ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min16ints ^ min16ints)>::value, "");
  _Static_assert(std::is_same<min16int, __decltype(min16ints ^ min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16ints ^ min16uints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints ^ bools)>::value, "");    /* fxc-pass {{}} */
  _Static_assert(std::is_same<int, __decltype(min12ints ^ ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min12ints ^ uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-52): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-53): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-58): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-59): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min12ints = (min12ints ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16int, __decltype(min12ints ^ min16ints)>::value, "");
  _Static_assert(std::is_same<min12int, __decltype(min12ints ^ min12ints)>::value, "");  // fxc-pass {{}}
  _Static_assert(std::is_same<min16uint, __decltype(min12ints ^ min16uints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints ^ bools)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints ^ ints)>::value, "");
  _Static_assert(std::is_same<uint, __decltype(min16uints ^ uints)>::value, "");
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-53): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-54): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints ^ halfs); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-54): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-55): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints ^ floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-55): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-56): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints ^ doubles); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints ^ min16floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  // X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,22-59): error X3082: int or unsigned int type required;X:\temp\Sfbl_grfx_dev_p\x86\chk\operators.js.hlsl(16,12-60): error X3013: 'get_value': no matching 1 parameter function;compilation failed; no code produced
  min16uints = (min16uints ^ min10floats); // expected-error {{int or unsigned int type required}} fxc-error {{X3082: int or unsigned int type required}}
  _Static_assert(std::is_same<min16uint, __decltype(min16uints ^ min16ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints ^ min12ints)>::value, "");
  _Static_assert(std::is_same<min16uint, __decltype(min16uints ^ min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints && min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(bools || min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(ints || min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(uints || min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(halfs || min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(floats || min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(doubles || min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16floats || min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min10floats || min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16ints || min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min12ints || min16uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || bools)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || uints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || halfs)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || doubles)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || min16floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || min10floats)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || min16ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || min12ints)>::value, "");
  _Static_assert(std::is_same<bool, __decltype(min16uints || min16uints)>::value, "");

  return param4;
}
