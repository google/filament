struct Scene {
  vEyePosition : vec4<f32>,
};

struct Material {
  vDiffuseColor : vec4<f32>,
  vAmbientColor : vec3<f32>,
  placeholder: f32,
  vEmissiveColor : vec3<f32>,
  placeholder2: f32,
};

struct Mesh {
  visibility : f32,
};

var<private> fClipDistance3 : f32;

var<private> fClipDistance4 : f32;

@group(0) @binding(0) var<uniform> x_29 : Scene;

@group(0) @binding(1) var<uniform> x_49 : Material;

@group(0) @binding(2) var<uniform> x_137 : Mesh;

var<private> glFragColor : vec4<f32>;

fn main_1() {
  var viewDirectionW : vec3<f32>;
  var baseColor : vec4<f32>;
  var diffuseColor : vec3<f32>;
  var alpha : f32;
  var normalW : vec3<f32>;
  var uvOffset : vec2<f32>;
  var baseAmbientColor : vec3<f32>;
  var glossiness : f32;
  var diffuseBase : vec3<f32>;
  var shadow : f32;
  var refractionColor : vec4<f32>;
  var reflectionColor : vec4<f32>;
  var emissiveColor : vec3<f32>;
  var finalDiffuse : vec3<f32>;
  var finalSpecular : vec3<f32>;
  var color : vec4<f32>;
  let x_9 : f32 = fClipDistance3;
  if ((x_9 > 0.0)) {
    discard;
  }
  let x_17 : f32 = fClipDistance4;
  if ((x_17 > 0.0)) {
    discard;
  }
  let x_34 : vec4<f32> = x_29.vEyePosition;
  let x_38 : vec3<f32> = vec3<f32>(0., 0., 0.);
  viewDirectionW = normalize((vec3<f32>(x_34.x, x_34.y, x_34.z) - x_38));
  baseColor = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  let x_52 : vec4<f32> = x_49.vDiffuseColor;
  diffuseColor = vec3<f32>(x_52.x, x_52.y, x_52.z);
  let x_60 : f32 = x_49.vDiffuseColor.w;
  alpha = x_60;
  let x_62 : vec3<f32> = vec3<f32>(0., 0., 0.);
  let x_64 : vec3<f32> = vec3<f32>(0., 0., 0.);
  // Violates uniformity analysis:
  // normalW = normalize(-(cross(dpdx(x_62), dpdy(x_64))));
  uvOffset = vec2<f32>(0.0, 0.0);
  let x_74 : vec4<f32> = vec4<f32>(0., 0., 0., 0.);
  let x_76 : vec4<f32> = baseColor;
  let x_78 : vec3<f32> = (vec3<f32>(x_76.x, x_76.y, x_76.z) * vec3<f32>(x_74.x, x_74.y, x_74.z));
  let x_79 : vec4<f32> = baseColor;
  baseColor = vec4<f32>(x_78.x, x_78.y, x_78.z, x_79.w);
  baseAmbientColor = vec3<f32>(1.0, 1.0, 1.0);
  glossiness = 0.0;
  diffuseBase = vec3<f32>(0.0, 0.0, 0.0);
  shadow = 1.0;
  refractionColor = vec4<f32>(0.0, 0.0, 0.0, 1.0);
  reflectionColor = vec4<f32>(0.0, 0.0, 0.0, 1.0);
  let x_94 : vec3<f32> = x_49.vEmissiveColor;
  emissiveColor = x_94;
  let x_96 : vec3<f32> = diffuseBase;
  let x_97 : vec3<f32> = diffuseColor;
  let x_99 : vec3<f32> = emissiveColor;
  let x_103 : vec3<f32> = x_49.vAmbientColor;
  let x_108 : vec4<f32> = baseColor;
  finalDiffuse = (clamp((((x_96 * x_97) + x_99) + x_103), vec3<f32>(0.0, 0.0, 0.0), vec3<f32>(1.0, 1.0, 1.0)) * vec3<f32>(x_108.x, x_108.y, x_108.z));
  finalSpecular = vec3<f32>(0.0, 0.0, 0.0);
  let x_113 : vec3<f32> = finalDiffuse;
  let x_114 : vec3<f32> = baseAmbientColor;
  let x_116 : vec3<f32> = finalSpecular;
  let x_118 : vec4<f32> = reflectionColor;
  let x_121 : vec4<f32> = refractionColor;
  let x_123 : vec3<f32> = ((((x_113 * x_114) + x_116) + vec3<f32>(x_118.x, x_118.y, x_118.z)) + vec3<f32>(x_121.x, x_121.y, x_121.z));
  let x_124 : f32 = alpha;
  color = vec4<f32>(x_123.x, x_123.y, x_123.z, x_124);
  let x_129 : vec4<f32> = color;
  let x_132 : vec3<f32> = max(vec3<f32>(x_129.x, x_129.y, x_129.z), vec3<f32>(0.0, 0.0, 0.0));
  let x_133 : vec4<f32> = color;
  color = vec4<f32>(x_132.x, x_132.y, x_132.z, x_133.w);
  let x_140 : f32 = x_137.visibility;
  let x_142 : f32 = color.w;
  color.w = (x_142 * x_140);
  let x_147 : vec4<f32> = color;
  glFragColor = x_147;
  return;
}

struct main_out {
  @location(0)
  glFragColor_1 : vec4<f32>,
};

@fragment
fn main(@location(2) fClipDistance3_param : f32, @location(3) fClipDistance4_param : f32) -> main_out {
  fClipDistance3 = fClipDistance3_param;
  fClipDistance4 = fClipDistance4_param;
  main_1();
  return main_out(glFragColor);
}
