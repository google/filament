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

using NUnit.Framework;
using System;

namespace Google.Protobuf.WellKnownTypes
{
    public class TimestampTest
    {
        [Test]
        public void FromAndToDateTime()
        {
            DateTime utcMin = DateTime.SpecifyKind(DateTime.MinValue, DateTimeKind.Utc);
            DateTime utcMax = DateTime.SpecifyKind(DateTime.MaxValue, DateTimeKind.Utc);
            AssertRoundtrip(new Timestamp { Seconds = -62135596800 }, utcMin);
            AssertRoundtrip(new Timestamp { Seconds = 253402300799, Nanos = 999999900 }, utcMax);
            AssertRoundtrip(new Timestamp(), new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc));
            AssertRoundtrip(new Timestamp { Nanos = 1000000}, new DateTime(1970, 1, 1, 0, 0, 0, 1, DateTimeKind.Utc));
            AssertRoundtrip(new Timestamp { Seconds = -1, Nanos = 999000000 }, new DateTime(1969, 12, 31, 23, 59, 59, 999, DateTimeKind.Utc));
            AssertRoundtrip(new Timestamp { Seconds = 3600 }, new DateTime(1970, 1, 1, 1, 0, 0, DateTimeKind.Utc));
            AssertRoundtrip(new Timestamp { Seconds = -3600 }, new DateTime(1969, 12, 31, 23, 0, 0, DateTimeKind.Utc));
        }

        [Test]
        public void ToDateTimeTruncation()
        {
            var t1 = new Timestamp { Seconds = 1, Nanos = 1000000 + Duration.NanosecondsPerTick - 1 };
            Assert.AreEqual(new DateTime(1970, 1, 1, 0, 0, 1, DateTimeKind.Utc).AddMilliseconds(1), t1.ToDateTime());

            var t2 = new Timestamp { Seconds = -1, Nanos = 1000000 + Duration.NanosecondsPerTick - 1 };
            Assert.AreEqual(new DateTime(1969, 12, 31, 23, 59, 59).AddMilliseconds(1), t2.ToDateTime());
        }

        [Test]
        [TestCase(Timestamp.UnixSecondsAtBclMinValue - 1, Timestamp.MaxNanos)]
        [TestCase(Timestamp.UnixSecondsAtBclMaxValue + 1, 0)]
        [TestCase(0, -1)]
        [TestCase(0, Timestamp.MaxNanos + 1)]
        public void ToDateTime_OutOfRange(long seconds, int nanoseconds)
        {
            var value = new Timestamp { Seconds = seconds, Nanos = nanoseconds };
            Assert.Throws<InvalidOperationException>(() => value.ToDateTime());
        }

        // 1ns larger or smaller than the above values
        [Test]
        [TestCase(Timestamp.UnixSecondsAtBclMinValue, 0)]
        [TestCase(Timestamp.UnixSecondsAtBclMaxValue, Timestamp.MaxNanos)]
        [TestCase(0, 0)]
        [TestCase(0, Timestamp.MaxNanos)]
        public void ToDateTime_ValidBoundaries(long seconds, int nanoseconds)
        {
            var value = new Timestamp { Seconds = seconds, Nanos = nanoseconds };
            value.ToDateTime();
        }

        private static void AssertRoundtrip(Timestamp timestamp, DateTime dateTime)
        {
            Assert.AreEqual(timestamp, Timestamp.FromDateTime(dateTime));
            Assert.AreEqual(dateTime, timestamp.ToDateTime());
            Assert.AreEqual(DateTimeKind.Utc, timestamp.ToDateTime().Kind);
        }

        [Test]
        public void Arithmetic()
        {
            Timestamp t1 = new Timestamp { Seconds = 10000, Nanos = 5000 };
            Timestamp t2 = new Timestamp { Seconds = 8000, Nanos = 10000 };
            Duration difference = new Duration { Seconds = 1999, Nanos = Duration.NanosecondsPerSecond - 5000 };
            Assert.AreEqual(difference, t1 - t2);
            Assert.AreEqual(-difference, t2 - t1);

            Assert.AreEqual(t1, t2 + difference);
            Assert.AreEqual(t2, t1 - difference);
        }

        [Test]
        public void ToString_NonNormalized()
        {
            // Just a single example should be sufficient...
            var duration = new Timestamp { Seconds = 1, Nanos = -1 };
            Assert.AreEqual("{ \"@warning\": \"Invalid Timestamp\", \"seconds\": \"1\", \"nanos\": -1 }", duration.ToString());
        }
    }
}
