Try to stress memory allocators in the implementation and driver.

TODO: plan and implement
- Tests which (pseudo-randomly?) allocate a bunch of memory and then assert things about the memory
  (it's not aliased, it's valid to read and write in various ways, accesses read/write the correct data)
    - Possibly also with OOB accesses/robust buffer access?
- Tests which are targeted against particular known implementation details
