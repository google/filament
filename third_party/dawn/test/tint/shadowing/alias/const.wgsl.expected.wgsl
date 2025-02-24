alias a = i32;

fn f() {
  {
    const a : a = a();
    const b = a;
  }
  const a : a = a();
  const b = a;
}
