//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[openLocalTransformTransaction](open-local-transform-transaction.md)

# openLocalTransformTransaction

[main]\
open fun [openLocalTransformTransaction](open-local-transform-transaction.md)()

Opens a local transform transaction. During a transaction, getWorldTransform can return an invalid transform until [commitLocalTransformTransaction](commit-local-transform-transaction.md) is called. However, setTransform will perform significantly better and in constant time. 

This is useful when updating many transforms and the transform hierarchy is deep (say more than 4 or 5 levels).

If the local transform transaction is already open, this is a no-op.

#### See also

| |
|---|
| [commitLocalTransformTransaction](commit-local-transform-transaction.md) |
| setTransform |
