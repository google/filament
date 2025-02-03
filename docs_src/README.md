# Documentation

Filament's documentation (which you are reading) is a collection of pages created with [`mdBook`].

## How the book is created and updated {#how-to-create}
### Prerequisites
 - Install [`mdBook`] for your platform
 - `selenium` package for python
   ```shell
   python3 -m pip install selenium
   ```

### Generate  {#how-to-generate}
We wrote a python script to gather and transform the different documents in the project tree into a
single book. This script can be found in [`docs_src/build/run.py`]. In addition,
[`docs_src/build/duplicates.json`] is used to describe the markdown files that are copied and
transformed from the source tree. These copies are placed into `docs_src/src/dup`.

To collect the pages and generate the book, run the following
```shell
cd docs_src
python3 build/run.py
```

### Copy to `docs`
`docs` is the github-specfic directory for producing a web frontend (i.e. documentation) for a
project.

(To be completed)

## Document sources
We list the different document sources and how they are copied and processed into the collection
of markdown files that are then processed with `mdBook`.

### Introductory docs {#introductory-doc}
The [github landing page] for Filament displays an extensive introduction to Filament. It
links to `BUILDING.md` and `CONTRIBUTING.md`, which are conventional pages for building or
contributing to the project. We copy these pages from their respective locations in the project
tree into `docs_src/src/dup`. Moreover, to restore valid linkage between the pages, we need
to perform a number of URL replacements in addition to the copy. These replacements are
described in [`docs_src/build/duplicates.json`].

### Core concept docs
The primary design of Filament as a phyiscally-based renderer and details of its materials
system are described in `Filament.md.html` and `Materials.md.html`, respectively. These two
documents are written in [`markdeep`]. To embed them into our book, we
 1. Convert the markdeep into html
 2. Embed the html output in a markdown file
 3. Place the markdown file in `docs_src/src/main`

We describe step 1 in detail for the sake of record:
 - Start a local-only server to serve the markdeep file (e.g. `Filament.md.html`)
 - Start a `selenium` driver (essentially run chromium in headless mode)
 - Visit the local page through the driver (i.e. open url `http://localhost:xx/Filament.md.html?export`)
 - Parse out the exported output in the retrieved html (note that the output of the markdeep
   export is an html with the output captured in a `<pre>` tag).
 - Replace css styling in the exported output as needed (so they don't interfere with the book's css.
 - Replace resource urls to refer to locations relative to the mdbook structure.

### READMEs
Filament depends on a number of libraries, which reside in the directory `libs`. These individual
libaries often have README.md in their root to describe itself. We collect these descriptions into our
book. In addition, client usage of Filament also requires using a set of binary tools, which are
located in `tools`. Some of tools also have README.md as description. We also collect them into the book.

The process for copying and processing these READMEs is outlined in [Introductory docs](#introductory-doc).

### Other technical notes
These are technical documents that do not fit into a library, tool, or directory of the
Filament source tree. We collect them into the `docs_src/src/notes` directory. No additional
processing are needed for these documents.

## Adding more documents
To add any documentation, first consider the type of the document you like to add. If it
belongs to any of the above sources, then simply place the document in the appropriate place,
add a link in `SUMMARY.md`, and perform the steps outlined in
[how-to create section](#how-to-create).

For example, if you are adding a general technical note, then you would
 - Place the document (file with extension `.md`) in `docs_src/src/notes`
 - Add a link in [`docs_src/src/SUMMARY.md`]
 - Run the commands in the [Generate](#how-to-generate) section

[github landing page]: https://google.github.io/filament
[`mdBook`]: https://rust-lang.github.io/mdBook/
[`markdeep`]: https://casual-effects.com/markdeep/
[`docs_src/build/run.py`]: https://github.com/google/filament/blob/main/docs_src/build/run.py
[`docs_src/build/duplicates.json`]: https://github.com/google/filament/blob/main/docs_src/build/duplicates.json
[`docs_src/src/SUMMARY.md`]: https://github.com/google/filament/blob/main/docs_src/src/SUMMARY.md
