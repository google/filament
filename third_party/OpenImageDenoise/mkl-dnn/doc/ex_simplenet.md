SimpleNet Example {#ex_simplenet}
================================

This C++ API example demonstrates how to build an AlexNet neural
network topology for forward-pass inference. Some key take-aways
include:

* How tensors implemented and submitted to primitives.
* How primitives are created.
* How primitives are sequentially submitted to the network, where the output from
  primitives is passed as input to the next primitive. The later specifies
  dependency between primitive input <-> output data.
* Specific 'inference-only' configurations.
* Limit the number of reorders performed which are decremental to performance.

The simple_net.cpp example implements the AlexNet layers
as numbered primitives (e.g. conv1, pool1, conv2).

## Highlights for implementing the simple_net.cpp Example:

1. Initialize a CPU engine. The last parameter in the engine() call represents the index of the
   engine.
~~~cpp
using namespace mkldnn;
auto cpu_engine = engine(engine::cpu, 0);
~~~

2. Create a primitives vector that represents the net.
~~~cpp
std::vector<primitive> net;
~~~

3. Additionally, create a separate vector holding the weights. This will allow
executing transformations only once and outside the topology stream.
~~~cpp
std::vector<primitive> net_weights;
~~~

4. Allocate a vector for input data and create the tensor to configure the dimensions.
~~~cpp
memory::dims conv1_src_tz = { batch, 3, 227, 227 };
std::vector<float> user_src(batch * 3 * 227 * 227);
/* similarly, specify tensor structure for output, weights and bias */
~~~

5. Create a memory primitive for data in user format as `nchw`
   (minibatch-channels-height-width). Create a memory descriptor
   for the convolution input, selecting `any` for the data format.
   The `any` format allows the convolution primitive to choose the data format
   that is most suitable for its input parameters (convolution kernel
   sizes, strides, padding, and so on). If the resulting format is different
   from `nchw`, the user data must be transformed to the format required for
   the convolution (as explained below).
~~~cpp
auto user_src_memory = memory({ { { conv1_src_tz }, memory::data_type::f32,
    memory::format::nchw }, cpu_engine}, user_src.data());
auto conv1_src_md = memory::desc({conv1_src_tz},
    memory::data_type::f32, memory::format::any);
/* similarly create conv_weights_md and conv_dst_md in format::any */
~~~

6. Create a convolution descriptor by specifying the algorithm([convolution algorithms](@ref winograd_convolution), propagation
   kind, shapes of input, weights, bias, output, convolution strides,
   padding, and kind of padding. Propagation kind is set to *forward_inference*
   -optimized for inference execution and omits computations that are only necessary
   for backward propagation. */
~~~cpp
auto conv1_desc = convolution_forward::desc(
    prop_kind::forward_inference, algorithm::convolution_direct,
    conv1_src_md, conv1_weights_md, conv1_bias_md, conv1_dst_md,
    conv1_strides, conv1_padding, padding_kind::zero);
~~~

7. Create a descriptor of the convolution primitive. Once created, this
   descriptor has specific formats instead of the `any` format specified
   in the convolution descriptor.
~~~cpp
auto conv1_prim_desc = convolution_forward::primitive_desc(conv1_desc, cpu_engine);
~~~

8. Create a convolution memory primitive from the user memory and check whether the user
   data format differs from the format that the convolution requires. In
   case it is different, create a reorder primitive that transforms the user data
   to the convolution format and add it to the net. Repeat this process for weights as well.
~~~cpp
auto conv1_src_memory = user_src_memory;

/* Check whether a reorder is necessary  */
if (memory::primitive_desc(conv1_prim_desc.src_primitive_desc())
        != user_src_memory.get_primitive_desc()) {
    /* Yes, a reorder is necessary */

    /* The convolution primitive descriptor contains the descriptor of a memory
     * primitive it requires as input. Because a pointer to the allocated
     * memory is not specified, Intel MKL-DNN allocates the memory. */
    conv1_src_memory = memory(conv1_prim_desc.src_primitive_desc());

    /* create a reorder between user and convolution data and put the reorder
     * into the net. The conv1_src_memory will be the input for the convolution */
    net.push_back(reorder(user_src_memory, conv1_src_memory));
}
~~~

9. Create a memory primitive for output.
~~~cpp
auto conv1_dst_memory = memory(conv1_prim_desc.dst_primitive_desc());
~~~

10. Create a convolution primitive and add it to the net.
~~~cpp
/* Note that the conv_reorder_src primitive
 * is an input dependency for the convolution primitive, which means that the
 * convolution primitive will not be executed before the data is ready. */
net.push_bash(convolution_forward(conv1_prim_desc, conv1_src_memory, conv1_weights_memory,
                              user_bias_memory, conv1_dst_memory));
~~~

11. Create relu primitive. For better performance keep ReLU
   (as well as for other operation primitives until another convolution or
    inner product is encountered) input data format in the same format as was chosen by
   convolution. Furthermore, ReLU is done in-place by using conv1 memory.
~~~cpp
auto relu1_desc = eltwise_forward::desc(prop_kind::forward_inference,
    algorithm::eltwise_relu, conv1_dst_memory.get_primitive_desc().desc(), negative1_slope);
auto relu1_prim_desc = eltwise_forward::primitive_desc(relu1_desc, cpu_engine);
net.push_back(eltwise_forward(relu1_prim_desc, conv1_dst_memory, conv1_dst_memory));
~~~

12. For training execution, pooling requires a private workspace memory to perform
the backward pass. However, pooling should not use 'workspace' for inference
as this is decremental to performance.
~~~cpp
/* create pooling indices memory from pooling primitive descriptor */
// auto pool1_indices_memory = memory(pool1_pd.workspace_primitive_desc());
auto pool1_dst_memory = memory(pool1_pd.dst_primitive_desc());

/* create pooling primitive an add it to net */
net.push_back(pooling_forward(pool1_pd, lrn1_dst_memory, pool1_dst_memory
    /* pool1_indices_memory */));
~~~
  The example continues to create more layers according to
  the AlexNet topology.

14. Finally, create a stream to execute weights data transformation. This is only
required once. Create another stream that will exeute the 'net' primitives. For
this example, the net is executed multiple times and each execution es timed
individually.
~~~cpp
/* Weight transformation - executed once */
stream(stream::kind::eager).submit(net_weights).wait();

/* Execute the topology */
mkldnn::stream(mkldnn::stream::kind::eager).submit(net).wait();
~~~
---

[Legal information](@ref legal_information)
