#region Copyright notice and license
// Protocol Buffers - Google's data interchange format
// Copyright 2015 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd
#endregion

using System;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using Google.Protobuf.Compatibility;

namespace Google.Protobuf.Reflection
{
    /// <summary>
    /// Accessor for single fields.
    /// </summary>
    internal sealed class SingleFieldAccessor : FieldAccessorBase
    {
        // All the work here is actually done in the constructor - it creates the appropriate delegates.
        // There are various cases to consider, based on the property type (message, string/bytes, or "genuine" primitive)
        // and proto2 vs proto3 for non-message types, as proto3 doesn't support "full" presence detection or default
        // values.

        private readonly Action<IMessage, object> setValueDelegate;
        private readonly Action<IMessage> clearDelegate;
        private readonly Func<IMessage, bool> hasDelegate;

        internal SingleFieldAccessor(
            [DynamicallyAccessedMembers(GeneratedClrTypeInfo.MessageAccessibility)]
            Type messageType, PropertyInfo property, FieldDescriptor descriptor) : base(property, descriptor)
        {
            if (!property.CanWrite)
            {
                throw new ArgumentException("Not all required properties/methods available");
            }
            setValueDelegate = ReflectionUtil.CreateActionIMessageObject(property.GetSetMethod());

            // Note: this looks worrying in that we access the containing oneof, which isn't valid until cross-linking
            // is complete... but field accessors aren't created until after cross-linking.
            // The oneof itself won't be cross-linked yet, but that's okay: the oneof accessor is created
            // earlier.

            // Message fields always support presence, via null checks.
            if (descriptor.FieldType == FieldType.Message)
            {
                hasDelegate = message => GetValue(message) != null;
                clearDelegate = message => SetValue(message, null);
            }
            // Oneof fields always support presence, via case checks.
            // Note that clearing the field is a no-op unless that specific field is the current "case".
            else if (descriptor.RealContainingOneof != null)
            {
                var oneofAccessor = descriptor.RealContainingOneof.Accessor;
                hasDelegate = message => oneofAccessor.GetCaseFieldDescriptor(message) == descriptor;
                clearDelegate = message =>
                {
                    // Clear on a field only affects the oneof itself if the current case is the field we're accessing.
                    if (oneofAccessor.GetCaseFieldDescriptor(message) == descriptor)
                    {
                        oneofAccessor.Clear(message);
                    }
                };
            }
            // Anything else that supports presence should have a "HasXyz" property and a "ClearXyz"
            // method.
            else if (descriptor.HasPresence)
            {
                MethodInfo hasMethod = messageType.GetRuntimeProperty("Has" + property.Name).GetMethod;
                if (hasMethod == null)
                {
                    throw new ArgumentException("Not all required properties/methods are available");
                }
                hasDelegate = ReflectionUtil.CreateFuncIMessageBool(hasMethod);
                MethodInfo clearMethod = messageType.GetRuntimeMethod("Clear" + property.Name, ReflectionUtil.EmptyTypes);
                if (clearMethod == null)
                {
                    throw new ArgumentException("Not all required properties/methods are available");
                }
                clearDelegate = ReflectionUtil.CreateActionIMessage(clearMethod);
            }
            // Otherwise, we don't support presence.
            else
            {
                hasDelegate = message => throw new InvalidOperationException("Presence is not implemented for this field");

                // While presence isn't supported, clearing still is; it's just setting to a default value.
                object defaultValue = GetDefaultValue(descriptor);
                clearDelegate = message => SetValue(message, defaultValue);
            }
        }

        private static object GetDefaultValue(FieldDescriptor descriptor) =>
            descriptor.FieldType switch
            {
                FieldType.Bool => false,
                FieldType.Bytes => ByteString.Empty,
                FieldType.String => "",
                FieldType.Double => 0.0,
                FieldType.SInt32 or FieldType.Int32 or FieldType.SFixed32 or FieldType.Enum => 0,
                FieldType.Fixed32 or FieldType.UInt32 => (uint)0,
                FieldType.Fixed64 or FieldType.UInt64 => 0UL,
                FieldType.SFixed64 or FieldType.Int64 or FieldType.SInt64 => 0L,
                FieldType.Float => 0f,
                FieldType.Message or FieldType.Group => null,
                _ => throw new ArgumentException("Invalid field type"),
            };

        public override void Clear(IMessage message) => clearDelegate(message);
        public override bool HasValue(IMessage message) => hasDelegate(message);
        public override void SetValue(IMessage message, object value) => setValueDelegate(message, value);
    }
}
