// RUN: %dxc -E main -T lib_6_3 %s -H | FileCheck %s
#include "Inputs/empty.h"

// CHECK: ; Opening file [{{.*}}/Inputs/empty.h], stack top [0]
