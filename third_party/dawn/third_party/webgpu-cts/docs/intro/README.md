# Introduction

These documents contains guidelines for contributors to the WebGPU CTS (Conformance Test Suite)
on how to write effective tests, and on the testing philosophy to adopt.

The WebGPU CTS is arguably more important than the WebGPU specification itself, because
it is what forces implementation to be interoperable by checking they conform to the specification.
However writing a CTS is hard and requires a lot of effort to reach good coverage.

More than a collection of tests like regular end2end and unit tests for software artifacts, a CTS
needs to be exhaustive. Contrast for example the WebGL2 CTS with the ANGLE end2end tests: they
cover the same functionality (WebGL 2 / OpenGL ES 3) but are structured very differently:

- ANGLE's test suite has one or two tests per functionality to check it works correctly, plus
  regression tests and special tests to cover implementation details.
- WebGL2's CTS can have thousands of tests per API aspect to cover every combination of
  parameters (and global state) used by an operation.

Below are guidelines based on our collective experience with graphics API CTSes like WebGL's.
They are expected to evolve over time and have exceptions, but should give a general idea of what
to do.

## Contributing

Testing tasks are tracked in the [CTS project tracker](https://github.com/orgs/gpuweb/projects/3).
Go here if you're looking for tasks, or if you have a test idea that isn't already covered.

If contributing conformance tests, the directory you'll work in is [`src/webgpu/`](../src/webgpu/).
This directory is organized according to the goal of the test (API validation behavior vs
actual results) and its target (API entry points and spec areas, e.g. texture sampling).

The contents of a test file (`src/webgpu/**/*.spec.ts`) are twofold:

- Documentation ("test plans") on what tests do, how they do it, and what cases they cover.
  Some test plans are fully or partially unimplemented:
  they either contain "TODO" in a description or are `.unimplemented()`.
- Actual tests.

**Please read the following short documents before contributing.**

### 0. [Developing](developing.md)

- Reviewers should also read [Review Requirements](../reviews.md).

### 1. [Life of a Test Change](life_of.md)

### 2. [Adding or Editing Test Plans](plans.md)

### 3. [Implementing Tests](tests.md)

## [Additional Documentation](../)

## Examples

### Operation testing of vertex input id generation

This section provides an example of the planning process for a test.
It has not been refined into a set of final test plan descriptions.
(Note: this predates the actual implementation of these tests, so doesn't match the actual tests.)

Somewhere under the `api/operation` node are tests checking that running `GPURenderPipelines` on
the device using the `GPURenderEncoderBase.draw` family of functions works correctly. Render
pipelines are composed of several stages that are mostly independent so they can be split in
several parts such as `vertex_input`, `rasterization`, `blending`.

Vertex input itself has several parts that are mostly separate in hardware:

- generation of the vertex and instance indices to run for this draw
- fetching of vertex data from vertex buffers based on these indices
- conversion from the vertex attribute `GPUVertexFormat` to the datatype for the input variable
  in the shader

Each of these are tested separately and have cases for each combination of the variables that may
affect them. This means that `api/operation/render/vertex_input/id_generation` checks that the
correct operation is performed for the cartesian product of all the following dimensions:

- for encoding in a `GPURenderPassEncoder` or a `GPURenderBundleEncoder`
- whether the draw is direct or indirect
- whether the draw is indexed or not
- for various values of the `firstInstance` argument
- for various values of the `instanceCount` argument
- if the draw is not indexed:
    - for various values of the `firstVertex` argument
    - for various values of the `vertexCount` argument
- if the draw is indexed:
    - for each `GPUIndexFormat`
    - for various values of the indices in the index buffer including the primitive restart values
    - for various values for the `offset` argument to `setIndexBuffer`
    - for various values of the `firstIndex` argument
    - for various values of the `indexCount` argument
    - for various values of the `baseVertex` argument

"Various values" above mean several small values, including `0` and the second smallest valid
value to check for corner cases, as well as some large value.

An instance of the test sets up a `draw*` call based on the parameters, using point rendering and
a fragment shader that outputs to a storage buffer. After the draw the test checks the content of
the storage buffer to make sure all expected vertex shader invocation, and only these ones have
been generated.
