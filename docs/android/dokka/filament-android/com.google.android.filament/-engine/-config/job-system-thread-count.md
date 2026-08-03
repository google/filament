//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Engine](../index.md)/[Config](index.md)/[jobSystemThreadCount](job-system-thread-count.md)

# jobSystemThreadCount

[main]\
open var [jobSystemThreadCount](job-system-thread-count.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Number of threads to use in Engine's JobSystem. Engine uses a utils::JobSystem to carry out paralleization of Engine workloads. This value sets the number of threads allocated for JobSystem. Configuring this value can be helpful in CPU-constrained environments where too many threads can cause contention of CPU and reduce performance. The default value is 0, which implies that the Engine will use a heuristic to determine the number of threads to use.
