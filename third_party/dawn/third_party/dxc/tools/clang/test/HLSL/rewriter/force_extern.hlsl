
float a;

namespace b {
   float c;
   namespace d {
      float e;
   }
}

static f = a;

float4 main() : SV_Target {
    return a + c + e + f;
}