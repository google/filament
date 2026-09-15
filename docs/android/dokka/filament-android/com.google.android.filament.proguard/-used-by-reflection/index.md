//[filament-android](../../../index.md)/[com.google.android.filament.proguard](../index.md)/[UsedByReflection](index.md)

# UsedByReflection

[main]\
@[Target](https://developer.android.com/reference/kotlin/java/lang/annotation/Target.html)(value = [[ElementType.METHOD](https://developer.android.com/reference/kotlin/java/lang/annotation/ElementType.html), [ElementType.FIELD](https://developer.android.com/reference/kotlin/java/lang/annotation/ElementType.html), [ElementType.TYPE](https://developer.android.com/reference/kotlin/java/lang/annotation/ElementType.html), [ElementType.CONSTRUCTOR](https://developer.android.com/reference/kotlin/java/lang/annotation/ElementType.html)])

annotation class [UsedByReflection](index.md)

Annotation used for marking methods and fields that are called by reflection. Useful for keeping components that would otherwise be removed by Proguard. Use the value parameter to mention a file that calls this method. Note that adding this annotation to a method is not enough to guarantee that it is kept - either its class must be referenced elsewhere in the program, or the class must be annotated with this as well. Usage example: 

```kotlin

@UsedByReflection("PeopleListItemView.java")
public PeopleListItemViewV11(Context context) {
    super(context);
}

```

## Functions

| Name | Summary |
|---|---|
| [annotationType](index.md#-2087345088%2FFunctions%2F-214843558) | [main]<br>abstract fun [annotationType](index.md#-2087345088%2FFunctions%2F-214843558)(): [Class](https://developer.android.com/reference/kotlin/java/lang/Class.html)&lt;out [Annotation](https://developer.android.com/reference/kotlin/java/lang/annotation/Annotation.html)&gt; |
| [equals](index.md#-1297283241%2FFunctions%2F-214843558) | [main]<br>abstract fun [equals](index.md#-1297283241%2FFunctions%2F-214843558)(p: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [hashCode](index.md#-867487634%2FFunctions%2F-214843558) | [main]<br>abstract fun [hashCode](index.md#-867487634%2FFunctions%2F-214843558)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [toString](index.md#-1045653699%2FFunctions%2F-214843558) | [main]<br>abstract fun [toString](index.md#-1045653699%2FFunctions%2F-214843558)(): [String](https://developer.android.com/reference/kotlin/java/lang/String.html) |
| [value](value.md) | [main]<br>abstract fun [value](value.md)(): [String](https://developer.android.com/reference/kotlin/java/lang/String.html) |
