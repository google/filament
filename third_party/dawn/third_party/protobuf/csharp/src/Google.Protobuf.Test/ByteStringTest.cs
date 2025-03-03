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
using System.Text;
using NUnit.Framework;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Buffers;
using System.Runtime.InteropServices;
using System.Threading;
using System.Runtime.CompilerServices;
#if !NET35
using System.Threading.Tasks;
#endif

namespace Google.Protobuf
{
    public class ByteStringTest
    {
        [Test]
        public void Equality()
        {
            ByteString b1 = ByteString.CopyFrom(1, 2, 3);
            ByteString b2 = ByteString.CopyFrom(1, 2, 3);
            ByteString b3 = ByteString.CopyFrom(1, 2, 4);
            ByteString b4 = ByteString.CopyFrom(1, 2, 3, 4);
            EqualityTester.AssertEquality(b1, b1);
            EqualityTester.AssertEquality(b1, b2);
            EqualityTester.AssertInequality(b1, b3);
            EqualityTester.AssertInequality(b1, b4);
            EqualityTester.AssertInequality(b1, null);
            EqualityTester.AssertEquality(ByteString.Empty, ByteString.Empty);
#pragma warning disable 1718 // Deliberately calling ==(b1, b1) and !=(b1, b1)
            Assert.IsTrue(b1 == b1);
            Assert.IsTrue(b1 == b2);
            Assert.IsFalse(b1 == b3);
            Assert.IsFalse(b1 == b4);
            Assert.IsFalse(b1 == null);
            Assert.IsTrue((ByteString) null == null);
            Assert.IsFalse(b1 != b1);
            Assert.IsFalse(b1 != b2);
            Assert.IsTrue(ByteString.Empty == ByteString.Empty);
#pragma warning disable 1718
            Assert.IsTrue(b1 != b3);
            Assert.IsTrue(b1 != b4);
            Assert.IsTrue(b1 != null);
            Assert.IsFalse((ByteString) null != null);
        }

        [Test]
        public void EmptyByteStringHasZeroSize()
        {
            Assert.AreEqual(0, ByteString.Empty.Length);
        }

        [Test]
        public void CopyFromStringWithExplicitEncoding()
        {
            ByteString bs = ByteString.CopyFrom("AB", Encoding.Unicode);
            Assert.AreEqual(4, bs.Length);
            Assert.AreEqual(65, bs[0]);
            Assert.AreEqual(0, bs[1]);
            Assert.AreEqual(66, bs[2]);
            Assert.AreEqual(0, bs[3]);
        }

        [Test]
        public void IsEmptyWhenEmpty()
        {
            Assert.IsTrue(ByteString.CopyFromUtf8("").IsEmpty);
        }

        [Test]
        public void IsEmptyWhenNotEmpty()
        {
            Assert.IsFalse(ByteString.CopyFromUtf8("X").IsEmpty);
        }

        [Test]
        public void CopyFromByteArrayCopiesContents()
        {
            byte[] data = new byte[1];
            data[0] = 10;
            ByteString bs = ByteString.CopyFrom(data);
            Assert.AreEqual(10, bs[0]);
            data[0] = 5;
            Assert.AreEqual(10, bs[0]);
        }

        [Test]
        public void CopyFromReadOnlySpanCopiesContents()
        {
            byte[] data = new byte[1];
            data[0] = 10;
            ReadOnlySpan<byte> byteSpan = data;
            var bs = ByteString.CopyFrom(byteSpan);
            Assert.AreEqual(10, bs[0]);
            data[0] = 5;
            Assert.AreEqual(10, bs[0]);
        }

        [Test]
        public void ToByteArrayCopiesContents()
        {
            ByteString bs = ByteString.CopyFromUtf8("Hello");
            byte[] data = bs.ToByteArray();
            Assert.AreEqual((byte)'H', data[0]);
            Assert.AreEqual((byte)'H', bs[0]);
            data[0] = 0;
            Assert.AreEqual(0, data[0]);
            Assert.AreEqual((byte)'H', bs[0]);
        }

        [Test]
        public void CopyFromUtf8UsesUtf8()
        {
            ByteString bs = ByteString.CopyFromUtf8("\u20ac");
            Assert.AreEqual(3, bs.Length);
            Assert.AreEqual(0xe2, bs[0]);
            Assert.AreEqual(0x82, bs[1]);
            Assert.AreEqual(0xac, bs[2]);
        }

        [Test]
        public void CopyFromPortion()
        {
            byte[] data = new byte[] {0, 1, 2, 3, 4, 5, 6};
            ByteString bs = ByteString.CopyFrom(data, 2, 3);
            Assert.AreEqual(3, bs.Length);
            Assert.AreEqual(2, bs[0]);
            Assert.AreEqual(3, bs[1]);
        }

