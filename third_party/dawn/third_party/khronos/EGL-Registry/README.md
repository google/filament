# EGL-Registry

The EGL-Registry repository contains the EGL API and Extension Registry,
including specifications, reference pages and reference cards, and the
enumerant registry. It is also used as a backing store for the web view of
the registry at https://www.khronos.org/registry/egl/ ; commits to the
master branch of this repository will be reflected there.

In the past, the EGL registry was maintained in a public Subversion
repository. The history in that repository has not been imported to github,
but it is still available at
https://cvs.khronos.org/svn/repos/registry/trunk/public/egl/ .

Interesting files in this repository include:

* index.php - toplevel index page for the web view. This relies on PHP
  include files found elsewhere on www.khronos.org and so is not very useful
  in isolation.
* registry.tcl - extension number registry. Documents the names and index
  numbers assigned to EGL extension specifications.
* api/egl.xml - extension enumerant and API registry. Defines the EGL API,
  including extensions, and is used to generate headers. Documents the EGL
  enumerant ranges assigned to different vendors.
* api/EGL/ and api/KHR/ - header files used by an EGL implementation.
  EGL/eglext.h and EGL/egl.h are generated from egl.xml. The other headers
  are handcoded and express OS and window system (platform) dependencies.
* extensions/ - EGL extension specifications, grouped into vendor-specific
  subdirectories.
* sdk/ - EGL reference pages and reference cards. There are separate sets
  for each API version.
* specs/ - EGL specification documents.

## Reserving EGL Enumerant Ranges

EGL enumerants are documented in api/egl.xml . New ranges can be allocated
by proposing a pull request to master modifying this file, following the
existing examples. Allocate ranges starting at the lowest free values
available (search for "Reservable for future use"). Ranges are not
officially allocated until your pull request is *accepted* into master. At
that point you can use values from your assigned range for API extensions.


## Adding Extension Specifications

Extension specification documents can be added by proposing a pull request
to master, adding the specification .txt file and related changes under
extensions/\<vendor\>/filename.txt. Your pull request must also:

* Allocate an extension number in registry.tcl (follow the existing
  ```<extension>``` examples, search for "Next free extension number", and use
  the lowest available extension number).
* Include that extension number in the extension specification document.
* Define the interfaces introduced by this extension in api/egl.xml,
  following the examples of existing extensions. If you have difficulty
  doing this, consult the registry schema documentation in the GL registry
  at www.khronos.org/registry/gl/; you may also create Issues in the
  EGL-Registry repository to request help.
* Verify that the EGL headers regenerate properly after applying your XML
  changes. In the api/ directory, you must be able to do the following without
  errors:
```
    # Validate XML changes
    make validate
    # Verify headers build and are legal C
    make clobber
    make
    make tests
```
* Finally, add a link from the extensions section of index.php to the
  extension document, using the specified extension number, so it shows up
  in the web view (this could in principle be generated automatically from
  registry.tcl / egl.xml, but isn't at present).

Sometimes extension text files contain inappropriate UTF-8 characters. They
should be restricted to the ASCII subset of UTF-8 at present. They can be
removed using the iconv Linux command-line tool via

    iconv -c -f utf-8 -t ascii filename.txt

(see internal Bugzilla issue 16141 for more).

We may transition to an asciidoc-based extension specification format at
some point.


## Build Tools

This section is not complete (see https://github.com/KhronosGroup/EGL-Registry/issues/92).

To validate the XML and build the headers you will need at least GNU make,
'jing' for the 'make validate' step (https://relaxng.org/jclark/jing.html),
and Python 3.5 and the lxml.etree Python library
(https://pypi.org/project/lxml/) for the 'make' step. The 'make tests' step
requires whatever the C and C++ compilers configured for GNU make are,
usually gcc and g++.

All of these components are available prepackaged for major Linux
distributions and for the Windows 10 Debian WSL.


