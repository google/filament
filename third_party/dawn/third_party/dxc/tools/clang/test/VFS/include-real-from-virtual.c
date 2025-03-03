// RUN: rm -rf %t
// RUN: mkdir -p %t
// RUN: echo "void baz(void);" > %t/real.h
// RUN: sed -e "s:INPUT_DIR:%S/Inputs:g" -e "s:OUT_DIR:%t:g" %S/Inputs/vfsoverlay.yaml > %t.yaml
// RUN: %clang_cc1 -Werror -ivfsoverlay %t.yaml -I %t -fsyntax-only %s
// REQUIRES: shell

#include "include_real.h"

void foo() {
  baz();
}
