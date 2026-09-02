// RUN: %dxc /T ps_6_0 /Od %s | FileCheck %s

// CHECK: @main

Texture2D t0 : register(t0);
Texture2D t1 : register(t1);
Texture2D t2 : register(t2);
Texture2D t3 : register(t3);
Texture2D t4 : register(t4);
Texture2D t5 : register(t5);
Texture2D t6 : register(t6);


static Texture2D textures[] = {
  t0, t1, t2, t3, t4, t5, t6,
};

Texture2D foo(uint i) {
  switch (i) {
    case 0:
      return textures[0];
    case 1:
      return textures[1];
    case 2:
      return textures[2];
    case 3:
      return textures[3];
    case 4:
      return textures[4];
  }
  return textures[0];
}

float main(uint3 off : OFF) : SV_Target {
  return foo(3).Load(off);
}


