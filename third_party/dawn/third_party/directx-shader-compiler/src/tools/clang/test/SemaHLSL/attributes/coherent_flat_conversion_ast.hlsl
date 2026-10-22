// RUN: %dxc -T lib_6_9 -ast-dump %s | FileCheck %s
// REQUIRES: dxil-1-9

// Initializing a coherent resource from ResourceDescriptorHeap[] produces an
// ICK_Flat_Conversion. Verify the converted expression's type still carries the
// coherence qualifier of its destination instead of dropping it.

[shader("raygeneration")]
void main()
{
  // CHECK: VarDecl {{.*}} used glcBuf 'globallycoherent RWByteAddressBuffer':'RWByteAddressBuffer' cinit
  // CHECK-NEXT: ImplicitCastExpr {{.*}} 'globallycoherent RWByteAddressBuffer':'RWByteAddressBuffer' <FlatConversion>
  globallycoherent RWByteAddressBuffer glcBuf = ResourceDescriptorHeap[0];
  glcBuf.Store(0, 0);

  // CHECK: VarDecl {{.*}} used rcBuf 'reordercoherent RWBuffer<float4>':'RWBuffer<vector<float, 4> >' cinit
  // CHECK-NEXT: ImplicitCastExpr {{.*}} 'reordercoherent RWBuffer<vector<float, 4> >':'RWBuffer<vector<float, 4> >' <FlatConversion>
  reordercoherent RWBuffer<float4> rcBuf = ResourceDescriptorHeap[1];
  rcBuf[0] = 5;
}
