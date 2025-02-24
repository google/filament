<!-- Copyright 2015 The Chromium Authors. All rights reserved.
     Use of this source code is governed by a BSD-style license that can be
     found in the LICENSE file.
-->
# Code of Conduct

We follow the [Chromium code of conduct](
https://chromium.googlesource.com/chromium/src/+/main/CODE_OF_CONDUCT.md) in
our our repos and organizations, mailing lists, and other communications.

# Issues

Please use crbug.com issue tracker. Relevant components to file bugs to include
"Speed", "Test>Telemetry", "Speed>Dashboard", "Speed>Tracing", and "Speed>Bisection".

Do not use the github issue tracker.

# Workflow

*Note*: If you are a chromium project contributor and already have a `chromium/src`  checkout,
you probably want to use the clone of this repo under that checkout's `third_party`
directory instead of cloning and editing this repo directly.
See [Contributing from a Chromium checkout](#contributing-from-a-chromium-checkout)
for details.

Install [depot_tools](
https://www.chromium.org/developers/how-tos/install-depot-tools).

Then checkout the catapult repo.

`git clone https://chromium.googlesource.com/catapult`

You can then create a local branch, make and commit your change.

```
cd catapult
git checkout -t -b foo origin/main
... edit files ...
git commit -a -m "New files"
```

Once you're ready for a review do:

`git cl upload`

Once uploaded you can view the CL in Gerrit and **request a review** by
clicking the "Start Review" button, and adding a reviewer from the
[OWNERS](/OWNERS) file. You can also click the "CQ Dry Run" button to run all
the tests on your change.

If you get review feedback, edit and commit locally and then do another upload
with the new files. Before you commit you'll want to sync to the tip-of-tree.
You can either merge or rebase, it's up to you.

Then, submit your changes through the commit queue by checking the "Commit" box.

Once everything is landed, you can cleanup your branch.

```
git checkout main
git branch -D foo
```

# Troubleshooting

If you get errors running git cl:
```
Credentials for the following hosts are required:
  github-review.com
  github.com
```
Then you cloned the github url of this repository. That confuses `git cl` /o\.
To make things work, you'll need to re-clone from
https://chromium.googlesource.com/catapult for things to work.


# Becoming a committer

If you're new to the chromium-family of projects, you will also need to sign the
chrome contributors license agreement. You can sign the
[Contributor License Agreement](
https://cla.developers.google.com/about/google-individual?csw=1), which you can
do online.
It only takes a minute. If you are contributing on behalf of a corporation, you
must fill out the [Corporate Contributor License Agreement](
https://cla.developers.google.com/about/google-corporate?csw=1) and send it to
us as described on that page.

If you've never submitted code before, you must add your (or your
organization's) name and contact info to the Chromium AUTHORS file.

Next, ask an admin to add you (see
[adding committers](/docs/adding-committers.md))

# Contributing from a Chromium checkout

If you already have catapult checked out as part of a Chromium checkout and want
to edit it in place (instead of having a separate clone of the repository), you
will probably want to disconnect it from gclient at this point so that it
doesn't do strange things on updating. This is done by editing the .gclient file
for your Chromium checkout and adding the following lines:

```
'custom_deps': {
    'src/third_party/catapult': None,
},
```

In order to be able to land patches, you will most likely need to update the
`origin` remote in your catapult checkout to point directly to this
repository. You can do this by executing the following command inside the
catapult folder (third_party/catapult):

`git remote set-url origin https://chromium.googlesource.com/catapult`

# Code style

See the [style guide](/docs/style-guide.md).

# Individual project documentation

Look to individual project documentation for more info on getting started:
   * [perf dashboard](/dashboard/README.md)
   * [systrace](/systrace/README.md)
   * [telemetry](/telemetry/README.md)
   * [trace-viewer](/tracing/README.md)

# Tests

Check individual project documentation for instructions on how to run tests.
You can also check the current status of our tests on the
[waterfall](http://build.chromium.org/p/client.catapult/waterfall).
Use the "Submit to CQ" button in Gerrit to commit through the commit queue,
which automatically runs all tests. Run the tests before committing with the
"CQ dry run" button.

# Updating Chromium's about:tracing (rolling DEPS)

Chromium's DEPS file needs to be rolled to the catapult revision containing your
change in order for it to appear in Chrome's about:tracing or other
third_party/catapult files. Follow the [directions for rolling DEPS](/docs/rolling-deps.md)
to do this.

# Adding a new project

Please read the [directory structure guide](/docs/directory-structure.md)
to learn the conventions for new directories.
