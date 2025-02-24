# Life of a Test Change

A "test change" could be a new test, an expansion of an existing test, a test bug fix, or a
modification to existing tests to make them match new spec changes.

**CTS contributors should contribute to the tracker and strive to keep it up to date, especially
relating to their own changes.**

Filing new draft issues in the CTS project tracker is very lightweight.
Anyone with access should do this eagerly, to ensure no testing ideas are forgotten.
(And if you don't have access, just file a regular issue.)

1. Enter a [draft issue](https://github.com/orgs/gpuweb/projects/3), with the Status
    set to "New (not in repo)", and any available info included in the issue description
    (notes/plans to ensure full test coverage of the change). The source of this may be:

    - Anything in the spec/API that is found not to be covered by the CTS yet.
    - Any test is found to be outdated or otherwise buggy.
    - A spec change from the "Needs CTS Issue" column in the
      [spec project tracker](https://github.com/orgs/gpuweb/projects/1).
      Once information on the required test changes is entered into the CTS project tracker,
      the spec issue moves to "Specification Done".

    Note: at some point, someone may make a PR to flush "New (not in repo)" issues into `TODO`s in
    CTS file/test description text, changing their "Status" to "Open".
    These may be done in bulk without linking back to the issue.

1. As necessary:

    - Convert the draft issue to a full, numbered issue for linking from later PRs.

        ![convert to issue button screenshot](convert_to_issue.png)

    - Update the "Assignees" of the issue when an issue is assigned or unassigned
      (you can assign yourself).
    - Change the "Status" of the issue to "Started" once you start the task.

1. Open one or more PRs, **each linking to the associated issue**.
    Each PR may is reviewed and landed, and may leave further TODOs for parts it doesn't complete.

    1. Test are "planned" in test descriptions. (For complex tests, open a separate PR with the
      tests `.unimplemented()` so a reviewer can evaluate the plan before you implement tests.)
    1. Tests are implemented.

1. When **no TODOs remain** for an issue, close it and change its status to "Complete".
    (Enter a new more, specific draft issue into the tracker if you need to track related TODOs.)
