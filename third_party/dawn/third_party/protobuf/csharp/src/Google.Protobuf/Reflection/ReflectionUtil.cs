#region Copyright notice and license
// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd
#endregion

using System;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.CompilerServices;
using Google.Protobuf.Compatibility;

namespace Google.Protobuf.Reflection {
  /// <summary>
  /// The methods in this class are somewhat evil, and should not be tampered with lightly.
  /// Basically they allow the creation of relatively weakly typed delegates from MethodInfos
  /// which are more strongly typed. They do this by creating an appropriate strongly typed
  /// delegate from the MethodInfo, and then calling that within an anonymous method.
  /// Mind-bending stuff (at least to your humble narrator) but the resulting delegates are
  /// very fast compared with calling Invoke later on.
  /// </summary>
  internal static class ReflectionUtil {
    static ReflectionUtil() {
      ForceInitialize<string>();  // Handles all reference types
      ForceInitialize<int>();
      ForceInitialize<long>();
      ForceInitialize<uint>();
      ForceInitialize<ulong>();
      ForceInitialize<float>();
      ForceInitialize<double>();
      ForceInitialize<bool>();
      ForceInitialize < int    ? > ();
      ForceInitialize < long   ? > ();
      ForceInitialize < uint   ? > ();
      ForceInitialize < ulong  ? > ();
      ForceInitialize < float  ? > ();
      ForceInitialize < double ? > ();
      ForceInitialize < bool   ? > ();
      ForceInitialize<SampleEnum>();
      SampleEnumMethod();
    }

    internal static void ForceInitialize<T>() => new ReflectionHelper<IMessage, T>();

    /// <summary>
    /// Empty Type[] used when calling GetProperty to force property instead of indexer fetching.
    /// </summary>
    internal static readonly Type[] EmptyTypes = new Type[0];

    /// <summary>
    /// Creates a delegate which will cast the argument to the type that declares the method,
    /// call the method on it, then convert the result to object.
    /// </summary>
    /// <param name="method">The method to create a delegate for, which must be declared in an
    /// IMessage implementation.</param>
    internal static Func<IMessage, object> CreateFuncIMessageObject(MethodInfo method) =>
        GetReflectionHelper(method.DeclaringType, method.ReturnType)
            .CreateFuncIMessageObject(method);

    /// <summary>
    /// Creates a delegate which will cast the argument to the type that declares the method,
    /// call the method on it, then convert the result to the specified type. The method is expected
    /// to actually return an enum (because of where we're calling it - for oneof cases). Sometimes
    /// that means we need some extra work to perform conversions.
    /// </summary>
    /// <param name="method">The method to create a delegate for, which must be declared in an
    /// IMessage implementation.</param>
    internal static Func<IMessage, int> CreateFuncIMessageInt32(MethodInfo method) =>
        GetReflectionHelper(method.DeclaringType, method.ReturnType)
            .CreateFuncIMessageInt32(method);

    /// <summary>
    /// Creates a delegate which will execute the given method after casting the first argument to
    /// the type that declares the method, and the second argument to the first parameter type of
    /// the method.
    /// </summary>
    /// <param name="method">The method to create a delegate for, which must be declared in an
    /// IMessage implementation.</param>
    internal static Action<IMessage, object> CreateActionIMessageObject(MethodInfo method) =>
        GetReflectionHelper(method.DeclaringType, method.GetParameters()[0].ParameterType)
            .CreateActionIMessageObject(method);

    /// <summary>
    /// Creates a delegate which will execute the given method after casting the first argument to
    /// type that declares the method.
    /// </summary>
    /// <param name="method">The method to create a delegate for, which must be declared in an
    /// IMessage implementation.</param>
    internal static Action<IMessage> CreateActionIMessage(MethodInfo method) =>
        GetReflectionHelper(method.DeclaringType, typeof(object)).CreateActionIMessage(method);

    internal static Func<IMessage, bool> CreateFuncIMessageBool(MethodInfo method) =>
        GetReflectionHelper(method.DeclaringType, method.ReturnType).CreateFuncIMessageBool(method);

