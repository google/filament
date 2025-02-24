// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 125)

namespace  n {
template<unsigned int M, unsigned int N>
struct Ackermann {
 enum {
   value = Ackermann<M-1, Ackermann<M, N-1>::value >::value
 };
};

template<unsigned int M> struct Ackermann<M, 0> {
 enum {
   value = Ackermann<M-1, 1>::value
 };
};

template<unsigned int N> struct Ackermann<0, N> {
 enum {
   value = N + 1
 };
};

template<> struct Ackermann<0, 0> {
 enum {
   value = 1
 };
};

  using A34 = Ackermann<3, 4>;

}


namespace  n2 {
    using n::A34;
}



int main(int a:A) : SV_Target {
  return n2::A34::value;
}
