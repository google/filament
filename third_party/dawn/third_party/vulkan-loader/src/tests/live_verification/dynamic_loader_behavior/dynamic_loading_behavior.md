# General behavior of dynamic linkers on linux and macOS

## Linking only

Symbols are found based on link order. If two libraries export the same symbol,
the first library in the link list is the library which is called.

## Loading

Applications cannot load symbols without specifying a library.
Putting RTLD_NEXT in dlsym will not find any symbols even if a loaded library
should contain them.
An app _must_ reference a library that was loaded with `dlopen` to be able to
get at its exported symbols through `dlsym`.
If a loaded library loads subsequent libraries, and they all export the same
symbols, the library which the application explicitly loaded is the one whose
symbols are used.

## Linking and Loading

Applications can now use RTLD_NEXT to query symbols.
They will always be the first linked library is that exported the symbol.
In other words, loading a library explicitly doesn't change the behavior.

Similarly, explicitly loaded libraries behave the same.
Symbols from loaded libraries must be referenced to load the symbol, RTLD_NEXT
doesn't work.
If a loaded library subsequently loads another library which exports the same
symbols, then the library the application explicitly loaded is used.
If one of the subsequently loaded libraries happened to be linked, this doesn't
affect the behavior when querying functions from the application loaded library.