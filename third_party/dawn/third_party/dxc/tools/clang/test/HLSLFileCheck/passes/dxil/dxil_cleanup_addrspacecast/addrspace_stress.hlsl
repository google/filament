// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Try a bunch of stuff and make sure we don't crash,
// and addrspacecast is removed.
// CHECK: @main()
// CHECK-NOT: addrspacecast
// CHECK: ret void

struct Base {
  int x;
  int getX() { return x; }
  void setX(int X) { x = X; }
};
struct Derived : Base {
  float y;
  float getYtimesX2() { return y * x * getX(); }
  void setXY(int X, float Y) { x = X; setX(X); y = Y; }
};

Derived c_Derived[2];
groupshared Derived gs_Derived0;
groupshared Derived gs_Derived1;
groupshared Derived gs_Derived[2];
groupshared float2 gs_vecArray[2][2];
groupshared float2x2 gs_matArray[2][2];

int i;

RWStructuredBuffer<Derived> sb_Derived[2];
RWStructuredBuffer<Base> sb_Base[2];

void assign(out Derived o, in Derived i) { o = i; }

[numthreads(1,1,1)]
void main() {
  gs_Derived0 = c_Derived[0];
  gs_Derived[1 - i] = c_Derived[i];
  gs_Derived[i].x = 1;
  gs_Derived[i].y = 2;
  int x = -1;
  float y = -1;
  [branch]
  if (!gs_Derived0.getX())
    assign(gs_Derived1, gs_Derived0);
  else if (gs_Derived[0].getX())
    assign(gs_Derived1, gs_Derived[i]);
  else
    assign(gs_Derived1, gs_Derived[0]);
  float f = gs_Derived1.getYtimesX2();
  gs_Derived[i].setXY(5, 6);
  gs_Derived[i].setX(f);
  sb_Derived[i][0] = gs_Derived[i];

  // Used to crash:
  // HLOperationLower(6439): in TranslateStructBufSubscriptUser
  // because it doesn't expect the bitcast to Base inside setXY when setX is called.
  sb_Derived[i][1].setXY(7, 8);

  sb_Base[i][2] = (Base)gs_Derived[i];

  [loop]
  int k = 4;
  for (int j = i; j < 4; ++j) {
    gs_vecArray[j/2][j%2] = float2(gs_Derived[j % 2].x, gs_Derived[(1 - j) % 2].y);
    gs_matArray[j/2][j%2] = float2x2(gs_vecArray[k/2][k%2], gs_vecArray[k%2][k/2]);
    k -= 1;
  }
  gs_vecArray[i][1-i] = gs_matArray[1-i][i][i];
  sb_Derived[i][1].x = gs_vecArray[1-i][k%2].x;
  sb_Derived[i][1].y = gs_vecArray[1-i][i].y;
}
