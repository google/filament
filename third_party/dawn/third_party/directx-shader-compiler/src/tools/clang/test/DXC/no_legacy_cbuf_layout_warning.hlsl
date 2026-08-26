// RUN: %dxc -T vs_6_0 %s -no-legacy-cbuf-layout 2>&1 | FileCheck %s
// RUN: %dxc -T vs_6_0 %s -not_use_legacy_cbuf_load 2>&1 | FileCheck %s

// CHECK: warning: -no-legacy-cbuf-layout is no longer supported and will be ignored. Future releases will not recognize it.

void main() {}
