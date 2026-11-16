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

### 1. The Global Epoch Boundaries & 17-Bit Index Space
`EntityManagerImpl` maintains a growing array of sealed epoch bitsets (`PagedArenaBitset`). Because EBR guarantees that two `Entity` instances sharing the same 17-bit base index can never coexist while alive or pending physical eviction, all `PagedArenaBitset` instances store only the 17-bit base index (`EntityManager::getIndex(e)`), and `mGens[index]` retains the dead entity's generation until `recycleGarbageEpochs()` increments it right before returning `index` to `mFreeList`. Any index read from a manager's or scene's bitset can therefore be lock-freely reinflated to its full 32-bit `Entity` via `EntityManager::getEntity(index)`.

### 2. Garbage Harvesting (`catchupGarbage()`)
In `catchupGarbage()`, dead 17-bit entity indices from newly sealed epochs are bitwise intersected with `mEntities` and merged into `mPendingDestruction`, advancing `mHarvestedEpoch`. The dead indices remain in `mEntities` (and `mWatermark` is held) until physical eviction completes, guaranteeing that `EntityManagerImpl` never recycles an entity index while a component for that index still physically resides in any manager or scene.

### 3. Physical Array Defragmentation & Watermark Commit (`gc()`)
When `gc()` executes, it pops batches of 17-bit indices from `mPendingDestruction`, reinflates each `Entity` via `EntityManager::getEntity(index)`, and invokes `removeComponent()`, performing a swap-and-pop move to keep the underlying `StructureOfArrays` tightly packed in memory and removing each evicted index from `mEntities`. Once `mPendingDestruction` drains (or across frames via `EntityManagerImpl`'s automatic `mEntities` intersection leap-forward), `mWatermark` advances, allowing `EntityManagerImpl` to increment `mGens[index]` and safely recycle the expired epoch indices.
