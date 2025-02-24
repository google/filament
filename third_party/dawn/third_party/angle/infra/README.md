# ANGLE Testing Infrastructure

ANGLE runs hundreds of thousands of tests on every change before it lands in
the tree. We scale our pre-commit and post-commit testing to many machines
using [Chromium Swarming][Swarming]. Our testing setup heavily leverages
existing work in Chromium. We also run compile-only
[Standalone Testing][Standalone] that does not depend on a Chromium checkout.

Also see the documentation on [ANGLE Wrangling][Wrangling] for more info.

## Pre-Commit Testing

See the pre-commit try waterfall here:

[`https://ci.chromium.org/p/chromium/g/tryserver.chromium.angle/builders`](https://ci.chromium.org/p/chromium/g/tryserver.chromium.angle/builders)

We currently run pre-commit tests on:

 * Windows 32-bit AMD and Windows 64-bit Intel and NVIDIA GPUs
 * Linux 64-bit NVIDIA and Intel GPUs
 * Mac NVIDIA, Intel and AMD GPUs
 * Pixel 4 and Nexus 5X
 * Fuchsia testing in a VM

Looking at an example build shows how tests are split up between machines. See for example:

[`https://ci.chromium.org/ui/p/angle/builders/ci/mac-rel/8123/overview`](https://ci.chromium.org/ui/p/angle/builders/ci/mac-rel/8123/overview)

This build ran 68 test steps across 3 GPU families. In some cases (e.g.
`angle_deqp_gles3_metal_tests`) the test is split up between multiple machines to
run faster (in this case 2 different machines at once). This build took 23
minutes to complete 72 minutes of real automated testing.

For more details on running and working with our test sets see the docs in [Contributing Code][Contrib].

[Swarming]: https://chromium-swarm.appspot.com/
[Standalone]: #ANGLE-Standalone-Testing
[Contrib]: ../doc/ContributingCode.md#Testing
[Wrangling]: ANGLEWrangling.md

## ANGLE Standalone Testing

In addition to the ANGLE try bots using Chrome, and the GPU.FYI bots, ANGLE
has standalone testing on the Chrome infrastructure. Currently these tests are
compile-only. This page is for maintaining the configurations that don't use
Chromium. Also see the main instructions for [ANGLE Wrangling](ANGLEWrangling.md).

It's the ANGLE team's responsibility for maintaining this testing
infrastructure. The bot configurations live in four different repos and six
branches.

## Info Consoles

Continuous builders for every ANGLE revision are found on the CI console:

[https://ci.chromium.org/p/angle/g/ci/console](https://ci.chromium.org/p/angle/g/ci/console)

Try jobs from pre-commit builds are found on the builders console:

[https://ci.chromium.org/p/angle/g/try/builders](https://ci.chromium.org/p/angle/g/try/builders)

## How to add a new build configuration

 1. [`bugs.chromium.org/p/chromium/issues/entry?template=Build+Infrastructure`](http://bugs.chromium.org/p/chromium/issues/entry?template=Build+Infrastructure):

    * If adding a Mac bot, request new slaves by filing an infra issue.

 1. [`chrome-internal.googlesource.com/infradata/config`](http://chrome-internal.googlesource.com/infradata/config):

    * Update **`configs/chromium-swarm/starlark/bots/angle.star`** with either Mac slaves requested in the previous step or increase the amount of Windows or Linux GCEs.

 1. [`chromium.googlesource.com/chromium/tools/build`](https://chromium.googlesource.com/chromium/tools/build):

    * Update **`scripts/slave/recipes/angle.py`** with new the config.
    * The recipe code requires 100% code coverage through mock bots, so add mock bot config to GenTests.
    * Maybe run `./scripts/slave/recipes.py test train` to update checked-in golden files. This might no longer be necessary.

 1. [`chromium.googlesource.com/angle/angle`](http://chromium.googlesource.com/angle/angle):

    * Update **`infra/config/global/cr-buildbucket.cfg`** to add the new builder (to ci and try), and set the new config option.
    * Update **`infra/config/global/luci-milo.cfg`** to make the builders show up on the ci and try waterfalls.
    * Update **`infra/config/global/luci-scheduler.cfg`** to make the builders trigger on new commits or try jobs respectively.
    * Update **`infra/config/global/commit-queue.cfg`** to add the builder to the default CQ jobs (if desired).

## Other Configuration

There are other places where configuration for ANGLE infra lives. These are files that we shouldn't need to modify very often:

 1. [`chrome-internal.googlesource.com/infradata/config`](http://chrome-internal.googlesource.com/infradata/config):

    * **`configs/luci-token-server/service_accounts.cfg`** (service account names)
    * **`configs/chromium-swarm/pools.cfg`** (swarming pools)

 1. [`chromium.googlesource.com/chromium/tools/depot_tools`](http://chromium.googlesource.com/chromium/tools/depot_tools):

    * **`recipes/recipe_modules/gclient/config.py`** (gclient config)
