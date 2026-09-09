///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HlslHost.cs                                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides support for working with an out-of-process rendering host.       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

using System;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Xml;

namespace MainNs
{
    internal class HlslHost
    {
        #region Private fields.

        /// <summary>Host component.</summary>
        private CHlslHost host;

        #endregion Private fields.

        #region Internal classes.

        internal enum HhMessageId : uint
        {
            GetPidMsgId = 1,
            ShutdownMsgId = 2,
            StartRendererMsgId = 3,
            StopRendererMsgId = 4,
            SetPayloadMsgId = 5,
            ReadLogMsgId = 6,
            SetSizeMsgId = 7,
            SetParentHwndMsgId = 8,
            GetPidMsgReplyId = 100 + GetPidMsgId,
            StartRendererMsgReplyId = 100 + StartRendererMsgId,
            StopRendererMsgReplyId = 100 + StopRendererMsgId,
            SetPayloadMsgReplyId = 100 + SetPayloadMsgId,
            ReadLogMsgReplyId = 100 + ReadLogMsgId,
            SetSizeMsgReplyId = 100 + SetSizeMsgId,
            SetParentHwndMsgReplyId = 100 + SetParentHwndMsgId,
        }

        internal class HhMessageReply
        {
            public HhMessageReply(uint kind)
            {
                Kind = kind;
            }
            public uint Kind;
        }

        internal class HhGetPidReply : HhMessageReply
        {
            public HhGetPidReply(uint pid) : base(GetPidMsgReplyId)
            {
                Pid = pid;
            }
            public uint Pid;
        }

        internal class HhResultReply : HhMessageReply
        {
            public HhResultReply(uint kind, uint hresult) : base(kind)
            {
                HResult = hresult;
            }
            public uint HResult;
        }

        internal class HhLogReply : HhMessageReply
        {
            public HhLogReply(string log) : base(ReadLogMsgReplyId)
            {
                Log = log;
            }
            public string Log;
        }

        #endregion Internal classes.

        #region Public properties.

        internal bool IsActive
        {
            get
            {
                return this.host != null;
            }
            set
            {
                if (!value)
                {
                    // Tear down host.
                    if (host != null)
                    {
                        try
                        {
                            SendHostMessage(ShutdownMsgId);
                        }
                        catch (System.Runtime.InteropServices.COMException)
                        {
                            host = null;
                        }

                    }
                    return;
                }

                EnsureActive();
            }
        }

        #endregion Public properties.

        #region Public methods.

        internal HhMessageReply GetReply()
        {
            try
            {
                var str = host as System.Runtime.InteropServices.ComTypes.IStream;
                byte[] response = new byte[8];
                str.Read(response, (int)HhMessageHeader.FixedSize, IntPtr.Zero);
                System.IO.BinaryReader r = new System.IO.BinaryReader(new System.IO.MemoryStream(response));
                uint len = r.ReadUInt32();
                if (len == 0) return null;
                uint kind = r.ReadUInt32();
                switch (kind)
                {
                    case GetPidMsgReplyId:
                        str.Read(response, 4, IntPtr.Zero);
                        return new HhGetPidReply(BytesAsUInt32(response));
                    case StartRendererMsgReplyId:
                    case StopRendererMsgReplyId:
                    case SetPayloadMsgReplyId:
                    case (uint)HhMessageId.SetParentHwndMsgReplyId:
                        str.Read(response, 4, IntPtr.Zero);
                        return new HhResultReply(kind, BytesAsUInt32(response));
                    case ReadLogMsgReplyId:
                        response = new byte[len - 8];
                        str.Read(response, response.Length, IntPtr.Zero);
                        System.IO.BinaryReader logReader = new System.IO.BinaryReader(new System.IO.MemoryStream(response), Encoding.Unicode);
                        uint charCount = logReader.ReadUInt32(); // text size.
                        string text = new string(logReader.ReadChars((int)charCount));
                        return new HhLogReply(text);
                    default:
                        throw new InvalidOperationException("Unknown reply kind from host: " + kind);
                }
            }
            catch (Exception runError)
            {
                System.Diagnostics.Debug.WriteLine(runError);
                host = null;
                throw;
            }
        }

        internal void SendHostMessage(HhMessageId kind)
        {
            SendHostMessage((uint)kind);
        }

        internal void SendHostMessagePlay(string payload)
        {
            var str = host as System.Runtime.InteropServices.ComTypes.IStream;
            HhMessageHeader h = new HhMessageHeader();
            h.Kind = SetPayloadMsgId;
            byte[] payloadBytes = Encoding.UTF8.GetBytes(payload);
            h.Length = (uint)(HhMessageHeader.FixedSize + payloadBytes.Length + 1);
            var stream = new System.IO.MemoryStream();
            var writer = new System.IO.BinaryWriter(stream);
            writer.Write(h.Length);
            writer.Write(h.Kind);
            writer.Write(payloadBytes);
            writer.Write('\0');
            writer.Flush();
            str.Write(stream.ToArray(), (int)h.Length, IntPtr.Zero);
        }

        internal void SetParentHwnd(IntPtr handle)
        {
            var str = host as System.Runtime.InteropServices.ComTypes.IStream;
            HhMessageHeader h = new HhMessageHeader();
            h.Kind = (uint)HhMessageId.SetParentHwndMsgId;
            h.Length = (uint)(HhMessageHeader.FixedSize + sizeof(UInt64));
            var stream = new System.IO.MemoryStream();
            var writer = new System.IO.BinaryWriter(stream);
            writer.Write(h.Length);
            writer.Write(h.Kind);
            writer.Write(handle.ToInt64());
            writer.Flush();
            str.Write(stream.ToArray(), (int)h.Length, IntPtr.Zero);
        }

