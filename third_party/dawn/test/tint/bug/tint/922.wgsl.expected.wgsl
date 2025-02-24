struct Mat4x4_ {
  mx : vec4<f32>,
  my : vec4<f32>,
  mz : vec4<f32>,
  mw : vec4<f32>,
}

struct Mat4x3_ {
  mx : vec4<f32>,
  my : vec4<f32>,
  mz : vec4<f32>,
}

struct Mat4x2_ {
  mx : vec4<f32>,
  my : vec4<f32>,
}

struct ub_SceneParams {
  u_Projection : Mat4x4_,
}

struct ub_MaterialParams {
  u_TexMtx : array<Mat4x2_, 1>,
  u_Misc0_ : vec4<f32>,
}

struct ub_PacketParams {
  u_PosMtx : array<Mat4x3_, 32>,
}

struct VertexOutput {
  @location(0)
  v_Color : vec4<f32>,
  @location(1)
  v_TexCoord : vec2<f32>,
  @builtin(position)
  member : vec4<f32>,
}

@group(0) @binding(0) var<uniform> global : ub_SceneParams;

@group(0) @binding(1) var<uniform> global1 : ub_MaterialParams;

@group(0) @binding(2) var<uniform> global2 : ub_PacketParams;

var<private> a_Position1 : vec3<f32>;

var<private> a_UV1 : vec2<f32>;

var<private> a_Color1 : vec4<f32>;

var<private> a_Normal1 : vec3<f32>;

var<private> a_PosMtxIdx1 : f32;

var<private> v_Color : vec4<f32>;

var<private> v_TexCoord : vec2<f32>;

var<private> gl_Position : vec4<f32>;

fn Mat4x3GetCol0_(m : Mat4x3_) -> vec3<f32> {
  var m1 : Mat4x3_;
  m1 = m;
  let x_e2 : Mat4x3_ = m1;
  let x_e5 : Mat4x3_ = m1;
  let x_e8 : Mat4x3_ = m1;
  return vec3<f32>(x_e2.mx.x, x_e5.my.x, x_e8.mz.x);
}

fn Mat4x3GetCol1_(m2 : Mat4x3_) -> vec3<f32> {
  var m3 : Mat4x3_;
  m3 = m2;
  let x_e2 : Mat4x3_ = m3;
  let x_e5 : Mat4x3_ = m3;
  let x_e8 : Mat4x3_ = m3;
  return vec3<f32>(x_e2.mx.y, x_e5.my.y, x_e8.mz.y);
}

fn Mat4x3GetCol2_(m4 : Mat4x3_) -> vec3<f32> {
  var m5 : Mat4x3_;
  m5 = m4;
  let x_e2 : Mat4x3_ = m5;
  let x_e5 : Mat4x3_ = m5;
  let x_e8 : Mat4x3_ = m5;
  return vec3<f32>(x_e2.mx.z, x_e5.my.z, x_e8.mz.z);
}

fn Mat4x3GetCol3_(m6 : Mat4x3_) -> vec3<f32> {
  var m7 : Mat4x3_;
  m7 = m6;
  let x_e2 : Mat4x3_ = m7;
  let x_e5 : Mat4x3_ = m7;
  let x_e8 : Mat4x3_ = m7;
  return vec3<f32>(x_e2.mx.w, x_e5.my.w, x_e8.mz.w);
}

fn Mul(m8 : Mat4x4_, v : vec4<f32>) -> vec4<f32> {
  var m9 : Mat4x4_;
  var v1 : vec4<f32>;
  m9 = m8;
  v1 = v;
  let x_e4 : Mat4x4_ = m9;
  let x_e6 : vec4<f32> = v1;
  let x_e8 : Mat4x4_ = m9;
  let x_e10 : vec4<f32> = v1;
  let x_e12 : Mat4x4_ = m9;
  let x_e14 : vec4<f32> = v1;
  let x_e16 : Mat4x4_ = m9;
  let x_e18 : vec4<f32> = v1;
  return vec4<f32>(dot(x_e4.mx, x_e6), dot(x_e8.my, x_e10), dot(x_e12.mz, x_e14), dot(x_e16.mw, x_e18));
}

