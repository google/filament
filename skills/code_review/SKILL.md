---
name: code-review
description: >
  Adversarial systems, concurrency, memory-model, and architecture code review protocol.
  Use this skill whenever asked to review code, PRs, diffs, or branches in Filament.
---

# Systems & Concurrency Code Review Protocol

This skill defines the mandatory protocol for conducting rigorous, adversarial code reviews on Filament source code. Standard single-pass reviews focusing on style or superficial idioms are strictly prohibited. Reviews must prioritize **correctness, concurrency invariants, memory models, lifecycle safety, API contracts, arithmetic boundaries, and test integrity**.

---

## 1. Core Review Philosophy

1. **Adversarial Stance**: Assume the code contains subtle race conditions, memory ordering errors, and unhandled edge cases until proven otherwise.
2. **Zero Superficial Feedback**: Do not waste review bandwidth on formatting, whitespace, or obvious variable naming unless it actively impairs correctness or violates [CODE_STYLE.md](../../CODE_STYLE.md).
3. **Cross-Function & State-Machine Tracing**: Never review functions in isolation. Concurrency bugs (e.g., lost wakeups, ABA races) live at the intersection of multiple functions (e.g., `put()`, `wakeOne()`, `waitForWork()`, `loop()`). Trace full execution state machines across caller/callee boundaries.
4. **False-Positive Elimination**: Before reporting a finding, attempt to disprove it by searching for compensating synchronization, refcount credits, or structural invariants elsewhere in the file or call chain.

---

## 2. Mandatory Analytical Lenses (The 6 Pillars)

Every review must systematically evaluate the change through these six analytical lenses:

### Pillar 1: Concurrency, Synchronization & Memory Models
*   **Atomic Memory Ordering & Barrier Direction**:
    *   Inspect every `std::atomic` operation and its `std::memory_order` (`relaxed`, `acquire`, `release`, `acq_rel`, `seq_cst`).
    *   Verify barrier directions: `memory_order_release` prevents *prior* memory writes from sinking below the release store, but does **not** prevent *subsequent* non-atomic writes from floating above it. `memory_order_acquire` prevents *subsequent* reads from floating above the acquire load, but does **not** protect prior reads.
*   **Lost Wakeups & Condition Variable Invariants**:
    *   Check if condition variable signals (`notify_one()` / `wakeOne()`) or waiting loops (`wait()`) are gated by unlocked relaxed loads (e.g., relaxed check of waiter counts).
    *   Verify that the happens-before relationship between producer publication and consumer sleep is never bypassed.
*   **ABA, Slot-Reuse & Torn Reads**:
    *   In wait-free, lock-free, or seqlock data structures, trace multi-step read-then-validate patterns.
    *   Ensure that slot identity (e.g., thread ID, generation counter) is re-validated *after* copying payload data, not just the slot state, to prevent returning payload data read during an erase/re-emplace.
*   **Data Races in Lock-Free / Wait-Free APIs**:
    *   Verify that plain (non-atomic) fields are never read concurrently with plain writes in other threads without a formal synchronization barrier.
*   **Deadlock & Lock Invariants**:
    *   Verify lock acquisition order. Ensure code uses `utils::Mutex` / `utils::Condition` with `LockGuard const` as mandated by [skills/cpp_static_thread_safety](../cpp_static_thread_safety/SKILL.md).

### Pillar 2: Arithmetic, Narrowing & Numerical Boundaries
*   **Integer Truncation & Narrowing Conversions**:
    *   Check all numeric assignments and casts between types of differing bit widths (e.g., `uint32_t` $\to$ `uint16_t`, `size_t` $\to$ `uint32_t`).
    *   Determine what happens when values equal or exceed the narrower type's capacity (e.g., exact multiples of 65,536 truncating to 0 and inducing infinite loops).
*   **Boundary Conditions**:
    *   Evaluate behavior at `count == 0`, `count == 1`, $2^{16}$, $2^{32}$, `INT_MAX`, and overflow/underflow points.
