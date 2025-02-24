# Review Requirements

A review should have several items checked off before it is landed.
Checkboxes are pre-filled into the pull request summary when it's created.

The uploader may pre-check-off boxes if they are not applicable
(e.g. TypeScript readability on a plan PR).

## Readability

A reviewer has "readability" for a topic if they have enough expertise in that topic to ensure
good practices are followed in pull requests, or know when to loop in other reviewers.
Perfection is not required!

**It is up to reviewers' own discretion** whether they are qualified to check off a
"readability" checkbox on any given pull request.

- WebGPU Readability: Familiarity with the API to ensure:

    - WebGPU is being used correctly; expected results seem reasonable.
    - WebGPU is being tested completely; tests have control cases.
    - Test code has a clear correspondence with the test description.
    - [Test helpers](./helper_index.txt) are used or created appropriately
      (where the reviewer is familiar with the helpers).

- TypeScript Readability: Make sure TypeScript is utilized in a way that:

    - Ensures test code is reasonably type-safe.
      Reviewers may recommend changes to make type-safety either weaker (`as`, etc.) or stronger.
    - Is understandable and has appropriate verbosity and dynamicity
      (e.g. type inference and `as const` are used to reduce unnecessary boilerplate).

## Plan Reviews

**Changes *must* have an author or reviewer with the following readability:** WebGPU

Reviewers must carefully ensure the following:

- The test plan name accurately describes the area being tested.
- The test plan covers the area described by the file/test name and file/test description
  as fully as possible (or adds TODOs for incomplete areas).
- Validation tests have control cases (where no validation error should occur).
- Each validation rule is tested in isolation, in at least one case which does not validate any
  other validation rules.

See also: [Adding or Editing Test Plans](intro/plans.md).

## Implementation Reviews

**Changes *must* have an author or reviewer with the following readability:** WebGPU, TypeScript

Reviewers must carefully ensure the following:

- The coverage of the test implementation precisely matches the test description.
- Everything required for test plan reviews above.

Reviewers should ensure the following:

- New test helpers are documented in [helper index](./helper_index.txt).
- Framework and test helpers are used where they would make test code clearer.

See also: [Implementing Tests](intro/tests.md).

## Framework

**Changes *must* have an author or reviewer with the following readability:** TypeScript

Reviewers should ensure the following:

- Changes are reasonably type-safe, and covered by unit tests where appropriate.
