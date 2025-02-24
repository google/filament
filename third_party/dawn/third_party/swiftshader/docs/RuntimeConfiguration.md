Runtime Configuration
=========================

SwiftShader provides a simple configuration mechanism based on a configuration file to control a variety of runtime options without needing to re-compile from source.

Configuration file
------------

SwiftShader looks for a file named `SwiftShader.ini` (case-sensitive) in the working directory. At startup, SwiftShader reads this file, if it exists, and sets the options specified in it.

The configuration file syntax is a series of key-value pairs, divided into sections. The following example shows three key-value pairs in two sections (`ThreadCount` and `AffinityMask` in the `[Processor]` section, and `EnableSpirvProfiling` in the `[Profiler]` section):
```
[Processor]
ThreadCount=4
AffinityMask=0xf

# Comment
[Profiler]
EnableSpirvProfiling=true
```

The syntax rules are as follows:
* Sections are defined via a name in brackets, e.g. `[Processor]`.
* Key-value pairs are in the format `Key=Value`.
* Keys are always strings, while values can be strings, booleans or integers depending on the semantics of the option:
  * For integer options, both decimal and hexademical values are supported. 
  * For boolean options, both decimal (`1` and `0`) and alphabetical (`true` and `false`) values are supported.
* Comments are supported through the use of the `#` character at the beginning of a line.

Options
------------

Refer to the [SwiftConfig.hpp](../src/System/SwiftConfig.hpp) header for an up-to-date overview of available options.

