//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[EntityManager](index.md)/[reclaimSafeEpochs](reclaim-safe-epochs.md)

# reclaimSafeEpochs

[main]\
open fun [reclaimSafeEpochs](reclaim-safe-epochs.md)()

Reclaims and recycles all indices from epochs that are safe to reclaim. 

This method is intended EXCLUSIVELY for testing, unit test harnesses, and micro-benchmarks to trigger harvesting sweeps in isolation. In production, Filament NEVER needs to call this method, as advanceEpoch() automatically and synchronously invokes the identical recycling sweep.
