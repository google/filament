#region Copyright notice and license
// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#endregion

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Reflection;
#if NET35
// Needed for ReadOnlyDictionary, which does not exist in .NET 3.5
using Google.Protobuf.Collections;
#endif

namespace Google.Protobuf.Reflection
{
    /// <summary>
    /// Describes a message type.
    /// </summary>
    public sealed class MessageDescriptor : DescriptorBase
    {
        private static readonly HashSet<string> WellKnownTypeNames = new HashSet<string>
        {
            "google/protobuf/any.proto",
            "google/protobuf/api.proto",
            "google/protobuf/duration.proto",
            "google/protobuf/empty.proto",
            "google/protobuf/wrappers.proto",
            "google/protobuf/timestamp.proto",
            "google/protobuf/field_mask.proto",
            "google/protobuf/source_context.proto",
            "google/protobuf/struct.proto",
            "google/protobuf/type.proto",
        };

        private readonly IList<FieldDescriptor> fieldsInDeclarationOrder;
        private readonly IList<FieldDescriptor> fieldsInNumberOrder;
        private readonly IDictionary<string, FieldDescriptor> jsonFieldMap;
        private Func<IMessage, bool> extensionSetIsInitialized;

        internal MessageDescriptor(DescriptorProto proto, FileDescriptor file, MessageDescriptor parent, int typeIndex, GeneratedClrTypeInfo generatedCodeInfo)
            : base(file, file.ComputeFullName(parent, proto.Name), typeIndex)
        {
            Proto = proto;
            Parser = generatedCodeInfo?.Parser;
            ClrType = generatedCodeInfo?.ClrType;
            ContainingType = parent;

            // If generatedCodeInfo is null, we just won't generate an accessor for any fields.
            Oneofs = DescriptorUtil.ConvertAndMakeReadOnly(
                proto.OneofDecl,
                (oneof, index) =>
                new OneofDescriptor(oneof, file, this, index, generatedCodeInfo?.OneofNames[index]));

            int syntheticOneofCount = 0;
            foreach (var oneof in Oneofs)
            {
                if (oneof.IsSynthetic)
                {
                    syntheticOneofCount++;
                }
                else if (syntheticOneofCount != 0)
                {
                    throw new ArgumentException("All synthetic oneofs should come after real oneofs");
                }
            }
            RealOneofCount = Oneofs.Count - syntheticOneofCount;

            NestedTypes = DescriptorUtil.ConvertAndMakeReadOnly(
                proto.NestedType,
                (type, index) =>
                new MessageDescriptor(type, file, this, index, generatedCodeInfo?.NestedTypes[index]));

            EnumTypes = DescriptorUtil.ConvertAndMakeReadOnly(
                proto.EnumType,
                (type, index) =>
                new EnumDescriptor(type, file, this, index, generatedCodeInfo?.NestedEnums[index]));

            Extensions = new ExtensionCollection(this, generatedCodeInfo?.Extensions);

            fieldsInDeclarationOrder = DescriptorUtil.ConvertAndMakeReadOnly(
                proto.Field,
                (field, index) =>
                new FieldDescriptor(field, file, this, index, generatedCodeInfo?.PropertyNames[index], null));
            fieldsInNumberOrder = new ReadOnlyCollection<FieldDescriptor>(fieldsInDeclarationOrder.OrderBy(field => field.FieldNumber).ToArray());
            // TODO: Use field => field.Proto.JsonName when we're confident it's appropriate. (And then use it in the formatter, too.)
            jsonFieldMap = CreateJsonFieldMap(fieldsInNumberOrder);
            file.DescriptorPool.AddSymbol(this);
            Fields = new FieldCollection(this);
        }

        private static ReadOnlyDictionary<string, FieldDescriptor> CreateJsonFieldMap(IList<FieldDescriptor> fields)
        {
            var map = new Dictionary<string, FieldDescriptor>();
            foreach (var field in fields)
            {
                map[field.Name] = field;
                map[field.JsonName] = field;
            }
            return new ReadOnlyDictionary<string, FieldDescriptor>(map);
        }

