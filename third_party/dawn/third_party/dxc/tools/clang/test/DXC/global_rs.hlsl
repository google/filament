// RUN:%dxc -T rootsig_1_1 %s -rootsig-define main -Fo %t
// RUN:%dxa  -listparts %t | FileCheck %s

#define main "CBV(b0)"

// CHECK:Part count: 1
// CHECK:#0 - RTS0
