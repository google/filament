Tests to check that the WebGPU IDL is correctly implemented, for examples that objects exposed
exactly the correct members, and that methods throw when passed incomplete dictionaries.

See https://github.com/gpuweb/cts/issues/332

TODO: exposed.html.ts: Test all WebGPU interfaces instead of just some of them.
TODO: Check prototype chains. (Add a helper in IDLTest for this.)
