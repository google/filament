struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 6u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var arr : array<i32, 3u>;
  var index : i32;
  var x_76 : bool;
  var x_86 : bool;
  var x_77_phi : bool;
  var x_87_phi : bool;
  let x_33 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_35 : i32 = x_6.x_GLF_uniform_int_values[5].el;
  let x_37 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  arr = array<i32, 3u>(x_33, x_35, x_37);
  index = 1;
  loop {
    var x_51 : bool;
    var x_52_phi : bool;
    x_52_phi = true;
    if (true) {
      let x_46 : i32 = x_6.x_GLF_uniform_int_values[0].el;
      let x_48 : i32 = index;
      x_51 = !(((x_46 == 1) & (x_48 <= 1)));
      x_52_phi = x_51;
    }
    let x_52 : bool = x_52_phi;
    if (!(x_52)) {
    } else {
      break;
    }
    let x_55 : i32 = index;
    let x_56_save = x_55;
    let x_57 : i32 = arr[x_56_save];
    arr[x_56_save] = (x_57 + 1);
    let x_59 : i32 = index;
    index = (x_59 + 1);
  }
  let x_62 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_64 : i32 = arr[x_62];
  let x_66 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_67 : bool = (x_64 == x_66);
  x_77_phi = x_67;
  if (x_67) {
    let x_71 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_73 : i32 = arr[x_71];
    let x_75 : i32 = x_6.x_GLF_uniform_int_values[4].el;
    x_76 = (x_73 == x_75);
    x_77_phi = x_76;
  }
  let x_77 : bool = x_77_phi;
  x_87_phi = x_77;
  if (x_77) {
    let x_81 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_83 : i32 = arr[x_81];
    let x_85 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_86 = (x_83 == x_85);
    x_87_phi = x_86;
  }
  let x_87 : bool = x_87_phi;
  if (x_87) {
    let x_92 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_95 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_98 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_101 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_92), f32(x_95), f32(x_98), f32(x_101));
  } else {
    let x_105 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_106 : f32 = f32(x_105);
    x_GLF_color = vec4<f32>(x_106, x_106, x_106, x_106);
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