        [Test]
        public void CopyTo()
        {
            byte[] data = new byte[] { 0, 1, 2, 3, 4, 5, 6 };
            ByteString bs = ByteString.CopyFrom(data);

            byte[] dest = new byte[data.Length];
            bs.CopyTo(dest, 0);

            CollectionAssert.AreEqual(data, dest);
        }

        [Test]
        public void GetEnumerator()
        {
            byte[] data = new byte[] { 0, 1, 2, 3, 4, 5, 6 };
            ByteString bs = ByteString.CopyFrom(data);

            IEnumerator<byte> genericEnumerator = bs.GetEnumerator();
            Assert.IsTrue(genericEnumerator.MoveNext());
            Assert.AreEqual(0, genericEnumerator.Current);

            IEnumerator enumerator = ((IEnumerable)bs).GetEnumerator();
            Assert.IsTrue(enumerator.MoveNext());
            Assert.AreEqual(0, enumerator.Current);

            // Call via LINQ
            CollectionAssert.AreEqual(bs.Span.ToArray(), bs.ToArray());
        }

        [Test]
        public void UnsafeWrap()
        {
            byte[] data = new byte[] { 0, 1, 2, 3, 4, 5, 6 };
            ByteString bs = UnsafeByteOperations.UnsafeWrap(data.AsMemory(2, 3));
            ReadOnlySpan<byte> s = bs.Span;

            Assert.AreEqual(3, s.Length);
            Assert.AreEqual(2, s[0]);
            Assert.AreEqual(3, s[1]);
            Assert.AreEqual(4, s[2]);

            // Check that the value is not a copy
            data[2] = byte.MaxValue;
            Assert.AreEqual(byte.MaxValue, s[0]);
        }

        [Test]
        public void CreateCodedInput_FromArraySegment()
        {
            byte[] data = new byte[] { 0, 1, 2, 3, 4, 5, 6 };
            ByteString bs = UnsafeByteOperations.UnsafeWrap(data.AsMemory(2, 3));
            CodedInputStream codedInputStream = bs.CreateCodedInput();

            byte[] bytes = codedInputStream.ReadRawBytes(3);

            Assert.AreEqual(3, bytes.Length);
            Assert.AreEqual(2, bytes[0]);
            Assert.AreEqual(3, bytes[1]);
            Assert.AreEqual(4, bytes[2]);
            Assert.IsTrue(codedInputStream.IsAtEnd);
        }

        [Test]
        public void WriteToStream()
        {
            byte[] data = new byte[] { 0, 1, 2, 3, 4, 5, 6 };
            ByteString bs = ByteString.CopyFrom(data);

            MemoryStream ms = new MemoryStream();
            bs.WriteTo(ms);

            CollectionAssert.AreEqual(data, ms.ToArray());
        }

        [Test]
        public void WriteToStream_Stackalloc()
        {
            byte[] data = Encoding.UTF8.GetBytes("Hello world");
            Span<byte> s = stackalloc byte[data.Length];
            data.CopyTo(s);

            MemoryStream ms = new MemoryStream();

            using (UnmanagedMemoryManager<byte> manager = new UnmanagedMemoryManager<byte>(s))
            {
                ByteString bs = ByteString.AttachBytes(manager.Memory);

                bs.WriteTo(ms);
            }

            CollectionAssert.AreEqual(data, ms.ToArray());
        }

        [Test]
        public void ToStringUtf8()
        {
            ByteString bs = ByteString.CopyFromUtf8("\u20ac");
            Assert.AreEqual("\u20ac", bs.ToStringUtf8());
        }

        [Test]
        public void ToStringWithExplicitEncoding()
        {
            ByteString bs = ByteString.CopyFrom("\u20ac", Encoding.Unicode);
            Assert.AreEqual("\u20ac", bs.ToString(Encoding.Unicode));
        }

        [Test]
        public void ToString_Stackalloc()
        {
            byte[] data = Encoding.UTF8.GetBytes("Hello world");
            Span<byte> s = stackalloc byte[data.Length];
            data.CopyTo(s);

            using (UnmanagedMemoryManager<byte> manager = new UnmanagedMemoryManager<byte>(s))
            {
                ByteString bs = ByteString.AttachBytes(manager.Memory);

                Assert.AreEqual("Hello world", bs.ToString(Encoding.UTF8));
            }
        }

        [Test]
        public void FromBase64_WithText()
        {
            byte[] data = new byte[] {0, 1, 2, 3, 4, 5, 6};
            string base64 = Convert.ToBase64String(data);
            ByteString bs = ByteString.FromBase64(base64);
            Assert.AreEqual(data, bs.ToByteArray());
        }

        [Test]
        public void FromBase64_Empty()
        {
            // Optimization which also fixes issue 61.
            Assert.AreSame(ByteString.Empty, ByteString.FromBase64(""));
        }

        [Test]
        public void ToBase64_Array()
        {
            ByteString bs = ByteString.CopyFrom(Encoding.UTF8.GetBytes("Hello world"));

            Assert.AreEqual("SGVsbG8gd29ybGQ=", bs.ToBase64());
        }