        internal static string GetLogReplyText(HhMessageReply reply)
        {
            if (reply == null)
                return null;
            HhGetPidReply pidReply = reply as HhGetPidReply;
            if (pidReply != null)
            {
                return "PID for host process is " + pidReply.Pid;
            }
            HhResultReply resultReply = reply as HhResultReply;
            if (resultReply != null)
            {
                return "Operation result for " + HhKindToText(resultReply.Kind) +
                    ": " + HResultToString(resultReply.HResult);
            }
            HhLogReply logReply = reply as HhLogReply;
            if (logReply != null)
            {
                return logReply.Log;
            }
            return null;
        }

        internal static string GetShaderOpPayload(string shaderText, string xml)
        {
            System.Xml.XmlDocument d = new System.Xml.XmlDocument();
            d.LoadXml(xml);
            XmlElement opElement;
            if (d.DocumentElement.LocalName != "ShaderOpSet")
            {
                var setElement = d.CreateElement("ShaderOpSet");
                opElement = (XmlElement)d.RemoveChild(d.DocumentElement);
                setElement.AppendChild(opElement);
                d.AppendChild(setElement);
            }
            else
            {
                opElement = d.DocumentElement;
            }
            if (opElement.LocalName != "ShaderOp")
            {
                throw new InvalidOperationException("Expected 'ShaderOp' or 'ShaderOpSet' root elements.");
            }

            var shaders = opElement.ChildNodes.OfType<XmlElement>().Where(elem => elem.LocalName == "Shader").ToList();
            foreach (var shader in shaders)
            {
                if (!shader.HasChildNodes || String.IsNullOrWhiteSpace(shader.InnerText))
                {
                    shader.InnerText = shaderText;
                }
            }

            return d.OuterXml;
        }

        internal void EnsureActive()
        {
            uint pid;
            if (TryGetProcessId(out pid))
            {
                return;
            }

            // Setup a communication channel.
            try
            {
                host = new CHlslHost();
            }
            catch (System.Runtime.InteropServices.COMException ctorErr)
            {
                System.Diagnostics.Debug.WriteLine(ctorErr);
                // Start the host process.
                ProcessStartInfo psi = new ProcessStartInfo("HLSLHost.exe");
                try
                {
                    Process p = System.Diagnostics.Process.Start(psi);
                }
                catch (System.ComponentModel.Win32Exception startErr)
                {
                    System.Diagnostics.Debug.WriteLine(startErr);
                    throw;
                }
                System.Threading.Thread.Sleep(200);
                host = new CHlslHost();
            }
        }

        internal bool TryGetProcessId(out uint pid)
        {
            pid = 0;
            try
            {
                this.SendHostMessage(HhMessageId.GetPidMsgId);
                for (;;)
                {
                    HhMessageReply reply = this.GetReply();
                    if (reply == null)
                        return false;
                    HhGetPidReply pidReply = reply as HhGetPidReply;
                    if (pidReply != null)
                    {
                        pid = pidReply.Pid;
                        return true;
                    }
                }
            }
            catch (Exception)
            {
                return false;
            }
        }

        #endregion Public methods.

        #region Private methods.

        private void SendHostMessage(uint kind)
        {
            var str = host as System.Runtime.InteropServices.ComTypes.IStream;
            HhMessageHeader h = new HhMessageHeader();
            h.Length = HhMessageHeader.FixedSize;
            h.Kind = kind;
            str.Write(h.ToByteArray(), (int)h.Length, IntPtr.Zero);
        }

        private uint BytesAsUInt32(byte[] bytes)
        {
            return (new System.IO.BinaryReader(new System.IO.MemoryStream(bytes))).ReadUInt32();
        }

        [System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential)]
        public struct HhMessageHeader
        {
            public uint Length;
            public uint Kind;
            public static readonly uint FixedSize = 8;
            public byte[] ToByteArray()
            {
                var stream = new System.IO.MemoryStream();
                var writer = new System.IO.BinaryWriter(stream);
                writer.Write(this.Length);
                writer.Write(this.Kind);
                writer.Flush();
                return stream.ToArray();
            }
        }

        public const uint GetPidMsgId = 1;
        public const uint ShutdownMsgId = 2;
        public const uint StartRendererMsgId = 3;
        public const uint StopRendererMsgId = 4;
        public const uint SetPayloadMsgId = 5;
        public const uint ReadLogMsgId = 6;
        public const uint GetPidMsgReplyId = 100 + GetPidMsgId;
        public const uint StartRendererMsgReplyId = 100 + StartRendererMsgId;
        public const uint StopRendererMsgReplyId = 100 + StopRendererMsgId;
        public const uint SetPayloadMsgReplyId = 100 + SetPayloadMsgId;
        public const uint ReadLogMsgReplyId = 100 + ReadLogMsgId;

        [System.Runtime.InteropServices.ComImport]
        [System.Runtime.InteropServices.Guid("7FD7A859-6C6B-4352-8F1E-C67BB62E774B")]
        class CHlslHost 
        {

        }

        private static string HhKindToText(uint kind)
        {
            return ((HhMessageId)kind).ToString();
        }

        private static string HResultToString(uint hr)
        {
            if (hr == 0) return "OK";
            if (hr == 1) return "S_FALSE";
            return "0x" + hr.ToString("x");
        }

        #endregion Private methods.
    }
}
