struct theSSBO {
  out_data : i32,
}

struct buf1 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<storage, read_write> x_4 : theSSBO;

@group(0) @binding(1) var<uniform> x_6 : buf1;

fn main_1() {
  var i : i32;
  var GLF_live3s : f32;
  var i_1 : i32;
  var z : i32;
  x_4.out_data = 42;
  let x_37 : f32 = x_6.injectionSwitch.x;
  let x_39 : f32 = x_6.injectionSwitch.y;
  if ((x_37 > x_39)) {
    workgroupBarrier();
  }
  let x_44 : f32 = x_6.injectionSwitch.x;
  let x_46 : f32 = x_6.injectionSwitch.y;
  if ((x_44 > x_46)) {
    workgroupBarrier();
  }
  let x_51 : f32 = x_6.injectionSwitch.y;
  i = i32(x_51);
  loop {
    let x_57 : i32 = i;
    if ((x_57 > 0)) {
    } else {
      break;
    }
    workgroupBarrier();

    continuing {
      let x_60 : i32 = i;
      i = (x_60 - 1);
    }
  }
  GLF_live3s = 0.0;
  loop {
    i_1 = 1;
    loop {
      let x_70 : i32 = i_1;
      if ((x_70 < 2)) {
      } else {
        break;
      }
      let x_74 : f32 = x_6.injectionSwitch.x;
      if ((x_74 > 1.0)) {
        workgroupBarrier();
      }
      let x_79 : f32 = x_6.injectionSwitch.x;
      let x_81 : f32 = x_6.injectionSwitch.y;
      if ((x_79 > x_81)) {
        workgroupBarrier();
      }
      let x_86 : f32 = x_6.injectionSwitch.y;
      z = i32(x_86);
      loop {
        let x_92 : i32 = z;
        if ((x_92 > 0)) {
        } else {
          break;
        }
        let x_95 : f32 = GLF_live3s;
        GLF_live3s = (x_95 + 1.0);

        continuing {
          let x_97 : i32 = z;
          z = (x_97 - 1);
        }
      }
      let x_99 : i32 = i_1;
      if ((x_99 >= 1)) {
        let x_104 : f32 = x_6.injectionSwitch.x;
        if ((x_104 > 1.0)) {
          workgroupBarrier();
        }
      }

      continuing {
        let x_108 : i32 = i_1;
        i_1 = (x_108 + 1);
      }
    }

    continuing {
      let x_111 : f32 = x_6.injectionSwitch.x;
      let x_113 : f32 = x_6.injectionSwitch.y;
      break if !(x_111 > x_113);
    }
  }
  return;
}

@compute @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
