# Choosing an ANGLE branch for your project

ANGLE is under continuous development, and does not create release tarballs or
tag specific revisions as releases, which may make the process of choosing a
point in ANGLE's history as the dependency for your project less than obvious.
This document illustrates how to choose a branch of ANGLE which can be expected
to be updated with critical fixes, should they be needed.

## ANGLE automatic branching

Branches are created automatically in ANGLE to correspond to branches created in
Chromium. These branches are named `chromium/####`, where the number is the
matching Chromium branch. These branches will be created from the revision which
that Chromium branch points to as its dependency. So, for example, the
`chromium/2013` branch point is at r28bcf4ff, because [Chromium's dependency
file for the 2013 branch]
(http://src.chromium.org/viewvc/chrome/branches/2013/src/DEPS?revision=272741)
uses this ANGLE revision.

It isn't necessary to be familiar with how Chromium's dependency management
works to choose an appropriate ANGLE branch for your project. You will, however,
likely want to make sure that you choose a branch that is used for a relatively
stable Chromium release channel build, as those branches will be deployed with
Chromium, and receive updates if bugs are found and fixed during release
lifetime, while the more volatile channels will turn over quickly, and their
branches will be short-lived.

**We recommend choosing a branch corresponding to a Beta or Stable Chromium
release** if you are pursuing periodic, not continuous, integration of ANGLE.

## Matching a Chromium release to an ANGLE branch

In order to determine which branches are used by Chromium releases, please use
[the OmahaProxy tool](http://omahaproxy.appspot.com/), which lists build
information about current Chromium releases. Find the entry for a suitable
release channel (for now, we recommend one of the Windows desktop releases), and
note the branch listed in the `true_branch` column. This identifies the ANGLE
branch used by that Chromium release.

## Updates to release branches

If bugs (stability, security, performance, or otherwise) are discovered after a
branch has been created, and that branch is used by a Chromium release, the
fixes for those bugs will be applied to the ANGLE branches for uptake by
Chromium and any other projects using that branch. You should need only to
perform a `git pull` to check for and apply any such changes.

## Cherry-picking a change to a release branch

Occasionally a bug fix must be merged back to an earlier Chromium
release branch. To do this, first look up the branch number in
[OmahaProxy](https://omahaproxy.appspot.com/). For example, M55
corresponds to branch number 2883.

In the simple case where there are no conflicts, the merge can be done
entirely in the Gerrit UI. Click the "Cherry pick" button and enter
`chromium/[branch_number]` as the branch to merge to.

If there are conflicts, however, follow these steps:

1. `git checkout chromium/[branch_number]`
2. `git cherry-pick [commit_hash]`
3. Fix any merge conflicts.
4. `git cl upload`

Have the cherry-pick reviewed, and then land it. It's also OK to skip
the review and land it yourself with TBR= in the issue description, if
you have that ability.

There is one final step to pick up the ANGLE change into a Chromium Beta or
Stable branch. A Skia auto-roller updates the DEPS file in the corresponding
Chromium branch automatically once the ANGLE change is merged into an ANGLE
branch. To make sure that your change has made it into Chromium you can check
these auto-rollers:

1. https://autoroll.skia.org/r/angle-chromium-beta-autoroll
1. https://autoroll.skia.org/r/angle-chromium-stable-autoroll
