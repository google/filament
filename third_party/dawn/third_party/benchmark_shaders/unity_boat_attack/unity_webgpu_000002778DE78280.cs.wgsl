diagnostic(off, derivative_uniformity);

alias RTArr = array<u32>;

alias RTArr_1 = array<u32>;

struct x_Input {
  /* @offset(0) */
  x_Input_buf : RTArr_1,
}

struct S {
  value : array<u32, 1u>,
}

struct x_Output_origX0X {
  /* @offset(0) */
  x_Output_origX0X_buf : RTArr_1,
}

var<private> u_xlatu0 : vec2<u32>;

var<private> gl_LocalInvocationIndex : u32;

var<private> u_xlati4 : i32;

var<private> gl_GlobalInvocationID : vec3<u32>;

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
  let x_19 : i32 = *(bits);
  let x_23 : i32 = *(offset_1);
  mask = (~((4294967295u << bitcast<u32>(x_19))) << bitcast<u32>(x_23));
  let x_26 : i32 = *(base);
  let x_28 : u32 = mask;
  let x_31 : i32 = *(insert);
  let x_33 : i32 = *(offset_1);
  let x_36 : u32 = mask;
  return bitcast<i32>(((bitcast<u32>(x_26) & ~(x_28)) | ((bitcast<u32>(x_31) << bitcast<u32>(x_33)) & x_36)));
}

