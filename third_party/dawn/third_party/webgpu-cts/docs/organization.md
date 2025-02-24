# Test Organization

## `src/webgpu/`

Because of the glorious amount of test needed, the WebGPU CTS is organized as a tree of arbitrary
depth (a filesystem with multiple tests per file).

Each directory may have a `README.txt` describing its contents.
Tests are grouped in large families (each of which has a `README.txt`);
the root and first few levels looks like the following (some nodes omitted for simplicity):

- **`api`** with tests for full coverage of the Javascript API surface of WebGPU.
    - **`validation`** with positive and negative tests for all the validation rules of the API.
    - **`operation`** with tests that checks the result of performing valid WebGPU operations,
      taking advantage of parametrization to exercise interactions between parts of the API.
    - **`regression`** for one-off tests that reproduce bugs found in implementations to prevent
      the bugs from appearing again.
- **`shader`** with tests for full coverage of the shaders that can be passed to WebGPU.
    - **`validation`**.
    - **`execution`** similar to `api/operation`.
    - **`regression`**.
- **`idl`** with tests to check that the WebGPU IDL is correctly implemented, for examples that
  objects exposed exactly the correct members, and that methods throw when passed incomplete
  dictionaries.
- **`web-platform`** with tests for Web platform-specific interactions like `GPUSwapChain` and
  `<canvas>`, WebXR and `GPUQueue.copyExternalImageToTexture`.

At the same time test hierarchies can be used to split the testing of a single sub-object into
several file for maintainability. For example `GPURenderPipeline` has a large descriptor and some
parts could be tested independently like `vertex_input` vs. `primitive_topology` vs. `blending`
but all live under the `render_pipeline` directory.

In addition to the test tree, each test can be parameterized. For coverage it is important to
test all enums values, for example for `GPUTextureFormat`. Instead of having a loop to iterate
over all the `GPUTextureFormat`, it is better to parameterize the test over them. Each format
will have a different entry in the test list which will help WebGPU implementers debug the test,
or suppress the failure without losing test coverage while they fix the bug.

Extra capabilities (limits and features) are often tested in the same files as the rest of the API.
For example, a compressed texture format capability would simply add a `GPUTextureFormat` to the
parametrization lists of many tests, while a capability adding significant new functionality
like ray-tracing could have a separate subtree.

Operation tests for optional features should be skipped using `t.selectDeviceOrSkipTestCase()` or
`t.skip()`. Validation tests should be written that test the behavior with and without the
capability enabled via `t.selectDeviceOrSkipTestCase()`, to ensure the functionality is valid
only with the capability enabled.

### Validation tests

Validation tests check the validation rules that are (or will be) set by the
WebGPU spec. Validation tests try to carefully trigger the individual validation
rules in the spec, without simultaneously triggering other rules.

Validation errors *generally* generate WebGPU errors, not exceptions.
But check the spec on a case-by-case basis.

Like all `GPUTest`s, `ValidationTest`s are wrapped in both types of error scope. These
"catch-all" error scopes look for any errors during the test, and report them as test failures.
Since error scopes can be nested, validation tests can nest an error scope to expect that there
*are* errors from specific operations.

#### Parameterization

Test parameterization can help write many validation tests more succinctly,
while making it easier for both authors and reviewers to be confident that
an aspect of the API is tested fully. Examples:

- [`webgpu:api,validation,render_pass,resolve:resolve_attachment:*`](https://github.com/gpuweb/cts/blob/ded3b7c8a4680a1a01621a8ac859facefadf32d0/src/webgpu/api/validation/render_pass/resolve.spec.ts#L35)
- [`webgpu:api,validation,createBindGroupLayout:bindingTypeSpecific_optional_members:*`](https://github.com/gpuweb/cts/blob/ded3b7c8a4680a1a01621a8ac859facefadf32d0/src/webgpu/api/validation/createBindGroupLayout.spec.ts#L68)

Use your own discretion when deciding the balance between heavily parameterizing
a test and writing multiple separate tests.

#### Guidelines

There are many aspects that should be tested in all validation tests:

- each individual argument to a method call (including `this`) or member of a descriptor
  dictionary should be tested including:
    - what happens when an error object is passed.
    - what happens when an optional feature enum or method is used.
    - what happens for numeric values when they are at 0, too large, too small, etc.
- each validation rule in the specification should be checked both with a control success case,
  and error cases.
- each set of arguments or state that interact for validation.

When testing numeric values, it is important to check on both sides of the boundary: if the error
happens for value N and not N - 1, both should be tested. Alignment of integer values should also
be tested but boundary testing of alignment should be between a value aligned to 2^N and a value
aligned to 2^(N-1).

Finally, this is probably also where we would test that extensions follow the rule that: if the
browser supports a feature but it is not enabled on the device, then calling methods from that
feature throws `TypeError`.

- Test providing unknown properties *that are definitely not part of any feature* are
  valid/ignored. (Unfortunately, due to the rules of IDL, adding a member to a dictionary is
  always a breaking change. So this is how we have to test this unless we can get a "strict"
  dictionary type in IDL. We can't test adding members from non-enabled extensions.)

### Operation tests

Operation tests test the actual results of using the API. They execute
(sometimes significant) code and check that the result is within the expected
set of behaviors (which can be quite complex to compute).

Note that operation tests need to test a lot of interactions between different
parts of the API, and so can become quite complex. Try to reduce the complexity by
utilizing combinatorics and [helpers](./helper_index.txt), and splitting/merging test files as needed.

#### Errors

Operation tests are usually `GPUTest`s. As a result, they automatically fail on any validation
errors that occur during the test.

When it's easier to write an operation test with invalid cases, use
`ParamsBuilder.filter`/`.unless` to avoid invalid cases, or detect and
`expect` validation errors in some cases.

#### Implementation

Use helpers like `expectContents` (and more to come) to check the values of data on the GPU.
(These are "eventual expectations" - the harness will wait for them to finish at the end).

When testing something inside a shader, it's not always necessary to output the result to a
render output. In fragment shaders, you can output to a storage buffer. In vertex shaders, you
can't - but you can render with points (simplest), send the result to the fragment shader, and
output it from there. (Someday, we may end up wanting a helper for this.)

#### Testing Default Values

Default value tests (for arguments and dictionary members) should usually be operation tests -
all you have to do is include `undefined` in parameterizations of other tests to make sure the
behavior with `undefined` has the same expected result that you have when the default value is
specified explicitly.

### IDL tests

TODO: figure out how to implement these. https://github.com/gpuweb/cts/issues/332

These tests test only rules that come directly from WebIDL. For example:

- Values out of range for `[EnforceRange]` cause exceptions.
- Required function arguments and dictionary members cause exceptions if omitted.
- Arguments and dictionary members cause exceptions if passed the wrong type.

They may also test positive cases like the following, but the behavior of these should be tested in
operation tests.

- OK to omit optional arguments/members.
- OK to pass the correct argument/member type (or of any type in a union type).

Every overload of every method should be tested.

## `src/stress/`, `src/manual/`

Stress tests and manual tests for WebGPU that are not intended to be run in an automated way.

## `src/unittests/`

Unit tests for the test framework (`src/common/framework/`).

## `src/demo/`

A demo of test hierarchies for the purpose of testing the `standalone` test runner page.
