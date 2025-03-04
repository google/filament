// RUN: %dxc /T ps_6_0 /Od %s | FileCheck %s

// CHECK: @main

Texture2D t0 : register(t0);
Texture2D t1 : register(t1);
Texture2D t2 : register(t2);
Texture2D t3 : register(t3);
Texture2D t4 : register(t4);
Texture2D t5 : register(t5);
Texture2D t6 : register(t6);

float foo(uint i) {
  switch (i) {
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 3;
    case 3:
      return 4;
    case 4:
      return 5;
  }
  return 0;
}

float main(uint3 off : OFF) : SV_Target {

  Texture2D textures[] = {
    t0, t1, t2, t3, t4, t5, t6,
  };

  return textures[foo(10)].Load(off);
}


