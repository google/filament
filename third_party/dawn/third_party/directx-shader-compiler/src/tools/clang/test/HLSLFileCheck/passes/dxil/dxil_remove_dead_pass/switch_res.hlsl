// RUN: %dxc /T ps_6_0 /Od %s | FileCheck %s

// CHECK: @main

Texture2D t0 : register(t0);
Texture2D t1 : register(t1);
Texture2D t2 : register(t2);
Texture2D t3 : register(t3);
Texture2D t4 : register(t4);
Texture2D t5 : register(t5);
Texture2D t6 : register(t6);

Texture2D foo(uint i) {
  switch (i) {
    case 0:
      return t0;
    case 1:
      return t1;
    case 2:
      return t2;
    case 3:
      return t3;
    case 4:
      return t4;
    case 5:
      return t5;
    case 6:
      return t6;
  }
  return t0;
}

float main(uint3 off : OFF) : SV_Target {
  return foo(5).Load(off);
}


