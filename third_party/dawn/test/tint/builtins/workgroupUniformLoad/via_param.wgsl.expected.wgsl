var<workgroup> v : array<i32, 4>;

fn foo(p : ptr<workgroup, i32>) -> i32 {
  return workgroupUniformLoad(p);
}

fn bar() -> i32 {
  return foo(&(v[0]));
}
