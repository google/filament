// RUN: rm -rf %t
// RUN: %clang_cc1 -fmodules -fimplicit-module-maps -fmodules-cache-path=%t -x objective-c %s -F %S/../Modules/Inputs -E -o - | FileCheck %s

// CHECK: int bar();
int bar();
// CHECK: @import Module; /* clang -E: implicit import for "{{.*Headers[/\\]Module.h}}" */
#include <Module/Module.h>
// CHECK: int foo();
int foo();
// CHECK: @import Module; /* clang -E: implicit import for "{{.*Headers[/\\]Module.h}}" */
#include <Module/Module.h>

#include "pp-modules.h" // CHECK: # 1 "{{.*}}pp-modules.h" 1
// CHECK: @import Module; /* clang -E: implicit import for "{{.*}}Module.h" */{{$}}
// CHECK: # 14 "{{.*}}pp-modules.c" 2