    [UnconditionalSuppressMessage(
        "Trimming", "IL2026",
        Justification =
            "Type parameter members are preserved with DynamicallyAccessedMembers on GeneratedClrTypeInfo.ctor clrType parameter.")]
    [UnconditionalSuppressMessage(
        "AotAnalysis", "IL3050:RequiresDynamicCode",
        Justification =
            "Type definition is explicitly specified and type argument is always a message type.")]
    internal static Func<IMessage, bool> CreateIsInitializedCaller([
      DynamicallyAccessedMembers(GeneratedClrTypeInfo.MessageAccessibility)
    ] Type msg) => ((IExtensionSetReflector)Activator
                        .CreateInstance(typeof(ExtensionSetReflector<>).MakeGenericType(msg)))
                       .CreateIsInitializedCaller();

    /// <summary>
    /// Creates a delegate which will execute the given method after casting the first argument to
    /// the type that declares the method, and the second argument to the first parameter type of
    /// the method.
    /// </summary>
    [UnconditionalSuppressMessage(
        "Trimming", "IL2026",
        Justification =
            "Type parameter members are preserved with DynamicallyAccessedMembers on GeneratedClrTypeInfo.ctor clrType parameter.")]
    [UnconditionalSuppressMessage("AOT", "IL3050",
                                  Justification = "Dynamic code won't call Type.MakeGenericType.")]
    internal static IExtensionReflectionHelper CreateExtensionHelper(Extension extension) {
#if NET5_0_OR_GREATER
      if (!RuntimeFeature.IsDynamicCodeSupported) {
        // Using extensions with reflection is not supported with AOT.
        // This helper is created when descriptors are populated. Delay throwing error until an app
        // uses IFieldAccessor with an extension field.
        return new AotExtensionReflectionHelper();
      }
#endif

      var t1 = extension.TargetType;
      var t3 = extension.GetType().GenericTypeArguments[1];
      return (IExtensionReflectionHelper)Activator.CreateInstance(
          typeof(ExtensionReflectionHelper<, >).MakeGenericType(t1, t3), extension);
    }

    /// <summary>
    /// Creates a reflection helper for the given type arguments. Currently these are created on
    /// demand rather than cached; this will be "busy" when initially loading a message's
    /// descriptor, but after that they can be garbage collected. We could cache them by type if
    /// that proves to be important, but creating an object is pretty cheap.
    /// </summary>
    [UnconditionalSuppressMessage(
        "Trimming", "IL2026",
        Justification =
            "Type parameter members are preserved with DynamicallyAccessedMembers on GeneratedClrTypeInfo.ctor clrType parameter.")]
    [UnconditionalSuppressMessage("AOT", "IL3050",
                                  Justification = "Dynamic code won't call Type.MakeGenericType.")]
    private static IReflectionHelper GetReflectionHelper(Type t1, Type t2) {
#if NET5_0_OR_GREATER
      if (!RuntimeFeature.IsDynamicCodeSupported) {
        return new AotReflectionHelper();
      }
#endif

      return (IReflectionHelper)Activator.CreateInstance(
          typeof(ReflectionHelper<, >).MakeGenericType(t1, t2));
    }

    // Non-generic interface allowing us to use an instance of ReflectionHelper<T1, T2> without
    // statically knowing the types involved.
    private interface IReflectionHelper {
      Func<IMessage, int> CreateFuncIMessageInt32(MethodInfo method);
      Action<IMessage> CreateActionIMessage(MethodInfo method);
      Func<IMessage, object> CreateFuncIMessageObject(MethodInfo method);
      Action<IMessage, object> CreateActionIMessageObject(MethodInfo method);
      Func<IMessage, bool> CreateFuncIMessageBool(MethodInfo method);
    }

    internal interface IExtensionReflectionHelper {
      object GetExtension(IMessage message);
      void SetExtension(IMessage message, object value);
      bool HasExtension(IMessage message);
      void ClearExtension(IMessage message);
    }

    private interface IExtensionSetReflector {
      Func<IMessage, bool> CreateIsInitializedCaller();
    }

