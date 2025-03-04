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

using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using Google.Protobuf.Reflection;
using Google.Protobuf.WellKnownTypes;

namespace Google.Protobuf
{
    /// <summary>
    /// <para>A tree representation of a FieldMask. Each leaf node in this tree represent
    /// a field path in the FieldMask.</para>
    ///
    /// <para>For example, FieldMask "foo.bar,foo.baz,bar.baz" as a tree will be:</para>
    /// <code>
    ///   [root] -+- foo -+- bar
    ///           |       |
    ///           |       +- baz
    ///           |
    ///           +- bar --- baz
    /// </code>
    ///
    /// <para>By representing FieldMasks with this tree structure we can easily convert
    /// a FieldMask to a canonical form, merge two FieldMasks, calculate the
    /// intersection to two FieldMasks and traverse all fields specified by the
    /// FieldMask in a message tree.</para>
    /// </summary>
    internal sealed class FieldMaskTree
    {
        private const char FIELD_PATH_SEPARATOR = '.';

        internal sealed class Node
        {
            public Dictionary<string, Node> Children { get; } = new Dictionary<string, Node>();
        }

        private readonly Node root = new Node();

        /// <summary>
        /// Creates an empty FieldMaskTree.
        /// </summary>
        public FieldMaskTree()
        {
        }

        /// <summary>
        /// Creates a FieldMaskTree for a given FieldMask.
        /// </summary>
        public FieldMaskTree(FieldMask mask)
        {
            MergeFromFieldMask(mask);
        }

        public override string ToString()
        {
            return ToFieldMask().ToString();
        }

        /// <summary>
        /// Adds a field path to the tree. In a FieldMask, every field path matches the
        /// specified field as well as all its sub-fields. For example, a field path
        /// "foo.bar" matches field "foo.bar" and also "foo.bar.baz", etc. When adding
        /// a field path to the tree, redundant sub-paths will be removed. That is,
        /// after adding "foo.bar" to the tree, "foo.bar.baz" will be removed if it
        /// exists, which will turn the tree node for "foo.bar" to a leaf node.
        /// Likewise, if the field path to add is a sub-path of an existing leaf node,
        /// nothing will be changed in the tree.
        /// </summary>
        public FieldMaskTree AddFieldPath(string path)
        {
            var parts = path.Split(FIELD_PATH_SEPARATOR);
            if (parts.Length == 0)
            {
                return this;
            }

            var node = root;
            var createNewBranch = false;

            // Find the matching node in the tree.
            foreach (var part in parts)
            {
                // Check whether the path matches an existing leaf node.
                if (!createNewBranch
                    && node != root
                    && node.Children.Count == 0)
                {
                    // The path to add is a sub-path of an existing leaf node.
                    return this;
                }

                Node childNode;
                if (!node.Children.TryGetValue(part, out childNode))
                {
                    createNewBranch = true;
                    childNode = new Node();
                    node.Children.Add(part, childNode);
                }
                node = childNode;
            }

            // Turn the matching node into a leaf node (i.e., remove sub-paths).
            node.Children.Clear();
            return this;
        }

        /// <summary>
        /// Merges all field paths in a FieldMask into this tree.
        /// </summary>
        public FieldMaskTree MergeFromFieldMask(FieldMask mask)
        {
            foreach (var path in mask.Paths)
            {
                AddFieldPath(path);
            }

            return this;
        }

        /// <summary>
        /// Converts this tree to a FieldMask.
        /// </summary>
        public FieldMask ToFieldMask()
        {
            var mask = new FieldMask();
            if (root.Children.Count != 0)
            {
                var paths = new List<string>();
                GetFieldPaths(root, "", paths);
                mask.Paths.AddRange(paths);
            }

            return mask;
        }

        /// <summary>
        /// Gathers all field paths in a sub-tree.
        /// </summary>
        private void GetFieldPaths(Node node, string path, List<string> paths)
        {
            if (node.Children.Count == 0)
            {
                paths.Add(path);
                return;
            }

            foreach (var entry in node.Children)
            {
                var childPath = path.Length == 0 ? entry.Key : path + "." + entry.Key;
                GetFieldPaths(entry.Value, childPath, paths);
            }
        }

