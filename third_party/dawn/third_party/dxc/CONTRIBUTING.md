# How to contribute

One of the easiest ways to contribute is to participate in discussions and discuss issues. You can also contribute by submitting pull requests with code changes.

## General feedback and discussions?

Please start a discussion on the repo issue tracker.

## Bugs and feature requests?

For non-security related bugs please log a new issue in the GitHub repo.

## Reporting security issues and bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://technet.microsoft.com/en-us/security/ff852094.aspx).

## Filing issues

When filing issues, please use our [bug filing templates](https://github.com/aspnet/Home/wiki/Functional-bug-template).
The best way to get your bug fixed is to be as detailed as you can be about the problem.
Providing a minimal project with steps to reproduce the problem is ideal.
Here are questions you can answer before you file a bug to make sure you're not missing any important information.

1. Did you read the documentation?
2. Did you include the snippet of broken code in the issue?
3. What are the *EXACT* steps to reproduce this problem?
4. What version are you using?

GitHub supports [markdown](https://help.github.com/articles/github-flavored-markdown/), so when filing bugs make sure you check the formatting before clicking submit.

## Contributing code and content

You will need to complete a Contributor License Agreement (CLA) before your pull request can be accepted. This agreement testifies that you are granting us permission to use the source code you are submitting, and that this work is being submitted under appropriate license that we can use it.

You can complete the CLA by going through the steps at the [Contribution License Agreement site](https://cla.microsoft.com). Once we have received the signed CLA, we'll review the request. You will only need to do this once.

Make sure you can build the code. Familiarize yourself with the project workflow and our coding conventions. If you don't know what a pull request is read this article: <https://help.github.com/articles/using-pull-requests>.

Before submitting a feature or substantial code contribution please discuss it with the team and ensure it follows the product roadmap. You might also read these two blogs posts on contributing code: [Open Source Contribution Etiquette](http://tirania.org/blog/archive/2010/Dec-31.html) by Miguel de Icaza and [Don't "Push" Your Pull Requests](https://www.igvita.com/2011/12/19/dont-push-your-pull-requests/) by Ilya Grigorik. Note that all code submissions will be rigorously reviewed and tested by the team, and only those that meet an extremely high bar for both quality and design/roadmap appropriateness will be merged into the source.

### Coding guidelines

The coding, style, and general engineering guidelines follow those described in the [LLVM Coding Standards](docs/CodingStandards.rst). For additional guidelines in code specific to HLSL, see the [HLSL Changes](docs/HLSLChanges.rst) docs.

DXC has adopted a clang-format requirement for all incoming changes to C and C++ files. PRs to DXC should have the *changed code* clang formatted to the LLVM style, and leave the remaining portions of the file unchanged. This can be done using the `git-clang-format` tool or IDE driven workflows. A GitHub action will run on all PRs to validate that the change is properly formatted.

#### Applying LLVM Standards

All new code contributed to DXC should follow the LLVM coding standards.

Note that the LLVM Coding Standards have a golden rule:

> **If you are extending, enhancing, or bug fixing already implemented code, use the style that is already being used so that the source is uniform and easy to follow.**

The golden rule should continue to be applied to places where DXC is self-consistent. A good example is DXC's common use of `PascalCase` instead of `camelCase` for APIs in some parts of the HLSL implementation. In any place where DXC is not self-consistent new code should follow the LLVM Coding Standard.

A good secondary rule to follow is:

> **When in doubt, follow LLVM.**

Adopting LLVM's coding standards provides a consistent set of rules and guidelines to hold all contributions to. This allows patch authors to clearly understand the expectations placed on contributions, and allows reviewers to have a bar to measure contributions against. Aligning with LLVM by default ensures the path of least resistance for everyone.

Since many of the LLVM Coding Standards are not enforced automatically we rely on code reviews to provide feedback and ensure contributions align with the expected coding standards. Since we rely on reviewers for enforcement and humans make mistakes, please keep in mind:

> **Code review is a conversation.**

It is completely reasonable for a patch author to question feedback and provide additional context about why something was done the way it was. Reviewers often see narrow slices in diffs rather than the full context of a file or part of the compiler, so they may not always provide perfect feedback. This is especially true with the application of the "golden rule" since it depends on understanding a wider context.

### Documenting Pull Requests

Pull request descriptions should have the following format:

```md
Title summary of the changes (Less than 80 chars)
 - Description Detail 1
 - Description Detail 2

Fixes #bugnumber (Where relevant. In this specific format)
```

#### Titles

The title should focus on what the change intends to do rather than how it was done.
The description can and should explain how it was done if not obvious.

Titles under 76 characters print nicely in unix terminals under `git log`.
This is not a hard requirement, but is good guidance.

Tags in titles allow for speedy categorization
Title tags  are generally one word or acronym enclosed in square brackets.
Limiting to one or two tags is ideal to keep titles short.
Some examples of common tags are:

- `[NFC]` - No Functional Change
- `[RFC]` - Request For Comments (often used for drafts to get feedback)
- `[Doc]` - Documentation change
- `[SPIRV]` - Changes related to SPIR-V
- `[HLSL2021]` - Changes related to HLSL 2021 features
- Other tags in use: `[Linux]`, `[mac]`, `[Win]`, `[PIX]`, etc...

Tags aren't formalized or any specific limited set.
If you're unsure of a reasonable tag to use, just don't use any.
If you want to invent a new tag, go for it!
These are to help categorize changes at a glance.

#### Descriptions

The PR description should include a more detailed description of the change,
 an explanation for the motivation of the change, and links to any relevant Issues.
This does not need to be a dissertation, but should leave breadcrumbs for the next person debugging your code (who might be you).

Using the words `Fixes`, `Fixed`, `Closes`, `Closed`, or `Close` followed by
 `#<issuenumber>`, will auto close an issue after the PR is merged.

#### Release Notes

Significant changes may require release notes that highlight important
 compiler behavior changes for each named release.
These include changes that are:

- Visible to the users
- Significant changes to compiler behavior
  - New features: Language, Hardware support, compiler options
  - Important bug fixes
  - Changes in default behavior

When such a change is made, the release note should be included as part of that change.
This is done in the docs/ReleaseNotes.md file.

If the change is meant for a named release, it should be added to that named release's section of the release notes file.
As the change is merged to the appropriate release branches, the release notes will come along with it.

If a change is meant for the next upcoming release, it should be added to the "Upcoming Release" section.
When the next upcoming release is named, the title will be updated and the release note will be included in the appropriate release.

When writing release note list entries:

- Keep the description to a single sentence.
- Links to specific PRs shouldn't be included.
- Markdown links to bugs are encouraged if the issue is too complicated to completely explain in a single sentence.
- Remember to update release notes as the nature of the change alters or is removed.

### Testing Pull Requests

All changes that make functional or behavioral changes to the compiler whether by fixing bugs or adding features
 must include additional testing in the implementing pull request.
Changes that do not change behavior may still be required to add testing if the change impacts areas with limited test coverage
 to verify that the change doesn't alter previously untested, but important behavior.
For bug fixes, at least one added test should fail in the absence of your non-test code changes.
Tests should include reasonable permutations of the target fix/change.
Include baseline changes with your change as needed.

Submitting a pull request kicks off an automated set of regression tests that verify the change introduces no unwanted changes in behavior.
For a pull request to be mergeable in GitHub, it will have to pass this regression test suite.
Changes made to DXC for the benefit of external projects should be verified using that project's testing protocols to avoid churn.

For cases where any of the above testing requirements are not possible, please specify why in the pull request.

### Merging Pull Requests

Pull requests should be a child commit of a reasonably recent commit in the main branch.
A pull request's commits should be squashed on merging except in very special circumstances usually involving release branches.

Ensure that the title and description are fully up to date before merging.
The title and description feed the final git commit message, and we want to ensure high quality commit messages in the repository history.
