
<!---

This README is automatically generated from the comments in these files:
iron-ajax.html  iron-request.html

Edit those files, and our readme bot will duplicate them over here!
Edit this file, and the bot will squash your changes :)

The bot does some handling of markdown. Please file a bug if it does the wrong
thing! https://github.com/PolymerLabs/tedium/issues

-->

[![Build status](https://travis-ci.org/PolymerElements/iron-ajax.svg?branch=master)](https://travis-ci.org/PolymerElements/iron-ajax)

_[Demo and API docs](https://elements.polymer-project.org/elements/iron-ajax)_

## Changes in 2.0

*   Promise polyfill is now a dev dependency and no longer shipped with `iron-ajax`.

    `iron-ajax` uses the `Promise` API, which is not yet supported in all browsers.

    The 1.x version of `iron-ajax` automatically loaded the promise polyfill. This 
    forced the application to include the polyfill, whether or not it was needed.

    When using `iron-ajax` 2.x with Polymer 1.x, you must provide your own Promise polyfill, 
    if needed. For example, you could use the promise polyfill by installing it in your project:

        bower install --save PolymerLabs/promise-polyfill#1 - 2

    Then your app should include the promise polyfill before loading `iron-ajax`:

        <link rel="import" href="bower_components/promise-polyfill/promise-polyfill-lite.html">

    You can use a different promise polyfill if you need a more fully-featured implementation of 
    Promise.

    For Polymer 2.x, you **do not need to provide your own Promise polyfill if  you are using
    the web components polyfills.** Because the web components v1 APIs depend on `Promise`, 
    a promise polyfill is loaded when needed by the v1 polyfills (`web-components-lite.js` or 
    `webcomponents-loader.js`).

*   New optional error information.

    The `generateRequest` method returns an `iron-request` element representing the 
    request, and the request element provides a `completes` property, which is a 
    promise that completes when the request either succeeds or fails.

    This version includes a new flag, `rejectWithRequest`, that modifies the error handling
    of the `completes` promise. By default, when the promise is rejected (because the request 
    failed), the rejection callback only receives an `Error` object describing the failure.

    With `rejectWithRequest` set to true, the callback receives an object with two keys, `error`, 
    the error message, and `request`, the original request that the error is related to:

        let request = ironAjaxElement.generateRequest();
        request.completes.then(function(req) {
            // succesful request, argument is iron-request element
            ...
          }, function(rejected) {
            // failed request, argument is an object
            let req = rejected.request;
            let error = rejected.error;
            ...
          }
        )

    Because this change could break existing code, `rejectWithRequest` is false by default,
    however, in the next major release, this option will be removed and the new behavior made 
    the default.


## &lt;iron-ajax&gt;

The `iron-ajax` element exposes network request functionality.

```html
<iron-ajax
    auto
    url="https://www.googleapis.com/youtube/v3/search"
    params='{"part":"snippet", "q":"polymer", "key": "YOUTUBE_API_KEY", "type": "video"}'
    handle-as="json"
    on-response="handleResponse"
    debounce-duration="300"></iron-ajax>
```

With `auto` set to `true`, the element performs a request whenever
its `url`, `params` or `body` properties are changed. Automatically generated
requests will be debounced in the case that multiple attributes are changed
sequentially.

Note: The `params` attribute must be double quoted JSON.

You can trigger a request explicitly by calling `generateRequest` on the
element.



## &lt;iron-request&gt;

iron-request can be used to perform XMLHttpRequests.

```html
<iron-request id="xhr"></iron-request>
...
this.$.xhr.send({url: url, body: params});
```


