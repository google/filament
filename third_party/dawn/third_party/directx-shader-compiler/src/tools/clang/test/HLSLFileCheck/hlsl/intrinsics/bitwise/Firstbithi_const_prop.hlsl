// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} i32 189

[RootSignature("")]
int main() : SV_Target {
    int x = 0x0;           // firstbithigh(y) = -1
    int y = 0x0010;        // firstbithigh(y) =  4
    int z = 0x80000000;    // firstbithigh(z) = 30
    int w = 0xffffffff;    // firstbithigh(w) = -1
    
    uint ux = 0x0;         // firstbithigh(ux) = -1
    uint uy = 0x1000;      // firstbithigh(uy) = 12
    uint uz = 0x80000000;  // firstbithigh(uz) = 31
    uint uw = 0xffffffff;  // firstbithigh(uw) = 31
    
    // TODO: add tests for int64_t when constant literals are fixed
    
    uint64_t lux = 0x0ULL;           // firstbithigh(lux) = -1
    uint64_t luy = 0x1000ULL;        // firstbithigh(luy) = 12
    uint64_t luz = 0x00100000000ULL; // firstbithi(luz) = 32
    uint64_t luw = 0x30000000000ULL; // firstbithi(luw) = 41
    
    
    return firstbithigh(x)
         + firstbithigh(y)
         + firstbithigh(z)
         + firstbithigh(w)
         + firstbithigh(ux)
         + firstbithigh(uy)
         + firstbithigh(uz)
         + firstbithigh(uw)
         + firstbithigh(lux)
         + firstbithigh(luy)
         + firstbithigh(luz)
         + firstbithigh(luw)
         ;
}