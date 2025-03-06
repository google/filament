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

package com.google.protobuf;

import static com.google.common.truth.Truth.assertThat;
import static com.google.protobuf.IsValidUtf8TestUtil.DIRECT_NIO_FACTORY;
import static com.google.protobuf.IsValidUtf8TestUtil.EXPECTED_ONE_BYTE_ROUNDTRIPPABLE_COUNT;
import static com.google.protobuf.IsValidUtf8TestUtil.EXPECTED_THREE_BYTE_ROUNDTRIPPABLE_COUNT;
import static com.google.protobuf.IsValidUtf8TestUtil.HEAP_NIO_FACTORY;
import static com.google.protobuf.IsValidUtf8TestUtil.LITERAL_FACTORY;
import static com.google.protobuf.IsValidUtf8TestUtil.ROPE_FACTORY;
import static com.google.protobuf.IsValidUtf8TestUtil.testBytes;

import com.google.protobuf.IsValidUtf8TestUtil.ByteStringFactory;
import com.google.protobuf.IsValidUtf8TestUtil.Shard;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/**
 * Tests cases for {@link ByteString#isValidUtf8()}. This includes three brute force tests that
 * actually test every permutation of one byte, two byte, and three byte sequences to ensure that
 * the method produces the right result for every possible byte encoding where "right" means it's
 * consistent with java's UTF-8 string encoding/decoding such that the method returns true for any
 * sequence that will round trip when converted to a String and then back to bytes and will return
 * false for any sequence that will not round trip. See also {@link IsValidUtf8FourByteTest}. It
 * also includes some other more targeted tests.
 */
@RunWith(JUnit4.class)
public class IsValidUtf8Test {
  /** Tests that round tripping of all two byte permutations work. */
  @Test
  public void testIsValidUtf8_1Byte() {
    testBytes(LITERAL_FACTORY, 1, EXPECTED_ONE_BYTE_ROUNDTRIPPABLE_COUNT);
    testBytes(HEAP_NIO_FACTORY, 1, EXPECTED_ONE_BYTE_ROUNDTRIPPABLE_COUNT);
    testBytes(DIRECT_NIO_FACTORY, 1, EXPECTED_ONE_BYTE_ROUNDTRIPPABLE_COUNT);
    testBytes(ROPE_FACTORY, 1, EXPECTED_ONE_BYTE_ROUNDTRIPPABLE_COUNT);
  }

  /** Tests that round tripping of all two byte permutations work. */
  @Test
  public void testIsValidUtf8_2Bytes() {
    testBytes(LITERAL_FACTORY, 2, IsValidUtf8TestUtil.EXPECTED_TWO_BYTE_ROUNDTRIPPABLE_COUNT);
    testBytes(HEAP_NIO_FACTORY, 2, IsValidUtf8TestUtil.EXPECTED_TWO_BYTE_ROUNDTRIPPABLE_COUNT);
    testBytes(DIRECT_NIO_FACTORY, 2, IsValidUtf8TestUtil.EXPECTED_TWO_BYTE_ROUNDTRIPPABLE_COUNT);
    testBytes(ROPE_FACTORY, 2, IsValidUtf8TestUtil.EXPECTED_TWO_BYTE_ROUNDTRIPPABLE_COUNT);
  }

  /** Tests that round tripping of all three byte permutations work. */
  @Test
  public void testIsValidUtf8_3Bytes() {
    testBytes(LITERAL_FACTORY, 3, EXPECTED_THREE_BYTE_ROUNDTRIPPABLE_COUNT);
    testBytes(HEAP_NIO_FACTORY, 3, EXPECTED_THREE_BYTE_ROUNDTRIPPABLE_COUNT);
    testBytes(DIRECT_NIO_FACTORY, 3, EXPECTED_THREE_BYTE_ROUNDTRIPPABLE_COUNT);
    testBytes(ROPE_FACTORY, 3, EXPECTED_THREE_BYTE_ROUNDTRIPPABLE_COUNT);
  }

