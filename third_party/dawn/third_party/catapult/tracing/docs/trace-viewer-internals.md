# TraceViewer’s Internals

## Module system

 * Tracing currently uses html imports for modules.
 * We aspire to one-class-per file. Feel free to break up big files as you
encounter them, they exist purely for legacy reasons.

## Tests

 * See unittest.html -- mostly compatible with closure tests
 * See [[/docs/dev-server-tests.md]] for more information

## Components

 * New UI elements should be Polymer components.
 * You will see some old references to tvcm.ui.define('x'). This is our old
approach for building components. Its like polymer, in that you can subclass
the element, but it doesn't use shadow dom or have any templating or data
binding.

## Rough module breakdown

 * *Importers:* load files, produce a model
 * *Model:* stateless, just the data that came from different trace formats
 * *TimelineTrackView:* shows the data in gantt-chart form
 * *Tracks:* visualize a particular part of the model
 * *Selection:* a vector of things in the tracks that the user has selected (counter samples, slices)
 * *Analysis:* provides summary of selection
 * *TimelineView:* glues everything together
 * *ProfilingView:* chrome-specific UI and glue

## Importer notes

 * The importer to model abstraction is meant to allow us to support multiple trace formats

## Model notes

 * The most important concept in the model is a slice. A slice is a range of time, and some metadata about that range, e.g. title, arguments, etc.
 * Model has
     * Processes
         * Counters
             * Counter samples (at ts=0.2s, we had 10mb allocated and 3mb free)
         * Threads
             * Slices (the FFT::compute function ran from 0.7s to 0.9s)
             * AsyncSlices (at 0.2s we started a file read in the background and it finished at 0.5s)
             * CpuSlices (at ts=0.2s we were running on cpu2)
         * CPUs
             * Slices (at ts=0.2 to 0.4 we were running "top")
             * Counters (the clock frequency was 1.2ghz at ts=0.1s)

## Slice
A slice is something which consumes time synchronously on a CPU or a thread. The
canonical example of this would be a B/E event pair. An async operation is also
considered a slice. Things get a bit more murky when looking at instant events.
A thread scoped instant event is a duration 0 slice. Other instant events,
process or global scoped, don't correlate to a thread or CPU and aren't
considered slices.

A flow event, on the other hand, is not a slice. It doesn't consume time and is,
conceptually, a graph of data flow in the system.


## Slice groups

 * When you see the tracing UI, you see lots of things like this:

```
Thread 7:     [  a     ]   [    c   ]
                        [ b ]
```

 * This of visualization starts as a *flat* array of slices:

```
 [{title: “a”, start: 0, end: 1), {title: “c”, start: 1.5, end: 3.5}, {title: “b”, start: 0.5, end: 0.8}]
```

 * We call this a slice group. A slice group can be composed into subRows -- a subRow is an array of slices that are all non-overlapping. E.g. in the thread7 example above, there are two subrows:

```
subrow 1:     [  a     ]   [    c   ]
subrow 2:     [ b ]
```

 * The SliceTrack is built around the idea of visualizing a single subrow. So when you see a thread like thread 7, you’re really looking at 2 !SliceTracks, each of which has its own subrow.

 * We have two slice group types:
     * SliceGroup, for nested data. Used for threads.
         * e.g.  like ( (0,2), (0.1,0.3) )
         * We convert the slices into subrows based on containment.
         * b is considered contained by a if b.start >= a.start && b.end <= a.end
     * AsyncSliceGroup, for overlapping data. Used for async operations.
         * e.g. ( (0, 2), (1, 3) )
         * We convert the slices into subrows by greedily packing them into rows, adding rows as needed when there’s no room on an existing subrow

## Timeline notes

 * Timeline is just an array of tracks. A track is one of the rows in the UI. A single thread of data may turn into 5+ tracks, one track for each row of squares.
 * The core of the Timeline is Slice
 * Panning/zooming state is on the TimelineViewport, as is the grid and user defined markers

## Tracks
### there are three broad types of tracks

 * Building blocks
     * *Container track*
         * A track that is itself made up of more tracks. Just a div plus logic to delegate overall track interface calls down to its children.
     * *CanvasBasedTrack*
         * A track that renders its content using HTML5 canvas
 * Visualizations
     * *SliceTrack:* visualizes an array of non-overlapping monotonically-increasing slices. Has some simple but critical logic to efficiently render even with thousands (or more) slices by merging small slices together when they really close together.
     * *CounterTrack:* visualizes an array of samples values over time. Has support for stacked area charts. Tries to merge samples together when they are not perceptually significant to reduce canvas drawing overhead.
     * *Model tracks:* e.g. ThreadTrack
         * Derives from a container track, takes a timeline model object, e.g. a thread, and creates the appropriate child tracks that visualize that thread

## Selection notes

 * When you drag to select, that creates a selection object by asking every track to append things that intersect the dragged-box to the selection
 * A selection object is an array of hits.
 * A hit is the pairing of the track-level entity that was selected with the model-level entity that that visual thing represents.
 * Eg a thread has a bunch of slices in it. That gets turned into a bunch of subrows that we then turn into !SliceTracks. When you click on a slice in a thread in the UI, the hit is {slice: <the slice you clicked>, thread: <the thread it came from>}.
 * The hit concept exists partly because slices can’t know their parent (see model section for why). Yet, analysis code want to know parentage in order to do things like group-by-thread.

## Analysis code

 * Takes as input a selection
 * Does the numeric analysis and dumps the numeric results to a builder
 * The builder is responsible for creating HTML (or any other textual representation of the results)
