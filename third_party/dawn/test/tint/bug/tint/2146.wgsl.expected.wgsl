enable f16;

var<private> localId : vec3<u32>;

var<private> localIndex : u32;

var<private> globalId : vec3<u32>;

var<private> numWorkgroups : vec3<u32>;

var<private> workgroupId : vec3<u32>;

fn globalId2Index() -> u32 {
  return globalId.x;
}

@compute @workgroup_size(1, 1, 1)
fn main() {
  var a = vec4<f16>(0, 0, 0, 0);
  let b = (f16(0) + f16(1));
  a[0] += b;
}
