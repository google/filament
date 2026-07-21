//[filament-android](../../../index.md)/[com.google.android.filament.proguard](../index.md)/[UsedByNative](index.md)

# UsedByNative

[main]\
@[Target](https://developer.android.com/reference/kotlin/java/lang/annotation/Target.html)(value = [[ElementType.METHOD](https://developer.android.com/reference/kotlin/java/lang/annotation/ElementType.html), [ElementType.FIELD](https://developer.android.com/reference/kotlin/java/lang/annotation/ElementType.html), [ElementType.TYPE](https://developer.android.com/reference/kotlin/java/lang/annotation/ElementType.html), [ElementType.CONSTRUCTOR](https://developer.android.com/reference/kotlin/java/lang/annotation/ElementType.html)])

annotation class [UsedByNative](index.md)

Annotation used for marking methods and fields that are called from native code. Useful for keeping components that would otherwise be removed by Proguard. Use the value parameter to mention a file that calls this method. Note that adding this annotation to a method is not enough to guarantee that it is kept - either its class must be referenced elsewhere in the program, or the class must be annotated with this as well. Usage example: 

```kotlin

@UsedByNative("NativeCrashHandler.cpp")
public static void reportCrash(int signal, int code, int address) {
    ...
}

```

## Functions

| Name | Summary |
|---|---|
| [annotationType](../-used-by-reflection/index.md#-2087345088%2FFunctions%2F-214843558) | [main]<br>abstract fun [annotationType](../-used-by-reflection/index.md#-2087345088%2FFunctions%2F-214843558)(): [Class](https://developer.android.com/reference/kotlin/java/lang/Class.html)&lt;out [Annotation](https://developer.android.com/reference/kotlin/java/lang/annotation/Annotation.html)&gt; |
| [equals](../-used-by-reflection/index.md#-1297283241%2FFunctions%2F-214843558) | [main]<br>abstract fun [equals](../-used-by-reflection/index.md#-1297283241%2FFunctions%2F-214843558)(p: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [hashCode](../-used-by-reflection/index.md#-867487634%2FFunctions%2F-214843558) | [main]<br>abstract fun [hashCode](../-used-by-reflection/index.md#-867487634%2FFunctions%2F-214843558)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [toString](../-used-by-reflection/index.md#-1045653699%2FFunctions%2F-214843558) | [main]<br>abstract fun [toString](../-used-by-reflection/index.md#-1045653699%2FFunctions%2F-214843558)(): [String](https://developer.android.com/reference/kotlin/java/lang/String.html) |
| [value](value.md) | [main]<br>abstract fun [value](value.md)(): [String](https://developer.android.com/reference/kotlin/java/lang/String.html) |
