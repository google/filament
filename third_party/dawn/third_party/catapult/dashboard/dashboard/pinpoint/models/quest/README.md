# Pinpoint Quests and Executions

A **Quest** is a description of work to do on a Change. For example, a Quest might say to kick off a build or test run, then check the result. When bound to a specific Change, an **Execution** is created.

Users wishing to spin up a version of Pinpoint might want to write their own set of Quests and Executions to integrate with their infrastructure or build system. The Quests and Executions must define the following methods.

## Quest

A **Quest** defines the following methods.

```python
def __str__(self)
```
Returns a short name to distinguish the Quest from other Quests in the same Job. E.g. "Build" and "Test".

```python
def Start(self, change, *args)
```
Returns an **Execution**, which is this Quest bound to a Change. `args` includes any arguments returned by the previous Execution's `Poll()` method. For example, the *Build* Execution might return the location of the build, which is then passed to the *Test* Execution to run a test on that build.

## Execution

`Quest.Start()` pulls in arguments from 3 sources to create the **Execution**:
* Arguments global to the Job that are used to create the Quest.
* The Change to bind the Execution to.
* Any `result_arguments` produced by the previous Execution.

With all this information, an Execution becomes a self-contained description of work to do. It defines the following methods.

```python
def _AsDict(self)
```
Returns a list of links and other debug information used to track the status of the Execution. Each entry of the list is a dict with the fields `key`, `value`, and `url`. Example:
```json
{
  "key": "Bot ID",
  "value": "build5-a4",
  "url": "https://chromium-swarm.appspot.com/bot?id=build5-a4",
}
```

```python
def _Poll(self)
```
Does all the work of the Execution. When the work is complete, it should call `self._Complete([result_values], [result_arguments])`. `result_values` contains any numeric results, like the value of a performance metric, and `result_arguments` is a dict containing any arguments to pass to the following Execution.

`_Poll()` can raise two kinds of Exceptions: Job-level Exceptions, which are fatal and cause the Job to fail; and Execution-level errors, which only cause that one Execution to fail. If something might pass at one commit and fail at another, it should be an Execution-level error. An Exception is Job-level if it inherits from `StandardError`, and Execution-level otherwise. This is so that things like `ImportError` and `SyntaxError` fail fast.

### Sharing information between Executions

Sometimes, a Quest's Executions don't want to run completely independently, but rather require some coordination between them. For example, maybe we want the first *Test* Execution to pick a device, and all following *Test* Executions to run on the same device. Any shared information can be stored on the Quest object and passed to the Executions via `Quest.Start()`.
