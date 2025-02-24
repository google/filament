Tests for behavior with multiple threads (main thread + workers).

TODO: plan and implement
- 'postMessage'
  Try postMessage'ing an object of every type (to same or different thread)
    - {main -> main, main -> worker, worker -> main, worker1 -> worker1, worker1 -> worker2}
    - through {global postMessage, MessageChannel}
    - {in, not in} transferrable object list, when valid
- 'concurrency'
  Short tight loop doing many of an action from two threads at the same time
    - e.g. {create {buffer, texture, shader, pipeline}}
