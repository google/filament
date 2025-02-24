var<workgroup> v : array<i32, 4>;

fn foo() -> array<i32, 4> {
  return workgroupUniformLoad(&v);
}
