var<workgroup> v : mat3x3<f32>;

fn foo() -> mat3x3<f32> {
  return workgroupUniformLoad(&v);
}
