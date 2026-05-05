diagnostic(off, derivative_uniformity);

var<private> u_xlatu0 : vec2<u32>;

var<private> u_xlati4 : i32;

var<private> u_xlat4 : f32;

struct _Input {
  _Input_buf : array<u32>,
}

@group(0u) @binding(0u) var<storage, read> v : _Input;

struct tint_symbol {
  value : array<u32, 1u>,
}

var<workgroup> TGSM0 : array<tint_symbol, 128u>;

var<private> u_xlatb1 : vec4<bool>;

var<private> u_xlati6 : i32;

var<private> u_xlatb0 : bool;

var<private> u_xlati0 : i32;

var<private> u_xlati2 : i32;

struct _Output_origX0X {
  _Output_origX0X_buf : array<u32>,
}

@group(0u) @binding(1u) var<storage, read_write> v_1 : _Output_origX0X;

@compute @workgroup_size(128u, 1u, 1u)
fn main(@builtin(local_invocation_index) gl_LocalInvocationIndex : u32, @builtin(global_invocation_id) gl_GlobalInvocationID : vec3<u32>) {
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  var u_xlat_precise_vec4 : vec4<f32>;
  var u_xlat_precise_ivec4 : vec4<i32>;
  var u_xlat_precise_bvec4 : vec4<bool>;
  var u_xlat_precise_uvec4 : vec4<u32>;
  u_xlatu0 = (vec2<u32>(gl_LocalInvocationIndex, gl_LocalInvocationIndex) & vec2<u32>(31u, 96u));
  u_xlati4 = (bitcast<i32>(gl_GlobalInvocationID.x) << bitcast<u32>(2i));
  u_xlat4 = bitcast<f32>(v._Input_buf[((u_xlati4 >> bitcast<u32>(2i)) + 0i)]);
  u_xlati4 = bitcast<i32>(select(0u, 4294967295u, (bitcast<i32>(u_xlat4) == bitcast<i32>(gl_LocalInvocationIndex))));
  let v_2 = (bitcast<i32>(gl_LocalInvocationIndex) & 31i);
  param = 0i;
  param_1 = u_xlati4;
  param_2 = v_2;
  param_3 = 1i;
  u_xlati4 = v_3(&(param), &(param_1), &(param_2), &(param_3));
  TGSM0[gl_LocalInvocationIndex].value[0i] = bitcast<u32>(u_xlati4);
  workgroupBarrier();
  u_xlatb1 = (u_xlatu0.xxxx < vec4<u32>(16u, 8u, 4u, 2u));
  if (u_xlatb1.x) {
    u_xlati4 = bitcast<i32>(TGSM0[gl_LocalInvocationIndex].value[0i]);
    u_xlati6 = (bitcast<i32>(gl_LocalInvocationIndex) + 16i);
    u_xlati6 = bitcast<i32>(TGSM0[u_xlati6].value[0i]);
    u_xlati4 = bitcast<i32>((bitcast<u32>(u_xlati6) | bitcast<u32>(u_xlati4)));
    TGSM0[gl_LocalInvocationIndex].value[0i] = bitcast<u32>(u_xlati4);
  }
  workgroupBarrier();
  if (u_xlatb1.y) {
    u_xlati4 = bitcast<i32>(TGSM0[gl_LocalInvocationIndex].value[0i]);
    u_xlati6 = (bitcast<i32>(gl_LocalInvocationIndex) + 8i);
    u_xlati6 = bitcast<i32>(TGSM0[u_xlati6].value[0i]);
    u_xlati4 = bitcast<i32>((bitcast<u32>(u_xlati6) | bitcast<u32>(u_xlati4)));
    TGSM0[gl_LocalInvocationIndex].value[0i] = bitcast<u32>(u_xlati4);
  }
  workgroupBarrier();
  if (u_xlatb1.z) {
    u_xlati4 = bitcast<i32>(TGSM0[gl_LocalInvocationIndex].value[0i]);
    u_xlati6 = (bitcast<i32>(gl_LocalInvocationIndex) + 4i);
    u_xlati6 = bitcast<i32>(TGSM0[u_xlati6].value[0i]);
    u_xlati4 = bitcast<i32>((bitcast<u32>(u_xlati6) | bitcast<u32>(u_xlati4)));
    TGSM0[gl_LocalInvocationIndex].value[0i] = bitcast<u32>(u_xlati4);
  }
  workgroupBarrier();
  if (u_xlatb1.w) {
    u_xlati4 = bitcast<i32>(TGSM0[gl_LocalInvocationIndex].value[0i]);
    u_xlati6 = (bitcast<i32>(gl_LocalInvocationIndex) + 2i);
    u_xlati6 = bitcast<i32>(TGSM0[u_xlati6].value[0i]);
    u_xlati4 = bitcast<i32>((bitcast<u32>(u_xlati6) | bitcast<u32>(u_xlati4)));
    TGSM0[gl_LocalInvocationIndex].value[0i] = bitcast<u32>(u_xlati4);
  }
  workgroupBarrier();
  u_xlatb0 = (u_xlatu0.x < 1u);
  if (u_xlatb0) {
    u_xlati0 = bitcast<i32>(TGSM0[gl_LocalInvocationIndex].value[0i]);
    u_xlati4 = (bitcast<i32>(gl_LocalInvocationIndex) + 1i);
    u_xlati4 = bitcast<i32>(TGSM0[u_xlati4].value[0i]);
    u_xlati0 = bitcast<i32>((bitcast<u32>(u_xlati4) | bitcast<u32>(u_xlati0)));
    TGSM0[gl_LocalInvocationIndex].value[0i] = bitcast<u32>(u_xlati0);
  }
  workgroupBarrier();
  u_xlati0 = bitcast<i32>(TGSM0[u_xlatu0.y].value[0i]);
  u_xlati0 = countOneBits(u_xlati0);
  u_xlatb0 = (u_xlati0 == 32i);
  u_xlati0 = select(0i, 1i, u_xlatb0);
  u_xlati2 = (bitcast<i32>(gl_LocalInvocationIndex) << bitcast<u32>(2i));
  let v_4 = (u_xlati2 >> bitcast<u32>(2i));
  v_1._Output_origX0X_buf[v_4] = bitcast<u32>(u_xlati0);
}

fn v_3(base : ptr<function, i32>, insert : ptr<function, i32>, offset : ptr<function, i32>, bits : ptr<function, i32>) -> i32 {
  var mask : u32;
  mask = (~((4294967295u << bitcast<u32>(*(bits)))) << bitcast<u32>(*(offset)));
  return bitcast<i32>(((bitcast<u32>(*(base)) & ~(mask)) | ((bitcast<u32>(*(insert)) << bitcast<u32>(*(offset))) & mask)));
}
