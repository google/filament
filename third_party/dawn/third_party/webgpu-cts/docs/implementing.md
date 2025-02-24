# Test Implementation

Concepts important to understand when writing tests. See existing tests for examples to copy from.

## Test fixtures

Most tests can use one of the several common test fixtures:

- `Fixture`: Base fixture, provides core functions like `expect()`, `skip()`.
- `GPUTest`: Wraps every test in error scopes. Provides helpers like `expectContents()`.
- `ValidationTest`: Extends `GPUTest`, provides helpers like `expectValidationError()`, `getErrorTextureView()`.
- Or create your own. (Often not necessary - helper functions can be used instead.)

Test fixtures or helper functions may be defined in `.spec.ts` files, but if used by multiple
test files, should be defined in separate `.ts` files (without `.spec`) alongside the files that
use them.

### GPUDevices in tests

`GPUDevice`s are largely stateless (except for `lost`-ness, error scope stack, and `label`).
This allows the CTS to reuse one device across multiple test cases using the `DevicePool`,
which provides `GPUDevice` objects to tests.

Currently, there is one `GPUDevice` with the default descriptor, and
a cache of several more, for devices with additional capabilities.
Devices in the `DevicePool` are automatically removed when certain things go wrong.

Later, there may be multiple `GPUDevice`s to allow multiple test cases to run concurrently.

## Test parameterization

The CTS provides helpers (`.params()` and friends) for creating large cartesian products of test parameters.
These generate "test cases" further subdivided into "test subcases".
See `basic,*` in `examples.spec.ts` for examples, and the [helper index](./helper_index.txt)
for a list of capabilities.

Test parameterization should be applied liberally to ensure the maximum coverage
possible within reasonable time. You can skip some with `.filter()`. And remember: computers are
pretty fast - thousands of test cases can be reasonable.

Use existing lists of parameters values (such as
[`kTextureFormats`](https://github.com/gpuweb/cts/blob/0f38b85/src/suites/cts/capability_info.ts#L61),
to parameterize tests), instead of making your own list. Use the info tables (such as
`kTextureFormatInfo`) to define and retrieve information about the parameters.

## Asynchrony in tests

Since there are no synchronous operations in WebGPU, almost every test is asynchronous in some
way. For example:

- Checking the result of a readback.
- Capturing the result of a `popErrorScope()`.

That said, test functions don't always need to be `async`; see below.

### Checking asynchronous errors/results

Validation is inherently asynchronous (`popErrorScope()` returns a promise). However, the error
scope stack itself is synchronous - operations immediately after a `popErrorScope()` are outside
that error scope.

As a result, tests can assert things like validation errors/successes without having an `async`
test body.

**Example:**

```typescript
t.expectValidationError(() => {
    device.createThing();
});
```

does:

- `pushErrorScope('validation')`
- `popErrorScope()` and "eventually" check whether it returned an error.

**Example:**

```typescript
t.expectGPUBufferValuesEqual(srcBuffer, expectedData);
```

does:

- copy `srcBuffer` into a new mappable buffer `dst`
- `dst.mapReadAsync()`, and "eventually" check what data it returned.

Internally, this is accomplished via an "eventual expectation": `eventualAsyncExpectation()`
takes an async function, calls it immediately, and stores off the resulting `Promise` to
automatically await at the end before determining the pass/fail state.

### Asynchronous parallelism

A side effect of test asynchrony is that it's possible for multiple tests to be in flight at
once. We do not currently do this, but it will eventually be an option to run `N` tests in
"parallel", for faster local test runs.
