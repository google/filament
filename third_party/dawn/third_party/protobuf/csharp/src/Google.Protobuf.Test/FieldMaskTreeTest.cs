﻿#region Copyright notice and license
// Protocol Buffers - Google's data interchange format
// Copyright 2015 Google Inc.  All rights reserved.
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

using System.Collections.Generic;
using Google.Protobuf.Collections;
using Google.Protobuf.TestProtos;
using NUnit.Framework;
using Google.Protobuf.WellKnownTypes;

namespace Google.Protobuf
{
    public class FieldMaskTreeTest
    {
        [Test]
        public void AddFieldPath()
        {
            FieldMaskTree tree = new FieldMaskTree();
            RepeatedField<string> paths = tree.ToFieldMask().Paths;
            Assert.AreEqual(0, paths.Count);

            tree.AddFieldPath("");
            paths = tree.ToFieldMask().Paths;
            Assert.AreEqual(1, paths.Count);
            Assert.Contains("", paths);

            // New branch.
            tree.AddFieldPath("foo");
            paths = tree.ToFieldMask().Paths;
            Assert.AreEqual(2, paths.Count);
            Assert.Contains("foo", paths);

            // Redundant path.
            tree.AddFieldPath("foo");
            paths = tree.ToFieldMask().Paths;
            Assert.AreEqual(2, paths.Count);

            // New branch.
            tree.AddFieldPath("bar.baz");
            paths = tree.ToFieldMask().Paths;
            Assert.AreEqual(3, paths.Count);
            Assert.Contains("bar.baz", paths);

            // Redundant sub-path.
            tree.AddFieldPath("foo.bar");
            paths = tree.ToFieldMask().Paths;
            Assert.AreEqual(3, paths.Count);

            // New branch from a non-root node.
            tree.AddFieldPath("bar.quz");
            paths = tree.ToFieldMask().Paths;
            Assert.AreEqual(4, paths.Count);
            Assert.Contains("bar.quz", paths);

            // A path that matches several existing sub-paths.
            tree.AddFieldPath("bar");
            paths = tree.ToFieldMask().Paths;
            Assert.AreEqual(3, paths.Count);
            Assert.Contains("foo", paths);
            Assert.Contains("bar", paths);
        }

        [Test]
        public void MergeFromFieldMask()
        {
            FieldMaskTree tree = new FieldMaskTree();
            tree.MergeFromFieldMask(new FieldMask
            {
                Paths = {"foo", "bar.baz", "bar.quz"}
            });
            RepeatedField<string> paths = tree.ToFieldMask().Paths;
            Assert.AreEqual(3, paths.Count);
            Assert.Contains("foo", paths);
            Assert.Contains("bar.baz", paths);
            Assert.Contains("bar.quz", paths);

            tree.MergeFromFieldMask(new FieldMask
            {
                Paths = {"foo.bar", "bar"}
            });
            paths = tree.ToFieldMask().Paths;
            Assert.AreEqual(2, paths.Count);
            Assert.Contains("foo", paths);
            Assert.Contains("bar", paths);
        }

        [Test]
        public void IntersectFieldPath()
        {
            FieldMaskTree tree = new FieldMaskTree();
            FieldMaskTree result = new FieldMaskTree();
            tree.MergeFromFieldMask(new FieldMask
            {
                Paths = {"foo", "bar.baz", "bar.quz"}
            });

            // Empty path.
            tree.IntersectFieldPath("", result);
            RepeatedField<string> paths = result.ToFieldMask().Paths;
            Assert.AreEqual(0, paths.Count);

            // Non-exist path.
            tree.IntersectFieldPath("quz", result);
            paths = result.ToFieldMask().Paths;
            Assert.AreEqual(0, paths.Count);

            // Sub-path of an existing leaf.
            tree.IntersectFieldPath("foo.bar", result);
            paths = result.ToFieldMask().Paths;
            Assert.AreEqual(1, paths.Count);
            Assert.Contains("foo.bar", paths);

            // Match an existing leaf node.
            tree.IntersectFieldPath("foo", result);
            paths = result.ToFieldMask().Paths;
            Assert.AreEqual(1, paths.Count);
            Assert.Contains("foo", paths);

            // Non-exist path.
            tree.IntersectFieldPath("bar.foo", result);
            paths = result.ToFieldMask().Paths;
            Assert.AreEqual(1, paths.Count);
            Assert.Contains("foo", paths);

            // Match a non-leaf node.
            tree.IntersectFieldPath("bar", result);
            paths = result.ToFieldMask().Paths;
            Assert.AreEqual(3, paths.Count);
            Assert.Contains("foo", paths);
            Assert.Contains("bar.baz", paths);
            Assert.Contains("bar.quz", paths);
        }

