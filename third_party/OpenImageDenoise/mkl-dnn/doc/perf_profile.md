Performance profiling {#perf_profile}
=====================================

It is often useful to collect information about how much of an application run
time is spent executing Intel(R) MKL-DNN primitives and which of those take
the most time. One of the popular methods to do this is to use profilers like
Linux\* perf or Intel(R) VTune(tm) Amplifier. Currently, Intel MKL-DNN has very
limited support for these tools since it does not annotate code generated at
run-time and thus the profiles cannot properly attribute it. However, Intel
MKL-DNN implements another feature called _verbose mode_ that allows tracing
execution of Intel MKL-DNN primitives and collection of basic statistics like
execution time and primitive parameters.

## Verbose mode

To enable Intel MKL-DNN verbose mode, set `MKLDNN_VERBOSE` environment variable
to `1` (to dump only execution time) or `2` (to dump both execution and
creation time). For example:

```
    $ export MKLDNN_VERBOSE=1
    $ ./benchdnn --conv ic16ih7oc16oh7kh5ph2n"wip"
```

This will produce the following output (the line break was added to fit into
the page width):

```
    mkldnn_verbose,info,Intel(R) MKL-DNN v0.18.0 (Git Hash 4cfed5bf82f1339d7c8c7f622fda02dc00ec8ad8), \
        Intel(R) Advanced Vector Extensions 2 (Intel(R) AVX2)
    mkldnn_verbose,exec,reorder,jit:uni,undef,in:f32_nchw out:f32_nChw8c,num:1,2x16x7x7,0.529053
    mkldnn_verbose,exec,reorder,jit:uni,undef,in:f32_oihw out:f32_OIhw8i8o,num:1,16x16x5x5,0.98999
    mkldnn_verbose,exec,reorder,jit:uni,undef,in:f32_nchw out:f32_nChw8c,num:1,2x16x7x7,0.453125
    mkldnn_verbose,exec,reorder,simple:any,undef,in:f32_x out:f32_x,num:1,16,0.388916
    mkldnn_verbose,exec,convolution,jit:avx2,forward_training,fsrc:nChw8c fwei:OIhw8i8o fbia:x \
        fdst:nChw8c,alg:convolution_direct,mb2_ic16oc16_ih7oh7kh5sh1dh0ph2_iw7ow7kw5sw1dw0pw2,0.0241699
    mkldnn_verbose,exec,reorder,jit:uni,undef,in:f32_nChw8c out:f32_nchw,num:1,2x16x7x7,0.469971
    0:PASSED __REPRO: ic16ih7oc16oh7kh5ph2nwip
    tests:1 passed:1 skipped:0 mistrusted:0 unimplemented:0 failed:0
```

The first line of verbose information contains the build version and git hash,
if available, as well as the supported instruction set architechture. Each
subsequent line of verbose information is formatted as a comma-separated list
containing:
- `mkldnn_verbose`
- `stage`, e.g. `create` or `exec`
- `primitive-kind`, e.g. `convolution`, `reorder`, `sum`, ...
- primitive implementation name
- propagation-kind, e.g. `forward_training`
- input/output data info, e.g. data type and data format
- auxiliary information, e.g. algorithm or number of input
- problem description
    - for convolution the problem description is dumped in benchdnn friendly format
    - for reorder, sum, and concat problem description is simply logical dims
    - for other primitives the problem description is similar to convolution one
- execution time in milliseconds

To get more information about verbose report format please refer to the
`verbose_templ()` function in the
[src/common/verbose.hpp](https://github.com/intel/mkl-dnn/blob/master/src/common/verbose.hpp)
file.

---
**NOTE**
The format is subject to change

---


---
**WARNING**
Verbose mode has non-negligible performance impact especially if the output
rate is high.

---

## Integration with performance profilers

When running under Intel VTune, Intel MKL-DNN notifies the Intel VTune runtime
about generated code so that it is able to correctly attribute PMU events.

Setting CMake option `MKLDNN_ENABLE_JIT_PROFILING=OFF` turns off integration
with Intel VTune completely. Adding `-DMKLDNN_ENABLE_JIT_PROFILING=0` to the
compiler flags has the same effect.

## Dumping JIT-kernels

To write JIT-kernels code to files, set `MKLDNN_JIT_DUMP` environment variable
to `1`. For example:

```
    $ export MKLDNN_JIT_DUMP=1
    $ ./simple-net-c
```

This will produce the following output files:

```
    mkldnn_dump_jit_uni_reorder_kernel_f32.0.bin
    mkldnn_dump_jit_avx2_conv_fwd_kernel_f32.1.bin
    mkldnn_dump_jit_uni_relu_kernel_f32.2.bin
    mkldnn_dump_jit_uni_lrn_fwd_kernel_f32.3.bin
    mkldnn_dump_jit_uni_lrn_fwd_kernel_f32.4.bin
    mkldnn_dump_jit_uni_lrn_fwd_kernel_f32.5.bin
    mkldnn_dump_jit_uni_reorder_kernel_f32.6.bin
    mkldnn_dump_jit_uni_pool_kernel_f32.7.bin
```

Use any disassembler to view the code. For example:

```
    $ xed -64 -ir mkldnn_dump_jit_avx2_conv_fwd_kernel_f32.1.bin
    XDIS 0: PUSH      BASE       53                       push rbx
    XDIS 1: PUSH      BASE       55                       push rbp
    XDIS 2: PUSH      BASE       4154                     push r12
    XDIS 4: PUSH      BASE       4155                     push r13
    XDIS 6: PUSH      BASE       4156                     push r14
    XDIS 8: PUSH      BASE       4157                     push r15
    XDIS a: DATAXFER  BASE       488B07                   mov rax, qword ptr [rdi]
    XDIS d: DATAXFER  BASE       488B7708                 mov rsi, qword ptr [rdi+0x8]
    XDIS 11: DATAXFER  BASE       488B5710                 mov rdx, qword ptr [rdi+0x10]
    XDIS 15: DATAXFER  BASE       488B5F18                 mov rbx, qword ptr [rdi+0x18]
    XDIS 19: DATAXFER  BASE       488B8F98000000           mov rcx, qword ptr [rdi+0x98]
    XDIS 20: DATAXFER  BASE       448BAF00010000           mov r13d, dword ptr [rdi+0x100]
    XDIS 27: DATAXFER  BASE       4C8BB7D0000000           mov r14, qword ptr [rdi+0xd0]
    XDIS 2e: BINARY    BASE       4983FE04                 cmp r14, 0x4
    XDIS 32: COND_BR   BASE       0F85EF030000             jnz 0x427
    XDIS 38: LOGICAL   BASE       4D31DB                   xor r11, r11
    XDIS 3b: LOGICAL   BASE       41F7C510000000           test r13d, 0x10
    XDIS 42: COND_BR   BASE       0F8558000000             jnz 0xa0
    XDIS 48: DATAXFER  AVX        C5FC1006                 vmovups ymm0, ymmword ptr [rsi]
    XDIS 4c: DATAXFER  AVX        C5FC104E20               vmovups ymm1, ymmword ptr [rsi+0x20]
    XDIS 51: DATAXFER  AVX        C5FC105640               vmovups ymm2, ymmword ptr [rsi+0x40]
    XDIS 56: DATAXFER  AVX        C5FC109E207A0100         vmovups ymm3, ymmword ptr [rsi+0x17a20]
    ...
```

Or use `objdump -D -b binary -mi386:x86-64`.

[Legal information](@ref legal_information)
