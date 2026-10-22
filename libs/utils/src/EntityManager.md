# Filament Epoch-Based Reclamation (EBR) Architecture

This document details the concurrent architectural data flows, timelines, and memory layout boundaries of Filament's asynchronous Epoch-Based Reclamation (EBR) garbage collector.

```
=================================================================================================
                          FILAMENT MULTITHREADED EBR ARCHITECTURE
=================================================================================================

 [ GLOBAL TIMELINE LAYER ] (EntityManagerImpl)
 
   Epoch ID:          ... [ Epoch 2 ]         [ Epoch 3 ]         [ Epoch 4 (ACTIVE) ]
   Status:                 (SEALED)            (SEALED)            (OPEN / UNSEALED)
                        +-------------+     +-------------+     +--------------------+
   Dead Entity Bitset:  | PagedArena  |     | PagedArena  |     | PagedArenaBitset   |
                        | Bitset(E2)  |     | Bitset(E3)  |     | (Active Deletions) |
                        +-------------+     +-------------+     +--------------------+
                               |                   |                      |
                               v                   v                      v
                             [---------------- STAGED GARBAGE ----------------]
                                                   |
                                                   | getMissedGarbage(Watermark = 2)
                                                   | Returns { E3 bitset }
                                                   v
-------------------------------------------------------------------------------------------------
 [ LOGICAL EXTRACTION LAYER ] (SingleInstanceComponentManagerBase)
 
   1. catchupGarbage() Execution Window:
   
      mMyWatermark (E2) -----------------------------------------------> Advance to E3
      
      PagedArenaBitset::intersect( Live Components (mEntities) , E3 Bitset )
                               |
                               +---> [ Logical Zombies Extracted ]
                                               |
                                               v
                                    mPendingDestruction.merge()
                                    mEntities.difference()
                                               |
                                               v
-------------------------------------------------------------------------------------------------
 [ PHYSICAL EVICTION LAYER ] (SingleInstanceComponentManager<T>)
 
   2. gc() Eviction Execution Window:
   
      mPendingDestruction.swap( readyToDestruct )
      
      readyToDestruct.forEachSetBit( ... )
             |
             +---> Index Collides with recycled Active Entity?
             |            |
             |            +- (Yes) -> JIT Zombie Purge (popPendingZombie) -> Compaction
             |            +- (No)  -> Invoke Concrete C++ Destructor (~Component)
             v
      StructureOfArrays (Dense Payload Mapping)
      +--------+--------+--------+--------+
      | Comp 0 | Comp 1 | Comp 2 | Comp 3 |  <--- Physical Payload Defragmentation
      +--------+--------+--------+--------+       (Swap-and-Pop on Destruction)
         [A]      [B]      [C]      [D]
=================================================================================================
```

## Architectural Key Concepts

### 1. The Global Epoch Boundaries
`EntityManagerImpl` maintains a growing array of sealed epoch bitsets. Only epochs that have been sealed via `advanceEpoch()` are processed by the managers.

### 2. Logical Deletion (O(1) Intersect)
In `catchupGarbage()`, dead entity IDs for sealed frames are bitwise intersected into `mPendingDestruction` and subtracted from `mEntities` instantly. Live rendering and game-loop queries immediately hide these components before their physical destructors have even been scheduled.

### 3. Physical Array Defragmentation
When `gc()` executes, it isolates the queue via an O(1) swap and processes each payload, utilizing `removeComponent()` to perform a swap-and-pop move to keep the underlying `StructureOfArrays` tightly packed in memory for maximum cache performance.

### 4. JIT Eviction (popPendingZombie)
Acts as an emergency safety net. If a recycled Entity ID index reallocates a payload spot before `gc()` has cleared the old frame's garbage, the implementation JIT-extracts the pending zombie from `mPendingDestruction` and safely purges the stale component to maintain layout integrity.
