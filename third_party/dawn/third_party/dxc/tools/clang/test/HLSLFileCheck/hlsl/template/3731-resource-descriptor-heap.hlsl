// RUN: %dxc -T cs_6_6 -E main -HV 2021 -ast-dump %s | FileCheck %s

struct SimpleBuffer {
  uint handle;

  template<typename T>
  T Load() {
    ByteAddressBuffer buf = ResourceDescriptorHeap[handle];
    return buf.Load<T>(0);
  }
};

[numthreads(1,1,1)]
void main() {
  SimpleBuffer b = (SimpleBuffer)0;
  b.Load<uint>();
}

// CHECK:      CXXOperatorCallExpr 0x{{[0-9a-fA-F]+}} <col:29, col:58> 'const .Resource'
// CHECK-NEXT: ImplicitCastExpr 0x{{[0-9a-fA-F]+}} <col:51, col:58> 'const .Resource (*)(unsigned int) const' <FunctionToPointerDecay>
// CHECK-NEXT: DeclRefExpr 0x{{[0-9a-fA-F]+}} <col:51, col:58> 'const .Resource (unsigned int) const' lvalue CXXMethod 0x{{[0-9a-fA-F]+}} 'operator[]' 'const .Resource (unsigned int) const'
// CHECK-NEXT: ImplicitCastExpr 0x{{[0-9a-fA-F]+}} <col:29> 'const .Resource' lvalue <NoOp>
// CHECK-NEXT: DeclRefExpr 0x{{[0-9a-fA-F]+}} <col:29> '.Resource' lvalue Var 0x{{[0-9a-fA-F]+}} 'ResourceDescriptorHeap' '.Resource'
// CHECK-NEXT: ImplicitCastExpr 0x{{[0-9a-fA-F]+}} <col:52> 'uint':'unsigned int' <LValueToRValue>
// CHECK-NEXT: MemberExpr 0x{{[0-9a-fA-F]+}} <col:52> 'uint':'unsigned int' lvalue .handle
// CHECK-NEXT: CXXThisExpr 0x{{[0-9a-fA-F]+}} <col:52> 'SimpleBuffer

// CHECK:      CXXOperatorCallExpr 0x{{[0-9a-fA-F]+}} <col:29, col:58> 'const .Resource'
// CHECK-NEXT: ImplicitCastExpr 0x{{[0-9a-fA-F]+}} <col:51, col:58> 'const .Resource (*)(unsigned int) const' <FunctionToPointerDecay>
// CHECK-NEXT: DeclRefExpr 0x{{[0-9a-fA-F]+}} <col:51, col:58> 'const .Resource (unsigned int) const' lvalue CXXMethod 0x{{[0-9a-fA-F]+}} 'operator[]' 'const .Resource (unsigned int) const'
// CHECK-NEXT: ImplicitCastExpr 0x{{[0-9a-fA-F]+}} <col:29> 'const .Resource' lvalue <NoOp>
// CHECK-NEXT: DeclRefExpr 0x{{[0-9a-fA-F]+}} <col:29> '.Resource' lvalue Var 0x{{[0-9a-fA-F]+}} 'ResourceDescriptorHeap' '.Resource'
// CHECK-NEXT: ImplicitCastExpr 0x{{[0-9a-fA-F]+}} <col:52> 'uint':'unsigned int' <LValueToRValue>
// CHECK-NEXT: MemberExpr 0x{{[0-9a-fA-F]+}} <col:52> 'uint':'unsigned int' lvalue .handle
// CHECK-NEXT: CXXThisExpr 0x{{[0-9a-fA-F]+}} <col:52> 'SimpleBuffer
