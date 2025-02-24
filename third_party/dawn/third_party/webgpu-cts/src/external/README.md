# External Modules

This directory contains external modules that are used by the WebGPU
CTS. These are included in the repo, as opposed to being fetched via a
package manager or CDN, so that there is a single canonical source of
truth for the CTS tests and the CTS tests can be run as a standalone
suite without needing to pull from a CDN or similar process.

## Adding modules

Each module that is added should be done consciously with a clear
reasoning on what the module is providing, since the bar for adding
new modules should be relatively high.

The module will need to be licensed via a compatible license to the
BSD-3 clause & W3C CTS licenses that the CTS currently is covered by.

It is preferred to use a single source build of the module if possible.

In addition to the source for the module a LICENSE file should be
included in the directory clearly identifying the owner of the module
and the license it is covered by.

Details of the specific module, including version, origin and purpose
should be listed below.

## Current Modules

| **Name**             | **Origin**                                       | **License** | **Version** | **Purpose**                                    |
|----------------------|--------------------------------------------------|-------------|-------------|------------------------------------------------|
| petamoriken/float16  | [github](https://github.com/petamoriken/float16) | MIT         | 3.8.6       | Fluent support for f16 numbers via TypedArrays |
