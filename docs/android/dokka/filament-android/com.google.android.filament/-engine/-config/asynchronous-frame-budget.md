//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[asynchronousFrameBudget](asynchronous-frame-budget.md)

# asynchronousFrameBudget

[main]\
open var [asynchronousFrameBudget](asynchronous-frame-budget.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Per-frame time budget for queued asynchronous operations. 

It applies when they run on the backend thread (AsynchronousMode::AMORTIZATION, or THREAD_PREFERRED falling back to it). It is the elapsed time on the backend thread, measured with std::chrono::steady_clock, so time the thread spends preempted counts too.

The operations run in Renderer::endFrame(), after the frame's own commands, and in Renderer::skipFrame(). A frame skipped because Renderer::beginFrame() returned false runs none, since the GPU is already behind. Each Renderer gets its own budget per frame, and so does each Renderer::renderStandaloneView() call.

One operation always runs per frame, even with a budget of 0, so the queue keeps moving. An operation that starts within the budget runs to completion, so a frame can go over by up to one operation.
