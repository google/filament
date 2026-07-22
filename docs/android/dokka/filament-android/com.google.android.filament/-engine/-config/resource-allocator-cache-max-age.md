//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[resourceAllocatorCacheMaxAge](resource-allocator-cache-max-age.md)

# resourceAllocatorCacheMaxAge

[main]\
open var [resourceAllocatorCacheMaxAge](resource-allocator-cache-max-age.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

This value determines how many frames texture entries are kept for in the cache. This is a soft limit, meaning some texture older than this are allowed to stay in the cache. Typically only one texture is evicted per frame. The default is 1.
