// REQUIRES: dxil-1-8
// RUN: %dxc -T lib_6_7 -disable-payload-qualifiers %s 2>&1 | FileCheck %s
// RUN: %dxc -T lib_6_8 -disable-payload-qualifiers %s 2>&1 | FileCheck %s

// Ensures that -disable-payload-qualifiers works properly for SM6.6+ versions


// CHECK: warning: payload access qualifiers ignored. These are only supported for lib_6_7+ targets and lib_6_6 with with the -enable-payload-qualifiers flag.

// CHECK-NOT: !dx.dxrPayloadAnnotations

struct [raypayload] Payload {
    int a : read(caller) : write(caller, miss);
};

[shader("miss")]
void main( inout Payload payload ) {
  payload.a = 0.0;
}