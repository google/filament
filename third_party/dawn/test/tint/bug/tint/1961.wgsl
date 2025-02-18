const TRUE = true;
const FALSE = false;
const_assert(true || FALSE);
const_assert(!(false && true));

fn f() {
  var x = false;
  var y = false;
  if (x && (true || y)) { }
}
