// flags: --overrides wgsize=64
override wgsize : i32;
alias Array = array<i32, wgsize * 2>;
var<workgroup> v : Array;

fn foo() -> i32 {
  return workgroupUniformLoad(&v)[0];
}