fn Mul1(m10 : Mat4x3_, v2 : vec4<f32>) -> vec3<f32> {
  var m11 : Mat4x3_;
  var v3 : vec4<f32>;
  m11 = m10;
  v3 = v2;
  let x_e4 : Mat4x3_ = m11;
  let x_e6 : vec4<f32> = v3;
  let x_e8 : Mat4x3_ = m11;
  let x_e10 : vec4<f32> = v3;
  let x_e12 : Mat4x3_ = m11;
  let x_e14 : vec4<f32> = v3;
  return vec3<f32>(dot(x_e4.mx, x_e6), dot(x_e8.my, x_e10), dot(x_e12.mz, x_e14));
}

fn Mul2(m12 : Mat4x2_, v4 : vec4<f32>) -> vec2<f32> {
  var m13 : Mat4x2_;
  var v5 : vec4<f32>;
  m13 = m12;
  v5 = v4;
  let x_e4 : Mat4x2_ = m13;
  let x_e6 : vec4<f32> = v5;
  let x_e8 : Mat4x2_ = m13;
  let x_e10 : vec4<f32> = v5;
  return vec2<f32>(dot(x_e4.mx, x_e6), dot(x_e8.my, x_e10));
}

fn Mul3(v6 : vec3<f32>, m14 : Mat4x3_) -> vec4<f32> {
  var v7 : vec3<f32>;
  var m15 : Mat4x3_;
  v7 = v6;
  m15 = m14;
  let x_e5 : Mat4x3_ = m15;
  let x_e6 : vec3<f32> = Mat4x3GetCol0_(x_e5);
  let x_e7 : vec3<f32> = v7;
  let x_e10 : Mat4x3_ = m15;
  let x_e11 : vec3<f32> = Mat4x3GetCol1_(x_e10);
  let x_e12 : vec3<f32> = v7;
  let x_e15 : Mat4x3_ = m15;
  let x_e16 : vec3<f32> = Mat4x3GetCol2_(x_e15);
  let x_e17 : vec3<f32> = v7;
  let x_e20 : Mat4x3_ = m15;
  let x_e21 : vec3<f32> = Mat4x3GetCol3_(x_e20);
  let x_e22 : vec3<f32> = v7;
  return vec4<f32>(dot(x_e6, x_e7), dot(x_e11, x_e12), dot(x_e16, x_e17), dot(x_e21, x_e22));
}

fn x_Mat4x4_(n : f32) -> Mat4x4_ {
  var n1 : f32;
  var o : Mat4x4_;
  n1 = n;
  let x_e4 : f32 = n1;
  o.mx = vec4<f32>(x_e4, 0.0, 0.0, 0.0);
  let x_e11 : f32 = n1;
  o.my = vec4<f32>(0.0, x_e11, 0.0, 0.0);
  let x_e18 : f32 = n1;
  o.mz = vec4<f32>(0.0, 0.0, x_e18, 0.0);
  let x_e25 : f32 = n1;
  o.mw = vec4<f32>(0.0, 0.0, 0.0, x_e25);
  let x_e27 : Mat4x4_ = o;
  return x_e27;
}

fn x_Mat4x4_1(m16 : Mat4x3_) -> Mat4x4_ {
  var m17 : Mat4x3_;
  var o1 : Mat4x4_;
  m17 = m16;
  let x_e4 : Mat4x4_ = x_Mat4x4_(1.0);
  o1 = x_e4;
  let x_e7 : Mat4x3_ = m17;
  o1.mx = x_e7.mx;
  let x_e10 : Mat4x3_ = m17;
  o1.my = x_e10.my;
  let x_e13 : Mat4x3_ = m17;
  o1.mz = x_e13.mz;
  let x_e15 : Mat4x4_ = o1;
  return x_e15;
}

fn x_Mat4x4_2(m18 : Mat4x2_) -> Mat4x4_ {
  var m19 : Mat4x2_;
  var o2 : Mat4x4_;
  m19 = m18;
  let x_e4 : Mat4x4_ = x_Mat4x4_(1.0);
  o2 = x_e4;
  let x_e7 : Mat4x2_ = m19;
  o2.mx = x_e7.mx;
  let x_e10 : Mat4x2_ = m19;
  o2.my = x_e10.my;
  let x_e12 : Mat4x4_ = o2;
  return x_e12;
}

