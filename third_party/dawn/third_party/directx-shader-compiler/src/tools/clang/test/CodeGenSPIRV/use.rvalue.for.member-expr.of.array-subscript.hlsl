// RUN: %dxc -T cs_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

// Tests that a rvalue is used for the index of ArraySubscriptExpr. The newly
// introduced template support generates a template instance of
// `BufferAccess::load(uint index)` that misses LValueToRValue cast for a
// MemberExpr. We prevent an array subscript from using lvalue.

[[vk::binding(0, 0)]] ByteAddressBuffer babuf[]: register(t0, space0);
[[vk::binding(0, 0)]] RWByteAddressBuffer rwbuf[]: register(u0, space0);

struct BufferAccess {
  uint handle;

  template<typename T>
  T load(uint index) {
    // CHECK: [[handle_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %param_this %int_0
    // CHECK: [[handle:%[0-9]+]] = OpLoad %uint [[handle_ptr]]
    // CHECK: OpAccessChain %_ptr_Uniform_type_ByteAddressBuffer %babuf [[handle]]
    return babuf[this.handle].Load<T>(index * sizeof(T));
  }

  template<typename T>
  void store(uint index, T value) {
    return rwbuf[this.handle].Store<T>(index * sizeof(T), value);
  }
};

struct Data {
  BufferAccess buf;
  uint a0;
  uint a1;
  uint a2;
};

struct A {
  uint x;
};

[[vk::push_constant]] ConstantBuffer<Data> cbuf;

[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadId) {
  A b = cbuf.buf.load<A>(0);
  b.x = 12;
  cbuf.buf.store<A>(0, b);
}
