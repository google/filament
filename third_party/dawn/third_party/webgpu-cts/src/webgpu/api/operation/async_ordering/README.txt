Test ordering of async resolutions between promises returned by the following calls (and possibly
between multiple of the same call), where there are constraints on the ordering.
Spec issue: https://github.com/gpuweb/gpuweb/issues/962

TODO: plan and implement
- createReadyPipeline() (not sure if this actually has any ordering constraints)
- cmdbuf.executionTime
- device.popErrorScope()
- device.lost
- queue.onSubmittedWorkDone()
- buffer.mapAsync()
- shadermodule.getCompilationInfo()