fn x_Mat4x3_(n2 : f32) -> Mat4x3_ {
  var n3 : f32;
  var o3 : Mat4x3_;
  n3 = n2;
  let x_e4 : f32 = n3;
  o3.mx = vec4<f32>(x_e4, 0.0, 0.0, 0.0);
  let x_e11 : f32 = n3;
  o3.my = vec4<f32>(0.0, x_e11, 0.0, 0.0);
  let x_e18 : f32 = n3;
  o3.mz = vec4<f32>(0.0, 0.0, x_e18, 0.0);
  let x_e21 : Mat4x3_ = o3;
  return x_e21;
}

fn x_Mat4x3_1(m20 : Mat4x4_) -> Mat4x3_ {
  var m21 : Mat4x4_;
  var o4 : Mat4x3_;
  m21 = m20;
  let x_e4 : Mat4x4_ = m21;
  o4.mx = x_e4.mx;
  let x_e7 : Mat4x4_ = m21;
  o4.my = x_e7.my;
  let x_e10 : Mat4x4_ = m21;
  o4.mz = x_e10.mz;
  let x_e12 : Mat4x3_ = o4;
  return x_e12;
}

fn main1() {
  var t_PosMtx : Mat4x3_;
  var t_TexSpaceCoord : vec2<f32>;
  let x_e15 : f32 = a_PosMtxIdx1;
  let x_e18 : Mat4x3_ = global2.u_PosMtx[i32(x_e15)];
  t_PosMtx = x_e18;
  let x_e23 : Mat4x3_ = t_PosMtx;
  let x_e24 : Mat4x4_ = x_Mat4x4_1(x_e23);
  let x_e25 : vec3<f32> = a_Position1;
  let x_e29 : Mat4x3_ = t_PosMtx;
  let x_e30 : Mat4x4_ = x_Mat4x4_1(x_e29);
  let x_e31 : vec3<f32> = a_Position1;
  let x_e34 : vec4<f32> = Mul(x_e30, vec4<f32>(x_e31, 1.0));
  let x_e35 : Mat4x4_ = global.u_Projection;
  let x_e37 : Mat4x3_ = t_PosMtx;
  let x_e38 : Mat4x4_ = x_Mat4x4_1(x_e37);
  let x_e39 : vec3<f32> = a_Position1;
  let x_e43 : Mat4x3_ = t_PosMtx;
  let x_e44 : Mat4x4_ = x_Mat4x4_1(x_e43);
  let x_e45 : vec3<f32> = a_Position1;
  let x_e48 : vec4<f32> = Mul(x_e44, vec4<f32>(x_e45, 1.0));
  let x_e49 : vec4<f32> = Mul(x_e35, x_e48);
  gl_Position = x_e49;
  let x_e50 : vec4<f32> = a_Color1;
  v_Color = x_e50;
  let x_e52 : vec4<f32> = global1.u_Misc0_;
  if ((x_e52.x == 2.0)) {
    {
      let x_e59 : vec3<f32> = a_Normal1;
      let x_e64 : Mat4x2_ = global1.u_TexMtx[0];
      let x_e65 : vec3<f32> = a_Normal1;
      let x_e68 : vec2<f32> = Mul2(x_e64, vec4<f32>(x_e65, 1.0));
      v_TexCoord = x_e68.xy;
      return;
    }
  } else {
    {
      let x_e73 : vec2<f32> = a_UV1;
      let x_e79 : Mat4x2_ = global1.u_TexMtx[0];
      let x_e80 : vec2<f32> = a_UV1;
      let x_e84 : vec2<f32> = Mul2(x_e79, vec4<f32>(x_e80, 1.0, 1.0));
      v_TexCoord = x_e84.xy;
      return;
    }
  }
}

@vertex
fn main(@location(0) a_Position : vec3<f32>, @location(1) a_UV : vec2<f32>, @location(2) a_Color : vec4<f32>, @location(3) a_Normal : vec3<f32>, @location(4) a_PosMtxIdx : f32) -> VertexOutput {
  a_Position1 = a_Position;
  a_UV1 = a_UV;
  a_Color1 = a_Color;
  a_Normal1 = a_Normal;
  a_PosMtxIdx1 = a_PosMtxIdx;
  main1();
  let x_e11 : vec4<f32> = v_Color;
  let x_e13 : vec2<f32> = v_TexCoord;
  let x_e15 : vec4<f32> = gl_Position;
  return VertexOutput(x_e11, x_e13, x_e15);
}
