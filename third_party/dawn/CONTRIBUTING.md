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
You can also open GitHub pull requests, but they will be reviewed on Gerrit.

Any submissions to the [Tint](src/tint) folders should follow the
[Tint style guide](docs/tint/style_guide.md).

### Discuss the change if needed

Some changes are inherently risky, because they have long-term or architectural
consequences, contain a lot of unknowns or other reasons. When that's the case
it is better to discuss it on the [Dawn Matrix Channel](https://matrix.to/#/#webgpu-dawn:matrix.org)
or the [Dawn mailing-list](https://groups.google.com/g/dawn-graphics).

### Pushing changes to code review

While GitHub pull requests are possible as mentioned above, iterating on code
review feedback is easier if you use Gerrit directly.

First, follow the one-time setup section below.

To upload changes to Gerrit, use `git cl upload`. This does the following:

- Runs "presubmit" checks with `git cl presubmit`.
- Pushes your changes to the Gerrit code review server. By default, it creates
  one Gerrit CL (ChangeList) for each commit, so make sure your commit history
  is split up the way you want the reviewers to see it (both on the first upload
  and on any subsequent revisions).

Once your change is uploaded successfully, in the terminal you will see a URL
where code review for this CL will happen.
CLs start in the "Work In Progress" state. To start the code review proper,
click on "Start Review", add reviewers and click "Send and start review". The
reviewers will review or triage the CL. If you are unsure which reviewers to
use, click the "Suggest owners" button which will help you pick reviewers from
the most relevant OWNERS files. You can also choose owners from the
[Dawn OWNERS file](src/dawn/OWNERS) or [Tint OWNERS file](src/tint/OWNERS).

### Tracking issues

We usually like to have commits associated with issues in the
[Dawn](https://crbug.com/dawn) component
([new bug link](https://crbug.com/dawn/new)) or
[Tint](https://crbug.com/tint) component
([new bug link](https://crbug.com/tint/new)) so that
commits for the issue can all be found on the same page. This is done
by adding a `Bug: <issue number>` tag at the end of the commit message (it must
be in the last "paragraph"). The bug number may refer to any Chromium bug ID
(Dawn, Tint, Blink, etc.)

Some small fixes (like typo fixes, or some one-off maintenance) don't need a
tracking issue. When that's the case, it's good practice to call it out by
adding `Bug: None` so your reviewers know it's not part of a larger change.

It is possible to make issues fixed automatically when the CL is merged by
adding a `Fixed: <issue number>` tag (instead of `Bug:`) in the commit message.

### Iterating on code review

The project follows the general
[Google code review guidelines](https://google.github.io/eng-practices/review/).
Most changes need reviews from two committers. Reviewers will set the
"Code Review" CR+1 or CR+2 label once the change looks good to them (although
it could still have comments that need to be addressed first). When addressing
comments, please mark them as "Done" if you just address them, or start a
discussion until they are resolved.

Once you are granted rights (you can ask on your first contribution), you can
add the "Commit Queue" CQ+1 label to "dry run" the automated tests. Once the
CL has CR+2 you can then add the CQ+2 label to run the automated tests *and*
submit the commit if they pass.

The "Auto-Submit" AS+1 label is recommended as a way to tell reviewers that
you're ready for your CL to land once they approve it. (This will make Gerrit
automatically set the CQ+2 label when a reviewer adds CR+2.)

## One-time Setup

### Gerrit's .gitcookies

(If you work at Google, skip this and use `gcert` to log in.)

To push commits to Gerrit your `git` command needs to be authenticated. This is
done with `.gitcookies` that will make `git` send authentication information
when connecting to the remote. To get the `.gitcookies`, log-in to
[Dawn's Gerrit](https://dawn-review.googlesource.com) and browse to the
[new-password](https://dawn.googlesource.com/new-password) page that will give
you shell/cmd commands to run to update `.gitcookie`.

### Uploading

The project is setup to use Gerrit in a fashion similar to the ANGLE project
(with `git cl upload` defaulting to `--no-squash`).
If you're used to a more Chromium-style workflow (`--squash`), see the
"Squash Workflow" section below.

Note if you want to use a consistent style across projects, you can globally
override the project defaults using:

```sh
git config --global --bool gerrit.override-squash-uploads false # Dawn style
# or
git config --global --bool gerrit.override-squash-uploads true  # Chromium style
```

### Uploading: No-Squash Workflow (Dawn/ANGLE-style, one commit = one CL)

Gerrit works a bit differently than Github (if that's what you're used to):
there are no forks. Instead everyone works on the same repository. Gerrit has
magic branches for various purpose:

- `refs/for/<branch>` (most commonly `refs/for/main`) is a branch that anyone
  can push to that will create or update code reviews (called CLs for ChangeList)
  for the commits pushed.
- `refs/changes/00/<change number>/<patchset>` is a branch that corresponds to
  the commits that were pushed for codereview for "change number" at a certain
  "patchset" (a new patchset is created each time you push to a CL). You can
  find this branch name under the "Download" button on Gerrit.

#### Set up the commit-msg hook

Gerrit associates commits to CLs based on a `Change-Id:` tag in the commit
message. Each push with commits with a `Change-Id:` will update the
corresponding CL.

To add the `commit-msg` hook that will automatically add a `Change-Id:` to your
commit messages, run the following commands:

```sh
f="$(git rev-parse --git-dir)/hooks/commit-msg"
mkdir -p "$(dirname "$f")"
curl -Lo "$f" https://gerrit-review.googlesource.com/tools/hooks/commit-msg
chmod +x "$f"
```

Gerrit helpfully reminds you of that command if you forgot to set up the hook
before pushing commits.

#### Upload changes

Then push your commits using `git cl upload` as described above
(or approximately equivalently,
`git cl presubmit && git push origin HEAD:refs/for/main`).
Each commit becomes one CL, so this is the best way to upload stacks of changes.

When code reviewer asks for changes, you can amend the commit(s) any way
you want (e.g. `git commit --amend` for a single commit or
`git commit --fixup=HASH` and `git rebase -i --autosquash main` for multiple).
Then, run the same `git cl upload` command.

### Uploading: Squash Workflow (Chromium-style, one branch = one CL)

In order to get a more Chromium style workflow (`git cl upload --squash`), there
are couple changes needed.

1.  Verify there is NOT a `.git/hooks/commit-msg` hook set up. (If you have one,
    just moving it to a `commit-msg.bak` will suffice.)
1.  Add `override-squash-uploads = true` to the `gerrit` section of your
    `.git/config` file, if you haven't done so globally already:

    ```sh
    git config --bool gerrit.override-squash-uploads true
    ```

With those changes, a `Commit-Id` should not be automatically appended to your
CLs and `git cl upload` (not `git push`) must be used to push changes to Gerrit.
During code review you can commit to your branch as usual, no need to amend.

This will also allow `git cl status` to work as expected without having to
specifically set the issue number for the branch.
