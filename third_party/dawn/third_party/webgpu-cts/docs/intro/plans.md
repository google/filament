# Adding or Editing Test Plans

## 1. Write a test plan

For new tests, if some notes exist already, incorporate them into your plan.

A detailed test plan should be written and reviewed before substantial test code is written.
This allows reviewers a chance to identify additional tests and cases, opportunities for
generalizations that would improve the strength of tests, similar existing tests or test plans,
and potentially useful [helpers](../helper_index.txt).

**A test plan must serve two functions:**

- Describes the test, succinctly, but in enough detail that a reader can read *only* the test
  plans and evaluate coverage completeness of a file/directory.
- Describes the test precisely enough that, when code is added, the reviewer can ensure that the
  test really covers what the test plan says.

There should be one test plan for each test. It should describe what it tests, how, and describe
important cases that need to be covered. Here's an example:

```ts
g.test('x,some_detail')
  .desc(
    `
Tests [some detail] about x. Tests calling x in various 'mode's { mode1, mode2 },
with various values of 'arg', and checks correctness of the result.
Tries to trigger [some conditional path].

- Valid values (control case) // <- (to make sure the test function works well)
- Unaligned values (should fail) // <- (only validation tests need to intentionally hit invalid cases)
- Extreme values`
  )
  .params(u =>
    u //
      .combine('mode', ['mode1', 'mode2'])
      .beginSubcases()
      .combine('arg', [
        // Valid  // <- Comment params as you see fit.
        4,
        8,
        100,
        // Invalid
        2,
        6,
        1e30,
      ])
  )
  .unimplemented();
```

"Cases" each appear as individual items in the `/standalone/` runner.
"Subcases" run inside each case, like a for-loop wrapping the `.fn(`test function`)`.
Documentation on the parameter builder can be found in the [helper index](../helper_index.txt).

It's often impossible to predict the exact case/subcase structure before implementing tests, so they
can be added during implementation, instead of planning.

For any notes which are not specific to a single test, or for preliminary notes for tests that
haven't been planned in full detail, put them in the test file's `description` variable at
the top. Or, if they aren't associated with a test file, put them in a `README.txt` file.

**Any notes about missing test coverage must be marked with the word `TODO` inside a
description or README.** This makes them appear on the `/standalone/` page.

## 2. Open a pull request

Open a PR, and work with the reviewer(s) to revise the test plan.

Usually (probably), plans will be landed in separate PRs before test implementations.

## Conventions used in test plans

- `Iff`: If and only if
- `x=`: "cartesian-cross equals", like `+=` for cartesian product.
  Used for combinatorial test coverage.
    - Sometimes this will result in too many test cases; simplify/reduce as needed
      during planning *or* implementation.
- `{x,y,z}`: list of cases to test
    - e.g. `x= texture format {r8unorm, r8snorm}`
- *Control case*: a case included to make sure that the rest of the cases aren't
  missing their target by testing some other error case.
