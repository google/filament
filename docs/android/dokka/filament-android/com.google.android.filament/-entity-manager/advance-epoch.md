//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[EntityManager](index.md)/[advanceEpoch](advance-epoch.md)

# advanceEpoch

[main]\
open fun [advanceEpoch](advance-epoch.md)()

Advances the timeline to the next epoch. 

Increments the current epoch ID and seals the current epoch. All subsequent entity destructions will be recorded in the new epoch. Automatically triggers reclaimSafeEpochs() to recycle safe, completed epochs. Thread-safe.