        [Test]
        public void ToBase64_Stackalloc()
        {
            byte[] data = Encoding.UTF8.GetBytes("Hello world");
            Span<byte> s = stackalloc byte[data.Length];
            data.CopyTo(s);

            using (UnmanagedMemoryManager<byte> manager = new UnmanagedMemoryManager<byte>(s))
            {
                ByteString bs = ByteString.AttachBytes(manager.Memory);

                Assert.AreEqual("SGVsbG8gd29ybGQ=", bs.ToBase64());
            }
        }

        [Test]
        public void FromStream_Seekable()
        {
            var stream = new MemoryStream(new byte[] { 1, 2, 3, 4, 5 });
            // Consume the first byte, just to test that it's "from current position"
            stream.ReadByte();
            var actual = ByteString.FromStream(stream);
            ByteString expected = ByteString.CopyFrom(2, 3, 4, 5);
            Assert.AreEqual(expected, actual, $"{expected.ToBase64()} != {actual.ToBase64()}");
        }

        [Test]
        public void FromStream_NotSeekable()
        {
            var stream = new MemoryStream(new byte[] { 1, 2, 3, 4, 5 });
            // Consume the first byte, just to test that it's "from current position"
            stream.ReadByte();
            // Wrap the original stream in LimitedInputStream, which has CanSeek=false
            var limitedStream = new LimitedInputStream(stream, 3);
            var actual = ByteString.FromStream(limitedStream);
            ByteString expected = ByteString.CopyFrom(2, 3, 4);
            Assert.AreEqual(expected, actual, $"{expected.ToBase64()} != {actual.ToBase64()}");
        }

#if !NET35
        [Test]
        public async Task FromStreamAsync_Seekable()
        {
            var stream = new MemoryStream(new byte[] { 1, 2, 3, 4, 5 });
            // Consume the first byte, just to test that it's "from current position"
            stream.ReadByte();
            var actual = await ByteString.FromStreamAsync(stream);
            ByteString expected = ByteString.CopyFrom(2, 3, 4, 5);
            Assert.AreEqual(expected, actual, $"{expected.ToBase64()} != {actual.ToBase64()}");
        }

        [Test]
        public async Task FromStreamAsync_NotSeekable()
        {
            var stream = new MemoryStream(new byte[] { 1, 2, 3, 4, 5 });
            // Consume the first byte, just to test that it's "from current position"
            stream.ReadByte();
            // Wrap the original stream in LimitedInputStream, which has CanSeek=false
            var limitedStream = new LimitedInputStream(stream, 3);
            var actual = await ByteString.FromStreamAsync(limitedStream);
            ByteString expected = ByteString.CopyFrom(2, 3, 4);
            Assert.AreEqual(expected, actual, $"{expected.ToBase64()} != {actual.ToBase64()}");
        }
#endif

        [Test]
        public void GetHashCode_Regression()
        {
            // We used to have an awful hash algorithm where only the last four
            // bytes were relevant. This is a regression test for
            // https://github.com/protocolbuffers/protobuf/issues/2511

            ByteString b1 = ByteString.CopyFrom(100, 1, 2, 3, 4);
            ByteString b2 = ByteString.CopyFrom(200, 1, 2, 3, 4);
            Assert.AreNotEqual(b1.GetHashCode(), b2.GetHashCode());
        }

        [Test]
        public void GetContentsAsReadOnlySpan()
        {
            var byteString = ByteString.CopyFrom(1, 2, 3, 4, 5);
            var copied = byteString.Span.ToArray();
            CollectionAssert.AreEqual(byteString, copied);
        }

        [Test]
        public void GetContentsAsReadOnlyMemory()
        {
            var byteString = ByteString.CopyFrom(1, 2, 3, 4, 5);
            var copied = byteString.Memory.ToArray();
            CollectionAssert.AreEqual(byteString, copied);
        }

        // Create Memory<byte> from non-array source.
        // Use by ByteString tests that have optimized path for array backed Memory<byte>.
        private sealed unsafe class UnmanagedMemoryManager<T> : MemoryManager<T> where T : unmanaged
        {
            private readonly T* _pointer;
            private readonly int _length;

            public UnmanagedMemoryManager(Span<T> span)
            {
                fixed (T* ptr = &MemoryMarshal.GetReference(span))
                {
                    _pointer = ptr;
                    _length = span.Length;
                }
            }

            public override Span<T> GetSpan() => new Span<T>(_pointer, _length);

            public override MemoryHandle Pin(int elementIndex = 0)
            {
                if (elementIndex < 0 || elementIndex >= _length)
                {
                    throw new ArgumentOutOfRangeException(nameof(elementIndex));
                }

                return new MemoryHandle(_pointer + elementIndex);
            }

            public override void Unpin() { }

            protected override void Dispose(bool disposing) { }
        }
    }
}