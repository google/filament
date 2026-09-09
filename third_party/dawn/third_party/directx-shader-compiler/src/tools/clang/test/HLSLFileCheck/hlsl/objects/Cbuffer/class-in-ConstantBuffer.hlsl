// RUN: %dxc -E main -T vs_6_0 -ast-dump %s | FileCheck %s

// CHECK: VarDecl 0x{{[0-9a-zA-Z]+}} {{.*}} used cbv 'ConstantBuffer<CBType>':'ConstantBuffer<CBType>'

class Inner {
  int a;
};

class CBType {
  Inner m;
};

ConstantBuffer<CBType> cbv;

int main() : OUT {
  return cbv.m.a;
}
