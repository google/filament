struct struct_base {
  data : i32,
  leftIndex : i32,
  rightIndex : i32,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> struct_array : array<struct_base, 3u>;

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var index : i32;
  struct_array = array<struct_base, 3u>(struct_base(1, 1, 1), struct_base(1, 1, 1), struct_base(1, 1, 1));
  index = 1;
  struct_array[1].rightIndex = 1;
  let x_39 : i32 = struct_array[1].leftIndex;
  if ((x_39 == 1)) {
    let x_45 : f32 = x_8.injectionSwitch.x;
    let x_48 : i32 = struct_array[i32(x_45)].rightIndex;
    index = x_48;
  } else {
    let x_50 : f32 = x_8.injectionSwitch.y;
    let x_53 : i32 = struct_array[i32(x_50)].leftIndex;
    index = x_53;
  }
  let x_55 : i32 = struct_array[1].leftIndex;
  if ((x_55 == 1)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
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
