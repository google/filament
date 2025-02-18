fn foo(x : f32) {
  switch (i32(x)) {
    default {
    }
  }
}

var<private> global : i32;
fn baz(x : i32) -> i32 {
    global = 42;
    return x;
}

fn bar(x : f32) {
  switch (baz(i32(x))) {
    default {
    }
  }
}

fn main() {
}
