
<!---

This README is automatically generated from the comments in these files:
iron-icons.html

Edit those files, and our readme bot will duplicate them over here!
Edit this file, and the bot will squash your changes :)

The bot does some handling of markdown. Please file a bug if it does the wrong
thing! https://github.com/PolymerLabs/tedium/issues

-->

[![Build status](https://travis-ci.org/PolymerElements/iron-icons.svg?branch=master)](https://travis-ci.org/PolymerElements/iron-icons)

_[Demo and API docs](https://elements.polymer-project.org/elements/iron-icons)_


##&lt;iron-icons&gt;

`iron-icons` is a utility import that includes the definition for the `iron-icon` element, `iron-iconset-svg` element, as well as an import for the default icon set.

The `iron-icons` directory also includes imports for additional icon sets that can be loaded into your project.

Example loading icon set:

```html
<link rel="import" href="../iron-icons/maps-icons.html">
```

To use an icon from one of these sets, first prefix your `iron-icon` with the icon set name, followed by a colon, ":", and then the icon id.

Example using the directions-bus icon from the maps icon set:

```html
<iron-icon icon="maps:directions-bus"></iron-icon>
```

To load a subset of icons from one of the default `iron-icons` sets, you can
use the [poly-icon](https://poly-icon.appspot.com/) tool. It allows you
to select individual icons, and creates an iconset from them that you can
use directly in your elements.

See [iron-icon](./iron-icon) for more information about working with icons.

See [iron-iconset](./iron-iconset) and [iron-iconset-svg](./iron-iconset-svg) for more information about how to create a custom iconset.


