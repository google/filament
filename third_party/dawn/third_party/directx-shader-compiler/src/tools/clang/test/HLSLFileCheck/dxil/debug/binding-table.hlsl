// RUN: %dxc -WX -Od -E main -T ps_6_0 %s | FileCheck %s -check-prefixes=ERR
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define NORMAL_TABLE %s | FileCheck %s
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define REORDER %s | FileCheck %s
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define INLINE -DINLINE="resourcename,binding,space;cb,b10,0x1e;resource,b42,999;samp0,s1,0x02;resource,t1,2;uav_0,u0,0;" %s | FileCheck %s
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define EXTRA_CELL %s | FileCheck %s -check-prefixes=EXTRA_CELL
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define MISSING_CELL %s | FileCheck %s -check-prefixes=MISSING_CELL
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define MISSING_COLUMN %s | FileCheck %s -check-prefixes=MISSING_COLUMN
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define EMPTY %s | FileCheck %s -check-prefixes=EMPTY
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define INVALID_BINDING %s | FileCheck %s -check-prefixes=INVALID_BINDING
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define INVALID_BINDING_2 %s | FileCheck %s -check-prefixes=INVALID_BINDING_2
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define INVALID_BINDING_3 %s | FileCheck %s -check-prefixes=INVALID_BINDING_3
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define OOB %s | FileCheck %s -check-prefixes=OOB
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define OOB_2 %s | FileCheck %s -check-prefixes=OOB_2
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define OOB_3 %s | FileCheck %s -check-prefixes=OOB_3
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define EMPTY_CELL %s | FileCheck %s -check-prefixes=EMPTY_CELL
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define NOT_INTEGER %s | FileCheck %s -check-prefixes=NOT_INTEGER
// RUN: %dxc -WX -Od -E main -T ps_6_0 -binding-table-define NOT_FOUND %s | FileCheck %s -check-prefixes=NOT_FOUND

// First check that the default compile fails
// ERR: Root Signature in DXIL container is not compatible with shader

// CHECK: @main
// NOT_FOUND: Binding table define'NOT_FOUND' not found.

#define NORMAL_TABLE \
         "resourcename, binding,  space;" \
         "cb,           b10,      0x1e;"  \
         "resource,     b42,      999; "  \
         "samp0,        s1,       0x02;"  \
         "resource,     t1,       2;   "  \
         "uav_0,        u0,       0;   "

#define REORDER \
         "  spacE,  binding,  ResourceName;  " \
         "  0x1e,   b10,      cb;            " \
         "  999,    b42,      resource;      " \
         "  0x02,   s1,       samp0;         " \
         "  2,      t1,       resource;      " \
         "  0,      u0,       uav_0;         "

// EXTRA_CELL: Unexpected cell at the end of row. There should only be 3
#define EXTRA_CELL \
         "  spacE,  binding,  ResourceName;  " \
         "  0x1e,   b10,      cb, extra;     " \
         "  999,    b42,      resource;      " \
         "  0x02,   s1,       samp0;         " \
         "  2,      t1,       resource;      " \
         "  0,      u0,       uav_0;         "

// MISSING_CELL: Row ended after just 2 columns. Expected 3.
#define MISSING_CELL \
         "spacE,  binding,  ResourceName;  " \
         "0x1e,   b10;                     " \
         "999,    b42,      resource;      " \
         "0x02,   s1,       samp0;         " \
         "2,      t1,       resource;      " \
         "0,      u0,       uav_0;         "

// MISSING_COLUMN: Input format is csv with headings: ResourceName, Binding, Space.
#define MISSING_COLUMN \
         "resourcename, space;" \
         "cb,           0x1e; " \
         "resource,     999;  " \
         "samp0,        0x02; " \
         "resource,     2;    " \
         "uav_0,        0;    "

// EMPTY: Unexpected EOF when parsing cell.
#define EMPTY ""

// INVALID_BINDING: Invalid resource class
#define INVALID_BINDING \
         "resourcename, binding,  space;" \
         "cb,           10,       0x1e;"

// INVALID_BINDING_2: Invalid resource class
#define INVALID_BINDING_2 \
         "resourcename, binding,  space;" \
         "cb,           e10,      0x1e;"

// INVALID_BINDING_3: 'b' is not a valid resource binding.
#define INVALID_BINDING_3 \
         "resourcename, binding,  space;" \
         "cb,           b,         0x1e;"

// OOB: '99999999999999999999999999999999999999999' is out of range of an 32-bit unsigned integer.
#define OOB \
         "resourcename, binding,  space;" \
         "cb,           b99999999999999999999999999999999999999999,      0x1e;"

// OOB_2: '99999999999999999999999999999999999' is out of range of an 32-bit unsigned integer.
#define OOB_2 \
         "resourcename, binding,  space;" \
         "cb,           b10,      99999999999999999999999999999999999"

// OOB_3: '0xffffffffffffffffffffffffffffffffffffffffffffffffffff' is out of range of an 32-bit unsigned integer.
#define OOB_3 \
         "resourcename, binding,  space;" \
         "cb,           b10,      0xffffffffffffffffffffffffffffffffffffffffffffffffffff"

// NOT_INTEGER: 'abcd' is not a valid 32-bit unsigned integer.
#define NOT_INTEGER \
         "resourcename, binding,  space;" \
         "cb,           b10,      abcd"

// EMPTY_CELL: Resource binding cannot be empty.
#define EMPTY_CELL \
         "resourcename, binding,  space;" \
         "cb,           ,         0x1e;"

cbuffer cb {
    float a;
};
cbuffer resource {
    float b;
};

SamplerState samp0;
Texture2D resource;
RWTexture1D<float> uav_0;

[RootSignature("CBV(b10,space=30), CBV(b42,space=999), DescriptorTable(Sampler(s1,space=2)), DescriptorTable(SRV(t1,space=2)), DescriptorTable(UAV(u0,space=0))")]
float main(float2 uv : UV, uint i : I) :SV_Target {
    return a + b + resource.Sample(samp0, uv).r + uav_0[i];
}