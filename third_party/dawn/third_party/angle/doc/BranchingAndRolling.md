# How to Branch and Roll Chromium's ANGLE Dependency

ANGLE provides an implementation of OpenGL ES on top of other APIs (e.g. DirectX11, Vulkan). ANGLE
uses (i.e. depends upon) other third-party software that comes from different repositories. ANGLE specifies
its dependencies on a specific version of each of these other repositories in the [ANGLE DEPS file](https://chromium.googlesource.com/angle/angle/+/main/DEPS).

Chromium relies upon ANGLE for hardware accelerated rendering and WebGL support. Chromium similarly
specifies its dependency on a specific version of ANGLE in the [Chromium repository's src/DEPS file](http://src.chromium.org/viewvc/chrome/trunk/src/DEPS).

This document describes how to update those dependencies, and, if necessary, create an ANGLE branch
to correspond to a branched release of Chrome.

ANGLE's commit queue also runs browser-level tests which are hosted in
the Chromium repository. To reduce the chance of a Chromium-side
change breaking ANGLE's CQ, the version of Chromium against which
ANGLE changes is also snapshotted, and rolled forward into ANGLE with
appropriate testing.

## Autorollers

At present, autorollers manage both the ANGLE roll into Chromium, and
the Chromium roll into ANGLE. There is also an autoroller for ANGLE into [Android AOSP](https://android.googlesource.com/platform/external/angle/).
All of the ANGLE-related autorollers are documented in the [ANGLE Wrangling documentation](../infra/ANGLEWrangling.md#the-auto-rollers).

## Manually rolling DEPS

As mentioned above, dependencies are encoded in `DEPS` files. The process to update a given
dependency is as follows:

 * Find the appropriate line in the relevant `DEPS` file that defines the dependency
 * Change the [git SHA-1 revision number](http://git-scm.com/book/ch6-1.html) to be that of the commit
on which to depend upon (Note: use the full SHA-1, not a
shortened version)
 * You can find the SHA-1 for a particular commit with `git log` on the appropriate branch of the
repository, or via a public repository viewer
 * If using the [ANGLE public repository viewer](https://chromium.googlesource.com/angle/angle), you will need to select the branch whose log you
wish to view from the list on the left-hand side, and then click on the "tree" link at the top of
the resulting page. Alternatively, you can navigate to
`https://chromium.googlesource.com/angle/angle/+/<branch name>/` --
including the terminating forward slash. (e.g.
`https://chromium.googlesource.com/angle/angle/+/main/`)

### Rolling Vulkan Memory Allocator (VMA)

ANGLE and other Google projects (e.g. Skia, Chrome) use the open-source [Vulkan Memory Allocator][vma-upstream] (VMA)
library. As with with other external repositories, these projects do not directly use the [upstream Vulkan Memory Allocator][vma-upstream] repository.
Instead, a [Google-local repository][vma-chrome] is used, which contains Google-local changes and fixes (e.g. changes
to `BUILD.gn`). This Google-local repository repository contains the following key branches:

- `upstream/master` is automatically mirrored with the contents of the [upstream VMA][vma-upstream] repository
- `main` is manually curated by Google, with a combination of upstream and Google-local changes

ANGLE's `DEPS` file points to a git SHA-1 revision of the `main` branch.

Manual rolls of the `main` branch currently involve rebasing all of the Google-local changes on top of newer upstream changes. The current process (done in 2022) is to:

 * Revert all of the Google-local changes (i.e. with a single commit)
 * Merge or cherry-pick all of the upstream changes
 * Cherry-pick the Google-local changes on top
 * Note: it may be possible to simply merge future upstream changes directly, without reverting the Google-local changes

Manual rolls of which SHA-1 revision the ANGLE's `DEPS` file points to is done via the process
outlined above. Within an ANGLE build, you can navigate to the `third_party/vulkan_memory_allocator`
directory, check out the `main` branch, and use `git log` to select the desired Git revision.
**Please note** that cross-project coordination may be required when rolling VMA, as some projects (e.g. Chrome) builds itself with a single VMA version across Chrome, ANGLE, and Skia.

[vma-upstream]: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
[vma-chrome]: https://chromium.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator

Note: When ANGLE is AutoRolled to the Android AOSP source tree, Google-local
changes to the VMA `BUILD.gn` file will be converted to the ANGLE `Android.bp` file.

## Branching ANGLE

Sometimes, individual changes to ANGLE are needed for a release of Chrome which
has already been branched. If this is the case, a branch of ANGLE should be
created to correspond to the Chrome release version, so that Chrome may
incorporate only these changes, and not everything that has been committed since
the version on which Chrome depended at branch time. **Please note: Only ANGLE
admins can create a new branch.** To create a branch of ANGLE for a branched
Chrome release:

 * Determine what the ANGLE dependency is for the Chrome release
by checking the DEPS file for that branch.
 * Check out this commit as a new branch in your local repository.
   * e.g., for [the Chrome 34 release at
chrome/branches/1847](http://src.chromium.org/viewvc/chrome/branches/1847/src/DEPS),
the ANGLE version is 4df02c1ed5e97dd54576b06964b1da67ea30238e. To
check this commit out locally and create a new branch named 'mybranch'
from this commit, use: ```git checkout -b mybranch
4df02c1ed5e97dd54576b06964b1da67ea30238e```
 * To create this new branch in the public repository, you'll need to push the
branch to the special Gerrit reference location, 'refs/heads/<branch name>'. You
must be an ANGLE administrator to be able to push this new branch.
    * e.g., to use your local 'mybranch' to create a branch in the public repository called
'chrome\_m34', use: ```git push origin mybranch:refs/heads/chrome_m34```
    * The naming convention that ANGLE uses for its release-dedicated branches is 'chrome\_m##'.
