// RUN: %dxc -E main -T ps_6_0 -Zi %s | FileCheck %s

// Make sure has 3 files in dx.source.contents instead of 4 which mix use / and \\ for same header.
//CHECK:!dx.source.contents = !{!{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}}

#include "inc_dir\inc_dir.h"
#include "inc_dir/inc_dir2.h"

float main(float a:A) : SV_Target {
  return foo(a) + bar2(a);
}