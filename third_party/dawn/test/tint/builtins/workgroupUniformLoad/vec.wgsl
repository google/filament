var<workgroup> v : vec4<f32>;

fn foo() -> vec4<f32> {
  return workgroupUniformLoad(&v);
}
