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
   
      mHarvestedEpoch (E2) --------------------------------------------> Advance to E3
      
      PagedArenaBitset::intersect( Live Components (mEntities) , E3 Bitset )
                               |
                               +---> [ Dead Entities Extracted ]
                                               |
                                               v
                                    mPendingDestruction.merge()
                                    (mEntities retains bits until physical eviction)
                                               |
                                               v
-------------------------------------------------------------------------------------------------
 [ PHYSICAL EVICTION LAYER ] (SingleInstanceComponentManager<T>)
 
   2. gc() Eviction Execution Window:
   
      mPendingDestruction.popSetBits( ... )
             |
             v
      Invoke Concrete C++ Destructor (~Component) -> eraseComponentFromMap()
             |
             +---> mPendingDestruction.fetchRemove(e)
             +---> mEntities.remove(e)
             +---> When mPendingDestruction.empty(): mWatermark.store(mHarvestedEpoch)
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

### 2. Garbage Harvesting (`catchupGarbage()`)
In `catchupGarbage()`, dead entity IDs from newly sealed epochs are bitwise intersected with `mEntities` and merged into `mPendingDestruction`, advancing `mHarvestedEpoch`. The dead entities remain in `mEntities` (and `mWatermark` is held) until physical eviction completes, guaranteeing that `EntityManagerImpl` never recycles an entity index while a component for that index still physically resides in any manager.

### 3. Physical Array Defragmentation & Watermark Commit (`gc()`)
When `gc()` executes, it pops batches from `mPendingDestruction` and invokes `removeComponent()`, performing a swap-and-pop move to keep the underlying `StructureOfArrays` tightly packed in memory and removing each evicted entity from `mEntities`. Once `mPendingDestruction` drains (or across frames via `EntityManagerImpl`'s automatic `mEntities` intersection leap-forward), `mWatermark` advances, allowing `EntityManagerImpl` to safely recycle the expired epoch indices.
