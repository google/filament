# Dawn's Continuous Testing Infrastructure

Dawn uses Chromium's continuous integration (CI) infrastructure to continually run tests on changes to Dawn and provide a way for developers to run tests against their changes before submitting. CI bots continually build and run tests for every new change, and Try bots build and run developers' pending changes before submission. Dawn uses two different build recipes. There is a Dawn build recipe which checks out Dawn standalone, compiles, and runs the `dawn_unittests`. And, there is the Chromium build recipe which checks out Dawn inside a Chromium checkout. Inside a Chromium checkout, there is more infrastructure available for triggering `dawn_end2end_tests` that run on real GPU hardware, and we are able to run Chromium integration tests as well as tests for WebGPU.

 - [Dawn CI Builders](https://ci.chromium.org/p/dawn/g/ci/builders)
 - [Dawn Try Builders](https://ci.chromium.org/p/dawn/g/try/builders)
 - [chromium.dawn Waterfall](https://ci.chromium.org/p/chromium/g/chromium.dawn/console)

For additional information on GPU testing in Chromium, please see [[chromium/src]//docs/gpu/gpu_testing_bot_details.md](https://chromium.googlesource.com/chromium/src.git/+/main/docs/gpu/gpu_testing_bot_details.md).

## Dawn CI/Try Builders
Dawn builders are specified in [[dawn]//infra/config/global/cr-buildbucket.cfg](../../infra/config/global/generated/cr-buildbucket.cfg). This file contains a few mixins such as `clang`, `no_clang`, `x64`, `x86`, `debug`, `release` which are used to specify the bot dimensions and build properties (builder_mixins.recipe.properties). At the time of writing, we have the following builders:
  - [dawn/try/presubmit](https://ci.chromium.org/p/dawn/builders/try/presubmit)
  - [dawn/try/linux-clang-dbg-x64](https://ci.chromium.org/p/dawn/builders/try/linux-clang-dbg-x64)
  - [dawn/try/linux-clang-dbg-x86](https://ci.chromium.org/p/dawn/builders/try/linux-clang-dbg-x86)
  - [dawn/try/linux-clang-rel-x64](https://ci.chromium.org/p/dawn/builders/try/linux-clang-rel-x64)
  - [dawn/try/mac-dbg](https://ci.chromium.org/p/dawn/builders/try/mac-dbg)
  - [dawn/try/mac-rel](https://ci.chromium.org/p/dawn/builders/try/mac-rel)
  - [dawn/try/win-clang-dbg-x86](https://ci.chromium.org/p/dawn/builders/try/win-clang-dbg-x86)
  - [dawn/try/win-clang-rel-x64](https://ci.chromium.org/p/dawn/builders/try/win-clang-rel-x64)
  - [dawn/try/win-msvc-dbg-x86](https://ci.chromium.org/p/dawn/builders/try/win-msvc-dbg-x86)
  - [dawn/try/win-msvc-rel-x64](https://ci.chromium.org/p/dawn/builders/try/win-msvc-rel-x64)

There are additional `chromium/try` builders, but those are described later in this document.

These bots are defined in both buckets luci.dawn.ci and luci.dawn.try, though their ACL permissions differ. luci.dawn.ci bots will be scheduled regularly based on [[dawn]//infra/config/global/luci-scheduler.cfg](../../infra/config/global/generated/luci-scheduler.cfg). luci.dawn.try bots will be triggered on the CQ based on [[dawn]//infra/config/global/commit-queue.cfg](../../infra/config/global/generated/commit-queue.cfg).

One particular note is `buckets.swarming.builder_defaults.recipe.name: "dawn"` which specifies these use the [`dawn.py`](https://source.chromium.org/search/?q=file:recipes/dawn.py) build recipe.

Build status for both CI and Try builders can be seen at this [console](https://ci.chromium.org/p/dawn) which is generated from [[dawn]//infra/config/global/luci-milo.cfg](../../infra/config/global/generated/luci-milo.cfg).

## Dawn Build Recipe
The [`dawn.py`](https://cs.chromium.org/search/?q=file:recipes/dawn.py) build recipe is simple and intended only for testing compilation and unit tests. It does the following:
  1. Checks out Dawn standalone and dependencies
  2. Builds based on the `builder_mixins.recipe.properties` coming from the builder config in [[dawn]//infra/config/global/cr-buildbucket.cfg](../../infra/config/global/generated/cr-buildbucket.cfg).
  3. Runs the `dawn_unittests` on that same bot.

## Dawn Chromium-Based CI Waterfall Bots
The [`chromium.dawn`](https://ci.chromium.org/p/chromium/g/chromium.dawn/console) waterfall consists of the bots specified in the `chromium.dawn` section of [[chromium/src]//testing/buildbot/waterfalls.pyl](https://source.chromium.org/search/?q=file:waterfalls.pyl%20chromium.dawn). Bots named "Builder" are responsible for building top-of-tree Dawn, whereas bots named "DEPS Builder" are responsible for building Chromium's DEPS version of Dawn.

The other bots, such as "Dawn Linux x64 DEPS Release (Intel HD 630)" receive the build products from the Builders and are responsible for running tests. The Tester configuration may specify `mixins` from [[chromium/src]//testing/buildbot/mixins.pyl](https://source.chromium.org/search/?q=file:buildbot/mixins.pyl) which help specify bot test dimensions like OS version and GPU vendor. The Tester configuration also specifies `test_suites` from [[chromium/src]//testing/buildbot/test_suites.pyl](https://source.chromium.org/search/?q=file:buildbot/test_suites.pyl%20dawn_end2end_tests) which declare the tests are arguments passed to tests that should be run on the bot.

The Builder and Tester bots are additionally configured at [[chromium/tools/build]//scripts/slave/recipe_modules/chromium_tests/chromium_dawn.py](https://source.chromium.org/search?q=file:chromium_dawn.py) which defines the bot specs for the builders and testers. Some things to note:
 - The Tester bots set `parent_buildername` to be their respective Builder bot.
 - The non DEPS bots use the `dawn_top_of_tree` config.
 - The bots apply the `mb` config which references [[chromium]//tools/mb/mb_config.pyl](https://source.chromium.org/search?q=file:mb_config.pyl%20%22Dawn%20Linux%20x64%20Builder%22) and [[chromium]//tools/mb/mb_config_buckets.pyl](https://source.chromium.org/search?q=file:mb_config_buckets.pyl%20%22Dawn%20Linux%20x64%20Builder%22). Various mixins there specify build dimensions like debug, release, gn args, x86, x64, etc.

Finally, builds on these waterfall bots are automatically scheduled based on the configuration in [[chromium/src]//infra/config/buckets/ci.star](https://source.chromium.org/search?q=file:ci.star%20%22Dawn%20Linux%20x64%20Builder%22). Note that the Tester bots are `triggered_by` the Builder bots.

## Dawn Chromium-Based Tryjobs
[[dawn]//infra/config/global/commit-queue.cfg](../../infra/config/global/generated/commit-queue.cfg) declares additional tryjob builders which are defined in the Chromium workspace. The reason for this separation is that jobs sent to these bots rely on the Chromium infrastructure for doing builds and triggering jobs on bots with GPU hardware in swarming.

At the time of writing, the bots for Dawn CLs are:
  - [chromium/try/linux-dawn-rel](https://ci.chromium.org/p/chromium/builders/try/linux-dawn-rel)
  - [chromium/try/mac-dawn-rel](https://ci.chromium.org/p/chromium/builders/try/mac-dawn-rel)
  - [chromium/try/win-dawn-rel](https://ci.chromium.org/p/chromium/builders/try/win-dawn-rel)

And for Chromium CLs:
  - [chromium/try/dawn-linux-x64-deps-rel](https://ci.chromium.org/p/chromium/builders/try/dawn-linux-x64-deps-rel)
  - [chromium/try/dawn-mac-x64-deps-rel](https://ci.chromium.org/p/chromium/builders/try/dawn-mac-x64-deps-rel)
  - [chromium/try/dawn-win10-x86-deps-rel](https://ci.chromium.org/p/chromium/builders/try/dawn-win10-x86-deps-rel)
  - [chromium/try/dawn-win10-x64-deps-rel](https://ci.chromium.org/p/chromium/builders/try/dawn-win10-x64-deps-rel)

 The configuration for these bots is generated from [[chromium]//infra/config/buckets/try.star](https://source.chromium.org/search/?q=file:try.star%20linux-dawn-rel) which uses the [`chromium_dawn_builder`](https://source.chromium.org/search/?q=%22def%20chromium_dawn_builder%22) function which sets the `mastername` to `tryserver.chromium.dawn`.

[[chromium/tools/build]//scripts/slave/recipe_modules/chromium_tests/trybots.py](https://source.chromium.org/search/?q=file:trybots.py%20tryserver.chromium.dawn) specifies `tryserver.chromium.dawn` bots as mirroring bots from the `chromium.dawn` waterfall. Example:
```
'dawn-linux-x64-deps-rel': {
    'bot_ids': [
        {
            'mastername': 'chromium.dawn',
            'buildername': 'Dawn Linux x64 DEPS Builder',
            'tester': 'Dawn Linux x64 DEPS Release (Intel HD 630)',
        },
        {
            'mastername': 'chromium.dawn',
            'buildername': 'Dawn Linux x64 DEPS Builder',
            'tester': 'Dawn Linux x64 DEPS Release (NVIDIA)',
        },
    ],
},
```

Using the [[chromium/tools/build]//scripts/slave/recipes/chromium_trybot.py](https://source.chromium.org/search/?q=file:chromium_trybot.py) recipe, these trybots will cherry-pick a CL and run the same tests as the CI waterfall bots. The trybots also pick up some build mixins from [[chromium]//tools/mb/mb_config.pyl](https://source.chromium.org/search?q=file:mb_config.pyl%20dawn-linux-x64-deps-rel).

## Bot Allocation

Bots are physically allocated based on the configuration in [[chromium/infradata/config]//configs/chromium-swarm/starlark/bots/dawn.star](https://chrome-internal.googlesource.com/infradata/config/+/refs/heads/main/configs/chromium-swarm/starlark/bots/dawn.star) (Google only).

`dawn/try` bots are using builderless configurations which means they use builderless GCEs shared with Chromium bots and don't need explicit allocation.

`chromium/try` bots are still explicitly allocated with a number of GCE instances and lifetime of the build cache. All of the GCE bots should eventually be migrated to builderless (crbug.com/dawn/328). Mac bots such as `dawn-mac-x64-deps-rel`, `mac-dawn-rel`, `Dawn Mac x64 Builder`, and `Dawn Mac x64 DEPS Builder` point to specific ranges of machines that have been reserved by the infrastructure team.
