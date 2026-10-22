---
name: cherry-pick
description: Cherry-pick a commit onto another branch and open the PR. Use when the user asks to cherry-pick or backport a commit or PR to a release branch.
---

# Cherry-picking commits

Cherry-pick the commit with `git cherry-pick -x <sha>` so the source SHA is
recorded in git's standard format. Keep the commit message as git generates it
by default. Prefix the PR title with `[Cherry-Pick]`.