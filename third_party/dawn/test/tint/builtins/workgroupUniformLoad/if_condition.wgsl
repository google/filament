var<workgroup> v : bool;

fn foo() -> i32 {
  if (workgroupUniformLoad(&v)) {
    return 42;
  }
  return 0;
}