    private sealed class ReflectionHelper<T1, T2> : IReflectionHelper {
      public Func<IMessage, int> CreateFuncIMessageInt32(MethodInfo method) {
        // On pleasant runtimes, we can create a Func<int> from a method returning
        // an enum based on an int. That's the fast path.
        if (CanConvertEnumFuncToInt32Func) {
          var del = (Func<T1, int>)method.CreateDelegate(typeof(Func<T1, int>));
          return message => del((T1)message);
        } else {
          // On some runtimes (e.g. old Mono) the return type has to be exactly correct,
          // so we go via boxing. Reflection is already fairly inefficient, and this is
          // only used for one-of case checking, fortunately.
          var del = (Func<T1, T2>)method.CreateDelegate(typeof(Func<T1, T2>));
          return message => (int)(object)del((T1)message);
        }
      }

      public Action<IMessage> CreateActionIMessage(MethodInfo method) {
        var del = (Action<T1>)method.CreateDelegate(typeof(Action<T1>));
        return message => del((T1)message);
      }

      public Func<IMessage, object> CreateFuncIMessageObject(MethodInfo method) {
        var del = (Func<T1, T2>)method.CreateDelegate(typeof(Func<T1, T2>));
        return message => del((T1)message);
      }

      public Action<IMessage, object> CreateActionIMessageObject(MethodInfo method) {
        var del = (Action<T1, T2>)method.CreateDelegate(typeof(Action<T1, T2>));
        return (message, arg) => del((T1)message, (T2)arg);
      }

      public Func<IMessage, bool> CreateFuncIMessageBool(MethodInfo method) {
        var del = (Func<T1, bool>)method.CreateDelegate(typeof(Func<T1, bool>));
        return message => del((T1)message);
      }
    }

    private sealed class ExtensionReflectionHelper<T1, T3> : IExtensionReflectionHelper
        where T1 : IExtendableMessage<T1> {
      private readonly Extension extension;

      public ExtensionReflectionHelper(Extension extension) {
        this.extension = extension;
      }

      public object GetExtension(IMessage message) {
        if (message is not T1 extensionMessage) {
          throw new InvalidCastException(
              "Cannot access extension on message that isn't IExtensionMessage");
        }

        if (extension is Extension<T1, T3> ext13) {
          return extensionMessage.GetExtension(ext13);
        } else if (extension is RepeatedExtension<T1, T3> repeatedExt13) {
          return extensionMessage.GetOrInitializeExtension(repeatedExt13);
        } else {
          throw new InvalidCastException(
              "The provided extension is not a valid extension identifier type");
        }
      }

      public bool HasExtension(IMessage message) {
        if (message is not T1 extensionMessage) {
          throw new InvalidCastException(
              "Cannot access extension on message that isn't IExtensionMessage");
        }

        if (extension is Extension<T1, T3> ext13) {
          return extensionMessage.HasExtension(ext13);
        } else if (extension is RepeatedExtension<T1, T3>) {
          throw new InvalidOperationException(
              "HasValue is not implemented for repeated extensions");
        } else {
          throw new InvalidCastException(
              "The provided extension is not a valid extension identifier type");
        }
      }

      public void SetExtension(IMessage message, object value) {
        if (message is not T1 extensionMessage) {
          throw new InvalidCastException(
              "Cannot access extension on message that isn't IExtensionMessage");
        }

        if (extension is Extension<T1, T3> ext13) {
          extensionMessage.SetExtension(ext13, (T3)value);
        } else if (extension is RepeatedExtension<T1, T3>) {
          throw new InvalidOperationException(
              "SetValue is not implemented for repeated extensions");
        } else {
          throw new InvalidCastException(
              "The provided extension is not a valid extension identifier type");
        }
      }

      public void ClearExtension(IMessage message) {
        if (message is not T1 extensionMessage) {
          throw new InvalidCastException(
              "Cannot access extension on message that isn't IExtensionMessage");
        }

        if (extension is Extension<T1, T3> ext13) {
          extensionMessage.ClearExtension(ext13);
        } else if (extension is RepeatedExtension<T1, T3> repeatedExt13) {
          extensionMessage.GetExtension(repeatedExt13).Clear();
        } else {
          throw new InvalidCastException(
              "The provided extension is not a valid extension identifier type");
        }
      }
    }

#if NET5_0_OR_GREATER
    /// <summary>
    /// This helper is compatible with .NET Native AOT.
    /// MakeGenericType doesn't work when a type argument is a value type in AOT.
    /// MethodInfo.Invoke is used instead of compiled expressions because it's faster in AOT.
    /// </summary>
    private sealed class AotReflectionHelper : IReflectionHelper {
      private static readonly object[] EmptyObjectArray = new object[0];
      public Action<IMessage> CreateActionIMessage(MethodInfo method) => message =>
          method.Invoke(message, EmptyObjectArray);
      public Action<IMessage, object> CreateActionIMessageObject(MethodInfo method) =>
          (message, arg) => method.Invoke(message, new object[] { arg });
      public Func<IMessage, bool> CreateFuncIMessageBool(MethodInfo method) => message =>
          (bool)method.Invoke(message, EmptyObjectArray);
      public Func<IMessage, int> CreateFuncIMessageInt32(MethodInfo method) => message =>
          (int)method.Invoke(message, EmptyObjectArray);
      public Func<IMessage, object> CreateFuncIMessageObject(MethodInfo method) => message =>
          method.Invoke(message, EmptyObjectArray);
    }

