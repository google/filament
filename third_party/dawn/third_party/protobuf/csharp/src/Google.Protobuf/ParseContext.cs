﻿#region Copyright notice and license
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
using System.Buffers;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;
using Google.Protobuf.Collections;

namespace Google.Protobuf
{
    /// <summary>
    /// An opaque struct that represents the current parsing state and is passed along
    /// as the parsing proceeds.
    /// All the public methods are intended to be invoked only by the generated code,
    /// users should never invoke them directly.
    /// </summary>
    [SecuritySafeCritical]
    public ref struct ParseContext
    {
        internal const int DefaultRecursionLimit = 100;
        internal const int DefaultSizeLimit = Int32.MaxValue;

        internal ReadOnlySpan<byte> buffer;
        internal ParserInternalState state;

        /// <summary>
        /// Initialize a <see cref="ParseContext"/>, building all <see cref="ParserInternalState"/> from defaults and
        /// the given <paramref name="buffer"/>.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static void Initialize(ReadOnlySpan<byte> buffer, out ParseContext ctx)
        {
            ParserInternalState state = default;
            state.sizeLimit = DefaultSizeLimit;
            state.recursionLimit = DefaultRecursionLimit;
            state.currentLimit = int.MaxValue;
            state.bufferSize = buffer.Length;

            Initialize(buffer, ref state, out ctx);
        }

        /// <summary>
        /// Initialize a <see cref="ParseContext"/> using existing <see cref="ParserInternalState"/>, e.g. from <see cref="CodedInputStream"/>.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static void Initialize(ReadOnlySpan<byte> buffer, ref ParserInternalState state, out ParseContext ctx)
        {
            ctx.buffer = buffer;
            ctx.state = state;
        }

        /// <summary>
        /// Creates a ParseContext instance from CodedInputStream.
        /// WARNING: internally this copies the CodedInputStream's state, so after done with the ParseContext,
        /// the CodedInputStream's state needs to be updated.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static void Initialize(CodedInputStream input, out ParseContext ctx)
        {
            ctx.buffer = new ReadOnlySpan<byte>(input.InternalBuffer);
            // ideally we would use a reference to the original state, but that doesn't seem possible
            // so we just copy the struct that holds the state. We will need to later store the state back
            // into CodedInputStream if we want to keep it usable.
            ctx.state = input.InternalState;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static void Initialize(ReadOnlySequence<byte> input, out ParseContext ctx)
        {
            Initialize(input, DefaultRecursionLimit, out ctx);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static void Initialize(ReadOnlySequence<byte> input, int recursionLimit, out ParseContext ctx)
        {
            ctx.buffer = default;
            ctx.state = default;
            ctx.state.lastTag = 0;
            ctx.state.recursionDepth = 0;
            ctx.state.sizeLimit = DefaultSizeLimit;
            ctx.state.recursionLimit = recursionLimit;
            ctx.state.currentLimit = int.MaxValue;
            SegmentedBufferHelper.Initialize(input, out ctx.state.segmentedBufferHelper, out ctx.buffer);
            ctx.state.bufferPos = 0;
            ctx.state.bufferSize = ctx.buffer.Length;

            ctx.state.DiscardUnknownFields = false;
            ctx.state.ExtensionRegistry = null;
        }

        /// <summary>
        /// Returns the last tag read, or 0 if no tags have been read or we've read beyond
        /// the end of the input.
        /// </summary>
        internal uint LastTag { get { return state.lastTag; } }

        /// <summary>
        /// Internal-only property; when set to true, unknown fields will be discarded while parsing.
        /// </summary>
        internal bool DiscardUnknownFields {
            get { return state.DiscardUnknownFields; }
            set { state.DiscardUnknownFields = value; }
        }

        /// <summary>
        /// Internal-only property; provides extension identifiers to compatible messages while parsing.
        /// </summary>
        internal ExtensionRegistry ExtensionRegistry
        {
            get { return state.ExtensionRegistry; }
            set { state.ExtensionRegistry = value; }
        }

        /// <summary>
        /// Reads a field tag, returning the tag of 0 for "end of input".
        /// </summary>
        /// <remarks>
        /// If this method returns 0, it doesn't necessarily mean the end of all
        /// the data in this CodedInputReader; it may be the end of the logical input
        /// for an embedded message, for example.
        /// </remarks>
        /// <returns>The next field tag, or 0 for end of input. (0 is never a valid tag.)</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public uint ReadTag()
        {
            return ParsingPrimitives.ParseTag(ref buffer, ref state);
        }

        /// <summary>
        /// Reads a double field from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public double ReadDouble()
        {
            return ParsingPrimitives.ParseDouble(ref buffer, ref state);
        }

        /// <summary>
        /// Reads a float field from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public float ReadFloat()
        {
            return ParsingPrimitives.ParseFloat(ref buffer, ref state);
        }

        /// <summary>
        /// Reads a uint64 field from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public ulong ReadUInt64()
        {
            return ParsingPrimitives.ParseRawVarint64(ref buffer, ref state);
        }

        /// <summary>
        /// Reads an int64 field from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public long ReadInt64()
        {
            return (long)ParsingPrimitives.ParseRawVarint64(ref buffer, ref state);
        }

        /// <summary>
        /// Reads an int32 field from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public int ReadInt32()
        {
            return (int)ParsingPrimitives.ParseRawVarint32(ref buffer, ref state);
        }

        /// <summary>
        /// Reads a fixed64 field from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public ulong ReadFixed64()
        {
            return ParsingPrimitives.ParseRawLittleEndian64(ref buffer, ref state);
        }

        /// <summary>
        /// Reads a fixed32 field from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public uint ReadFixed32()
        {
            return ParsingPrimitives.ParseRawLittleEndian32(ref buffer, ref state);
        }

        /// <summary>
        /// Reads a bool field from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool ReadBool()
        {
            return ParsingPrimitives.ParseRawVarint64(ref buffer, ref state) != 0;
        }
        /// <summary>
        /// Reads a string field from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public string ReadString()
        {
            return ParsingPrimitives.ReadString(ref buffer, ref state);
        }

        /// <summary>
        /// Reads an embedded message field value from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void ReadMessage(IMessage message)
        {
            ParsingPrimitivesMessages.ReadMessage(ref this, message);
        }

        /// <summary>
        /// Reads an embedded group field from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void ReadGroup(IMessage message)
        {
            ParsingPrimitivesMessages.ReadGroup(ref this, message);
        }

        /// <summary>
        /// Reads a bytes field value from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public ByteString ReadBytes()
        {
            return ParsingPrimitives.ReadBytes(ref buffer, ref state);
        }
        /// <summary>
        /// Reads a uint32 field value from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public uint ReadUInt32()
        {
            return ParsingPrimitives.ParseRawVarint32(ref buffer, ref state);
        }

        /// <summary>
        /// Reads an enum field value from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public int ReadEnum()
        {
            // Currently just a pass-through, but it's nice to separate it logically from WriteInt32.
            return (int)ParsingPrimitives.ParseRawVarint32(ref buffer, ref state);
        }

        /// <summary>
        /// Reads an sfixed32 field value from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public int ReadSFixed32()
        {
            return (int)ParsingPrimitives.ParseRawLittleEndian32(ref buffer, ref state);
        }

        /// <summary>
        /// Reads an sfixed64 field value from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public long ReadSFixed64()
        {
            return (long)ParsingPrimitives.ParseRawLittleEndian64(ref buffer, ref state);
        }

        /// <summary>
        /// Reads an sint32 field value from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public int ReadSInt32()
        {
            return ParsingPrimitives.DecodeZigZag32(ParsingPrimitives.ParseRawVarint32(ref buffer, ref state));
        }

        /// <summary>
        /// Reads an sint64 field value from the input.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public long ReadSInt64()
        {
            return ParsingPrimitives.DecodeZigZag64(ParsingPrimitives.ParseRawVarint64(ref buffer, ref state));
        }

        /// <summary>
        /// Reads a length for length-delimited data.
        /// </summary>
        /// <remarks>
        /// This is internally just reading a varint, but this method exists
        /// to make the calling code clearer.
        /// </remarks>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public int ReadLength()
        {
            return (int)ParsingPrimitives.ParseRawVarint32(ref buffer, ref state);
        }

        internal void CopyStateTo(CodedInputStream input)
        {
            input.InternalState = state;
        }

        internal void LoadStateFrom(CodedInputStream input)
        {
            state = input.InternalState;
        }
    }
}