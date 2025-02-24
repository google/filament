Though there are some concepts hold the same across all trace formats we've encountered, there are an always plenty of domain-specific details to a given expertise area that defy standard treatment.

In trace-viewer, we distinguish between "core" pieces, which are domain-neutral and belong in `trace_viewer/core` and domain-specific pieces, which we are in `trace_viewer/extras`. As such, core/ has a variety of extension points that then extras/ pulls in.

# Importers
TraceViewer is not tied to one specific trace file format: everyone has their own ideal way for getting performance data, storing it, and eventually getting it into the HTML file for viewing. And, since trace-viewer tries to be able to view traces from multiple systems all together, it may not even be possible to get traces into a single file format. Thats fine, as we see it.

The main unit of extension here is the Importer object, `core/importer/importer.html`. To teach trace viewer about a new file format, subclass that importer, then hook it up to `default_importers.html`. Voila, you have the beginnings

When you call TraceModel.import, you pass array of objects. We then run over this array one at a time, then walk through the registered importers looking for one that `.canHandle` that trace. Once it is found, we assign that trace to the importer.

Because some trace formats are container formats, we support sub-traces, where an importer does a bit of processing, then yields another trace that needs more importing. This is, for instance, how we import gzip files.

# Slice Views
The display and storage of slices can be overridden based on their model-level name and category. This allows domain specific customization of that particular type of data. Some keywords to search for are  SliceView.register and AsyncSlice.register.

One way this is used is to customize the display title of a slice. In the trace files and the model, slices with the "net" category are traced with titles that correspond to their probe point. And, the URL of a request is just one of many events in the trace that is discovered quite late in the overall sequence of events. But, when viewing a network trace, the most interesting thing to see is the URL for which a traces corresponds. This transformation is accomplished by registering a custom net async slice, which overrides the `displayTitle` property: this leaves the model in-tact [e.g. exactly as it was traced] but improves on the display.

# Object Views and Types
In Chrome, some of our traces have a complex and massive JSON dump from our graphics subsystem that, when
interpreted exactly the right way, let us reconstruct a view of the page just from the trace.

There are two extension points that make this possible:
- We allow subtypes to be registered for ObjectSnapshots and ObjectInstances. This way you can build up a domain-specific model of the trace instead of having to parse the trace yourself after the fact. See `extras/cc/layer_tree_host_impl.html` for an example.

- We allow custom viewer objects to be registered for Snapshots and Instances. When a user clicks on one, we look for a viewer and use that object instead. See `extras/cc/layer_tree_host_impl_view.html` as an example.