    /// <summary>
    /// Reflection with extensions isn't supported because IExtendableMessage members are used to
    /// get values. Can't use reflection to invoke those methods because they have a generic
    /// argument. MakeGenericMethod can't be used because it will break whenever the extension type
    /// is a value type. This could be made to work if there were non-generic methods available for
    /// getting and setting extension values.
    /// </summary>
    private sealed class AotExtensionReflectionHelper : IExtensionReflectionHelper {
      private const string Message = "Extensions reflection is not supported with AOT.";
      public object GetExtension(IMessage message) => throw new NotSupportedException(Message);
      public bool HasExtension(IMessage message) => throw new NotSupportedException(Message);
      public void SetExtension(IMessage message,
                               object value) => throw new NotSupportedException(Message);
      public void ClearExtension(IMessage message) => throw new NotSupportedException(Message);
    }
#endif

    private sealed class ExtensionSetReflector<[DynamicallyAccessedMembers(
        DynamicallyAccessedMemberTypes.PublicProperties |
        DynamicallyAccessedMemberTypes.NonPublicProperties)] T1> : IExtensionSetReflector
        where T1 : IExtendableMessage<T1> {
      public Func<IMessage, bool> CreateIsInitializedCaller() {
        var prop = typeof(T1).GetTypeInfo().GetDeclaredProperty("_Extensions");
        var getFunc = (Func<T1, ExtensionSet<T1>>)prop.GetMethod.CreateDelegate(
            typeof(Func<T1, ExtensionSet<T1>>));
        var initializedFunc = (Func<ExtensionSet<T1>, bool>)typeof(ExtensionSet<T1>)
                                  .GetTypeInfo()
                                  .GetDeclaredMethod("IsInitialized")
                                  .CreateDelegate(typeof(Func<ExtensionSet<T1>, bool>));
        return (m) => {
          var set = getFunc((T1)m);
          return set == null || initializedFunc(set);
        };
      }
    }

    // Runtime compatibility checking code - see ReflectionHelper<T1, T2>.CreateFuncIMessageInt32
    // for details about why we're doing this.

    // Deliberately not inside the generic type. We only want to check this once.
    private static bool CanConvertEnumFuncToInt32Func { get; } =
        CheckCanConvertEnumFuncToInt32Func();

    private static bool CheckCanConvertEnumFuncToInt32Func() {
      try {
        // Try to do the conversion using reflection, so we can see whether it's supported.
        MethodInfo method = typeof(ReflectionUtil).GetMethod(nameof(SampleEnumMethod));
        // If this passes, we're in a reasonable runtime.
        method.CreateDelegate(typeof(Func<int>));
        return true;
      } catch (ArgumentException) {
        return false;
      }
    }

    public enum SampleEnum { X }

    // Public to make the reflection simpler.
    public static SampleEnum SampleEnumMethod() => SampleEnum.X;
  }
}
