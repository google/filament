//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[commitLocalTransformTransaction](commit-local-transform-transaction.md)

# commitLocalTransformTransaction

[main]\
open fun [commitLocalTransformTransaction](commit-local-transform-transaction.md)()

Commits the currently open local transform transaction. When this returns, calls to getWorldTransform will return the proper value. 

Failing to call this method when done updating the local transform will cause a lot of rendering problems. The system never closes the transaction automatically.

If the local transform transaction is not open, this is a no-op.

#### See also

| |
|---|
| [openLocalTransformTransaction](open-local-transform-transaction.md) |
| setTransform |
