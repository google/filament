# depot_tools

Tools for working with Chromium development. It requires python 3.8.


## Tools

The most important tools are:

- `fetch`: A `gclient` wrapper to checkout a project. Use `fetch --help` for
  more details.
- `gclient`: A meta-checkout tool. Think
  [repo](https://source.android.com/source/using-repo.html) or [git
  submodules](https://git-scm.com/docs/git-submodule), except that it support
  OS-specific rules, e.g. do not checkout Windows only dependencies when
  checking out for Android. Use `gclient help` for more details and
  [README.gclient.md](README.gclient.md).
- `git cl`: A code review tool to interact with Rietveld or Gerrit. Use `git cl
  help` for more details and [README.git-cl.md](README.git-cl.md).
- `roll-dep`: A gclient dependency management tool to submit a _dep roll_,
  updating a dependency to a newer revision.

There are a lot of git utilities included.


## Updating

`depot_tools` updates itself automatically when running `gclient` tool. To
disable auto update, set the environment variable `DEPOT_TOOLS_UPDATE=0` or
run `./update_depot_tools_toggle.py --disable`.

To update package manually, run `update_depot_tools.bat` on Windows,
or `./update_depot_tools` on Linux or Mac.

On Windows only, running `gclient` will install `git` and `python`.


## Contributing

To contribute change for review:

    git new-branch <somename>
    # Hack
    git add .
    git commit -a -m "Fixes goat teleporting"
    # find reviewers
    git cl owners
    git log -- <yourfiles>

    # Request a review.
    git cl upload -r reviewer1@chromium.org,reviewer2@chromium.org --send-mail

    # Edit change description if needed.
    git cl desc

    # If change is approved, flag it to be committed.
    git cl set-commit

    # If change needs more work.
    git rebase-update
    ...
    git cl upload -t "Fixes goat teleporter destination to be Australia"

See also [open bugs](https://issues.chromium.org/issues?q=status:open%20componentid:1456102),
[open reviews](https://chromium-review.googlesource.com/q/status:open+project:chromium%252Ftools%252Fdepot_tools),
[forum](https://groups.google.com/a/chromium.org/forum/#!forum/infra-dev) or
[report problems](https://issues.chromium.org/issues/new?component=1456102).

### cpplint.py

Until 2018, our `cpplint.py` was a copy of the upstream version at
https://github.com/google/styleguide/tree/gh-pages/cpplint. Unfortunately, that
repository is not maintained any more.
If you want to update `cpplint.py` in `depot_tools`, just upload a patch to do
so. We will figure out a long-term strategy via issue https://crbug.com/916550.

Note that the `cpplint.py` here is also used by the [Tricium
analyzer](https://chromium.googlesource.com/infra/infra/+/HEAD/go/src/infra/tricium/functions/cpplint),
so if the cpplint.py here changes, we should also update the copy used there.
