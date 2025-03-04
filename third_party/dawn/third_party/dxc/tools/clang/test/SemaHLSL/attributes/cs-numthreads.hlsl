// RUN: %dxc -E main -T cs_6_3 -verify %s
// RUN: %dxc -E main -T cs_6_3 -verify -DANNOTATE %s
// RUN: %dxc -T lib_6_3 -verify -DANNOTATE %s

#ifdef ANNOTATE
[shader("compute")]
#endif
// expected-error@+1{{compute entry point must have a valid numthreads attribute}}
void main() {}
