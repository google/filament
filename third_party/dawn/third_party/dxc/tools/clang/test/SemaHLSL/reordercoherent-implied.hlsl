// RUN: %dxc -E main -T lib_6_9 -verify %s
// REQUIRES: dxil-1-9

using Ty = RWTexture1D<float4>;

using GTy = globallycoherent Ty;
using RTy = reordercoherent Ty;

// expected-warning@+1{{attribute 'globallycoherent' is already applied}}
using GGTy = globallycoherent GTy;
// expected-warning@+1{{attribute 'reordercoherent' is already applied}}
using RRTy = reordercoherent RTy;

// expected-warning@+1{{attribute 'globallycoherent' implies 'reordercoherent'}}
using GRTy = globallycoherent RTy;
// expected-warning@+1{{attribute 'globallycoherent' implies 'reordercoherent'}}
using RGTy = reordercoherent GTy;

// expected-warning@+1{{attribute 'globallycoherent' is already applied}}
using GGRTy = globallycoherent GRTy;
// expected-warning@+1{{attribute 'reordercoherent' is already applied}}
using RRGTy = reordercoherent RGTy;

// expected-warning@+1{{attribute 'globallycoherent' implies 'reordercoherent'}}
using GRTy2 = globallycoherent reordercoherent Ty;
// expected-warning@+1{{attribute 'globallycoherent' implies 'reordercoherent'}}
using RGTy2 = reordercoherent globallycoherent Ty;

// expected-warning@+2{{attribute 'globallycoherent' implies 'reordercoherent'}}
// expected-warning@+1{{attribute 'globallycoherent' is already applied}}
using GGRTy2 = globallycoherent globallycoherent reordercoherent Ty;
// expected-warning@+2{{attribute 'globallycoherent' implies 'reordercoherent'}}
// expected-warning@+1{{attribute 'globallycoherent' is already applied}}
using GRGTy2 = globallycoherent reordercoherent globallycoherent Ty;

// expected-warning@+2{{attribute 'globallycoherent' implies 'reordercoherent'}}
// expected-warning@+1{{attribute 'reordercoherent' is already applied}}
using RGRTy2 = reordercoherent globallycoherent reordercoherent Ty;
// expected-warning@+2{{attribute 'globallycoherent' implies 'reordercoherent'}}
// expected-warning@+1{{attribute 'reordercoherent' is already applied}}
using RRGTy2 = reordercoherent reordercoherent globallycoherent Ty;
