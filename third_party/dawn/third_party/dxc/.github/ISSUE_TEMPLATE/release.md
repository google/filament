---
name: Release
about: Create a tracking issue for a release
title: 'Release 1.x.xxxx'
labels: []
assignees: ''

---

# Schedule

- [ ] MM/DD/YYYY - Release branch forks from `main`
    - At this point, changes must be cherry-picked into the release branch in
      order for them to be included in the release.
- [ ] MM/DD/YYYY - Release Candidate 1 (begin Ask Mode[^1] for release branch).
    - At this point, cherry-picked changes must be approved by @microsoft/hlsl-release
- [ ] MM/DD/YYYY - Final Release Candidate
- [ ] MM/DD/YYYY - Target Release Date


# Tasks

## Before Fork

This part of the release process is to 'prime the pump' - that is to make sure
that all the various parts of the engineering system are set into place so that
we are confident we can generate builds for the new branch

- [ ] Update version numbers in utils/version/latest-release.json and utils/version/version.inc
- [ ] Create the release branch from `main`
    - The release branch is kept into sync with main via regular fast-forward
      merges.
- [ ] Internal branches and build pipelines configured
    - Verify that the engineering system can build:
    - [ ] Zip files for github release
    - [ ] NuGet package
    - [ ] VPack
- [ ] Final merge of `main` into the release branch

## After Fork

- [ ] Update README.md if necessary
- [ ] Create draft of Release post on GitHub

## Quality Sign Off

- [ ] Microsoft Testing Sign-off (@pow2clk)
- [ ] Google Testing Sign-off (@s-perron / @Keenuts)

## Release

- [ ] Tag final release and post binaries


[^1]: [Ask Mode](https://devblogs.microsoft.com/oldnewthing/20140722-00/?p=433)
    is a Microsoft-ism to denote when changes require approval before accepting
    merges. For DXC this will require approval from @microsoft/hlsl-release
