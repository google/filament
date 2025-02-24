struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 7u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v1 : vec4<f32>;
  var v2 : vec4<f32>;
  var v3 : vec4<f32>;
  var v4 : vec4<f32>;
  var x_69 : bool;
  var x_77 : bool;
  var x_85 : bool;
  var x_93 : bool;
  var x_70_phi : bool;
  var x_78_phi : bool;
  var x_86_phi : bool;
  var x_94_phi : bool;
  let x_41 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  let x_43 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  let x_45 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_47 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  v1 = vec4<f32>(x_41, x_43, x_45, x_47);
  v2 = vec4<f32>(1.570796371, 1.119769573, 1.0, 0.927295208);
  let x_50 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  v3 = vec4<f32>(x_50, x_50, x_50, x_50);
  let x_52 : vec4<f32> = v1;
  let x_53 : vec4<f32> = v2;
  let x_54 : vec4<f32> = v3;
  v4 = smoothstep(x_52, x_53, x_54);
  let x_56 : vec4<f32> = v4;
  x_GLF_color = vec4<f32>(x_56.x, x_56.y, x_56.w, x_56.x);
  let x_59 : f32 = v4.x;
  let x_61 : f32 = x_6.x_GLF_uniform_float_values[4].el;
  let x_62 : bool = (x_59 > x_61);
  x_70_phi = x_62;
  if (x_62) {
    let x_66 : f32 = v4.x;
    let x_68 : f32 = x_6.x_GLF_uniform_float_values[5].el;
    x_69 = (x_66 < x_68);
    x_70_phi = x_69;
  }
  let x_70 : bool = x_70_phi;
  x_78_phi = x_70;
  if (x_70) {
    let x_74 : f32 = v4.y;
    let x_76 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    x_77 = (x_74 > x_76);
    x_78_phi = x_77;
  }
  let x_78 : bool = x_78_phi;
  x_86_phi = x_78;
  if (x_78) {
    let x_82 : f32 = v4.y;
    let x_84 : f32 = x_6.x_GLF_uniform_float_values[6].el;
    x_85 = (x_82 < x_84);
    x_86_phi = x_85;
  }
  let x_86 : bool = x_86_phi;
  x_94_phi = x_86;
  if (x_86) {
    let x_90 : f32 = v4.w;
    let x_92 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    x_93 = (x_90 == x_92);
    x_94_phi = x_93;
  }
  let x_94 : bool = x_94_phi;
  if (x_94) {
    let x_99 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_101 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_103 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_105 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4<f32>(x_99, x_101, x_103, x_105);
  } else {
    let x_108 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    x_GLF_color = vec4<f32>(x_108, x_108, x_108, x_108);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
