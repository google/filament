//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[DepthOfFieldOptions](index.md)/[foregroundRingCount](foreground-ring-count.md)

# foregroundRingCount

[main]\
open var [foregroundRingCount](foreground-ring-count.md): Int

Number of of rings used by the gather kernels. 

The number of rings affects quality and performance. The actual number of sample per pixel is defined as (ringCount * 2 - 1)^2. Here are a few commonly used values: 

- 3 rings : 25 ( 5x 5 grid)
- 4 rings : 49 ( 7x 7 grid)
- 5 rings : 81 ( 9x 9 grid)
- 17 rings : 1089 (33x33 grid)

With a maximum circle-of-confusion of 32, it is never necessary to use more than 17 rings.

Usually all three settings below are set to the same value, however, it is often acceptable to use a lower ring count for the &quot;fast tiles&quot;, which improves performance. Fast tiles are regions of the screen where every pixels have a similar circle-of-confusion radius.

A value of 0 means default, which is 5 on desktop and 3 on mobile.

number of kernel rings for foreground tiles

#### See also

| |
|---|
| [backgroundRingCount](background-ring-count.md) |
| [fastGatherRingCount](fast-gather-ring-count.md) |