        private void Merge(FieldMaskTree tree, IMessage source, IMessage destination, FieldMask.MergeOptions options, bool useDynamicMessage)
        {
            if (useDynamicMessage)
            {
                var newSource = source.Descriptor.Parser.CreateTemplate();
                newSource.MergeFrom(source.ToByteString());

                var newDestination = source.Descriptor.Parser.CreateTemplate();
                newDestination.MergeFrom(destination.ToByteString());

                tree.Merge(newSource, newDestination, options);

                // Clear before merging:
                foreach (var fieldDescriptor in destination.Descriptor.Fields.InFieldNumberOrder())
                {
                    fieldDescriptor.Accessor.Clear(destination);
                }
                destination.MergeFrom(newDestination.ToByteString());
            }
            else
            {
                tree.Merge(source, destination, options);
            }
        }

        [Test]
        [TestCase(true)]
        [TestCase(false)]
        public void Merge(bool useDynamicMessage)
        {
            TestAllTypes value = new TestAllTypes
            {
                SingleInt32 = 1234,
                SingleNestedMessage = new TestAllTypes.Types.NestedMessage {Bb = 5678},
                RepeatedInt32 = {4321},
                RepeatedNestedMessage = {new TestAllTypes.Types.NestedMessage {Bb = 8765}}
            };

            NestedTestAllTypes source = new NestedTestAllTypes
            {
                Payload = value,
                Child = new NestedTestAllTypes {Payload = value}
            };
            // Now we have a message source with the following structure:
            //   [root] -+- payload -+- single_int32
            //           |           +- single_nested_message
            //           |           +- repeated_int32
            //           |           +- repeated_nested_message
            //           |
            //           +- child --- payload -+- single_int32
            //                                 +- single_nested_message
            //                                 +- repeated_int32
            //                                 +- repeated_nested_message

            FieldMask.MergeOptions options = new FieldMask.MergeOptions();

            // Test merging each individual field.
            NestedTestAllTypes destination = new NestedTestAllTypes();
            Merge(new FieldMaskTree().AddFieldPath("payload.single_int32"),
                source, destination, options, useDynamicMessage);
            NestedTestAllTypes expected = new NestedTestAllTypes
            {
                Payload = new TestAllTypes
                {
                    SingleInt32 = 1234
                }
            };
            Assert.AreEqual(expected, destination);

            destination = new NestedTestAllTypes();
            Merge(new FieldMaskTree().AddFieldPath("payload.single_nested_message"),
                source, destination, options, useDynamicMessage);
            expected = new NestedTestAllTypes
            {
                Payload = new TestAllTypes
                {
                    SingleNestedMessage = new TestAllTypes.Types.NestedMessage {Bb = 5678}
                }
            };
            Assert.AreEqual(expected, destination);

            destination = new NestedTestAllTypes();
            Merge(new FieldMaskTree().AddFieldPath("payload.repeated_int32"),
                source, destination, options, useDynamicMessage);
            expected = new NestedTestAllTypes
            {
                Payload = new TestAllTypes
                {
                    RepeatedInt32 = {4321}
                }
            };
            Assert.AreEqual(expected, destination);

            destination = new NestedTestAllTypes();
            Merge(new FieldMaskTree().AddFieldPath("payload.repeated_nested_message"),
                source, destination, options, useDynamicMessage);
            expected = new NestedTestAllTypes
            {
                Payload = new TestAllTypes
                {
                    RepeatedNestedMessage = {new TestAllTypes.Types.NestedMessage {Bb = 8765}}
                }
            };
            Assert.AreEqual(expected, destination);

            destination = new NestedTestAllTypes();
            Merge(
                new FieldMaskTree().AddFieldPath("child.payload.single_int32"),
                source,
                destination,
                options,
                useDynamicMessage);
            expected = new NestedTestAllTypes
            {
                Child = new NestedTestAllTypes
                {
                    Payload = new TestAllTypes
                    {
                        SingleInt32 = 1234
                    }
                }
            };
            Assert.AreEqual(expected, destination);

            destination = new NestedTestAllTypes();
            Merge(
                new FieldMaskTree().AddFieldPath("child.payload.single_nested_message"),
                source,
                destination,
                options,
                useDynamicMessage);
            expected = new NestedTestAllTypes
            {
                Child = new NestedTestAllTypes
                {
                    Payload = new TestAllTypes
                    {
                        SingleNestedMessage = new TestAllTypes.Types.NestedMessage {Bb = 5678}
                    }
                }
            };
            Assert.AreEqual(expected, destination);

            destination = new NestedTestAllTypes();
            Merge(new FieldMaskTree().AddFieldPath("child.payload.repeated_int32"),
                source, destination, options, useDynamicMessage);
            expected = new NestedTestAllTypes
            {
                Child = new NestedTestAllTypes
                {
                    Payload = new TestAllTypes
                    {
                        RepeatedInt32 = {4321}
                    }
                }
            };
            Assert.AreEqual(expected, destination);

            destination = new NestedTestAllTypes();
            Merge(new FieldMaskTree().AddFieldPath("child.payload.repeated_nested_message"),
                source, destination, options, useDynamicMessage);
            expected = new NestedTestAllTypes
            {
                Child = new NestedTestAllTypes
                {
                    Payload = new TestAllTypes
                    {
                        RepeatedNestedMessage = {new TestAllTypes.Types.NestedMessage {Bb = 8765}}
                    }
                }
            };
            Assert.AreEqual(expected, destination);

            destination = new NestedTestAllTypes();
            Merge(new FieldMaskTree().AddFieldPath("child").AddFieldPath("payload"),
                source, destination, options, useDynamicMessage);
            Assert.AreEqual(source, destination);

            // Test repeated options.
            destination = new NestedTestAllTypes
            {
                Payload = new TestAllTypes
                {
                    RepeatedInt32 = { 1000 }
                }
            };
            Merge(new FieldMaskTree().AddFieldPath("payload.repeated_int32"),
                    source, destination, options, useDynamicMessage);
            // Default behavior is to append repeated fields.
            Assert.AreEqual(2, destination.Payload.RepeatedInt32.Count);
            Assert.AreEqual(1000, destination.Payload.RepeatedInt32[0]);
            Assert.AreEqual(4321, destination.Payload.RepeatedInt32[1]);
            // Change to replace repeated fields.
            options.ReplaceRepeatedFields = true;
            Merge(new FieldMaskTree().AddFieldPath("payload.repeated_int32"),
                source, destination, options, useDynamicMessage);
            Assert.AreEqual(1, destination.Payload.RepeatedInt32.Count);
            Assert.AreEqual(4321, destination.Payload.RepeatedInt32[0]);

            // Test message options.
            destination = new NestedTestAllTypes
            {
                Payload = new TestAllTypes
                {
                    SingleInt32 = 1000,
                    SingleUint32 = 2000
                }
            };
            Merge(new FieldMaskTree().AddFieldPath("payload"),
                    source, destination, options, useDynamicMessage);
            // Default behavior is to merge message fields.
            Assert.AreEqual(1234, destination.Payload.SingleInt32);
            Assert.AreEqual(2000, destination.Payload.SingleUint32);

            // Test merging unset message fields.
            NestedTestAllTypes clearedSource = source.Clone();
            clearedSource.Payload = null;
            destination = new NestedTestAllTypes();
            Merge(new FieldMaskTree().AddFieldPath("payload"),
                clearedSource, destination, options, useDynamicMessage);
            Assert.IsNull(destination.Payload);

            // Skip a message field if they are unset in both source and target.
            destination = new NestedTestAllTypes();
            Merge(new FieldMaskTree().AddFieldPath("payload.single_int32"),
                clearedSource, destination, options, useDynamicMessage);
            Assert.IsNull(destination.Payload);

            // Change to replace message fields.
            options.ReplaceMessageFields = true;
            destination = new NestedTestAllTypes
            {
                Payload = new TestAllTypes
                {
                    SingleInt32 = 1000,
                    SingleUint32 = 2000
                }
            };
            Merge(new FieldMaskTree().AddFieldPath("payload"),
                    source, destination, options, useDynamicMessage);
            Assert.AreEqual(1234, destination.Payload.SingleInt32);
            Assert.AreEqual(0, destination.Payload.SingleUint32);

            // Test merging unset message fields.
            destination = new NestedTestAllTypes
            {
                Payload = new TestAllTypes
                {
                    SingleInt32 = 1000,
                    SingleUint32 = 2000
                }
            };
            Merge(new FieldMaskTree().AddFieldPath("payload"),
                    clearedSource, destination, options, useDynamicMessage);
            Assert.IsNull(destination.Payload);

            // Test merging unset primitive fields.
            destination = source.Clone();
            destination.Payload.SingleInt32 = 0;
            NestedTestAllTypes sourceWithPayloadInt32Unset = destination;
            destination = source.Clone();
            Merge(new FieldMaskTree().AddFieldPath("payload.single_int32"),
                sourceWithPayloadInt32Unset, destination, options, useDynamicMessage);
            Assert.AreEqual(0, destination.Payload.SingleInt32);

            // Change to clear unset primitive fields.
            options.ReplacePrimitiveFields = true;
            destination = source.Clone();
            Merge(new FieldMaskTree().AddFieldPath("payload.single_int32"),
                sourceWithPayloadInt32Unset, destination, options, useDynamicMessage);
            Assert.IsNotNull(destination.Payload);
        }

    }
}
