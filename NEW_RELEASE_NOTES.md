# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes
We are chaning the way Vulkan buffers are handled. We need to switch over to a managed (or view-based) model where the data stored inside the object is a proxy to a Vulkan object that can dynamically be swapped around.

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- `setFrameScheduledCallback` now works on all backends (frame presentation scheduling is still only
  available on Metal). Non-Metal backends can use the callback to be notified when Filament has
  finished processing a frame on the CPU.
- materials: added `getEyeFromViewMatrix()` for vertex shader [⚠️ **Recompile Materials**]
- matc: make `--workarounds=none` the default [**Recompile Materials to take effect**]
