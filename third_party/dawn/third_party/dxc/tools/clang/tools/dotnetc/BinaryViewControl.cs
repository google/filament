///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// BinaryViewControl.cs                                                      //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace MainNs
{
    public class BinaryViewControl : Control
    {
        private const int LeftMargin = 8;
        private const int RightMargin = 8;
        private const int TopMargin = 8;
        private const int BytesPerLine = 8;
        private const int BottomMargin = 8;
        private const int ContentGutter = 8;
        private const int AddressCount = 4;

        private byte[] bytes;
        private int LineCount;
        private float LineHeight;
        private float CharWidth;
        private int selectionOffset;
        private int selectionLength;
        private int selectionEnd;
        private int selectionByteOffset;
        private int selectionByteLength;

        public BinaryViewControl()
        {

        }

        public byte[] Bytes
        {
            get { return this.bytes; }
            set
            {
                this.bytes = value;
                this.OnBytesChanged();
            }
        }

        public void SetSelection(int offset, int length)
        {
            this.selectionOffset = offset;
            this.selectionLength = length;
            this.selectionByteOffset = offset / 8;
            this.selectionByteLength = (offset + length) / 8 - this.selectionByteOffset;
            if (length % 8 > 0)
                this.selectionByteLength += 1;
            this.selectionEnd = offset + length;
            this.Invalidate();
        }

        private void OnBytesChanged()
        {
            if (this.Bytes == null)
                return;
            using (Graphics g = this.CreateGraphics())
            using (GraphicResources gr = new GraphicResources(g))
            {
                SizeF sampleText = g.MeasureString("Wj", gr.Font);
                LineHeight = sampleText.Height;
                LineCount = (int)Math.Ceiling((float)this.Bytes.Length / BytesPerLine);
                this.Height = (int)(LineHeight * LineCount) + TopMargin + BottomMargin;
                this.CharWidth = sampleText.Width / 2;
                this.Width = (int)(
                    LeftMargin + AddressCount * CharWidth + ContentGutter +
                    BytesPerLine * CharWidth * 8 + ContentGutter * BytesPerLine + RightMargin);
            }
        }

        private static string[] NibbleBits = System.Linq.Enumerable.Range(0, 16).Select(i => NibbleAsBitString(i)).ToArray();
        private static string NibbleAsBitString(int i)
        {
            char[] ch = new char[4];
            ch[0] = (0 == (i & 1 << 3)) ? '0' : '1';
            ch[1] = (0 == (i & 1 << 2)) ? '0' : '1';
            ch[2] = (0 == (i & 1 << 1)) ? '0' : '1';
            ch[3] = (0 == (i & 1)) ? '0' : '1';
            return new string(ch);
        }

        private static string ByteAsBitString(byte b)
        {
            return NibbleBits[(b & 0xF0) >> 4] + NibbleBits[b & 0x0F];
        }

        private string SelectionIntersection(string bits, int byteOffset)
        {
            // Fast cases - no selection, or no intersection.
            if (this.selectionLength == 0)
                return null;
            if (byteOffset < this.selectionByteOffset || this.selectionByteLength + this.selectionByteOffset < byteOffset)
                return null;

            int bitOffset = byteOffset * 8;
            // Last fast case: everything intersects.
            if (this.selectionOffset <= bitOffset && (bitOffset + 8) <= this.selectionEnd)
                return bits;

            StringBuilder sb = new StringBuilder(bits);
            for (int i = 0; i < 8; ++i)
            {
                // If no intersection, clear bit.
                if (bitOffset < this.selectionOffset || this.selectionEnd <= bitOffset)
                {
                    sb[bits.Length - i - 1] = ' ';
                }
                ++bitOffset;
            }

            return sb.ToString();
        }

        public class BitEventArgs : EventArgs
        {
            public readonly int BitOffset;
            public BitEventArgs(int bitOffset)
            {
                this.BitOffset = bitOffset;
            }
        }
        public event EventHandler<BitEventArgs> BitClick;
        public event EventHandler<BitEventArgs> BitMouseMove;

        public class HitTestResult
        {
            public int X;
            public int Y;
            public int Line;
            public bool AddressHit;
            public bool IsHit;
            public int BitOffset;
        }

        private HitTestResult HitTest(MouseEventArgs e)
        {
            if (this.bytes.Length == 0 || LineHeight == 0)
            {
                return new HitTestResult();
            }

            int x = e.X;
            int y = e.Y;
            int line = (int)((y - TopMargin) / LineHeight);
            bool addressHit;

            int bitOffset = line * BytesPerLine * 8;

            // Remove the left margin, address and gutter.
            x -= LeftMargin;
            x -= (int)(AddressCount * CharWidth);
            x -= ContentGutter;
            if (x > 0)
            {
                addressHit = false;

                // See how many bytes to remove.
                float byteWidth = ContentGutter + CharWidth * 8;
                int byteCount = (int)(x / byteWidth);
                x -= (int)(byteCount * byteWidth);
                bitOffset += byteCount * 8;
            }
            else
            {
                addressHit = true;
            }

            // Snap to valid values.
            if (bitOffset < 0)
            {
                bitOffset = 0;
            }
            else if (bitOffset >= this.bytes.Length * 8)
            {
                bitOffset = this.bytes.Length * 8 - 1;
            }

            return new HitTestResult()
            {
                X = e.X,
                Y = e.Y,
                Line = line,
                AddressHit = addressHit,
                BitOffset = bitOffset,
                IsHit = true
            };
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);
            HitTestResult hitTest = HitTest(e);
            if (hitTest.IsHit && !hitTest.AddressHit)
                this.BitMouseMove(this, new BitEventArgs(hitTest.BitOffset));
        }

        protected override void OnMouseClick(MouseEventArgs e)
        {
            base.OnMouseClick(e);
            if (e.Button != MouseButtons.Left)
                return;

            HitTestResult hitTest = HitTest(e);
            if (hitTest.IsHit && !hitTest.AddressHit)
                this.BitClick(this, new BitEventArgs(hitTest.BitOffset));
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            if (this.Bytes == null)
                return;

            using (GraphicResources gr = new GraphicResources(e.Graphics))
            {
                int offset = 0;
                float y = TopMargin;
                while (offset < this.Bytes.Length)
                {
                    // Start a new line.
                    float x = LeftMargin;

                    // Write the offset on the left margin.
                    string offsetText = offset.ToString("X4");
                    e.Graphics.DrawString(offsetText, gr.Font, gr.BrushForAddress, x, y);
                    x += AddressCount * CharWidth;

                    for (int i = 0; i < BytesPerLine; ++i)
                    {
                        if (offset == this.Bytes.Length)
                            break;
                        byte b = this.Bytes[offset];
                        x += ContentGutter;
                        string bits = ByteAsBitString(b);
                        e.Graphics.DrawString(bits, gr.Font, gr.BrushForNonSelection, x, y);
                        bits = SelectionIntersection(bits, offset);
                        if (bits != null)
                        {
                            e.Graphics.DrawString(bits, gr.Font, gr.BrushForSelection, x, y);
                        }
                        x += 8 * CharWidth;
                        offset++;
                    }

                    y += LineHeight;
                }
            }
        }

        class GraphicResources : IDisposable
        {
            private Font fixedFont;

            internal GraphicResources(Graphics graphics)
            {
                fixedFont = new Font("Consolas", 10);
            }

            public void Dispose()
            {
                fixedFont.Dispose();
            }

            public Font Font
            {
                get { return fixedFont; }
            }

            public Brush BrushForAddress
            {
                get { return Brushes.Black; }
            }

            public Brush BrushForNonSelection
            {
                get { return Brushes.Gray; }
            }

            public Brush BrushForSelection
            {
                get { return Brushes.Black; }
            }
        }
    }

    [DebuggerDisplay("Range {Offset} +{Length}")]
    public class TreeNodeRange
    {
        public TreeNodeRange(int offset, int length)
        {
            Debug.Assert(length > 0);
            Offset = offset;
            Length = length;
        }

        public int Offset;
        public int Length;
        public bool Contains(int bitOffset)
        {
            return Offset <= bitOffset && bitOffset < Offset + Length;
        }

        public static TreeNode For(string text)
        {
            return new TreeNode(text);
        }
        public static TreeNode For(string text, int offset, int length)
        {
            TreeNode result = new TreeNode(text);
            result.Tag = new TreeNodeRange(offset, length);
            return result;
        }
    }
}