*   **Resource & Pool Budgets**:
    *   Ensure budget calculations cannot silently drop minimum guarantees (e.g., combined thread limits reducing worker pool size to 0 without assertion or fallback).

### Pillar 3: API Contracts & Behavioral Deltas
*   **Semantic Regressions**:
    *   Compare old vs. new contracts. Did zero-sized inputs (`count == 0`), empty containers, or null arguments change behavior (e.g., skipping a functor that previously ran once)?
*   **Trait Specializations & SFINAE / Concepts**:
    *   Verify that template traits (e.g., `SplitterTraits`) do not silently ignore legacy interfaces or custom types without a compilation error or fallback.
*   **Doc-Comment vs. Reality**:
    *   Verify that documented thread-safety guarantees (e.g., "wait-free", "thread-safe for concurrent readers") are actually fulfilled by the implementation.

### Pillar 4: Object Lifetime, Teardown & RAII
*   **Thread Lifecycle & Cleanup**:
    *   Verify that adopted threads or registered slots properly decrement active counters upon exit/emancipation (`emancipate()`), preventing capacity degradation.
*   **Asynchronous Lambda Captures**:
    *   Ensure captured references (`[&]`) or raw pointers (`this`) outlive background job execution.
*   **Refcount Draining & Destruction Races**:
    *   Verify that root jobs / parent structures hold an explicit refcount credit during worker dispatch so refcounts cannot hit zero mid-execution on worker failure.

### Pillar 5: Test Integrity & False-Pass Audits
*   **Tautological Assertions**:
    *   Inspect test assertions to ensure they are not tautologies (e.g., assertions that can never evaluate to false).
*   **Concurrency Stress & Race Coverage**:
    *   Verify that multi-threaded tests genuinely create contention on the guarded path rather than executing deterministically or passing due to timing coincidence.
*   **Build Wiring**:
    *   Verify new test files are wired into the corresponding `CMakeLists.txt`.

### Pillar 6: Architectural Completeness & Scaling
*   **Feature Completeness**:
    *   When introducing a new capability (e.g., adopted worker threads), verify that all sizing, partitioning, and helper calculations (e.g., `parallel_for` helper job counts) scale with the new capability rather than only querying legacy state.

---

## 3. Standard Review Output Format

Reviews must be structured using the following format:

```markdown
<File Path>
  - <line> [<category>] <Concise finding description detailing the exact failure mechanism.>

...

Review Summary:
- **The Blocker**: Identify the single most critical blocking issue, explaining the thread interleaving or failure sequence.
- **Intent vs. Code**: Assess whether the PR achieves its architectural goals and note any systematic omissions.
- **Explicitly Checked & Ruled Out**: List non-obvious complex areas investigated and verified as safe/correct (proves thorough inspection and eliminates false positives).
- **Minor Cleanups**: Note dead code, minor un-decremented tracking counters, or triplicated logic below the blocking finding bar.
```

### Category Tags:
*   `[correctness]` — Logic errors, data races, memory ordering bugs, lost wakeups, narrowing bugs, crashes.
*   `[efficiency]` — Scaling bottlenecks, false sharing, unnecessary allocations, missed parallelism.
*   `[api-contract]` — Broken traits, dropped interface support, violated preconditions/postconditions.
*   `[behavior-change]` — Silent changes to edge-case behavior (e.g., `count == 0`).
*   `[test-coverage]` — Tautological tests, missing failure checks, unwired tests.

---

## 4. Review Execution Workflow for AI Agents

When instructed to review code, a branch, a commit range, or a PR:

1. **Discover Changes**:
   ```bash
   git diff <base_branch>...<target_branch>
   # or
   git log -n <count> --stat
   ```
2. **Read Related Context**:
   Read the modified files alongside their callers and dependencies using `view_file` to understand the complete cross-function state machine.
3. **Execute the 6 Analytical Pillars**:
   Systematically audit the diff against each pillar.
4. **Draft & Filter Findings**:
   Verify every potential finding against the codebase to eliminate false positives.
5. **Format Report**:
   Output the structured findings following the format in Section 3.
