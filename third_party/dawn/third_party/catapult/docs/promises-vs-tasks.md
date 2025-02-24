# Promises vs Tasks

`Promise` is a well-defined [built-in object](https://developer.mozilla.org/en/docs/Web/JavaScript/Reference/Global_Objects/Promise) that makes it easy, among other things, to chain synchronous or asynchronous operations.

`Task` is an object defined in trace_viewer/base that can be used to chain synchronous operations.

Since both tasks and promises allow chaining operations it's easy to confuse them. The goal of this page is to clarify how the classes differ and when to use either.

## Semantic differences

A first important difference is that tasks cannot be used with asynchronous operations. In a `Promise`, you can chain an asynchronous operation by calling `then(functionReturningAPromise)`. On the other hand, if you create a task from `functionReturningAPromise` and call `after()` on it, then the subsequent task will execute before the promise is resolved.

Another difference is that it's possible to run a task in a synchronous way. Calling `run()` on a task will block until the first task in the chain is completed. This is made possible by the fact that all operations in the task queue are synchronous. Compare this to `Promise` which doesn't have a `run` method and for which execution is scheduled as soon as the promise is created.

Finally, where tasks allow a queue of dependent operations, promises are more flexible and allow creating a more complex dependency graph between operations via `then()`, `Promise.all()` and `Promise.race()`.

## Differences in API

Chaining operations is similar:

    taskA.after(taskB).after(taskC);
    promiseA.then(functionReturningPromiseB).then(functionReturningPromiseC);

An important difference, though is a task can only have one `after()` task. Trying to do `taskA.after(taskB); taskA.after(taskC);` will result in a runtime error. Promises have no such limitations.

In addition to `after()`, tasks expose the method `subtask()` that makes it possible to insert tasks in the execution chain, even as the operations in the chain are being executed. Promises do not directly support this feature.

Tasks have a `run()` method to synchronously execute the next operation in the queue. Promises have no such thing, and getting control back once a promise has executed requires the use of `then()`.

`Task.RunWhenIdle` is another API call that is not available on promises. It schedule the task queue to be gradually consumed, tying task execution to `requestAnimationFrame` in a way that makes sure not to bust the time budget for a frame. Given its asynchronous nature `RunWhenIdle` returns a `Promise`.

Note that there are a number of other minor API differences that are omitted here.

## What should you use?

If some of the operations you want to chain are asynchronous then you don't have a choice and must use `Promise`. If you want to chain only synchronous operations, then `Task` may be the right choice for you, especially if you plan on executing your operations when the application is idle, in which case you'll want to benefit from `RunWhenIdle`.

## Future works

It's sad that the nature of tasks makes it impossible to use `RunWhenIdle` with an asynchronous operation. It might be interesting to figure out whether it's possible to build a `RunWhenIdle` on top of promises. Also, if the `after()` and `subtask()` API of tasks make the code simpler in some instances it may be interesting to try to reproduce it on top of promises. Given these improvements to `Promise` it may be possible to eventually remove `Task` entirely.