        /// <summary>
        /// The brief name of the descriptor's target.
        /// </summary>
        public override string Name => Proto.Name;

        internal override IReadOnlyList<DescriptorBase> GetNestedDescriptorListForField(int fieldNumber)
        {
            switch (fieldNumber)
            {
                case DescriptorProto.FieldFieldNumber:
                    return (IReadOnlyList<DescriptorBase>) fieldsInDeclarationOrder;
                case DescriptorProto.NestedTypeFieldNumber:
                    return (IReadOnlyList<DescriptorBase>) NestedTypes;
                case DescriptorProto.EnumTypeFieldNumber:
                    return (IReadOnlyList<DescriptorBase>) EnumTypes;
                default:
                    return null;
            }
        }

        internal DescriptorProto Proto { get; }

        /// <summary>
        /// Returns a clone of the underlying <see cref="DescriptorProto"/> describing this message.
        /// Note that a copy is taken every time this method is called, so clients using it frequently
        /// (and not modifying it) may want to cache the returned value.
        /// </summary>
        /// <returns>A protobuf representation of this message descriptor.</returns>
        public DescriptorProto ToProto() => Proto.Clone();

        internal bool IsExtensionsInitialized(IMessage message)
        {
            if (Proto.ExtensionRange.Count == 0)
            {
                return true;
            }

            if (extensionSetIsInitialized == null)
            {
                extensionSetIsInitialized = ReflectionUtil.CreateIsInitializedCaller(ClrType);
            }

            return extensionSetIsInitialized(message);
        }

        /// <summary>
        /// The CLR type used to represent message instances from this descriptor.
        /// </summary>
        /// <remarks>
        /// <para>
        /// The value returned by this property will be non-null for all regular fields. However,
        /// if a message containing a map field is introspected, the list of nested messages will include
        /// an auto-generated nested key/value pair message for the field. This is not represented in any
        /// generated type, so this property will return null in such cases.
        /// </para>
        /// <para>
        /// For wrapper types (<see cref="Google.Protobuf.WellKnownTypes.StringValue"/> and the like), the type returned here
        /// will be the generated message type, not the native type used by reflection for fields of those types. Code
        /// using reflection should call <see cref="IsWrapperType"/> to determine whether a message descriptor represents
        /// a wrapper type, and handle the result appropriately.
        /// </para>
        /// </remarks>
        [DynamicallyAccessedMembers(GeneratedClrTypeInfo.MessageAccessibility)]
        public Type ClrType { get; }

        /// <summary>
        /// A parser for this message type.
        /// </summary>
        /// <remarks>
        /// <para>
        /// As <see cref="MessageDescriptor"/> is not generic, this cannot be statically
        /// typed to the relevant type, but it should produce objects of a type compatible with <see cref="ClrType"/>.
        /// </para>
        /// <para>
        /// The value returned by this property will be non-null for all regular fields. However,
        /// if a message containing a map field is introspected, the list of nested messages will include
        /// an auto-generated nested key/value pair message for the field. No message parser object is created for
        /// such messages, so this property will return null in such cases.
        /// </para>
        /// <para>
        /// For wrapper types (<see cref="Google.Protobuf.WellKnownTypes.StringValue"/> and the like), the parser returned here
        /// will be the generated message type, not the native type used by reflection for fields of those types. Code
        /// using reflection should call <see cref="IsWrapperType"/> to determine whether a message descriptor represents
        /// a wrapper type, and handle the result appropriately.
        /// </para>
        /// </remarks>
        public MessageParser Parser { get; }

        /// <summary>
        /// Returns whether this message is one of the "well known types" which may have runtime/protoc support.
        /// </summary>
        internal bool IsWellKnownType => File.Package == "google.protobuf" && WellKnownTypeNames.Contains(File.Name);

