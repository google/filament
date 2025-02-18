struct S_inner {
  float a;
};

struct S {
  bool member_bool;
  int member_i32;
  uint member_u32;
  float member_f32;
  int2 member_v2i32;
  uint3 member_v3u32;
  float4 member_v4f32;
  float2x3 member_m2x3;
  float member_arr[4];
  S_inner member_struct;
};


[numthreads(1, 1, 1)]
void main() {
  S s = (S)0;
}

