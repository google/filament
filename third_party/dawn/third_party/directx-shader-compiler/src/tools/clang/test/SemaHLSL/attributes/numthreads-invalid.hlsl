// RUN: %dxc -E active_entry_point -T cs_6_3 -verify %s

// expected-warning@+1 {{Group size of 0 (0 * 0 * 0) is outside of valid range [1..1024] - attribute will be ignored}}
[numthreads(0, 0, 0)]
// expected-error@+1 {{compute entry point must have a valid numthreads attribute}}
void active_entry_point() {}

// expected-warning@+1 {{Group size of 4096 (64 * 64 * 1) is outside of valid range [1..1024] - attribute will be ignored}}
[numthreads(64, 64, 1)]
void inactive_entry_point() {}

// expected-warning@+1 {{Group size of 6000000 (100 * 200 * 300) is outside of valid range [1..1024] - attribute will be ignored}}
[numthreads(100, 200, 300)]
void fun1() {}
