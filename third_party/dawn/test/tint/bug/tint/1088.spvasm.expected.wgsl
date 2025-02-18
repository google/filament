alias Arr = array<mat4x4f, 2u>;

struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr, 4u>;

struct LeftOver {
  /* @offset(0) */
  worldViewProjection : mat4x4f,
  /* @offset(64) */
  time : f32,
  /* @offset(80) */
  test2 : Arr,
  /* @offset(208) */
  test : Arr_1,
}

var<private> position_1 : vec3f;

@group(2) @binding(2) var<uniform> x_14 : LeftOver;

var<private> vUV : vec2f;

var<private> uv : vec2f;

var<private> normal : vec3f;

var<private> gl_Position : vec4f;

fn main_1() {
  var q : vec4f;
  var p : vec3f;
  q = vec4f(position_1.x, position_1.y, position_1.z, 1.0f);
  p = q.xyz;
  p.x = (p.x + sin(((x_14.test[0i].el * position_1.y) + x_14.time)));
  p.y = (p.y + sin((x_14.time + 4.0f)));
  gl_Position = (x_14.worldViewProjection * vec4f(p.x, p.y, p.z, 1.0f));
  vUV = uv;
  gl_Position.y = (gl_Position.y * -1.0f);
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4f,
  @location(0)
  vUV_1 : vec2f,
}

@vertex
fn main(@location(0) position_1_param : vec3f, @location(2) uv_param : vec2f, @location(1) normal_param : vec3f) -> main_out {
  position_1 = position_1_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  return main_out(gl_Position, vUV);
}
