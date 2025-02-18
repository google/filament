struct buf0 {
  injectionSwitch : vec2<f32>,
}

struct StructType {
  col : vec3<f32>,
  bbbb : vec4<bool>,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_33 : StructType;
  var x_38 : i32;
  var x_42 : StructType;
  var x_33_phi : StructType;
  var x_9_phi : i32;
  var x_42_phi : StructType;
  var x_10_phi : i32;
  x_33_phi = StructType(vec3<f32>(), vec4<bool>());
  x_9_phi = 0;
  loop {
    var x_34 : StructType;
    var x_7 : i32;
    x_33 = x_33_phi;
    let x_9 : i32 = x_9_phi;
    let x_37 : f32 = x_5.injectionSwitch.y;
    x_38 = i32(x_37);
    if ((x_9 < x_38)) {
    } else {
      break;
    }

    continuing {
      x_34 = x_33;
      x_34.col = vec3<f32>(1.0, 0.0, 0.0);
      x_7 = (x_9 + 1);
      x_33_phi = x_34;
      x_9_phi = x_7;
    }
  }
  x_42_phi = x_33;
  x_10_phi = 0;
  loop {
    var x_43 : StructType;
    var x_8 : i32;
    x_42 = x_42_phi;
    let x_10 : i32 = x_10_phi;
    if ((x_10 < x_38)) {
    } else {
      break;
    }

    continuing {
      x_43 = x_42;
      x_43.col = vec3<f32>(1.0, 0.0, 0.0);
      x_8 = (x_10 + 1);
      x_42_phi = x_43;
      x_10_phi = x_8;
    }
  }
  let x_47 : vec3<f32> = x_42.col;
  x_GLF_color = vec4<f32>(x_47.x, x_47.y, x_47.z, 1.0);
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
