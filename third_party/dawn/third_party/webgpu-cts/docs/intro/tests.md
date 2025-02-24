# Implementing Tests

Once a test plan is done, you can start writing tests.
To add new tests, imitate the pattern in neigboring tests or neighboring files.
New test files must be named ending in `.spec.ts`.

For an example test file, see [`src/webgpu/examples.spec.ts`](../../src/webgpu/examples.spec.ts).
For a more complex, well-structured reference test file, see
[`src/webgpu/api/validation/vertex_state.spec.ts`](../../src/webgpu/api/validation/vertex_state.spec.ts).

Implement some tests and open a pull request. You can open a PR any time you're ready for a review.
(If two tests are non-trivial but independent, consider separate pull requests.)

Before uploading, you can run pre-submit checks (`npm test`) to make sure it will pass CI.
Use `npm run fix` to fix linting issues.

## Test Helpers

It's best to be familiar with helpers available in the test suite for simplifying
test implementations.

New test helpers can be added at any time to either of those files, or to new `.ts` files anywhere
near the `.spec.ts` file where they're used.

Documentation on existing helpers can be found in the [helper index](../helper_index.txt).
