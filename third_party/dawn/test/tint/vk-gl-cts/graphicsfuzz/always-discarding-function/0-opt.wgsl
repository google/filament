struct tmp_struct {
  nmb : array<i32, 1u>,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_11 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_24 : array<i32, 1u>;
  var x_68 : bool = false;
  var x_17 : i32;
  var x_18 : i32;
  var x_19 : i32;
  var x_20 : i32;
  var x_69 : bool = false;
  var tmp_float : f32;
  var color : vec3<f32>;
  loop {
    var x_25 : i32;
    var x_101 : vec3<f32>;
    var x_79_phi : bool;
    var x_26_phi : i32;
    let x_75 : f32 = x_11.injectionSwitch.y;
    tmp_float = x_75;
    let x_76 : vec3<f32> = vec3<f32>(x_75, x_75, x_75);
    color = x_76;
    x_24 = tmp_struct(array<i32, 1u>()).nmb;
    x_68 = false;
    x_79_phi = false;
    loop {
      var x_21_phi : i32;
      var x_25_phi : i32;
      var x_93_phi : bool;
      let x_79 : bool = x_79_phi;
      x_18 = 1;
      x_21_phi = 1;
      loop {
        let x_21 : i32 = x_21_phi;
        x_25_phi = 0;
        x_93_phi = x_79;
        if ((x_21 > 10)) {
        } else {
          break;
        }
        let x_22 : i32 = (x_21 - 1);
        x_19 = x_22;
        let x_23 : i32 = x_24[x_22];
        if ((x_23 == 1)) {
          x_68 = true;
          x_17 = 1;
          x_25_phi = 1;
          x_93_phi = true;
          break;
        }
        x_18 = x_22;

        continuing {
          x_21_phi = x_22;
        }
      }
      x_25 = x_25_phi;
      let x_93 : bool = x_93_phi;
      x_26_phi = x_25;
      if (x_93) {
        break;
      }
      x_68 = true;
      x_17 = -1;
      x_26_phi = -1;
      break;

      continuing {
        x_79_phi = false;
      }
    }
    let x_26 : i32 = x_26_phi;
    x_20 = x_26;
    if ((x_26 == -1)) {
      discard;
    } else {
      x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
      let x_100 : vec2<f32> = (vec2<f32>(x_76.y, x_76.z) + vec2<f32>(1.0, 1.0));
      x_101 = vec3<f32>(x_76.x, x_100.x, x_100.y);
      color = x_101;
      let x_103 : f32 = x_11.injectionSwitch.x;
      if ((x_103 > 1.0)) {
        x_69 = true;
        break;
      }
    }
    x_GLF_color = vec4<f32>(x_101.x, x_101.y, x_101.z, 1.0);
    x_69 = true;
    break;
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

fn binarySearch_struct_tmp_struct_i1_1_1_(obj : ptr<function, tmp_struct>) -> i32 {
  var x_112 : bool = false;
  var x_16 : i32;
  var one : i32;
  var zero : i32;
  var x_27 : i32;
  var x_114_phi : bool;
  var x_28_phi : i32;
  x_114_phi = false;
  loop {
    var x_15_phi : i32;
    var x_27_phi : i32;
    var x_128_phi : bool;
    let x_114 : bool = x_114_phi;
    one = 1;
    x_15_phi = 1;
    loop {
      let x_15 : i32 = x_15_phi;
      x_27_phi = 0;
      x_128_phi = x_114;
      if ((x_15 > 10)) {
      } else {
        break;
      }
      let x_13 : i32 = (x_15 - 1);
      zero = x_13;
      let x_14 : i32 = (*(obj)).nmb[x_13];
      if ((x_14 == 1)) {
        x_112 = true;
        x_16 = 1;
        x_27_phi = 1;
        x_128_phi = true;
        break;
      }
      one = x_13;

      continuing {
        x_15_phi = x_13;
      }
    }
    x_27 = x_27_phi;
    let x_128 : bool = x_128_phi;
    x_28_phi = x_27;
    if (x_128) {
      break;
    }
    x_112 = true;
    x_16 = -1;
    x_28_phi = -1;
    break;

    continuing {
      x_114_phi = false;
    }
  }
  let x_28 : i32 = x_28_phi;
  return x_28;
}