  /**
   * Tests that round tripping of a sample of four byte permutations work. All permutations are
   * prohibitively expensive to test for automated runs; {@link IsValidUtf8FourByteTest} is used for
   * full coverage. This method tests specific four-byte cases.
   */
  @Test
  public void testIsValidUtf8_4BytesSamples() {
    // Valid 4 byte.
    assertValidUtf8(0xF0, 0xA4, 0xAD, 0xA2);

    // Bad trailing bytes
    assertInvalidUtf8(0xF0, 0xA4, 0xAD, 0x7F);
    assertInvalidUtf8(0xF0, 0xA4, 0xAD, 0xC0);

    // Special cases for byte2
    assertInvalidUtf8(0xF0, 0x8F, 0xAD, 0xA2);
    assertInvalidUtf8(0xF4, 0x90, 0xAD, 0xA2);
  }

  /** Tests some hard-coded test cases. */
  @Test
  public void testSomeSequences() {
    // Empty
    assertThat(asBytes("").isValidUtf8()).isTrue();

    // One-byte characters, including control characters
    assertThat(asBytes("\u0000abc\u007f").isValidUtf8()).isTrue();

    // Two-byte characters
    assertThat(asBytes("\u00a2\u00a2").isValidUtf8()).isTrue();

    // Three-byte characters
    assertThat(asBytes("\u020ac\u020ac").isValidUtf8()).isTrue();

    // Four-byte characters
    assertThat(asBytes("\u024B62\u024B62").isValidUtf8()).isTrue();

    // Mixed string
    assertThat(asBytes("a\u020ac\u00a2b\\u024B62u020acc\u00a2de\u024B62").isValidUtf8()).isTrue();

    // Not a valid string
    assertInvalidUtf8(-1, 0, -1, 0);
  }

  @Test
  public void testShardsHaveExpectedRoundTrippables() {
    // A sanity check.
    int actual = 0;
    for (Shard shard : IsValidUtf8TestUtil.FOUR_BYTE_SHARDS) {
      actual = (int) (actual + shard.expected);
    }
    assertThat(actual).isEqualTo(IsValidUtf8TestUtil.EXPECTED_FOUR_BYTE_ROUNDTRIPPABLE_COUNT);
  }

  private byte[] toByteArray(int... bytes) {
    byte[] realBytes = new byte[bytes.length];
    for (int i = 0; i < bytes.length; i++) {
      realBytes[i] = (byte) bytes[i];
    }
    return realBytes;
  }

  private void assertValidUtf8(ByteStringFactory factory, int[] bytes, boolean not) {
    byte[] realBytes = toByteArray(bytes);
    assertThat(not ^ Utf8.isValidUtf8(realBytes)).isTrue();
    assertThat(not ^ Utf8.isValidUtf8(realBytes, 0, bytes.length)).isTrue();
    ByteString leaf = factory.newByteString(realBytes);
    ByteString sub = leaf.substring(0, bytes.length);
    assertThat(not ^ leaf.isValidUtf8()).isTrue();
    assertThat(not ^ sub.isValidUtf8()).isTrue();
    ByteString[] ropes = {
      RopeByteString.newInstanceForTest(ByteString.EMPTY, leaf),
      RopeByteString.newInstanceForTest(ByteString.EMPTY, sub),
      RopeByteString.newInstanceForTest(leaf, ByteString.EMPTY),
      RopeByteString.newInstanceForTest(sub, ByteString.EMPTY),
      RopeByteString.newInstanceForTest(sub, leaf)
    };
    for (ByteString rope : ropes) {
      assertThat(not ^ rope.isValidUtf8()).isTrue();
    }
  }

  private void assertValidUtf8(int... bytes) {
    assertValidUtf8(LITERAL_FACTORY, bytes, false);
    assertValidUtf8(HEAP_NIO_FACTORY, bytes, false);
    assertValidUtf8(DIRECT_NIO_FACTORY, bytes, false);
    assertValidUtf8(ROPE_FACTORY, bytes, false);
  }

  private void assertInvalidUtf8(int... bytes) {
    assertValidUtf8(LITERAL_FACTORY, bytes, true);
    assertValidUtf8(HEAP_NIO_FACTORY, bytes, true);
    assertValidUtf8(DIRECT_NIO_FACTORY, bytes, true);
    assertValidUtf8(ROPE_FACTORY, bytes, true);
  }

  private static ByteString asBytes(String s) {
    return ByteString.copyFromUtf8(s);
  }
}
