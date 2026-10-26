# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut
- filamat: convert FOG variant into a specialization constant. [⚠️ **Recompile Materials**]
- engine: in `AsynchronousMode::AMORTIZATION`, queued async operations now run at the end of each
  frame and in `Renderer::skipFrame()`, within a per-frame time budget set by the new
  `Engine::Config::asynchronousFrameBudget` (2 ms by default), instead of 5 operations per backend
  tick. Metal now runs them every frame too
- java: `Engine.Config.asynchronousFrameBudget` (in nanoseconds) is added
- web: `Engine$Config.asynchronousFrameBudgetMs` is added
- vulkan: fix a crash when more than 32 timer queries are outstanding. `createTimerQueryS` returns
  a null handle once the pool is exhausted, and `getTimerQueryValue` reports `ERROR` for it
  (#10658)
