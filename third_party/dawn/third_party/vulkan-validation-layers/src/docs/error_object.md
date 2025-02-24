# Error Object

When we print error messages, we want to include as much useful information as possible, while also not giving anything that would just be noise. This requires at a `LogError` call to have this information available.

In the past, there was a lot of parameters passed around functions only for the sake of being able to print it out in the `LogError` message.  The `ErrorObject` was created as a way to improve this. With a easier way to pass information, it is less error prone to both forgot the information and/or provide bad information

## Single ErrorObject

The `chassis.cpp` holds the single `ErrorObject` reference which is passed to all `PreCallValidate` calls.

## Location

It is very important to know "where" in a function call the error occured, this is where the `Location` object comes in.

inside `chassis.cpp` we generate the starting `Location` for `ErrorObject`. From here inside each function, we can append what we need

```cpp
// example

bool PreCallValidateQueueBindSparse(/*..*/ ErrorObject &error_obj) const {
    // automatically has Func::vkQueueBindSparse
    // note: error_obj.location is used if just want to print the function
    LogError("VUID-A", error_obj.location);

    for (uint32_t i = 0; i < bindInfoCount; ++i) {
        const Location loc = error_obj.location.dot(Field::pBindInfo, i);
        LogError("VUID-B", loc); // i == 3

        for (uint32_t j = 0; j < bufferBindCount; ++j) {
            const Location buffer_loc = loc.dot(Field::pBufferBinds, j);
            LogError("VUID-C", buffer_loc); // j == 2
        }

        if (pNext == VkTimelineSemaphoreSubmitInfo) {
            // use pNext() instead of dot() to print out it was part of a pNext chain
            const Location pnext_loc = loc.pNext(Struct::VkTimelineSemaphoreSubmitInfo, Field::waitSemaphoreValueCount);
            LogError("VUID-D", pnext_loc);
        }
    }
}
```

will produce the following location in the error message

```
[VUID-A] vkQueueBindSparse():
[VUID-B] vkQueueBindSparse(): pBindInfo[3]
[VUID-C] vkQueueBindSparse(): pBindInfo[3].pBufferBinds[2]
[VUID-D] vkQueueBindSparse(): pBindInfo[3].pNext<VkTimelineSemaphoreSubmitInfo>.waitSemaphoreValueCount
```

> We generate the `Func`/`Struct`/`Field` for all possible items from the XML

### Using fields in the  error messages

using the `Location::Fields()` you can print the location, minus the function, as a string

```cpp
const Location loc = error_obj.location.dot(Field::pBindInfo, i); // vkQueueBindSparse(): pBindInfo[3]

// prints "pBindInfo[3]"
LogError(/*..*/, "%s". loc.Fields().c_str());
```

### Limitations

When using the `.dot()` operation, it returns a new copy of `Location`. If you chain two `.dot().dot()` you need to be mindful.

The following will produce a `stack-use-after-scope` runtime error

```cpp
const Location layout_loc = loc.dot(Field::attachment).dot(Field::layout);
LogError(/*..*/, layout_loc, "error");
```

The 2 ways around the are:

```cpp
// Create 2nd variable
const Location attachment_loc = loc.dot(Field::attachment)
const Location layout_loc     = attachment_loc.dot(Field::layout);
LogError(/*..*/, layout_loc, "good");
```

or has been found to be more common

```cpp
// Pass an argument
LogError(/*..*/, loc.dot(Field::attachment).dot(Field::layout), "good");
```

### Using stack and making copies

The original design was to not make copies of `Location` and just have it modify the `ErrorObject::Location` but this would require you to remove items after a function call

```cpp
// example
error_obj.location.add(Struct::VkSomething);
ValidateItem(error_obj);
error_obj.location.remove(Struct::VkSomething);
```

which slowly leads to a LOT more code and becomes very error prone to forget to remove `Location` values. Instead the `dot` operator return a new copy of `Location`.

## LogObjectList

// TODO
