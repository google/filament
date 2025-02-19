# Contributing to SPIR-V Tools

## For users: Reporting bugs and requesting features

We organize known future work in GitHub projects. See
[Tracking SPIRV-Tools work with GitHub projects](https://github.com/KhronosGroup/SPIRV-Tools/blob/main/docs/projects.md)
for more.

To report a new bug or request a new feature, please file a GitHub issue. Please
ensure the bug has not already been reported by searching
[issues](https://github.com/KhronosGroup/SPIRV-Tools/issues) and
[projects](https://github.com/KhronosGroup/SPIRV-Tools/projects). If the bug has
not already been reported open a new one
[here](https://github.com/KhronosGroup/SPIRV-Tools/issues/new).

When opening a new issue for a bug, make sure you provide the following:

*   A clear and descriptive title.
    *   We want a title that will make it easy for people to remember what the
        issue is about. Simply using "Segfault in spirv-opt" is not helpful
        because there could be (but hopefully aren't) multiple bugs with
        segmentation faults with different causes.
*   A test case that exposes the bug, with the steps and commands to reproduce
    it.
    *   The easier it is for a developer to reproduce the problem, the quicker a
        fix can be found and verified. It will also make it easier for someone
        to possibly realize the bug is related to another issue.

For feature requests, we use
[issues](https://github.com/KhronosGroup/SPIRV-Tools/issues) as well. Please
create a new issue, as with bugs. In the issue provide

*   A description of the problem that needs to be solved.
*   Examples that demonstrate the problem.

## For developers: Contributing a patch

Before we can use your code, you must sign the
[Khronos Open Source Contributor License Agreement](https://cla-assistant.io/KhronosGroup/SPIRV-Tools)
(CLA), which you can do online. The CLA is necessary mainly because you own the
copyright to your changes, even after your contribution becomes part of our
codebase, so we need your permission to use and distribute your code. We also
need to be sure of various other things -- for instance that you'll tell us if
you know that your code infringes on other people's patents. You don't have to
sign the CLA until after you've submitted your code for review and a member has
approved it, but you must do it before we can put your code into our codebase.

See
[README.md](https://github.com/KhronosGroup/SPIRV-Tools/blob/main/README.md)
for instruction on how to get, build, and test the source. Once you have made
your changes:

*   Ensure the code follows the
    [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
    Running `clang-format -style=file -i [modified-files]` can help.
*   Create a pull request (PR) with your patch.
*   Make sure the PR description clearly identified the problem, explains the
    solution, and references the issue if applicable.
*   If your patch completely fixes bug 1234, the commit message should say
    `Fixes https://github.com/KhronosGroup/SPIRV-Tools/issues/1234` When you do
    this, the issue will be closed automatically when the commit goes into
    main. Also, this helps us update the [CHANGES](CHANGES) file.
*   Watch the continuous builds to make sure they pass.
*   Request a code review.

The reviewer can either approve your PR or request changes. If changes are
requested:

*   Please add new commits to your branch, instead of amending your commit.
    Adding new commits makes it easier for the reviewer to see what has changed
    since the last review.
*   Once you are ready for another round of reviews, add a comment at the
    bottom, such as "Ready for review" or "Please take a look" (or "PTAL"). This
    explicit handoff is useful when responding with multiple small commits.

After the PR has been reviewed it is the job of the reviewer to merge the PR.
Instructions for this are given below.

## For maintainers: Reviewing a PR

The formal code reviews are done on GitHub. Reviewers are to look for all of the
usual things:

*   Coding style follows the
    [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
*   Identify potential functional problems.
*   Identify code duplication.
*   Ensure the unit tests have enough coverage.
*   Ensure continuous integration (CI) bots run on the PR. If not run (in the
    case of PRs by external contributors), add the "kokoro:run" label to the
    pull request which will trigger running all CI jobs.

When looking for functional problems, there are some common problems reviewers
should pay particular attention to:

*   Does the code work for both Shader (Vulkan and OpenGL) and Kernel (OpenCL)
    scenarios? The respective SPIR-V dialects are slightly different.
*   Changes are made to a container while iterating through it. You have to be
    careful that iterators are not invalidated or that elements are not skipped.
*   For SPIR-V transforms: The module is changed, but the analyses are not
    updated. For example, a new instruction is added, but the def-use manager is
    not updated. Later on, it is possible that the def-use manager will be used,
    and give wrong results.
*   If a pass gets the id of a type from the type manager, make sure the type is
    not a struct or array. It there are two structs that look the same, the type
    manager can return the wrong one.

## For maintainers: Merging a PR

We intend to maintain a linear history on the GitHub main branch, and the
build and its tests should pass at each commit in that history. A linear
always-working history is easier to understand and to bisect in case we want to
find which commit introduced a bug. The
[Squash and Merge](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/incorporating-changes-from-a-pull-request/about-pull-request-merges#squash-and-merge-your-commits)
button on the GitHub web interface. All other ways of merging on the web
interface have been disabled.

Before merging, we generally require:

1.  All tests except for the smoke test pass. See
    [failing smoke test](#failing-smoke-test).
1.  The PR is approved by at least one of the maintainers. If the PR modifies
    different parts of the code, then multiple reviewers might be necessary.

The squash-and-merge button will turn green when these requirements are met.
Maintainers have the to power to merge even if the button is not green, but that
is discouraged.

### Failing smoke test

The purpose of the smoke test is to let us know if
[shaderc](https://github.com/google/shaderc) fails to build with the change. If
it fails, the maintainer needs to determine if the reason for the failure is a
problem in the current PR or if another repository needs to be changed. Most of
the time [Glslang](https://github.com/KhronosGroup/glslang) needs to be updated
to account for the change in SPIR-V Tools.

The PR can still be merged if the problem is not with that PR.

## For maintainers: Running tests

For security reasons, not all tests will run automatically. When they do not, a
maintainer will have to start the tests.

If the Github actions tests do not run on a PR, they can be initiated by closing
and reopening the PR.

If the kokoro tests are not run, they can be run by adding the label
`kokoro:run` to the PR.
