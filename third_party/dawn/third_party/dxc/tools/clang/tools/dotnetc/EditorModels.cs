///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// EditorModels.cs                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides support for model classes used by the editor UI.                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

using DotNetDxc;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Xml.Linq;

namespace MainNs
{
    public class DiagnosticDetail
    {
        [DisplayName("Error")]
        public int ErrorCode { get; set; }
        [DisplayName("Line")]
        public int ErrorLine { get; set; }
        [DisplayName("Column")]
        public int ErrorColumn { get; set; }
        [DisplayName("File")]
        public string ErrorFile { get; set; }
        [DisplayName("Offset")]
        public int ErrorOffset { get; set; }
        [DisplayName("Length")]
        public int ErrorLength { get; set; }
        [DisplayName("Message")]
        public string ErrorMessage { get; set; }
    }

    [DebuggerDisplay("{Name}")]
    class PassArgInfo
    {
        public string Name { get; set; }
        public string Description { get; set; }
        public PassInfo PassInfo { get; set; }
        public override string ToString()
        {
            return Name;
        }
    }

    [DebuggerDisplay("{Arg.Name} = {Value}")]
    class PassArgValueInfo
    {
        public PassArgInfo Arg { get; set; }
        public string Value { get; set; }
        public override string ToString()
        {
            if (String.IsNullOrEmpty(Value))
                return Arg.Name;
            return Arg.Name + "=" + Value;
        }
    }

    [DebuggerDisplay("{Name}")]
    class PassInfo
    {
        public string Name { get; set; }
        public string Description { get; set; }
        public PassArgInfo[] Args { get; set; }
        public static PassInfo FromOptimizerPass(IDxcOptimizerPass pass)
        {
            PassInfo result = new PassInfo()
            {
                Name = pass.GetOptionName(),
                Description = pass.GetDescription()
            };
            PassArgInfo[] args = new PassArgInfo[pass.GetOptionArgCount()];
            for (int i = 0; i < pass.GetOptionArgCount(); ++i)
            {
                PassArgInfo info = new PassArgInfo()
                {
                    Name = pass.GetOptionArgName((uint)i),
                    Description = pass.GetOptionArgDescription((uint)i),
                    PassInfo = result
                };
                args[i] = info;
            }
            result.Args = args;
            return result;
        }
        public override string ToString()
        {
            return Name;
        }
    }

    class PassInfoWithValues
    {
        public PassInfoWithValues(PassInfo pass)
        {
            this.PassInfo = pass;
            this.Values = new List<PassArgValueInfo>();
        }
        public PassInfo PassInfo { get; set; }
        public List<PassArgValueInfo> Values { get; set; }
        public override string ToString()
        {
            string result = this.PassInfo.Name;
            if (this.Values.Count == 0)
                return result;
            result += String.Concat(this.Values.Select(v => "," + v.ToString()));
            return result;
        }
    }

    class MRUManager
    {
        #region Private fields.

        private List<string> MRUFiles = new List<string>();

        #endregion Private fields.

        #region Constructors.

        public MRUManager()
        {
            this.MaxCount = 8;
            this.MRUPath =
                System.IO.Path.Combine(
                    System.Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                    "dndxc",
                    "mru.txt");
        }

        #endregion Constructors.

        #region Public properties.

        public int MaxCount { get; set; }

        public string MRUPath { get; set; }

        public IEnumerable<string> Paths
        {
            get { return this.MRUFiles; }
        }

        #endregion Public properties.

        #region Public methods.

        public void LoadFromFile()
        {
            this.LoadFromFile(this.MRUPath);
        }

        public void LoadFromFile(string path)
        {
            if (!System.IO.File.Exists(path))
                return;
            this.MRUFiles = System.IO.File.ReadAllLines(path).ToList();
        }

        public void SaveToFile()
        {
            this.SaveToFile(this.MRUPath);
        }

        public void SaveToFile(string path)
        {
            string dirName = System.IO.Path.GetDirectoryName(path);
            if (!System.IO.Directory.Exists(dirName))
                System.IO.Directory.CreateDirectory(dirName);
            System.IO.File.WriteAllLines(path, this.MRUFiles);
        }

        public void HandleFileLoad(string path)
        {
            this.HandleFileSave(path);
        }

        public void HandleFileSave(string path)
        {
            path = System.IO.Path.GetFullPath(path);
            int index = this.MRUFiles.IndexOf(path);
            if (index >= 0)
                this.MRUFiles.RemoveAt(index);
            this.MRUFiles.Insert(0, path);
            while (this.MRUFiles.Count > this.MaxCount)
                this.MRUFiles.RemoveAt(this.MRUFiles.Count - 1);
        }

