// RUN: %dxc -T ps_6_6 -E main -HV 2021 -ast-dump %s | FileCheck %s
template <typename T> struct MyTex2D {
  uint heapId;

  template <typename Arg0> T Load(Arg0 arg0) { return Get().Load(arg0); }

  Texture2D<T> Get() { return (Texture2D<T>)ResourceDescriptorHeap[heapId]; }
};

cbuffer constantBuffer : register(b0) {
  MyTex2D<float4> tex;
};

float4 main() : SV_Target {
  float4 output = tex.Load(float3(0, 0, 0));
  return output;
}

// CHECK:      CXXMemberCallExpr 0x{{[0-9a-fA-F]+}} <col:55, col:70> 'vector<float, 4>'
// CHECK-NEXT: MemberExpr 0x{{[0-9a-fA-F]+}} <col:55, col:61> '<bound member function type>' .Load
// CHECK-NEXT: CXXMemberCallExpr 0x{{[0-9a-fA-F]+}} <col:55, col:59> 'Texture2D<vector<float, 4> >':'Texture2D<vector<float, 4> >'
// CHECK-NEXT: MemberExpr 0x{{[0-9a-fA-F]+}} <col:55> '<bound member function type>' .Get
// CHECK-NEXT: CXXThisExpr 0x{{[0-9a-fA-F]+}} <col:55> 'MyTex2D<vector<float, 4>
// >' lvalue this
