# Making standalone HTML files

If you have a trace file that you want to turn into a html file with a viewer then:

```
sys.path.append(os.path.join(path_to_catapult, 'tracing'))
from trace_viewer_build import trace2html
with open('my_trace.html', 'w') as new_file:
   trace2html.WriteHTMLForTracesToFile(['my_trace.json'], new_file)
```

This will produce a standalone trace viewer with my_trace packed inside.

# Embedding the Easy Way
Running `$CATAPULT/tracing/bin/vulcanize_trace_viewer` will create `$CATAPULT/tracing/bin/trace_viewer_full.html`. That file has all the js, css and html-templates that you need for a standalone trace viewer instance.

In your code, `<link rel="import" href="trace_viewer_full.html">`. Then, to get a trace viewer up, you need to do two things: make the timeline viewer, and make a model and give it to the viewer:
```
    var container = document.createElement('track-view-container');
    container.id = 'track_view_container';

    viewer = document.createElement('tr-ui-timeline-view');
    viewer.track_view_container = container;
    Polymer.dom(viewer).appendChild(container);

    viewer.id = 'trace-viewer';
    viewer.globalMode = true;
    Polymer.dom(document.body).appendChild(viewer);
```

With the viewer created, you need to then make a TraceModel:
```
    var model = new tr.Model();
    var i = new tr.importer.Import(m);
    var p = i.importTracesWithProgressDialog([result]);
    p.then(function() {
      viewer.model = model;
    }, onImportFail);

```

Model has a variety of import options, from synchronous import to importWithProgressDialog. And, it
lets you customize the types of postprocessing to be done on the model before it is displayed by the view.

# Configs
Trace viewer has a lot of extra pieces, for domain-specific use cases. By default, trace2html and vulcanize take everything and combine them together. We call this the "full" config. Passing --help to
vulcanize or trace2html will show the current set of configs we support, which maps to
`trace_viewer/extras/*_config.html`. Some of the other configs are smaller, leading to a more compact redistributable.

# Customizing
For more information on how to customize and extend trace viewer, see [Extending-and-Customizing-Trace-Viewer](Extending-and-Customizing-Trace-Viewer)

# Example
See tracing_examples/trace_viewer_embedder.html for an example of using the
embedding system. Note that you have to include the WebComponentsV0 polyfill
(see https://crbug.com/1036492) unless you have an origin trial token.