        public void HandleFileFail(string path)
        {
            path = System.IO.Path.GetFullPath(path);
            int index = this.MRUFiles.IndexOf(path);
            if (index >= 0)
                this.MRUFiles.RemoveAt(index);
        }

        #endregion Public methods.
    }

    /// <summary>Use this class to represent a range of int values.</summary>
    [DebuggerDisplay("{Lo} - {Hi}")]
    struct NumericRange
    {
        /// <summary>Low range, inclusive.</summary>
        public int Lo;
        /// <summary>High range, exclusive.</summary>
        public int Hi;

        public NumericRange(int lo, int hi)
        {
            this.Lo = lo;
            this.Hi = hi;
        }

        public bool Equals(NumericRange other)
        {
            return this.IsEmpty && other.IsEmpty ||
                this.Lo == other.Lo && this.Hi == other.Hi;
        }

        public override bool Equals(object other)
        {
            return other is NumericRange && Equals((NumericRange)other);
        }

        public override int GetHashCode()
        {
            return this.Lo | (this.Hi & 0xffff) << 16;
        }

        public static bool operator==(NumericRange left, NumericRange right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(NumericRange left, NumericRange right)
        {
            return !left.Equals(right);
        }

        public bool IsEmpty { get { return Lo == Hi; } }

        public bool Contains(int val)
        {
            return (Lo <= val && val < Hi);
        }

        public bool Contains(NumericRange range)
        {
            return (Lo <= range.Lo && range.Hi <= Hi);
        }

        public bool IsAdjacent(NumericRange other)
        {
            return !this.IsEmpty && !other.IsEmpty &&
                (other.Hi == this.Lo || this.Hi == other.Lo);
        }

        public bool IsDisjoint(NumericRange other)
        {
            return this.Hi <= other.Lo || other.Hi <= this.Lo;
        }

        public bool Subtract(NumericRange other, out NumericRange reminder)
        {
            // If empty, nothing to subtract.
            reminder = new NumericRange();
            if (this.IsEmpty || other.IsEmpty)
            {
                return false;
            }

            // If disjoint, result is unchanged.
            if (IsDisjoint(other))
            {
                return false;
            }

            // If fully contained, result is empty.
            if (other.Contains(this))
            {
                this.Lo = 0;
                this.Hi = 0;
                return true;
            }

            // If only low edge is contained, the high segment is a reminder.
            if (other.Contains(this.Lo) && !other.Contains(this.Hi - 1))
            {
                this.Lo = other.Hi;
                return true;
            }
            // If only high edge is contained, the low segment is a reminder.
            if (other.Contains(this.Hi - 1) && !other.Contains(this.Lo))
            {
                this.Hi = other.Lo;
                return true;
            }
            // In this case, the other result is contained within, so the
            // subtraction produces two segments: 'this' keeps the low end,
            // and 'reminder' takes the high end.
            reminder.Hi = this.Hi;
            this.Hi = other.Lo;
            reminder.Lo = other.Hi;
            return true;
        }

        public bool TryCoalesce(NumericRange other, out NumericRange coalesced)
        {
            // If either is empty, position is meaningless.
            // Coalescing can happen unless disjoint.
            if (this.IsEmpty || other.IsEmpty || (IsDisjoint(other) && !IsAdjacent(other)))
            {
                coalesced = new NumericRange();
                return false;
            }

            int newLo = Math.Min(this.Lo, other.Lo);
            int newHi = Math.Max(this.Hi, other.Hi);
            coalesced = new NumericRange(newLo, newHi);
            return true;
        }

        public override string ToString()
        {
            return this.Lo.ToString() + "-" + this.Hi.ToString();
        }
    }

    /// <summary>Use this class to represent multiple ranges that can be implicitly coalesced and split.</summary>
    class NumericRanges : IEnumerable<NumericRange>
    {
        /// <summary>Ordered ranges.</summary>
        private List<NumericRange> ranges = new List<NumericRange>();

        public void Add(int lo, int hi)
        {
            if (lo == hi) return;
            // Trivial common case - appending to last.
            if (this.ranges.Count > 0)
            {
                NumericRange range = this.ranges[this.ranges.Count - 1];
                if (range.Lo <= lo && lo <= range.Hi)
                {
                    if (hi <= range.Hi)
                    {
                        return; // already contained
                    }
                    this.ranges[this.ranges.Count - 1] = new NumericRange(range.Lo, hi);
                    return;
                }
            }

            // To keep this simple, add the new range by lo end, then coalesce.
            bool added = false;
            int coalesce = 0;
            for (int i = 0; i < ranges.Count; ++i)
            {
                if (lo < ranges[i].Lo)
                {
                    // Comes prior to this range.
                    ranges.Insert(i, new NumericRange(lo, hi));
                    added = true;
                    coalesce = i - 1;
                    break;
                }
                if (hi <= ranges[i].Hi)
                {
                    // Fully overlapped.
                    return;
                }
            }
            if (!added)
            {
                ranges.Add(new NumericRange(lo, hi));
                coalesce = ranges.Count - 2;
            }

            // Coalesce a few items ahead from our starting point.
            coalesce = Math.Max(0, coalesce);
            for (int i = coalesce; i < coalesce + 2;)
            {
                int next = i + 1;
                if (next >= ranges.Count)
                    return;
                NumericRange range;
                if (ranges[i].TryCoalesce(ranges[next], out range))
                {
                    ranges[i] = range;
                    ranges.RemoveAt(next);
                }
                else
                {
                    ++i;
                }
            }
        }

