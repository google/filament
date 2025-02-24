# Contributing to **node-addon-api**

* [Code of Conduct](#code-of-conduct)
* [Developer's Certificate of Origin 1.1](#developers-certificate-of-origin)
* [Tests](#tests)
* [Debug](#debug)
* [Benchmarks](#benchmarks)
* [node-addon-api Contribution Philosophy](#node-addon-api-contribution-philosophy)

## Code of Conduct

The Node.js project has a
[Code of Conduct](https://github.com/nodejs/admin/blob/HEAD/CODE_OF_CONDUCT.md)
to which all contributors must adhere.

See [details on our policy on Code of Conduct](https://github.com/nodejs/node/blob/main/doc/contributing/code-of-conduct.md).

<a id="developers-certificate-of-origin"></a>

## Developer's Certificate of Origin 1.1

<pre>
By making a contribution to this project, I certify that:

 (a) The contribution was created in whole or in part by me and I
     have the right to submit it under the open source license
     indicated in the file; or

 (b) The contribution is based upon previous work that, to the best
     of my knowledge, is covered under an appropriate open source
     license and I have the right under that license to submit that
     work with modifications, whether created in whole or in part
     by me, under the same open source license (unless I am
     permitted to submit under a different license), as indicated
     in the file; or

 (c) The contribution was provided directly to me by some other
     person who certified (a), (b) or (c) and I have not modified
     it.

 (d) I understand and agree that this project and the contribution
     are public and that a record of the contribution (including all
     personal information I submit with it, including my sign-off) is
     maintained indefinitely and may be redistributed consistent with
     this project or the open source license(s) involved.
</pre>


## Tests

To run the **node-addon-api** tests do:

```
npm install
npm test
```

To avoid testing the deprecated portions of the API run
```
npm install
npm test --disable-deprecated
```

To run the tests targeting a specific version of Node-API run
```
npm install
export NAPI_VERSION=X
npm test --NAPI_VERSION=X
```

where X is the version of Node-API you want to target.

To run a specific unit test, filter conditions are available

**Example:**
  compile and run only tests on objectwrap.cc and objectwrap.js
  ```
    npm run unit --filter=objectwrap
  ```

Multiple unit tests cane be selected with wildcards

**Example:**
compile and run all test files ending with "reference" -> function_reference.cc, object_reference.cc, reference.cc
 ```
    npm run unit --filter=*reference
 ```

Multiple filter conditions can be joined to broaden the test selection

**Example:**
 compile and run all tests under folders threadsafe_function and typed_threadsafe_function and also the objectwrap.cc file
    npm run unit --filter='*function objectwrap'

## Debug

To run the **node-addon-api** tests with `--debug` option:

```
npm run-script dev
```

If you want a faster build, you might use the following option:

```
npm run-script dev:incremental
```

Take a look and get inspired by our **[test suite](https://github.com/nodejs/node-addon-api/tree/HEAD/test)**

## Benchmarks

You can run the available benchmarks using the following command:

```
npm run-script benchmark
```

See [benchmark/README.md](benchmark/README.md) for more details about running and adding benchmarks.

## **node-addon-api** Contribution Philosophy

The **node-addon-api** team loves contributions. There are many ways in which you can
contribute to **node-addon-api**:
- [New APIs](#new-apis)
- [Source code fixes](#source-changes)
- Additional tests
- Documentation improvements
- Joining the Node-API working group and participating in meetings

### New APIs

As new APIs are added to Node-API, node-addon-api must be updated to provide
wrappers for those new APIs. For this reason, node-addon-api provides
methods that allow callers to obtain the underlying Node-API handles so
direct calls to Node-API and the use of the objects/methods provided by
node-addon-api can be used together. For example, in order to be able
to use an API for which the node-addon-api does not yet provide a wrapper.

APIs exposed by node-addon-api are generally used to create and
manipulate JavaScript values. Concepts and operations generally map
to ideas specified in the **ECMA262 Language Specification**.

### Source changes

**node-addon-api** is meant to be a thin convenience wrapper around Node-API. With this
in mind, contributions of any new APIs that wrap around a core Node-API API will
be considered for merging. However, changes that wrap existing **node-addon-api**
APIs are encouraged to instead be provided as an ecosystem module. The
**node-addon-api** team is happy to link to a curated set of modules that build on
top of **node-addon-api** if they have broad usefulness to the community and promote
a recommended idiom or pattern.

### Rationale

The Node-API team considered a couple of different approaches with regard to changes
extending **node-addon-api**
- Larger core module - Incorporate these helpers and patterns into **node-addon-api**
- Extras package - Create a new package (strawman name '**node-addon-api**-extras')
that contain utility classes and methods that help promote good patterns and
idioms while writing native addons with **node-addon-api**.
- Ecosystem - Encourage the creation of a module ecosystem around **node-addon-api**
where folks can build on top of it.

#### Larger Core

This is probably our simplest option in terms of immediate action needed. It
would involve landing any open PRs against **node-addon-api**, and continuing to
encourage folks to make PRs for utility helpers against the same repository.

The downside of the approach is the following:
- Less coherency for our API set
- More maintenance burden on the Node-API WG core team.

#### Extras Package

This involves us spinning up a new package that contains the utility classes
and methods. This has the benefit of having a separate module where helpers
make it easier to implement certain patterns and idioms for native addons
easier.

The downside of this approach is the following:
- Potential for confusion - we'll need to provide clear documentation to help the
community understand where a particular contribution should be directed to (what
belongs in **node-addon-api** vs **node-addon-api-extras**)
- Need to define the level of support/API guarantees
- Unclear if the maintenance burden on the Node-API WG is reduced or not

#### Ecosystem

This doesn't require a ton of up-front work from the Node-API WG. Instead of
accepting utility PRs into **node-addon-api** or creating and maintaining a new
module, the WG will encourage the creation of an ecosystem of modules that
build on top of **node-addon-api**, and provide some level of advertising for these
modules (listing them out on the repository/wiki, using them in workshops/tutorials
etc).

The downside of this approach is the following:
- Potential for lack of visibility. Evangelism and education are hard, and module
authors might not find the right patterns and instead implement things themselves
- There might be greater friction for the Node-API WG in evolving APIs since the
ecosystem would have taken dependencies on the API shape of **node-addon-api**

