var<workgroup> v : bool;

fn foo() -> bool {
  return workgroupUniformLoad(&v);
}
