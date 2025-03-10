    Intel's static_vertex_array and vertex_array_set specifications are
    proposals for new functionality in OpenGL 1.2 or beyond. They are not
    current implemented by anyone that we know of (although SGI supports
    compiled_vertex_array, which is very close to static_vertex_array).

    There are two proposed versions of vertex_array_set, one of which names
    the set with integers, the other (in vertex_array_set.alt.spec) with
    pointers.

    Jon Leech

-------------------------------------------
Comments from Bimal Poddar of Intel follow:
-------------------------------------------

I have created this extension only for the purpose of inclusion
in OpenGL 1.2. If it does not go in OpenGL 1.2, this does not
exist.

Firstly, everybody had a problem with the compiled_vertex_array
name. Hence, I renamed it to static_vertex_array.spec.

Secondly, keeping in mind that the goal is to get this extension
into OpenGL 1.2 irrespective of VertexArraySet, I have made minor
tweaks to the original extension. One tweak is to disallow the
change of array pointers and enables. This is consistent with
what we talked about at the ARB meeting. The second tweak is
to remove the "first" since there seemed to be a consensus to
do that.

If VertexArraySet does get into OpenGL 1.2, then it might be nice
to have LockArraysExt and UnlockArraysExt accept the ArraySet name
as input parameters. But the whole package works with or without
that change.

Finally, another point to ponder - it was clear at the ARB meeting
that this extension did not serve the purpose of explicity signaling
to the implementation that it should go ahead and pre-compute a
range of indices. I have a radical thought here. I would suggest
eliminating DrawElementsRange and instead use the following commands:

	  DefineElementRange (first, count)
	  UndefineElementRange (void)

This would not only serve Drew's purpose of providing an explicit
hint about requesting the geometry pipeline to pre-compute the
data, but it would also add the range element functionality with
a very good potential of reuse across multiple DrawElement()
calls. The anticipated usage of these would be:

	DefineElementRange (first, count);
	   DrawElements();
	   DrawElements();
	   DrawElements();
	   DrawElements();
	UndefineElementRange ();

I am hoping that somebody creative out there could come up with a
better name than Define/Undefine.

To summarize, if you have problems/ideas with the following list,
please let me know as soon as possible:

1. Restriction on not allowing pointer change and enable change for
   vertex array state within lock/unlock.

2. Elimination of "first".

3. Should Lock/Unlock take the VertexArraySet name?

4. Point to ponder about DefineElementRange.

Thanks

Bimal
