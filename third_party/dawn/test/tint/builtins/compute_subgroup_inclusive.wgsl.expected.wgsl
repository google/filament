enable subgroups;

@compute @workgroup_size(1)
fn main() {
  let val : f32 = 2.0;
  let subadd : f32 = subgroupInclusiveAdd(val);
  let submul : f32 = subgroupInclusiveMul(val);
}