        /// <summary>
        /// Returns whether this message is one of the "wrapper types" used for fields which represent primitive values
        /// with the addition of presence.
        /// </summary>
        internal bool IsWrapperType => File.Package == "google.protobuf" && File.Name == "google/protobuf/wrappers.proto";

        /// <value>
        /// If this is a nested type, get the outer descriptor, otherwise null.
        /// </value>
        public MessageDescriptor ContainingType { get; }

        /// <value>
        /// A collection of fields, which can be retrieved by name or field number.
        /// </value>
        public FieldCollection Fields { get; }

        /// <summary>
        /// An unmodifiable list of extensions defined in this message's scope.
        /// Note that some extensions may be incomplete (FieldDescriptor.Extension may be null)
        /// if they are declared in a file generated using a version of protoc that did not fully
        /// support extensions in C#.
        /// </summary>
        public ExtensionCollection Extensions { get; }

        /// <value>
        /// An unmodifiable list of this message type's nested types.
        /// </value>
        public IList<MessageDescriptor> NestedTypes { get; }

        /// <value>
        /// An unmodifiable list of this message type's enum types.
        /// </value>
        public IList<EnumDescriptor> EnumTypes { get; }

        /// <value>
        /// An unmodifiable list of the "oneof" field collections in this message type.
        /// All "real" oneofs (where <see cref="OneofDescriptor.IsSynthetic"/> returns false)
        /// come before synthetic ones.
        /// </value>
        public IList<OneofDescriptor> Oneofs { get; }

        /// <summary>
        /// The number of real "oneof" descriptors in this message type. Every element in <see cref="Oneofs"/>
        /// with an index less than this will have a <see cref="OneofDescriptor.IsSynthetic"/> property value
        /// of <c>false</c>; every element with an index greater than or equal to this will have a
        /// <see cref="OneofDescriptor.IsSynthetic"/> property value of <c>true</c>.
        /// </summary>
        public int RealOneofCount { get; }

        /// <summary>
        /// Finds a field by field name.
        /// </summary>
        /// <param name="name">The unqualified name of the field (e.g. "foo").</param>
        /// <returns>The field's descriptor, or null if not found.</returns>
        public FieldDescriptor FindFieldByName(String name) => File.DescriptorPool.FindSymbol<FieldDescriptor>(FullName + "." + name);

        /// <summary>
        /// Finds a field by field number.
        /// </summary>
        /// <param name="number">The field number within this message type.</param>
        /// <returns>The field's descriptor, or null if not found.</returns>
        public FieldDescriptor FindFieldByNumber(int number) => File.DescriptorPool.FindFieldByNumber(this, number);

        /// <summary>
        /// Finds a nested descriptor by name. The is valid for fields, nested
        /// message types, oneofs and enums.
        /// </summary>
        /// <param name="name">The unqualified name of the descriptor, e.g. "Foo"</param>
        /// <returns>The descriptor, or null if not found.</returns>
        public T FindDescriptor<T>(string name)  where T : class, IDescriptor =>
            File.DescriptorPool.FindSymbol<T>(FullName + "." + name);

        /// <summary>
        /// The (possibly empty) set of custom options for this message.
        /// </summary>
        [Obsolete("CustomOptions are obsolete. Use the GetOptions() method.")]
        public CustomOptions CustomOptions => new CustomOptions(Proto.Options?._extensions?.ValuesByNumber);

        /// <summary>
        /// The <c>MessageOptions</c>, defined in <c>descriptor.proto</c>.
        /// If the options message is not present (i.e. there are no options), <c>null</c> is returned.
        /// Custom options can be retrieved as extensions of the returned message.
        /// NOTE: A defensive copy is created each time this property is retrieved.
        /// </summary>
        public MessageOptions GetOptions() => Proto.Options?.Clone();

