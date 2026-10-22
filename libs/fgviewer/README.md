# fgviewer

1. [Capabilities](#capabilities)
2. [Setup for Desktop](#setup-for-desktop)
3. [Setup for Android](#setup-for-android)
4. [Debugger Usage](#debugger-usage)
5. [Architecture Overview](#architecture-overview)
6. [C++ Server](#c-server)
7. [JavaScript Client](#javascript-client)
8. [HTTP Requests](#http-requests)
9. [Wish List](#wish-list)

## Capabilities

fgviewer is a library and web application for real-time visualization of the frame graph in Filament.
It displays active passes and resource usage, providing insights into the rendering pipeline.

## Setup for Desktop

When using the easy build script, include the `-t` argument. For example:

    ./build.sh -ft debug gltf_viewer

The `t` enables a CMake option called `FILAMENT_ENABLE_FGVIEWER` and the `f` ensures that CMake gets
re-run so that the option is honored.

Next, set an environment variable as follows. In Windows, use `set` instead of `export`.

    export FILAMENT_FGVIEWER_PORT=8050

Next, launch any app that links against a debug build of a Filament and point your web browser to
http://localhost:8050. Skip ahead to **Debugger Usage**.

## Setup for Android

Rebuild Filament for Android after enabling a CMake option called `FILAMENT_ENABLE_FGVIEWER`. Note that
CMake is invoked from several places for Android (both gradle and our easy build script), so one
pragmatic and reliable way of doing this is to simply hack `CMakeLists.txt` and
`filament-android/CMakeLists.txt` by unconditionally setting `FILAMENT_ENABLE_FGVIEWER` to `ON`.

After rebuilding Filament with the option enabled, ensure that internet permissions are enabled in
your app by adding the following into your manifest as a child of the `<manifest>` element.

    <uses-permission android:name="android.permission.INTERNET" />

Now launch your app as usual. The Filament Engine sets up a server that is hardcoded to listen to
port `8085`. Next, you will need to forward your device's TCP port `8085` to your host port of choice.
For example, to forward the fgviewer server on your device to port `8085` on your host machine, do the
following:

    adb forward tcp:8085 tcp:8085

This lets you go to http://localhost:8085 in Chrome on your host machine.

## Debugger Usage

After opening the fgviewer page in your browser, you can see the active views are on the left panel.
Then you can select any of them to see the active passes and resources for that view.

<p align="center">
  <img width="600px" src=https://github.com/user-attachments/assets/2d31767f-fc25-4f17-8c14-528fe5c6b698>
</p>

## Architecture Overview

<p align="center">
  <img width="450" src=https://github.com/user-attachments/assets/537ebb89-6ad0-4b93-bbeb-207d4fe9ec5a>
</p>

The fgviewer library has two parts: a C++ server and a JavaScript client. The C++ server is
responsible for instancing a [civetweb][1] context that handles HTTP requests. The
JavaScript client is a small web app that contains a view into an in-browser database of framegraphs.

When a new connection is established, the client asks the server for a list of framegraphs
in order to populate its in-browser database. If the connection is lost (e.g. if the app crashes),
then the database stays intact and the web app is still functional.

## C++ Server

The civetweb server is wrapped by our `DebugServer` class, which provides a public interface consisting of
a few methods invoked by the Filament engine.

Since each view corresponds to a frame graph, the engine should notify `DebugServer` of any changes to the views
on the engine side.

- **createView** Notifies the debugger that a new view has been created.
- **updateView** Notifies the debugger of updates to an existing view.
- **destroyView** Notifies the debugger that a view is being removed.

## JavaScript Client

The web app is built using LitElement, a lightweight library for creating Web Components. Our goal is to keep the code simple and modern, avoiding frameworks like React or Angular.

The app presents a view over a pseudo-database, which is essentially a global variable holding a dictionary that maps frame graph ids to objects following the JSON structure described below.

## HTTP requests

The server responds to the following GET requests by returning a JSON blob. The `{id}` in these
requests is a concept specific to fgviewer (not Filament) which is an 8-digit hex string for identifying frame graphs.

---

`/api/framegraphs`

Returns an array containing all framegraphs in an app. Example:

```json
[{
    "fgid": "00000000",
    "viewName": "Main View",
    "passes": [{
        "name": "shadow pass",
        "reads": [],
        "writes": ["0"]
    }],
    "resources": [{
        "id": "0",
        "name": "shadowmap",
        "properties": [{"resolution": "256x256"}, {"is_subresource": "false"}]
    }]
},
{
    "fgid": "00000001",
    "viewName": "UI view",
    ...
}]
```

---

`/api/framegraph?fg={id}`

Returns a specific framegraph info. Example:

```json
{
  "fgid": "00000000",
  "viewName": "Main View",
  "passes": [{
    "name": "shadow pass",
    "reads": [],
    "writes": ["0"]
  }],
  "resources": [{
    "id": "0",
    "name": "shadowmap",
    "properties": [{"resolution": "256x256"}, {"is_subresource": "false"}]
  }]
}
```

---

`/api/status`

Returns one of the below:
- `0`: first time connected
- `1`: no-op
- `{fgid}`: the corresponding frame graph has an update
    - Then the web view can request for the actual info using the previous api

If the request gets timeout, the web page show disconnected to the user.

---

## Wish List
- Display the texture contents on the webview

[1]: https://github.com/civetweb/civetweb