        public void Add(NumericRange range)
        {
            Add(range.Lo, range.Hi);
        }

        public void Clear()
        {
            this.ranges.Clear();
        }

        public bool Contains(NumericRange range)
        {
            foreach (var r in ranges)
            {
                if (range.Hi < r.Lo)
                {
                    break;
                }
                if (r.Contains(range))
                {
                    return true;
                }
            }
            return false;
        }

        public bool IsEmpty
        {
            get { return this.ranges.Count == 0; }
        }

        public IEnumerator<NumericRange> GetEnumerator()
        {
            return this.ranges.GetEnumerator();
        }

        public NumericRanges ListGaps(NumericRange range)
        {
            NumericRanges result = new NumericRanges();
            result.Add(range);
            foreach (var r in ranges)
            {
                result.Remove(r);
            }
            return result;
        }

        public void Remove(NumericRange range)
        {
            for (int i = 0; i < ranges.Count;)
            {
                NumericRange r = ranges[i];
                if (range.Hi < r.Lo)
                    return;
                NumericRange remainder;
                if (r.Subtract(range, out remainder))
                {
                    if (r.IsEmpty)
                    {
                        ranges.RemoveAt(i);
                    }
                    else
                    {
                        ranges[i] = r;
                        if (!remainder.IsEmpty)
                        {
                            ranges.Insert(i + 1, remainder);
                        }
                    }
                }
                else
                {
                    i++;
                }
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.ranges.GetEnumerator();
        }

        public override string ToString()
        {
            return String.Join(",", ranges.Select(r => r.ToString()));
        }
    }

    class SettingsManager
    {
        #region Private fields.

        private XDocument doc = new XDocument();

        #endregion Private fields.

        #region Constructors.

        public SettingsManager()
        {
            this.doc = new XDocument(new XElement("settings"));
            this.SettingsPath =
                System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory,
                    "dndxc.settings.xml");
        }

        #endregion Constructors.

        #region Public properties.

        internal string SettingsPath { get; set; }
        [Description("The name of an external DLL implementing the compiler.")]
        public string ExternalLib
        {
            get { return this.GetPathTextOrDefault("", "external", "lib"); }
            set { this.SetPathText(value, "external", "lib"); }
        }
        [Description("The name of the factory function export on the external DLL implementing the compiler.")]
        public string ExternalFunction
        {
            get { return this.GetPathTextOrDefault("", "external", "fun"); }
            set { this.SetPathText(value, "external", "fun"); }
        }
        [Description("The command to run in place of View | Render, replacing %in with XML path.")]
        public string ExternalRenderCommand
        {
            get { return this.GetPathTextOrDefault("", "external-render", "command"); }
            set { this.SetPathText(value, "external-render", "command"); }
        }
        [Description("Whether to use an external render tool.")]
        public bool ExternalRenderEnabled
        {
            get { return bool.Parse(this.GetPathTextOrDefault("false", "external-render", "enabled")); }
            set { this.SetPathText(value.ToString(), "external-render", "enabled"); }
        }

        #endregion Public properties.

        #region Public methods.

        public void LoadFromFile()
        {
            this.LoadFromFile(this.SettingsPath);
        }

        public void LoadFromFile(string path)
        {
            if (!System.IO.File.Exists(path))
                return;
            this.doc = XDocument.Load(path);
        }

        public void SaveToFile()
        {
            this.SaveToFile(this.SettingsPath);
        }

        public void SaveToFile(string path)
        {
            string dirName = System.IO.Path.GetDirectoryName(path);
            if (!System.IO.Directory.Exists(dirName))
                System.IO.Directory.CreateDirectory(dirName);
            this.doc.Save(path);
        }

        #endregion Public methods.

        #region Private methods.

        private string GetPathTextOrDefault(string defaultValue, params string[] paths)
        {
            var element = this.doc.Root;
            foreach (string path in paths)
            {
                element = element.Element(XName.Get(path));
                if (element == null) return defaultValue;
            }
            return element.Value;
        }

