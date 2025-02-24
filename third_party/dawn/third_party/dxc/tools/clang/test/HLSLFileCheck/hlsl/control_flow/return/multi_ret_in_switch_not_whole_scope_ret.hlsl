// RUN: %dxc -E main -opt-enable structurize-returns -T ps_6_0 %s | FileCheck %s

// Make sure the return value is 9.
// CHECK:define void @main() {
// CHECK:  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 9.000000e+00)
// CHECK-NEXT:ret void

// This is for case
//        if ( b == 0xFF )
//        return 0 ;
//        else if ( b  != 0 )
//        return 1 ;
// where the return 1 in else if need branch to new endif create for return 0, instead of old endif.

static const
uint data [8] = {2,1,2,1,2,1,2,1};
float main(float a:A) : SV_Target {
        uint b  = 0 ;
        for ( uint i = 0 ; i < 8 ; i ++ )
        {
            b  |= data[i];
        }

switch (b) {
  case 77:
    return 3;
  case 3:
    return 9;
  default:
    return 12;
  case 39:
  case 0:
    break;
}

        if ( b == 77 )
        return 0 ;
        else if ( b  != 0 )
        return 1 ;

        uint x = 2;
        for ( uint i = 0 ; i < 8 ; i ++ )
        {
             x *= data[i];
        }
        if ( x == 32)
        {
            return 0 ;
        }
        return a;
}