fn main_1() {
  var param : i32;
  var param_1 : i32;
  var param_2 : i32;
  var param_3 : i32;
  var u_xlat_precise_vec4 : vec4<f32>;
  var u_xlat_precise_ivec4 : vec4<i32>;
  var u_xlat_precise_bvec4 : vec4<bool>;
  var u_xlat_precise_uvec4 : vec4<u32>;
  let x_47 : u32 = gl_LocalInvocationIndex;
  let x_48 : u32 = gl_LocalInvocationIndex;
  u_xlatu0 = (vec2<u32>(x_47, x_48) & vec2<u32>(31u, 96u));
  let x_61 : u32 = gl_GlobalInvocationID.x;
  u_xlati4 = (bitcast<i32>(x_61) << bitcast<u32>(2i));
  let x_73 : i32 = u_xlati4;
  let x_78 : u32 = x_71.x_Input_buf[((x_73 >> bitcast<u32>(2i)) + 0i)];
  u_xlat4 = bitcast<f32>(x_78);
  let x_80 : f32 = u_xlat4;
  let x_82 : u32 = gl_LocalInvocationIndex;
  u_xlati4 = bitcast<i32>(select(0u, 4294967295u, (bitcast<i32>(x_80) == bitcast<i32>(x_82))));
  let x_88 : u32 = gl_LocalInvocationIndex;
  param = 0i;
  let x_95 : i32 = u_xlati4;
  param_1 = x_95;
  param_2 = (bitcast<i32>(x_88) & 31i);
  param_3 = 1i;
  let x_98 : i32 = int_bitfieldInsert_i1_i1_i1_i1_(&(param), &(param_1), &(param_2), &(param_3));
  u_xlati4 = x_98;
  let x_106 : u32 = gl_LocalInvocationIndex;
  let x_107 : i32 = u_xlati4;
  TGSM0[x_106].value[0i] = bitcast<u32>(x_107);
  workgroupBarrier();
  let x_117 : vec2<u32> = u_xlatu0;
  u_xlatb1 = (vec4<u32>(x_117.x, x_117.x, x_117.x, x_117.x) < vec4<u32>(16u, 8u, 4u, 2u));
  let x_126 : bool = u_xlatb1.x;
  if (x_126) {
    let x_129 : u32 = gl_LocalInvocationIndex;
    let x_131 : u32 = TGSM0[x_129].value[0i];
    u_xlati4 = bitcast<i32>(x_131);
    let x_134 : u32 = gl_LocalInvocationIndex;
    u_xlati6 = (bitcast<i32>(x_134) + 16i);
    let x_138 : i32 = u_xlati6;
    let x_140 : u32 = TGSM0[x_138].value[0i];
    u_xlati6 = bitcast<i32>(x_140);
    let x_142 : i32 = u_xlati6;
    let x_144 : i32 = u_xlati4;
    u_xlati4 = bitcast<i32>((bitcast<u32>(x_142) | bitcast<u32>(x_144)));
    let x_148 : u32 = gl_LocalInvocationIndex;
    let x_149 : i32 = u_xlati4;
    TGSM0[x_148].value[0i] = bitcast<u32>(x_149);
  }
  workgroupBarrier();
  let x_153 : bool = u_xlatb1.y;
  if (x_153) {
    let x_156 : u32 = gl_LocalInvocationIndex;
    let x_158 : u32 = TGSM0[x_156].value[0i];
    u_xlati4 = bitcast<i32>(x_158);
    let x_160 : u32 = gl_LocalInvocationIndex;
    u_xlati6 = (bitcast<i32>(x_160) + 8i);
    let x_164 : i32 = u_xlati6;
    let x_166 : u32 = TGSM0[x_164].value[0i];
    u_xlati6 = bitcast<i32>(x_166);
    let x_168 : i32 = u_xlati6;
    let x_170 : i32 = u_xlati4;
    u_xlati4 = bitcast<i32>((bitcast<u32>(x_168) | bitcast<u32>(x_170)));
    let x_174 : u32 = gl_LocalInvocationIndex;
    let x_175 : i32 = u_xlati4;
    TGSM0[x_174].value[0i] = bitcast<u32>(x_175);
  }
  workgroupBarrier();
  let x_179 : bool = u_xlatb1.z;
  if (x_179) {
    let x_182 : u32 = gl_LocalInvocationIndex;
    let x_184 : u32 = TGSM0[x_182].value[0i];
    u_xlati4 = bitcast<i32>(x_184);
    let x_186 : u32 = gl_LocalInvocationIndex;
    u_xlati6 = (bitcast<i32>(x_186) + 4i);
    let x_190 : i32 = u_xlati6;
    let x_192 : u32 = TGSM0[x_190].value[0i];
    u_xlati6 = bitcast<i32>(x_192);
    let x_194 : i32 = u_xlati6;
    let x_196 : i32 = u_xlati4;
    u_xlati4 = bitcast<i32>((bitcast<u32>(x_194) | bitcast<u32>(x_196)));
    let x_200 : u32 = gl_LocalInvocationIndex;
    let x_201 : i32 = u_xlati4;
    TGSM0[x_200].value[0i] = bitcast<u32>(x_201);
  }
  workgroupBarrier();
  let x_206 : bool = u_xlatb1.w;
  if (x_206) {
    let x_209 : u32 = gl_LocalInvocationIndex;
    let x_211 : u32 = TGSM0[x_209].value[0i];
    u_xlati4 = bitcast<i32>(x_211);
    let x_213 : u32 = gl_LocalInvocationIndex;
    u_xlati6 = (bitcast<i32>(x_213) + 2i);
    let x_216 : i32 = u_xlati6;
    let x_218 : u32 = TGSM0[x_216].value[0i];
    u_xlati6 = bitcast<i32>(x_218);
    let x_220 : i32 = u_xlati6;
    let x_222 : i32 = u_xlati4;
    u_xlati4 = bitcast<i32>((bitcast<u32>(x_220) | bitcast<u32>(x_222)));
    let x_226 : u32 = gl_LocalInvocationIndex;
    let x_227 : i32 = u_xlati4;
    TGSM0[x_226].value[0i] = bitcast<u32>(x_227);
  }
  workgroupBarrier();
  let x_233 : u32 = u_xlatu0.x;
  u_xlatb0 = (x_233 < 1u);
  let x_235 : bool = u_xlatb0;
  if (x_235) {
    let x_239 : u32 = gl_LocalInvocationIndex;
    let x_241 : u32 = TGSM0[x_239].value[0i];
    u_xlati0 = bitcast<i32>(x_241);
    let x_243 : u32 = gl_LocalInvocationIndex;
    u_xlati4 = (bitcast<i32>(x_243) + 1i);
    let x_246 : i32 = u_xlati4;
    let x_248 : u32 = TGSM0[x_246].value[0i];
    u_xlati4 = bitcast<i32>(x_248);
    let x_250 : i32 = u_xlati4;
    let x_252 : i32 = u_xlati0;
    u_xlati0 = bitcast<i32>((bitcast<u32>(x_250) | bitcast<u32>(x_252)));
    let x_256 : u32 = gl_LocalInvocationIndex;
    let x_257 : i32 = u_xlati0;
    TGSM0[x_256].value[0i] = bitcast<u32>(x_257);
  }
  workgroupBarrier();
  let x_261 : u32 = u_xlatu0.y;
  let x_263 : u32 = TGSM0[x_261].value[0i];
  u_xlati0 = bitcast<i32>(x_263);
  let x_265 : i32 = u_xlati0;
  u_xlati0 = countOneBits(x_265);
  let x_267 : i32 = u_xlati0;
  u_xlatb0 = (x_267 == 32i);
  let x_270 : bool = u_xlatb0;
  u_xlati0 = select(0i, 1i, x_270);
  let x_273 : u32 = gl_LocalInvocationIndex;
  u_xlati2 = (bitcast<i32>(x_273) << bitcast<u32>(2i));
  let x_280 : i32 = u_xlati2;
  let x_282 : i32 = u_xlati0;
  x_279.x_Output_origX0X_buf[(x_280 >> bitcast<u32>(2i))] = bitcast<u32>(x_282);
  return;
}

@compute @workgroup_size(128i, 1i, 1i)
fn main(@builtin(local_invocation_index) gl_LocalInvocationIndex_param : u32, @builtin(global_invocation_id) gl_GlobalInvocationID_param : vec3<u32>) {
  gl_LocalInvocationIndex = gl_LocalInvocationIndex_param;
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}

