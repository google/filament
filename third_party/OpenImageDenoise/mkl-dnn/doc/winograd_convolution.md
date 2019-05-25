Winograd Convolution {#winograd_convolution}
==========================================
## Why use a different convolution algorithm?
Executing convolution using the **Winograd algorithm** often gives a significant performance boost compared with using the **Direct algorithm**.
Details about the algorithm can be found in [<b>Fast Algorithms for Convolutional Neural Networks by A. Lavin and S. Gray</b>](https://arxiv.org/abs/1509.09308).

## Winograd in Intel(R) MKL-DNN
Intel(R) MKL-DNN supports the **Winograd algorithm** for convolutions with the following sizes:
* 2D convolution (i.e. spatial depth `d=1`)
* kernel sizes `kh=3,kw=3`.
* strides `sh=sw=1`.

* **Inference** - Based on convolution sizes, MKLDNN chooses between two different tile sizes F(2x2, 3x3) or F(4x4, 3x3)(refer to [Winograd paper](https://arxiv.org/abs/1509.09308) for more informartion on tile sizes).
* **Training** - Uses F(4x4, 3x3) winograd.

Create a Winograd convolution by simply creating a convolution descriptor (step 6 in [SimpleNet Example](@ref ex_simplenet)) with right algorithm.
The rest of the steps for creating convolution are exactly the same as shown in the example.
~~~cpp
auto conv1_desc = convolution_forward::desc(
    prop_kind::forward_inference, algorithm::convolution_winograd,
    conv1_src_md, conv1_weights_md, conv1_bias_md, conv1_dst_md,
    conv1_strides, conv1_padding, padding_kind::zero);
~~~

## Auto dispatching of convolution algorithm
Instead of choosing a convolution algorithm for each and every convolution in a topology, a user could simply ask MKLDNN to make the choice.

Creating a convolution by using `convolution_auto` allows MKLDNN to dispatch the *best* algorithm. 
~~~cpp
auto conv1_desc = convolution_forward::desc(
    prop_kind::forward_inference, algorithm::convolution_auto,
    conv1_src_md, conv1_weights_md, conv1_bias_md, conv1_dst_md,
    conv1_strides, conv1_padding, padding_kind::zero);
~~~

MKLDNN would choose the algorithm which will potentially give *best performance* based on
* convolution dimensions
* number of logical processors available. (For auto-dispatching to work as intended,
        use the same thread affinity settings when creating the convolution as when executing the convolution.)
*The relationship between convolution sizes and the best performing algorithm is empirically based on performance observations*

### Example using benchdnn
The following examples use [<b>benchdnn</b>](https://github.com/intel/mkl-dnn/tree/master/tests/benchdnn) to illustrate the performance benefits of using `convolution_auto`.

On a 2 Socket Intel Xeon 8180 processor with 28 cores/socket and HT off:
~~~sh
OMP_NUM_THREADS=56 KMP_AFFINITY=granularity=fine,compact numactl -l tests/benchdnn/benchdnn --mode=p --conv -v5 --alg=auto --dir=BWD_WB mb112ic64ih300oc64oh300kh3ph1n"ssd_300_voc0712:conv1_2"

mkldnn implementation: jit_wino_4x3:avx512_core
...
mkldnn_verbose,exec,convolution,jit_wino_4x3:avx512_core,backward_weights,fsrc:nChw16c fwei:gOIhw16i16o fbia:x fdst:nChw16c,alg:convolution_winograd,mb112_g1ic64oc64_ih300oh300kh3sh1dh0ph1_iw300ow300kw3sw1dw0pw1,61.32
...
perf,ssd_300_voc0712:conv1_2,--dir=BWD_WB --alg=auto mb112ic64ih300oc64oh300kh3ph1nssd_300_voc0712:conv1_2,739.879,0,61.332,12063.5,62.503,11837.5
~~~

In the above test-case `convolution_auto` choses winograd convolution (using a heuristic based on the convolution sizes and number of threads), as winograd convolution is faster than direct in this case.
~~~sh
OMP_NUM_THREADS=56 KMP_AFFINITY=granularity=fine,compact numactl -l tests/benchdnn/benchdnn --mode=p --conv -v5 --alg=direct --dir=BWD_WB mb112ic64ih300oc64oh300kh3ph1n"ssd_300_voc0712:conv1_2"

mkldnn implementation: jit:avx512_common
...
mkldnn_verbose,exec,convolution,jit:avx512_common,backward_weights,fsrc:nchw fwei:gOhwi16o fbia:x fdst:nChw16c,alg:convolution_direct,mb112_g1ic64oc64_ih300oh300kh3sh1dh0ph1_iw300ow300kw3sw1dw0pw1,176.10
...
perf,ssd_300_voc0712:conv1_2,--dir=BWD_WB mb112ic64ih300oc64oh300kh3ph1nssd_300_voc0712:conv1_2,739.879,0,175.422,4217.7,180.315,4103.26
~~~

<br/>

In the following example, `convolution_auto` chooses direct convolution because the winograd implementation is slower than direct in this case.
~~~sh
OMP_NUM_THREADS=56 KMP_AFFINITY=granularity=fine,compact tests/benchdnn/benchdnn --mode=p --conv -v5 --alg=auto --dir=BWD_WB mb112ic64ih28oc64oh28kh3ph1n"googlenet_v2:inception_3a/3x3"

mkldnn implementation: jit:avx512_common
...
mkldnn_verbose,exec,convolution,jit:avx512_common,backward_weights,fsrc:nChw16c fwei:gOIhw16i16o fbia:x fdst:nChw16c,alg:convolution_direct,mb112_g1ic64oc64_ih28oh28kh3sh1dh0ph1_iw28ow28kw3sw1dw0pw1,1.13
perf,googlenet_v2:inception_3a/3x3,--dir=BWD_WB --alg=auto mb112ic64ih28oc64oh28kh3ph1ngooglenet_v2:inception_3a/3x3,6.1693,0,1.04272,5916.52,1.13284,5445.88
~~~
~~~sh
OMP_NUM_THREADS=56 KMP_AFFINITY=granularity=fine,compact tests/benchdnn/benchdnn --mode=p --conv -v5 --alg=wino --dir=BWD_WB mb112ic64ih28oc64oh28kh3ph1n"googlenet_v2:inception_3a/3x3"

mkldnn implementation: jit_wino_4x3:avx512_core
...
mkldnn_verbose,exec,convolution,jit_wino_4x3:avx512_core,backward_weights,fsrc:nChw16c fwei:gOIhw16i16o fbia:x fdst:nChw16c,alg:convolution_winograd,mb112_g1ic64oc64_ih28oh28kh3sh1dh0ph1_iw28ow28kw3sw1dw0pw1,2.15
...
perf,googlenet_v2:inception_3a/3x3,--dir=BWD_WB --alg=wino mb112ic64ih28oc64oh28kh3ph1ngooglenet_v2:inception_3a/3x3,6.1693,0,2.14404,2877.41,2.20445,2798.56
~~~

## Other considerations when using Winograd
The following side-effects should be weighed against the performance boost achieved when using Winograd:
* **Memory** - Transforms are intermmediate results in winograd, which often require significant memory. Currently this memory is allocated internally by MKLDNN as scratchpad memory. As more convolutions using winograd
are added to the topology, this memory could grow significantly. This growth is mitigated when several convolutions using Winograd are created by the same instance and executed sequentially, because then
this scratchpad can be shared between convolutions.
* **Accuracy** - In some cases Winograd can be signficantly less accurate than direct as demontrated in [Winograd paper](https://arxiv.org/abs/1509.09308).
