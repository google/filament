// RUN: %dxc -T vs_6_0 -E main -ast-dump %s | FileCheck %s

// CHECK: VarDecl 0x{{[0-9a-zA-Z]+}} {{.*}} used sb 'StructuredBuffer<Element>':'StructuredBuffer<Element>'
// CHECK-NEXT: VarDecl 0x{{[0-9a-zA-Z]+}} {{.*}} used rwsb 'RWStructuredBuffer<Element>':'RWStructuredBuffer<Element>'

class Element
{
    int e;
};

StructuredBuffer<Element> sb;
RWStructuredBuffer<Element> rwsb;

int main() : OUT
{
    return sb[0].e + rwsb[0].e;
}
