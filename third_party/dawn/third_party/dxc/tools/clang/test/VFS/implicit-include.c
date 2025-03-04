// RUN: sed -e "s:INPUT_DIR:%S/Inputs:g" -e "s:OUT_DIR:%t:g" %S/Inputs/vfsoverlay.yaml > %t.yaml
// RUN: %clang_cc1 -Werror -ivfsoverlay %t.yaml -I %t -include "not_real.h" -fsyntax-only %s
// REQUIRES: shell

void foo() {
  bar();
}
