//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[EntityManager](index.md)

# EntityManager

[main]\
open class [EntityManager](index.md)

Thread-safe coordinator managing the lifecycle, indices, and generation tracking for all Entity instances. 

The EntityManager is the core source-of-truth for Entity identities in Filament. It coordinates the creation, recycling, and safe reclamation of 32-bit Entity IDs. Due to the highly multithreaded nature of the engine, the EntityManager provides **two distinct groups of APIs**:

### 1. Public API (Gameplay &Application Layer)

This group is aimed at standard usage for applications, renderers, and game systems.

- **Entity Creation**: Methods like `create`() and `create`(size_t, Entity*) allocate new or recycled Entity identities.
- **Entity Destruction**: Methods like `destroy`(Entity) and `destroy`(size_t, Entity*) logically kill entities, instantly advancing the active timeline.
- **Queries** &**Hooks**: Mechanisms to query logical presence (`isAlive`()) or subscribe to global destruction notifications via `registerChangeCallback`().

### 2. Internal Component API (ECS Integration &Component Managers)

This group supports the underlying asynchronous **Epoch-Based Reclamation (EBR)** garbage collector. Designed exclusively for subclasses of `SingleInstanceComponentManagerBase` (e.g. `FCameraManager`, `FTransformManager`), these methods govern thread-consensus and physical payload purging budgets:

- **Consensus Registration**: Methods like `registerWatermark`() and `unregisterWatermark`() allow components to join or vacate the global timeline consensus.
- **Timeline Sweeps**: Methods like `advanceEpoch`() and `reclaimSafeEpochs`() seal completed frames and process physical reclaiming boundaries.
- **Missed Garbage Harvests**: APIs like `getMissedGarbage`() enable Component Managers to retrieve logically dead Entity bitsets precisely synced against their local watermarks in atomic, collision-free transaction blocks.

## Functions

| Name | Summary |
|---|---|
| [advanceEpoch](advance-epoch.md) | [main]<br>open fun [advanceEpoch](advance-epoch.md)()<br>Advances the timeline to the next epoch. |
| [create](create.md) | [main]<br>open fun [create](create.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Allocates and creates a single new or recycled Entity.<br>[main]<br>open fun [create](create.md)(entities: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;)<br>Allocates and creates a batch of new or recycled Entities. |
| [destroy](destroy.md) | [main]<br>open fun [destroy](destroy.md)(e: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Globally and logically destroys a single Entity identity.<br>[main]<br>open fun [destroy](destroy.md)(entities: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;)<br>Globally and logically destroys a batch of Entities. |
| [flushNotifications](flush-notifications.md) | [main]<br>open fun [flushNotifications](flush-notifications.md)()<br>Flushes all pending entity lifecycle change notifications to registered callbacks. |
| [get](get.md) | [main]<br>open fun [get](get.md)(): [EntityManager](index.md)<br>Gets a reference to the global EntityManager singleton. |
| [getEntityCount](get-entity-count.md) | [main]<br>open fun [getEntityCount](get-entity-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the total number of currently active/alive Entities. |
| [getIndex](get-index.md) | [main]<br>open fun [getIndex](get-index.md)(e: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Use carefully, several entities can have the same index. |
| [getLatestEpochID](get-latest-epoch-i-d.md) | [main]<br>open fun [getLatestEpochID](get-latest-epoch-i-d.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Returns the current active (unsealed) epoch ID. |
| [getMaxEntityCount](get-max-entity-count.md) | [main]<br>open fun [getMaxEntityCount](get-max-entity-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Retrieves the maximum theoretical upper bound of entities that can exist concurrently. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [isAlive](is-alive.md) | [main]<br>open fun [isAlive](is-alive.md)(e: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Queries the logical lifecycle state of a given Entity. |
| [reclaimSafeEpochs](reclaim-safe-epochs.md) | [main]<br>open fun [reclaimSafeEpochs](reclaim-safe-epochs.md)()<br>Reclaims and recycles all indices from epochs that are safe to reclaim. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [EntityManager](index.md) |