        /// <summary>
        /// Adds the intersection of this tree with the given <paramref name="path"/> to <paramref name="output"/>.
        /// </summary>
        public void IntersectFieldPath(string path, FieldMaskTree output)
        {
            if (root.Children.Count == 0)
            {
                return;
            }

            var parts = path.Split(FIELD_PATH_SEPARATOR);
            if (parts.Length == 0)
            {
                return;
            }

            var node = root;
            foreach (var part in parts)
            {
                if (node != root
                    && node.Children.Count == 0)
                {
                    // The given path is a sub-path of an existing leaf node in the tree.
                    output.AddFieldPath(path);
                    return;
                }

                if (!node.Children.TryGetValue(part, out node))
                {
                    return;
                }
            }

            // We found a matching node for the path. All leaf children of this matching
            // node is in the intersection.
            var paths = new List<string>();
            GetFieldPaths(node, path, paths);
            foreach (var value in paths)
            {
                output.AddFieldPath(value);
            }
        }

        /// <summary>
        /// Merges all fields specified by this FieldMaskTree from <paramref name="source"/> to <paramref name="destination"/>.
        /// </summary>
        public void Merge(IMessage source, IMessage destination, FieldMask.MergeOptions options)
        {
            if (source.Descriptor != destination.Descriptor)
            {
                throw new InvalidProtocolBufferException("Cannot merge messages of different types.");
            }

            if (root.Children.Count == 0)
            {
                return;
            }

            Merge(root, "", source, destination, options);
        }

        /// <summary>
        /// Merges all fields specified by a sub-tree from <paramref name="source"/> to <paramref name="destination"/>.
        /// </summary>
        private void Merge(
            Node node,
            string path,
            IMessage source,
            IMessage destination,
            FieldMask.MergeOptions options)
        {
            if (source.Descriptor != destination.Descriptor)
            {
                throw new InvalidProtocolBufferException($"source ({source.Descriptor}) and destination ({destination.Descriptor}) descriptor must be equal");
            }

            var descriptor = source.Descriptor;
            foreach (var entry in node.Children)
            {
                var field = descriptor.FindFieldByName(entry.Key);
                if (field == null)
                {
                    Debug.WriteLine($"Cannot find field \"{entry.Key}\" in message type \"{descriptor.FullName}\"");
                    continue;
                }

                if (entry.Value.Children.Count != 0)
                {
                    if (field.IsRepeated
                        || field.FieldType != FieldType.Message)
                    {
                        Debug.WriteLine($"Field \"{field.FullName}\" is not a singular message field and cannot have sub-fields.");
                        continue;
                    }

                    var sourceField = field.Accessor.GetValue(source);
                    var destinationField = field.Accessor.GetValue(destination);
                    if (sourceField == null
                        && destinationField == null)
                    {
                        // If the message field is not present in both source and destination, skip recursing
                        // so we don't create unnecessary empty messages.
                        continue;
                    }

                    if (destinationField == null)
                    {
                        // If we have to merge but the destination does not contain the field, create it.
                        destinationField = field.MessageType.Parser.CreateTemplate();
                        field.Accessor.SetValue(destination, destinationField);
                    }

                    var childPath = path.Length == 0 ? entry.Key : path + "." + entry.Key;
                    Merge(entry.Value, childPath, (IMessage)sourceField, (IMessage)destinationField, options);
                    continue;
                }

                if (field.IsRepeated)
                {
                    if (options.ReplaceRepeatedFields)
                    {
                        field.Accessor.Clear(destination);
                    }

                    var sourceField = (IList)field.Accessor.GetValue(source);
                    var destinationField = (IList)field.Accessor.GetValue(destination);
                    foreach (var element in sourceField)
                    {
                        destinationField.Add(element);
                    }
                }
                else
                {
                    var sourceField = field.Accessor.GetValue(source);
                    if (field.FieldType == FieldType.Message)
                    {
                        if (options.ReplaceMessageFields)
                        {
                            if (sourceField == null)
                            {
                                field.Accessor.Clear(destination);
                            }
                            else
                            {
                                field.Accessor.SetValue(destination, sourceField);
                            }
                        }
                        else
                        {
                            if (sourceField != null)
                            {
                                var sourceByteString = ((IMessage)sourceField).ToByteString();
                                var destinationValue = (IMessage)field.Accessor.GetValue(destination);
                                if (destinationValue != null)
                                {
                                    destinationValue.MergeFrom(sourceByteString);
                                }
                                else
                                {
                                    field.Accessor.SetValue(destination, field.MessageType.Parser.ParseFrom(sourceByteString));
                                }
                            }
                        }
                    }
                    else
                    {
                        if (sourceField != null
                            || !options.ReplacePrimitiveFields)
                        {
                            field.Accessor.SetValue(destination, sourceField);
                        }
                        else
                        {
                            field.Accessor.Clear(destination);
                        }
                    }
                }
            }
        }
    }
}
