# How to become a contributor and submit your own code

## Contributor License Agreement

Contributions to this project must be accompanied by a Contributor License
Agreement. You (or your employer) retain the copyright to your contribution;
this simply gives us permission to use and redistribute your contributions as
part of the project. Head over to <https://cla.developers.google.com/> to see
your current agreements on file or to sign a new one.

You generally only need to submit a CLA once, so if you've already submitted one
(even if it was for a different project), you probably don't need to do it
again.

## Contributing A Patch

1. Submit an issue describing your proposed change to the repo in question.
1. The repo owner will respond to your issue promptly.
1. If your proposed change is accepted, and you haven't already done so, sign a
   Contributor License Agreement (see details above).
1. Fork the desired repo, develop and test your code changes.
1. Ensure that your code adheres to the existing style in the sample to which
   you are contributing. Refer to [CodeStyle.md](/CODE_STYLE.md) for the recommended coding
   standards for this project.
1. Ensure that your code has an appropriate set of unit tests which all pass.
1. Submit a pull request.

## Code Style

See [CODE_STYLE.md](/CODE_STYLE.md)

## Code reviews

All submissions, including submissions by project members, require review. We
use GitHub pull requests for this purpose. Consult
[GitHub Help](https://help.github.com/articles/about-pull-requests/) for more
information on using pull requests.

## Community Guidelines

This project follows
[Google's Open Source Community Guidelines](https://opensource.google.com/conduct/).

## Dependencies

One of our design goals is that Filament itself should have no dependencies or as few dependencies
as possible. The current external dependencies of the runtime library include:

- STL
- robin-map (header only library)

When building with Vulkan enabled, we have a few additional small dependencies:

- vkmemalloc
- smol-v

Host tools (such as `matc` or `cmgen`) can use external dependencies freely.
