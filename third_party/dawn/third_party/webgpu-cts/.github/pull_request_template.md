


Issue: #<!-- Fill in the issue number here. See docs/intro/life_of.md -->

<hr>

**Requirements for PR author:**

- [ ] All missing test coverage is tracked with "TODO" or `.unimplemented()`.
- [ ] New helpers are `/** documented */` and new helper files are found in `helper_index.txt`.
- [ ] Test behaves as expected in a WebGPU implementation. (If not passing, explain above.)
- [ ] Test have be tested with compatibility mode validation enabled and behave as expected. (If not passing, explain above.)

**Requirements for [reviewer sign-off](https://github.com/gpuweb/cts/blob/main/docs/reviews.md):**

- [ ] Tests are properly located in the test tree.
- [ ] [Test descriptions](https://github.com/gpuweb/cts/blob/main/docs/intro/plans.md) allow a reader to "read only the test plans and evaluate coverage completeness", and accurately reflect the test code.
- [ ] Tests provide complete coverage (including validation control cases). **Missing coverage MUST be covered by TODOs.**
- [ ] Helpers and types promote readability and maintainability.

When landing this PR, be sure to make any necessary issue status updates.
