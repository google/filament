# ANGLE Wrangling

As an ANGLE Sheriff. Your job is to:

 1. Keep the [ANGLE Standalone][StandaloneCI] and [ANGLE/Chromium][ANGLEChromiumCI] waterfalls green.
 1. Ensure developers have reliable pre-commit testing via the
    [ANGLE Standalone][StandaloneTry] and [ANGLE/Chromium][ANGLEChromiumTry] try waterfalls.
 1. Control and monitor the [ANGLE auto-rollers](#the-auto-rollers).
 1. Act as a point of contact for the Chromium Sheriff and other teams monitoring ANGLE regressions.
 1. **Note: currently not working!** Monitor and respond to ANGLE's [Perf alerts][PerfAlertGroup].

[StandaloneCI]: https://ci.chromium.org/p/angle/g/ci/console
[ANGLEChromiumCI]: https://ci.chromium.org/p/chromium/g/chromium.angle/console
[StandaloneTry]: https://ci.chromium.org/ui/p/angle/g/try/builders
[ANGLEChromiumTry]: https://ci.chromium.org/p/chromium/g/tryserver.chromium.angle/builders
[PerfAlertGroup]: https://groups.google.com/u/0/a/chromium.org/g/angle-perf-alerts

If you're not an ANGLE team member, you can contact us on the public ANGLE project
[Google group](https://groups.google.com/forum/#!forum/angleproject).

**Note**: Please review and if needed update the [wrangler schedule].

**Note**: If you need to suppress test failures (e.g. to keep an auto-roller unblocked), see
[Handling Test Failures](../doc/TestingAndProcesses.md).

[wrangler schedule]: https://rotations.corp.google.com/rotation/5080504293392384

## Task: Monitor ANGLE CI and Try Testers

Your first job is to keep the ANGLE builders green and unblocked.

### Post-commit CI builders

There are two consoles for ANGLE continuous integration builders:

 * Standalone ANGLE: https://ci.chromium.org/p/angle/g/ci/console
 * Chromium + integrated ANGLE: https://ci.chromium.org/p/chromium/g/chromium.angle/console

We recommend you track ANGLE build failures is via [Sheriff-o-matic][ANGLESoM].
Bookmark the link and check it regularly during your shift. **Note**:
currently flaky failures show up as separate failure instances.

[ANGLESoM]: https://sheriff-o-matic.appspot.com/angle

We expect these waterfalls to be as "green" as possible. As a wrangler
please help clean out any failures by finding and reverting problematic CLs,
suppressing flaky tests that can't be fixed, or finding other solutions. We
aim to have zero failing builds, so follow the campsite rule and leave the
waterfall cleaner than when you started your shift.

When you encounter red builds or flakiness, [file an ANGLE bug](http://anglebug.com/new)
and set the label: `Hotlist-Wrangler` ([search for existing bugs][WranglerBugs]).

[WranglerBugs]: https://bugs.chromium.org/p/angleproject/issues/list?q=Hotlist%3DWrangler&can=2

See more detailed instructions on ANGLE testing by following [this link](README.md).

### Pre-commit try builders

In addition to the CI builders, we have a console for try jobs on the ANGLE CV (change verifier):

 * Standalone ANGLE: https://ci.chromium.org/ui/p/angle/g/try/builders
 * Chromium + integrated ANGLE: https://ci.chromium.org/p/chromium/g/tryserver.chromium.angle/builders

Failures are intended on this waterfall as developers test WIP changes.
You must act on any persistent flakiness or failure that causes developer drag
by filing bugs, reverting CLs, or taking other action as with the CI waterfall.

If you find a failure that is unrelated to ANGLE, [file a Chromium bug](http://crbug.com/new).
Set the bug label `Hotlist-PixelWrangler`. Ensure you cc the current ANGLE and Chrome GPU
wranglers, which you can find by consulting
[build.chromium.org](https://ci.chromium.org/p/chromium/g/main/console).
For more information see [Filing Chromium Bug Reports](#filing-chromium-bug-reports) below.

You can optionally follow [Chromium bugs in the `Internals>GPU>ANGLE` component][ChromiumANGLEBugs]
to be alerted to reports of ANGLE-related breakage in Chrome.

[ChromiumANGLEBugs]: https://bugs.chromium.org/p/chromium/issues/list?q=component%3AInternals%3EGPU%3EANGLE&can=2

**NOTE: When all builds seem to be purple or otherwise broken:**

This could be a major infrastructure outage. File a high-priority bug using
[g.co/bugatrooper](http://g.co/bugatrooper).

### Filing Chromium Bug Reports

The GPU Pixel Wrangler is responsible for *Chromium* bugs. Please file
Chromium issues with the Label `Hotlist-PixelWrangler` for bugs outside of
the ANGLE project.

*IMPORTANT* info to include in Chromium bug reports:

 * Links to all first failing builds (e.g. first windows failure, first mac failure, etc).
 * Related regression ranges. See below on how to determine the ANGLE regression range.
 * Relevant error messages.
 * Set the **Components** to one or more value, such as (start typing "Internals" and you'll see choices):
   * `Internals>GPU` for general GPU bugs
   * `Internals>GPU>Testing` for failures that look infrastructure-related
   * `Internals>GPU>ANGLE` for ANGLE-related Chromium bugs
   * `Internals>Skia` for Skia-specific bugs
 * Cc relevant sheriffs or blame suspects, as well as yourself or the current ANGLE Wrangler.
 * Set the `Hotlist-PixelWrangler` Label.

### How to determine the ANGLE regression range on Chromium bots:

 1. Open the first failing and last passing builds.
 1. For test failures: record `parent_got_angle_revision` in both builds.
 1. For compile failures record `got_angle_revision`.
 1. Create a regression link with this URL template:
    `https://chromium.googlesource.com/angle/angle.git/+log/<last passing revision>..<first failing revision>`

## <a name="the-auto-rollers"></a>Task: The Auto-Rollers

The [ANGLE into Chrome auto-roller](https://autoroll.skia.org/r/angle-chromium-autoroll) automatically updates
Chrome with the latest ANGLE changes.

The [ANGLE into Android auto-roller](https://autoroll.skia.org/r/angle-android-autoroll) updates Android with
the latest ANGLE changes. You must manually approve and land these rolls.  The
recommendation is to pre-approve the roll and set "**Autosubmit**".
 * The auto-roller abandons a presubmit-passed roll whenever a new ANGLE change
   comes.  During work hours, it's hard for Wrangler to approve and land in
   time.

We also use additional auto-rollers to roll third party libraries into ANGLE and Chromium:

 * [SwiftShader into ANGLE](https://autoroll.skia.org/r/swiftshader-angle-autoroll)
 * [vulkan-deps into ANGLE](https://autoroll.skia.org/r/vulkan-deps-angle-autoroll)
 * [VK-GL-CTS into ANGLE](https://autoroll.skia.org/r/vk-gl-cts-angle-autoroll)
 * [Chromium into ANGLE](https://autoroll.skia.org/r/chromium-angle-autoroll)
 * [SwiftShader into Chromium](https://autoroll.skia.org/r/swiftshader-chromium-autoroll)

**Roller health**: You will be cc'ed on all rolls. Please check failed rolls
  to verify there is no blocking breakage.

For all rollers, you can trigger manual rolls using the dashboards to land
high-priority changes. For example: Chromium-side test expectation updates or
suppressions. When a roll fails, stop the roller, determine if the root cause
is a problem with ANGLE or with the upstream repo, and file an issue with an
appropriate next step.

To monitor the rollers during your shift, you can:
  1. Open the [autoroll dashboard](https://autoroll.skia.org/) and put "angle" in `Filter`.  The dashboard provides the status of ANGLE related rollers.  Monitor their modes and numbers.
  1. Filter out the non-critical emails by a filter: “`from:(*-autoroll (Gerrit))`”.  This improves the signal to noise ratio of your inbox, so the important emails, ex) "`The roll is failing consistently. Time to investigate.`", can stand out.

The autoroller configurations live in the
[skia-autoroll-internal-config repository](https://skia.googlesource.com/skia-autoroll-internal-config.git/+/main/skia-public).
Feel free to maintain these configs yourself, or file a Skia [autoroll bug][SkiaAutorollBug]
for help as needed.

[SkiaAutorollBug]: https://bugs.chromium.org/p/skia/issues/entry?template=Autoroller+Bug

### Vulkan Dependencies auto-roller: Handling failures

**Vulkan-deps consists of several related Vulkan dependencies**: Vulkan-Tools,
Vulkan-Loader, Vulkan-ValidationLayers, Vulkan-Headers and other related
repos. One common source of breaks is a Vulkan Headers update, which can take
a while to be integrated into other repos like the Vulkan Validation Layers.
No action on your part is needed for header updates.

If a vulkan-deps AutoRoll CL triggers an failure in the `presubmit` bot, in
the "export targets" step, you can:

 1. Add missing headers to the upstream `BUILD.gn` if possible. See this [example CL][GNHeaderExample].
 1. Otherwise, add headers to `IGNORED_INCLUDES` in [`export_targets.py`][ExportTargetsPy]. See this
[example CL][ExportHeaderExample].

[GNHeaderExample]: https://github.com/KhronosGroup/Vulkan-Loader/pull/968
[ExportTargetsPy]: ../scripts/export_targets.py
[ExportHeaderExample]: https://chromium-review.googlesource.com/c/angle/angle/+/3399044

If the roll fails for a reason other than a header update or presubmit,
determine the correct upstream repo and file an issue upstream. For simple
compilation failures, we usually submit fixes ourselves. For more info on
vulkan-deps see the [README][VulkanDepsREADME].

[VulkanDepsREADME]: https://chromium.googlesource.com/vulkan-deps/+/refs/heads/main/README.md

### ANGLE into Chrome auto-roller: SwANGLE builders

The ANGLE into Chromium roller has two SwiftShader + ANGLE (SwANGLE) builders:
[linux-swangle-try-x64](https://luci-milo.appspot.com/p/chromium/builders/try/linux-swangle-try-x64)
and
[win-swangle-try-x86](https://luci-milo.appspot.com/p/chromium/builders/try/win-swangle-try-x86).
However, failures on these bots may be due to SwiftShader changes.

To handle failures on these bots:
1. If possible, suppress the failing tests in ANGLE, opening a bug to investigate later.
1. If you supsect an ANGLE CL caused a regression,
   consider whether reverting it or suppressing the failures is a better course of action.
1. If you suspect a SwiftShader CL, and the breakage is too severe to suppress,
   (a lot of tests fail in multiple suites),
   consider reverting the responsible SwiftShader roll into Chromium
   and open a SwiftShader [bug](http://go/swiftshaderbugs). SwiftShader rolls into Chromium
   should fail afterwards, but if the bad roll manages to reland,
   stop the [autoroller](https://autoroll.skia.org/r/swiftshader-chromium-autoroll) as well.

## Task: Monitor and respond to ANGLE's perf alerts

Any large regressions should be triaged with a new ANGLE bug linked to any suspected CLs that may
have caused performance to regress. If it's a known/expected regression, the bug can be closed as
such. The tests are very flaky right now, so a WontFix resolution is often appropriate.
