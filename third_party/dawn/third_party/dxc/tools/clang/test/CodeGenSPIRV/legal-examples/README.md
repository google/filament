Legalization Examples
=====================

HLSL legalization for SPIR-V is a fuzzy topic; it cannot be expressed precisely
using some grammar. Instead, we've collected a few simple examples here to
show what cases are allowed and what not.

These examples are all variants to `0-copy-sbuf.hlsl`. And `*-ok.hlsl` are
allowed cases, while `*-fail.hlsl` are not allowed cases.

Also keep in mind that legalization is an ongoing effort; support has evolved
and will continue to evolve over time in response to new use cases in the wild.
