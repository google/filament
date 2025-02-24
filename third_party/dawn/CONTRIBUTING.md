# How to contribute

First off, we'd love to get your contributions.

Everything helps other folks using Dawn, Tint and WebGPU: from small fixes and
documentation improvements to larger features and optimizations. Please read on
to learn about the contribution process.

## Contributor License Agreement

Contributions to this project must be accompanied by a Contributor License
Agreement. You (or your employer) retain the copyright to your contribution;
this simply gives us permission to use and redistribute your contributions as
part of the project. Head over to <https://cla.developers.google.com/> to see
your current agreements on file or to sign a new one.

You generally only need to submit a CLA once, so if you've already submitted one
(even if it was for a different project), you probably don't need to do it
again.

## Community Guidelines

This project follows
[Google's Open Source Community Guidelines](https://opensource.google.com/conduct/).

## Code reviews

All submissions, including submissions by project members, require review. We
use [Dawn's Gerrit](https://dawn-review.googlesource.com/) for this purpose.

Any submissions to the [Tint](src/tint) folders should follow the
[Tint style guide](docs/tint/style_guide.md).


### Discuss the change if needed

Some changes are inherently risky, because they have long-term or architectural
consequences, contain a lot of unknowns or other reasons. When that's the case
it is better to discuss it on the [Dawn Matrix Channel](https://matrix.to/#/#webgpu-dawn:matrix.org)
or the [Dawn mailing-list](https://groups.google.com/g/dawn-graphics).

### Pushing changes to code review

Before pushing changes to code review, it is better to run `git cl presubmit`
that will check the formatting of files and other small things.

Pushing commits is done with `git push origin HEAD:refs/for/main`. Which means
push to `origin` (i.e. Gerrit) the currently checkout out commit to the
`refs/for/main` magic branch that creates or updates CLs.

In the terminal you will see a URL where code review for this CL will happen.
CLs start in the "Work In Progress" state. To start the code review proper,
click on "Start Review", add reviewers and click "Send and start review". If
you are unsure which reviewers to use, pick one of the reviewers in the
[Dawn OWNERS file](src/dawn/OWNERS) or [Tint OWNERS file](src/tint/OWNERS)
who will review or triage the CL.

When code review asks for changes in the commits, you can amend them any way
you want (small fixup commit and `git rebase -i` are crowd favorites) and run
the same `git push origin HEAD:refs/for/main` command.

### Tracking issues

We usually like to have commits associated with issues in either
[Dawn's issue tracker](https://bugs.chromium.org/p/dawn/issues/list) or
[Tint's issue tracker](https://bugs.chromium.org/p/tint/issues/list) so that
commits for the issue can all be found on the same page. This is done
by adding a `Bug: dawn:<issue number>` or `Bug: tint:<issue number>` tag at the
end of the commit message. It is also possible to reference Chromium issues with
`Bug: chromium:<issue number>`.

Some small fixes (like typo fixes, or some one-off maintenance) don't need a
tracking issue. When that's the case, it's good practice to call it out by
adding a `Bug: None` tag.

It is possible to make issues fixed automatically when the CL is merged by
adding a `Fixed: <project>:<issue number>` tag in the commit message.

### Iterating on code review

The project follows the general
[Google code review guidelines](https://google.github.io/eng-practices/review/).
Most changes need reviews from two committers. Reviewers will set the
"Code Review" CR+1 or CR+2 label once the change looks good to them (although
it could still have comments that need to be addressed first). When addressing
comments, please mark them as "Done" if you just address them, or start a
discussion until they are resolved.

Once you are granted rights (you can ask on your first contribution), you can
add the "Commit Queue" CQ+1 label to run the automated tests. Once the
CL has CR+2 you can then add the CQ+2 label to run the automated tests and
submit the commit if they pass.

The "Auto Submit" AS+1 label can be used to make Gerrit automatically set the
CQ+2 label once the CR+2 label is added.

## One-time Setup

The project is setup to use Gerrit in a fashion similar to the Angle project.
If you're used to a more Chromium based control flow, see the
[Alternate setup](#alternate-setup) section below.

### Gerrit setup

Gerrit works a bit differently than Github (if that's what you're used to):
there are no forks. Instead everyone works on the same repository. Gerrit has
magic branches for various purpose:

 - `refs/for/<branch>` (most commonly `refs/for/main`) is a branch that anyone
can push to that will create or update code reviews (called CLs for ChangeList)
for the commits pushed.
 - `refs/changes/00/<change number>/<patchset>` is a branch that corresponds to
the commits that were pushed for codereview for "change number" at a certain
"patchset" (a new patchset is created each time you push to a CL).

To create a Gerrit change for review, type:

```bash
git push origin HEAD:refs/for/main
```

#### Gerrit's .gitcookies

To push commits to Gerrit your `git` command needs to be authenticated. This is
done with `.gitcookies` that will make `git` send authentication information
when connecting to the remote. To get the `.gitcookies`, log-in to
[Dawn's Gerrit](https://dawn-review.googlesource.com) and browse to the
[new-password](https://dawn.googlesource.com/new-password) page that will give
you shell/cmd commands to run to update `.gitcookie`.

#### Set up the commit-msg hook

Gerrit associates commits to CLs based on a `Change-Id:` tag in the commit
message. Each push with commits with a `Change-Id:` will update the
corresponding CL.

To add the `commit-msg` hook that will automatically add a `Change-Id:` to your
commit messages, run the following command:

```
f=`git rev-parse --git-dir`/hooks/commit-msg ; mkdir -p $(dirname $f) ; curl -Lo $f https://gerrit-review.googlesource.com/tools/hooks/commit-msg ; chmod +x $f
```

Gerrit helpfully reminds you of that command if you forgot to set up the hook
before pushing commits.

### Alternate setup
In order to get a more Chromium style workflow there are couple changes need.

1. Verify there is `.git/hooks/commit-msg` hook setup. (Just moving it to a
   `commit-msg.bak` will suffice)
2. Add `override-squash-uploads = True` to the `gerrit` section of your
   `.git/config` file

With those changes, a `Commit-Id` should not be auto-matically appended to your
CLs and `git cl upload` needs to be used to push changes to Gerrit. During
code review you can commit to your branch as usual, no need to amend.

This will also allow `git cl status` to work as expected without having to
specifically set the issue number for the branch.
