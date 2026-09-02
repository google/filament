// RUN: %dxc -Tlib_6_3 -HV 2021 -verify %s
// RUN: %dxc -Tcs_6_0 -HV 2021 -verify %s

// Ensure that atomic operations fail when used with bitfields
// Use structured and typed buffers as the resources that can use structs
// and also both binary op and exchange atomic ops as either difference
// can cause the compiler to take different paths

struct TexCoords {
  uint s : 8;
  uint t : 8;
  uint r : 8;
  uint q : 8;
};

RWStructuredBuffer<TexCoords> str;
groupshared TexCoords gs;

[shader("compute")]
[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID) {

  InterlockedOr(str[tid.y].q, 2);                           /* expected-error {{no matching function for call to 'InterlockedOr'}} expected-note {{candidate function not viable: no known conversion from 'uint' to 'unsigned int &' for 1st argument}} expected-note {{candidate function not viable: no known conversion from 'uint' to 'unsigned long long &' for 1st argument}} */
  InterlockedCompareStore(str[tid.y].q, 3, 1);              /* expected-error {{no matching function for call to 'InterlockedCompareStore'}} expected-note {{candidate function not viable: no known conversion from 'uint' to 'unsigned int &' for 1st argument}} expected-note {{candidate function not viable: no known conversion from 'uint' to 'unsigned long long &' for 1st argument}} */

  InterlockedOr(gs.q, 2);                                   /* expected-error {{no matching function for call to 'InterlockedOr'}} expected-note {{candidate function not viable: 1st argument ('__attribute__((address_space(3))) uint') is in address space 3, but parameter must be in address space 0}} expected-note {{candidate function not viable: no known conversion from '__attribute__((address_space(3))) uint' to 'unsigned long long &' for 1st argument}} */
  InterlockedCompareStore(gs.q, 3, 1);                      /* expected-error {{no matching function for call to 'InterlockedCompareStore'}} expected-note {{candidate function not viable: 1st argument ('__attribute__((address_space(3))) uint') is in address space 3, but parameter must be in address space 0}} expected-note {{candidate function not viable: no known conversion from '__attribute__((address_space(3))) uint' to 'unsigned long long &' for 1st argument}} */
}
