A Performance Library for Deep Learning
================

The Intel(R) Math Kernel Library for Deep Neural Networks (Intel(R) MKL-DNN) is an
open source performance library for Deep Learning (DL) applications intended
for acceleration of DL frameworks on Intel(R) architecture. Intel MKL-DNN
includes highly vectorized and threaded building blocks for implementation of
convolutional neural networks (CNNs) and reccurent neural networks (RNNs) with
C and C++ interfaces. This project is created to help the DL community innovate
on the Intel(R) processor family.

The library provides optimized implementations for the most common
computational functions (also called primitives) used in deep neural
networks covering a wide range of applications, including image recognition,
object detection, semantic segmentation, neural machine translation,
and speech recognition.
The table below summarizes the list of supported functions and their variants.

| Primitive class   | Primitive                | fp32 training | fp32 inference | int8 inference |
| :---------------- | :----------------------- | :-----------: | :------------: | :------------: |
| Convolution       | 1D direct convolution    | x             | x              |                |
|                   | 2D direct convolution    | x             | x              | x              |
|                   | 2D direct deconvolution  | x             | x              | x              |
|                   | 2D winograd convolution  | x             | x              | x              |
|                   | 3D direct convolution    | x             | x              |                |
|                   | 3D direct deconvolution  | x             | x              |                |
| Inner Product     | 2D inner product         | x             | x              | x              |
|                   | 3D inner product         | x             | x              |                |
| RNN               | Vanilla RNN              | x             | x              |                |
|                   | LSTM                     | x             | x              | x              |
|                   | GRU                      | x             | x              |                |
| Pooling           | 2D maximum pooling       | x             | x              | x              |
|                   | 2D average pooling       | x             | x              | x              |
|                   | 3D maximum pooling       | x             | x              |                |
|                   | 3D average pooling       | x             | x              |                |
| Normalization     | 2D LRN (within channel)  | x             | x              |                |
|                   | 2D LRN (across channels) | x             | x              |                |
|                   | 2D batch normalization   | x             | x              |                |
|                   | 3D batch normalization   | x             | x              |                |
| Activation and    | ReLU                     | x             | x              | x              |
| elementwise       | Tanh                     | x             | x              |                |
| functions         | ELU                      | x             | x              |                |
|                   | Square                   | x             | x              |                |
|                   | Sqrt                     | x             | x              |                |
|                   | Abs                      | x             | x              |                |
|                   | Linear                   | x             | x              |                |
|                   | Bounded ReLU             | x             | x              |                |
|                   | Soft ReLU                | x             | x              |                |
|                   | Logistic                 | x             | x              |                |
|                   | Softmax                  | x             | x              |                |
| Data manipulation | Reorder/quantization     | x             | x              | x              |
|                   | Sum                      | x             | x              | x              |
|                   | Concat                   | x             | x              | x              |
|                   | Shuffle                  | x             | x              | x              |

## Programming Model

Intel MKL-DNN models memory as a primitive similar to an operation
primitive.  This allows reconstruction of the graph of computations
at run time.

### Basic Terminology

Intel MKL-DNN operates on the following main objects:

* **Primitive** - any operation, such as convolution, data format reordering, and even
  memory. Primitives can have other primitives as inputs, but can have only
  memory primitives as outputs.

* **Engine** - an execution device. Currently the only supported engine is CPU.
  Every primitive is mapped to a specific engine.

* **Stream** - an execution context. You submit primitives to a stream and
  wait for their completion. Primitives submitted to the same stream can have
  different engines. The stream also tracks dependencies between the primitives.

A typical workflow is to create a set of primitives to run,
push them to a stream all at once or one at a time, and wait for completion.

### Creating Primitives

In Intel MKL-DNN, creating primitives involves three levels of
abstraction:

* **Operation/memory descriptor** - a high-level description with logical
  parameters of an operation or memory. It is a lightweight structure that does
  not allocate any physical memory or computation resources.

* **Primitive descriptor** - a complete description of a primitive that contains
  an operation descriptor, descriptors of primitive inputs and outputs, and the
  target engine. This permits future API extensions to enable querying the descriptor
  for estimated performance, memory consumptions, and so on. A primitive
  descriptor is also a lightweight structure.

