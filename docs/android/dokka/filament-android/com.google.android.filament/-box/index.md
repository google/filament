//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Box](index.md)

# Box

[main]\
open class [Box](index.md)

An axis aligned 3D box represented by its center and half-extent.

## Constructors

| | |
|---|---|
| [Box](-box.md) | [main]<br>constructor()constructor(centerX: Float, centerY: Float, centerZ: Float, halfExtentX: Float, halfExtentY: Float, halfExtentZ: Float)constructor(center: Array&lt;Float&gt;, halfExtent: Array&lt;Float&gt;) |

## Functions

| Name | Summary |
|---|---|
| [getBoundingSphere](get-bounding-sphere.md) | [main]<br>open fun [getBoundingSphere](get-bounding-sphere.md)(out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Computes the smallest bounding sphere of the box. |
| [getCenter](get-center.md) | [main]<br>open fun [getCenter](get-center.md)(): Array&lt;Float&gt;<br>open fun [getCenter](get-center.md)(out: Array&lt;Float&gt;): Array&lt;Float&gt; |
| [getCenterX](get-center-x.md) | [main]<br>open fun [getCenterX](get-center-x.md)(): Float |
| [getCenterY](get-center-y.md) | [main]<br>open fun [getCenterY](get-center-y.md)(): Float |
| [getCenterZ](get-center-z.md) | [main]<br>open fun [getCenterZ](get-center-z.md)(): Float |
| [getHalfExtent](get-half-extent.md) | [main]<br>open fun [getHalfExtent](get-half-extent.md)(): Array&lt;Float&gt;<br>open fun [getHalfExtent](get-half-extent.md)(out: Array&lt;Float&gt;): Array&lt;Float&gt; |
| [getHalfExtentX](get-half-extent-x.md) | [main]<br>open fun [getHalfExtentX](get-half-extent-x.md)(): Float |
| [getHalfExtentY](get-half-extent-y.md) | [main]<br>open fun [getHalfExtentY](get-half-extent-y.md)(): Float |
| [getHalfExtentZ](get-half-extent-z.md) | [main]<br>open fun [getHalfExtentZ](get-half-extent-z.md)(): Float |
| [getMax](get-max.md) | [main]<br>open fun [getMax](get-max.md)(out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Computes the largest coordinates corner of the box. |
| [getMin](get-min.md) | [main]<br>open fun [getMin](get-min.md)(out: Array&lt;Float&gt;): Array&lt;Float&gt;<br>Computes the lowest coordinates corner of the box. |
| [isEmpty](is-empty.md) | [main]<br>open fun [isEmpty](is-empty.md)(): Boolean<br>Whether the box is empty, i.e.: its extents are zero. |
| [set](set.md) | [main]<br>open fun [set](set.md)(min: Array&lt;Float&gt;, max: Array&lt;Float&gt;): [Box](index.md)<br>open fun [set](set.md)(minx: Float, miny: Float, minz: Float, maxx: Float, maxy: Float, maxz: Float): [Box](index.md)<br>Initializes the 3D box from its min / max coordinates on each axis |
| [setCenter](set-center.md) | [main]<br>open fun [setCenter](set-center.md)(center: Array&lt;Float&gt;)<br>open fun [setCenter](set-center.md)(centerX: Float, centerY: Float, centerZ: Float) |
| [setCenterX](set-center-x.md) | [main]<br>open fun [setCenterX](set-center-x.md)(centerX: Float) |
| [setCenterY](set-center-y.md) | [main]<br>open fun [setCenterY](set-center-y.md)(centerY: Float) |
| [setCenterZ](set-center-z.md) | [main]<br>open fun [setCenterZ](set-center-z.md)(centerZ: Float) |
| [setHalfExtent](set-half-extent.md) | [main]<br>open fun [setHalfExtent](set-half-extent.md)(halfExtent: Array&lt;Float&gt;)<br>open fun [setHalfExtent](set-half-extent.md)(halfExtentX: Float, halfExtentY: Float, halfExtentZ: Float) |
| [setHalfExtentX](set-half-extent-x.md) | [main]<br>open fun [setHalfExtentX](set-half-extent-x.md)(halfExtentX: Float) |
| [setHalfExtentY](set-half-extent-y.md) | [main]<br>open fun [setHalfExtentY](set-half-extent-y.md)(halfExtentY: Float) |
| [setHalfExtentZ](set-half-extent-z.md) | [main]<br>open fun [setHalfExtentZ](set-half-extent-z.md)(halfExtentZ: Float) |
| [transform](transform.md) | [main]<br>open fun [transform](transform.md)(m: Array&lt;Float&gt;, t: Array&lt;Float&gt;, box: [Box](index.md)): [Box](index.md)<br>open fun [transform](transform.md)(m: Array&lt;Float&gt;, tx: Float, ty: Float, tz: Float, box: [Box](index.md)): [Box](index.md)<br>Transform a Box by a linear transform and a translation. |
| [translateTo](translate-to.md) | [main]<br>open fun [translateTo](translate-to.md)(tr: Array&lt;Float&gt;): [Box](index.md)<br>open fun [translateTo](translate-to.md)(trx: Float, try_: Float, trz: Float): [Box](index.md)<br>Translates the box *to* a given center position |
| [unionSelf](union-self.md) | [main]<br>open fun [unionSelf](union-self.md)(box: [Box](index.md)): [Box](index.md)<br>Computes the bounding box of the union of two boxes |
