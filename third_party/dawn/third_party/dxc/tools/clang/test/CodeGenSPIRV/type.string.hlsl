// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: {{%[0-9]+}} = OpString "first string"
string first = "first string";
// CHECK: {{%[0-9]+}} = OpString "second string"
string second = "second string";
// CHECK: {{%[0-9]+}} = OpString "third string"
const string third = "third string";
// CHECK-NOT: {{%[0-9]+}} = OpString "first string"
const string a = "first string";

[numthreads(1,1,1)]
void main() {
}