* **Primitive** - a specific instance of a primitive created using the
  corresponding primitive descriptor. A primitive structure contains pointers to
  input primitives and output memory. Creating a primitive is a potentially
  expensive operation because when a primitive is created, Intel MKL-DNN
  allocates the necessary resources to execute the primitive.

To create a memory primitive:

1. Create a memory descriptor. The memory descriptor contains the dimensions, precision, and
   format of the data layout in memory. The data layout can be either user-specified
   or set to `any`. The `any` format allows the operation primitives
   (convolution and inner product) to choose the best memory format for optimal
   performance.
2. Create a memory primitive descriptor. The memory primitive descriptor contains the memory
   descriptor and the
   target engine.
3. Create a memory primitive. The memory primitive requires allocating a memory buffer and
   attaching the data handle to the memory primitive descriptor.
   **Note:** in the C++ API for creating an output memory primitive, you
   do not need to allocate buffer unless the output is needed in a
   user-defined format.

To create an operation primitive:

1. Create a logical description of the operation. For example, the description
   of a convolution operation contains parameters such as sizes, strides, and
   propagation type. It also contains the input and output memory descriptors.
2. Create a primitive descriptor by attaching the target engine to the logical
   description.
3. Create an instance of the primitive and specify the input and output
   primitives.

## Examples

A walk-through example for implementing an AlexNet topology using the c++ API:

* [SimpleNet Example](@ref ex_simplenet)

An introductory example to low-precision 8-bit computations:

* [Int8 SimpleNet Example](@ref ex_int8_simplenet)

The following examples are available in the /examples directory and provide more details about the API.
* Creation of forward primitives
    - C: simple_net.c
    - C++: simple_net.cpp

* Creation of full training net (forward and backward primitives)
    - C: simple_training.c
    - C++: simple_training_net.cpp

* Creation of forward propagation of GNMT topology
    - C++: simple_rnn.cpp

* Training RNN with sequences of variable length
    - C++: simple_rnn_training.cpp

### Performance Considerations

*  Convolution and inner product primitives choose the memory format when you create them with the unspecified memory
   format `any` for input or output.
   The memory format chosen is based on different circumstances such as hardware and
   convolutional parameters.
*  Convolution could be executed using the [Winograd algorithm](@ref winograd_convolution) for a significant performance boost.
*  Operation primitives (such as ReLU, LRN, or pooling) following convolution or
   inner product, should have input in the same memory format as the
   convolution or inner-product. Reordering can be an expensive
   operation, so you should avoid it unless it is necessary for performance in
   convolution, inner product, or output specifications.
*  Pooling, concat and sum can be created with the output memory format `any`.
*  An operation primitive (typically operations such as pooling, LRN, or softmax)
   might need workspace memory for storing results of intermediate operations
   that help with backward propagation.


The following link provides a guide to MKLDNN verbose mode for profiling execution:

* [Performance profiling](@ref perf_profile)

### Operational Details

*  You might need to create a reorder primitive to convert the data from a user
   format to the format preferred by convolution or inner product.
*  All operations should be queried for workspace memory requirements.
   If workspace is needed, it should only be created during the forward
   propagation and then shared with the corresponding primitive on
   backward propagation.
*  A primitive descriptor from forward propagation must be provided while
   creating corresponding primitive descriptor for backward propagation. This
   tells the backward operation what exact implementation is chosen for
   the primitive on forward propagation. This in turn helps the backward operation
   to decode the workspace memory correctly.
*  You should always check the correspondence between current data format and
   the format that is required by a primitive. For instance, forward convolution
   and backward convolution with respect to source might choose different memory
   formats for weights (if created with `any`). In this case, you should create
   a reorder primitive for weights.
   Similarly, a reorder primitive might be required for a source data between
   forward convolution and backward convolution with respect to weights.

   **Note:** Please refer to extended examples to illustrate these details.

### Auxiliary Types

* **Primitive_at** - a structure that contains a primitive and an index. This
  structure specifies which output of the primitive to use as an input for
  another primitive. For a memory primitive the index is always `0`
  because it does not have a output.


## Architecture and design of Intel MKL-DNN

For better understanding the architecture and design of Intel MKL-DNN
as well as the concepts used in the library please read the following
topics:

[Understanding Memory Formats](@ref understanding_memory_formats)


--------

[Legal information](@ref legal_information)
