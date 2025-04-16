diagnostic(off, derivative_uniformity);

alias RTArr = array<u32>;

struct x_Input {
  /* @offset(0) */
  x_Input_buf : RTArr,
}

struct S {
  value : array<u32, 1u>,
}

alias RTArr_1 = array<u32>;

struct x_Output_origX0X {
  /* @offset(0) */
  x_Output_origX0X_buf : RTArr_1,
}

var<private> u_xlatu0 : vec2u;

var<private> gl_LocalInvocationIndex : u32;

var<private> u_xlati4 : i32;

var<private> gl_GlobalInvocationID : vec3u;

var<private> u_xlat4 : f32;

@group(0) @binding(0) var<storage, read> x_71 : x_Input;

var<workgroup> TGSM0 : array<S, 128u>;

var<private> u_xlatb1 : vec4<bool>;

var<private> u_xlati6 : i32;

var<private> u_xlatb0 : bool;

var<private> u_xlati0 : i32;

var<private> u_xlati2 : i32;

@group(0) @binding(1) var<storage, read_write> x_279 : x_Output_origX0X;

fn int_bitfieldInsert_i1_i1_i1_i1_(base : ptr<function, i32>, insert : ptr<function, i32>, offset_1 : ptr<function, i32>, bits : ptr<function, i32>) -> i32 {
  var mask : u32;
  mask = (~((4294967295u << bitcast<u32>(*(bits)))) << bitcast<u32>(*(offset_1)));
  let x_26 = *(base);
  let x_28 = mask;
  let x_31 = *(insert);
  let x_33 = *(offset_1);
  let x_36 = mask;
  return bitcast<i32>(((bitcast<u32>(x_26) & ~(x_28)) | ((bitcast<u32>(x_31) << bitcast<u32>(x_33)) & x_36)));
}

fn main_1() {
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  var u_xlat_precise_vec4 : vec4f;
  var u_xlat_precise_ivec4 : vec4i;
  var u_xlat_precise_bvec4 : vec4<bool>;
  var u_xlat_precise_uvec4 : vec4u;
  u_xlatu0 = (vec2u(gl_LocalInvocationIndex, gl_LocalInvocationIndex) & vec2u(31u, 96u));
  u_xlati4 = (bitcast<i32>(gl_GlobalInvocationID.x) << bitcast<u32>(2i));
  u_xlat4 = bitcast<f32>(x_71.x_Input_buf[((u_xlati4 >> bitcast<u32>(2i)) + 0i)]);
  u_xlati4 = bitcast<i32>(select(0u, 4294967295u, (bitcast<i32>(u_xlat4) == bitcast<i32>(gl_LocalInvocationIndex))));
  let x_88 = gl_LocalInvocationIndex;
  param = 0i;
  param_1 = u_xlati4;
  param_2 = (bitcast<i32>(x_88) & 31i);
  param_3 = 1i;
  let x_98 = int_bitfieldInsert_i1_i1_i1_i1_(&(param), &(param_1), &(param_2), &(param_3));
  u_xlati4 = x_98;
  let x_106 = gl_LocalInvocationIndex;
  TGSM0[x_106].value[0i] = bitcast<u32>(u_xlati4);
  workgroupBarrier();
  u_xlatb1 = (u_xlatu0.xxxx < vec4u(16u, 8u, 4u, 2u));
  if (u_xlatb1.x) {
    u_xlati4 = bitcast<i32>(TGSM0[gl_LocalInvocationIndex].value[0i]);
    u_xlati6 = (bitcast<i32>(gl_LocalInvocationIndex) + 16i);
    u_xlati6 = bitcast<i32>(TGSM0[u_xlati6].value[0i]);
    u_xlati4 = bitcast<i32>((bitcast<u32>(u_xlati6) | bitcast<u32>(u_xlati4)));
    let x_148 = gl_LocalInvocationIndex;
    TGSM0[x_148].value[0i] = bitcast<u32>(u_xlati4);
  }
  workgroupBarrier();
  if (u_xlatb1.y) {
    u_xlati4 = bitcast<i32>(TGSM0[gl_LocalInvocationIndex].value[0i]);
    u_xlati6 = (bitcast<i32>(gl_LocalInvocationIndex) + 8i);
    u_xlati6 = bitcast<i32>(TGSM0[u_xlati6].value[0i]);
    u_xlati4 = bitcast<i32>((bitcast<u32>(u_xlati6) | bitcast<u32>(u_xlati4)));
    let x_174 = gl_LocalInvocationIndex;
    TGSM0[x_174].value[0i] = bitcast<u32>(u_xlati4);
  }
  workgroupBarrier();
  if (u_xlatb1.z) {
    u_xlati4 = bitcast<i32>(TGSM0[gl_LocalInvocationIndex].value[0i]);
    u_xlati6 = (bitcast<i32>(gl_LocalInvocationIndex) + 4i);
    u_xlati6 = bitcast<i32>(TGSM0[u_xlati6].value[0i]);
    u_xlati4 = bitcast<i32>((bitcast<u32>(u_xlati6) | bitcast<u32>(u_xlati4)));
    let x_200 = gl_LocalInvocationIndex;
    TGSM0[x_200].value[0i] = bitcast<u32>(u_xlati4);
  }
  workgroupBarrier();
  if (u_xlatb1.w) {
    u_xlati4 = bitcast<i32>(TGSM0[gl_LocalInvocationIndex].value[0i]);
    u_xlati6 = (bitcast<i32>(gl_LocalInvocationIndex) + 2i);
    u_xlati6 = bitcast<i32>(TGSM0[u_xlati6].value[0i]);
    u_xlati4 = bitcast<i32>((bitcast<u32>(u_xlati6) | bitcast<u32>(u_xlati4)));
    let x_226 = gl_LocalInvocationIndex;
    TGSM0[x_226].value[0i] = bitcast<u32>(u_xlati4);
  }
  workgroupBarrier();
  u_xlatb0 = (u_xlatu0.x < 1u);
  if (u_xlatb0) {
    u_xlati0 = bitcast<i32>(TGSM0[gl_LocalInvocationIndex].value[0i]);
    u_xlati4 = (bitcast<i32>(gl_LocalInvocationIndex) + 1i);
    u_xlati4 = bitcast<i32>(TGSM0[u_xlati4].value[0i]);
    u_xlati0 = bitcast<i32>((bitcast<u32>(u_xlati4) | bitcast<u32>(u_xlati0)));
    let x_256 = gl_LocalInvocationIndex;
    TGSM0[x_256].value[0i] = bitcast<u32>(u_xlati0);
  }
  workgroupBarrier();
  u_xlati0 = bitcast<i32>(TGSM0[u_xlatu0.y].value[0i]);
  u_xlati0 = countOneBits(u_xlati0);
  u_xlatb0 = (u_xlati0 == 32i);
  u_xlati0 = select(0i, 1i, u_xlatb0);
  u_xlati2 = (bitcast<i32>(gl_LocalInvocationIndex) << bitcast<u32>(2i));
  let x_280 = u_xlati2;
  x_279.x_Output_origX0X_buf[(x_280 >> bitcast<u32>(2i))] = bitcast<u32>(u_xlati0);
  return;
}

@compute @workgroup_size(128i, 1i, 1i)
fn main(@builtin(local_invocation_index) gl_LocalInvocationIndex_param : u32, @builtin(global_invocation_id) gl_GlobalInvocationID_param : vec3u) {
  gl_LocalInvocationIndex = gl_LocalInvocationIndex_param;
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}
