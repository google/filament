// flags: --overrides wgsize=64
override wgsize : i32;
var<workgroup> v : array<i32, wgsize * 2>;

fn foo() -> i32 {
  return workgroupUniformLoad(&v)[0];
}