        /// <summary>
        /// Gets a single value message option for this descriptor
        /// </summary>
        [Obsolete("GetOption is obsolete. Use the GetOptions() method.")]
        public T GetOption<T>(Extension<MessageOptions, T> extension)
        {
            var value = Proto.Options.GetExtension(extension);
            return value is IDeepCloneable<T> ? (value as IDeepCloneable<T>).Clone() : value;
        }

        /// <summary>
        /// Gets a repeated value message option for this descriptor
        /// </summary>
        [Obsolete("GetOption is obsolete. Use the GetOptions() method.")]
        public Collections.RepeatedField<T> GetOption<T>(RepeatedExtension<MessageOptions, T> extension)
        {
            return Proto.Options.GetExtension(extension).Clone();
        }

        /// <summary>
        /// Looks up and cross-links all fields and nested types.
        /// </summary>
        internal void CrossLink()
        {
            foreach (MessageDescriptor message in NestedTypes)
            {
                message.CrossLink();
            }

            foreach (FieldDescriptor field in fieldsInDeclarationOrder)
            {
                field.CrossLink();
            }

            foreach (OneofDescriptor oneof in Oneofs)
            {
                oneof.CrossLink();
            }

            Extensions.CrossLink();
        }

        /// <summary>
        /// A collection to simplify retrieving the field accessor for a particular field.
        /// </summary>
        public sealed class FieldCollection
        {
            private readonly MessageDescriptor messageDescriptor;

            internal FieldCollection(MessageDescriptor messageDescriptor)
            {
                this.messageDescriptor = messageDescriptor;
            }

            /// <value>
            /// Returns the fields in the message as an immutable list, in the order in which they
            /// are declared in the source .proto file.
            /// </value>
            public IList<FieldDescriptor> InDeclarationOrder() => messageDescriptor.fieldsInDeclarationOrder;

            /// <value>
            /// Returns the fields in the message as an immutable list, in ascending field number
            /// order. Field numbers need not be contiguous, so there is no direct mapping from the
            /// index in the list to the field number; to retrieve a field by field number, it is better
            /// to use the <see cref="FieldCollection"/> indexer.
            /// </value>
            public IList<FieldDescriptor> InFieldNumberOrder() => messageDescriptor.fieldsInNumberOrder;

            // TODO: consider making this public in the future. (Being conservative for now...)

            /// <value>
            /// Returns a read-only dictionary mapping the field names in this message as they're available
            /// in the JSON representation to the field descriptors. For example, a field <c>foo_bar</c>
            /// in the message would result two entries, one with a key <c>fooBar</c> and one with a key
            /// <c>foo_bar</c>, both referring to the same field.
            /// </value>
            internal IDictionary<string, FieldDescriptor> ByJsonName() => messageDescriptor.jsonFieldMap;

            /// <summary>
            /// Retrieves the descriptor for the field with the given number.
            /// </summary>
            /// <param name="number">Number of the field to retrieve the descriptor for</param>
            /// <returns>The accessor for the given field</returns>
            /// <exception cref="KeyNotFoundException">The message descriptor does not contain a field
            /// with the given number</exception>
            public FieldDescriptor this[int number]
            {
                get
                {
                    var fieldDescriptor = messageDescriptor.FindFieldByNumber(number);
                    if (fieldDescriptor == null)
                    {
                        throw new KeyNotFoundException("No such field number");
                    }
                    return fieldDescriptor;
                }
            }

            /// <summary>
            /// Retrieves the descriptor for the field with the given name.
            /// </summary>
            /// <param name="name">Name of the field to retrieve the descriptor for</param>
            /// <returns>The descriptor for the given field</returns>
            /// <exception cref="KeyNotFoundException">The message descriptor does not contain a field
            /// with the given name</exception>
            public FieldDescriptor this[string name]
            {
                get
                {
                    var fieldDescriptor = messageDescriptor.FindFieldByName(name);
                    if (fieldDescriptor == null)
                    {
                        throw new KeyNotFoundException("No such field name");
                    }
                    return fieldDescriptor;
                }
            }
        }
    }
}
