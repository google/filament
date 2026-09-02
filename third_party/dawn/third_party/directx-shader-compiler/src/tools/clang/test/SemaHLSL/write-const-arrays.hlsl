// RUN: %dxc -Tlib_6_3 -verify %s
// RUN: %dxc -Tcs_6_0 -verify %s

// Array subscripts lost their qualifiers including const due to taking a different
// path for HLSL on account of having no array to ptr decay
// This test verifies that subscripts of constant and cbuf arrays can't be assigned
// or otherwise altered by a few mechanisms

/* Expected note with no locations (implicit built-in):
  expected-note@? {{function 'operator[]<const unsigned int &>' which returns const-qualified type 'const unsigned int &' declared here}}
*/

StructuredBuffer<uint> g_robuf;
Texture1D<uint> g_tex;
uint g_cbuf[4];

const groupshared uint gs_val[4];

[shader("compute")]
[NumThreads(1, 1, 1)]
void main() {
  const uint local[4];

  // Assigning using assignment operator
  local[0] = 0;                                             /* expected-error {{read-only variable is not assignable}} */
  gs_val[0] = 0;                                            /* expected-error {{read-only variable is not assignable}} */
  g_cbuf[0] = 0;                                            /* expected-error {{read-only variable is not assignable}} */
  g_robuf[0] = 0;                                           /* expected-error {{cannot assign to return value because function 'operator[]<const unsigned int &>' returns a const value}} */

  // Assigning using out param of builtin function
  double d = 1.0;
  asuint(d, local[0], local[1]);                          /* expected-error {{no matching function for call to 'asuint'}} expected-note {{candidate function not viable: 2nd argument ('const uint') would lose const qualifier}} */
  asuint(d, gs_val[0], gs_val[1]);                        /* expected-error {{no matching function for call to 'asuint'}} expected-note {{candidate function not viable: 2nd argument ('const uint') would lose const qualifier}} */
  asuint(d, g_cbuf[0], g_cbuf[1]);                        /* expected-error {{no matching function for call to 'asuint'}} expected-note {{candidate function not viable: 2nd argument ('const uint') would lose const qualifier}} */
  asuint(d, g_robuf[0], g_robuf[1]);                      /* expected-error {{no matching function for call to 'asuint'}} expected-note {{candidate function not viable: 2nd argument ('const unsigned int') would lose const qualifier}} */


  // Assigning using out param of method
  g_tex.GetDimensions(local[0]);                            /* expected-error {{no matching member function for call to 'GetDimensions'}} expected-note {{candidate function template not viable: requires 3 arguments, but 1 was provided}} */
  g_tex.GetDimensions(gs_val[0]);                           /* expected-error {{no matching member function for call to 'GetDimensions'}} expected-note {{candidate function template not viable: requires 3 arguments, but 1 was provided}} */
  g_tex.GetDimensions(g_cbuf[0]);                           /* expected-error {{no matching member function for call to 'GetDimensions'}} expected-note {{candidate function template not viable: requires 3 arguments, but 1 was provided}} */
  g_tex.GetDimensions(g_robuf[0]);                          /* expected-error {{no matching member function for call to 'GetDimensions'}} expected-note {{candidate function template not viable: requires 3 arguments, but 1 was provided}} */


  // Assigning using dest param of atomics
  // Distinct because of special handling of atomics dest param
  InterlockedAdd(local[0], 1);                              /* expected-error {{no matching function for call to 'InterlockedAdd'}} expected-note {{candidate function not viable: 1st argument ('const uint') would lose const qualifier}} expected-note {{candidate function not viable: no known conversion from 'const uint' to 'unsigned long long &' for 1st argument}} */
  InterlockedAdd(gs_val[0], 1);                             /* expected-error {{no matching function for call to 'InterlockedAdd'}} expected-note {{candidate function not viable: 1st argument ('const uint') would lose const qualifier}} expected-note {{candidate function not viable: no known conversion from 'const uint' to 'unsigned long long &' for 1st argument}} */
  InterlockedAdd(g_cbuf[0], 1);                             /* expected-error {{no matching function for call to 'InterlockedAdd'}} expected-note {{candidate function not viable: 1st argument ('const uint') would lose const qualifier}} expected-note {{candidate function not viable: no known conversion from 'const uint' to 'unsigned long long &' for 1st argument}} */
  InterlockedAdd(g_robuf[0], 1);                            /* expected-error {{no matching function for call to 'InterlockedAdd'}} expected-note {{candidate function not viable: 1st argument ('const unsigned int') would lose const qualifier}} expected-note {{candidate function not viable: no known conversion from 'const unsigned int' to 'unsigned long long &' for 1st argument}} */

}