        private void SetPathText(string value, params string[] paths)
        {
            var element = this.doc.Root;
            foreach (string path in paths)
            {
                var next = element.Element(XName.Get(path));
                if (next == null)
                {
                    next = new XElement(XName.Get(path));
                    element.Add(next);
                }
                element = next;
            }
            element.Value = value;
        }

        #endregion Private methods.
    }

    class ContainerData
    {
        public static System.Windows.Forms.DataFormats.Format DataFormat =
            System.Windows.Forms.DataFormats.GetFormat("DXBC");

        public static string BlobToBase64(IDxcBlob blob)
        {
            return System.Convert.ToBase64String(BlobToBytes(blob));
        }

        public static byte[] BlobToBytes(IDxcBlob blob)
        {
            byte[] bytes;
            unsafe
            {
                char* pBuffer = blob.GetBufferPointer();
                uint size = blob.GetBufferSize();
                bytes = new byte[size];
                IntPtr ptr = new IntPtr(pBuffer);
                System.Runtime.InteropServices.Marshal.Copy(ptr, bytes, 0, (int)size);
            }
            return bytes;
        }

        public static byte[] DataObjectToBytes(object data)
        {
            System.IO.Stream stream = data as System.IO.Stream;
            if (stream == null)
                return null;
            byte[] bytes = new byte[stream.Length];
            stream.Read(bytes, 0, (int)stream.Length);
            return bytes;
        }

        public static string DataObjectToString(object data)
        {
            byte[] bytes = DataObjectToBytes(data);
            if (bytes == null)
                return "";
            return System.Convert.ToBase64String(bytes);
        }
    }

#if FALSE
    public class NumericRangesTests
    {
        public static void Assert(bool condition)
        {
            Debug.Assert(condition);
        }
        private static NumericRange NR(int lo, int hi) { return new NumericRange(lo, hi); }
        public static void Run()
        {
            NumericRange empty = new NumericRange();
            Assert(empty.IsEmpty);

            NumericRange n = new NumericRange();
            Assert(!n.Contains(0));
            n.Hi = 1;
            Assert(n.Contains(0));
            Assert(!n.Contains(1));
            Assert(n.IsDisjoint(new NumericRange()));
            Assert(n.IsDisjoint(new NumericRange(1, 2)));

            NumericRange remainder;
            // Subtract - empty cases
            Assert(false == n.Subtract(empty, out remainder));
            Assert(remainder.IsEmpty);
            Assert(false == empty.Subtract(n, out remainder));

            // Subtract - disjoint
            Assert(false == NR(0, 3).Subtract(NR(3, 5), out remainder));
            n = NR(0, 3);

            // Subtract - contained
            Assert(n.Subtract(NR(0, 4), out remainder));
            Assert(n.IsEmpty);

            // Subtract - partial overlaps
            n = NR(10, 12);
            Assert(n.Subtract(NR(8, 11), out remainder));
            Assert(n == NR(11, 12));
            Assert(remainder.IsEmpty);
            n = NR(10, 12);
            Assert(n.Subtract(NR(11, 12), out remainder));
            Assert(n == NR(10, 11));
            Assert(remainder.IsEmpty);
            n = NR(10, 21);
            Assert(n.Subtract(NR(13, 17), out remainder));
            Assert(n == NR(10, 13));
            Assert(remainder == NR(17, 21));

            // Coalesce - no-ops.
            NumericRange coalesced;
            n = NR(10, 21);
            Assert(false == n.TryCoalesce(NR(0, 9), out coalesced));
            Assert(false == n.TryCoalesce(empty, out coalesced));
            Assert(n.TryCoalesce(NR(0, 10), out coalesced));
            Assert(coalesced == NR(0, 21));
            n = NR(10, 21);
            Assert(n.TryCoalesce(NR(0, 100), out coalesced));
            Assert(coalesced == NR(0, 100));

            NumericRanges r = new NumericRanges();
            Assert(r.IsEmpty);
            r.Add(0, 10);
            Assert(r.ToString() == "0-10");
            r.Add(0, 4);
            Assert(r.ToString() == "0-10");
            r.Add(20, 20);
            Assert(r.ToString() == "0-10");
            r.Add(20, 30);
            Assert(r.ToString() == "0-10,20-30");
            r.Add(10, 20);
            Assert(r.ToString() == "0-30");
            r.Add(30, 32);
            Assert(r.ToString() == "0-32");

            r.Clear();
            Assert(r.ToString() == "");

            r.Add(NR(0, 10));
            r.Add(NR(20, 30));

            var gaps = r.ListGaps(NR(0, 40)).ToList();
            Assert(gaps[0] == NR(10, 20));
            Assert(gaps[1] == NR(30, 40));
        }
    }
#endif
}
