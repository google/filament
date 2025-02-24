// RUN: %dxc -DTEMPLATE= -T ps_6_0 -HV 2021 %s | FileCheck %s -check-prefix=CHK_FAIL
// RUN: %dxc -DTEMPLATE=template -T ps_6_0 -HV 2021 %s | FileCheck %s -check-prefix=CHK_PASS

// CHK_PASS:define void @main

template<typename T>
struct F1 {
  template <int B>
  struct Iterator {
    T t;
  };
};

template<typename T>
struct F2  {
  // CHK_FAIL: use 'template' keyword to treat 'Iterator' as a dependent template name
  typename F1<T>:: TEMPLATE  Iterator<0> Mypos; // expected-error {{
};

struct F2<float4> ts;

template <typename T>
float4 f(){
  // CHK_FAIL: use 'template' keyword to treat 'Iterator' as a dependent template name
  typename F1<T>:: TEMPLATE Iterator<0> Mypos = ts.Mypos;
  return Mypos.t;
}

float4 main() : SV_Target {
  return f<float4>();
}
