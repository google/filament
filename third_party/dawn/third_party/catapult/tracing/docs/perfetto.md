**July 2022 update**: chrome://tracing is deprecated, and by default will
redirect to https://ui.perfetto.dev. At the moment, it's still possible to use
the old UI, but there's no guarantee that it will continue to function;
please file [feature requests](https://perfetto.dev/docs/#bugs) for any
blockers that prevent you from migrating from chrome://tracing to Perfetto.

[Perfetto](https://perfetto.dev) is an evolution of chrome://tracing. Try it
out!

Perfetto offers various advantages, including:

* Support for larger trace sizes and more responsive trace navigation
* Interactive SQL queries to analyze the trace model
* Android system trace recording via WebUSB

So far, the Perfetto UI has been optimized for use cases on Android. If your
Chromium use case could be supported in better ways, or if you're missing a
loved feature from the catapult viewer, please leave us
[feedback](https://perfetto.dev/docs/#bugs).
