struct Inner {
  float scalar_f32;
  float3 vec3_f32;
  float2x4 mat2x4_f32;
};

struct S {
  Inner inner;
};


ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void v_2(uint offset, float2x4 obj) {
  v_1.Store4((offset + 0u), asuint(obj[0u]));
  v_1.Store4((offset + 16u), asuint(obj[1u]));
}

void v_3(uint offset, Inner obj) {
  v_1.Store((offset + 0u), asuint(obj.scalar_f32));
  v_1.Store3((offset + 16u), asuint(obj.vec3_f32));
  v_2((offset + 32u), obj.mat2x4_f32);
}

void v_4(uint offset, S obj) {
  Inner v_5 = obj.inner;
  v_3((offset + 0u), v_5);
}

float2x4 v_6(uint offset) {
  return float2x4(asfloat(v.Load4((offset + 0u))), asfloat(v.Load4((offset + 16u))));
}

Inner v_7(uint offset) {
  float v_8 = asfloat(v.Load((offset + 0u)));
  float3 v_9 = asfloat(v.Load3((offset + 16u)));
  Inner v_10 = {v_8, v_9, v_6((offset + 32u))};
  return v_10;
}

S v_11(uint offset) {
  Inner v_12 = v_7((offset + 0u));
  S v_13 = {v_12};
  return v_13;
}

[numthreads(1, 1, 1)]
void main() {
  S t = v_11(0u);
  v_4(0u, t);
}

